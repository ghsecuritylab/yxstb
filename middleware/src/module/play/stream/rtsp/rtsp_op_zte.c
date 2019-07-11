
/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#include <errno.h>
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
static int rtsp_op_seek(struct RTSP* rtsp);
static int rtsp_op_fast(struct RTSP* rtsp);

static int rtsp_op_stop(struct RTSP* rtsp);
static int rtsp_op_stop_ok(struct RTSP* rtsp);

static int rtsp_op_get_range(struct RTSP* rtsp);
static int rtsp_op_get_range_ok(struct RTSP* rtsp);
static int rtsp_op_get_range_err(struct RTSP* rtsp);

static int rtsp_op_nat(struct RTSP* rtsp);

static int rtsp_op_keeplive(struct RTSP* rtsp);

static int rtsp_op_rtcpapp(struct RTSP* rtsp);
static int rtsp_op_nat_send(int data_sock, int type, int sock, struct sockaddr_in* sin, struct RTSP* rtsp);
static int rtsp_op_rtcpapp_send(int data_sock, char type, int sock, struct sockaddr_in* sin, struct RTSP* rtsp);
static int rtsp_op_rtcpfb_send(int data_sock, char type, int sock, struct sockaddr_in* sin, struct RTSP* rtsp);

void rtsp_op_init_zte(struct RTSP* rtsp)
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

    ops[RTSP_OP_GET_RANGE].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_GET_RANGE].op_f = rtsp_op_get_range;
    ops[RTSP_OP_GET_RANGE].ok_f = rtsp_op_get_range_ok;
    ops[RTSP_OP_GET_RANGE].err_f = rtsp_op_get_range_err;

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

    ops[RTSP_OP_SEEK].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_SEEK].op_f = rtsp_op_seek;
    ops[RTSP_OP_SEEK].ok_f = NULL;
    ops[RTSP_OP_SEEK].err_f = NULL;

    ops[RTSP_OP_FAST].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_FAST].op_f = rtsp_op_fast;
    ops[RTSP_OP_FAST].ok_f = NULL;
    ops[RTSP_OP_FAST].err_f = NULL;

    ops[RTSP_OP_STOP].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_STOP].op_f = rtsp_op_stop;
    ops[RTSP_OP_STOP].ok_f = rtsp_op_stop_ok;
    ops[RTSP_OP_STOP].err_f = rtsp_op_open_err;

    ops[RTSP_OP_NAT].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_NAT].op_f = rtsp_op_nat;
    ops[RTSP_OP_NAT].ok_f = NULL;
    ops[RTSP_OP_NAT].err_f = NULL;

    ops[RTSP_OP_KEEPLIVE].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_KEEPLIVE].op_f = rtsp_op_keeplive;
    ops[RTSP_OP_KEEPLIVE].ok_f = NULL;
    ops[RTSP_OP_KEEPLIVE].err_f = NULL;

    ops[RTSP_OP_RTCPAPP].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_RTCPAPP].op_f = rtsp_op_rtcpapp;
    ops[RTSP_OP_RTCPAPP].ok_f = NULL;
    ops[RTSP_OP_RTCPAPP].err_f = NULL;
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

    if (rtsp->ContentBase[0] != 0 && rtsp->trackID != -1)
        len += sprintf(rtsp->send_buf + len, "SETUP %s/trackID=%d RTSP/1.0\r\n", rtsp->ContentBase, rtsp->trackID);
    else
        len += sprintf(rtsp->send_buf + len, "SETUP %s RTSP/1.0\r\n", rtsp->url);
    len += sprintf(rtsp->send_buf + len, "CSeq: %d\r\n", ++rtsp->CSeq);
    len += sprintf(rtsp->send_buf + len, "User-Agent: %s\r\n", rtsp->UserAgent);

    if (rtsp_clt_datasocket(rtsp))
        LOG_STRM_ERROUT("#%d rtsp_data_open\n", rtsp->index);

	if (rtsp_clt_rtcpsocket(rtsp))
		ERR_OUT("#%d rtcp_data_open\n", rtsp->index);

    port = rtsp->data_port;

    LOG_STRM_PRINTF("#%d transport = %d\n", rtsp->index, rtsp->transport);
    len += sprintf(rtsp->send_buf + len, "Transport: ");
    switch (rtsp->transport) {
    case 0:
        len += sprintf(rtsp->send_buf + len, "MP2T/TCP;unicast;destination=%s;interleaved=0-1,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
        len += sprintf(rtsp->send_buf + len, "MP2T/RTP/TCP;unicast;destination=%s;interleaved=0-1,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
        len += sprintf(rtsp->send_buf + len, "MP2T/RTP/UDP;unicast;destination=%s;client_port=%u-%u,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
        len += sprintf(rtsp->send_buf + len, "MP2T/UDP;unicast;destination=%s;client_port=%u-%u\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
        break;
    case 1:
        len += sprintf(rtsp->send_buf + len, "MP2T/UDP;unicast;destination=%s;client_port=%u-%u,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
        len += sprintf(rtsp->send_buf + len, "MP2T/RTP/UDP;unicast;destination=%s;client_port=%u-%u,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
        len += sprintf(rtsp->send_buf + len, "MP2T/TCP;unicast;destination=%s;interleaved=0-1,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
        len += sprintf(rtsp->send_buf + len, "MP2T/RTP/TCP;unicast;destination=%s;interleaved=0-1\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
        break;
    case 2:
        len += sprintf(rtsp->send_buf + len, "MP2T/RTP/TCP;unicast;destination=%s;interleaved=0-1,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
        len += sprintf(rtsp->send_buf + len, "MP2T/TCP;unicast;destination=%s;interleaved=0-1,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
        len += sprintf(rtsp->send_buf + len, "MP2T/RTP/UDP;unicast;destination=%s;client_port=%u-%u,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
        len += sprintf(rtsp->send_buf + len, "MP2T/UDP;unicast;destination=%s;client_port=%u-%u\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
        break;
    default:
        len += sprintf(rtsp->send_buf + len, "MP2T/RTP/UDP;unicast;destination=%s;client_port=%u-%u,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
        len += sprintf(rtsp->send_buf + len, "MP2T/UDP;unicast;destination=%s;client_port=%u-%u,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
        len += sprintf(rtsp->send_buf + len, "MP2T/TCP;unicast;destination=%s;interleaved=0-1,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
        len += sprintf(rtsp->send_buf + len, "MP2T/RTP/TCP;unicast;destination=%s;interleaved=0-1\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
        break;
    }
//纯中兴    len += sprintf(rtsp->send_buf + len, "Transport: RTP/AVP/TCP;unicast;destination=%s;interleaved=0-1,RTP/AVP/UDP;unicast;client_port=%u-%u\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);

    if (rtsp->nat_flag) {
        if(rtsp->serv_sin.family != AF_INET)
            ERR_OUT("just support IPV4\n");

        len += sprintf(rtsp->send_buf + len, "x-NAT: %s:%d\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), rtsp->data_port);
    }
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
            close(rtsp->rtcp_sock);
            rtsp->rtcp_sock = -1;
            rtsp_clt_fdset(rtsp);
        } else if (strncmp(p, ";", 1) == 0) {//live555
            rtsp->transtype = SOCK_DGRAM;
        } else {
            LOG_STRM_ERROUT("#%d Transport = %s\n", rtsp->index, p);
        }
    }
    {
        int idx, transport;

        if (SOCK_STREAM == rtsp->transtype) {
            if (rtsp->rtp_flag)
                transport = RTSP_TRANSPORT_RTP_TCP;
            else
                transport = RTSP_TRANSPORT_TCP;
        } else {
            if (rtsp->rtp_flag)
                transport = RTSP_TRANSPORT_RTP_UDP;
            else
                transport = RTSP_TRANSPORT_UDP;
        }
        idx = 0;
        if (STREAM_INDEX_PIP == rtsp->index)
            idx = 1;
        int_steam_setTransportProtocol(idx, transport);
    }

    if (rtsp->transtype == SOCK_STREAM) {
        if (rtsp->burst_flag == 3)
            rtsp->burst_flag = 1;
    } else {
        rtsp->sockname.port = rtsp->data_port;
    }

    if (rtsp->transtype == SOCK_STREAM || !rtsp->rtp_flag) {
        if (rtsp->arq_flag >= 2)
            rtsp->arq_flag = 1;
    }

    if (rtsp->transtype == SOCK_STREAM || rtsp->index != STREAM_INDEX_PIP) {
        if (rtsp->arq_flag >= 2 || rtsp->ret_flag >= 2) {
            if (APP_TYPE_VOD == rtsp->apptype && rtsp->index < STREAM_INDEX_PIP)
                int_steam_setToalBufferSize(rtsp->size_play);
            strm_play_resize(rtsp->strm_play, rtsp->index, RTP_BUFFER_LENGTH, rtsp->size_play);
        } else {
            int size;
            
            if (rtsp->burst_flag >= 2)
                size = rtsp->size_cache;
            else
                size = rtsp->size_play;
            if (APP_TYPE_VOD == rtsp->apptype && rtsp->index < STREAM_INDEX_PIP)
                int_steam_setToalBufferSize(size);
            strm_play_resize(rtsp->strm_play, rtsp->index, RTP_BUFFER_LENGTH, size);
        }
    }
    
    if(SOCK_DGRAM == rtsp->transtype && rtsp->nat_flag) {
        p = ind_stristr(rtsp->ctrl_buf, "x-NAT: on");
        if(!p)
            p = ind_stristr(rtsp->ctrl_buf, "x-NAT:on");
        if (p) {
            p = ind_stristr(rtsp->ctrl_buf, "ssrc=");
            if (p) {
                p += 5;
                rtsp->nat_ssrc = ind_atoui(p);
            }
        }
    }

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

    if (SOCK_DGRAM == rtsp->transtype && rtsp->nat_flag == 1) {
        p = ind_stristr(rtsp->ctrl_buf, "x-KeepAliveInterval: ");
        if (p) {
            rtsp->nat_flag = 2;
            p += 21;
            rtsp->nat_heartbitperiod = ind_atoui(p);
        }
    }
    rtsp->rtsp_state = RTSP_STATE_PAUSE;

    {
        int interval = rtsp->heartbit_period * 100;
        ind_timer_create(rtsp->tlink, rtsp->clk + (uint32_t)interval, interval, rtsp_clt_heartbit, rtsp);
    }

    if (rtsp->nat_flag == 2)
        rtsp_op_immediate_reg(rtsp, RTSP_OP_NAT);
    else
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

    if (rtsp->nat_flag == 2)
        rtsp->nat_flag = 1;
    if (rtsp->iptvtype != IPTV_TYPE_UNICAST)
        rtsp_op_stop_ok(rtsp);
    else
        rtsp_op_open_err(rtsp);

    return 0;
}

static int rtsp_op_play_send(struct RTSP* rtsp)
{
    char range[128];
    int l, len, scale;
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
    LOG_STRM_PRINTF("#%d apptype = %d\n", rtsp->index, rtsp->apptype);
    if (rtsp->apptype == APP_TYPE_VOD) {
        if (rtsp->recv_safe >= 1 && (scale != 1 || type == PLAY_TYPE_SET))
            rtsp->recv_safe = 2;
        if (rtsp->ctc_begin >= 0) {//分段播放
            if (type == PLAY_TYPE_SET) {
                if (rtsp->time_length <= 0)
                    LOG_STRM_ERROUT("#%d time_length = %d\n", rtsp->index, rtsp->time_length);
                if (scale < 0) {
                    if (off <= 0)
                        off = 1;
                    sprintf(range, "Range: npt=%d-%d", rtsp->time_begin + off, rtsp->time_begin);
                } else {
                    if (off >= rtsp->time_length)//华为服务器 end-end时会直接播放下一分段
                        off = rtsp->time_length - 1;
                    sprintf(range, "Range: npt=%d-%d", rtsp->time_begin + off, rtsp->time_begin + rtsp->time_length);
                }
            } else {
                if (scale < 0)
                    sprintf(range, "Range: npt=now-%d", rtsp->time_begin);
                else
                    sprintf(range, "Range: npt=now-%d", rtsp->time_begin + rtsp->time_length);
            }
        } else {
            switch(type) {
            case PLAY_TYPE_SET:
                if (off > rtsp->time_length)
                    off = rtsp->time_length;
                if (rtsp->time_begin > 0)
                    off += rtsp->time_begin;
                sprintf(range, "Range: npt=%d-", off);
                break;
            default:
                sprintf(range, "Range: npt=now-");
                break;
            }
        }
    } else {
        uint32_t now, begin;

        now = rtsp_clt_time_server(rtsp);
        begin = now - rtsp->time_length;

        l = 0;
        switch(type) {
        case PLAY_TYPE_SET:
            LOG_STRM_PRINTF("#%d PLAY_TYPE_SET off = %d, begin = %d, now = %d\n", rtsp->index, off, begin, now);
            if (off > now - TIMESHIFT_BOUNDARY_SEC) {
                LOG_STRM_DEBUG("#%d off > now - TIMESHIFT_BOUNDARY_SEC\n", rtsp->index);
                off = now - TIMESHIFT_BOUNDARY_SEC;
            } else if (off < begin + TIMESHIFT_BOUNDARY_SEC) {
                LOG_STRM_DEBUG("#%d off < begin + TIMESHIFT_BOUNDARY_SEC\n", rtsp->index);
                if (scale == 1) {
                    sprintf(range, "Range: npt=beginning-");
                    break;
                }
                off = begin + TIMESHIFT_BOUNDARY_SEC;
            }

            l += sprintf(range, "Range: clock=");
            l += rtsp_clt_time_local(off, range + l);
            l += sprintf(range + l, ".00Z-");
            break;
        case PLAY_TYPE_NOW:
            LOG_STRM_PRINTF("#%d PLAY_TYPE_NOW\n", rtsp->index);
            l += sprintf(range, "Range: npt=now-");
            break;
        case PLAY_TYPE_END:
            LOG_STRM_PRINTF("#%d PLAY_TYPE_END\n", rtsp->index);
            l += sprintf(range, "Range: npt=end-");
            break;
        }
        if (rtsp->ppv_begin && scale < 0) {//PPV节目快退
            l += rtsp_clt_time_local(rtsp->ppv_begin + rtsp->servtime_diff, range + l);
            l += sprintf(range + l, ".00Z");
        }
    }
    LOG_STRM_PRINTF("#%d scale=%d, range=%s\n", rtsp->index, scale, range);
    if (range[0] != 0)
        len += sprintf(rtsp->send_buf + len, "%s\r\n", range);

    if (RTSP_MAIN_PLAY(rtsp) && type == PLAY_TYPE_SET && scale == 1 && rtsp->burst_flag == 3) {
        rtsp->burst_clk = rtsp->clk + TIMEOUT_CLK_BURST;
        len += sprintf(rtsp->send_buf + len, "x-BurstSize: %d\r\n", STREAM_BLOCK_BURST_SIZE);
        len += sprintf(rtsp->send_buf + len, "Speed: 1.1\r\n");
    }

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
        case STRM_STATE_FAST:
            LOG_STRM_PRINTF("#%d STRM_STATE_FAST\n", rtsp->index);
            rtsp->op_type = PLAY_TYPE_SET;
            rtsp->op_off = rtsp->retry_time;
            rtsp->op_scale = rtsp->scale;
            break;
        case STRM_STATE_IPTV:
            LOG_STRM_PRINTF("#%d STRM_STATE_IPTV\n", rtsp->index);
            if (rtsp->iptvtype == IPTV_TYPE_UNICAST)
                rtsp_op_stop(rtsp);
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
        case RTSP_OP_PAUSE:
            LOG_STRM_PRINTF("#%d RTSP_OP_PAUSE\n", rtsp->index);
            rtsp_op_pause(rtsp);
            return;
        case RTSP_OP_SEEK:
            LOG_STRM_PRINTF("#%d RTSP_OP_SEEK\n", rtsp->index);
            rtsp->op_off = rtsp->retry_op_off;
            if (rtsp->state == STRM_STATE_IPTV) {
                rtsp_op_seek(rtsp);
                return;
            }
            rtsp->op_type = PLAY_TYPE_SET;
            rtsp->op_scale = 1;
            break;
        case RTSP_OP_FAST:
            LOG_STRM_PRINTF("#%d RTSP_OP_FAST\n", rtsp->index);
            if (rtsp->state == STRM_STATE_IPTV) {
                rtsp_clt_time_set_current(rtsp, rtsp_clt_time_server(rtsp));
                LOG_STRM_DEBUG("#%d time_current = %d\n", rtsp->index, rtsp->time_current);
                rtsp->time_start = rtsp->time_current;
                rtsp->op_type = PLAY_TYPE_END;
            } else {
                rtsp->op_type = PLAY_TYPE_SET;
            }
            rtsp->op_off = rtsp->retry_op_off;
            rtsp->op_scale = rtsp->retry_op_scale;
            break;
        case RTSP_OP_STOP:
            LOG_STRM_PRINTF("#%d RTSP_OP_STOP\n", rtsp->index);
            rtsp_op_stop(rtsp);
            return;
        case RTSP_OP_PLAY:
            LOG_STRM_PRINTF("#%d RTSP_OP_PLAY\n", rtsp->index);
            rtsp->op_type = PLAY_TYPE_SET;
            rtsp->op_off = rtsp->retry_op_off;
            rtsp->op_scale = 1;
            break;
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

    if (rtsp->apptype == APP_TYPE_VOD) {
        if (rtsp->time_current >= rtsp->time_length) {
            LOG_STRM_WARN("#%d sec(%d) >= rtsp->time_length(%d)\n", rtsp->index, rtsp->time_current, rtsp->time_length);
            rtsp_clt_time_set_current(rtsp, 0);
            LOG_STRM_DEBUG("#%d time_current = %d\n", rtsp->index, rtsp->time_current);
        }

        rtsp->op_off = rtsp->time_current;
        rtsp->op_type = PLAY_TYPE_SET;

    } else {
        if (rtsp->iptvtype != IPTV_TYPE_UNICAST) {
            rtsp_op_none(rtsp);
            return 0;
        }
        rtsp->op_type = PLAY_TYPE_END;
    }
    if (rtsp_op_play_send(rtsp))
        LOG_STRM_ERROUT("#%d APP_TYPE_VOD play\n", rtsp->index);
    rtsp_op_timeout_reg(rtsp, RTSP_OP_PLAY_INIT);

    return 0;
Err:
    rtsp_op_open_err(rtsp);
    return -1;
}

static void rtsp_op_rtcp_open(struct RTSP* rtsp)
{
    if(rtsp->arq_flag < 2 && rtsp->rtp_flag && rtsp->transtype == SOCK_DGRAM) {
        if (rtsp->rtcp_flag == 1) {
            rtsp->rtcp_flag = 2;
            rtsp_clt_rtcp_open(rtsp);
            ind_timer_create(rtsp->tlink, 100, 100, rtsp_clt_rtcpapp, rtsp);
        } else {
            if (3 == rtsp->rtcp_flag)
                rtsp_clt_rtcp_close(rtsp);
            rtsp_clt_rtcp_open(rtsp);
        }
    }
}

static int rtsp_op_play_init_ok(struct RTSP* rtsp)
{
    char *p;
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp->rtsp_state = RTSP_STATE_PLAY;
    rtsp->clock = rtsp->clk;

    if (rtsp->apptype == APP_TYPE_VOD) {
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
    }
    if (rtsp->apptype == APP_TYPE_VOD) {
        LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", rtsp->index);
        rtsp_clt_state(rtsp, STRM_STATE_PLAY, 1);

        if (rtsp->open_play) {
            rtsp_stat_set(rtsp, 1);
            strm_play_resume(rtsp->strm_play, rtsp->index, 0);
        }
    } else {
        if (rtsp->iptvtype == IPTV_TYPE_UNICAST) {
            LOG_STRM_PRINTF("#%d STRM_STATE_IPTV\n", rtsp->index);
            rtsp_clt_state(rtsp, STRM_STATE_IPTV, 1);
            if (rtsp->open_play) {
                rtsp_stat_set(rtsp, 1);
                strm_play_resume(rtsp->strm_play, rtsp->index, 0);
            }
        } else {
            LOG_STRM_WARN("#%d rtsp->apptype = %d\n", rtsp->index, rtsp->apptype);
        }
    }

    p = ind_stristr(rtsp->ctrl_buf, "RTP-Info: url=");
    if (p) {
        p += 14;
        LOG_STRM_PRINTF("RTP-Info: url=[%s]\n", p);
        struct sockaddr_in sin;
        if (ind_net_pton(p + 7, (struct ind_sin*)&sin) > 0) {
            rtsp->playname.in_addr = sin.sin_addr;
            rtsp->playname.port = sin.sin_port;
            LOG_STRM_PRINTF("#%d rtsp->playname.in_addr = %p, rtsp->playname.port = %p\n",
                rtsp->index, rtsp->playname.in_addr, rtsp->playname.port);
        }
    }
    rtsp_clt_fcc_reset(rtsp);

    if (RTSP_MAIN_PLAY(rtsp)) {
        LOG_STRM_PRINTF("#%d arq_flag = %d\n", rtsp->index, rtsp->arq_flag);
        if (rtsp->arq_flag >= 2) {
            rtsp_clt_arq_open(rtsp);
        }
    }

    rtsp_clt_post_valid(rtsp, 1);
    ind_timer_create(rtsp->tlink, 0, 700, rtsp_clt_natheartbit, rtsp);
    rtsp_op_rtcp_open(rtsp);

    if (rtsp->open_play)
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
    char *p;
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp->retry_op = RTSP_OP_NONE;

    if (abs(rtsp->scale) > 1 && rtsp->op_scale == 1) {
        LOG_STRM_PRINTF("#%d PLAY_TYPE_SET\n", rtsp->index);
        rtsp->op_type = PLAY_TYPE_SET;//重新获取基准时间
    }

    rtsp->rtsp_state = RTSP_STATE_PLAY;

    if (rtsp->op_type != PLAY_TYPE_NOW || abs(rtsp->op_scale) > 1) {
        char *buf;

        buf = rtsp->ctrl_buf;
        p = ind_stristr(buf, "Range: ");
        if (p) {
            p += 7;
            if (rtsp->apptype == APP_TYPE_VOD) {
                if (strncmp(p, "npt=", 4) == 0 && isdigit(*(p + 4))) {
                    int start;
                    sscanf(p + 4, "%d", &start);
                    if (rtsp->time_begin > 0)
                        start -= rtsp->time_begin;
                    if (start < 0)
                        start = 0;
                    rtsp->time_start = start;
                    LOG_STRM_PRINTF("#%d VOD start = %d\n", rtsp->index, rtsp->time_start);
                } else {
                    rtsp->time_start = rtsp->time_current;
                    LOG_STRM_WARN("#%d Unknown VOD range! current = %d\n", rtsp->index, rtsp->time_current);
                }
            } else {
                if (strncmp(p, "clock=", 6) == 0) {
                    unsigned int t;
                    p += 6;
                    t = rtsp_clt_time_make(p);
                    if (t == 0)
                        LOG_STRM_WARN("#%d Unknown clock range!\n", rtsp->index);
                    else
                        rtsp->time_start = t;
                    LOG_STRM_PRINTF("#%d IPTV start = %s\n", rtsp->index, rtsp_clt_time_fmt(rtsp, 0, rtsp->time_start));
                } else if (strncmp(p, "npt=", 4) == 0) {
                    p += 4;
                    if (strncmp(p, "beginning", 9) == 0)
                        rtsp->time_start = rtsp_clt_time_server(rtsp) - rtsp->time_length;
                    else if (strncmp(p, "end", 3) == 0)
                        rtsp->time_start = rtsp_clt_time_server(rtsp);
                    else
                        LOG_STRM_WARN("#%d Unknown ntp range!\n", rtsp->index);
                } else {
                    LOG_STRM_WARN("#%d IPTV Unknown range!\n", rtsp->index);
                }
            }
        } else {
            if (RTSP_MAIN_PLAY(rtsp)) {
                LOG_STRM_WARN("#%d Range is empty! op_type = %d, op_off = %d\n", rtsp->index, rtsp->op_type, rtsp->op_off);
                if (rtsp->op_type == PLAY_TYPE_SET) {
                    rtsp->time_start = rtsp->op_off;
                    if (rtsp->apptype != APP_TYPE_VOD) {
                        uint32_t begin = rtsp_clt_time_server(rtsp) - rtsp->time_length + TIMESHIFT_BOUNDARY_SEC;
                        if (rtsp->time_start < begin)
                            rtsp->time_start = begin;
                    } else {
                        rtsp->time_start = rtsp->op_off;//解决ZTE快进快退没有Range字段时，进度条变0的错误
                    }
                } else {
                    if (rtsp->apptype != APP_TYPE_VOD && rtsp->op_type == PLAY_TYPE_END) {
                        rtsp->time_start = rtsp_clt_time_server(rtsp) - TIMESHIFT_BOUNDARY_SEC;
                    } else {
                        rtsp_clt_time_sync(rtsp);
                        rtsp->time_start = rtsp->time_current;
                    }
                }
            }
        }

        rtsp->ts_len = 0;
    }
    p = ind_stristr(rtsp->ctrl_buf, "RTP-Info: url=");
    if (p) {
        p += 14;
        LOG_STRM_PRINTF("RTP-Info: url=[%s]\n", p);
        struct sockaddr_in sin;
        if (ind_net_pton(p + 7, (struct ind_sin*)&sin) > 0) {
            rtsp->playname.in_addr = sin.sin_addr;
            rtsp->playname.port = sin.sin_port;
            LOG_STRM_PRINTF("#%d rtsp->playname.in_addr = %p, rtsp->playname.port = %p\n",
                rtsp->index, rtsp->playname.in_addr, rtsp->playname.port);
        }
    }

    if (abs(rtsp->op_scale) > 1 || rtsp->op_type != PLAY_TYPE_NOW) {
        if (rtsp->open_play)
            rtsp_clt_reset_play(rtsp, 1);
    }

    if (abs(rtsp->op_scale) > 1) {
        strm_play_tplay(rtsp->strm_play, rtsp->index, rtsp->op_scale);
        rtsp_stat_set(rtsp, 0);
        LOG_STRM_PRINTF("#%d STRM_STATE_FAST\n", rtsp->index);
        rtsp_clt_state(rtsp, STRM_STATE_FAST, rtsp->op_scale);
        rtsp_op_none(rtsp);
        if (rtsp->op_scale < 0 && rtsp->timeshift_len > 0 && rtsp->timeshift_len < rtsp->timeshift_real)
            ind_timer_create(rtsp->tlink, rtsp->clk + INTERVAL_CLK_SHIFTCHK, INTERVAL_CLK_SHIFTCHK, rtsp_clt_shiftcheck, rtsp);
    } else {
        if (rtsp->open_play) {
            rtsp_stat_set(rtsp, 1);
            strm_play_resume(rtsp->strm_play, rtsp->index, 0);
        }
        LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", rtsp->index);
        rtsp_clt_state(rtsp, STRM_STATE_PLAY, 1);

        rtsp_op_none(rtsp);
    }

    rtsp->clock = rtsp->clk;

    if (RTSP_MAIN_PLAY(rtsp)) {
        LOG_STRM_PRINTF("#%d state = %d, apptype = %d\n", rtsp->index, rtsp->state, rtsp->apptype);

        if(rtsp->state == STRM_STATE_PLAY
            || (rtsp->state == STRM_STATE_IPTV && rtsp->iptvtype == IPTV_TYPE_UNICAST)) {
            LOG_STRM_PRINTF("#%d arq_flag = %d\n", rtsp->index, rtsp->arq_flag);
            if (rtsp->arq_flag >= 2) {
                rtsp_clt_arq_open(rtsp);
            }
        }

        if (STRM_STATE_PLAY == rtsp->state && PLAY_TYPE_SET == rtsp->op_type)
             rtsp_op_rtcp_open(rtsp);

        rtsp_clt_time_sync(rtsp);
    }

    if (rtsp->open_play == OPEN_PLAY_END)
        rtsp->open_play = OPEN_PLAY_RUNNING;

    {
        RTSPStat_t stat = &rtsp->stat;

        if (stat->stat_vodplay) {
            int ms = (int)(rtsp->clk - stat->stat_vodplay) * 10;
            stat->stat_vodpause = 0;

            if (rtsp->op_scale < 0) {
                stream_port_post_vodbackward(ms);
                LOG_STRM_DEBUG("#%d vodbackward = %d\n", rtsp->index, ms);
            } else if (rtsp->op_scale > 1) {
                stream_port_post_vodforward(ms);
                LOG_STRM_DEBUG("#%d vodforward = %d\n", rtsp->index, ms);
            } else {
                stream_port_post_vodresume(ms);
                LOG_STRM_DEBUG("#%d vodresume = %d\n", rtsp->index, ms);
            }
        }
    }

    if (rtsp->open_play)
        strm_play_set_idle(rtsp->strm_play, rtsp->index, 0);
    rtsp_clt_post_valid(rtsp, 1);

    rtsp_clt_natheartbit(rtsp);

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
    case STRM_STATE_FAST:
        rtsp_clt_time_sync(rtsp);
        break;
    case STRM_STATE_IPTV:
        rtsp_clt_time_set_current(rtsp, rtsp_clt_time_server(rtsp));
        LOG_STRM_DEBUG("#%d time_current = %d\n", rtsp->index, rtsp->time_current);
        rtsp->time_start = rtsp->time_current;
        break;
    default:
        rtsp_op_none(rtsp);
        LOG_STRM_ERROR("#%d rtsp->state = %d\n", rtsp->index, rtsp->state);
        return -1;
    }

    rtsp->stat.stat_vodpause = rtsp->clk;

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

    rtsp->iptv_pause = 0;
    switch(rtsp->state) {
    case STRM_STATE_FAST:
        rtsp_clt_time_sync(rtsp);
        rtsp->time_start = rtsp->time_current;
        rtsp_clt_reset_play(rtsp, 1);
        break;
    case STRM_STATE_IPTV:
        rtsp->iptv_pause = 1;
        if (IPTV_TYPE_UNICAST != rtsp->iptvtype || 0 != rtsp->timeshift_support)
            rtsp_clt_reset_play(rtsp, 1);
        rtsp->retry_flg = 0;
        rtsp_clt_time_set_current(rtsp, rtsp_clt_time_server(rtsp));
        LOG_STRM_DEBUG("#%d time_current = %d\n", rtsp->index, rtsp->time_current);
        rtsp->time_start = rtsp->time_current;
        break;
    default:
        rtsp_clt_time_sync(rtsp);
        break;
    }

    LOG_STRM_PRINTF("#%d STRM_STATE_PAUSE\n", rtsp->index);
    rtsp_clt_state(rtsp, STRM_STATE_PAUSE, 0);

    rtsp->rtsp_state = RTSP_STATE_PAUSE;
    rtsp_stat_set(rtsp, 0);

    strm_play_pause(rtsp->strm_play, rtsp->index);

    {
        RTSPStat_t stat = &rtsp->stat;
        if (stat->stat_vodpause) {
            int ms = (int)(rtsp->clk - stat->stat_vodpause) * 10;

            stream_port_post_vodpause(ms);

            stat->stat_vodpause = 0;
            LOG_STRM_DEBUG("#%d vodpause = %d\n", rtsp->index, ms);
        }
    }

    rtsp_clt_post_valid(rtsp, 1);
    rtsp_op_none(rtsp);
    return 0;
}

static int rtsp_op_resume(struct RTSP* rtsp)
{
    int state;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    state = rtsp->state;
    switch(state) {
    case STRM_STATE_FAST:
        rtsp_clt_time_sync(rtsp);
        rtsp->op_off = rtsp->time_current;
        rtsp->op_type = PLAY_TYPE_SET;
        break;
    case STRM_STATE_PAUSE:
        rtsp->op_off = 0;
        rtsp->op_type = PLAY_TYPE_NOW;

        if (rtsp->iptv_pause)
            rtsp_op_rtcp_open(rtsp);

        if (rtsp->apptype == APP_TYPE_IPTV) {
            uint32_t begin, end, current;

            end = rtsp_clt_time_server(rtsp);
            begin = end - rtsp->time_length + TIMESHIFT_BOUNDARY_SEC;
            end -= TIMESHIFT_BOUNDARY_SEC;
            current = rtsp->time_current;
            LOG_STRM_DEBUG("#%d: begin = %s current = %s\n", rtsp->index, rtsp_clt_time_fmt(rtsp, 0, begin), rtsp_clt_time_fmt(rtsp, 1, current));

            if (current <= begin) {
                LOG_STRM_PRINTF("#%d PLAY_TYPE_SET\n", rtsp->index);
                rtsp->op_type = PLAY_TYPE_SET;
            } else if (current >= end) {//避免暂停立即转播放出错
                LOG_STRM_PRINTF("#%d PLAY_TYPE_SET current = %d / %d\n", rtsp->index, current, end);
                rtsp->op_off = end;
                rtsp->op_type = PLAY_TYPE_SET;
            }
        }
        break;
    default:
        LOG_STRM_ERROUT("#%d rtsp->state = %d\n", rtsp->index, rtsp->state);
    }

    rtsp->stat.stat_vodplay = rtsp->clk;

    rtsp->op_scale = 1;

    rtsp->retry_op = RTSP_OP_PLAY;
    rtsp->retry_op_off = rtsp->op_off;
    rtsp_op_play(rtsp);

    return 0;
Err:
    rtsp_op_none(rtsp);
    return -1;
}

static int rtsp_op_seek(struct RTSP* rtsp)
{
    uint32_t begin, end;
    LOG_STRM_PRINTF("#%d, off = %d\n", rtsp->index, rtsp->op_off);

    rtsp->iptv_pause = 0;

    if (rtsp->apptype == APP_TYPE_IPTV) {
        end = rtsp_clt_time_server(rtsp);
        begin = end - rtsp->time_length;
    } else {
        end = rtsp->time_length;
        begin = 0;
    }

    rtsp->seek_end = SEEK_END_NONE;

    if (rtsp->apptype == APP_TYPE_VOD) {
        if (rtsp->op_off <= begin + 1)
            rtsp_post_msg(rtsp, STRM_MSG_SEEK_BEGIN, 0);
        else if (rtsp->op_off >= end)
            rtsp->seek_end = SEEK_END_NORMAL;
    } else {
        if (rtsp->op_off <= begin + TIMESHIFT_BOUNDARY_SEC) {
            LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_BEGIN!\n", rtsp->index);
            rtsp_post_msg(rtsp, STRM_MSG_STREAM_BEGIN, 0);
        } else if (rtsp->op_off >= end - TIMESHIFT_BOUNDARY_SEC) {
            LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END!\n", rtsp->index);
            rtsp_post_msg(rtsp, STRM_MSG_STREAM_END, 0);
        }
    }

    switch(rtsp->state) {
    case STRM_STATE_PLAY:
    case STRM_STATE_FAST:
        rtsp_clt_time_sync(rtsp);
        break;
    case STRM_STATE_PAUSE:    break;
    case STRM_STATE_IPTV:
        rtsp_clt_time_set_current(rtsp, rtsp_clt_time_server(rtsp));
        LOG_STRM_DEBUG("#%d time_current = %d\n", rtsp->index, rtsp->time_current);
        rtsp->time_start = rtsp->time_current;
        break;
    default:
        LOG_STRM_ERROUT("#%d rtsp->state=%d\n", rtsp->index, rtsp->state);
    }

    rtsp->op_type = PLAY_TYPE_SET;
    rtsp->op_scale = 1;

    rtsp->retry_flg = 0;
    rtsp->retry_op = RTSP_OP_SEEK;
    rtsp->retry_op_off = rtsp->op_off;
    rtsp_op_play(rtsp);
    return 0;
Err:
    rtsp_op_none(rtsp);
    return -1;
}

static int rtsp_op_fast(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp->iptv_pause = 0;

    rtsp->op_type = PLAY_TYPE_SET;//中兴服务器PLAY响应有时不返回播放时间
    rtsp_clt_time_sync(rtsp);
    switch(rtsp->state) {
    case STRM_STATE_PLAY:
        break;
    case STRM_STATE_PAUSE:
        if (rtsp->apptype == APP_TYPE_IPTV
            && rtsp->time_current <= rtsp_clt_time_server(rtsp) - rtsp->time_length + TIMESHIFT_BOUNDARY_SEC) {
                if (rtsp->op_scale < 0) {
                    rtsp_msg_back(rtsp, STRM_MSG_STREAM_BEGIN, 0);
                    rtsp_op_none(rtsp);
                    return 0;
                }
                rtsp->op_off = 0;
                rtsp->op_type = PLAY_TYPE_SET;
        }
        break;
    case STRM_STATE_FAST:
        break;
    case STRM_STATE_IPTV:
        rtsp_clt_time_set_current(rtsp, rtsp_clt_time_server(rtsp));
        LOG_STRM_DEBUG("#%d time_current = %d\n", rtsp->index, rtsp->time_current);
        rtsp->time_start = rtsp->time_current;
        rtsp->op_type = PLAY_TYPE_END;
        break;
    default:
        LOG_STRM_ERROUT("#%d rtsp->state = %d\n", rtsp->index, rtsp->state);
    }

    rtsp->op_off = rtsp->time_current;

    rtsp->stat.stat_vodplay = rtsp->clk;

    rtsp->retry_op = RTSP_OP_FAST;
    rtsp->retry_op_off = rtsp->time_current;
    rtsp->retry_op_scale = rtsp->op_scale;
    rtsp_op_play(rtsp);
    return 0;
Err:
    rtsp_op_none(rtsp);
    return -1;
}

static int rtsp_op_stop(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp->iptv_pause = 0;

    rtsp->retry_op = RTSP_OP_STOP;
    rtsp->retry_op_off = 0;

    if (rtsp->open_play)
        rtsp_stat_set(rtsp, 0);

    if (rtsp->apptype == APP_TYPE_VOD) {

        rtsp->retry_flg = 0;
        if (rtsp_op_pause(rtsp) == 0)
            rtsp->op = RTSP_OP_STOP;

    } else {
        if (rtsp->iptvtype == IPTV_TYPE_UNICAST) {
            rtsp->op_type = PLAY_TYPE_END;
            rtsp->op_scale = 1;

            rtsp->retry_flg = 0;
            if (rtsp_op_play(rtsp))
                LOG_STRM_ERROUT("#%d rtsp_op_play\n", rtsp->index);
            rtsp->op = RTSP_OP_STOP;
        } else {
            rtsp_op_teardown(rtsp);
            rtsp->op = RTSP_OP_STOP;
        }
    }

    return 0;
Err:
    rtsp_op_none(rtsp);
    return -1;
}

static int rtsp_op_stop_ok(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    char *p = ind_stristr(rtsp->ctrl_buf, "RTP-Info: url=");
    if (p) {
        p += 14;
        LOG_STRM_PRINTF("RTP-Info: url=[%s]\n", p);
        struct sockaddr_in sin;
        if (ind_net_pton(p + 7, (struct ind_sin*)&sin) > 0) {
            rtsp->playname.in_addr = sin.sin_addr;
            rtsp->playname.port = sin.sin_port;
            LOG_STRM_PRINTF("rtsp->playname.port=[%p] ,rtsp->playname.in_addr=[%p]\n",
                rtsp->playname.port ,rtsp->playname.in_addr);
        }
    }
    rtsp_clt_natheartbit(rtsp);

    rtsp->retry_op = RTSP_OP_NONE;

    if (rtsp->open_play)
        rtsp->open_play = OPEN_PLAY_RUNNING;

    if (rtsp->iptvtype == IPTV_TYPE_UNICAST) {
        if (rtsp->open_play) {
            rtsp_stat_set(rtsp, 1);
            if (STREAM_INDEX_PIP != rtsp->index) {
                rtsp_clt_reset_play(rtsp, 1);
                strm_play_resume(rtsp->strm_play, rtsp->index, 0);
            }
        }
        LOG_STRM_PRINTF("#%d STRM_STATE_IPTV\n", rtsp->index);
        rtsp_clt_state(rtsp, STRM_STATE_IPTV, 1);

        rtsp->rtsp_state = RTSP_STATE_PLAY;
    } else {
        rtsp_clt_close_vod(rtsp);

        if (0 == rtsp->mult_flag)
            rtsp_clt_iptv_open(rtsp);
    }

    if (rtsp->arq_flag >= 2)
        rtsp_clt_arq_close(rtsp);

    if (rtsp->rtcp_flag >= 2)
        rtsp_clt_rtcp_close(rtsp);

    rtsp_op_none(rtsp);

    rtsp_clt_post_valid(rtsp, 1);
    return 0;
}

static int rtsp_op_get_range(struct RTSP* rtsp)
{
    int    len;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    len = rtsp_op_header(rtsp, "GET_PARAMETER");
    len += sprintf(rtsp->send_buf + len, "x-Timeshift_Range\r\n\r\n");

    if (rtsp_clt_send(rtsp, len))
        LOG_STRM_ERROUT("#%d rtsp_clt_send\n", rtsp->index);

    rtsp_op_timeout_reg(rtsp, RTSP_OP_GET_RANGE);

    return 0;
Err:
    rtsp_op_none(rtsp);
    rtsp_op_get_range_err(rtsp);
    return -1;
}

static int rtsp_op_get_range_ok(struct RTSP* rtsp)
{
    char *p;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    p = ind_stristr(rtsp->ctrl_buf, "x-Timeshift_Range: clock=");
    if (p == NULL)
        LOG_STRM_ERROUT("#%d 'x-Timeshift_Range' not found\n", rtsp->index);
    p += 25;

    rtsp_clt_time_range(rtsp, p);

    if (rtsp->iptvtype == IPTV_TYPE_MULTICAST && rtsp->state == STRM_STATE_IPTV && rtsp->retry_flg == 0) {
        rtsp_clt_close_vod(rtsp);
        rtsp_op_none(rtsp);
    } else {
        rtsp_op_immediate_reg(rtsp, RTSP_OP_SETUP);
    }
    return 0;
Err:
    rtsp_op_get_range_err(rtsp);
    return -1;
}

static int rtsp_op_get_range_err(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp_op_immediate_reg(rtsp, RTSP_OP_SETUP);
    return 0;
}

static int rtsp_op_nat(struct RTSP* rtsp)
{
    struct sockaddr_in serv_sin;

    PRINTF("#%d\n", rtsp->index);

    serv_sin.sin_family = AF_INET;
    serv_sin.sin_port = htons(rtsp->peername.port);
    serv_sin.sin_addr = rtsp->peername.in_addr;

    if (rtsp->data_sock < 0)
        ERR_OUT("#%d socket errno = %d! %s\n", rtsp->index, errno, strerror(errno));

    if (rtsp->rtcp_sock < 0)
        ERR_OUT("#%d socket errno = %d! %s\n", rtsp->index, errno, strerror(errno));

    rtsp_op_nat_send(rtsp->data_sock, 0, rtsp->sock, &serv_sin, rtsp);
    rtsp_op_nat_send(rtsp->rtcp_sock, 1, rtsp->sock, &serv_sin, rtsp);//rtcp sock data sock port + 1

    rtsp_op_immediate_reg(rtsp, RTSP_OP_PLAY_INIT);

    return 0;
Err:
    rtsp_op_open_err(rtsp);
    return -1;
}

static int rtsp_op_keeplive(struct RTSP* rtsp)
{
    struct sockaddr_in serv_sin;
    PRINTF("#%d\n", rtsp->index);

    serv_sin.sin_family = AF_INET;
    serv_sin.sin_port = htons(rtsp->playname.port);
    serv_sin.sin_addr = rtsp->playname.in_addr;

    if (rtsp->data_sock < 0)
        ERR_OUT("#%d socket errno = %d! %s\n", rtsp->index, errno, strerror(errno));

    if (rtsp->rtcp_sock < 0)
        ERR_OUT("#%d socket errno = %d! %s\n", rtsp->index, errno, strerror(errno));

    rtsp_op_nat_send(rtsp->data_sock, 0, rtsp->sock, &serv_sin, rtsp);
    rtsp_op_nat_send(rtsp->rtcp_sock, 1, rtsp->sock, &serv_sin, rtsp);//rtcp sock data sock port + 1

    rtsp_op_none(rtsp);
    return 0;
Err:
    rtsp_op_none(rtsp);
    return -1;
}

int rtsp_op_rtcpfb(struct RTSP* rtsp)
{
    struct sockaddr_in serv_sin;
    PRINTF("#%d\n", rtsp->index);

    serv_sin.sin_family = AF_INET;
    serv_sin.sin_port = htons(rtsp->playname.port);
    serv_sin.sin_addr = rtsp->playname.in_addr;

    if (rtsp->rtcp_sock < 0)
        ERR_OUT("#%d socket errno = %d! %s\n", rtsp->index, errno, strerror(errno));

    if(rtsp->rtcp_lostseq) {
        rtsp_op_rtcpfb_send(rtsp->rtcp_sock, 0, rtsp->sock, &serv_sin, rtsp);        
    }

    rtsp_op_none(rtsp);
    return 0;
Err:
    rtsp_op_none(rtsp);
    return -1;
}

static int rtsp_op_rtcpapp(struct RTSP* rtsp)
{
    struct sockaddr_in serv_sin;
    PRINTF("#%d\n", rtsp->index);

    serv_sin.sin_family = AF_INET;
    serv_sin.sin_port = htons(rtsp->playname.port);
    serv_sin.sin_addr = rtsp->playname.in_addr;

    if (rtsp->rtcp_sock < 0)
        ERR_OUT("#%d socket errno = %d! %s\n", rtsp->index, errno, strerror(errno));

    rtsp_op_rtcpapp_send(rtsp->rtcp_sock, 0, rtsp->sock, &serv_sin, rtsp);

    rtsp_op_none(rtsp);
    return 0;
Err:
    rtsp_op_none(rtsp);
    return -1;
}

static int rtsp_op_nat_send(int data_sock, int type, int sock, struct sockaddr_in* sin, struct RTSP* rtsp)
{
    struct natpkt_zte pkt;
    struct sockaddr_in server_addr;
    struct sockaddr_in sock_sin;
    socklen_t size = sizeof(struct sockaddr_in);

	memset(&pkt, 0, sizeof(struct natpkt_zte));

    pkt.stbtype[0] = 0x5a;
    pkt.stbtype[1] = 0x58;
    pkt.stbtype[2] = 0x56;
    pkt.stbtype[3] = 0x31;
    pkt.stbtype[4] = 0x30;
    pkt.stbtype[5] = 0x53;
    pkt.stbtype[6] = 0x54;
    pkt.stbtype[7] = 0x42;

    pkt.session = htonl(atoi(rtsp->Session));

    memset(&sock_sin, 0, sizeof(sock_sin));
    getsockname(data_sock, (struct sockaddr*)&sock_sin, &size);
	pkt.data_port = sock_sin.sin_port;

    memset(&sock_sin, 0, sizeof(sock_sin));
    getsockname(sock, (struct sockaddr*)&sock_sin, &size);
	pkt.data_addr = sock_sin.sin_addr.s_addr;
  	pkt.cmd_port = sock_sin.sin_port;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
    if(type == 1)
    	server_addr.sin_port = sin->sin_port + htons(1);
    else
        server_addr.sin_port = sin->sin_port;
	server_addr.sin_addr.s_addr = sin->sin_addr.s_addr;

	ind_net_sendto(data_sock, &pkt, sizeof(pkt), 0, &server_addr);
	return 0;
}

static int rtsp_op_rtcpapp_send(int data_sock, char type, int sock, struct sockaddr_in* sin, struct RTSP* rtsp)
{
    struct rtcppkt_app pkt;
    struct sockaddr_in server_addr;

	memset(&pkt, 0, sizeof(struct rtcppkt_app));

    pkt.version = 0x80;
    pkt.subtype = 0xcc;
    pkt.len = htons((unsigned short)(sizeof(pkt)/4 - 1));

    memcpy(pkt.name, "PSS0", 4);

    pkt.ssrc = pkt.ssrc_app = htonl(rtsp->rtcp_ssrc);
    pkt.delay = 0x00;
    pkt.nsn = htons((unsigned short)rtsp->rtp_seq);
    pkt.reserved = 0x00;
    pkt.fbs = 0x00;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = sin->sin_port + htons(1);
	server_addr.sin_addr.s_addr = sin->sin_addr.s_addr;

	ind_net_sendto(data_sock, &pkt, sizeof(pkt), 0, &server_addr);
	return 0;
}

static int rtsp_op_rtcpfb_send(int data_sock, char type, int sock, struct sockaddr_in* sin, struct RTSP* rtsp)
{
    char hostname[40] = {0};
    struct rtcppkt_lost pkt;
    struct sockaddr_in server_addr;
    socklen_t size = sizeof(struct sockaddr_in);

	memset(&pkt, 0, sizeof(struct rtcppkt_lost));
    memset(&server_addr, 0, sizeof(server_addr));

    pkt.pktrr.version = 0x80;
    pkt.pktrr.subtype = 0xc9;
    pkt.pktrr.len = htons((unsigned short)(sizeof(struct rtcppkt_rr)/4 - 1));
    pkt.pktrr.ssrc = htonl(rtsp->rtcp_ssrc);

    pkt.pktsd.version = 0x81;
    pkt.pktsd.subtype = 0xca;
    pkt.pktsd.len = htons((unsigned short)(sizeof(struct rtcppkt_sd)/4 - 1));
    pkt.pktsd.ssrc = htonl(rtsp->rtcp_ssrc);
    pkt.pktsd.type = 0x01;
    pkt.pktsd.sd_len = 0x1c;

    getsockname(sock, (struct sockaddr*)&server_addr, &size);
    ind_net_ntoa(&(server_addr), hostname);
    memcpy(pkt.pktsd.text, "www.zte.com.cn:", strlen("www.zte.com.cn:"));
    memcpy(pkt.pktsd.text + strlen("www.zte.com.cn:"), hostname, strlen(hostname));
    pkt.pktsd.end = 0x00;

    pkt.pktfb.version = 0x81;
    pkt.pktfb.subtype = 0xcd;
    pkt.pktfb.len = htons((unsigned short)(sizeof(struct rtcppkt_fb)/4 - 1));
    pkt.pktfb.ssrc = htonl(rtsp->rtcp_ssrc);
    pkt.pktfb.ssrc_rtp = 0x00;
    pkt.pktfb.rtcp_fb = htons((unsigned short)rtsp->rtcp_lostseq);
    pkt.pktfb.rtcp_blp = htons((unsigned short)rtsp->rtcp_lostblp);

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = sin->sin_port + htons(1);
	server_addr.sin_addr.s_addr = sin->sin_addr.s_addr;

	ind_net_sendto(data_sock, &pkt, sizeof(pkt), 0, &server_addr);
	return 0;
}



