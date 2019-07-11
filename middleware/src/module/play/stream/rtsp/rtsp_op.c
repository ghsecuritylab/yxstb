
/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "rtsp_app.h"

static int rtsp_op_resolve_ok(struct RTSP* rtsp);
static int rtsp_op_resolve_err(struct RTSP* rtsp);

static int rtsp_op_connect(struct RTSP* rtsp);
static int rtsp_op_connect_ok(struct RTSP* rtsp);

static int rtsp_op_describe(struct RTSP* rtsp);
static int rtsp_op_describe_ok(struct RTSP* rtsp);

static int rtsp_op_timeshift(struct RTSP* rtsp);
static int rtsp_op_timeshift_ok(struct RTSP* rtsp);

static int rtsp_op_connect_err(struct RTSP* rtsp);

static int rtsp_op_burst(struct RTSP* rtsp, int up);
static int rtsp_op_burst_ok(struct RTSP* rtsp);

static int rtsp_op_burst_up(struct RTSP* rtsp);
static int rtsp_op_burst_down(struct RTSP* rtsp);

static int rtsp_op_arq_ctc(struct RTSP* rtsp);
static int rtsp_op_arq_ctc_ok(struct RTSP* rtsp);
static int rtsp_op_arq_ctc_err(struct RTSP* rtsp);

void rtsp_op_init(struct RTSP* rtsp)
{
    int i;
    struct RTSPOps *ops = rtsp->ops;

    rtsp->op_server = RTSP_OP_SERVER_NONE;

    for (i = 0; i < RTSP_OP_MAX; i ++) {
        ops[i].out = 0;
        ops[i].op_f = NULL;
        ops[i].ok_f = NULL;
        ops[i].err_f = NULL;
    }

    ops[RTSP_OP_RESOLVE].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_RESOLVE].op_f = NULL;
    ops[RTSP_OP_RESOLVE].ok_f = rtsp_op_resolve_ok;
    ops[RTSP_OP_RESOLVE].err_f = rtsp_op_resolve_err;

    ops[RTSP_OP_CONNECT].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_CONNECT].op_f = rtsp_op_connect;
    ops[RTSP_OP_CONNECT].ok_f = rtsp_op_connect_ok;
    ops[RTSP_OP_CONNECT].err_f = rtsp_op_connect_err;

    ops[RTSP_OP_DESCRIBE].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_DESCRIBE].op_f = rtsp_op_describe;
    ops[RTSP_OP_DESCRIBE].ok_f = rtsp_op_describe_ok;
    ops[RTSP_OP_DESCRIBE].err_f = rtsp_op_open_err;

    //2010-12-21 18:09:22 移到rtsp_op_huawei.c会导致直播无法进入时移
    ops[RTSP_OP_TIMESHIFT].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_TIMESHIFT].op_f = rtsp_op_timeshift;
    ops[RTSP_OP_TIMESHIFT].ok_f = rtsp_op_timeshift_ok;
    ops[RTSP_OP_TIMESHIFT].err_f = rtsp_op_timeshift_ok;

    //2010-12-21 18:09:26 移到rtsp_op_huawei.c会导致播放无法退出
    ops[RTSP_OP_TEARDOWN].out = TIMEOUT_CLK_TEARDOWN;
    ops[RTSP_OP_TEARDOWN].op_f = rtsp_op_teardown;
    ops[RTSP_OP_TEARDOWN].ok_f = rtsp_op_teardown_ok;
    ops[RTSP_OP_TEARDOWN].err_f = rtsp_op_teardown_ok;

    ops[RTSP_OP_BURST_UP].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_BURST_UP].op_f = rtsp_op_burst_up;
    ops[RTSP_OP_BURST_UP].ok_f = rtsp_op_burst_ok;
    ops[RTSP_OP_BURST_UP].err_f = NULL;

    ops[RTSP_OP_BURST_DOWN].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_BURST_DOWN].op_f = rtsp_op_burst_down;
    ops[RTSP_OP_BURST_DOWN].ok_f = rtsp_op_burst_ok;
    ops[RTSP_OP_BURST_DOWN].err_f = NULL;

    ops[RTSP_OP_ARQ].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_ARQ].op_f = rtsp_op_arq_ctc;
    ops[RTSP_OP_ARQ].ok_f = rtsp_op_arq_ctc_ok;
    ops[RTSP_OP_ARQ].err_f = rtsp_op_arq_ctc_err;
}

static void rtsp_op_timeout(void* arg);

void rtsp_op_none(struct RTSP* rtsp)
{
    if (rtsp->cmdsn)
        rtsp_clt_cmdback(rtsp);

    if (rtsp->recv_safe == 2)
        rtsp->recv_safe = 1;
    ind_timer_delete(rtsp->tlink, rtsp_op_timeout, rtsp);
    rtsp->op = RTSP_OP_NONE;
}

void rtsp_op_succeed(struct RTSP* rtsp)
{
    RTSP_OP op;
    struct RTSPOps *ops = rtsp->ops;

    op = rtsp->op;
    if (op <= 0 || op >= RTSP_OP_MAX) {
        LOG_STRM_ERROR("#%d op = %d\n", rtsp->index, op);
        return;
    }

    if (ops[op].ok_f)
        ops[op].ok_f(rtsp);
    else
        rtsp_op_none(rtsp);
    ind_timer_delete(rtsp->tlink, rtsp_op_timeout, rtsp);
}

void rtsp_op_failed(struct RTSP* rtsp)
{
    RTSP_OP op;
    struct RTSPOps *ops = rtsp->ops;

    op = rtsp->op;
    if (op == RTSP_OP_NONE) {
        LOG_STRM_ERROR("#%d op is none\n", rtsp->index);
        rtsp_op_open_err(rtsp);//socket出错处理。
        return;
    }

    if (op <= 0 || op >= RTSP_OP_MAX) {
        LOG_STRM_ERROR("#%d op = %d\n", rtsp->index, op);
        return;
    }

    if (ops[op].err_f)
        ops[op].err_f(rtsp);
    else
        rtsp_op_none(rtsp);

    ind_timer_delete(rtsp->tlink, rtsp_op_timeout, rtsp);
}

static void rtsp_op_timeout(void* arg)
{
    struct RTSP* rtsp = (struct RTSP*)arg;

    LOG_STRM_WARN("rtsp_op_failed op = %d\n", rtsp->op);
    if (RTSP_OP_PLAY == rtsp->op || RTSP_OP_PLAY_INIT == rtsp->op || RTSP_OP_PAUSE == rtsp->op)
        rtsp->rtsp_code = RTSP_CODE_Play_Timeout;
    else
        rtsp->rtsp_code = RTSP_CODE_Session_Timeout;
    rtsp_op_failed((struct RTSP*)arg);
}

void rtsp_op_timeout_reg(struct RTSP* rtsp, RTSP_OP op)
{
    struct RTSPOps *ops = rtsp->ops;

    if (op <= 0 || op >= RTSP_OP_MAX)
        LOG_STRM_ERROUT("#%d op = %d\n", rtsp->index, op);
    rtsp->op = op;
    rtsp->op_clk = rtsp->clk;
    rtsp->op_cseq = rtsp->CSeq;

    ind_timer_create(rtsp->tlink, rtsp->op_clk + ops[op].out, 0, rtsp_op_timeout, rtsp);

    rtsp_clt_fdset(rtsp);

    return;
Err:
    rtsp_op_none(rtsp);
}

void rtsp_op_immediate(void* arg)
{
    RTSP_OP op;
    struct RTSPOps *ops;
    struct RTSP* rtsp = (struct RTSP*)arg;

    ops = rtsp->ops;
    op = rtsp->op;
    if (op <= 0 || op >= RTSP_OP_MAX)
        LOG_STRM_ERROUT("#%d op = %d\n", rtsp->index, op);
    if (ops[op].op_f == NULL)
        LOG_STRM_ERROUT("#%d op = %d op_f is NULL\n", rtsp->index, op);

    rtsp->rtsp_code = 0;
    ops[op].op_f(rtsp);

    return;
Err:
    rtsp_op_none(rtsp);
    return;
}

void rtsp_op_immediate_reg(struct RTSP* rtsp, RTSP_OP op)
{
    rtsp->op = op;
    ind_timer_create(rtsp->tlink, 0, 0, rtsp_op_immediate, rtsp);
}

int rtsp_op_open_err(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d state = %d\n", rtsp->index, rtsp->state);
    if (rtsp->err_no == 0) {
        if (rtsp->op == RTSP_OP_CONNECT)
            rtsp->err_no = RTSP_ERROR_CONNECT;
        else
            rtsp->err_no = RTSP_ERROR_OPEN;
    }

    if (rtsp->post_clk) {
        if (rtsp->open_play)
            stream_port_post_fail(0, rtsp->url, rtsp->err_no);
        rtsp->post_clk = 0;
    }
    rtsp->err_no = 0;

    if (rtsp->open_play) {
        int idx = 0;

        if (STREAM_INDEX_PIP == rtsp->index)
            idx = 1;
        switch (rtsp->op) {
        case RTSP_OP_DESCRIBE:
            int_back_rtspMethod(idx, "DESCRIBE");
            break;
        case RTSP_OP_SETUP:
            int_back_rtspMethod(idx, "SETUP");
            break;
        case RTSP_OP_PLAY_INIT:
        case RTSP_OP_PLAY:
            int_back_rtspMethod(idx, "PLAY");
            break;
        case RTSP_OP_PAUSE:
            int_back_rtspMethod(idx, "PAUSE");
            break;
        case RTSP_OP_CACHE_ON:
        case RTSP_OP_CACHE_OFF:
            int_back_rtspMethod(idx, "SET_PARAMETER");
            break;
        case RTSP_OP_GET_RET:
        case RTSP_OP_HEARTBIT:
            int_back_rtspMethod(idx, "GET_PARAMETER");
            break;
        default:
            int_back_rtspMethod(idx, "");
            break;
        }
    }
    rtsp_op_none(rtsp);

    strm_msgq_reset(rtsp->strm_msgq, rtsp->index);
    rtsp_msg_back(rtsp, STRM_MSG_OPEN_ERROR, rtsp->rtsp_code);

    return 0;
}

int rtsp_op_location(struct RTSP* rtsp)
{
    char *buf, *p;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    buf = rtsp->ctrl_buf;

    p = ind_stristr(buf, "Location: ");
    if (p == NULL)
        LOG_STRM_ERROUT("#%d Location not exist!\n", rtsp->index);
//重定向

    {
        RTSPStat_t stat = &rtsp->stat;
        stat->stat_vodpause = 0;
        stat->stat_vodplay = 0;
    }

    LOG_STRM_PRINTF("#%d REDIRECT!\n", rtsp->index);
    p += 10;
    if (ind_strline(p, rtsp->url, STREAM_URL_SIZE) < 0)
        LOG_STRM_ERROUT("#%d ind_strline\n", rtsp->index);
    if (strstr(rtsp->url, ".wmv") || strstr(rtsp->url, ".mp4"))
        LOG_STRM_ERROUT("#%d unavailable_media\n", rtsp->index);

    if (rtsp->op == RTSP_OP_PLAY || rtsp->op == RTSP_OP_PAUSE)
        rtsp->retry_flg = 1;

    rtsp_clt_close_vod(rtsp);
    rtsp->redirect = 1;
    rtsp_op_immediate_reg(rtsp, RTSP_OP_CONNECT);
    return 0;
Err:
    rtsp_op_open_err(rtsp);
    return -1;
}

void rtsp_op_resolve(struct RTSP* rtsp, int idx)
{
    StrmRRS_t strmRRS = &rtsp->strmRRS;

    rtsp->redirect = 0;
    strmRRS->rrs_idx = idx;

    if (idx > 0 || strmRRS->rrs_sins[0].in_addr.s_addr) {
        rtsp_clt_close_vod(rtsp);

        rtsp_clt_make_url(rtsp);
        rtsp_op_immediate_reg(rtsp, RTSP_OP_CONNECT);

        return;
    }

    strm_tool_dns_resolve(strmRRS->rrs_name);
    rtsp_op_timeout_reg(rtsp, RTSP_OP_RESOLVE);
}

static int rtsp_op_resolve_ok(struct RTSP* rtsp)
{
    rtsp_clt_make_url(rtsp);
    rtsp_op_immediate_reg(rtsp, RTSP_OP_CONNECT);
    return 0;
}

static int rtsp_op_resolve_err(struct RTSP* rtsp)
{
    rtsp->rtsp_code = RTSP_CODE_Connect_Error;
    return rtsp_op_open_err(rtsp);
}

static int rtsp_op_connect(struct RTSP* rtsp)
{
    char                ch;
    struct ind_sin     *serv_sin;
    int                 len, sock = -1;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp->cache = CACHE_STATE_OFF_INIT;
    rtsp->tcp_flag = 0;

    //失败重连处理
    rtsp_clt_post_valid(rtsp, 0);

    rtsp->Session[0] = 0;

    if (rtsp->open_play) {
        int idx = 0;
        if (STREAM_INDEX_PIP == rtsp->index)
            idx = 1;
        int_back_rtspURL(idx, rtsp->url);

        if (!rtsp->redirect)
            rtsp->post_clk = rtsp->clk;
    }

    len = strlen(rtsp->url);
    ch = rtsp->url[4];

    //B100 重定向也出现了rtspt
    if (ch != ':') {
        if (ch == 'u')
            rtsp->transtype = SOCK_DGRAM;
        else if (ch == 't')
            rtsp->transtype = SOCK_STREAM;
        else
            LOG_STRM_ERROUT("#%d url\n", rtsp->index);
        if (len < 8)
            LOG_STRM_ERROUT("#%d url2\n", rtsp->index);
        len --;
        memmove(rtsp->url + 4, rtsp->url + 5, len - 5);
        rtsp->url[len] = 0;
    }

    if (strncasecmp(rtsp->url, "rtsp://", 7))
        LOG_STRM_ERROUT("#%d url3\n", rtsp->index);

    serv_sin = &rtsp->serv_sin;
    len = ind_net_pton(rtsp->url + 7, serv_sin);
    if (len <= 0)
        LOG_STRM_ERROUT("#%d len: %d\n", rtsp->index, len);
    if (serv_sin->port == 0)
        serv_sin->port = RTSP_DEFAULT_PORT;

    LOG_STRM_DEBUG("#%d family = %hd, port = %d\n", rtsp->index, serv_sin->family, serv_sin->port);
    sock = socket(serv_sin->family, SOCK_STREAM, 0);
    if (sock < 0)
        LOG_STRM_ERROUT("#%d socket errno = %d! %s\n", rtsp->index, errno, strerror(errno));

    {
        int opt = 256 * 1024;
        if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&opt, sizeof(opt)) < 0)
            LOG_STRM_ERROUT("#%d Can't change system network size (wanted size = %d)\n", rtsp->index, opt);
    }

#if 1
    {
        struct linger l;
        l.l_onoff = 1;
        l.l_linger = 0;
        setsockopt(sock, SOL_SOCKET, SO_LINGER, (void *)(&l), sizeof(struct linger));
    }
#endif

#if defined(x86win32)
    {
        unsigned long   nob = 1;
        ioctlsocket(sock, FIONBIO, &nob);
    }
#else
    fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK);
#endif

    if (rtsp->port_tcp) {
        struct ind_sin sin;
        memset(&sin, 0, sizeof(sin));
        sin.family = serv_sin->family;
        sin.port = (unsigned short)rtsp->port_tcp;
        ind_net_bind(sock, &sin);
    }

    ind_net_connect(sock, serv_sin);

    {
        uint32_t to = int_stream_rrs_timeout();
        if (to) {
            LOG_STRM_DEBUG("#%d to = %u\n", rtsp->index, to);
            rtsp->ops[RTSP_OP_CONNECT].out = to;
        }
    }
    rtsp_op_timeout_reg(rtsp, RTSP_OP_CONNECT);

    rtsp->sock = sock;
    rtsp_clt_fdset(rtsp);

    if (rtsp->recv_safe)
        rtsp->recv_safe = 1;

    rtsp->rtsp_code = 0;
    rtsp->servtime_sync = 0;

    return 0;
Err:
    rtsp->err_no = RTSP_ERROR_URL;
    if (sock >= 0)
        close(sock);
    rtsp_op_open_err(rtsp);
    return -1;
}

static int rtsp_op_connect_ok(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d sock = %d\n", rtsp->index, rtsp->sock);

    rtsp_op_immediate_reg(rtsp, RTSP_OP_DESCRIBE);

    return 0;
}

static int rtsp_op_connect_err(struct RTSP* rtsp)
{
    int idx;
    StrmRRS_t strmRRS = &rtsp->strmRRS;

    LOG_STRM_PRINTF("#%d redirect = %d\n", rtsp->index, rtsp->redirect);

    if (RTSP_CODE_Session_Timeout == rtsp->rtsp_code)
        rtsp->rtsp_code = RTSP_CODE_Connect_Timeout;
    else
        rtsp->rtsp_code = RTSP_CODE_Connect_Error;

    if (rtsp->redirect)
        return rtsp_op_open_err(rtsp);

    idx = strmRRS->rrs_idx;

    idx ++;
    LOG_STRM_PRINTF("#%d rrs_idx = %d / rrs_num = %d\n", rtsp->index, strmRRS->rrs_idx, strmRRS->rrs_num);

    if (idx >= strmRRS->rrs_num)
        return rtsp_op_open_err(rtsp);

    rtsp_clt_close_vod(rtsp);

    rtsp_op_resolve(rtsp, idx);

    return 0;
}

static int rtsp_op_describe(struct RTSP* rtsp)
{
    int len;

    LOG_STRM_PRINTF("#%d burst_flag = %d\n", rtsp->index, rtsp->burst_flag);

    len = sprintf(rtsp->send_buf, "DESCRIBE %s", rtsp->url);
    len += sprintf(rtsp->send_buf + len, " RTSP/1.0\r\n");
    len += sprintf(rtsp->send_buf + len, "CSeq: %d\r\n", ++rtsp->CSeq);
    len += sprintf(rtsp->send_buf + len, "User-Agent: %s\r\n", rtsp->UserAgent);
    len += sprintf(rtsp->send_buf + len, "Accept: application/sdp\r\n");
    if (rtsp->timeshift_second && rtsp->apptype != APP_TYPE_VOD) {
        if (rtsp->timeshift_status)
            len += sprintf(rtsp->send_buf + len, "Timeshift: 1\r\n");
        else
            len += sprintf(rtsp->send_buf + len, "Timeshift: 0\r\n");
    }

    if (rtsp->burst_flag)
        len += sprintf(rtsp->send_buf + len, "x-Burst\r\n");

    if (rtsp->arq_flag)
        len += sprintf(rtsp->send_buf + len, "x-Retrans\r\n");

    len += sprintf(rtsp->send_buf + len, "\r\n");

    if (rtsp_clt_send(rtsp, len))
        LOG_STRM_ERROUT("#%d rtsp_clt_send\n", rtsp->index);

    rtsp_op_timeout_reg(rtsp, RTSP_OP_DESCRIBE);

    return 0;
Err:
    rtsp_op_open_err(rtsp);
    return -1;
}

static int rtsp_op_describe_ok(struct RTSP* rtsp)
{
    int len, addrType;
    char ip[16];
    char *buf, *p;
    int timeshift_support = 1;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    buf = rtsp->ctrl_buf;

    if (rtsp->burst_flag) {
        if (ind_stristr(buf, "x-Burst: yes"))
            rtsp->burst_flag = 2;
        else
            rtsp->burst_flag = 1;
    }

    if (rtsp->arq_flag) {
        if (ind_stristr(buf, "x-Retrans: yes")) {
            rtsp->arq_flag = 2;
        } else {
            rtsp->arq_flag = 1;
        }
    }

    {
        char *srv = ind_stristr(buf, "Server: ");
        if (srv) {
            srv += 8;
            if (memcmp(srv, "ZXUSS", 5) == 0 || memcmp(srv, "ZMSS", 4) == 0) {//中兴
                rtsp->op_server = RTSP_OP_SERVER_ZTE;
            } else if (memcmp(srv, "Elecard", 7) == 0) {
                rtsp->standard = RTSP_STANDARD_ELECARD;
                rtsp->op_server = RTSP_OP_SERVER_ELECARD;
//            } else if (memcmp(srv, "MGU", 3) == 0) {//江苏增值服务器
//                rtsp->op_server = RTSP_OP_SERVER_SIMPLE;
            }
        }
    }

    if (rtsp->op_server == RTSP_OP_SERVER_NONE) {
        p = ind_stristr(buf, "s=");
        if (p) {
            if (ind_linestr(p + 2, "LIVE555 Media Server"))
                rtsp->op_server = RTSP_OP_SERVER_LIVE555;
        }
    }
    LOG_STRM_PRINTF("#%d op_server = %d\n", rtsp->index, rtsp->op_server);

    if (rtsp->timeshift_second) {
        if (ind_stristr(buf, "Timeshift-Status: 0"))
            timeshift_support = 0;
        if (rtsp->timeshift_status == 0) {
            rtsp->timeshift_support = timeshift_support;
        } else if (timeshift_support == 0) {
            LOG_STRM_WARN("#%d timeshift: status = 1 and support = 0\n", rtsp->index);
            rtsp->timeshift_status = 0;
            rtsp->retry_op = RTSP_OP_STOP;
        }
        LOG_STRM_PRINTF("#%d timeshift_support = %d\n", rtsp->index, rtsp->timeshift_support);
    }

    rtsp->trackID = -1;
    p = ind_stristr(buf, "a=control:trackID=");
    if (p)
        sscanf(p + 18, "%d", &rtsp->trackID);
    p = ind_stristr(buf, "Content-Base: ");
    if (p) {
        len = ind_strline(p + 14, rtsp->ContentBase, STREAM_URL_SIZE - 4);
        if (len <= 0)
            LOG_STRM_ERROUT("#%d ind_strline Content-Base\n", rtsp->index);
        if (rtsp->ContentBase[len - 1] == '/')
            rtsp->ContentBase[len - 1] = 0;
    }

    p = ind_stristr(buf, "b=AS:");
    if (p)
        sscanf(p + 5, "%d", &rtsp->bitrate);

    if (rtsp->apptype == APP_TYPE_VOD) {//点播

        rtsp->rtsp_code = RTSP_CODE_Slice_Range_Error;

        p = ind_stristr(buf, "a=range:npt=");
        if (p) {
            int begin, end;

            p += 12;
            begin = 0;
            sscanf(p, "%d", &begin);

            p = strchr(p, '-');
            if (p == NULL)
                LOG_STRM_ERROUT("#%d '-' nod found\n", rtsp->index);
            p += 1;
            end = 0;
            if (sscanf(p, "%d", &end) != 1)
                LOG_STRM_WARN("#%d sscanf end failed\n", rtsp->index);
            if (begin >= end || end <= 0) {
                rtsp->time_begin = 0;
                rtsp_clt_time_set_total(rtsp, 0);
            } else {
                if (end <= begin || end <= 0)
                    LOG_STRM_ERROUT("#%d  begin = %d, end = %d\n", rtsp->index, begin, end);

                if (begin < 0)
                    begin = 0;
                if (rtsp->ctc_begin == -1) {
                    rtsp->time_begin = begin;
                    rtsp_clt_time_set_total(rtsp, end - begin);
                } else {
                    if (rtsp->ctc_end <= begin || rtsp->ctc_end > end) {
                        LOG_STRM_WARN("#%d ctc_end = %d, begin = %d, end = %d\n", rtsp->index, rtsp->ctc_end, begin, end);
                        rtsp->ctc_end = end;
                    }
                    if (rtsp->ctc_begin < begin || rtsp->ctc_begin >= end) {
                        LOG_STRM_WARN("#%d ctc_begin = %d, begin = %d, end = %d\n", rtsp->index, rtsp->ctc_begin, begin, end);
                        rtsp->ctc_begin = begin;
                    }
                    if (rtsp->ctc_begin >= rtsp->ctc_end) {
                        LOG_STRM_WARN("#%d ctc_begin = %d, ctc_end = %d\n", rtsp->index, rtsp->ctc_begin, rtsp->ctc_end);
                        rtsp->ctc_end = end;
                    }

                    rtsp->time_begin = rtsp->ctc_begin;
                    rtsp_clt_time_set_total(rtsp, rtsp->ctc_end - rtsp->ctc_begin);
                }
            }
            if (rtsp->time_current != 0 && rtsp->time_current > rtsp->time_length) {
                rtsp->rtsp_code = RTSP_CODE_INVALID_RANGE;
                LOG_STRM_ERROUT("time_current = %d, time_length = %d\n", rtsp->time_current, rtsp->time_length);
            }
            rtsp->rtsp_code = 0;
            LOG_STRM_DEBUG("#%d time_length = %d\n", rtsp->index, rtsp->time_length);
            goto End;
        }
        LOG_STRM_ERROUT("#%d 'a=range:npt=' not found\n", rtsp->index);
    }

    //直播
	if (0 == rtsp->timeshift_second || timeshift_support) {
        p = ind_stristr(buf, "a=range:clock=");
        if (p) {
            p += 14;
            rtsp_clt_time_range(rtsp, p);
        }
    }

    addrType = 0;
    p = ind_stristr(buf, "c=IN IP4 ");
    if (p) {
        p += 9;
        addrType = atoi(p);
    }

    LOG_STRM_PRINTF("#%d retry_flg = %d, unicast = %d, forbid = %d, addrType = %d\n", rtsp->index, rtsp->retry_flg, rtsp->multicast_unicast, rtsp->multicast_forbid, addrType);

    //重连情况下忽略单播和多播的检查
    if (rtsp->retry_flg == 1 || rtsp->multicast_unicast == 3)
        goto End;

    //单播
    if (rtsp->multicast_forbid == 1 || addrType < 224 || addrType > 239) {
        rtsp->iptvtype = IPTV_TYPE_UNICAST;
        goto End;
    }

    //组播
    len = 0;
    while((p[len] >= '0' && p[len] <= '9') || p[len] == '.')
        len ++;
    if (len < 7 || len > 15)
        LOG_STRM_ERROUT("#%d len = %d\n", rtsp->index, len);
    IND_MEMCPY(ip, p, len);
    ip[len] = 0;
    ind_net_pton(ip, &rtsp->mult_sin);

    p = ind_stristr(buf, "m=video");
    if (p == NULL)
        p = ind_stristr(buf, "m=audio ");
    if (p == NULL)
        LOG_STRM_ERROUT("#%d 'm=audio ' not found\n", rtsp->index);
    rtsp->mult_sin.port = (unsigned short)atoi(p + 8);

    rtsp->iptvtype = IPTV_TYPE_MULTICAST;
    rtsp_clt_iptv_open(rtsp);

    rtsp_clt_close_vod(rtsp);
    rtsp_op_none(rtsp);

    return 0;

End:
    if (rtsp->serv_sin.family == AF_INET) {
        struct sockaddr_in peername, sockname;
        socklen_t size = sizeof(struct sockaddr_in);

        getpeername(rtsp->sock, (struct sockaddr*)&peername, &size);
        getsockname(rtsp->sock, (struct sockaddr*)&sockname, &size);

        rtsp->peername.family = AF_INET;
        rtsp->peername.port = ntohs(peername.sin_port);
        rtsp->peername.in_addr = peername.sin_addr;

        rtsp->sockname.family = AF_INET;
        rtsp->sockname.port = ntohs(sockname.sin_port);
        rtsp->sockname.in_addr = sockname.sin_addr;
#if ENABLE_IPV6
    } else {//IPv6
        struct sockaddr_in6 peername6, sockname6;
        size_t size = sizeof(struct sockaddr_in6);

        getpeername(rtsp->sock, (struct sockaddr*)&peername6, &size);
        getsockname(rtsp->sock, (struct sockaddr*)&sockname6, &size);

        rtsp->peername.family = AF_INET6;
        rtsp->peername.port = ntohs(peername6.sin6_port);
        rtsp->peername.in6_addr = peername6.sin6_addr;

        rtsp->sockname.family = AF_INET6;
        rtsp->sockname.port = ntohs(sockname6.sin6_port);
        rtsp->sockname.in6_addr = sockname6.sin6_addr;
#endif
    }

    if (rtsp->cmdsn == 1)    //已经获取到码流长度，表面打开成功
        rtsp_clt_cmdback(rtsp);

    switch(rtsp->op_server) {
    case RTSP_OP_SERVER_LIVE555:
        LOG_STRM_PRINTF("RTSP_OP_SERVER_LIVE555\n");
        rtsp->cache = CACHE_STATE_UNSPPORT;
        if (rtsp->apptype != APP_TYPE_VOD)
            LOG_STRM_ERROUT("#%d apptype = %d\n", rtsp->index, rtsp->apptype);
        rtsp_op_init_live555(rtsp);
        break;
    case RTSP_OP_SERVER_ZTE:
        LOG_STRM_PRINTF("RTSP_OP_SERVER_ZTE\n");
        rtsp->cache = CACHE_STATE_UNSPPORT;
        rtsp_op_init_zte(rtsp);
        break;
    default:
        LOG_STRM_PRINTF("RTSP_OP_SERVER_HUAWEI\n");
        rtsp_op_init_huawei(rtsp);
    }

    LOG_STRM_PRINTF("#%d standard = %d, servtime_sync = %d, timeshift_support = %d, timeshift_status = %d\n",
        rtsp->index, rtsp->standard, rtsp->servtime_sync, rtsp->timeshift_support, rtsp->timeshift_status);
    if (rtsp->servtime_sync == 1
        || rtsp->apptype == APP_TYPE_VOD
        || (rtsp->standard != RTSP_STANDARD_CTC_SHANGHAI && rtsp->timeshift_second == 1 && rtsp->timeshift_support == 0 && rtsp->timeshift_status == 0))
        rtsp_op_immediate_reg(rtsp, RTSP_OP_SETUP);
    else
        rtsp_op_immediate_reg(rtsp, RTSP_OP_GET_RANGE);

    return 0;
Err:
    rtsp->err_no = RTSP_ERROR_DISCRIPT;
    rtsp_op_open_err(rtsp);
    return -1;
}

int rtsp_op_header(struct RTSP* rtsp, char* method)
{
    int len = 0;

    len += sprintf(rtsp->send_buf + len, "%s %s RTSP/1.0\r\n", method, rtsp->url);
    len += sprintf(rtsp->send_buf + len, "CSeq: %d\r\n", ++rtsp->CSeq);
    if (rtsp->Session[0])
        len += sprintf(rtsp->send_buf + len, "Session: %s\r\n", rtsp->Session);
    len += sprintf(rtsp->send_buf + len, "User-Agent: %s\r\n", rtsp->UserAgent);

    return len;
}

static int rtsp_op_timeshift(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp->rtsp_code = 0;
    if (rtsp->apptype == APP_TYPE_IPTV) {
        if (rtsp->iptvtype == IPTV_TYPE_UNICAST) {
            if (rtsp_op_teardown(rtsp) == 0)
                rtsp->op = RTSP_OP_TIMESHIFT;
        } else {
            if (rtsp->state != STRM_STATE_IPTV)
                LOG_STRM_ERROUT("#%d state = %d\n", rtsp->index, rtsp->state);
            rtsp_op_timeshift_ok(rtsp);
        }
    } else {
        LOG_STRM_ERROUT("#%d apptype = %d\n", rtsp->index, rtsp->apptype);
    }

    return 0;
Err:
    if (rtsp->rtsp_code)//RTSP_CODE_URL_FORMAT_Error
        rtsp_op_open_err(rtsp);
    else
        rtsp_op_none(rtsp);
    return -1;
}

static int rtsp_op_timeshift_ok(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d apptype = %d\n", rtsp->index, rtsp->apptype);

    if (rtsp->apptype == APP_TYPE_IPTV && rtsp->iptvtype == IPTV_TYPE_UNICAST) {
        rtsp_stat_set(rtsp, 0);
        rtsp_clt_close_vod(rtsp);
    }

    if (rtsp->timeshift_status == 0)
        rtsp_clt_parse_url(rtsp, rtsp->channel_url);
    else
        rtsp_clt_parse_url(rtsp, rtsp->tmshift_url);

    rtsp_op_resolve(rtsp, 0);

    return 0;
}

int rtsp_op_teardown(struct RTSP* rtsp)
{
    int len;

    LOG_STRM_PRINTF("#%d cache = %d\n", rtsp->index, rtsp->cache);

    rtsp_clt_post_valid(rtsp, 0);

    if (rtsp->open_play && rtsp->op == RTSP_OP_TEARDOWN) {
        if ((rtsp->apptype == APP_TYPE_VOD && rtsp->index != STREAM_INDEX_ADV) || rtsp->clear_flg) //clear screen
            strm_play_close(rtsp->strm_play, rtsp->index, 1);
    }

    if (rtsp->rtsp_state == RTSP_STATE_TEARDOWN)
        goto End;

    if (CACHE_STATE_ON == rtsp->cache)
        rtsp_op_param_set(rtsp, PARAM_X_FAST_CACHE_OFF);

    len = rtsp_op_header(rtsp, "TEARDOWN");
    len += sprintf(rtsp->send_buf + len, "\r\n");

    if (rtsp_clt_send(rtsp, len))
        goto End;

    rtsp->rtsp_state = RTSP_STATE_TEARDOWN;
    rtsp_op_timeout_reg(rtsp, RTSP_OP_TEARDOWN);

    return 0;
End:
    rtsp_op_teardown_ok(rtsp);
    return 0;
}

int rtsp_op_teardown_ok(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    if (RTSP_MAIN_PLAY(rtsp) && rtsp->state != STRM_STATE_IPTV) {
        rtsp_clt_time_sync(rtsp);
        rtsp->time_start = rtsp->time_current;
    }

    rtsp_clt_close_vod(rtsp);

    if (rtsp->state == STRM_STATE_IPTV && rtsp->iptvtype != IPTV_TYPE_UNICAST)
        rtsp_clt_iptv_close(rtsp);

    if (rtsp->rtsp_state != RTSP_STATE_TEARDOWN)
        rtsp->rtsp_state = RTSP_STATE_TEARDOWN;

    LOG_STRM_PRINTF("#%d STRM_STATE_CLOSE\n", rtsp->index);
    rtsp_clt_state(rtsp, STRM_STATE_CLOSE, 0);

#ifdef INCLUDE_PVR
    if (rtsp->open_shift) {
        strm_record_close(rtsp->strm_record, 0);
        rtsp->open_shift = 0;
    }

#endif
    if (rtsp->open_play)
        rtsp_clt_close_play(rtsp);

    rtsp_op_none(rtsp);
    return 0;
}

static int rtsp_op_burst_up(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    return rtsp_op_burst(rtsp, 1);
}

static int rtsp_op_burst_down(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    return rtsp_op_burst(rtsp, 0);
}

static int rtsp_op_burst(struct RTSP* rtsp, int up)
{
    int len, bytrate = rtsp->stat.stat_bitrate / 8;
    LOG_STRM_PRINTF("#%d bytrate = %d\n", rtsp->index, bytrate);

    if (rtsp->index >= STREAM_INDEX_PIP || rtsp->state != STRM_STATE_PLAY || rtsp->burst_flag != 3)
        LOG_STRM_ERROUT("#%d, state = %d, burst_flag = %d\n", rtsp->index, rtsp->state, rtsp->burst_flag);

    len = rtsp_op_header(rtsp, "PLAY");
    len += sprintf(rtsp->send_buf + len, "Scale: 1\r\n");
    if (rtsp->standard != RTSP_STANDARD_YUXING)
        len += sprintf(rtsp->send_buf + len, "Range: npt=now-\r\n");

    rtsp->burst_clk = rtsp->clk + TIMEOUT_CLK_BURST;

    if (!bytrate)
        bytrate = STREAM_BLOCK_BURST_SIZE;

    if (up) {
        len += sprintf(rtsp->send_buf + len, "x-BurstSize: %d\r\n", bytrate);
        len += sprintf(rtsp->send_buf + len, "Speed: 1.1\r\n");
    } else {
        len += sprintf(rtsp->send_buf + len, "x-BurstSize: -%d\r\n", bytrate);
        len += sprintf(rtsp->send_buf + len, "Speed: 0.9\r\n");
    }

    len += sprintf(rtsp->send_buf + len, "\r\n");

    if (rtsp_clt_send(rtsp, len))
        LOG_STRM_ERROUT("#%d rtsp_clt_send\n", rtsp->index);

    rtsp->burst_clk = rtsp->clk + TIMEOUT_CLK_BURST;

    if(up)
        rtsp_op_timeout_reg(rtsp, RTSP_OP_BURST_UP);
    else
        rtsp_op_timeout_reg(rtsp, RTSP_OP_BURST_DOWN);
    return 0;
Err:
    rtsp_op_none(rtsp);
    return -1;
}

static int rtsp_op_burst_ok(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp_op_none(rtsp);
    return 0;
}

static int rtsp_op_arq_ctc(struct RTSP* rtsp)
{
    int len;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    if(rtsp->arq_ctc_len <= 0)
        LOG_STRM_ERROUT("arq_ctc_len = %d\n", rtsp->arq_ctc_len);

    if(!isprint(rtsp->arq_ctc_seqbuf[0]))
        LOG_STRM_ERROUT("arq_ctc_seqbuf[0] = %d\n", rtsp->arq_ctc_seqbuf[0]);

    LOG_STRM_PRINTF("#%d, op_cseq = %d\n", rtsp->index, rtsp->op_cseq);

    len = rtsp_op_header(rtsp, "GET_PARAMETER");
    len += sprintf(rtsp->send_buf + len, "x-RetransSeq: %s", rtsp->arq_ctc_seqbuf);
    len += sprintf(rtsp->send_buf + len, "\r\n");
    len += sprintf(rtsp->send_buf + len, "\r\n");

    rtsp->arq_clk = rtsp->clk + TIMEOUT_CLK_ARQ;

    if (rtsp_clt_send(rtsp, len))
        LOG_STRM_ERROUT("#%d rtsp_clt_send\n", rtsp->index);

    rtsp_op_timeout_reg(rtsp, RTSP_OP_ARQ);
    return 0;
Err:
    IND_MEMSET(rtsp->arq_ctc_seqbuf, 0, rtsp->arq_ctc_len);
    rtsp->arq_ctc_len = 0;
    rtsp_op_none(rtsp);
    return -1;
}


static int rtsp_op_arq_ctc_ok(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    IND_MEMSET(rtsp->arq_ctc_seqbuf, 0, rtsp->arq_ctc_len);
    rtsp->arq_ctc_len = 0;

    rtsp_op_none(rtsp);
    return 0;
}

static int rtsp_op_arq_ctc_err(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    IND_MEMSET(rtsp->arq_ctc_seqbuf, 0, rtsp->arq_ctc_len);
    rtsp->arq_ctc_len = 0;

    rtsp_op_none(rtsp);
    return 0;
}
