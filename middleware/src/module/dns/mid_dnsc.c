
#include "mid_dns.h"
#include "mid/mid_msgq.h"
#include "mid/mid_time.h"
#include "independs/ind_net.h"
#include "independs/ind_tmr.h"
#include "mid_dnsc_priv.h"
#include "ind_mem.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include "Assertions.h"

#define LogUserOperPrintf LogUserOperDebug

#define LogUserOperErrOut(args...)  \
do { \
    LogUserOperError(args); \
    goto Err; \
}while(0)


#define DNS_PORT    53

//最大超时时间
#define MID_DNS_TIMEOUT_MAX        60
#define MID_DNS_TIMEOUT_MIN        2


//*********************************

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

//**********************************

static uint32_t     g_sn = 1;
static int          g_nscnt = 0;
static uint32_t     g_nsip[2];

//**********************************

#define HOSTNAME_SIZE    64
#define TIMEOUT_COUNT    4

typedef struct mid_resolve* mid_resolve_t;
struct mid_resolve {
    mid_resolve_t   next;
    uint32_t        clkout;
    mid_dns_f       func;
    int             arg;
    int             sn;
};

typedef struct mid_dns* mid_dns_t;
struct mid_dns {
    mid_dns_t   next;

    uint32_t    sn;
    int         ns_index;
    int         sock;

    int         to_count;
    int         to_index;
    int         to_array[TIMEOUT_COUNT];

    int         sec_step;
    int         sec_count;
    int         sec_timeout;

    char        hostname[HOSTNAME_SIZE];

    mid_resolve_t    resolve;
};

typedef struct int_cache* int_cache_t;
struct int_cache {
    int_cache_t next;
    uint32_t    clkout;
    uint32_t    hostip;
    char        hostname[HOSTNAME_SIZE];
};

typedef struct int_msgq* int_msgq_t;
struct int_msgq {
    uint32_t    sn;
    mid_msgq_t  msgq;
};

struct int_msg {
    uint32_t hostip;
    int      msgno;
};

static mid_dns_t g_dns_list = NULL;
static int_cache_t g_dns_cache = NULL;

#define MSGQ_NUMBER    16
static int_msgq_t g_msgq_array = NULL;

static void int_dns_timer(void *arg);
static int int_dns_resolve(mid_dns_t dns);
static void int_dns_select(mid_dns_t dns);

static mid_msgq_t gMsgq = NULL;
static ind_tlink_t gTlink = NULL;
static int gTimer = 0;

static void *int_dns_task(void *arg)
{
    int fd, mfd, max, msg;
    uint32_t out, clk, clks;
    fd_set rset;
    struct timeval tv;

    mid_dns_t dns;

    mfd = mid_msgq_fd(gMsgq);
    for (;;) {
        clk = mid_10ms( );

        pthread_mutex_lock(&g_mutex);
        if (g_dns_list) {
            if (!gTimer) {
                LogUserOperDebug("timer create\n");
                ind_timer_create(gTlink, clk + 100, 100, int_dns_timer, g_dns_list);
                gTimer = 1;
            }
        } else {
            if (gTimer) {
                LogUserOperDebug("timer delete\n");
                ind_timer_delete(gTlink, int_dns_timer, g_dns_list);
                gTimer = 0;
            }
        }
        pthread_mutex_unlock(&g_mutex);

        out = ind_timer_clock(gTlink);
        if (out <= clk) {
            ind_timer_deal(gTlink, clk);
            continue;
        }
        clks = out - clk;

        tv.tv_sec = clks / 100;
        tv.tv_usec = (clks % 100) * 10000;

        FD_ZERO(&rset);
        FD_SET(mfd, &rset);
        max = mfd;

        pthread_mutex_lock(&g_mutex);
        dns = g_dns_list;
        while (dns) {
            fd = dns->sock;
            if (fd >= 0) {
                FD_SET(fd, &rset);
                if (max < fd)
                    max = fd;
            }
            dns = dns->next;
        }
        pthread_mutex_unlock(&g_mutex);

        if (select(max + 1, &rset, NULL,  NULL, &tv) <= 0)
            continue;

        if (FD_ISSET(mfd, &rset))
            mid_msgq_getmsg(gMsgq, (char *)&msg);

        pthread_mutex_lock(&g_mutex);
        dns = g_dns_list;
        while (dns) {
            fd = dns->sock;
            if (fd >= 0 && FD_ISSET(fd, &rset))
                break;
            dns = dns->next;
        }
        pthread_mutex_unlock(&g_mutex);

        if (dns)
            int_dns_select(dns);
    }
    return NULL;
}

static void int_cache_insert(const char *hostname, uint32_t hostip, uint32_t ttl)
{
    int_cache_t prev, next, cache;

    if (ttl > 86400)
        ttl = 86400;
    cache = (int_cache_t)IND_MALLOC(sizeof(struct int_cache));
    if (cache == NULL)
        LogUserOperErrOut("malloc\n");
    cache->clkout = mid_10ms( ) + ttl * 100;
    cache->hostip = hostip;
    IND_STRCPY(cache->hostname, hostname);

    prev = NULL;
    next = g_dns_cache;
    while (next && next->clkout < cache->clkout) {
        prev = next;
        next = next->next;
    }
    cache->next = next;
    if (prev)
        prev->next = cache;
    else
        g_dns_cache = cache;

Err:
    return;
}

static int_cache_t int_cache_search(const char *hostname)
{
    uint32_t clk;
    int_cache_t cache;

    clk = mid_10ms( );
    while (g_dns_cache && g_dns_cache->clkout < clk) {
        cache = g_dns_cache->next;
        IND_FREE(g_dns_cache);
        g_dns_cache = cache;
    }

    cache = g_dns_cache;
    while (cache) {
        if (strcmp(cache->hostname, hostname) == 0)
            return cache;
        cache = cache->next;
    }
    return NULL;
}

static void int_cache_reset(void)
{
    int_cache_t cache;

    while (g_dns_cache) {
        cache = g_dns_cache->next;
        IND_FREE(g_dns_cache);
        g_dns_cache = cache;
    }
}

/* Build a qname structure. The structure consists of pairs of
   <len><label> where <label> is eg ma in tux.ma.tech.ascom.ch. len is
   the length of the label. */
static int build_qname(char *ptr, const char *hostname)
{
    const char *label = hostname;
    char *end_label;
    int total_len = 0;
    int len;

    while (*label) {
        end_label = (char *)strchr(label, '.'); //WZW modified to fix pc-lint Error 158
        if (end_label == NULL) {
            end_label = (char *)strchr(label, '\0');
        }
        len = end_label - label;
        if (len > 63) {
            return -1;
        }
        *ptr++ = (char) len;            /* Put the length of the label */
        IND_MEMCPY(ptr, label, len);        /* and now the label */
        ptr += len;

        total_len += len +1;
        label = end_label;            /* Move onto the . or the null. */
        if (*label == '.') {            /* Move over the . if there is one */
            label++;
        }
    }
    *ptr = 0;                            /* Add the last length of zero
                                           to mark the end */
    return (total_len+1);
}

/* Given a pointer to a qname, find the length of the qname. This is
   the number of bytes needed to represent the qname, not the length
   of the name. A qname is terminated by either a 00, or a pointer
   into another qname. This pointer has the top two bits set. */
static int qname_len(unsigned char * qname)
{
    unsigned char * ptr = qname;

    while ((*ptr != 0) && ((*ptr & 0xc0) != 0xc0)) {
        ptr += *ptr + 1;
    }
    /* Pointers are two bytes */
    if ((*ptr & 0xc0) == 0xc0) {
        ptr++;
    }
    ptr++;                                /* Skip over the trailing byte */

    return (ptr - qname);
}

/* Build a query message which can be sent to the server. If something
   goes wrong return -1, otherwise the length of the query message */
static int build_query(int sn, char* msg, const char* hostname, short rr_type, uint16_t id)
{
    struct dns_header *dns_hdr;
    char *ptr;
    int len;

    /* Fill out the header */
    dns_hdr = (struct dns_header *) msg;
    dns_hdr->id = htons(id);
    dns_hdr->rd = 1;
    dns_hdr->opcode = DNS_QUERY;
    dns_hdr->qdcount = htons(1);

    /* Now the question we want to ask */
    ptr = (char *)&dns_hdr[1];

    len = build_qname(ptr, hostname);

    if (len < 0)
        LogUserOperErrOut("#%d NO_RECOVERY\n", sn);

    ptr += len;

    /* Set the type and class fields */
    *ptr++ = (rr_type >> 8) & 0xff;
    *ptr++ = rr_type & 0xff;
    *ptr++ = (DNS_CLASS_IN >> 8) & 0xff;
    *ptr++ = DNS_CLASS_IN & 0xff;

    len = ptr - msg;

    return len;
Err:
    return -1;
}

/* Decode the answer from the server.
*/
static int parse_answer(int sn, char* msg, const char* hostname, u_int* hostip)
{
    struct dns_header *dns_hdr;
    struct resource_record rr, *rr_p = NULL;
    char *qname = NULL;
    char *ptr;

    dns_hdr = (struct dns_header *)msg;

    if (DNS_REPLY_NAME_ERROR == dns_hdr->rcode)
        LogUserOperErrOut("#%d HOST_NOT_FOUND\n", sn);

    if ((dns_hdr->qr != 1) ||
        (dns_hdr->opcode != DNS_QUERY) ||
        (dns_hdr->rcode != DNS_REPLY_NOERR))
        LogUserOperErrOut("#%d NO_RECOVERY\n", sn);

    dns_hdr->ancount = ntohs(dns_hdr->ancount);
    dns_hdr->qdcount = ntohs(dns_hdr->qdcount);
    ptr = (char *)&dns_hdr[1];

    /* Skip over the query section */
    if (dns_hdr->qdcount > 0) {
        while (dns_hdr->qdcount) {
            ptr += qname_len((unsigned char *)ptr);
            ptr += 4;                    /* skip type & class */
            dns_hdr->qdcount--;
        }
    }
    /* Skip over the answers resource records to find an answer of the
       correct type. */
    while (dns_hdr->ancount) {
        qname = ptr;
        ptr += qname_len((unsigned char *)ptr);
        rr_p = (struct resource_record *)ptr;
        memcpy(&rr, ptr, sizeof(rr));
        if ((rr.rr_type == htons(DNS_TYPE_A)) &&
            (rr.class == htons(DNS_CLASS_IN))) {
            break;
        }
        ptr += sizeof(struct resource_record) - sizeof(rr.rdata) + ntohs(rr.rdlength);
        dns_hdr->ancount--;
    }

    /* If we found one. decode it */
    if (dns_hdr->ancount <= 0)
        LogUserOperErrOut("#%d NO_DATA\n", sn);

    IND_MEMCPY(hostip, rr_p->rdata, 4);
    int_cache_insert(hostname, *hostip, ntohl(rr_p->ttl));

    return 0;
Err:
    return -1;
}

static void int_dns_timeout(void)
{
    uint32_t clk = mid_10ms( );
    mid_dns_t dhead, dnext, dns;
    mid_resolve_t rhead, rnext, resolve;

    dhead = NULL;
    rhead = NULL;
    pthread_mutex_lock(&g_mutex);

    dns = g_dns_list;
    g_dns_list = NULL;
    while(dns) {//容错处理
        dnext = dns->next;
        {
            resolve = dns->resolve;
            dns->resolve = NULL;

            while (resolve) {
                rnext = resolve->next;
                if (resolve->clkout <= clk) {
                    resolve->next = rhead;
                    rhead = resolve;
                } else {
                    resolve->next = dns->resolve;
                    dns->resolve = resolve;
                }
                resolve = rnext;
            }
        }

        if (dns->resolve) {
            dns->next = g_dns_list;
            g_dns_list = dns;
        } else {
            dns->next = dhead;
            dhead = dns;
        }
        dns = dnext;
    }

    pthread_mutex_unlock(&g_mutex);

    resolve = rhead;
    while (resolve) {
        rnext = resolve->next;

        LogUserOperError("#%d resolve = %p, DNS_MSG_TIMEOUT\n", resolve->sn, resolve);
        if (resolve->func)
            resolve->func(resolve->arg, DNS_MSG_TIMEOUT, 0);

        IND_FREE(resolve);

        resolve = rnext;
    }

    dns = dhead;
    while (dns) {
        dnext = dns->next;

        LogUserOperDebug("#%d delete dns\n", dns->sn);
        if (dns->sock >= 0) {
            int msg = 0;
            mid_msgq_putmsg(gMsgq, (char *)&msg);

            close(dns->sock);
            dns->sock = -1;
        }
        IND_FREE(dns);

        dns = dnext;
    }
}

static void int_dns_timer(void *arg)
{
    mid_dns_t dns;

    int_dns_timeout( );

    pthread_mutex_lock(&g_mutex);

    dns = g_dns_list;
    while(dns) {
        if (-1 == dns->ns_index) {
            dns->ns_index = 0;
            int_dns_resolve(dns);
            continue;
        }

        dns->sec_count ++;

        if (dns->sec_count >= dns->sec_step) {
            if (g_nscnt == 2)
                dns->ns_index = 1 - dns->ns_index;
            if (dns->ns_index == 0)
                dns->to_index ++;

            if (dns->to_index >= dns->to_count)
                dns->to_index = dns->to_count - 1;
            LogUserOperPrintf("#%d ns_index = %d, timeout = %d\n", dns->sn, dns->ns_index, dns->to_array[dns->to_index]);
            if (dns->sec_step < dns->sec_timeout)
                int_dns_resolve(dns);
            dns->sec_step += dns->to_array[dns->to_index];
        }

        dns = dns->next;
    }

    pthread_mutex_unlock(&g_mutex);
}


static void int_dns_select(mid_dns_t dns)
{
    int len, msgno;
    uint32_t hostip;
    char msg[MAXDNSMSGSIZE];
    struct dns_header *dns_hdr;

    mid_resolve_t resolve = NULL;

    dns_hdr = (struct dns_header *)msg;

    pthread_mutex_lock(&g_mutex);

    len = recv(dns->sock, msg, MAXDNSMSGSIZE, 0);
    if (len < (int)sizeof(struct dns_header))
        LogUserOperErrOut("#%d recv len = %d/%d\n", dns->sn, len, sizeof(struct dns_header));

    /* Reply to an old query. Ignore it */
    {
        uint16_t id = ntohs(dns_hdr->id);
        if (id != (uint16_t)dns->sn) {
            WARN_PRN("#%d id = %08x / %08x\n", dns->sn, (uint32_t)id, dns->sn);
            goto Err;
        }
    }

    /* Decode the answer */
    if (parse_answer(dns->sn, msg, dns->hostname, &hostip)) {
        LogUserOperError("#%d DNS_MSG_NOTFOUND\n", dns->sn);
        msgno = DNS_MSG_NOTFOUND;
        hostip = 0;
    } else {
        LogUserOperPrintf("#%d DNS_MSG_OK hostip = %08x\n", dns->sn, hostip);
        msgno = DNS_MSG_OK;
    }

    resolve = dns->resolve;
    dns->resolve = NULL;

Err:
    pthread_mutex_unlock(&g_mutex);

    while (resolve) {
        mid_resolve_t next = resolve->next;

        LogUserOperDebug("func = %p, arg = %d\n", resolve->func, resolve->arg);
        if (resolve->func) {
            resolve->func(resolve->arg, msgno, hostip);
        } else {
            int_msgq_t msgq;
            int idx = resolve->arg;

            pthread_mutex_lock(&g_mutex);
            msgq = &g_msgq_array[idx];
            LogUserOperDebug("sn = %d / %d, msgq = %p\n", resolve->sn, msgq->sn, msgq->msgq);
            if (resolve->sn == msgq->sn) {
                struct int_msg msg;

                msg.msgno = msgno;
                msg.hostip = hostip;
                if (msgq->msgq)
                    mid_msgq_putmsg(msgq->msgq, (char*)&msg);
            }
            pthread_mutex_unlock(&g_mutex);
        }

        IND_FREE(resolve);
        resolve = next;
    }
}

static int int_dns_resolve(mid_dns_t dns)
{
    char msg[MAXDNSMSGSIZE];
    int ret, len;
    struct sockaddr_in sin;

    /* First try the name as passed in */
    IND_MEMSET(msg, 0, sizeof(msg));
    len = build_query(dns->sn, msg, dns->hostname, DNS_TYPE_A, (uint16_t)dns->sn);
    if (len < 0)
        LogUserOperErrOut("#%d build_query\n", dns->sn);

    LogUserOperPrintf("#%d ns_index = %d, server = %08x\n", dns->sn, dns->ns_index, g_nsip[dns->ns_index]);

    memset(&sin, 0, sizeof(sin));
    sin.sin_addr.s_addr = g_nsip[dns->ns_index];
    sin.sin_port = htons(DNS_PORT);//sent->s_port;
    sin.sin_family = AF_INET;

    ret = sendto(dns->sock, msg, len, 0, (struct sockaddr*)&sin, sizeof(sin));
    if (ret != len)
        LogUserOperErrOut("#%d ret = %d / %d errno = %s\n", dns->sn, ret, len, strerror(errno));

    {
        int msg = 0;
        mid_msgq_putmsg(gMsgq, (char *)&msg);
    }

    return 0;
Err:
    return -1;
}

uint32_t int_dns_resolve_to(const char* hostname, mid_dns_f callfunc, int callarg, int timeout, int* to_array, int to_count)
{
    int i, sn, len;
    uint32_t clk, hostip = INADDR_NONE;    //WZW modified to fix pc-lint warning 650

    mid_resolve_t resolve = NULL;

    pthread_mutex_lock(&g_mutex);

    clk = mid_10ms( );

    if (0 == g_sn)
        g_sn ++;
    sn = g_sn ++;

    if (g_nscnt == 0)
        LogUserOperErrOut("#%d server is NULL\n", sn);

    if (timeout < MID_DNS_TIMEOUT_MIN)
        timeout = MID_DNS_TIMEOUT_MIN;
    else if (timeout > MID_DNS_TIMEOUT_MAX)
        timeout = MID_DNS_TIMEOUT_MAX;

    len = strlen(hostname);
    if (len <= 0 ||len >= HOSTNAME_SIZE)
        LogUserOperErrOut("#%d len = %d\n", sn, len);

    if (to_array == NULL || to_count <= 0)
        LogUserOperErrOut("#%d to_array = %p, to_count = %d\n", sn, to_array, to_count);
    for (i = 0; i < to_count; i ++) {
        if (to_array[i] <= 0)
            LogUserOperErrOut("#%d to_array[%d] = %d!\n", sn, i, to_array[i]);
    }

    {
        int_cache_t cache = int_cache_search(hostname);
        if (cache) {
            hostip = cache->hostip;
            LogUserOperDebug("#%d cache hostip = 0x%08x\n", sn, hostip);
            goto Err;
        }
    }

    resolve = (mid_resolve_t)IND_CALLOC(sizeof(struct mid_resolve), 1);
    if (resolve == NULL)
        LogUserOperErrOut("#%d malloc mid_resolve\n", sn);

    resolve->clkout = clk + timeout * 100;
    resolve->func = callfunc;
    resolve->arg = callarg;
    resolve->sn = sn;
    if (NULL == callfunc) {
        LogUserOperDebug("#%d idx = %d\n", sn, callarg);
        g_msgq_array[callarg].sn = sn;
    }

    {
        mid_dns_t elem = g_dns_list;

        while (elem) {
            if (elem->resolve && strcmp(elem->hostname, hostname) == 0)
                break;
            elem = elem->next;
        }

        if (elem) {
            LogUserOperPrintf("#%d resolve = %p, mix dns\n", sn, resolve);
            resolve->next = elem->resolve;
            elem->resolve = resolve;
        } else {
            mid_dns_t dns;

            LogUserOperPrintf("#%d resolve = %p, create dns\n", sn, resolve);

            dns = (mid_dns_t)IND_CALLOC(sizeof(struct mid_dns), 1);
            if (dns == NULL)
                LogUserOperErrOut("#%d malloc mid_dns\n", sn);

            dns->sock = socket(PF_INET, SOCK_DGRAM, 0);
            if (dns->sock < 0) {
                IND_FREE(dns);
                LogUserOperErrOut("#%d socket!\n", sn);
            }

            dns->sn = sn;

            if (to_count > TIMEOUT_COUNT)
                to_count = TIMEOUT_COUNT;
            for (i = 0; i < to_count; i ++)
                dns->to_array[i] = to_array[i];
            dns->to_count = to_count;
            dns->to_index = 0;

            dns->sec_step = dns->to_array[0];
            dns->sec_count = 0;
            dns->sec_timeout = timeout;

            IND_STRCPY(dns->hostname, hostname);

            if (NULL == g_dns_list)
                dns->ns_index = 0;
            else
                dns->ns_index = -1;

            resolve->next = dns->resolve;
            dns->resolve = resolve;

            dns->next = g_dns_list;
            g_dns_list = dns;

            LogUserOperPrintf("create dns = %p\n", dns);
            if (0 == dns->ns_index)
                int_dns_resolve(dns);
        }
    }

    hostip = 0;

Err:
    pthread_mutex_unlock(&g_mutex);

    if (hostip) {
        if (resolve)
            IND_FREE(resolve);
    }

    return hostip;
}

int mid_dns_resolve(const char* hostname, mid_dns_f callfunc, int callarg, int timeout)
{
    int    i, to_array[TIMEOUT_COUNT];
    uint32_t hostip = INADDR_NONE;    //WZW modified to fix pc-lint warning 650

    if (hostname == NULL || hostname[0] == 0)
        LogUserOperErrOut("hostname is NULL\n");

    LogUserOperPrintf("hostname = %s, timeout = %d, nscnt = %d\n", hostname, timeout, g_nscnt);

    if (NULL == callfunc)
        LogUserOperErrOut("callfunc is NULL\n");

    for (i = 0; i < TIMEOUT_COUNT; i ++)
        to_array[i] = 2 << i;
    hostip = int_dns_resolve_to(hostname, callfunc, callarg, timeout, to_array, TIMEOUT_COUNT);

Err:
    if (hostip) {
        if (hostip == INADDR_NONE) {    //WZW modified to fix pc-lint warning 650
            LogUserOperDebug("DNS_MSG_ERROR\n");
            callfunc(callarg, DNS_MSG_ERROR, 0);
        } else {
            LogUserOperDebug("DNS_MSG_OK\n");
            callfunc(callarg, DNS_MSG_OK, hostip);
        }
    }

    return 0;
}

uint32_t mid_dns_cache(const char* hostname)
{
    int_cache_t cache;
    uint32_t hostip = INADDR_NONE;

    pthread_mutex_lock(&g_mutex);

    cache = int_cache_search(hostname);
    if (cache)
        hostip = cache->hostip;

    pthread_mutex_unlock(&g_mutex);
    return hostip;
}

void mid_dns_clean(const char* hostname)
{
    int_cache_t prev, cache;

    pthread_mutex_lock(&g_mutex);

    prev = NULL;
    cache = g_dns_cache;
    while (cache) {
        if (0 == strcmp(cache->hostname, hostname))
            break;
        prev = cache;
        cache = cache->next;
    }
    if (cache) {
        if (prev)
            prev->next = cache->next;
        else
            g_dns_cache = cache->next;
        IND_FREE(cache);
    }

    pthread_mutex_unlock(&g_mutex);
}

/* Given a hostname find out the IP address */
int mid_dns_gethost(const char* hostname, uint32_t *pHostip, int timeout)
{
    struct int_msg msg;
    mid_msgq_t msgq = NULL;

    uint32_t hostip = INADDR_NONE;    //WZW modified to fix pc-lint warning 650
    int    i, idx, to_array[TIMEOUT_COUNT];

    msg.msgno = DNS_MSG_ERROR;

    if (hostname == NULL || hostname[0] == 0)
        LogUserOperErrOut("hostname is NULL\n");

    LogUserOperPrintf("hostname = %s, timeout = %d, nscnt = %d\n", hostname, timeout, g_nscnt);

    msgq = mid_msgq_create(1, sizeof(struct int_msg));
    if (NULL == msgq)
        LogUserOperErrOut("mid_msgq_create\n");

    pthread_mutex_lock(&g_mutex);
    for (idx = 0; idx < MSGQ_NUMBER; idx ++) {
        if (NULL == g_msgq_array[idx].msgq) {
            g_msgq_array[idx].sn = 0;
            g_msgq_array[idx].msgq = msgq;
            break;
        }
    }
    pthread_mutex_unlock(&g_mutex);
    if (idx >= MSGQ_NUMBER)
        LogUserOperErrOut("msq_array is FULL\n");

    for (i = 0; i < TIMEOUT_COUNT; i ++)
        to_array[i] = 2 << i;

    if (strncasecmp(hostname, "http://", 7) == 0)
        hostname += 7;
    hostip = int_dns_resolve_to(hostname, NULL, idx, timeout, to_array, TIMEOUT_COUNT);
    if (INADDR_NONE == hostip)    //WZW modified to fix pc-lint warning 650
        LogUserOperErrOut("int_dns_resolve_to\n");

    if (hostip) {
        *pHostip = hostip;
        msg.msgno = DNS_MSG_OK;
    } else {
        int fd;
        uint32_t clks, clk, out;

        fd_set rset;
        struct timeval tv;

        fd = mid_msgq_fd(msgq);
        out = mid_10ms( ) + timeout * 100;

        while (1) {
            clk = mid_10ms( );
            if (clk >= out)
                break;

            clks = out - clk;
            tv.tv_sec = clks / 100;
            tv.tv_usec = clks % 100 * 10000;

            FD_ZERO(&rset);
            FD_SET(fd, &rset);
            if (select(fd + 1, &rset , NULL,  NULL, &tv) <= 0)
                continue;

            mid_msgq_getmsg(msgq, (char *)(&msg));
            if (msg.msgno)
                LogUserOperErrOut("msg = %d\n", msg.msgno);
            LogUserOperPrintf("DNS_MSG_OK\n");
            *pHostip = msg.hostip;
            goto Err;
        }
        if (clk >= out) {
            LogUserOperError("DNS_MSG_TIMEOUT\n");
            msg.msgno = DNS_MSG_TIMEOUT;
        }
    }
Err:
    if (msgq) {
        if (idx < MSGQ_NUMBER) {
            pthread_mutex_lock(&g_mutex);
            g_msgq_array[idx].sn = 0;
            g_msgq_array[idx].msgq = NULL;
            pthread_mutex_unlock(&g_mutex);
        }
        mid_msgq_delete(msgq);
    }
    return msg.msgno;
}

static int send_query(int sn, int sock, uint32_t serverip, char *msg, int len)
{
    int ret = -1;
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_addr.s_addr = serverip;
    sin.sin_port = htons(DNS_PORT);//sent->s_port;
    sin.sin_family = AF_INET;

    ret = sendto(sock, msg, len, 0, (struct sockaddr*)&sin, sizeof(sin));
    if (ret != len)
        LogUserOperErrOut("#%d ret = %d, len = %d / err = %s\n", sn, ret, len, strerror(errno));

    return 0;
Err:
    return -1;
}

static int recv_queue(uint32_t sn, int sock, const char* hostname, u_int* hostip, int tvsec)
{
    int ret = -1;
    char msg[MAXDNSMSGSIZE];
    int len;

    struct dns_header *dns_hdr;
    struct timeval tv;
    fd_set rset;
    uint32_t outclk, clk;
    uint16_t id;

    outclk = mid_10ms( ) + tvsec * 100;

    dns_hdr = (struct dns_header *)msg;

    for (;;) {
        clk = mid_10ms( );
        if (clk >= outclk)
            break;

        FD_ZERO(&rset);
        FD_SET(sock, &rset);

        clk = outclk - clk;
        tv.tv_sec = clk / 100;
        tv.tv_usec = clk % 100 * 10000;

        len = select(sock + 1, &rset, NULL, NULL, &tv);
        if (len < 0)
            LogUserOperErrOut("#%d select / err = %s\n", sn, strerror(errno));
        if (len == 0)
            continue;

        len = recv(sock, msg, MAXDNSMSGSIZE, 0);
        if (len < (int)sizeof(struct dns_header))
            LogUserOperErrOut("#%d read len = %d/%d\n", sn, len, sizeof(struct dns_header));

        /* Reply to an old query. Ignore it */
        id = ntohs(dns_hdr->id);
        if (id != (uint16_t)sn) {
            WARN_PRN("#%d id = %08x / %08x\n", sn, (uint32_t)id, sn);
            continue;
        }

        /* Decode the answer */
        if (parse_answer(sn, msg, hostname, hostip)) {
            LogUserOperError("#%d DNS_MSG_NOTFOUND\n", sn);
            ret = DNS_MSG_NOTFOUND;
            goto Err;
        }
        return DNS_MSG_OK;
    }

    return DNS_MSG_TIMEOUT;
Err:
    return ret;
}

int mid_dns_gethost_to(const char* hostname, u_int *hostip, int timeout, int* to_array, int to_count)
{
    int cnt, idx, len, ret = -1;
    int sock = -1;
    int to_index, outsec, tvsec;
    uint32_t sn, nsip[2];
    char msg[MAXDNSMSGSIZE];

    if (hostname == NULL || hostname[0] == 0 || hostip == NULL)
        LogUserOperErrOut("hostname = %p, hostip = %p\n", hostname, hostip);

    if (to_array == NULL || to_count <= 0)
        LogUserOperErrOut("to_array = %p, to_count = %d\n", to_array, to_count);
    for (idx = 0; idx < to_count; idx ++) {
        if (to_array[idx] <= 0)
            LogUserOperErrOut("to_array[%d] = %d!\n", idx, to_array[idx]);
    }

    pthread_mutex_lock(&g_mutex);

    if (0 == g_sn)
        g_sn ++;
    sn = g_sn ++;

    cnt = g_nscnt;
    if (cnt == 1 || cnt == 2) {
        int_cache_t cache;

        cnt = g_nscnt;
        idx = 0;
        nsip[0] = g_nsip[0];
        nsip[1] = g_nsip[1];

        cache = int_cache_search(hostname);
        if (cache) {
            LogUserOperDebug("#%d cache %08x for %s\n", sn, cache->hostip, hostname);
            *hostip = cache->hostip;
            ret = 0;
            cnt = 0;
        }
    } else {
        LogUserOperError("#%d g_nscnt = %d\n", sn, cnt);
    }
    pthread_mutex_unlock(&g_mutex);
    if (cnt != 1 && cnt != 2) {
        //LogUserOperError("#%d cnt = %d\n", sn, cnt);
        return ret;
    }

    if (timeout < MID_DNS_TIMEOUT_MIN)
        timeout = MID_DNS_TIMEOUT_MIN;
    else if (timeout > MID_DNS_TIMEOUT_MAX)
        timeout = MID_DNS_TIMEOUT_MAX;

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        LogUserOperErrOut("#%d socket!\n", sn);

    to_index = 0;
    tvsec = to_array[to_index];
    outsec = timeout;
    while (outsec > 0) {
        if (tvsec > outsec)
            tvsec = outsec;

        IND_MEMSET(msg, 0, sizeof(msg));
        len = build_query(sn, msg, hostname, DNS_TYPE_A, (uint16_t)sn);
        if (len < 0)
            LogUserOperErrOut("#%d build_query\n", sn);
        LogUserOperPrintf("#%d idx = %d, server = %08x\n", sn, idx, g_nsip[idx]);
        if (send_query(sn, sock, g_nsip[idx], msg, len))
            LogUserOperErrOut("#%d send_query\n", sn);

        ret = recv_queue(sn, sock, hostname, hostip, tvsec);
        if (ret != DNS_MSG_TIMEOUT) {
            if (ret >= 0) {
                LogUserOperDebug("#%d DNS_MSG_OK hostip = %08x\n", sn, *hostip);
            } else {
                LogUserOperError("#%d ret = %d\n", sn, ret);
            }
            goto Err;/*End*/
        }

        if (cnt == 2)
            idx = 1 - idx;
        outsec -= tvsec;

        if (idx == 0)
            to_index ++;
        if (to_index >= to_count)
            tvsec = outsec;
        else
            tvsec = to_array[to_index];
    }

    LogUserOperError("#%d DNS_MSG_TIMEOUT\n", sn);
    ret = DNS_MSG_TIMEOUT;
Err:
    if (sock != -1)
        close(sock);
    return ret;
}

void mid_dns_setsrv(char* dns1, char* dns2)
{
    u_int ip;

    if (!gMsgq)
        mid_dns_init( );

    if (dns1)
        LogUserOperPrintf("dns1 = %s\n", dns1);
    if (dns2)
        LogUserOperPrintf("dns2 = %s\n", dns2);

    pthread_mutex_lock(&g_mutex);

    if (dns1 == NULL)
        LogUserOperErrOut("dns1 is NULL\n");

    ip = inet_addr(dns1);
    if (ip == INADDR_ANY || ip == INADDR_NONE)
        LogUserOperErrOut("dsn1 = %s\n", dns1);
    g_nsip[0] = ip;
    g_nscnt = 1;

    if (dns2 == NULL || strcmp(dns1, dns2) == 0)
        goto End;

    ip = inet_addr(dns2);
    if (ip == INADDR_ANY || ip == INADDR_NONE) {
        LogUserOperError("dsn2 = %s\n", dns2);
        goto End;
    }
    g_nsip[1] = ip;
    g_nscnt = 2;

End:
    int_cache_reset( );
Err:
    pthread_mutex_unlock(&g_mutex);
}

void mid_dns_init(void)
{
    pthread_t thrd;

    if (gMsgq)
        return;

    LogUserOperPrintf("SIZE: mid_dns = %d\n", sizeof(struct mid_dns));
    LogUserOperPrintf("SIZE: int_cache = %d\n", sizeof(struct int_cache));

    gMsgq = mid_msgq_create(10, sizeof(int));
    gTlink = ind_tlink_create(10);
    g_msgq_array = (int_msgq_t)IND_CALLOC(sizeof(struct int_msgq), MSGQ_NUMBER);

    pthread_create(&thrd, NULL, int_dns_task, NULL);
}

//TODO come from mid_net.c 
static char g_dns1[64] = "0.0.0.0";
static char g_dns2[64] = "0.0.0.0";
int mid_net_dns_set(char* dns1, char* dns2)
{
    if(inet_addr(dns1) == INADDR_NONE) {
        LogRunOperError("Dns1(%s)\n", dns1);
        return -1;
    }
    if(dns2 && inet_addr(dns2) == INADDR_NONE) {
        LogRunOperError("Dns2(%s)\n", dns2);
        return -1;
    }

    LogRunOperDebug("Prepare to set dns1<%s>, dns2<%s> to system\n", dns1, dns2);
    strcpy(g_dns1, dns1);
    if(dns2)
        strcpy(g_dns2, dns2);

    mid_dns_setsrv(dns1, dns2);
    if(yos_net_setDNSServer(g_dns1, g_dns2))
        LogRunOperError("yos_net_setDNSServer\n");

    return 0;
}

int mid_net_dns_get(char* dns1, char* dns2)
{
    if(dns1)
        strcpy(dns1, g_dns1);
    if(dns2)
        strcpy(dns2, g_dns2);
    return 0;
}

#define PROC_LEN 100
void mid_net_igmpver_set2proc(const char *itrface, int ver)
{
    int version, len;
    char type[2];
    char ifname[10];
    char file[PROC_LEN];

    memset(type, 0, 2);
    memset(ifname, 0, 10);
    memset(file, 0, PROC_LEN);

    LogRunOperDebug("Set IGMP version(%d)\n", ver);

    if(ver == 0) {
        appSettingGetInt("igmpversion", &version, 0);
    } else {
        version = ver;
    }
    LogRunOperDebug("Set force_igmp_version(%d)\n", version);
    //added by teddy, 2011.03.26 Sat 12:29:33
    //这里都改成函数形式,不要使用system().
    if(version != 2 && version != 3) {
        LogRunOperError("igmp version err, is %d\n", version);
        return;
    }

    if(isprint(itrface)) {
        len = snprintf(file, PROC_LEN, "/proc/sys/net/ipv4/conf/%s/force_igmp_version", itrface);
    } else {
        if(sys_get_net_ifname(ifname, 10) < 0) {
            LogRunOperDebug("get net ifname error\n");
        } else {
            len = snprintf(file, PROC_LEN, "/proc/sys/net/ipv4/conf/%s/force_igmp_version", ifname);
        }
    }
    len = snprintf(type, 2, "%d", version);
    osex_igmp_version_set(file, type);

    return;
}
