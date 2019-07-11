
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
static int rtsp_op_heartbit_err(struct RTSP* rtsp);

static int rtsp_op_setup(struct RTSP* rtsp);
static int rtsp_op_setup_ok(struct RTSP* rtsp);
static int rtsp_op_setup_err(struct RTSP* rtsp);

static int rtsp_op_cache_on(struct RTSP* rtsp);
static int rtsp_op_cache_on_ok(struct RTSP* rtsp);
static int rtsp_op_cache_on_err(struct RTSP* rtsp);

static int rtsp_op_cache_off(struct RTSP* rtsp);
static int rtsp_op_cache_off_ok(struct RTSP* rtsp);
static int rtsp_op_cache_off_err(struct RTSP* rtsp);

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

static int rtsp_op_advertise(struct RTSP* rtsp);
static int rtsp_op_advertise_ok(struct RTSP* rtsp);

static int rtsp_op_advertise(struct RTSP* rtsp);
static int rtsp_op_advertise_ok(struct RTSP* rtsp);

static int rtsp_op_get_ret(struct RTSP* rtsp);
static int rtsp_op_get_ret_ok(struct RTSP* rtsp);
static int rtsp_op_get_ret_err(struct RTSP* rtsp);

static int rtsp_op_get_range(struct RTSP* rtsp);
static int rtsp_op_get_range_ok(struct RTSP* rtsp);
static int rtsp_op_get_range_err(struct RTSP* rtsp);

static int rtsp_op_nat(struct RTSP* rtsp);
static int rtsp_op_nat_ok(struct RTSP* rtsp);

static int rtsp_op_keeplive(struct RTSP* rtsp);


void rtsp_op_init_huawei(struct RTSP* rtsp)
{
    struct RTSPOps *ops = rtsp->ops;

    ops[RTSP_OP_HEARTBIT].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_HEARTBIT].op_f = rtsp_op_heartbit;
    ops[RTSP_OP_HEARTBIT].ok_f = rtsp_op_heartbit_ok;
    ops[RTSP_OP_HEARTBIT].err_f = rtsp_op_heartbit_err;

    ops[RTSP_OP_SETUP].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_SETUP].op_f = rtsp_op_setup;
    ops[RTSP_OP_SETUP].ok_f = rtsp_op_setup_ok;
    ops[RTSP_OP_SETUP].err_f = rtsp_op_setup_err;

    ops[RTSP_OP_CACHE_ON].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_CACHE_ON].op_f = rtsp_op_cache_on;
    ops[RTSP_OP_CACHE_ON].ok_f = rtsp_op_cache_on_ok;
    ops[RTSP_OP_CACHE_ON].err_f = rtsp_op_cache_on_err;

    ops[RTSP_OP_CACHE_OFF].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_CACHE_OFF].op_f = rtsp_op_cache_off;
    ops[RTSP_OP_CACHE_OFF].ok_f = rtsp_op_cache_off_ok;
    ops[RTSP_OP_CACHE_OFF].err_f = rtsp_op_cache_off_err;

    ops[RTSP_OP_GET_RET].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_GET_RET].op_f = rtsp_op_get_ret;
    ops[RTSP_OP_GET_RET].ok_f = rtsp_op_get_ret_ok;
    ops[RTSP_OP_GET_RET].err_f = rtsp_op_get_ret_err;

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

    ops[RTSP_OP_ADVERTISE].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_ADVERTISE].op_f = rtsp_op_advertise;
    ops[RTSP_OP_ADVERTISE].ok_f = rtsp_op_advertise_ok;
    ops[RTSP_OP_ADVERTISE].err_f = rtsp_op_open_err;

    ops[RTSP_OP_NAT].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_NAT].op_f = rtsp_op_nat;
    ops[RTSP_OP_NAT].ok_f = rtsp_op_nat_ok;
    ops[RTSP_OP_NAT].err_f = rtsp_op_open_err;

    ops[RTSP_OP_KEEPLIVE].out = TIMEOUT_CLK_COMMAND;
    ops[RTSP_OP_KEEPLIVE].op_f = rtsp_op_keeplive;
    ops[RTSP_OP_KEEPLIVE].ok_f = NULL;
    ops[RTSP_OP_KEEPLIVE].err_f = NULL;
}

static int rtsp_op_header_content(struct RTSP* rtsp, char* method, char* content)
{
    int len;

    len = rtsp_op_header(rtsp, method);
    len += sprintf(rtsp->send_buf + len, "Content-length: %d\r\n", strlen(content) + 4);
    len += sprintf(rtsp->send_buf + len, "Content-type: text/parameters\r\n");
    len += sprintf(rtsp->send_buf + len, "\r\n");
    len += sprintf(rtsp->send_buf + len, "%s\r\n\r\n", content);

    return len;
}

static int rtsp_op_heartbit(struct RTSP* rtsp)
{
    int len;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    if (rtsp->heartbit_standard == 1 || rtsp->standard == RTSP_STANDARD_CTC_SHANGHAI || rtsp->standard == RTSP_STANDARD_ELECARD)
        len = rtsp_op_header(rtsp, "GET_PARAMETER");
    else
        len = rtsp_op_header(rtsp, "OPTIONS");
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

static int rtsp_op_heartbit_err(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    if (RTSP_CODE_Session_Timeout != rtsp->rtsp_code) {
        rtsp_op_open_err(rtsp);
        return 0;
    }

    rtsp_op_none(rtsp);
    return 0;
}

static int rtsp_op_setup(struct RTSP* rtsp)
{
    int len;
    uint32_t port;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    len = 0;

    if (rtsp->standard == RTSP_STANDARD_CTC_SHANGHAI && rtsp->ContentBase[0] != 0 && rtsp->trackID != -1)
        len += sprintf(rtsp->send_buf + len, "SETUP %s/trackID=%d RTSP/1.0\r\n", rtsp->ContentBase, rtsp->trackID);
    else
        len += sprintf(rtsp->send_buf + len, "SETUP %s RTSP/1.0\r\n", rtsp->url);
    len += sprintf(rtsp->send_buf + len, "CSeq: %d\r\n", ++rtsp->CSeq);
    len += sprintf(rtsp->send_buf + len, "User-Agent: %s\r\n", rtsp->UserAgent);

    if (rtsp->standard != RTSP_STANDARD_YUXING || rtsp->transtype == SOCK_DGRAM) {
        if (rtsp_clt_datasocket(rtsp))
            LOG_STRM_ERROUT("#%d rtsp_data_open\n", rtsp->index);
    }

    port = rtsp->data_port;
    if (rtsp->standard == RTSP_STANDARD_ELECARD) {
        rtsp->transtype = SOCK_DGRAM;
        len += sprintf(rtsp->send_buf + len, "Transport: RAW/RAW/UDP;unicast;destination=%s;client_port=%u-%u\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
    } else if (rtsp->standard == RTSP_STANDARD_YUXING) {
        if (rtsp->transtype == SOCK_DGRAM) {
            LOG_STRM_PRINTF("#%d ########: rtspu\n", rtsp->index);
            len += sprintf(rtsp->send_buf + len, "Transport: MP2T/UDP;unicast;destination=%s;client_port=%u-%u\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
        } else {
            LOG_STRM_PRINTF("#%d ########: rtspt\n", rtsp->index);
            len += sprintf(rtsp->send_buf + len, "Transport: MP2T/TCP;unicast;destination=%s;interleaved=0-1\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
        }
    } else {
        LOG_STRM_PRINTF("#%d transport = %d\n", rtsp->index, rtsp->transport);
        len += sprintf(rtsp->send_buf + len, "Transport: ");
        switch (rtsp->transport) {
        case RTSP_TRANSPORT_TCP:
            len += sprintf(rtsp->send_buf + len, "MP2T/TCP;unicast;destination=%s;interleaved=0-1,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
            len += sprintf(rtsp->send_buf + len, "MP2T/RTP/TCP;unicast;destination=%s;interleaved=0-1,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
            len += sprintf(rtsp->send_buf + len, "MP2T/RTP/UDP;unicast;destination=%s;client_port=%u-%u,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
            len += sprintf(rtsp->send_buf + len, "MP2T/UDP;unicast;destination=%s;client_port=%u-%u\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
            break;
        case RTSP_TRANSPORT_UDP:
            len += sprintf(rtsp->send_buf + len, "MP2T/UDP;unicast;destination=%s;client_port=%u-%u,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
            len += sprintf(rtsp->send_buf + len, "MP2T/RTP/UDP;unicast;destination=%s;client_port=%u-%u,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
            len += sprintf(rtsp->send_buf + len, "MP2T/TCP;unicast;destination=%s;interleaved=0-1,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
            len += sprintf(rtsp->send_buf + len, "MP2T/RTP/TCP;unicast;destination=%s;interleaved=0-1\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
            break;
        case RTSP_TRANSPORT_RTP_TCP:
            len += sprintf(rtsp->send_buf + len, "MP2T/RTP/TCP;unicast;destination=%s;interleaved=0-1,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
            len += sprintf(rtsp->send_buf + len, "MP2T/TCP;unicast;destination=%s;interleaved=0-1,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
            len += sprintf(rtsp->send_buf + len, "MP2T/RTP/UDP;unicast;destination=%s;client_port=%u-%u,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
            len += sprintf(rtsp->send_buf + len, "MP2T/UDP;unicast;destination=%s;client_port=%u-%u\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
            break;
        case RTSP_TRANSPORT_RTP_UDP:
            len += sprintf(rtsp->send_buf + len, "MP2T/RTP/UDP;unicast;destination=%s;client_port=%u-%u,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
            len += sprintf(rtsp->send_buf + len, "MP2T/UDP;unicast;destination=%s;client_port=%u-%u,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), port, port + 1);
            len += sprintf(rtsp->send_buf + len, "MP2T/TCP;unicast;destination=%s;interleaved=0-1,", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
            len += sprintf(rtsp->send_buf + len, "MP2T/RTP/TCP;unicast;destination=%s;interleaved=0-1\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
            break;
        default:
            len += sprintf(rtsp->send_buf + len, "MP2T/TCP;unicast;destination=%s;interleaved=0-1\r\n", rtsp_clt_addr_fmt(rtsp, &rtsp->sockname));
            break;
        }
//纯中兴    len += sprintf(rtsp->send_buf + len, "Transport: RTP/AVP/TCP;unicast;interleaved=0-1,RTP/AVP/UDP;unicast;client_port=%u-%u\r\n", port, port + 1);
    }

    if(rtsp->nat_flag) {
        char hostname[40];
        socklen_t size = sizeof(struct sockaddr_in);
        struct sockaddr_in sockname;

        rtsp->nat_flag = 1;
        rtsp->nat_ssrc = 0;

        if(rtsp->serv_sin.family != AF_INET)
            LOG_STRM_ERROUT("just support IPV4\n");

        IND_MEMSET(hostname, 0 , 40);
        memset(&sockname, 0, sizeof(sockname));

        getsockname(rtsp->sock, (struct sockaddr*)&sockname, &size);

        ind_net_ntoa_ex(&(sockname), hostname);
        len += sprintf(rtsp->send_buf + len, "x-NAT_Address:%s\r\n", hostname);
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

    rtsp->rtp_flag = 0;
    if (rtsp->standard == RTSP_STANDARD_ELECARD) {
        rtsp->transtype = SOCK_DGRAM;
    } else if (rtsp->standard != RTSP_STANDARD_YUXING) {
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
        }
        if (strncmp(p, "UDP", 3) == 0) {
            LOG_STRM_PRINTF("#%d ########: UDP arq = %d, ret = %d, rtp = %d\n", rtsp->index, rtsp->arq_flag, rtsp->ret_flag, rtsp->rtp_flag);
            rtsp->transtype = SOCK_DGRAM;
        } else if (strncmp(p, "TCP", 3) == 0) {
            LOG_STRM_PRINTF("#%d ########: TCP\n", rtsp->index);
            rtsp->transtype = SOCK_STREAM;
            close(rtsp->data_sock);
            rtsp->data_sock = -1;
            rtsp_clt_fdset(rtsp);

            if (rtsp->index != STREAM_INDEX_PIP) {
                if (APP_TYPE_VOD == rtsp->apptype && rtsp->index < STREAM_INDEX_PIP)
                    int_steam_setToalBufferSize(rtsp->size_cache);
                strm_play_resize(rtsp->strm_play, rtsp->index, RTP_BUFFER_LENGTH, rtsp->size_cache);
            }
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
        if (rtsp->burst_flag >= 2)
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

    if (rtsp->standard != RTSP_STANDARD_ELECARD) {
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
                rtsp->sockname.port = (uint16_t)ind_atoui(p + 12);
            p = ind_stristr(transport, "server_port=");
            if (p)
                rtsp->peername.port = (uint16_t)ind_atoui(p + 12);
        }

        LOG_STRM_PRINTF("#%d pearname %s:%hu\n", rtsp->index, rtsp_clt_addr_fmt(rtsp, &rtsp->peername), rtsp->peername.port);
        LOG_STRM_PRINTF("#%d sockname %s:%hu\n", rtsp->index, rtsp_clt_addr_fmt(rtsp, &rtsp->sockname), rtsp->sockname.port);
    }

    rtsp->rtsp_state = RTSP_STATE_PAUSE;

    {
        int interval = rtsp->heartbit_period * 100;
        ind_timer_create(rtsp->tlink, rtsp->clk + (uint32_t)interval, interval, rtsp_clt_heartbit, rtsp);
    }

    //x-NAT: off or str NONE
    if (rtsp->nat_flag && rtsp->nat_ssrc) {
        rtsp->nat_flag = 2;
        rtsp_clt_set_rtspsock(rtsp->data_sock);
    }

    LOG_STRM_DEBUG("#%d ret_flag = %d nat_flag = %d, transtype = %d, rtp_flag = %d\n", rtsp->index, rtsp->ret_flag, rtsp->nat_flag, rtsp->transtype, rtsp->rtp_flag);
    if (rtsp->ret_flag)
        rtsp->ret_flag = 1;

    if (rtsp->ret_flag && rtsp->transtype == SOCK_DGRAM && rtsp->rtp_flag) {
        rtsp_op_immediate_reg(rtsp, RTSP_OP_GET_RET);
    } else {
        if(rtsp->nat_flag == 2)
            rtsp_op_immediate_reg(rtsp, RTSP_OP_NAT);
        else
            rtsp_op_immediate_reg(rtsp, RTSP_OP_PLAY_INIT);
    }
    if (rtsp->recv_safe > 1)
        rtsp->recv_safe = 1;

    if (rtsp->index == STREAM_INDEX_PIP || rtsp->cache_clk_level <= 0)
        rtsp->cache = CACHE_STATE_UNSPPORT;
    LOG_STRM_PRINTF("#%d cache = %d level = %d, burst = %d\n", rtsp->index, rtsp->cache, rtsp->cache_clk_level, rtsp->burst_flag);

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

static int rtsp_op_get_ret(struct RTSP* rtsp)
{
    int    len;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    len = rtsp_op_header_content(rtsp, "GET_PARAMETER", "x-RET_Enable");

    if (rtsp_clt_send(rtsp, len))
        LOG_STRM_ERROUT("#%d rtsp_clt_send\n", rtsp->index);

    rtsp_op_timeout_reg(rtsp, RTSP_OP_GET_RET);

    return 0;
Err:
    rtsp_op_open_err(rtsp);
    return -1;
}

static void rtsp_op_get_ret_end(struct RTSP* rtsp)
{
    printf("#%d arq_flag = %d, burst_flag = %d, nat_flag = %d\n", rtsp->index, rtsp->arq_flag, rtsp->burst_flag, rtsp->nat_flag);

    if (1 == rtsp->ret_flag && rtsp->arq_flag < 2 && rtsp->burst_flag < 2)
        rtsp->ret_flag = 2;//开启RET功能用于包乱序处理

    if(rtsp->nat_flag == 2)
        rtsp_op_immediate_reg(rtsp, RTSP_OP_NAT);
    else
        rtsp_op_immediate_reg(rtsp, RTSP_OP_PLAY_INIT);
}

static int rtsp_op_get_ret_ok(struct RTSP* rtsp)
{
    char *p;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    p = ind_stristr(rtsp->ctrl_buf, "x-RET_Enable: 1");
    if (p || rtsp->arq_flag <= 1) {//不支持ARQ时开启RET用于包排序
        rtsp->ret_flag = 2;

        if (rtsp->arq_flag >= 2)
            rtsp->arq_flag = 1;
    }

    rtsp_op_get_ret_end(rtsp);

    return 0;
}

static int rtsp_op_get_ret_err(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d rtsp_code = %d\n", rtsp->index, rtsp->rtsp_code);

    rtsp_op_open_err(rtsp);

    return 0;
}

int rtsp_op_param_set(struct RTSP* rtsp, RTSP_PARAM param)
{
    int    len;
    char str[32];

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    switch(param) {
    case PARAM_X_FAST_CACHE_ON:        sprintf(str, "x-Fast_Cache: on");        break;
    case PARAM_X_FAST_CACHE_OFF:    sprintf(str, "x-Fast_Cache: off");        break;
    case PARAM_X_HEARTBIT_TIME:        sprintf(str, "x-Heartbeat_Time: %d", rtsp->heartbit_period);    break;
    default: LOG_STRM_ERROUT("#%d param = %d\n", rtsp->index, param);
    }

    LOG_STRM_PRINTF("#%d %s\n", rtsp->index, str);
    len = rtsp_op_header_content(rtsp, "SET_PARAMETER", str);

    if (rtsp_clt_send(rtsp, len))
        LOG_STRM_ERROUT("#%d rtsp_clt_send\n", rtsp->index);

    return 0;
Err:
    return -1;
}

static int rtsp_op_cache_on(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    if (rtsp_op_param_set(rtsp, PARAM_X_FAST_CACHE_ON))
        LOG_STRM_ERROUT("#%d rtsp_op_param_set\n", rtsp->index);

    rtsp_op_timeout_reg(rtsp, RTSP_OP_CACHE_ON);

    return 0;
Err:
    rtsp_op_none(rtsp);
    return -1;
}

static int rtsp_op_cache_on_ok(struct RTSP* rtsp)
{
    rtsp->cache = CACHE_STATE_ON;
    if (ind_stristr(rtsp->ctrl_buf, "x-Fast_Cache")) {

        LOG_STRM_DEBUG("#%d cache = %d, ret_flag == %d\n", rtsp->index, rtsp->cache, rtsp->ret_flag);

        if (rtsp->ret_flag >= 3) {
            LOG_STRM_DEBUG("#%d ret_port_cache on\n", rtsp->index);
            ret_port_cache(rtsp->ret_handle, 1);
        }
        if (rtsp->burst_flag >= 2)
            rtsp->burst_flag = 1;
    } else {
        if (rtsp->burst_flag >= 2)
            rtsp->burst_flag = 3;
        rtsp->cache = CACHE_STATE_UNSPPORT;
    }
    LOG_STRM_PRINTF("#%d cache = %d, burst_flag = %d\n", rtsp->index, rtsp->cache, rtsp->burst_flag);

    rtsp_op_none(rtsp);
    return 0;
}

static int rtsp_op_cache_on_err(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d rtsp_code = %d\n", rtsp->index, rtsp->rtsp_code);

    if (RTSP_CODE_Session_Timeout != rtsp->rtsp_code) {
        rtsp_op_open_err(rtsp);
        return 0;
    }

    rtsp->cache = CACHE_STATE_UNSPPORT;
    rtsp_op_none(rtsp);
    return 0;
}

static int rtsp_op_cache_off(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    if (rtsp_op_param_set(rtsp, PARAM_X_FAST_CACHE_OFF))
        LOG_STRM_ERROUT("#%d rtsp_op_param_set\n", rtsp->index);

    rtsp_op_timeout_reg(rtsp, RTSP_OP_CACHE_OFF);

    return 0;
Err:
    rtsp_op_none(rtsp);
    return -1;
}

static void rtsp_op_cache_off_end(struct RTSP* rtsp)
{
    if (rtsp->retry_op != RTSP_OP_NONE) {
        rtsp->op_off = rtsp->retry_op_off;
        rtsp->op_scale = rtsp->retry_op_scale;

        switch(rtsp->retry_op) {
        case RTSP_OP_SEEK:
            LOG_STRM_PRINTF("#%d RTSP_OP_SEEK\n", rtsp->index);
            rtsp_op_immediate_reg(rtsp, RTSP_OP_SEEK);
            break;

        case RTSP_OP_FAST:
            LOG_STRM_PRINTF("#%d RTSP_OP_FAST\n", rtsp->index);
            rtsp_op_immediate_reg(rtsp, RTSP_OP_FAST);
            break;

        default:
            LOG_STRM_PRINTF("#%d Unknown!\n", rtsp->index);
            rtsp_op_none(rtsp);
            break;
        }
        rtsp->retry_op = RTSP_OP_NONE;
    } else {
        rtsp_op_none(rtsp);
    }
}

static int rtsp_op_cache_off_ok(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d ret_flag == %d\n", rtsp->index, rtsp->ret_flag);

    if (rtsp->ret_flag >= 3) {
        LOG_STRM_DEBUG("#%d ret_port_cache off\n", rtsp->index);
        ret_port_cache(rtsp->ret_handle, 0);
    }

    rtsp->cache = CACHE_STATE_OFF;

    rtsp_op_cache_off_end(rtsp);
    return 0;
}

static int rtsp_op_cache_off_err(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d rtsp_code = %d\n", rtsp->index, rtsp->rtsp_code);

    if (RTSP_CODE_Session_Timeout != rtsp->rtsp_code) {
        rtsp_op_open_err(rtsp);
        return 0;
    }

    rtsp_op_cache_off_end(rtsp);
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
        //为避免命令响应延时，临时关闭安全接收
        if (rtsp->recv_safe >= 1 && (scale != 1 || type == PLAY_TYPE_SET))
            rtsp->recv_safe = 2;
        //华为服务器 end-end时会直接播放下一分段
        //TVOD 发 end- 时服务器返回无效范围
        if (type == PLAY_TYPE_SET && off >= rtsp->time_length)
            off = rtsp->time_length - 1;
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
                if (rtsp->time_begin > 0)
                    off += rtsp->time_begin;
                if (rtsp->standard == RTSP_STANDARD_ELECARD)
                    sprintf(range, "Range: npt=%d.000-", off);
                else
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
                if (scale == 1 && rtsp->standard != RTSP_STANDARD_YUXING) {
                    if (rtsp->apptype == APP_TYPE_IPTV && rtsp->standard == RTSP_STANDARD_HUAWEI)
                        sprintf(range, "Range: clock=beginning-");
                    else
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
        default:
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
        case STRM_STATE_ADVERTISE:
            LOG_STRM_PRINTF("#%d STRM_STATE_ADVERTISE\n", rtsp->index);
            rtsp_op_none(rtsp);
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
        case RTSP_OP_ADVERTISE:
            LOG_STRM_PRINTF("#%d STRM_STATE_ADVERTISE\n", rtsp->index);
            rtsp_op_none(rtsp);
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

    if (rtsp->apptype == APP_TYPE_VOD) {
        if (rtsp->time_current >= rtsp->time_length) {
            LOG_STRM_WARN("#%d sec(%d) >= rtsp->time_length(%d)\n", rtsp->index, rtsp->time_current, rtsp->time_length);
            if (rtsp->time_length <= 3)
                rtsp->time_current = 0;
            else
                rtsp->time_current = rtsp->time_length - 3;
        }

        if (rtsp->adv_num > 0) {
            int insert = rtsp->time_current;
            if (insert == 0)
                insert = -1;
            if (rtsp_clt_advertise_elem(rtsp, insert)) {
                LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", rtsp->index);
                rtsp_clt_state(rtsp, STRM_STATE_PLAY, 1);
                rtsp->adv_insert = insert;
                rtsp_op_advertise(rtsp);
                return 0;
            }

            rtsp_clt_advertise_select(rtsp, 0, rtsp->time_current);
            LOG_STRM_PRINTF("time_current = %d, adv_insert = %d\n", rtsp->time_current, rtsp->adv_insert);
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
    if (rtsp_op_play_send(rtsp)) {
        rtsp->rtsp_code = RTSP_CODE_Play_Error;
        LOG_STRM_ERROUT("#%d APP_TYPE_VOD play\n", rtsp->index);
    }
    rtsp_op_timeout_reg(rtsp, RTSP_OP_PLAY_INIT);

    return 0;
Err:
    rtsp_op_open_err(rtsp);
    return -1;
}

static int rtsp_op_play_init_ok(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp->rtsp_state = RTSP_STATE_PLAY;
    rtsp->clock = rtsp->clk;

    if (rtsp->apptype == APP_TYPE_VOD) {
        LOG_STRM_DEBUG("#%d time_length = %d\n", rtsp->index, rtsp->time_length);

        rtsp->time_start = rtsp->time_current;
        LOG_STRM_PRINTF("#%d VOD start = %d\n", rtsp->index, rtsp->time_start);
    }
    if (rtsp->apptype == APP_TYPE_VOD) {
        LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", rtsp->index);
        rtsp_clt_state(rtsp, STRM_STATE_PLAY, 1);

        if (rtsp->open_play)
            rtsp_stat_set(rtsp, 1);

        rtsp_clt_resume(rtsp);
    } else {
        if (rtsp->iptvtype == IPTV_TYPE_UNICAST) {
            LOG_STRM_PRINTF("#%d STRM_STATE_IPTV\n", rtsp->index);
            rtsp_clt_state(rtsp, STRM_STATE_IPTV, 1);
            if (rtsp->open_play) {
                rtsp_stat_set(rtsp, 1);
                strm_play_resume(rtsp->strm_play, rtsp->index, 1);
            }
        } else {
            LOG_STRM_WARN("#%d rtsp->apptype = %d\n", rtsp->index, rtsp->apptype);
        }
    }

    if (RTSP_MAIN_PLAY(rtsp)) {
        LOG_STRM_PRINTF("#%d arq_flag = %d\n", rtsp->index, rtsp->arq_flag);
        if (rtsp->arq_flag >= 2) {
            rtsp_clt_arq_open(rtsp);
        }
    }

    if (rtsp->ret_flag >= 2)
        rtsp_clt_ret_open(rtsp);

    if (rtsp->open_play)
        strm_play_set_idle(rtsp->strm_play, rtsp->index, 0);
    rtsp_clt_post_valid(rtsp, 1);

    rtsp_op_none(rtsp);

    return 0;
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

    if (rtsp_op_play_send(rtsp)) {
        rtsp->rtsp_code = RTSP_CODE_Play_Error;
        LOG_STRM_ERROUT("#%d rtsp_clt_send\n", rtsp->index);
    }

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

    if (abs(rtsp->scale) > 1 && rtsp->op_scale == 1) {
        LOG_STRM_PRINTF("#%d PLAY_TYPE_SET\n", rtsp->index);
        rtsp->op_type = PLAY_TYPE_SET;//重新获取基准时间
    }

    rtsp->rtsp_state = RTSP_STATE_PLAY;

    if (rtsp->op_type != PLAY_TYPE_NOW || abs(rtsp->op_scale) > 1) {
        if (rtsp->op_type == PLAY_TYPE_SET) {
            rtsp->time_start = rtsp->op_off;
            if (rtsp->apptype != APP_TYPE_VOD) {
                uint32_t begin = rtsp_clt_time_server(rtsp) - rtsp->time_length + TIMESHIFT_BOUNDARY_SEC;
                if (rtsp->time_start < begin)
                    rtsp->time_start = begin;
            }
        } else {
            char *buf, *p;

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
                LOG_STRM_WARN("#%d Range is empty\n", rtsp->index);
                if (rtsp->apptype != APP_TYPE_VOD && rtsp->op_type == PLAY_TYPE_END) {
                    rtsp->time_start = rtsp_clt_time_server(rtsp) - TIMESHIFT_BOUNDARY_SEC;
                } else {
                    rtsp_clt_time_sync(rtsp);
                    rtsp->time_start = rtsp->time_current;
                }
            }
        }

        rtsp->ts_len = 0;
    }

    if (abs(rtsp->op_scale) > 1 || rtsp->op_type != PLAY_TYPE_NOW) {
        if (rtsp->open_play)
            rtsp_clt_reset_play(rtsp, 1);
    }

    if (rtsp->ret_flag >= 2) {
        if (rtsp->op_scale == 1) {
            if (rtsp->op_type != PLAY_TYPE_NOW || rtsp->ret_flag == 2) {
                rtsp_clt_ret_close(rtsp);
                rtsp_clt_ret_open(rtsp);
            }
        } else {
            if (rtsp->ret_flag > 2)
                rtsp_clt_ret_close(rtsp);
        }
    }
    rtsp_clt_fcc_reset(rtsp);

    if (abs(rtsp->op_scale) > 1) {
        strm_play_tplay(rtsp->strm_play, rtsp->index, rtsp->op_scale);
        if (SOCK_DGRAM == rtsp->transtype)
            strm_play_tskip(rtsp->strm_play, rtsp->index);
        rtsp_stat_set(rtsp, 0);

        if (rtsp->state == STRM_STATE_PLAY && rtsp->skipfast_clks) {
            LOG_STRM_PRINTF("#%d skipfast_clks = %d\n", rtsp->index, rtsp->skipfast_clks);
            rtsp->skipfast_clk = rtsp->clk + rtsp->skipfast_clks;
        } else {
            rtsp->skipfast_clk = 0;
        }
        LOG_STRM_PRINTF("#%d STRM_STATE_FAST\n", rtsp->index);
        rtsp_clt_state(rtsp, STRM_STATE_FAST, rtsp->op_scale);
        if (rtsp->op_scale < 0 && rtsp->timeshift_len > 0 && rtsp->timeshift_len < rtsp->timeshift_real)
            ind_timer_create(rtsp->tlink, rtsp->clk + INTERVAL_CLK_SHIFTCHK, INTERVAL_CLK_SHIFTCHK, rtsp_clt_shiftcheck, rtsp);
    } else {
        if (rtsp->open_play)
            rtsp_stat_set(rtsp, 1);
        LOG_STRM_PRINTF("#%d STRM_STATE_PLAY cache = %d\n", rtsp->index, rtsp->cache);
        rtsp_clt_state(rtsp, STRM_STATE_PLAY, 1);

        if (rtsp->open_play)
            rtsp_clt_resume(rtsp);
        if (SEEK_END_DUMMY == rtsp->seek_end)
            strm_play_set_idle(rtsp->strm_play, rtsp->index, 1);
    }

    if (CACHE_STATE_ON == rtsp->cache) {
            LOG_STRM_DEBUG("#%d ret_flag = %d\n", rtsp->index, rtsp->ret_flag);
        if (rtsp->ret_flag >= 3)
            ret_port_cache(rtsp->ret_handle, 0);
        rtsp->cache = CACHE_STATE_OFF;
    }
    rtsp_op_none(rtsp);

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

        if (!rtsp->timeshift_support) {
            rtsp->op_type = PLAY_TYPE_END;
            rtsp->op_scale = 1;
            if (rtsp_op_play_send(rtsp))
                LOG_STRM_ERROUT("#%d rtsp_op_play\n", rtsp->index);
        }

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
    if (rtsp_clt_send(rtsp, len)) {
        rtsp->rtsp_code = RTSP_CODE_Play_Error;
        LOG_STRM_ERROUT("#%d rtsp_clt_send\n", rtsp->index);
    }

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

    switch(rtsp->state) {
    case STRM_STATE_FAST:
        rtsp_clt_time_sync(rtsp);
        rtsp->time_start = rtsp->time_current;
        rtsp_clt_reset_play(rtsp, 1);
        break;
    case STRM_STATE_IPTV:
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

    rtsp->op_type = PLAY_TYPE_NOW;

    state = rtsp->state;
    switch(state) {
    case STRM_STATE_FAST:
        rtsp_clt_time_sync(rtsp);
        break;
    case STRM_STATE_PAUSE:
        if (rtsp->apptype == APP_TYPE_IPTV) {
            uint32_t begin = rtsp_clt_time_server(rtsp) - rtsp->time_length + TIMESHIFT_BOUNDARY_SEC;
            uint32_t current = rtsp->time_current;
            LOG_STRM_DEBUG("#%d: begin = %s current = %s\n", rtsp->index, rtsp_clt_time_fmt(rtsp, 0, begin), rtsp_clt_time_fmt(rtsp, 1, current));
            if (current <= begin) {
                LOG_STRM_PRINTF("#%d PLAY_TYPE_SET\n", rtsp->index);
                rtsp->op_type = PLAY_TYPE_SET;

                rtsp_clt_reset_play(rtsp, 0);
                strm_play_resume(rtsp->strm_play, rtsp->index, 0);
            }
        }
        break;
    default:
        LOG_STRM_ERROUT("#%d rtsp->state = %d\n", rtsp->index, rtsp->state);
    }

    rtsp->stat.stat_vodplay = rtsp->clk;

    rtsp->op_off = rtsp->time_current;
    rtsp->op_scale = 1;

    rtsp->retry_op = RTSP_OP_PLAY;
    rtsp->retry_op_off = rtsp->time_current;
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

    if (rtsp->apptype == APP_TYPE_IPTV) {
        end = rtsp_clt_time_server(rtsp);
        begin = end - rtsp->time_length;
    } else {
        end = rtsp->time_length;
        begin = 0;
    }

    rtsp->seek_end = SEEK_END_NONE;

    if (rtsp->apptype == APP_TYPE_VOD) {
        if (rtsp->state != STRM_STATE_ADVERTISE) {
            LOG_STRM_PRINTF("#%d seek = %d / (%d, %d), scale = %d!\n", rtsp->index, rtsp->op_off, begin, end, rtsp->op_scale);
            if (rtsp->op_scale == -1 && rtsp->op_off <= begin + 1)//op_scale为-1表示由seek命令而产生
                rtsp_post_msg(rtsp, STRM_MSG_SEEK_BEGIN, 0);
            else if (rtsp->op_off >= end) {
                rtsp->seek_end = SEEK_END_NORMAL;
                if (!rtsp_clt_advertise_elem(rtsp, -2)) {
                    rtsp_post_msg(rtsp, STRM_MSG_STREAM_END, 0);
                    rtsp->seek_end = SEEK_END_DUMMY;
                }
            }
        }
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
    case STRM_STATE_ADVERTISE:
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

    switch(rtsp->state) {
    case STRM_STATE_PLAY:
    case STRM_STATE_PAUSE:
        if (STRM_STATE_PLAY == rtsp->state)
            rtsp_clt_time_sync(rtsp);
        if (rtsp->apptype == APP_TYPE_IPTV
            && rtsp->time_current <= rtsp_clt_time_server(rtsp) - rtsp->time_length + TIMESHIFT_BOUNDARY_SEC) {
                if (rtsp->op_scale < 0) {
                    rtsp_msg_back(rtsp, STRM_MSG_STREAM_BEGIN, 0);
                    rtsp_op_none(rtsp);
                    return 0;
                }
                rtsp->op_off = 0;
                rtsp->op_type = PLAY_TYPE_SET;
        } else if (rtsp->standard == RTSP_STANDARD_CTC_SHANGHAI/* || (rtsp->apptype == APP_TYPE_VOD && rtsp->ctc_begin > 0)*/) {
            rtsp->op_type = PLAY_TYPE_NOW;
        } else {
            rtsp->op_off = rtsp->time_current;
            rtsp->op_type = PLAY_TYPE_SET;
        }
        break;
    case STRM_STATE_FAST:
        rtsp_clt_time_sync(rtsp);
        rtsp->op_type = PLAY_TYPE_NOW;
        break;
    case STRM_STATE_IPTV:
        rtsp_clt_time_set_current(rtsp, rtsp_clt_time_server(rtsp));
        LOG_STRM_DEBUG("#%d time_current = %d, timeshift_support = %d\n", rtsp->index, rtsp->time_current, rtsp->timeshift_support);
        rtsp->time_start = rtsp->time_current;
        if (rtsp->timeshift_support)
            rtsp->op_type = PLAY_TYPE_NOW;
        else
            rtsp->op_type = PLAY_TYPE_END;
        break;
    default:
        LOG_STRM_ERROUT("#%d rtsp->state = %d\n", rtsp->index, rtsp->state);
    }

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

    rtsp->retry_op = RTSP_OP_NONE;

    if (rtsp->open_play)
        rtsp->open_play = OPEN_PLAY_RUNNING;

    if (rtsp->iptvtype == IPTV_TYPE_UNICAST) {
        if (rtsp->open_play) {
            rtsp_stat_set(rtsp, 1);
            if (STREAM_INDEX_PIP != rtsp->index) {
                rtsp_clt_reset_play(rtsp, 1);
                strm_play_resume(rtsp->strm_play, rtsp->index, 1);
            }
        }
        LOG_STRM_PRINTF("#%d STRM_STATE_IPTV\n", rtsp->index);
        rtsp_clt_state(rtsp, STRM_STATE_IPTV, 1);
        rtsp->rtsp_state = RTSP_STATE_PLAY;
    } else {
        if (rtsp->standard == RTSP_STANDARD_CTC_SHANGHAI && rtsp->rtsp_state != RTSP_STATE_TEARDOWN)
            rtsp->rtsp_state = RTSP_STATE_PAUSE;
        else
            rtsp_clt_close_vod(rtsp);

        if (0 == rtsp->mult_flag)
            rtsp_clt_iptv_open(rtsp);
    }

    if (rtsp->ret_flag >= 2) {
        if (rtsp->iptvtype == IPTV_TYPE_UNICAST)
            rtsp_clt_ret_open(rtsp);
        else if (rtsp->ret_flag > 2)
            rtsp_clt_ret_close(rtsp);
    }

    if (rtsp->arq_flag >= 2)
        rtsp_clt_arq_close(rtsp);

    if (IPTV_TYPE_UNICAST == rtsp->iptvtype && rtsp->cache == CACHE_STATE_ON)
        rtsp_op_immediate_reg(rtsp, RTSP_OP_CACHE_OFF);
    else
        rtsp_op_none(rtsp);

    rtsp_clt_post_valid(rtsp, 1);
    return 0;
}

static int rtsp_op_advertise(struct RTSP* rtsp)
{
    int len;

    LOG_STRM_PRINTF("#%d state = %d\n", rtsp->index, rtsp->state);

    if (rtsp->rtsp_state != RTSP_STATE_PLAY) {
        rtsp_op_advertise_ok(rtsp);
        return 0;
    }

    rtsp_clt_post_valid(rtsp, 0);

    rtsp_clt_time_sync(rtsp);
    strm_play_pause(rtsp->strm_play, rtsp->index);

    len = rtsp_op_header(rtsp, "PAUSE");
    len += sprintf(rtsp->send_buf + len, "\r\n");

    rtsp->retry_op = RTSP_OP_ADVERTISE;
    rtsp->retry_op_off = -1;
    if (rtsp_clt_send(rtsp, len))
        LOG_STRM_ERROUT("#%d rtsp_clt_send\n", rtsp->index);

    rtsp_op_timeout_reg(rtsp, RTSP_OP_ADVERTISE);

    return 0;
Err:
    rtsp_op_open_err(rtsp);
    return -1;
}

static int rtsp_op_advertise_ok(struct RTSP* rtsp)
{
    struct Advertise* adv;

    LOG_STRM_PRINTF("#%d adv_insert = %d\n", rtsp->index, rtsp->adv_insert);

    LOG_STRM_PRINTF("#%d STRM_STATE_ADVERTISE\n", rtsp->index);
    rtsp_clt_state(rtsp, STRM_STATE_ADVERTISE, 1);
    adv = rtsp_clt_advertise_elem(rtsp, rtsp->adv_insert);

    if (adv == NULL) {
        rtsp->op_off = rtsp->time_current;
        rtsp_op_immediate_reg(rtsp, RTSP_OP_SEEK);
    } else {
        rtsp_clt_ret_close(rtsp);
        rtsp_clt_arq_close(rtsp);

        strm_play_close(rtsp->strm_play, rtsp->index, 0);
        LOG_STRM_PRINTF("#%d STRM_MSG_ADVERTISE_BEGIN adv_insert - %d\n", rtsp->index, rtsp->adv_insert);
        rtsp_post_msg(rtsp, STRM_MSG_ADVERTISE_BEGIN, rtsp->adv_insert);

        strm_play_leader_set(rtsp->strm_play, STREAM_INDEX_ADV);

        mid_stream_open(STREAM_INDEX_ADV, adv->url, APP_TYPE_VOD, 0);
        rtsp_op_none(rtsp);
    }

    return 0;
}

static int rtsp_op_get_range(struct RTSP* rtsp)
{
    int    len;

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    if (rtsp->standard == RTSP_STANDARD_CTC_SHANGHAI) {
        len = rtsp_op_header(rtsp, "GET_PARAMETER");
        len += sprintf(rtsp->send_buf + len, "x-Timeshift_Range\r\n\r\n");
    } else {
        len = rtsp_op_header_content(rtsp, "GET_PARAMETER", "x-Timeshift_Range");
    }

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

static int int_sendnat(int sock, char type, int ssrc, struct sockaddr_in* sin)
{
	struct natpkt pkt;
	struct sockaddr_in server_addr;

	memset(&pkt, 0, sizeof(struct natpkt));

	pkt.version = 0x00;
	pkt.type = type;
	pkt.reserved = 0x0000;
	pkt.ssrc = htonl(ssrc);

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = sin->sin_port;
	server_addr.sin_addr.s_addr = sin->sin_addr.s_addr;

	ind_net_sendto(sock, (void *)&pkt, sizeof(pkt), 0, &server_addr);
	return 0;
}

static int rtsp_op_nat(struct RTSP* rtsp)
{
    struct sockaddr_in serv_sin;
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    serv_sin.sin_family = AF_INET;
    serv_sin.sin_port = htons(rtsp->peername.port);
    serv_sin.sin_addr = rtsp->peername.in_addr;

    if (rtsp->data_sock < 0)
        LOG_STRM_ERROUT("#%d socket errno = %d! %s\n", rtsp->index, errno, strerror(errno));

    int_sendnat(rtsp->data_sock, NAT_MSG_RENEW, rtsp->nat_ssrc, &serv_sin);
    int_sendnat(rtsp->data_sock, NAT_MSG_RENEW, rtsp->nat_ssrc, &serv_sin);

    rtsp_op_timeout_reg(rtsp, RTSP_OP_NAT);
    return 0;
Err:
    rtsp_op_open_err(rtsp);
    return -1;
}

static int rtsp_op_nat_ok(struct RTSP* rtsp)
{
    int interval;
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp_op_immediate_reg(rtsp, RTSP_OP_PLAY_INIT);
    interval = rtsp->nat_heartbitperiod * 100;

    ind_timer_create(rtsp->tlink, 0, interval, rtsp_clt_natheartbit, rtsp);
    return 0;
}

static int rtsp_op_keeplive(struct RTSP* rtsp)
{
    struct sockaddr_in serv_sin;
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    serv_sin.sin_family = AF_INET;
    serv_sin.sin_port = htons(rtsp->peername.port);
    serv_sin.sin_addr = rtsp->peername.in_addr;

    if (rtsp->data_sock < 0)
        LOG_STRM_ERROUT("#%d socket errno = %d! %s\n", rtsp->index, errno, strerror(errno));

    int_sendnat(rtsp->data_sock, NAT_MSG_KEEPALIVE, rtsp->nat_ssrc, &serv_sin);
    int_sendnat(rtsp->data_sock, NAT_MSG_KEEPALIVE, rtsp->nat_ssrc, &serv_sin);

    rtsp_op_none(rtsp);
    return 0;
Err:
    rtsp_op_none(rtsp);
    return -1;
}
