#include "sys_basic_macro.h"

#include "app/Assertions.h"

#include "ind_net.h"
#include "ind_string.h"

#include "dns/mid_dns.h"
#include "mid_http.h"
#include "mid_msgq.h"
#include "mid_mutex.h"
#include "mid_task.h"
#include "mid_time.h"
#include "mid_timer.h"
#include "mid_tools.h"   

#include "ind_mem.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>


#define 	HTTP_TIMEOUT			(30*100)
#define		HEAD_MAX_LEN			(4*1024)
#define		HEAD_MAX_SIZE			(512*1024)

#ifndef STAILQ_HEAD

/*
 * Singly-linked Tail queue declarations.
 */
#define	STAILQ_HEAD(name, type)								\
struct name {												\
	struct type *stqh_first;/* first element */				\
	struct type **stqh_last;/* addr of last next element */	\
}

#define	STAILQ_ENTRY(type)									\
struct {													\
	struct type *stqe_next;	/* next element */				\
}

#define	STAILQ_NEXT(elm, field)	((elm)->field.stqe_next)

#define	STAILQ_FIRST(head)	((head)->stqh_first)

#define	STAILQ_FOREACH(var, head, field)					\
	for ((var) = STAILQ_FIRST((head));						\
	   (var);												\
	   (var) = STAILQ_NEXT((var), field))

#define	STAILQ_REMOVE(head, elm, type, field) do {			\
	if (STAILQ_FIRST((head)) == (elm)) {					\
		STAILQ_REMOVE_HEAD((head), field);					\
	}														\
	else {													\
		struct type *curelm = STAILQ_FIRST((head));			\
		while (STAILQ_NEXT(curelm, field) != (elm))			\
			curelm = STAILQ_NEXT(curelm, field);			\
		if ((STAILQ_NEXT(curelm, field) =					\
		     STAILQ_NEXT(STAILQ_NEXT(curelm, field), field)) == NULL)	\
			(head)->stqh_last = &STAILQ_NEXT((curelm), field);			\
	}																	\
} while (0)

#define	STAILQ_REMOVE_HEAD(head, field) do {				\
	if ((STAILQ_FIRST((head)) =								\
	     STAILQ_NEXT(STAILQ_FIRST((head)), field)) == NULL)	\
		(head)->stqh_last = &STAILQ_FIRST((head));			\
} while (0)

#endif //STAILQ_HEAD

typedef enum {
    SOCKET_OPEN = 0,
    SOCKET_CONNECTING,
    SOCKET_READ_HEADER,
    SOCKET_READ_DATA
} SOCKET_STATE;

struct SOCKET_ELEM {
    STAILQ_ENTRY(SOCKET_ELEM)	entry;
    struct ind_sin	sin;

    int             sock;
    uint32_t            clk;
    uint32_t            clock;
    SOCKET_STATE    state;
    char            head_buf[HEAD_MAX_LEN + 4];
    int             head_len;

    char            url[LARGE_URL_MAX_LEN];
    int             code;

    mid_http_f		func;
    mid_write_f 	write;
    int				arg;

    int				buf_ready;
    int				buf_size;

    char*			data_buf;
    int				data_len;
    int				data_length;
    int				data_offset;

    char			chunk_buf[8];
    int				chunk_len;
    int				chunk_size;
};

struct SResolve {
    struct SResolve *next;
    int magic;

    struct ind_sin	sin;
    char			url[LARGE_URL_MAX_LEN];
    mid_http_f		callback;
    int				arg;
    mid_write_f		writecall;
    char*			data_buf;
    int				buf_size;
    char			ex[LARGE_URL_MAX_LEN];
};

struct SOCKET_BREAK {
    STAILQ_ENTRY(SOCKET_BREAK)	entry;
    mid_http_f		func;
    int				arg;
};

static STAILQ_HEAD(, SOCKET_BREAK) g_slist_break = {NULL};
static STAILQ_HEAD(, SOCKET_ELEM) g_slist_queue = {NULL};
static STAILQ_HEAD(, SOCKET_ELEM) g_slist_header = {NULL};
static struct SResolve *g_rslv_queue = NULL;//域名解析队列
static int g_rslv_magic = 0;

#define	STAILQ_INSERT_TAIL_EX(head, elm, field) do {	\
	STAILQ_NEXT((elm), field) = NULL;					\
	if (STAILQ_FIRST(head) == NULL)						\
		STAILQ_FIRST((head)) = (elm);					\
	else												\
		*(head)->stqh_last = (elm);						\
	(head)->stqh_last = &STAILQ_NEXT((elm), field);		\
} while (0)

static mid_mutex_t	g_mutex = NULL;
static mid_msgq_t	g_msgq = NULL;
static int			g_msgfd = -1;

static mid_http_info_f g_call_httpinfo = NULL;
void mid_http_infoCall(mid_http_info_f httpinfo)
{
    g_call_httpinfo = httpinfo;
}

static mid_http_rrt_f g_call_httprrt = NULL;
static mid_http_fail_f g_call_httpfail = NULL;
void mid_http_statCall(mid_http_rrt_f httprrt, mid_http_fail_f httpfail)
{
    g_call_httprrt = httprrt;
    g_call_httpfail = httpfail;
}

static void socket_elem_free(struct SOCKET_ELEM* elem, int type)
{
    if(elem->func != NULL) {
        PRINTF("elem = %p %d\n", elem, type);
        if(type == HTTP_OK_READDATA || type == HTTP_OK_LOCATION)
            elem->func(type, elem->data_buf, elem->data_len, elem->arg);
        else
            elem->func(type, NULL, 0, elem->arg);
    } else {
        PRINTF("elem = %p func is NULL\n", elem);
    }
    if(elem->buf_ready == 0 && elem->data_buf)
        IND_FREE(elem->data_buf);

    DBG_PRN("elem = %p sock=%d close\n", elem, elem->sock);
    close(elem->sock);


    IND_FREE(elem);


}

static void socket_elem_free_header(struct SOCKET_ELEM* elem, int type)
{
    STAILQ_REMOVE(&g_slist_header, elem, SOCKET_ELEM, entry);
    if(elem->clk) {
        mid_http_rrt_f httprrt = g_call_httprrt;
        if(httprrt)
            httprrt(mid_ms() - elem->clk);
    }
    if(type != HTTP_OK_READDATA && type != HTTP_OK_LOCATION) {
        mid_http_fail_f httpfail = g_call_httpfail;
        if(httpfail)
            httpfail(elem->url, elem->code);
    }
    socket_elem_free(elem, type);
}

static void socket_elem_timer(int arg)
{
    char buf[4];
    mid_msgq_putmsg(g_msgq, buf);
}

//从g_queue中移动到g_header
static void socket_elem_move(void)
{
    while(1) {
        struct SOCKET_ELEM 	*e, *elem;
        struct linger		l;

        mid_mutex_lock(g_mutex);
        elem = STAILQ_FIRST(&g_slist_queue);
        if(elem == NULL) {
            mid_mutex_unlock(g_mutex);
            goto Brk;
        }

        STAILQ_FOREACH(e, &g_slist_header, entry) {
            if(e->func == elem->func) {
                mid_mutex_unlock(g_mutex);
                mid_timer_create(5, 1, socket_elem_timer, 0);
                goto Brk;
            }
        }
        STAILQ_REMOVE_HEAD(&g_slist_queue, entry);
        mid_mutex_unlock(g_mutex);

        elem->sock = socket(elem->sin.family, SOCK_STREAM, 0);
        if(elem->sock < 0) {
            ERR_PRN("elem = %p %d\n", elem, elem->sock);
            if(elem->func)
                elem->func(HTTP_ERROR_CONNECT, NULL, 0, elem->arg);

            IND_FREE(elem);

            elem = NULL;
            break;
        }
        DBG_PRN("elem = %p sock=%d open\n", elem, elem->sock);
        l.l_onoff = 1;
        l.l_linger = 0;
        setsockopt(elem->sock, SOL_SOCKET, SO_LINGER, (void *)(&l), sizeof(struct linger));
        STAILQ_INSERT_TAIL_EX(&g_slist_header, elem, entry);
        elem->clk = mid_ms();

        elem->state = SOCKET_OPEN;
        elem->clock = mid_10ms() + HTTP_TIMEOUT;
        DBG_PRN("elem = %p SOCKET_OPEN\n", elem);
    }

Brk:
    while(1) {
        struct SOCKET_BREAK *SocketBreak;
        struct SOCKET_ELEM 	*elem;

        mid_mutex_lock(g_mutex);
        SocketBreak = STAILQ_FIRST(&g_slist_break);
        if(SocketBreak == NULL) {
            mid_mutex_unlock(g_mutex);
            break;
        }
        STAILQ_REMOVE_HEAD(&g_slist_break, entry);

        PRINTF("break func = %p, arg = %d\n", SocketBreak->func, SocketBreak->arg);
Brk1:
        STAILQ_FOREACH(elem, &g_slist_queue, entry) {
            if(elem->func == SocketBreak->func && elem->arg == SocketBreak->arg) {
                STAILQ_REMOVE(&g_slist_queue, elem, SOCKET_ELEM, entry);
                mid_mutex_unlock(g_mutex);

                socket_elem_free(elem, HTTP_OK_BREAK);

                mid_mutex_lock(g_mutex);
                goto Brk1;
            }
        }

Brk2:
        STAILQ_FOREACH(elem, &g_slist_header, entry) {
            if(elem->func == SocketBreak->func && elem->arg == SocketBreak->arg) {
                STAILQ_REMOVE(&g_slist_header, elem, SOCKET_ELEM, entry);
                mid_mutex_unlock(g_mutex);

                socket_elem_free(elem, HTTP_OK_BREAK);

                mid_mutex_lock(g_mutex);
                goto Brk2;
            }
        }


        IND_FREE(SocketBreak);

        mid_mutex_unlock(g_mutex);
    }
}

static int int_http_regist(ind_sin_t sin, const char* url, mid_http_f callback, int arg, mid_write_f writecall, char *data_buf, int buf_size, char *ex)
{
    char 	*p, *uri;
    char	addr[IND_ADDR_LEN];
    int		len;

    struct SOCKET_ELEM *elem = NULL;


    elem = (struct SOCKET_ELEM *)IND_MALLOC(sizeof(struct SOCKET_ELEM));


    if(elem == NULL)
        ERR_OUT("malloc!\n");
    memset(elem, 0, sizeof(struct SOCKET_ELEM));

    memcpy(&elem->sin, sin, sizeof(struct ind_sin));

    DBG_PRN("elem = %p url = %s\n", elem, url);

    uri = (char *)strchr(url + 7, '/');  //WZW modified to fix pc-lint Error 158
    if(uri == NULL)
        ERR_OUT("uri not found! %s\n", url);

    ind_net_ntop(sin, addr, IND_ADDR_LEN);
    p = elem->head_buf;

    len = snprintf(p, HEAD_MAX_LEN, "GET %s HTTP/1.1\r\nHost: %s:%d\r\n", uri, addr, sin->port);
    if(len <= 0)
        ERR_OUT("snprintf!\n");

    if(data_buf == NULL && buf_size > 0) {
        elem->data_offset = buf_size;
        if(elem->data_offset > 0)
            len += snprintf(p + len, HEAD_MAX_LEN - len, "Range: bytes=%d-\r\n", elem->data_offset);
    } else {
        elem->data_offset = 0;
    }

    if(ex) {
        len += snprintf(p + len, HEAD_MAX_LEN - len, "%s\r\n", ex);
        if(len <= 0)
            ERR_OUT("snprintf ex!\n");
    }
    len += snprintf(p + len, HEAD_MAX_LEN - len, "\r\n");
    if(len <= 0)
        ERR_OUT("snprintf end!\n");

    {
        mid_http_info_f httpinfo = g_call_httpinfo;
        if(httpinfo)
            httpinfo(p, len);
    }
    elem->func = callback;
    elem->arg = arg;
    elem->write = writecall;
    if(data_buf) {
        elem->buf_ready = 1;
        elem->data_buf = data_buf;
        elem->buf_size = buf_size;
    }

    mid_mutex_lock(g_mutex);
    STAILQ_INSERT_TAIL_EX(&g_slist_queue, elem, entry);
    mid_mutex_unlock(g_mutex);

    mid_msgq_putmsg(g_msgq, addr);

    return 0;
Err:
    if(callback)
        callback(HTTP_ERROR_CONNECT, NULL, 0, arg);

    if(elem)
        IND_FREE(elem);


    return -1;
}

static void int_http_dnsback(int arg, int dnsmsg, unsigned int hostip)
{
    char *ex;
    struct SResolve *prev, *rslv = NULL;

    mid_mutex_lock(g_mutex);
    prev = NULL;
    rslv = g_rslv_queue;

    while(rslv && rslv->magic != arg) {
        prev = rslv;
        rslv = rslv->next;
    }
    if(rslv) {
        if(prev)
            prev->next = rslv->next;
        else
            g_rslv_queue = rslv->next;
    }
    mid_mutex_unlock(g_mutex);

    if(rslv == NULL)
        ERR_OUT("rslv not found!\n");

    if(dnsmsg != DNS_MSG_OK) {
        rslv->callback(HTTP_ERROR_CONNECT, NULL, 0, rslv->arg);
        ERR_OUT("dnsmsg = %d\n", dnsmsg);
    }

    rslv->sin.family = AF_INET;
    rslv->sin.in_addr.s_addr = hostip;

    if(rslv->ex[0] == 0)
        ex = NULL;
    else
        ex = rslv->ex;
    if(int_http_regist(&rslv->sin, rslv->url, rslv->callback, rslv->arg, rslv->writecall, rslv->data_buf, rslv->buf_size, ex))
        ERR_OUT("int_http_regist %s\n", rslv->url);

Err:
    if(rslv)
        IND_FREE(rslv);

    return;
}

static int mid_http_regist(const char* url, mid_http_f callback, int arg, mid_write_f writecall, char *data_buf, int buf_size, char *ex)
{
    char			host[IND_HOST_LEN];
    int				port;
    struct ind_sin	sin;

    if(callback == NULL)
        ERR_OUT("0!\n");
    if(url == NULL || strlen(url) >= LARGE_URL_MAX_LEN)
        ERR_OUT("url tool large!\n");
    if(ex && strlen(ex) >= LARGE_URL_MAX_LEN)
        ERR_OUT("ex tool large!\n");

    if(mid_tool_checkURL(url, host, &port))
        ERR_OUT("mid_tool_checkURL\n");

    if(ind_net_pton(host, &sin) <= 0) {

        struct SResolve* rslv = (struct SResolve*)IND_MALLOC(sizeof(struct SResolve));
        if(rslv == NULL)
            ERR_OUT("malloc SResolve\n");
        memset(rslv, 0, sizeof(struct SResolve));
        rslv->sin.port = (uint16_t)port;
        strncpy(rslv->url, url, LARGE_URL_MAX_LEN);
        rslv->callback = callback;
        rslv->writecall = writecall;
        rslv->arg = arg;
        rslv->data_buf = data_buf;
        rslv->buf_size = buf_size;
        if(ex)
            strncpy(rslv->ex, ex, LARGE_URL_MAX_LEN);
        else
            rslv->ex[0] = 0;

        mid_mutex_lock(g_mutex);
        rslv->next = g_rslv_queue;
        g_rslv_queue = rslv;
        g_rslv_magic ++;
        rslv->magic = g_rslv_magic;
        mid_mutex_unlock(g_mutex);

        PRINTF("dns resolve %s\n", url);
        mid_dns_resolve(host, int_http_dnsback, rslv->magic, 28);
    } else {
        sin.port = (uint16_t)port;
        if(int_http_regist(&sin, url, callback, arg, writecall, data_buf, buf_size, ex))
            ERR_OUT("int_http_regist %s\n", url);
    }

    return 0;
Err:
    return -1;
}

int mid_http_call(const char* url, mid_http_f callback, int arg, char *data_buf, int buf_size, char *ex)
{
    return mid_http_regist(url, callback, arg, NULL, data_buf, buf_size, ex);
}

int mid_http_simplecall(char* url, mid_http_f callback, int arg)
{
    return mid_http_regist(url, callback, arg, NULL, NULL, 0, NULL);
}

int mid_http_writecall(char* url, mid_http_f callback, int arg, mid_write_f writecall, char *ex)
{
    return mid_http_regist(url, callback, arg, writecall, NULL, 0, ex);
}

int mid_http_offsetcall(const char* url, mid_http_f callback, int arg, mid_write_f writecall, int offset, char *ex)
{
    return mid_http_regist(url, callback, arg, writecall, NULL, offset, ex);
}

int mid_http_break(mid_http_f callback, int arg)
{
    char buf[1];
    struct SOCKET_BREAK *SocketBreak = NULL;


    SocketBreak = (struct SOCKET_BREAK *)IND_MALLOC(sizeof(struct SOCKET_BREAK));


    if(SocketBreak == NULL)
        ERR_OUT("malloc!\n");
    memset(SocketBreak, 0, sizeof(struct SOCKET_BREAK));

    SocketBreak->func = callback;
    SocketBreak->arg = arg;

    mid_mutex_lock(g_mutex);
    STAILQ_INSERT_TAIL_EX(&g_slist_break, SocketBreak, entry);
    mid_mutex_unlock(g_mutex);

    mid_msgq_putmsg(g_msgq, buf);
    return 0;

Err:
    return -1;
}

static void connect_end(struct SOCKET_ELEM* elem)
{
    int ret, len;

    len = strlen((char *)(elem->head_buf));
    DBG_PRN("elem = %p SEND:\n%s\n", elem, elem->head_buf);
    ret = send(elem->sock, elem->head_buf, len, MSG_NOSIGNAL);

    if(ret != len) {
        ERR_PRN("elem = %p HTTP_ERROR_SENDDATA len = %d, ret = %d, errno = %d / %s!\n", elem, len, ret, errno, strerror(errno));
        socket_elem_free_header(elem, HTTP_ERROR_SENDDATA);
        return;
    }

    elem->head_len = 0;
    elem->state = SOCKET_READ_HEADER;
    elem->clock = mid_10ms() + HTTP_TIMEOUT;
    DBG_PRN("elem = %p SOCKET_READ_HEADER\n", elem);
}

static void connect_begin(struct SOCKET_ELEM* elem)
{
    int sock;

    sock = elem->sock;

    if(fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK) == -1)
       DBG_PRN("fcntl err\n");

    ind_net_connect(sock, &elem->sin);

    elem->state = SOCKET_CONNECTING;
    elem->clock = mid_10ms() + HTTP_TIMEOUT;
    DBG_PRN("elem = %p SOCKET_CONNECTING\n", elem);
}

static void connect_check(struct SOCKET_ELEM* elem)
{
    int error;
    socklen_t len;
    int sock = elem->sock;

    {
        int ret;
        fd_set rset;
        struct timeval tv;

        FD_ZERO(&rset);
        FD_SET(sock, &rset);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(sock + 1, &rset, NULL, NULL, &tv);
        if(ret > 0 && FD_ISSET(sock, &rset)) {
            ERR_PRN("elem = %p select HTTP_ERROR_NET, errno = %d / %s!\n", elem, errno, strerror(errno));
            socket_elem_free_header(elem, HTTP_ERROR_NET);
            return;
        }
    }

    error = -1;
    len = sizeof(error);
    getsockopt(sock, SOL_SOCKET, SO_ERROR, (caddr_t)&error, &len);
    if(error == 0) {
        DBG_PRN("elem = %p connect SUCCEED\n", elem);
        connect_end(elem);
    } else {
        ERR_PRN("elem = %p error = %d, errno = %d HTTP_ERROR_NET\n", elem, error, errno);
        socket_elem_free_header(elem, HTTP_ERROR_NET);
    }
}

static void http_read_head_chunk(struct SOCKET_ELEM* elem, char *buf, int len)
{
    int size;

    elem->state = SOCKET_READ_DATA;
    elem->clock = mid_10ms() + HTTP_TIMEOUT;
    //DBG_PRN("elem = %p SOCKET_READ_DATA\n", elem);

Chunk:
    if(elem->chunk_size == 0) {
        while(len > 0 && buf[0] != '\n') {
            if(elem->chunk_len >= 8) {
                ERR_PRN("elem = %p chunk_len = %d\n", elem, elem->chunk_len);
                socket_elem_free_header(elem, HTTP_ERROR_DATA);
                return;
            }
            elem->chunk_buf[elem->chunk_len] = buf[0];
            elem->chunk_len ++;
            buf ++;
            len --;
        }
        if(len <= 0)
            return;
        buf ++;
        len --;

        elem->chunk_len --;
        if(elem->chunk_len == 0)
            goto Chunk;
        elem->chunk_buf[elem->chunk_len] = 0;

        //DBG_PRN("elem = %p chunk_len = %d, chunk_buf = %s\n", elem, elem->chunk_len, elem->chunk_buf);
        if(ind_hextoi(elem->chunk_buf, elem->chunk_len, (uint32_t*)&elem->chunk_size)) {
            ERR_PRN("elem = %p chunk_buf = %s\n", elem, elem->chunk_buf);
            socket_elem_free_header(elem, HTTP_ERROR_DATA);
            return;
        }
        //DBG_PRN("elem = %p chunk_size = 0x%x\n", elem, elem->chunk_size);
        elem->chunk_len = 0;

        if(elem->chunk_size == 0) {
            if(elem->data_length == 0) {
                DBG_PRN("elem = %p HTTP_OK_EMPTY\n", elem);
                socket_elem_free_header(elem, HTTP_OK_EMPTY);
                return;
            }
            DBG_PRN("elem = %p HTTP_OK_READDATA\n", elem);
            socket_elem_free_header(elem, HTTP_OK_READDATA);
            return;
        }
        elem->data_length += elem->chunk_size;
    }
    if(len <= 0)
        return;

    if(elem->write == NULL && elem->buf_size < elem->data_length) {
        if(elem->buf_ready == 1) {
            DBG_PRN("elem = %p HTTP_ERROR_DATA buf_size = %d, data_length = %d\n", elem, elem->buf_size, elem->data_length);
            socket_elem_free_header(elem, HTTP_ERROR_DATA);
            return;
        }

        if(elem->data_buf == 0) {
            elem->data_buf = (char*)IND_MALLOC(elem->chunk_size);
            if(elem->data_buf == NULL) {
                DBG_PRN("elem = %p HTTP_ERROR_MALLOC\n", elem);
                socket_elem_free_header(elem, HTTP_ERROR_MALLOC);
                return;
            }
            elem->buf_size = elem->chunk_size;
        } else {
            int buf_size;
            char *data_buf;

            if(elem->buf_size >= HEAD_MAX_SIZE) {
                DBG_PRN("elem = %p HTTP_ERROR_DATA\n", elem);
                socket_elem_free_header(elem, HTTP_ERROR_DATA);
                return;
            }
            buf_size = elem->data_len + elem->chunk_size * 3;
            if(buf_size > HEAD_MAX_SIZE)
                buf_size = HEAD_MAX_SIZE;
            if(buf_size < elem->data_length) {
                DBG_PRN("elem = %p HTTP_ERROR_DATA\n", elem);
                socket_elem_free_header(elem, HTTP_ERROR_DATA);
                return;
            }

            data_buf = (char*)IND_MALLOC(buf_size);
            if(data_buf == NULL) {
                DBG_PRN("elem = %p HTTP_ERROR_MALLOC\n", elem);
                socket_elem_free_header(elem, HTTP_ERROR_MALLOC);
                return;
            }
            memcpy(data_buf, elem->data_buf, elem->data_len);
            IND_FREE(elem->data_buf);
            elem->data_buf = data_buf;
            elem->buf_size = buf_size;
        }
    }

    size = elem->data_length - elem->data_len;
    if(size > len)
        size = len;
    if(elem->write) {
        if(elem->write(elem->arg, 0, elem->data_len, buf, size)) {
            ERR_PRN("elem = %p HTTP_ERROR_WRITEDATA\n", elem);
            socket_elem_free_header(elem, HTTP_ERROR_WRITEDATA);
            return;
        }
    } else {
        memcpy(elem->data_buf + elem->data_len, buf, size);
    }

    buf += size;
    len -= size;
    elem->data_len += size;
    if(elem->data_len >= elem->data_length) {
        elem->chunk_size = 0;
        goto Chunk;
    }
    if(len > 0) {
        ERR_PRN("elem = %p len = %d\n", elem, len);
        socket_elem_free_header(elem, HTTP_ERROR_DATA);
        return;
    }
}

static void http_read_head(struct SOCKET_ELEM* elem)
{
    int		len, length;
    char	*p, *end;

    len = recv(elem->sock, elem->head_buf + elem->head_len, HEAD_MAX_LEN - elem->head_len, 0);
    if(len <= 0) {
        ERR_PRN("elem = %p HTTP_ERROR_READDATA, errno = %d / %s!\n", elem, errno, strerror(errno));
        socket_elem_free_header(elem, HTTP_ERROR_READDATA);
        return;
    }

    //找http头部结束标记
    elem->head_len += len;
    elem->head_buf[elem->head_len] = 0;
    end = strstr(elem->head_buf, "\r\n\r\n");
    if(end == NULL || elem->head_len < 15) {
        if(elem->head_len >= HEAD_MAX_LEN) {
            ERR_PRN("elem = %p head_len = %d HTTP_ERROR_READDATA\n", elem, elem->head_len);
            socket_elem_free_header(elem, HTTP_ERROR_READDATA);
        }
        return;
    }
    end[0] = 0;
    DBG_PRN("elem = %p RECV:\n%s\n\n", elem, elem->head_buf);
    end += 4;

    if(memcmp(elem->head_buf, "HTTP/1.", 7)) {
        ERR_PRN("elem = %p invalid http response! %s\n", elem, elem->head_buf);
        socket_elem_free_header(elem, HTTP_ERROR_DATA);
        return;
    }
    elem->code = atoi(elem->head_buf + 9);

    {
        mid_http_rrt_f httprrt = g_call_httprrt;
        if(httprrt)
            httprrt(mid_ms() - elem->clk);
        elem->clk = 0;
    }

    //重定向
    if(elem->head_buf[9] == '3') {
        p = strstr(elem->head_buf, "Location: ");
        mid_tool_line_print(elem->head_buf);
        if(p == NULL) {
            ERR_PRN("elem = %p HTTP_ERROR_DATA\n", elem);
            socket_elem_free_header(elem, HTTP_ERROR_DATA);
            return;
        }
        p += 10;
        length = mid_tool_line_len(p);

        if(elem->buf_ready == 0) {
            elem->buf_size = length + 4;
            elem->data_buf = (char*)IND_MALLOC(length + 4);
        }
        if(elem->data_buf == NULL || length >= elem->buf_size) {
            ERR_PRN("elem = %p HTTP_ERROR_MALLOC length = %d, %d\n", elem, length, elem->buf_size);
            socket_elem_free_header(elem, HTTP_ERROR_MALLOC);
            return;
        }
        mid_tool_line_first(p, elem->data_buf);
        elem->data_buf[length] = 0;
        elem->data_len = length;
        DBG_PRN("elem = %p HTTP_OK_LOCATION\n", elem);
        socket_elem_free_header(elem, HTTP_OK_LOCATION);
        return;
    }

    //非200
    if(memcmp(elem->head_buf + 9, "20", 2)) {
        ERR_PRN("elem = %p invalid http code!\n%s\n", elem, elem->head_buf);
        socket_elem_free_header(elem, HTTP_ERROR_NOT200);
        return;
    }
    len = elem->head_len - (end - elem->head_buf);

    p = strstr(elem->head_buf, "Content-Length: ");
    if(p == NULL) {
        p = strstr(elem->head_buf, "Transfer-Encoding: chunked");
        if(p) {
            elem->data_len = 0;
            elem->data_length = 0;
            elem->data_offset = 0;
            elem->chunk_len = 0;
            elem->chunk_size = 0;
            http_read_head_chunk(elem, end, len);
            return;
        }
        ERR_PRN("elem = %p Content-Length not fond!\n", elem);
        socket_elem_free_header(elem, HTTP_ERROR_DATA);
        return;
    }
    if(sscanf(p + 16, "%d", &length) != 1 || length < 0) {
        ERR_PRN("elem = %p length = %d\n", elem, length);
        socket_elem_free_header(elem, HTTP_ERROR_DATA);
        return;
    }

    elem->chunk_size = -1;
    elem->data_length = length;

    if(len > length) {
        ERR_PRN("elem = %p length=%d, len = %d\n", elem, length, len);
        socket_elem_free_header(elem, HTTP_ERROR_DATA);
        return;
    }

    if(elem->data_offset > 0)
        DBG_PRN("data_offset = %d\n", elem->data_offset);
    else
        elem->data_offset = 0;
    elem->data_len = elem->data_offset;
    elem->data_length += elem->data_offset;

    if(length == 0) {
        DBG_PRN("elem = %p HTTP_OK_EMPTY\n", elem);
        socket_elem_free_header(elem, HTTP_OK_EMPTY);
        return;
    }
    if(elem->write == NULL) {
        if((elem->buf_ready == 0 && length > HEAD_MAX_SIZE) ||
           (elem->buf_ready == 1 && length >= elem->buf_size)) {
            ERR_PRN("elem = %p data_buf = %p, buf_size = %d, length = %d\n", elem, elem->data_buf, elem->buf_size, length);
            socket_elem_free_header(elem, HTTP_ERROR_DATA);
            return;
        }
        if(elem->buf_ready == 0) {
            elem->buf_size = length + 4;
            elem->data_buf = (char*)IND_MALLOC(length + 4);
        }
        if(elem->data_buf == NULL) {
            ERR_PRN("elem = %p HTTP_ERROR_MALLOC\n", elem);
            socket_elem_free_header(elem, HTTP_ERROR_MALLOC);
            return;
        }
        memset(elem->data_buf, 0, elem->buf_size);
    }

    DBG_PRN("elem = %p length=%d, len = %d\n", elem, length, len);
    if(len > 0) {
        if(elem->write) {
            if(elem->write(elem->arg, elem->data_length, elem->data_len, end, len)) {
                ERR_PRN("elem = %p HTTP_ERROR_WRITEDATA\n", elem);
                socket_elem_free_header(elem, HTTP_ERROR_WRITEDATA);
                return;
            }
        } else {
            memcpy(elem->data_buf, end, len);
        }
        elem->data_len += len;
        DBG_PRN("elem = %p data_length=%d, data_len = %d\n", elem, elem->data_length, elem->data_len);
        if(elem->data_len >= elem->data_length) {
            DBG_PRN("elem = %p HTTP_OK_READDATA\n", elem);
            socket_elem_free_header(elem, HTTP_OK_READDATA);
            return;
        }
    }
    elem->state = SOCKET_READ_DATA;
    elem->clock = mid_10ms() + HTTP_TIMEOUT;
    DBG_PRN("elem = %p SOCKET_READ_DATA\n", elem);
}

static void http_read_data(struct SOCKET_ELEM* elem)
{
    int len;

    if(elem->write || elem->chunk_size >= 0) {
        len = recv(elem->sock, elem->head_buf, HEAD_MAX_LEN, 0);
        if(elem->chunk_size >= 0) {
            http_read_head_chunk(elem, elem->head_buf, len);
            return;
        }
    } else {
        len = recv(elem->sock, elem->data_buf + elem->data_len, elem->data_length - elem->data_len, 0);
    }

    if(len <= 0) {
        ERR_PRN("elem = %p HTTP_ERROR_READDATA, errno = %d / %s!\n", elem, errno, strerror(errno));
        socket_elem_free_header(elem, HTTP_ERROR_READDATA);
        return;
    }

    if(elem->write) {
        if(elem->write(elem->arg, elem->data_length, elem->data_len, elem->head_buf, len)) {
            ERR_PRN("elem = %p HTTP_ERROR_WRITEDATA\n", elem);
            socket_elem_free_header(elem, HTTP_ERROR_WRITEDATA);
            return;
        }
    }

    elem->data_len += len;
    if(elem->data_len >= elem->data_length) {
        DBG_PRN("elem = %p HTTP_OK_READDATA\n", elem);
        socket_elem_free_header(elem, HTTP_OK_READDATA);
        return;
    }
    elem->clock = mid_10ms() + HTTP_TIMEOUT;
}


static void mid_http_task(void *arg)
{
    struct SOCKET_ELEM	*elem, *next;
    struct timeval		tv;
    fd_set				rfds, wfds;
    unsigned int		clk;
    int					maxfd;

    while(1) {
Loop:
        clk = mid_10ms();

        FD_ZERO(&rfds); /*yanglw: 读*/
        FD_SET(g_msgfd, &rfds);
        FD_ZERO(&wfds); /*yanglw: 写*/

        maxfd = g_msgfd;
        STAILQ_FOREACH(elem, &g_slist_header, entry) {
            if(elem->clock < clk) {
                ERR_PRN("elem = %p HTTP_ERROR_TIMEOUT\n", elem);
                socket_elem_free_header(elem, HTTP_ERROR_TIMEOUT);
                goto Loop;
            }
            switch(elem->state) {
            case SOCKET_OPEN:
                connect_begin(elem);
                goto Loop;

            case SOCKET_CONNECTING:
                FD_SET(elem->sock, &wfds);
                if(elem->sock > maxfd)
                    maxfd = elem->sock;
                break;

            case SOCKET_READ_HEADER:
            case SOCKET_READ_DATA:
                FD_SET(elem->sock, &rfds);
                if(elem->sock > maxfd)
                    maxfd = elem->sock;
                break;

            default:
                break;
            }
        }
        if(STAILQ_FIRST(&g_slist_header) == NULL)
            tv.tv_sec = 3600;
        else
            tv.tv_sec = 1;
        tv.tv_usec = 0;
        if(select(maxfd + 1, &rfds , &wfds, NULL, &tv) <= 0)
            continue;

        if(FD_ISSET(g_msgfd, &rfds)) {
            char buf[4];
            mid_msgq_getmsg(g_msgq, buf);
            socket_elem_move();  /*!yanglw: 这句是什么作用?*/
            goto Loop;
        }

        next = STAILQ_FIRST(&g_slist_header);

        while(next) {
            elem = next;
            next = STAILQ_NEXT(elem, entry);

            if(elem->state == SOCKET_CONNECTING) {
                if(FD_ISSET(elem->sock, &wfds))
                    connect_check(elem);
                    //goto Loop;
                    continue;
            }

            if(FD_ISSET(elem->sock, &rfds)) {
                switch(elem->state) {
                case SOCKET_READ_HEADER:
                    http_read_head(elem); /*yanglw: 读取网页头信息到elem*/
                    break;
                case SOCKET_READ_DATA:
                    http_read_data(elem);
                    break;
                default:
                    ERR_PRN("elem = %p state = %d HTTP_ERROR_NET\n", elem, elem->state);
                    socket_elem_free_header(elem, HTTP_ERROR_NET);
                    break;
                }
                continue;
            }
        }
    }
}

int httpDefaultFunc(int type, char* buf, int len, int arg)
{
	return 0;
}

int mid_http_init(void)
{
    if(g_mutex)
        ERR_OUT("http already init\n"); /*yanglw: g_mutex是static类型的，注意它的使用方法*/
    g_mutex = mid_mutex_create();
    g_msgq = mid_msgq_create(20, 1); /*yanglw: 创建消息队列，这里是管道，消息数量为1*/
    g_msgfd = mid_msgq_fd(g_msgq); /*获取管道读端的fd*/

    mid_task_create("mid_http", (mid_func_t)mid_http_task, 0);  /*yanglw: 创建一个线程去处理传进的函数*/

    return 0;
Err:
    return -1;
}
