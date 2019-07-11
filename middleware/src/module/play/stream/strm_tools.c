
#include "stream.h"

int strm_tool_parse_uri(char *uri)
{
    char *p;

    p = uri;
    while ('?' != *p && '\0' != *p)
        p ++;
    while ('/' != *p && p > uri)
        p --;

    if ('/' != *p)
        return -1;

    return (int)(p - uri) + 1;
}

//RTSP_DEFAULT_PORT
static void strm_tool_parse_rrs(StrmRRS_t strmRRS, char *str, int len)
{
    int i, l;
    struct ind_sin sin;

    while(strmRRS->rrs_num < RRS_NUM_4) {
        if (strncmp(str, "rrsip=", 6))
            LOG_STRM_ERROUT("str = %s\n", str);
        str += 6;

        memset(&sin, 0, sizeof(sin));
        l = ind_net_pton(str, &sin);
        if (l <= 0)
            LOG_STRM_ERROUT("parse addr ret = %d, url = %s\n", l, str);
        for (i = 0; i < strmRRS->rrs_num; i ++) {
            if (ind_net_equal(&strmRRS->rrs_sins[i], &sin) > 1)
                break;
        }
        if (i >= strmRRS->rrs_num) {
            strmRRS->rrs_sins[strmRRS->rrs_num] = sin;
            strmRRS->rrs_num ++;
        }
        str += l;
        if (str[0] != ',')
            break;
        str ++;
    }

    LOG_STRM_PRINTF("rrs_num = %d\n", strmRRS->rrs_num);

Err:
    return;
}

int strm_tool_parse_url(char* url, StrmRRS_t strmRRS)
{
    char *p;
    int l, len;
    struct ind_sin sin;

    memset(&sin, 0, sizeof(sin));

    IND_STRCPY(strmRRS->rrs_name, url);
    url = strmRRS->rrs_name;
    p = strchr(url, '/');
    if (!p)
        LOG_STRM_ERROUT("parse addr url = %s\n", url);

    *p = 0;
    strmRRS->rrs_uri = p + 1;

    l = ind_net_pton(url, &sin);
    strmRRS->rrs_sins[0] = sin;
    strmRRS->rrs_num = 1;
    strmRRS->rrs_idx = 0;

    if (l <= 0) {//ÓòÃû
        p = strmRRS->rrs_name;
        len = strlen(p);
        l = len - 1;
        while (l > 0 && p[l] >= '0' && p[l] <= '9')
            l--;
        if (l > 0 && l < len - 1 && ':' == p[l]) {
            p[l] = '\0';
            strmRRS->rrs_sins[0].port = (uint16_t)atoi(p + l + 1);
        }
        return 1;
    }

    url = strmRRS->rrs_uri;

    p = strchr(url, '?');
    if (!p)
        return 0;

    len = strlen(p);
    do {
        len--;
        url = p + 1;
        p = strchr(url, '&');
        if (p)
            l = p - url;
        else
            l = len;
        len -= l;

        if (strncmp(url, "rrsip=", 6) == 0) {
            strm_tool_parse_rrs(strmRRS, url, l);
            if (!p) {
                url--;
                url[0] = 0;
                return 0;
            }
            memmove(url, p + 1, len);
            if (0 != url[len - 1])
                LOG_STRM_ERROUT("\n");
        }
    } while(p);

    return 0;
Err:
    return -1;
}

typedef struct __StrmDns StrmDns;
typedef struct __StrmDns* StrmDns_t;
struct __StrmDns {
    StrmDns_t next;
    uint32_t clk;
    int magic;
    char name[1];
};
static StrmDns_t g_dns = NULL;
static int g_magic = 0;

static void int_dns_back(int arg, int dnsmsg, unsigned int hostip)
{
    StrmDns_t dns, prev;
    mid_mutex_t mutex;

    mutex = int_stream_mutex( );
    mid_mutex_lock(mutex);

    dns = g_dns;
    prev = NULL;
    while (dns && arg != dns->magic) {
        prev = dns;
        dns = dns->next;
    }

    if (dns) {
        if (DNS_MSG_OK == dnsmsg)
            LOG_STRM_PRINTF("name = %s, hostip = 0x%08x!\n", dns->name, hostip);
        else
            LOG_STRM_WARN("name = %s, dnsmsg = %d!\n", dns->name, dnsmsg);
        if (prev)
            prev->next = dns->next;
        else
            g_dns = dns->next;
        IND_FREE(dns);
    }

    mid_mutex_unlock(mutex);
}

void strm_tool_dns_resolve(char* name)
{
    int magic = 0;
    uint32_t clk;
    StrmDns_t prev, dns;
    mid_mutex_t mutex;

    clk = mid_10ms( );
    mutex = int_stream_mutex( );
    mid_mutex_lock(mutex);

    dns = g_dns;
    prev = NULL;
    while (dns && strcmp(dns->name, name)) {
        prev = dns;
        dns = dns->next;
    }

    if (dns && dns->clk < clk) {
        if (prev)
            prev->next = dns->next;
        else
            g_dns = dns->next;
        IND_FREE(dns);
        dns = NULL;
    }

    if (!dns) {
        int len;

        len = strlen(name);
        dns = (StrmDns_t)IND_MALLOC(sizeof(StrmDns) + len);
        if (!dns)
            LOG_STRM_ERROUT("malloc StrmDns! name = %s\n", name);

        dns->clk = clk + 2900;
        if (0 == g_magic)
            g_magic ++;
        magic = g_magic;
        g_magic ++;

        dns->magic = magic;
        IND_MEMCPY(dns->name, name, len);
        dns->name[len] = 0;

        dns->next = g_dns;
        g_dns = dns;
    }

Err:
    mid_mutex_unlock(mutex);

    if (magic)
        mid_dns_resolve(name, int_dns_back, magic, 28);
}

uint32_t strm_tool_dns_find(char* name)
{
    uint32_t ip;
    StrmDns_t dns;
    mid_mutex_t mutex;

    ip = INADDR_NONE;
    mutex = int_stream_mutex( );
    mid_mutex_lock(mutex);

    dns = g_dns;
    while (dns && strcmp(dns->name, name))
        dns = dns->next;
    if (dns)
        ip = INADDR_ANY;
    else
        ip = mid_dns_cache(name);

    mid_mutex_unlock(mutex);
    return ip;
}
