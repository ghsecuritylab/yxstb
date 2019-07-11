/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "http.h"
#include "stream_port.h"

#define STAT_ARRAY_SIZE        3
#define CHUNK_HEAD_SIZE        16


/*
    1）socket函数connect( )连接服务器失败，机顶盒按照2、4、8、16秒间隔重试，然后
等待30秒。然后进入第二轮发送，发送周期同第一轮（即默认每轮60秒），如果还是失败，
进入第三轮相同周期的发送（2、4、8、16秒间隔），三轮时间共计2分钟30秒。如果还是失
败，关闭连接，缓存数据播完后画面停在最后一帧，并上报错误事件；
*/
#define RETRY_INTERVAL_MAX      1600
#define RETRY_INTERVAL_INIT     100
#define RETRY_INTERVAL_EXTEND   3000
#define RETRY_TIMES             5 //2分30秒
/*
    2）http get请求发出后，开始接收或接收过程中发生超时，缓存数据播完后画面停在最
后一帧，然后开始计时，如果15秒内没收到数据。机顶盒关闭连接，重新建链connect( )；
*/
#define RECV_TIMEOUT            1500

#define RECV_ELEM_SIZE          1316

typedef enum {
    HTTP_STATE_CLOSE = 0,
    HTTP_STATE_DNS,
    HTTP_STATE_OPEN,
    HTTP_STATE_RECV_HEAD,
    HTTP_STATE_RECV_BODY,
    HTTP_STATE_RECV_EXT,
    HTTP_STATE_END
} HTTP_STATE;

//流统计
typedef struct
{
    uint32_t clk_begin;
    uint32_t clk_end;

    int bytes;
} ByteStat;

struct HTTP {
    HTTP_t          prev;
    HTTP_t          next;
    HTTPLoop_t      loop;

    int             index;
    HTTP_STATE      state;
    int             sock;
    int             code;
    char            cookie[HTTP_COOKIE_SIZE];
    char            url[STREAM_URL_SIZE];

    uint32_t        retryClk;
    int             retryTimes;
    uint32_t        retryInterval;

    int             timeout;
    int             downloadRate;

    uint32_t        recvClk;
    uint32_t        recvTimes;
    int             ignoreTimes;

    int             chunk_len;
    char            chunk_head_buf[CHUNK_HEAD_SIZE];
    int             chunk_head_len;

    int             end_flag;

    char            head_buf[STREAM_HEAD_LEN + 4];
    int             head_len;

    struct StrmRRS  strmRRS;

    char            content_type[CONTENT_TYPE_SIZE];
    long long       content_len;
    long long       content_length;

    long long       content_bytes;

    int             connection_close;

    long long       byte_start;
    long long       byte_count;

    int             sb_size;
    StrmBuffer*     sb;
    StrmBufQue*     sbq;

    ts_buf_t        ts_buf;
    ts_buf_t        ts_buffer;

    //recv_buf和recv_len是为了控制给SQM传送数据是188整数倍
    char*           recv_buf;
    int             recv_len;
    int             recv_size;

    int             sqm_flg;

    int             gzip;
    int             gzip_flag;

    char            etag[64];
    int             etag_flag;

    void*           handle;
    HttpOp          op;

    ByteStat        stat_array[STAT_ARRAY_SIZE];

    uint32_t        start_clk;
    int             limit_len;
    int             limit_byterate;
};

struct HTTPLoop {
    int             index;
    int             exit;

    uint32_t        clk;
    HTTP_t          http;

    ind_tlink_t     tlink;
    mid_msgq_t      msgq;
    int             msgfd;

    StreamMsgQ*     strm_msgq;

    void*           handle;
    HttpLoopOp      op;
};

StrmHttpUnzip g_httpUnzip = NULL;

static int int_http_connect(HTTP_t http, long long byte_start, long long byte_count);
static void int_http_error(HTTP_t http, int msgno);

static int int_http_byte_rate(HTTP_t http);
static void int_http_byte_reset(HTTP_t http);
static void int_http_byte_stat(HTTP_t http, int bytes);
static void int_http_data_timeout(void* arg);
static void int_http_open_timeout(void* arg);

static int int_http_resovle(HTTP_t http);

static void int_httploop_100ms(void *arg)
{
    HTTP_t http;
    HTTPLoop_t loop = (HTTPLoop_t)arg;

    http = loop->http;
    while (http) {
        if (HTTP_STATE_DNS == http->state) {
            StrmRRS_t strmRRS = &http->strmRRS;

            uint32_t hostip = mid_dns_cache(strmRRS->rrs_name);
            if (INADDR_NONE != hostip) {
                LOG_STRM_PRINTF("#%d: hostname = %s, hostip =0x%08x\n", http->index, strmRRS->rrs_name, hostip);
                strmRRS->rrs_sins[0].in_addr.s_addr = hostip;
                ind_timer_delete(loop->tlink, int_http_open_timeout, http);
                int_http_resovle(http);
            }
        }
        if (http->retryInterval) {
            if (HTTP_STATE_OPEN == http->state && http->retryClk <= loop->clk) {
                if (http->retryInterval >= RETRY_INTERVAL_MAX)
                    http->retryTimes++;
                if (http->retryTimes >= RETRY_TIMES) { //2分30秒
                    LOG_STRM_ERROR("#%d retryTimes = %d\n", http->index, http->retryTimes);
                    int_http_error(http, HTTP_MSG_ERROR);
                } else {
                    if (http->retryInterval > RETRY_INTERVAL_MAX)
                    http->retryInterval = RETRY_INTERVAL_INIT;
                    if (int_http_connect(http, http->byte_start, 0)) {
                        LOG_STRM_ERROR("#%d int_http_connect\n", http->index);
                        int_http_error(http, HTTP_MSG_ERROR);
                    }
                }
            } else if (http->state > HTTP_STATE_OPEN && http->state < HTTP_STATE_END && http->recvClk && http->recvClk < loop->clk) {
                http->byte_start += http->content_bytes;
                http->content_bytes = 0;

                http->retryTimes = 0;
                http->retryInterval = RETRY_INTERVAL_INIT;
                if (int_http_connect(http, http->byte_start, 0)) {
                    LOG_STRM_ERROR("#%d int_http_connect\n", http->index);
                    int_http_error(http, HTTP_MSG_ERROR);
                }
            }
        }
        int_http_byte_stat(http, 0);
        http = http->next;
    }

    if (loop->op.local_100ms)
        loop->op.local_100ms(loop->handle);
}

static void int_httploop_1000ms(void *arg)
{
    int byterate, space;
    HTTP_t http;
    HTTPLoop_t loop = (HTTPLoop_t)arg;

    if (loop->op.local_1000ms)
        loop->op.local_1000ms(loop->handle);

    http = loop->http;
    while (http) {
        http->ignoreTimes ++;
        byterate = int_http_byte_rate(http);
        if (http->sbq)
            space = strm_bufque_space(http->sbq);
        else
            space = 0;

        if (http->ignoreTimes >= 5) {
            LOG_STRM_PRINTF("#%d byterate = %d, recvtimes = %d, bytes = %lld / %lld, space = %d, limit = %d\n", http->index, byterate, http->recvTimes, http->content_bytes, http->content_len, space, http->limit_byterate);
            http->ignoreTimes = 0;
        } else {
            //LOG_STRM_DEBUG("#%d byterate = %d, recvtimes = %d, bytes = %lld / %lld, space = %d, limit = %d\n", http->index, byterate, http->recvTimes, http->content_bytes, http->content_len, space, http->limit_byterate);
        }

        if (http->end_flag == 0 && space >= STREAM_HEAD_LEN) {
            http->timeout ++;
            if (http->timeout >= HTTP_TIMEOUT_SEC) {
                if (http->limit_byterate != 1)
                    LOG_STRM_ERROR("#%d Receive Timeout!\n", http->index);
                http->timeout = 0;
            }
        }

        http = http->next;
    }
}

HTTP_t strm_http_create(HTTPLoop_t loop, int size)
{
    HTTP_t http = (HTTP_t)IND_CALLOC(sizeof(struct HTTP), 1);

    http->state = HTTP_STATE_CLOSE;
    http->sock = -1;
    http->downloadRate = -1;

    http->loop = loop;
    http->index = loop->index;

    int_http_byte_reset(http);

    if (size > 0) {
        http->ts_buffer = ts_buf_create(size);
        if (!http->ts_buffer)
            LOG_STRM_ERROUT("#%d ts_buf_create size = %d!\n", http->index, size);
    }

    if (loop->http)
        loop->http->prev = http;
    http->next = loop->http;
    loop->http = http;

    return http;
Err:
    if (http)
        IND_FREE(http);
    return NULL;
}

static void int_http_delete(HTTP_t http)
{
    HTTPLoop_t loop;
    LOG_STRM_PRINTF("#%d\n", http->index);

    if (!http)
        return;

    strm_http_reset(http);

    loop = http->loop;
    if (http == loop->http) {
        loop->http = http->next;
        if (http->next)
            http->next->prev = NULL;
    } else {
        http->prev->next = http->next;
        if (http->next)
            http->next->prev = http->prev;
    }

    if (http->sb)
        strm_buf_free(http->sb);

    if (http->ts_buffer)
        ts_buf_delete(http->ts_buffer);

    IND_FREE(http);
}

HTTPLoop_t strm_httploop_create(int idx, HttpLoopOp* op, void* handle, mid_msgq_t msgq)
{
    HTTPLoop_t loop = (HTTPLoop_t)IND_CALLOC(sizeof(struct HTTPLoop), 1);

    loop->index = idx;

    loop->op.deal_cmd = op->deal_cmd;
    loop->op.deal_msg = op->deal_msg;
    loop->op.local_100ms = op->local_100ms;
    loop->op.local_1000ms = op->local_1000ms;

    loop->handle = handle;

    loop->msgq = msgq;
    loop->msgfd = mid_msgq_fd(loop->msgq);

    loop->clk = mid_10ms( );

    loop->tlink = int_stream_tlink(idx);
    loop->strm_msgq = int_strm_msgq(idx);

    ind_timer_create(loop->tlink, loop->clk + INTERVAL_CLK_100MS, INTERVAL_CLK_100MS, int_httploop_100ms, loop);
    ind_timer_create(loop->tlink, loop->clk + INTERVAL_CLK_1000MS, INTERVAL_CLK_1000MS, int_httploop_1000ms, loop);

    return loop;
}

void strm_httploop_break(HTTPLoop_t loop)
{
    loop->exit = 1;
}

void strm_httploop_delete(HTTPLoop_t loop)
{
    HTTP_t http, next;

    http = loop->http;
    while (http) {
        next = http->next;
        int_http_delete(http);
        http = next;
    }

    ind_timer_delete_all(loop->tlink);

    IND_FREE(loop);
}

static void int_http_close(HTTP_t http)
{
    HTTPLoop_t loop;

    if (2 == http->sqm_flg) {
        stream_port_post_datasock(0, -1, NULL, NULL, NULL);
        http->sqm_flg = 1;
    }
    if (-1 != http->sock) {
        close(http->sock);
        http->sock = -1;
    }

    loop = http->loop;
    ind_timer_delete(loop->tlink, int_http_open_timeout, http);
    ind_timer_delete(loop->tlink, int_http_data_timeout, http);
}

void strm_http_reset(HTTP_t http)
{
    http->recvTimes = 0;
    http->ignoreTimes = 0;

    http->chunk_len = -1;
    http->end_flag = 0;

    int_http_close(http);

    http->state = HTTP_STATE_CLOSE;
}

int strm_http_buf_get(HTTP_t http, char** pbuf, int* plen)
{
    int len;
    char *buf;

    len = 0;
    buf = NULL;
    if (!http->ts_buf)
        return 0;

    ts_buf_read_get(http->ts_buf, &buf, &len);

    if (pbuf)
        *pbuf = buf;

    if (plen)
        *plen = len;

    return 0;
}

void strm_http_buf_pop(HTTP_t http, int len)
{
    if (http->chunk_len >= 0)
        http->chunk_len -= len;
    ts_buf_read_pop(http->ts_buf, len);
}

int strm_http_buf_length(HTTP_t http)
{
    return ts_buf_length(http->ts_buf);
}

void strm_http_buf_replace(HTTP_t http, ts_buf_t ts_buf)
{
    if (ts_buf) {
        http->sbq = NULL;
        http->ts_buf = ts_buf;
    }
}

void strm_http_sbq_replace(HTTP_t http, StrmBufQue* sbq, int size)
{
    if (http->sb_size && size != http->sb_size) {
        strm_buf_free(http->sb);
        http->sb_size = 0;
    }
    if (!http->sb_size) {
        http->sb = strm_buf_malloc(size);
        http->sb_size = size;
    }
    http->sb->len = 0;
    http->sbq = sbq;
}

static void int_http_request(HTTP_t http)
{
    char *buf;
    int len;
    StrmRRS_t strmRRS;
    HTTPLoop_t loop;

    loop = http->loop;
    strmRRS = &http->strmRRS;

    buf = http->head_buf;

    if (strmRRS->rrs_num > 1 && strmRRS->rrs_idx > 0) {
        char host[IND_ADDR_LEN];
        ind_net_ntop(&strmRRS->rrs_sins[strmRRS->rrs_idx], host, IND_ADDR_LEN);
        len = sprintf(buf, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: Keep-Alive\r\n", strmRRS->rrs_uri, host);
    } else {
        len = sprintf(buf, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: Keep-Alive\r\n", strmRRS->rrs_uri, strmRRS->rrs_name);
    }
    len += sprintf(buf + len, "User-Agent: Mozilla/5.0 (iPad; U; CPU OS 4_3_3 like Mac OS X; zh-cn) AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8J3 Safari/6533.18.5\r\n");

    if (http->etag_flag) {
        if (http->etag[0])
            len += sprintf(buf + len, "If-None-Match: %s\r\n", http->etag);
    }
    if (http->gzip_flag) {
        if (g_httpUnzip)
            len += sprintf(buf + len, "Accept-Encoding: gzip\r\n");
    }
    if (http->cookie[0]) {
        if (ind_stristr(http->cookie, "Cookie: "))
            len += sprintf(buf + len, "%s\r\n", http->cookie);
        else
            len += sprintf(buf + len, "Cookie: %s\r\n", http->cookie);
    }

    if (http->byte_start > 0 || http->byte_count > 0) {
        if (http->byte_count > 0)
            len += sprintf(buf + len, "Range: bytes=%lld-%lld\r\n", http->byte_start, http->byte_start + http->byte_count - 1);
        else
            len += sprintf(buf + len, "Range: bytes=%lld-\r\n", http->byte_start);
    }

    if (http->downloadRate >= 0)
        len += sprintf(buf + len, "DownLoadRate: %d\r\n", http->downloadRate);

    len += sprintf(buf + len, "\r\n");
    http->head_len = len;

    LOG_STRM_PRINTF("#%d SEND: \n%s\n", http->index, buf);
    send(http->sock, http->head_buf, http->head_len, MSG_NOSIGNAL);
    http->recvClk = loop->clk + RECV_TIMEOUT;
    http->head_len = 0;

    LOG_STRM_PRINTF("#%d CLR end_flag\n", http->index);
    http->end_flag = 0;

    ind_timer_create(loop->tlink, loop->clk + INTERVAL_CLK_DATA_TIMEOUT, 0, int_http_data_timeout, http);

    LOG_STRM_PRINTF("#%d HTTP_STATE_RECV_HEAD\n", http->index);
    http->state = HTTP_STATE_RECV_HEAD;
}

static void int_http_error(HTTP_t http, int msgno)
{
    strm_http_reset(http);
    http->op.deal_error(http->handle, msgno);
}

static void int_recv_error(HTTP_t http)
{
    strm_http_reset(http);

    //3)recv或send返回的非超时socket错误，机顶盒关闭连接，重新建链connect( )；
    if (http->retryInterval) {
        http->retryTimes = 0;
        http->retryInterval = RETRY_INTERVAL_INIT;
        if (int_http_connect(http, http->byte_start, 0)) {
            LOG_STRM_ERROR("#%d int_http_connect\n", http->index);
            int_http_error(http, HTTP_MSG_ERROR);
        }
    } else {
        http->op.deal_error(http->handle, HTTP_MSG_ERROR);
    }
}

static void int_http_data_timeout(void* arg)
{
    HTTP_t http = (HTTP_t)arg;

    LOG_STRM_ERROR("#%d HTTP_MSG_NET_TIMEOUT\n", http->index);
    int_http_error(http, HTTP_MSG_NET_TIMEOUT);
}

static int int_http_connect(HTTP_t http, long long byte_start, long long byte_count)
{
    int sock = -1;
    struct ind_sin serv_sin;
    StrmRRS_t strmRRS;
    HTTPLoop_t loop;

    loop = http->loop;
    strmRRS = &http->strmRRS;

    http->timeout = 0;
    http->connection_close = 0;

    strm_http_reset(http);

    serv_sin = strmRRS->rrs_sins[strmRRS->rrs_idx];
    if (0 == serv_sin.port)
        serv_sin.port = 80;

    LOG_STRM_PRINTF("#%d hostip = %08x, port = %u\n", http->index, serv_sin.in_addr.s_addr, (uint32_t)serv_sin.port);
    sock = socket(serv_sin.family, SOCK_STREAM, 0);
    if (sock < 0)
        LOG_STRM_ERROUT("#%d socket\n", http->index);

    {
        socklen_t optlen;
        int size;

        optlen = sizeof(int);
        size = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void *)&size, optlen);
        if (size < HTTP_RECV_SIZE) {
            size = HTTP_RECV_SIZE;
            setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&size, sizeof(size));
        }
    }
    {
        struct linger l;
        l.l_onoff = 1;
        l.l_linger = 0;
        setsockopt(sock, SOL_SOCKET, SO_LINGER, (void *)(&l), sizeof(struct linger));
    }

#if defined(_WIN32)
    {
        unsigned long   nob = 1;
        ioctlsocket(sock, FIONBIO, &nob);
    }
#else
    {
        int k;

        k = 1;
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void*)&k , sizeof(k));
        k = 5;
        setsockopt(sock, SOL_TCP, TCP_KEEPIDLE, (void*)&k , sizeof(k));
        k = 5;
        setsockopt(sock, SOL_TCP, TCP_KEEPINTVL, (void*)&k , sizeof(k));
        k = 24;
        setsockopt(sock, SOL_TCP, TCP_KEEPCNT, (void*)&k, sizeof(k));
    }

    fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK);
#endif

    ind_net_connect(sock, &serv_sin);

    http->sock = sock;
    LOG_STRM_PRINTF("#%d HTTP_STATE_OPEN\n", http->index);
    http->state = HTTP_STATE_OPEN;

    if (http->retryInterval) {
        if (http->retryInterval < RETRY_INTERVAL_MAX)
            http->retryInterval *= 2;
        else
            http->retryInterval = RETRY_INTERVAL_EXTEND;
        http->retryClk = loop->clk + http->retryInterval;
    } else {
        http->retryClk = 0;
        if (strmRRS->rrs_num > 1) {
            uint32_t to = int_stream_rrs_timeout( );
            if (to == 0)
                to = INTERVAL_CLK_DATA_TIMEOUT;
            ind_timer_create(loop->tlink, loop->clk + to, 0, int_http_open_timeout, http);
        } else {
            ind_timer_create(loop->tlink, loop->clk + INTERVAL_CLK_DATA_TIMEOUT, 0, int_http_data_timeout, http);
        }
    }

    return 0;
Err:
    if (sock != -1)
        close(sock);
    return -1;
}

int strm_http_parse_url(char *url, StrmRRS_t strmRRS)
{
    int ret;

    if (strncasecmp(url, "http://", 7))
        LOG_STRM_ERROUT("unknown format\n");

    ret = strm_tool_parse_url(url + 7, strmRRS);
    if (ret < 0)
        LOG_STRM_ERROUT("strm_tool_parse_url\n");
    if (strmRRS->rrs_num <= 0)
        LOG_STRM_ERROUT("rrs_num = %d\n", strmRRS->rrs_num);

    strmRRS->rrs_idx = 0;

    return ret;
Err:
    return -1;
}

static int int_http_resovle(HTTP_t http)
{
    if (HTTP_STATE_END != http->state) {
        if (int_http_connect(http, http->byte_start, http->byte_count))
            LOG_STRM_ERROUT("#%d int_http_connect\n", http->index);
    } else {
        int_http_request(http);
    }

    return 0;
Err:
    return -1;
}


void int_dns_callback(int arg, int dnsmsg, unsigned int hostip)
{
}

int strm_http_request(HTTP_t http, char *url, long long byte_start, long long byte_count)
{
    StrmRRS_t strmRRS;

    LOG_STRM_PRINTF("#%d start = %lld, count = %lld, close = %d\n", http->index, byte_start, byte_count, http->connection_close);

    http->code = 0;
    http->retryClk = 0;
    if (http->retryInterval) {
        http->retryTimes = 0;
        http->retryInterval = RETRY_INTERVAL_INIT;
    }
    http->content_bytes = 0;

    http->sbq = NULL;
    http->ts_buf = http->ts_buffer;

    http->byte_start = byte_start;
    http->byte_count = byte_count;

    strmRRS = &http->strmRRS;

    if (url && '/' != url[0]) {
        int ret;
        uint32_t hostip;

        strm_http_reset(http);
        IND_STRCPY(http->url, url);

        ret = strm_http_parse_url(url, strmRRS);
        if (ret < 0)
            LOG_STRM_ERROUT("#%d strm_http_parse_url\n", http->index);

        if (ret) {
            hostip = mid_dns_cache(strmRRS->rrs_name);
            if (INADDR_NONE == hostip) {
                http->state = HTTP_STATE_DNS;

                mid_dns_resolve(strmRRS->rrs_name, int_dns_callback, 0, 28);
                ind_timer_create(http->loop->tlink, http->loop->clk + INTERVAL_CLK_DATA_TIMEOUT, 0, int_http_open_timeout, http);
                return 0;
            }
            strmRRS->rrs_sins[0].in_addr.s_addr = hostip;
        }
    } else {
        if (strmRRS->rrs_num <= 0)
            LOG_STRM_ERROUT("#%d rrs_num = %d\n", http->index, strmRRS->rrs_num);
        if (HTTP_STATE_END != http->state && strm_tool_parse_url(http->url + 7, strmRRS))
            LOG_STRM_ERROUT("#%d strm_tool_parse_url\n", http->index);

        if (url && '/' == url[0])
            IND_STRCPY(strmRRS->rrs_uri, url + 1);
    }

    if (int_http_resovle(http))
        goto Err;

    return 0;
Err:
    return -1;
}

static void int_http_open_error(HTTP_t http, int err)
{
    if (HTTP_STATE_OPEN == http->state) {
        StrmRRS_t strmRRS = &http->strmRRS;

        if (strmRRS->rrs_idx + 1 >= strmRRS->rrs_num)
            LOG_STRM_ERROUT("#%d rrs_idx = %d, rrs_num = %d\n", http->index, strmRRS->rrs_idx, strmRRS->rrs_num);

        int_http_close(http);

        strmRRS->rrs_idx ++;
        if (int_http_connect(http, http->byte_start, 0))
            LOG_STRM_ERROUT("#%d local_connect\n", http->index);

        return;
    }

Err:
    int_http_error(http, err);
}

static void int_http_open_timeout(void* arg)
{
    HTTP_t http = (HTTP_t)arg;

    LOG_STRM_ERROR("#%d HTTP_MSG_ERROR\n", http->index);
    int_http_open_error(http, HTTP_MSG_ERROR);
}

static void int_http_end(HTTP_t http)
{
    if (http->connection_close) {
        int_http_close(http);
        LOG_STRM_PRINTF("#%d HTTP_STATE_CLOSE\n", http->index);
        http->state = HTTP_STATE_CLOSE;
    } else {
        LOG_STRM_PRINTF("#%d HTTP_STATE_END\n", http->index);
        http->state = HTTP_STATE_END;
    }

    http->op.recv_end(http->handle);
}

static void int_http_push(HTTP_t http)
{
    StrmBuffer *sb = http->sb;

    if (2 == http->sqm_flg && 0 == http->gzip_flag)
        stream_port_post_datapush(0, sb->buf, sb->len, -1);
    sb->off = 0;
    strm_bufque_push(http->sbq, &sb);
    sb->len = 0;
    http->sb = sb;
}

static void int_http_recv_sb1(HTTP_t http)
{
    StrmBuffer *sb;

    if (http->chunk_len < 0 && http->content_bytes >= http->content_len) {
        sb = http->sb;

        LOG_STRM_PRINTF("#%d SET end_flag\n", http->index);
        http->end_flag = 1;
    }

    if (http->end_flag && sb->len > 0)
        int_http_push(http);

    http->op.recv_sync(http->handle);
    if (http->end_flag)
        int_http_end(http);
}

static void int_http_recv_sb0(HTTP_t http, char* data_buf, int data_len)
{
    int len;
    StrmBuffer *sb;

    while (data_len > 0) {
        sb = http->sb;

        len = sb->size - sb->len;
        if (len > data_len)
            len = data_len;

        IND_MEMCPY(sb->buf + sb->len, data_buf, len);
        sb->len += len;
        data_buf += len;
        data_len -= len;

        if (sb->len >= sb->size)
            int_http_push(http);
    }

    int_http_recv_sb1(http);
}

static void int_http_recv_sb(HTTP_t http)
{
    int ret, len;
    char *buf;
    StrmBuffer *sb = http->sb;

    buf = sb->buf + sb->len;
    len = sb->size - sb->len;

    if (http->limit_len && len > http->limit_len)
        len = http->limit_len;
    if (http->chunk_len > 0 && len > http->chunk_len)
        len = http->chunk_len;

    if (len > 0) {
        ret = recv(http->sock, buf, len, 0);
        if (ret <= 0) {
            if (errno == EINPROGRESS)//EINPROGRESS 是socket被服务请求关闭导致的
                LOG_STRM_ERROR("#%d result = %d, EINPROGRESS\n", http->index, ret);
            else
                LOG_STRM_ERROR("#%d len = %d / %d, errno = %d! %s\n", http->index, ret, len, errno, strerror(errno));
            int_recv_error(http);
            return;
        }
        http->recvClk = http->loop->clk + RECV_TIMEOUT;
        sb->len += ret;

        if (http->chunk_len > 0)
            http->chunk_len -= ret;

        if (sb->len >= sb->size)
            int_http_push(http);

        http->recvTimes ++;
        int_http_byte_stat(http, ret);
        http->content_bytes += ret;
    }

    int_http_recv_sb1(http);
}

static void int_http_recv_buf1(HTTP_t http)
{
    if (http->chunk_len < 0 && http->content_len > 0 && http->content_bytes >= http->content_len) {
        LOG_STRM_PRINTF("#%d SET end_flag\n", http->index);
        http->end_flag = 1;
    }

    if (http->end_flag) {
        if (http->recv_buf) {
            if (http->recv_len > 0)
                ts_buf_write_put(http->ts_buf, http->recv_len);
            http->recv_buf = NULL;
        }
    }

    http->op.recv_sync(http->handle);

    if (http->end_flag) {
        if (http->gzip) {
            char *buf, *data_buf;
            int len, size, data_len;

            data_len = 0;
            data_buf = NULL;
            ts_buf_read_get(http->ts_buf, &data_buf, &data_len);
            size = ts_buf_size(http->ts_buf);
            len = size / 16;
            if (len < data_len)
                LOG_STRM_ERROUT("#%d len = %d / %d\n", http->index, len, data_len);

            {
                int ret;
                buf = IND_MALLOC(len);
                if (NULL == buf)
                    LOG_STRM_ERROUT("#%d malloc %d\n", http->index, len);

                IND_MEMCPY(buf, data_buf, data_len);
                len = size;
                ret = g_httpUnzip(buf, data_len, data_buf, &len);
                if (0 == ret) {
                    char *w_buf;
                    int w_len;

                    ts_buf_read_pop(http->ts_buf, data_len);

                    w_len = 0;
                    w_buf = NULL;
                    ts_buf_write_get(http->ts_buf, &w_buf, &w_len);
                    if (w_buf != data_buf || w_len < len) {
                        ret = -1;
                        LOG_STRM_ERROR("#%d w_buf = %p / %p\n", http->index, w_buf, data_buf);
                    } else {
                        ts_buf_write_put(http->ts_buf, len);
                    }
                }
                IND_FREE(buf);
                if (ret)
                    LOG_STRM_ERROUT("#%d g_httpUnzip %d\n\n", http->index, len);
            }
        }
        int_http_end(http);
    }

    return;
Err:
    LOG_STRM_ERROR("#%d HTTP_MSG_ERROR\n", http->index);
    int_http_error(http, HTTP_MSG_ERROR);
}

static void int_http_recv_bufGet(HTTP_t http)
{
    if (!http->recv_buf) {
        http->recv_len = 0;
        http->recv_size = 0;
        ts_buf_write_get(http->ts_buf, &http->recv_buf, &http->recv_size);
        if (http->recv_size > RECV_ELEM_SIZE)
            http->recv_size = RECV_ELEM_SIZE;
    }
}

static void int_http_recv_bufPut(HTTP_t http)
{
    if (http->recv_len <= 0)
        return;
    if (http->gzip_flag || http->recv_len >= http->recv_size || 0 == (http->recv_len % 188)) {
        if (2 == http->sqm_flg && 0 == http->gzip_flag)
            stream_port_post_datapush(0, http->recv_buf, http->recv_len, -1);
        ts_buf_write_put(http->ts_buf, http->recv_len);
        http->recv_buf = NULL;
    }
}

static void int_http_recv_buf0(HTTP_t http, char* data_buf, int data_len)
{
    int len;

    while (data_len > 0) {
        int_http_recv_bufGet(http);
        len = http->recv_size - http->recv_len;

        if (len > data_len)
            len = data_len;
        IND_MEMCPY(http->recv_buf + http->recv_len, data_buf, len);

        http->recv_len += len;
        int_http_recv_bufPut(http);

        data_buf += len;
        data_len -= len;
    }

    int_http_recv_buf1(http);
}

static void int_http_recv_end(HTTP_t http)
{
     http->end_flag = 1;
     if (http->sbq)
         int_http_recv_sb1(http);
     else
         int_http_recv_buf1(http);
}

static int int_http_recv_chunk0(HTTP_t http, char* data_buf, int data_len)
{
    int len, chunk_len;
    char *p, *head_buf = http->chunk_head_buf;

    while (data_len > 0) {
        p = ind_memstr(data_buf, data_len, "\r\n");
        if (!p) {
            if (data_len >= CHUNK_HEAD_SIZE)
                LOG_STRM_ERROUT("#%d chunk sync!\n", http->index);
            IND_MEMCPY(head_buf, data_buf, data_len);
            http->chunk_head_len = len;

            return 0;
        }
        if (p == data_buf) {
            data_buf += 2;
            data_len -= 2;
            continue;
        }
        len = p - data_buf;
        chunk_len = -1;
        ind_hextoi(data_buf, len, (uint32_t*)&chunk_len);
        if (chunk_len < 0)
            LOG_STRM_ERROUT("#%d ind_hextoi chunk_len = %d\n", http->index, chunk_len);

        if (0 == chunk_len) {
            LOG_STRM_PRINTF("#%d SET end_flag\n", http->index);
            int_http_recv_end(http);
            return 0;
        }

        http->chunk_len = chunk_len;

        len += 2;
        data_buf += len;
        data_len -= len;

        len = chunk_len;
        if (len > data_len)
            len = data_len;

        if (http->sbq)
            int_http_recv_sb0(http, data_buf, len);
        else
            int_http_recv_buf0(http, data_buf, len);

        data_buf += len;
        data_len -= len;
        http->chunk_len -= len;
    }

    return 0;
Err:
    return -1;
}

static void int_http_recv_buf(HTTP_t http)
{
    int ret, len;

    int_http_recv_bufGet(http);
    len = http->recv_size - http->recv_len;

    if (http->limit_len && len > http->limit_len)
        len = http->limit_len;
    if (http->chunk_len > 0 && len > http->chunk_len)
        len = http->chunk_len;
    ret = recv(http->sock, http->recv_buf + http->recv_len, len, 0);
    if (ret <= 0) {
        if (errno == EINPROGRESS) {//EINPROGRESS 是socket被服务请求关闭导致的
            if (http->connection_close && 0 == http->content_len) {
                LOG_STRM_PRINTF("#%d result = %d, EINPROGRESS\n", http->index, ret);
                http->end_flag = 1;
                int_http_recv_buf1(http);
                return;
            } else {
                LOG_STRM_ERROR("#%d result = %d, EINPROGRESS\n", http->index, ret);
            }
        } else {
            LOG_STRM_ERROR("#%d len = %d / %d, errno = %d! %s\n", http->index, ret, len, errno, strerror(errno));
        }
        int_recv_error(http);
        return;
    }
    http->recvClk = http->loop->clk + RECV_TIMEOUT;

    http->recv_len += ret;
    if (http->chunk_len > 0)
        http->chunk_len -= ret;

    int_http_recv_bufPut(http);

    http->recvTimes ++;
    int_http_byte_stat(http, ret);
    http->content_bytes += ret;

    int_http_recv_buf1(http);
}

static void int_http_recv_chunk(HTTP_t http)
{
    int len, head_len, chunk_len;
    char *p, *head_buf;

    head_buf = http->chunk_head_buf;
    head_len = http->chunk_head_len;

    len = recv(http->sock, head_buf + head_len, CHUNK_HEAD_SIZE - head_len, 0);
    if (len <= 0) {
        if (errno == EINPROGRESS) {//EINPROGRESS 是socket被服务请求关闭导致的
            if (http->connection_close && 0 == http->content_len) {
                LOG_STRM_PRINTF("#%d result = %d, EINPROGRESS\n", http->index, len);
                int_http_recv_end(http);
                return;
            }
            LOG_STRM_ERROUT("#%d result = %d, EINPROGRESS\n", http->index, len);
        } else {
            LOG_STRM_ERROUT("#%d len = %d / %d, errno = %d! %s\n", http->index, len, CHUNK_HEAD_SIZE - head_len, errno, strerror(errno));
        }
    }
    head_len += len;

    while (head_len > 0) {
        p = ind_memstr(head_buf, head_len, "\r\n");
        if (!p) {
            if (head_len >= CHUNK_HEAD_SIZE)
                LOG_STRM_ERROUT("#%d chunk sync!\n", http->index);

            if (head_buf != http->chunk_head_buf)
                memmove(http->chunk_head_buf, head_buf, head_len);
            http->chunk_head_len = head_len;

            return;
        }
        if (p == head_buf) {
            head_buf += 2;
            head_len -= 2;
            continue;
        }

        len = p - head_buf;
        chunk_len = -1;

        ind_hextoi(head_buf, len, (uint32_t*)&chunk_len);
        if (chunk_len < 0)
            LOG_STRM_ERROUT("#%d ind_hextoi len = %d\n", http->index, chunk_len);

        if (0 == chunk_len) {
            LOG_STRM_PRINTF("#%d SET end_flag\n", http->index);
            int_http_recv_end(http);
            return;
        }

        http->chunk_len = chunk_len;

        len += 2;
        head_buf += len;
        head_len -= len;

        len = chunk_len;
        if (len > head_len)
            len = head_len;

        if (http->sbq)
            int_http_recv_sb0(http,head_buf, len);
        else
            int_http_recv_buf0(http,head_buf, len);

        head_buf += len;
        head_len -= len;
        http->chunk_len -= len;
    }

    http->chunk_head_len = 0;
    return;
Err:
    int_recv_error(http);
}

static void int_http_recv_head(HTTP_t http)
{
    int len, data_len;
    char *p, *buf;
    HTTPLoop_t loop = http->loop;

    data_len = 0;
    http->timeout = 0;

    buf = http->head_buf;
    len = recv(http->sock, buf + http->head_len, STREAM_HEAD_LEN - http->head_len, 0);
    if (len <= 0) {
        LOG_STRM_ERROR("#%d len = %d, errno = %d! %s\n", http->index, len, errno, strerror(errno));
        int_recv_error(http);
        return;
    }
    http->recvClk = loop->clk + RECV_TIMEOUT;
    int_http_byte_stat(http, len);
    http->head_len += len;

    p = ind_memstr(buf, http->head_len, "\r\n\r\n");
    if (!p) {
        if (http->head_len >= STREAM_HEAD_LEN) {
            http->head_buf[STREAM_HEAD_LEN] = 0;
            LOG_STRM_ERROUT("#%d head too large!\n", http->index);
        }
        return;
    }
    len = p + 4 - buf;
    data_len = http->head_len - len;

    buf[len - 4] = 0;

    if (ind_memicmp(buf, "HTTP/1.", 7))
        LOG_STRM_ERROUT("#%d HTTP/1. not fond!\n", http->index);

    http->code = atoi(buf + 9);

    if (301 == http->code || 302 == http->code) {
		int ret = 0;
		
        LOG_STRM_PRINTF("#%d RECV\n", http->index);
        p = ind_stristr(buf, "Location: ");
        if (!p)
            LOG_STRM_ERROUT("#%d Location not fond!\n", http->index);
        p += 10;

        len = ind_linelen(p);
        p[len] = 0;
        LOG_STRM_PRINTF("#%d redirection\n", http->index);
		ret = strm_http_parse_url(p, &http->strmRRS);
        if (ret < 0)
            LOG_STRM_ERROUT("#%d strm_http_parse_url\n", http->index);
		if (ret) {
			StrmRRS_t strmRRS = &http->strmRRS;
            uint32_t hostip = mid_dns_cache(strmRRS->rrs_name);
            if (INADDR_NONE == hostip) {
                http->state = HTTP_STATE_DNS;

                mid_dns_resolve(strmRRS->rrs_name, int_dns_callback, 0, 28);
                ind_timer_create(http->loop->tlink, http->loop->clk + INTERVAL_CLK_DATA_TIMEOUT, 0, int_http_open_timeout, http);
                return;
            }
            strmRRS->rrs_sins[0].in_addr.s_addr = hostip;
        }
        if (int_http_connect(http, http->byte_start, http->byte_count))
            LOG_STRM_ERROUT("#%d local_connect\n", http->index);
        return;
    }
    LOG_STRM_PRINTF("#%d RECV: \n%s\n", http->index, buf);

    ind_timer_delete(loop->tlink, int_http_open_timeout, http);
    ind_timer_delete(loop->tlink, int_http_data_timeout, http);

    if (304 == http->code) {
        http->content_len = 0;
        http->content_length = 0;
        http->op.recv_begin(http->handle);
        LOG_STRM_PRINTF("#%d SET end_flag\n", http->index);
        http->end_flag = 1;

        int_http_end(http);

        return;
    }

    if (http->code < 200 || http->code > 299) {
        LOG_STRM_ERROR("#%d code = %d!\n", http->index, http->code);
        int_http_error(http, http->code);
        return;
    }

    if (buf[7] != '0' && buf[7] != '1')
        LOG_STRM_ERROUT("#%d HTTP version!\n", http->index);

    len = 0;
    p = ind_stristr(buf, "Content-Type: ");
    if (p) {
        p += 14;
        len = ind_linelen(p);
    }
    if (len <= 0 || len >= CONTENT_TYPE_SIZE) {
        LOG_STRM_WARN("#%d Content-Type is empty! len = %d\n", http->index, len);
        http->content_type[0] = 0;
    } else {
        IND_MEMCPY(http->content_type, p, len);
        http->content_type[len] = 0;
    }

    if (http->etag_flag) {
        len = 0;
        p = ind_stristr(buf, "ETag: ");
        if (p) {
            p += 6;
            len = ind_linelen(p);
        }
        if (len <= 0 || len >= 64) {
            LOG_STRM_WARN("#%d ETag is empty! len = %d\n", http->index, len);
            http->etag[0] = 0;
        } else {
            IND_MEMCPY(http->etag, p, len);
            http->etag[len] = 0;
        }
    }

    http->gzip = 0;
    if (http->etag_flag) {
        p = ind_stristr(buf, "Content-Encoding: gzip");
        if (p)
            http->gzip = 1;
    }
    if (ind_stristr(buf, "Connection: close"))
        http->connection_close = 1;

    http->content_len = 0;
    http->content_length = 0;
    if (ind_stristr(buf, "Transfer-Encoding: chunked")) {
        if (http->byte_start != 0)
            LOG_STRM_ERROUT("#%d Transfer-Encoding: chunked!\n", http->index);
        http->chunk_len = 0;
    } else {
        p = ind_stristr(buf, "Content-length: ");
        if (p) {
            http->content_len = ind_ato64(p + 16);

            if (206 == http->code) {//Partial Content
                p = ind_stristr(buf, "Content-Range: ");
                if (!p)
                    LOG_STRM_ERROUT("#%d Content-Range not fond!\n", http->index);
                p = ind_stristr(p, "/");
                if (!p)
                    LOG_STRM_ERROUT("#%d Content-Range invalid!\n", http->index);
                http->content_length = ind_ato64(p + 1);
            } else {
                http->content_length = http->content_len;
            }
            LOG_STRM_PRINTF("#%d Content-length = %lld, Content-len = %lld\n", http->index, http->content_length, http->content_len);
        } else {
            if (!http->connection_close)
                LOG_STRM_ERROUT("#%d Content-length not fond!\n", http->index);
        }
    }

    if (http->op.recv_begin(http->handle) || HTTP_STATE_CLOSE == http->state)
        LOG_STRM_ERROUT("#%d recv_begin\n", http->index);

    if (1 == http->sqm_flg && 0 == http->gzip_flag) {
        socklen_t size;
        struct sockaddr_in peername, sockname;

        size = sizeof(struct sockaddr_in);
        getpeername(http->sock, (struct sockaddr*)&peername, &size);
        getsockname(http->sock, (struct sockaddr*)&sockname, &size);
        stream_port_post_datasock(0, 1, &peername, &sockname, http->url);
        http->sqm_flg = 2;
    }

    LOG_STRM_PRINTF("#%d HTTP_STATE_RECV_BODY sbq = %p, sqm = %d, gzip = %d\n", http->index, http->sbq, http->sqm_flg, http->gzip_flag);
    http->state = HTTP_STATE_RECV_BODY;

    http->recv_buf = NULL;

    http->start_clk = loop->clk;
    if (data_len > 0) {
        char* data_buf = buf + http->head_len - data_len;

        http->content_bytes = data_len;

        if (http->chunk_len >= 0) {
            if (int_http_recv_chunk0(http, data_buf, data_len))
                LOG_STRM_ERROUT("#%d int_http_recv_chunk0\n", http->index);
        } else {
            if (http->sbq)
                int_http_recv_sb0(http, data_buf, data_len);
            else
                int_http_recv_buf0(http, data_buf, data_len);
        }
    }
    return;
Err:
    LOG_STRM_ERROR("#%d HTTP_MSG_ERROR\n", http->index);
    int_http_error(http, HTTP_MSG_ERROR);
    return;
}

static int int_http_byte_rate(HTTP_t http)
{
    int i, byterate, bytes;
    uint32_t clks;
    ByteStat *stat, *stat_array;

    clks = 0;
    bytes = 0;
    stat_array = http->stat_array;
    for (i = 0; i < STAT_ARRAY_SIZE; i ++) {
        stat = &stat_array[i];
        if (stat->clk_begin == 0)
            break;
        clks += stat->clk_end - stat->clk_begin;
        bytes += stat->bytes;
    }

    if (clks <= 0)
        clks = 1;
    byterate = bytes * 100 / clks;

    return byterate;
}

static void int_http_byte_reset(HTTP_t http)
{
    int i;
    for (i = 0; i < STAT_ARRAY_SIZE; i ++)
        http->stat_array[i].clk_begin = 0;
    http->limit_byterate = 0;
}

static void int_http_byte_stat(HTTP_t http, int bytes)
{
    int i;
    uint32_t clk;
    ByteStat *stat, *stat_array;

    clk = http->loop->clk;
    stat_array = http->stat_array;

    stat = &http->stat_array[0];
    if (stat->clk_begin == 0 || clk - stat->clk_begin >= 100) {
        for (i = STAT_ARRAY_SIZE - 1; i > 0; i --)
            stat_array[i] = stat_array[i - 1];
        stat->bytes = 0;
        stat->clk_begin = clk;
    }
    stat->bytes += bytes;
    stat->clk_end = clk;
}

void strm_http_debug(HTTP_t http)
{
}

void strm_httploop_loop(HTTPLoop_t loop)
{
    uint32_t        clk, dif, out;
    fd_set          rset, wset;
    int             rnum, wnum, space, maxfd;
    HTTP_t          http;
    struct timeval  tv;

    while (!loop->exit) {

        clk = mid_10ms( );
        loop->clk = clk;
        out = ind_timer_clock(loop->tlink);
        if (out <= clk) {
            ind_timer_deal(loop->tlink, clk);
            //LOG_STRM_PRINTF("@1 out = %d\n", (int)out);
            continue;
        }

        if (strm_msgq_valid(loop->strm_msgq)) {
            StreamMsg msg;
            strm_msgq_print(loop->strm_msgq);
            if (strm_msgq_pump(loop->strm_msgq, &msg) == 1) {
                if (loop->op.deal_msg)
                    loop->op.deal_msg(loop->handle, msg.msg, msg.arg);
            }
            continue;
        }

        dif = out - clk;
        tv.tv_sec = 0;
        if (dif >= 100)
            tv.tv_usec = 99 * 10000;
        else
            tv.tv_usec = dif * 10000;

        FD_ZERO(&rset);
        FD_SET((uint32_t)loop->msgfd, &rset);
        maxfd = loop->msgfd;

        rnum = 0;
        wnum = 0;

        http = loop->http;
        while (http) {
            if (http->sock != -1) {
                switch (http->state) {
                case HTTP_STATE_OPEN:
                    if (0 == wnum)
                        FD_ZERO(&wset);
                    FD_SET((uint32_t)http->sock, &wset);
                    wnum ++;
                    if (maxfd < http->sock)
                        maxfd = http->sock;
                    break;
                case HTTP_STATE_RECV_HEAD:
                    FD_SET((uint32_t)http->sock, &rset);
                    rnum ++;
                    if (maxfd < http->sock)
                        maxfd = http->sock;
                    break;
                case HTTP_STATE_RECV_BODY:
                    if (http->sbq)
                        space = strm_bufque_space(http->sbq);
                    else
                        space = ts_buf_size(http->ts_buf) - ts_buf_length(http->ts_buf);
                    if (space <= 0) {
                        http->recvClk = 0;
                        break;
                    }
                    if (http->limit_byterate) {
                        long long bytes;

                        bytes = (long long)http->limit_byterate * (loop->clk - http->start_clk) / 100;
                        if (bytes <= http->content_bytes) {
                            http->recvClk = 0;
                            break;
                        }

                        bytes -= http->content_bytes;
                        if (bytes > 0x1000000)
                            http->limit_len = 0x1000000;
                        else
                            http->limit_len = (int)bytes;
                    }
                    if (!http->recvClk)
                        http->recvClk = loop->clk + RECV_TIMEOUT;
                    FD_SET((uint32_t)http->sock, &rset);
                    rnum ++;
                    if (maxfd < http->sock)
                        maxfd = http->sock;
                    break;
                default:
                    break;
                }
            }
            http = http->next;
        }
        if (wnum > 0) {
            if (select(maxfd + 1, &rset, &wset,  NULL, &tv) <= 0)
                continue;
        } else {
            if (select(maxfd + 1, &rset, NULL,  NULL, &tv) <= 0)
                continue;
        }

        if (FD_ISSET((uint32_t)loop->msgfd, &rset)) {
            StreamCmd strmCmd;

            memset(&strmCmd, 0, sizeof(strmCmd));
            mid_msgq_getmsg(loop->msgq, (char *)(&strmCmd));
            loop->op.deal_cmd(loop->handle, &strmCmd);
            continue;
        }

        if (wnum > 0) {
            http = loop->http;
            while (http) {
                if (http->state == HTTP_STATE_OPEN && FD_ISSET((uint32_t)http->sock, &wset)) {
                    socklen_t len;
                    int err;

                    err = -1;
                    len = sizeof(err);

                    getsockopt(http->sock, SOL_SOCKET, SO_ERROR, (void*)&err, &len);
                    if (err == 0) {
                        int_http_request(http);
                    } else {
                        LOG_STRM_ERROR("#%d retry = %d, error = %d! %s\n", loop->index, http->retryTimes, err, strerror(err));
                        if (http->retryInterval) {
                            int_http_close(http);
                        } else {
                            if (err == ECONNREFUSED)
                                int_http_open_error(http, HTTP_MSG_REFUSED_SOCKET);
                            else
                                int_http_open_error(http, HTTP_MSG_ERROR);
                        }
                    }
                }
                http = http->next;
            }
        }
        if (rnum > 0) {
            http = loop->http;
            while (http) {
                if (http->sock != -1 && FD_ISSET((uint32_t)http->sock, &rset)) {
                    switch(http->state) {
                    case HTTP_STATE_RECV_HEAD:
                        int_http_recv_head(http);
                        break;
                    case HTTP_STATE_RECV_BODY:
                        if (0 == http->chunk_len) {
                            int_http_recv_chunk(http);
                        } else {
                            if (http->sbq)
                                int_http_recv_sb(http);
                            else
                                int_http_recv_buf(http);
                        }
                        break;
                    default:
                        break;
                    }
                }
                http = http->next;
            }
        }
    }
}

unsigned int strm_httploop_clk(HTTPLoop_t loop)
{
    return loop->clk;
}

ind_tlink_t strm_httploop_tlink(HTTPLoop_t loop)
{
    return loop->tlink;
}

int strm_http_get_host(HTTP_t http, char *host)
{
    StrmRRS_t strmRRS = &http->strmRRS;

    if (strmRRS->rrs_num <= 0)
        return -1;

    if (strmRRS->rrs_idx > 0) {
        ind_sin_t sin = &strmRRS->rrs_sins[strmRRS->rrs_idx];
        ind_net_ntop(sin, host, HTTP_HOST_SIZE);
    }
    else
        strcpy(host, strmRRS->rrs_name);

    return 0;
}

int strm_http_get_uri(HTTP_t http, char *uri)
{
    StrmRRS_t strmRRS = &http->strmRRS;

    if (strmRRS->rrs_num <= 0)
        LOG_STRM_ERROUT("rrs_num = %d\n", strmRRS->rrs_num);

    uri[0] = '/';
    IND_STRCPY(uri + 1, strmRRS->rrs_uri);

    {
        char *path, *p = NULL;

        for (path = uri; *path; path++) {
            if ('?' == *path)
                break;
            if ('/' == *path)
                p = path;
        }
        if (!p)
            goto Err;//不可能错误
        *p = 0;
    }

    return 0;
Err:
    return -1;
}

int strm_http_get_code(HTTP_t http)
{
    return http->code;
}
int strm_http_get_active(HTTP_t http)
{
    if (HTTP_STATE_CLOSE != http->state)
        return 1;
    return 0;
}

uint32_t strm_http_get_recvTimes(HTTP_t http)
{
    return http->recvTimes;
}

long long strm_http_get_contentLen(HTTP_t http)
{
    return http->content_len;
}

void strm_http_get_contentType(HTTP_t http, char* content_type)
{
    IND_STRCPY(content_type, http->content_type);
}

long long strm_http_get_contentBytes(HTTP_t http)
{
    return http->content_bytes;
}

long long strm_http_get_contentLength(HTTP_t http)
{
    return http->content_length;
}

void strm_http_set_limit(HTTP_t http, int byterate)
{
    LOG_STRM_PRINTF("#%d byterate = %d\n", http->index, byterate);
    int_http_byte_reset(http);
    if (byterate <= 0)
        http->limit_byterate = 0;
    else
        http->limit_byterate = byterate;
}

void strm_http_set_sqm(HTTP_t http, int sqm)
{
    LOG_STRM_PRINTF("#%d sqm = %d!\n", http->index, sqm);
    if (0 == http->sqm_flg && sqm)
        http->sqm_flg = 1;
}

void strm_http_set_etag(HTTP_t http, int etag)
{
    LOG_STRM_PRINTF("#%d etag = %d!\n", http->index, etag);
    http->etag_flag = etag;
}

void strm_http_set_retry(HTTP_t http, int retry)
{
    LOG_STRM_PRINTF("#%d retry = %d!\n", http->index, retry);
    http->retryTimes = 0;
    if (retry)
        http->retryInterval = RETRY_INTERVAL_INIT;
    else
        http->retryInterval = 0;
}

void strm_http_set_gzip(HTTP_t http, int gzip)
{
    LOG_STRM_PRINTF("#%d gzip = %d!\n", http->index, gzip);
    http->gzip_flag = gzip;
}

void strm_http_set_opset(HTTP_t http, HttpOp* op, void* handle)
{
    http->op = *op;
    http->handle = handle;
}

void strm_http_set_cookie(HTTP_t http, char *cookie)
{
    IND_STRCPY(http->cookie, cookie);
}

void strm_http_set_bitrate(HTTP_t http, int bitrate)
{
    http->downloadRate = bitrate / 1024;
}

void mid_stream_http_unzip(StrmHttpUnzip httpUnzip)
{
    LOG_STRM_PRINTF("httpUnzip = %p\n", httpUnzip);
    g_httpUnzip = httpUnzip;
}
