
/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>


#include "rtsp_app.h"

static int rtsp_op_heartbit(struct RTSP* rtsp);
static int rtsp_op_heartbit_ok(struct RTSP* rtsp);

static int rtsp_op_setup(struct RTSP* rtsp);
static int rtsp_op_setup_ok(struct RTSP* rtsp);
static int rtsp_op_setup_err(struct RTSP* rtsp);

static int rtsp_op_play_init(struct RTSP* rtsp);
static int rtsp_op_play_init_ok(struct RTSP* rtsp);

static int rtsp_op_play(struct RTSP* rtsp);
static int rtsp_op_play_ok(struct RTSP* rtsp);

static int rtsp_op_pause(struct RTSP* rtsp);
static int rtsp_op_pause_ok(struct RTSP* rtsp);

static int rtsp_op_resume(struct RTSP* rtsp);

void rtsp_op_init_live555(struct RTSP* rtsp)
{
    struct RTSPOps *ops = rtsp->ops;

    ops[RTSP_OP_HEARTBIT].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_HEARTBIT].op_f = rtsp_op_heartbit;
    ops[RTSP_OP_HEARTBIT].ok_f = rtsp_op_heartbit_ok;
    ops[RTSP_OP_HEARTBIT].err_f = NULL;

    ops[RTSP_OP_SETUP].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_SETUP].op_f = rtsp_op_setup;
    ops[RTSP_OP_SETUP].ok_f = rtsp_op_setup_ok;
    ops[RTSP_OP_SETUP].err_f = rtsp_op_setup_err;

    ops[RTSP_OP_PLAY_INIT].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_PLAY_INIT].op_f = rtsp_op_play_init;
    ops[RTSP_OP_PLAY_INIT].ok_f = rtsp_op_play_init_ok;
    ops[RTSP_OP_PLAY_INIT].err_f = rtsp_op_open_err;

    ops[RTSP_OP_PLAY].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_PLAY].op_f = rtsp_op_play;
    ops[RTSP_OP_PLAY].ok_f = rtsp_op_play_ok;
    ops[RTSP_OP_PLAY].err_f = rtsp_op_open_err;

    ops[RTSP_OP_PAUSE].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_PAUSE].op_f = rtsp_op_pause;
    ops[RTSP_OP_PAUSE].ok_f = rtsp_op_pause_ok;
    ops[RTSP_OP_PAUSE].err_f = rtsp_op_open_err;

    ops[RTSP_OP_RESUME].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_RESUME].op_f = rtsp_op_resume;
    ops[RTSP_OP_RESUME].ok_f = NULL;
    ops[RTSP_OP_RESUME].err_f = NULL;

    rtsp->cache = CACHE_STATE_UNSPPORT;
}

static int rtsp_op_heartbit(struct RTSP* rtsp)
{
    int len;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    len = rtsp_op_header(rtsp, "GET_PARAMETER");
    len += sprintf(rtsp->send_buf + len, "\r\n");

    if (rtsp_clt_send(rtsp, len))
        LOG_STRM_ERROUT("#%d rtsp_clt_send\n", rtsp->index);

    rtsp_op_timeout_reg(rtsp, RTSP_OP_HEARTBIT);

    return 0;
Err:
    rtsp_op_none(rtsp);
    return -1;
}

static int rtsp_op_heartbit_ok(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp_op_none(rtsp);
    return 0;
}

static int rtsp_op_setup(struct RTSP* rtsp)
{
    int len;
    uint32_t port;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    len = 0;

    len += sprintf(rtsp->send_buf + len, "SETUP %s RTSP/1.0\r\n", rtsp->url);
    len += sprintf(rtsp->send_buf + len, "CSeq: %d\r\n", ++rtsp->CSeq);
    len += sprintf(rtsp->send_buf + len, "User-Agent: %s\r\n", rtsp->UserAgent);

    if (rtsp_clt_datasocket(rtsp))
        LOG_STRM_ERROUT("#%d rtsp_data_open\n", rtsp->index);

    port = rtsp->data_port;

    LOG_STRM_PRINTF("#%d transport = %d\n", rtsp->index, rtsp->transport);
    len += sprintf(rtsp->send_buf + len, "Transport: ");
    len += sprintf(rtsp->send_buf + len, "MP2T/RTP/UDP;unicast;destination=%s;client_port=%u-%u,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
    len += sprintf(rtsp->send_buf + len, "MP2T/UDP;unicast;destination=%s;client_port=%u-%u,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
    len += sprintf(rtsp->send_buf + len, "MP2T/TCP;unicast;destination=%s;interleaved=0-1,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
    len += sprintf(rtsp->send_buf + len, "MP2T/RTP/TCP;unicast;destination=%s;interleaved=0-1\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));

    len += sprintf(rtsp->send_buf + len, "\r\n");

    if (rtsp_clt_send(rtsp, len))
        LOG_STRM_ERROUT("#%d rtsp_clt_send\n", rtsp->index);

    rtsp_op_timeout_reg(rtsp, RTSP_OP_SETUP);

    return 0;
Err:
    rtsp_op_open_err(rtsp);
    return -1;
}

static int rtsp_op_setup_ok(struct RTSP* rtsp)
{
    int len;
    char *p, *p1, *p2;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    p = ind_stristr(rtsp->ctrl_buf, "Session: ");
    if (p == NULL)
        LOG_STRM_ERROUT("#%d 'Session: ' not found\n", rtsp->index);
    p += 9;
    p1 = strstr(p, ";");
    p2 = strstr(p, "\r\n");
    if (NULL == p2)
        LOG_STRM_ERROUT("#%d SDP formart\n", rtsp->index);
    if (p1 == NULL || p1 > p2)
        p1 = p2;
    len = p1 - p;
    if (len >= SESSION_LENGTH)
        LOG_STRM_ERROUT("#%d len = %d\n", rtsp->index, len);
    IND_STRNCPY(rtsp->Session, p, len);
    rtsp->Session[len] = '\0';

    {
        p = ind_stristr(rtsp->ctrl_buf, "Transport: ");
        if (p == NULL)
            LOG_STRM_ERROUT("#%d Transport not fond\n", rtsp->index);
        p += 11;
        if (strncmp(p, "RTP/AVP/", 8) == 0) {
            rtsp->rtp_flag = 1;
            p += 8;
        } else if (strncmp(p, "RTP/AVP", 7) == 0) {//live555
            rtsp->rtp_flag = 1;
            p += 7;
        } else if (strncmp(p, "MP2T/RTP/", 9) == 0) {
            rtsp->rtp_flag = 1;
            p += 9;
        } else {
            if (strncmp(p, "MP2T/", 5))
                LOG_STRM_ERROUT("#%d Transport = %s\n", rtsp->index, p);
            p += 5;
            rtsp->rtp_flag = 0;
        }
        if (strncmp(p, "UDP", 3) == 0) {
            LOG_STRM_PRINTF("#%d ########: UDP\n", rtsp->index);
            rtsp->transtype = SOCK_DGRAM;
        } else if (strncmp(p, "TCP", 3) == 0) {
            LOG_STRM_PRINTF("#%d ########: TCP\n", rtsp->index);
            rtsp->transtype = SOCK_STREAM;
            close(rtsp->data_sock);
            rtsp->data_sock = -1;
            rtsp_clt_fdset(rtsp);
        } else if (strncmp(p, ";", 1) == 0) {//live555
            rtsp->transtype = SOCK_DGRAM;
        } else {
            LOG_STRM_ERROUT("#%d Transport = %s\n", rtsp->index, p);
        }
    }
    if (SOCK_DGRAM == rtsp->transtype)
        rtsp->sockname.port = rtsp->data_port;

    {
        if (rtsp->data_sock >= 0) {
            char *transport = ind_stristr(rtsp->ctrl_buf, "Transport: ");

            p = ind_stristr(transport, "source=");
            if (p) {
                uint16_t port = rtsp->peername.port;
                ind_net_pton(p + 7, &rtsp->peername);
                rtsp->peername.port = port;
            }
            p = ind_stristr(transport, "client_port=");
            if (p)
                rtsp->sockname.port = (u_short)ind_atoui(p + 12);
            p = ind_stristr(transport, "server_port=");
            if (p)
                rtsp->peername.port = (u_short)ind_atoui(p + 12);
        }

        LOG_STRM_PRINTF("#%d pearname %s:%hu\n", rtsp->index, rtsp_clt_addr_fmt(rtsp, &rtsp->peername), rtsp->peername.port);
        LOG_STRM_PRINTF("#%d sockname %s:%hu\n", rtsp->index, rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), rtsp->sockname.port);
    }

    rtsp->rtsp_state = RTSP_STATE_PAUSE;

    {
        int interval = rtsp->heartbit_period * 100;
        ind_timer_create(rtsp->tlink, rtsp->clk + (uint32_t)interval, interval, rtsp_clt_heartbit, rtsp);
    }

    rtsp_op_immediate_reg(rtsp, RTSP_OP_PLAY_INIT);

    if (rtsp->recv_safe > 1)
        rtsp->recv_safe = 1;

    return 0;
Err:
    rtsp_op_open_err(rtsp);
    return -1;
}

static int rtsp_op_setup_err(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp->err_no = RTSP_ERROR_SETUP;
    rtsp_op_open_err(rtsp);

    return 0;
}

static int rtsp_op_play_send(struct RTSP* rtsp)
{
    char range[128];
    int len, scale;
    PLAY_TYPE type;
    uint32_t off;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    if (rtsp->mult_flag)
        rtsp_clt_iptv_close(rtsp);

    off = rtsp->op_off;
    type = rtsp->op_type;
    scale = rtsp->op_scale;
    if (rtsp->rtsp_state == RTSP_STATE_TEARDOWN)
        LOG_STRM_ERROUT("#%d RTSP_STATE_TEARDOWN\n", rtsp->index);

    range[0] = 0;
    len = rtsp_op_header(rtsp, "PLAY");
    len += sprintf(rtsp->send_buf + len, "Scale: %d\r\n", scale);

    if (rtsp->recv_safe >= 1 && (scale != 1 || type == PLAY_TYPE_SET))
        rtsp->recv_safe = 2;

    switch(type) {
    case PLAY_TYPE_SET:
        if (off > rtsp->time_length)
            off = rtsp->time_length;
        if (rtsp->time_begin > 0)
            off += rtsp->time_begin;
        sprintf(range, "Range: npt=%d-", off);
        break;
    default:
        break;
    }

    if (range[0] != 0)
        len += sprintf(rtsp->send_buf + len, "%s\r\n", range);

    len += sprintf(rtsp->send_buf + len, "\r\n");

    if (rtsp_clt_send(rtsp, len))
        LOG_STRM_ERROUT("#%d rtsp_clt_send\n", rtsp->index);

    return 0;
Err:
    return -1;
}

static void rtsp_op_retry(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d retry_op = %d, state = %d\n", rtsp->index, rtsp->retry_op, rtsp->state);

    if (rtsp->retry_op == RTSP_OP_NONE) {
        switch(rtsp->state) {
        case STRM_STATE_PLAY:
            LOG_STRM_PRINTF("#%d STRM_STATE_PLAY retry_time = %d\n", rtsp->index, rtsp->retry_time);
            rtsp->op_type = PLAY_TYPE_SET;
            rtsp->op_off = rtsp->retry_time;
            rtsp->op_scale = 1;
            break;
        case STRM_STATE_PAUSE:
            LOG_STRM_PRINTF("#%d STRM_STATE_PAUSE\n", rtsp->index);
            rtsp_op_pause_ok(rtsp);
            return;
        default:
            LOG_STRM_PRINTF("#%d state default!\n", rtsp->index);
            rtsp_op_none(rtsp);
            return;
        }
    } else {
        if (rtsp->retry_op_off == -1)
            rtsp->retry_op_off = rtsp->retry_time;
        if (rtsp->mult_flag)
            rtsp_clt_iptv_close(rtsp);
        switch(rtsp->retry_op) {
        case RTSP_OP_PLAY:
            LOG_STRM_PRINTF("#%d RTSP_OP_PLAY\n", rtsp->index);
            rtsp->op_type = PLAY_TYPE_SET;
            rtsp->op_off = rtsp->retry_op_off;
            rtsp->op_scale = 1;
            break;
        case RTSP_OP_PAUSE:
            LOG_STRM_PRINTF("#%d RTSP_OP_PAUSE\n", rtsp->index);
            rtsp_op_pause(rtsp);
            return;
        default:
            LOG_STRM_PRINTF("#%d DEFAULT\n", rtsp->index);
            rtsp_op_none(rtsp);
            return;
        }
    }

    rtsp->retry_flg = 0;
    rtsp_op_play(rtsp);
}

static int rtsp_op_play_init(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    //处理重连
    if (rtsp->retry_flg == 1) {
        rtsp_op_retry(rtsp);
        return 0;
    }

    rtsp->op_scale = 1;

    if (rtsp->time_current >= rtsp->time_length) {
        LOG_STRM_WARN("#%d sec(%d) >= rtsp->time_length(%d)\n", rtsp->index, rtsp->time_current, rtsp->time_length);
        rtsp_clt_time_set_current(rtsp, 0);
        LOG_STRM_DEBUG("#%d time_current = %d\n", rtsp->index, rtsp->time_current);
    }

    rtsp->op_off = rtsp->time_current;
    rtsp->op_type = PLAY_TYPE_SET;

    if (rtsp_op_play_send(rtsp))
        LOG_STRM_ERROUT("#%d APP_TYPE_VOD play\n", rtsp->index);
    rtsp_op_timeout_reg(rtsp, RTSP_OP_PLAY_INIT);

    return 0;
Err:
    rtsp_op_open_err(rtsp);
    return -1;
}

static int rtsp_op_play_init_ok(struct RTSP* rtsp)
{
    char *p;
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp->rtsp_state = RTSP_STATE_PLAY;
    rtsp->clock = rtsp->clk;

    LOG_STRM_DEBUG("#%d time_length = %d\n", rtsp->index, rtsp->time_length);

    p = ind_stristr(rtsp->ctrl_buf, "Range: npt=");
    if (p) {
        int start;
        p += 11;
        if (sscanf(p, "%d", &start) != 1)
            LOG_STRM_ERROUT("#%d sscanf start!\n", rtsp->index);
        if (rtsp->time_begin > 0)
            start -= rtsp->time_begin;
        if (start < 0) {
            LOG_STRM_WARN("#%d start = %d!\n", rtsp->index, start);
            start = 0;
        }
        p = strchr(p, '-');
        if (p == NULL)
            LOG_STRM_ERROUT("#%d '-' lost!\n", rtsp->index);
        rtsp->time_start = start;
        LOG_STRM_PRINTF("#%d VOD start = %d\n", rtsp->index, rtsp->time_start);
    }

    LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", rtsp->index);
    rtsp_clt_state(rtsp, STRM_STATE_PLAY, 1);

    if (rtsp->open_play)
        rtsp_stat_set(rtsp, 1);

    rtsp_clt_resume(rtsp);

    strm_play_set_idle(rtsp->strm_play, rtsp->index, 0);
    rtsp_op_none(rtsp);

    return 0;
Err:
    rtsp->err_no = RTSP_ERROR_ANALYZE;
    rtsp_op_open_err(rtsp);
    return -1;
}

static int rtsp_op_play(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    if (rtsp->retry_flg == 1) {
        rtsp->op_off = rtsp->retry_time;
        LOG_STRM_PRINTF("#%d PLAY_TYPE_SET\n", rtsp->index);
        rtsp->op_type = PLAY_TYPE_SET;
        rtsp->retry_flg = 0;
    }
    if (rtsp->scale < 0 && rtsp->timeshift_len > 0)
        ind_timer_delete(rtsp->tlink, rtsp_clt_shiftcheck, rtsp);

    if (rtsp_op_play_send(rtsp))
        LOG_STRM_ERROUT("#%d rtsp_clt_send\n", rtsp->index);

    rtsp_op_timeout_reg(rtsp, RTSP_OP_PLAY);

    return 0;
Err:
    rtsp_op_open_err(rtsp);
    return -1;
}

static int rtsp_op_play_ok(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp->retry_op = RTSP_OP_NONE;

    rtsp->rtsp_state = RTSP_STATE_PLAY;

    if (rtsp->op_type != PLAY_TYPE_NOW) {
        char *buf, *p;

        buf = rtsp->ctrl_buf;
        p = ind_stristr(buf, "Range: ");
        if (p) {
            p += 7;
            if (strncmp(p, "npt=", 4) == 0) {
                p += 4;
                if (isdigit(*p)) {
                    int start;
                    sscanf(p, "%d", &start);
                    if (rtsp->time_begin > 0)
                        start -= rtsp->time_begin;
                    if (start < 0)
                        start = 0;
                    rtsp->time_start = start;
                    LOG_STRM_PRINTF("#%d VOD start = %d\n", rtsp->index, rtsp->time_start);
                }
            } else {
                LOG_STRM_WARN("#%d Unknown VOD range!\n", rtsp->index);
            }
        } else {
            if (RTSP_MAIN_PLAY(rtsp)) {
                LOG_STRM_WARN("#%d Range is empty\n", rtsp->index);
                if (rtsp->op_type == PLAY_TYPE_SET) {
                    rtsp->time_start = rtsp->op_off;
                } else {
                    rtsp_clt_time_sync(rtsp);
                    rtsp->time_start = rtsp->time_current;
                }
            }
        }

        rtsp->ts_len = 0;
    }

    if (rtsp->op_type != PLAY_TYPE_NOW)
        rtsp_clt_reset_play(rtsp, 1);

    if (rtsp->open_play)
        rtsp_stat_set(rtsp, 1);
    LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", rtsp->index);
    rtsp_clt_state(rtsp, STRM_STATE_PLAY, 1);

    rtsp_clt_resume(rtsp);

    rtsp_op_none(rtsp);

    rtsp->clock = rtsp->clk;

    if (RTSP_MAIN_PLAY(rtsp)) {
        LOG_STRM_PRINTF("#%d state = %d, apptype = %d\n", rtsp->index, rtsp->state, rtsp->apptype);
        rtsp_clt_time_sync(rtsp);
    }

    if (rtsp->open_play == OPEN_PLAY_END)
        rtsp->open_play = OPEN_PLAY_RUNNING;
    strm_play_set_idle(rtsp->strm_play, rtsp->index, 0);

    return 0;
}

static int rtsp_op_pause(struct RTSP* rtsp)
{
    int len;

    LOG_STRM_PRINTF("#%d state = %d\n", rtsp->index, rtsp->state);

    if (rtsp->scale < 0 && rtsp->timeshift_len > 0)
        ind_timer_delete(rtsp->tlink, rtsp_clt_shiftcheck, rtsp);

    switch(rtsp->state) {
    case STRM_STATE_PLAY:
        rtsp_clt_time_sync(rtsp);
        break;
    default:
        rtsp_op_none(rtsp);
        LOG_STRM_ERROR("#%d rtsp->state = %d\n", rtsp->index, rtsp->state);
        return -1;
    }

    len = rtsp_op_header(rtsp, "PAUSE");
    len += sprintf(rtsp->send_buf + len, "\r\n");

    rtsp->retry_op = RTSP_OP_PAUSE;
    rtsp->retry_op_off = -1;
    if (rtsp_clt_send(rtsp, len))
        LOG_STRM_ERROUT("#%d rtsp_clt_send\n", rtsp->index);

    rtsp_op_timeout_reg(rtsp, RTSP_OP_PAUSE);

    return 0;
Err:
    rtsp_op_open_err(rtsp);
    return -1;
}

static int rtsp_op_pause_ok(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d rtsp->state = %d\n", rtsp->index, rtsp->state);

    rtsp->retry_op = RTSP_OP_NONE;
    rtsp_clt_time_sync(rtsp);

    LOG_STRM_PRINTF("#%d STRM_STATE_PAUSE\n", rtsp->index);
    rtsp_clt_state(rtsp, STRM_STATE_PAUSE, 0);

    rtsp->rtsp_state = RTSP_STATE_PAUSE;
    rtsp_stat_set(rtsp, 0);

    strm_play_pause(rtsp->strm_play, rtsp->index);

    rtsp_op_none(rtsp);
    return 0;
}

static int rtsp_op_resume(struct RTSP* rtsp)
{
    int state;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp->op_off = 0;
    rtsp->op_type = PLAY_TYPE_NOW;

    state = rtsp->state;
    switch(state) {
    case STRM_STATE_PAUSE:
        break;
    default:
        LOG_STRM_ERROUT("#%d rtsp->state = %d\n", rtsp->index, state);
    }

    rtsp->op_scale = 1;

    rtsp->retry_op = RTSP_OP_PLAY;
    rtsp->retry_op_off = -1;
    rtsp_op_play(rtsp);

    return 0;
Err:
    rtsp_op_none(rtsp);
    return -1;
}
