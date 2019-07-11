
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

static int g_difftime = 0;
static int g_sock = -1;

static void int_clt_time_play(struct RTSP* rtsp);
static void int_clt_time_fast(struct RTSP* rtsp, int align);

void rtsp_msg_back(void *handle, STRM_MSG msgno, int arg)
{
    struct RTSP *rtsp = (struct RTSP *)handle;
    LOG_STRM_DEBUG("#%d msgno = %d, arg = %d\n", rtsp->index, msgno, arg);
    if (RECORD_MSG_SUCCESS_END == msgno) {
#ifdef INCLUDE_PVR
        LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_END\n", rtsp->index);
        record_post_msg(rtsp->rec_index, (uint32_t)arg, RECORD_MSG_SUCCESS_END, 0);
        LOG_STRM_PRINTF("#%d RECORD_MSG_CLOSE\n", rtsp->index);
        record_post_msg(rtsp->rec_index, (uint32_t)arg, RECORD_MSG_CLOSE, 0);
#endif
    } else {
        strm_msgq_queue(rtsp->strm_msgq, msgno, arg);
    }
}

StreamCmd* rtsp_cmd_pump(struct RTSP* rtsp)
{
    StreamCmd *strmCmd;

    strmCmd = rtsp->cmd_queue;
    if (strmCmd) {
        rtsp->cmd_queue = strmCmd->next;
        strmCmd->next = NULL;
    }

    return strmCmd;
}

void rtsp_cmd_free(struct RTSP* rtsp, StreamCmd *strmCmd)
{
    strmCmd->next = rtsp->cmd_pool;
    rtsp->cmd_pool = strmCmd;
}

void rtsp_cmd_empty(struct RTSP* rtsp)
{
    StreamCmd *strmCmd, *next;

    strmCmd = rtsp->cmd_queue;
    while(strmCmd) {
        next = strmCmd->next;
        strmCmd->next = rtsp->cmd_pool;
        rtsp->cmd_pool = strmCmd;

        strmCmd = next;
    }
    rtsp->cmd_queue = NULL;
}

//加到队列头部
static void rtsp_cmd_push(struct RTSP* rtsp, int cmd, int arg)
{
    StreamCmd *strmCmd;

    if (rtsp->cmd_close == 1) {
        LOG_STRM_WARN("#%d inoge cmd = %d\n", rtsp->index, cmd);
        goto Err;
    }

    if (rtsp->cmd_queue && rtsp->cmd_queue->cmd == STREAM_CMD_CLOSE)
        LOG_STRM_ERROUT("#%d STREAM_CMD_CLOSE!\n", rtsp->index);

    if (rtsp->cmd_pool == NULL)
        LOG_STRM_ERROUT("#%d msg pool empty!\n", rtsp->index);
    strmCmd = rtsp->cmd_pool;
    rtsp->cmd_pool = strmCmd->next;
    strmCmd->next = rtsp->cmd_queue;
    rtsp->cmd_queue = strmCmd;

    strmCmd->cmd = cmd;
    strmCmd->arg0 = arg;
    strmCmd->arg1 = 0;
    strmCmd->arg2 = 0;
    strmCmd->arg3 = 0;

Err:
    return;
}

//加到队列尾部
void rtsp_cmd_queue(struct RTSP* rtsp, StreamCmd *strmCmd)
{
    StreamCmd *m, *q;

    if (rtsp->cmd_pool == NULL)
        LOG_STRM_ERROUT("#%d msg pool empty!\n", rtsp->index);

    q = rtsp->cmd_queue;
    while(q && q->next)
        q = q->next;

    if (strmCmd->cmd == STREAM_CMD_RESUME || strmCmd->cmd == STREAM_CMD_PAUSE || strmCmd->cmd == STREAM_CMD_FAST) {
        if (rtsp_clt_trickmode(rtsp) == 0)
            LOG_STRM_ERROUT("#%d rtsp_clt_trickmode!\n", rtsp->index);

        if (q && (q->cmd == STREAM_CMD_RESUME || q->cmd == STREAM_CMD_PAUSE || q->cmd == STREAM_CMD_FAST)) {
            LOG_STRM_PRINTF("#%d old = %d, new = %d\n", rtsp->index, q->cmd, strmCmd->cmd);
            m = q;
            goto Cpy;
        }
    }

    m = rtsp->cmd_pool;
    rtsp->cmd_pool = m->next;
    m->next = NULL;

    if (q)
        q->next = m;
    else
        rtsp->cmd_queue = m;

Cpy:
    m->cmd = strmCmd->cmd;
    m->arg0 = strmCmd->arg0;
    m->arg1 = strmCmd->arg1;
    m->arg2 = strmCmd->arg2;
    m->arg3 = strmCmd->arg3;

Err:
    return;
}

void rtsp_cmd_queuecmd(struct RTSP* rtsp, int msgno)
{
    StreamCmd strmCmd;

    strmCmd.cmd = msgno;
    strmCmd.arg0 = 0;
    strmCmd.arg1 = 0;
    strmCmd.arg2 = 0;
    strmCmd.arg3 = 0;
    rtsp_cmd_queue(rtsp, &strmCmd);
}

void rtsp_clt_cmdback(struct RTSP* rtsp)
{
    if (rtsp->cmdsn) {
        if (RTSP_MAIN_PLAY(rtsp))
            stream_back_cmd(rtsp->index, rtsp->cmdsn);
        rtsp->cmdsn = 0;
    }
}

void rtsp_clt_state(struct RTSP* rtsp, STRM_STATE state, int scale)
{
    STRM_STATE oldstate = rtsp->state;

    rtsp->state = state;
    rtsp->scale = scale;

    if (RTSP_MAIN_PLAY(rtsp)) {
        if (oldstate == STRM_STATE_ADVERTISE && state == STRM_STATE_PLAY)
            return;
        if (state == STRM_STATE_ADVERTISE) {
            if (oldstate == STRM_STATE_PLAY || oldstate == STRM_STATE_ADVERTISE)
                return;
            state = STRM_STATE_PLAY;
        }
        rtsp_post_state(rtsp, state, scale);

        if (STRM_STATE_PLAY == state && SEEK_END_NORMAL == rtsp->seek_end) {//seek end 时不向上层发送状态消息，避免上层显示混乱
            if (rtsp_clt_advertise_elem(rtsp, -2))
                rtsp_post_msg(rtsp, STRM_MSG_SEEK_END, 1);//存在片尾广告
            else
                rtsp_post_msg(rtsp, STRM_MSG_SEEK_END, 0);
        }
    }
}

static void rtsp_fd_unregist(struct RTSP* rtsp, int fd);
static int rtsp_fd_regist(struct RTSP* rtsp, void* fcc, int fd, fd_recv_f call)
{
    struct RegistFd *rfd;

    //LOG_STRM_PRINTF("#%d fd = %d\n", rtsp->index, fd);

    rtsp_fd_unregist(rtsp, fd);

    if (rtsp->regist_num >= REGIST_FD_NUM)
        LOG_STRM_ERROUT("#%d regist_num = %d\n", rtsp->index, rtsp->regist_num);

    rfd = &rtsp->regist_fd[rtsp->regist_num];
    rtsp->regist_num ++;
    rfd->handle = fcc;
    rfd->fd = fd;
    rfd->recv = call;

    rtsp_clt_fdset(rtsp);
    rtsp->regist_magic ++;

    return 0;
Err:
    return -1;
}

static void rtsp_fd_unregist(struct RTSP* rtsp, int fd)
{
    int i;
    struct RegistFd *rfd;

    //LOG_STRM_PRINTF("#%d fd = %d\n", rtsp->index, fd);

    for (i = 0; i < rtsp->regist_num; i ++) {
        rfd = &rtsp->regist_fd[i];
        if (rfd->fd == fd) {
            rtsp->regist_num --;
            if (rtsp->regist_num > i) {
                char *buf = (char *)&rtsp->regist_fd[i];
                memmove(buf, buf + sizeof(struct RegistFd), sizeof(struct RegistFd) * (rtsp->regist_num - i));
            }
            rtsp_clt_fdset(rtsp);
            rtsp->regist_magic ++;
            return;
        }
    }
}

static int rtsp_fd_datasize(struct RTSP *rtsp)
{
    return 0;
}

static void rtsp_fd_writedata(struct RTSP *rtsp, char *buf, int len)
{
    int ret;
    StrmBuffer* sb = rtsp->rtp_sb;

    if (rtsp->clear_flg)
        return;
    if (len > sb->size)
        LOG_STRM_ERROUT("#%d len = %d / %d\n", rtsp->index, len, sb->size);

    rtsp->fcc_times++;
    IND_MEMCPY(sb->buf, buf, len);
    sb->len = len;
    ret = rtsp_recv_rtp(rtsp);
    if (ret < 0)
        LOG_STRM_ERROUT("#%d rtsp_recv_rtp\n", rtsp->index);

Err:
    return;
}

static int rtsp_fd_writeable(struct RTSP *rtsp)
{
#ifdef INCLUDE_PVR
    if (APP_TYPE_TSTV == rtsp->apptype && STRM_STATE_IPTV != rtsp->pvr_state) {
        int space = strm_record_space(rtsp->strm_record);
        if (space < STREAM_BLOCK_LEVEL_RECORD / 8)
            return 0;
        return 1;
    }
#endif
	if (STREAM_INDEX_PIP == rtsp->index)
	    return 1;
    if (STRM_STATE_PAUSE == rtsp->state)
        return 0;

    if (rtsp->fcc_times < rtsp->fcc_counter)
        return 1;

    return 0;
}

static void rtsp_clt_datastat(struct RTSP *rtsp, char* buf, int len)
{
    if (RTSP_STANDARD_CTC_SHANGHAI == rtsp->standard && 2 == rtsp->fcc_flag) {
        uint32_t seq = 0;
        int hdr = ind_rtp_parse(buf, len, &seq);
        if (hdr > 0)
            rtsp_stat_seq(rtsp, &rtsp->stat.lost_correct, seq);
    }
}

void rtsp_clt_fdset(struct RTSP* rtsp)
{
    int i;
    struct RegistFd *rfd;

    FD_ZERO(&rtsp->rset);

    if (rtsp->cmd_close == 0) {
        rtsp->maxfd = rtsp->msgfd;
        FD_SET((uint32_t)rtsp->msgfd, &rtsp->rset);
    } else {
        rtsp->maxfd = 0;
    }

    if (rtsp->recv_safe != 3 && rtsp->sock != -1) {
        FD_SET((uint32_t)rtsp->sock, &rtsp->rset);
        if (rtsp->sock > rtsp->maxfd)
            rtsp->maxfd = rtsp->sock;
    }
    if (rtsp->data_sock != -1) {
        FD_SET((uint32_t)rtsp->data_sock, &rtsp->rset);
        if (rtsp->data_sock > rtsp->maxfd)
            rtsp->maxfd = rtsp->data_sock;
    }
    if (rtsp->mult_sock != -1) {
        FD_SET((uint32_t)rtsp->mult_sock, &rtsp->rset);
        if (rtsp->mult_sock > rtsp->maxfd)
            rtsp->maxfd = rtsp->mult_sock;
    }

    for (i = 0; i < rtsp->regist_num; i ++) {
        rfd = &rtsp->regist_fd[i];
        //LOG_STRM_PRINTF("#%d fd = %d\n", rtsp->index, rfd->fd);
        FD_SET((uint32_t)rfd->fd, &rtsp->rset);
        if (rfd->fd > rtsp->maxfd)
            rtsp->maxfd = rfd->fd;
    }

    rtsp->maxfd += 1;
}

void rtsp_clt_reset(struct RTSP* rtsp)
{
    uint32_t clk = mid_10ms( );

    rtsp_op_init(rtsp);

    rtsp->clk = clk;

    rtsp->rec_wait = 0;

#ifdef INCLUDE_PVR
    rtsp->pvr_end = 0;
#endif

    rtsp->scale = 1;

    rtsp->url[0] = 0;

    rtsp->psi_view_play = 0;
    rtsp->psi_view_record = 0;
    rtsp->music_flg = -1;

    rtsp->seek_end = SEEK_END_NONE;

    rtsp->err_no = 0;

    rtsp->recv_clk = clk;
    rtsp->recv_times = 0;

    rtsp->op = RTSP_OP_NONE;

    rtsp->retry_flg = 0;
    rtsp->clear_flg = 0;

    rtsp->retry_op = RTSP_OP_NONE;

    rtsp->op_cseq = 0;
    rtsp->ig_cseq = 0;

    rtsp->rtsp_code = 0;
    rtsp->rtsp_state = RTSP_STATE_TEARDOWN;
    if (rtsp->transport == 1)
        rtsp->transtype = SOCK_STREAM;
    else
        rtsp->transtype = SOCK_DGRAM;

    rtsp->sock = -1;
    rtsp->data_sock = -1;
    rtsp->rtcp_sock = -1;
    rtsp->mult_sock = -1;

    rtsp->data_port = 0;

    rtsp->Session[0] = 0;
    if (rtsp->standard != RTSP_STANDARD_CTC_SHANGHAI || rtsp->CSeq >= 10000)
        rtsp->CSeq = 0;
    rtsp->trackID = 0;
    rtsp->ContentBase[0] = 0;

    rtsp->mult_save = 0;
    memset(&rtsp->mult_sin, 0, sizeof(rtsp->mult_sin));
    memset(&rtsp->peername, 0, sizeof(rtsp->peername));
    rtsp->peername.family = AF_INET;
    memset(&rtsp->sockname, 0, sizeof(rtsp->sockname));
    rtsp->sockname.family = AF_INET;
    memset(&rtsp->playname, 0, sizeof(rtsp->playname));
    rtsp->playname.family = AF_INET;

    rtsp->timeshift_status = 0;
    rtsp->timeshift_support = 0;

    rtsp->timeshift_len = 0;
    rtsp->timeshift_real = 0;

    rtsp->ctc_begin = -1;

    rtsp->ppv_begin = 0;
    rtsp->ppv_end = 0;

    rtsp->time_begin = -1;
    rtsp->time_length = 0;
    rtsp->time_start = 0;

    rtsp->servtime_sync = 0;
    rtsp->servtime_diff = 0;

    rtsp->signal_flg = 1;

    rtsp->clk_full = clk;
    rtsp->clk_empty = clk;

    rtsp->post_clk = 0;

    rtsp->rtp_seq = RTP_INVALID_SEQ;

    {
        RTSPStat_t stat = &rtsp->stat;

        stat->stat_clock = clk;
        stat->stat_bytes = 0;
        stat->stat_bitrate = 0;

        stat->stat_vodreq = 0;
        stat->stat_vodstop = 0;
        stat->stat_vodpause = 0;
        stat->stat_vodplay = 0;

        rtsp_stat_reset(&stat->lost_rtp, 1);
        rtsp_stat_reset(&stat->lost_correct, 1);
    }

    rtsp->ts_len = 0;

    rtsp->rtp_flag = 0;

    rtsp->strmRRS.rrs_num = 0;

    rtsp->regist_num = 0;
    rtsp->regist_magic = 0;

    rtsp->fcc_handle = NULL;
    rtsp->fcc_timeout = -1;

    rtsp->burst_clk = 0;
    rtsp->arq_clk = 0;

    rtsp->postvalid = -1;
    rtsp->bitrate = 0;
    rtsp->cmdsn = 0;

    rtsp->iptv_pause = 0;

    srand(time(NULL));
    rtsp->rtcp_ssrc = rand() % 0xffffffff;
    rtsp->rtcp_seq = RTP_INVALID_SEQ;
    rtsp->rtcp_clk = 0;
    rtsp->rtcp_lostnum = 0;
    rtsp->rtcp_lostblp = 0;    
    rtsp->rtcp_lostseq = 0;
    rtsp->rtcp_handle = NULL;
}

void rtsp_clt_save(void* arg)
{
    int idx = 0;
    struct ind_sin sin;

    char buf[IND_ADDR_LEN];
    struct RTSP *rtsp = (struct RTSP *)arg;

    if (rtsp->open_play == OPEN_PLAY_CLOSE)
        return;
    if (rtsp->index == STREAM_INDEX_PIP)
        idx = 1;

    rtsp->mult_save = 1;

    stream_port_multicast_read(idx, buf);

    ind_net_pton(buf, &sin);
    if(ind_net_equal(&sin, &rtsp->mult_sin) == 0) {
        LOG_STRM_PRINTF("#%d family = %d, port = %d\n", rtsp->index, rtsp->mult_sin.family, rtsp->mult_sin.port);
        ind_net_ntop(&rtsp->mult_sin, buf, IND_ADDR_LEN);
        stream_port_multicast_write(idx, buf);
    }
}

int rtsp_clt_parse_iptv(struct RTSP* rtsp, char* channel_url, char* tmshift_url)
{
    int l;

    rtsp->channel_url[0] = 0;
    rtsp->tmshift_url[0] = 0;
    if (tmshift_url && tmshift_url[0] == 0)
        tmshift_url = NULL;

    if (strncasecmp(channel_url, "igmp://", 7) == 0) {
        channel_url += 7;
        l = ind_net_pton(channel_url, &rtsp->mult_sin);
        if (l <= 0)
            LOG_STRM_ERROUT("#%d parse addr ret = %d\n", rtsp->index, l);
        channel_url += l;

        if (channel_url[0] == ':')
            LOG_STRM_ERROUT("#%d ind_net_aton\n", rtsp->index);

        if (rtsp->multicast_unicast == 0)
            channel_url[0] = 0;
        if (channel_url[0] == 0) {
            rtsp->iptvtype = IPTV_TYPE_IGMP;
        } else {
            rtsp->iptvtype = IPTV_TYPE_MULTICAST;
            if (channel_url[0] != '|' && channel_url[0] != '^')
                LOG_STRM_ERROUT("#%d *url = %c\n", rtsp->index, channel_url[0]);
            channel_url += 1;
        }
    }

    if (channel_url[0] != 0)
        IND_STRCPY(rtsp->channel_url, channel_url);

    if (tmshift_url && rtsp_clt_parse_url(rtsp, tmshift_url)) {
        LOG_STRM_ERROR("#%d tmshift_url\n", rtsp->index);
        tmshift_url = NULL;
    }

    if (tmshift_url) {
        IND_STRCPY(rtsp->tmshift_url, tmshift_url);
    } else if (rtsp->trickmode) {
        LOG_STRM_ERROR("#%d tmshift_url is NULL\n", rtsp->index);
        rtsp->trickmode = 0;
    }

    //下面代码放在 rtsp_clt_parse_url(rtsp, tmshift_url) 之前就会导致单播频道将时移地址做单播使用 by liujianhua 2013-8-29 21:33:12
    if (rtsp->iptvtype == IPTV_TYPE_UNICAST) {
        if (0 == channel_url[0] || rtsp_clt_parse_url(rtsp, channel_url))
            LOG_STRM_ERROUT("#%d channel_url\n", rtsp->index);
    }

    return 0;
Err:
    return -1;
}

int rtsp_clt_parse_url(struct RTSP* rtsp, char* url)
{
    if (strncasecmp(url, "rtspu://", 8) == 0) {
        rtsp->transtype = SOCK_DGRAM;
        url += 8;
    } else if (strncasecmp(url, "rtspt://", 8) == 0) {
        rtsp->transtype = SOCK_STREAM;
        url += 8;
    } else {
        if (strncasecmp(url, "rtsp://", 7))
            LOG_STRM_ERROUT("#%d url\n", rtsp->index);
        url += 7;
    }

    if (strm_tool_parse_url(url, &rtsp->strmRRS))
        LOG_STRM_ERROUT("#%d strm_tool_parse_url\n", rtsp->index);

    rtsp->strmRRS.rrs_idx = 0;

    return 0;
Err:
    rtsp->rtsp_code = RTSP_CODE_URL_FORMAT_Error;
    LOG_STRM_ERROR("#%d RTSP_CODE_URL_FORMAT_Error\n", rtsp->index);
    return -1;
}

void rtsp_clt_make_url(struct RTSP* rtsp)
{
    ind_sin_t sin;
    StrmRRS_t strmRRS = &rtsp->strmRRS;

    sin = &strmRRS->rrs_sins[strmRRS->rrs_idx];
    if (0 == sin->port)
        sprintf(rtsp->url, "rtsp://%s/%s", rtsp_clt_addr_fmt(rtsp, sin), strmRRS->rrs_uri);
    else
        sprintf(rtsp->url, "rtsp://%s:%hd/%s", rtsp_clt_addr_fmt(rtsp, sin), sin->port, strmRRS->rrs_uri);
}

static void rtsp_clt_ret_cache_on(struct RTSP *rtsp)
{
    uint32_t clk = rtsp->clk;

    LOG_STRM_DEBUG("#%d open_play = %d, state = %d, burst = %d\n", rtsp->index, rtsp->open_play, rtsp->state, rtsp->burst_flag);
    if (OPEN_PLAY_END == rtsp->open_play || STRM_STATE_PLAY != rtsp->state)
        return;

        if (rtsp->burst_flag == 3 && rtsp->op == RTSP_OP_NONE) {
            if (rtsp->burst_clk < clk && rtsp->state == STRM_STATE_PLAY) {
                rtsp->burst_clk = clk + TIMEOUT_CLK_BURST;
                rtsp_cmd_push(rtsp, STREAM_CMD_INTERNAL, RTSP_CMD_RTSP_BURST_UP);
            }
        } else {
            if (rtsp->ret_clk_cache_on < clk) {
                LOG_STRM_PRINTF("#%d STRM_MSG_BUF_EMPTY\n", rtsp->index);
                rtsp_msg_back(rtsp, STRM_MSG_BUF_EMPTY, 0);
                rtsp->ret_clk_cache_on = clk + INTERVAL_CLK_MESSAGE;
            }
        }
    rtsp->ret_clk_cache_off = 0;
}

static void rtsp_clt_ret_cache_off(struct RTSP *rtsp)
{
    uint32_t clk = rtsp->clk;

    if (rtsp->ret_clk_cache_off < clk) {
        LOG_STRM_PRINTF("#%d STRM_MSG_BUF_EMPTY\n", rtsp->index);
        rtsp_msg_back(rtsp, STRM_MSG_BUF_FULL, 0);
        rtsp->ret_clk_cache_off = clk + INTERVAL_CLK_MESSAGE;
    }

    rtsp->ret_clk_cache_on = 0;
}

void rtsp_clt_fcc_reset(struct RTSP* rtsp)
{
    int space;

    space = strm_play_space(rtsp->strm_play);
    rtsp->fcc_counter = space / RTP_BUFFER_LENGTH;
    if (rtsp->fcc_counter <= 16)
        rtsp->fcc_counter = 0;
    else
        rtsp->fcc_counter -= 16;

    rtsp->fcc_times = 0;
}

void rtsp_clt_ret_open(struct RTSP* rtsp)
{
    struct RETArg arg;

    LOG_STRM_PRINTF("#%d open_play = %d, ret_flag = %d\n", rtsp->index, rtsp->open_play, rtsp->ret_flag);
    if (rtsp->open_play == OPEN_PLAY_CLOSE || rtsp->ret_flag < 2)
        return;

    rtsp_clt_fcc_reset(rtsp);
    rtsp->ret_cache_iptv = 0;
    if (rtsp->ret_flag > 2) {
        ret_port_reset(rtsp->ret_handle);
        rtsp->ret_flag = 3;
        return;
    }

    memset(&arg, 0, sizeof(arg));
    arg.rtsp = rtsp;
    arg.sin = rtsp->serv_sin;
    arg.bitrate = rtsp->bitrate;
    arg.fd_regist = rtsp_fd_regist;
    arg.fd_unregist = rtsp_fd_unregist;
    arg.fd_datasize = rtsp_fd_datasize;
    arg.fd_writedata = rtsp_fd_writedata;
    arg.fd_writeable = rtsp_fd_writeable;

    arg.clt_port = rtsp->sockname.port + 1;
    arg.srv_port = rtsp->peername.port + 1;
    arg.cache_on = rtsp_clt_ret_cache_on;
    arg.cache_off = rtsp_clt_ret_cache_off;

    rtsp->ret_clk_cache_on = 0;
    rtsp->ret_clk_cache_off = 0;

    rtsp->ret_handle = ret_port_open(&arg);
    if (rtsp->ret_handle) {
        rtsp->ret_flag = 3;
    } else {
        LOG_STRM_ERROR("#%d ret_port_open\n", rtsp->index);
        rtsp->ret_flag = 1;

        if (rtsp->open_play && rtsp->index != STREAM_INDEX_PIP) {
            if (APP_TYPE_VOD == rtsp->apptype && rtsp->index < STREAM_INDEX_PIP)
                int_steam_setToalBufferSize(rtsp->size_cache);
            strm_play_resize(rtsp->strm_play, rtsp->index, RTP_BUFFER_LENGTH, rtsp->size_cache);
        }
    }

}

void rtsp_clt_ret_close(struct RTSP* rtsp)
{
    LOG_STRM_DEBUG("#%d ret_flag = %d\n", rtsp->index, rtsp->ret_flag);
    if (rtsp->ret_flag >= 3) {
        ret_port_close(rtsp->ret_handle);
        rtsp->ret_handle = NULL;
        rtsp->ret_flag = 2;
    }
}

void rtsp_clt_arq_ctc_callback(struct RTSP* rtsp, uint8_t *buf, uint32_t len)
{
    uint32_t clk = rtsp->clk;

    if (rtsp == NULL)
        LOG_STRM_ERROUT("rtsp is NULL\n");
    if (rtsp == NULL || len <= 0 || len > ARQ_SEQ_LEN)
        LOG_STRM_ERROUT("#%d, len = %d\n", rtsp->index, len);
    if (rtsp->arq_flag < 2)
        LOG_STRM_ERROUT("#%d, arq_flag = %d\n", rtsp->index, rtsp->arq_flag);

    if (rtsp->arq_clk < clk) {
        rtsp->arq_ctc_len = len;
        IND_MEMCPY(rtsp->arq_ctc_seqbuf, buf, rtsp->arq_ctc_len);
        rtsp->arq_clk = clk + TIMEOUT_CLK_ARQ;
        rtsp_cmd_push(rtsp, STREAM_CMD_INTERNAL, RTSP_CMD_RTSP_ARQ);
    }
Err:
    return;
}

void rtsp_clt_rtcp_callback(struct RTSP* rtsp, uint8_t *buf, uint32_t len)
{
    if (rtsp == NULL)
        ERR_OUT("rtsp is NULL\n");
    if (rtsp == NULL || len <= 0 || len > ARQ_SEQ_LEN)
        ERR_OUT("#%d, len = %d\n", rtsp->index, len);
    if (rtsp->rtcp_flag < 2)
        ERR_OUT("#%d, arq_flag = %d\n", rtsp->index, rtsp->arq_flag);

    //rtsp_op_rtcpapp(rtsp);
Err:
    return;
}

void rtsp_clt_arq_open(struct RTSP* rtsp)
{
    struct RETArg arg;

    LOG_STRM_PRINTF("#%d open_play = %d, arq_flag = %d\n", rtsp->index, rtsp->open_play, rtsp->arq_flag);
    if (rtsp->open_play == OPEN_PLAY_CLOSE || rtsp->arq_flag < 2)
        return;

    rtsp_clt_fcc_reset(rtsp);
    rtsp->ret_cache_iptv = 0;
    if (rtsp->arq_flag >= 3 && rtsp->arq_ctc_handle) {
        arq_port_reset(rtsp->arq_ctc_handle);
        return;
    }

    memset(&arg, 0, sizeof(arg));
    arg.rtsp = rtsp;
    arg.sin = rtsp->serv_sin;
    arg.bitrate = rtsp->bitrate;
    arg.fd_regist = rtsp_fd_regist;
    arg.fd_unregist = rtsp_fd_unregist;
    arg.fd_datasize = rtsp_fd_datasize;
    arg.fd_writedata = rtsp_fd_writedata;
    arg.fd_writeable = rtsp_fd_writeable;

    arg.clt_port = 0;
    arg.srv_port = 0;

    arg.cache_on = rtsp_clt_ret_cache_on;
    arg.cache_off = rtsp_clt_ret_cache_off;

    arg.rtsp_pktfb = rtsp_clt_arq_ctc_callback;

    rtsp->ret_clk_cache_on = 0;
    rtsp->ret_clk_cache_off = 0;

    rtsp->arq_ctc_handle = arq_port_open(&arg);
    if (rtsp->arq_ctc_handle) {
        rtsp->arq_flag = 3;
    } else {
        LOG_STRM_ERROR("#%d arq_port_open\n", rtsp->index);
        rtsp->arq_flag = 1;
    }
}

void rtsp_clt_arq_close(struct RTSP* rtsp)
{
    LOG_STRM_DEBUG("#%d arq_flag = %d\n", rtsp->index, rtsp->arq_flag);
    if (rtsp->arq_flag >= 3) {
        arq_port_close(rtsp->arq_ctc_handle);
        rtsp->arq_ctc_handle = NULL;
        rtsp->arq_flag = 2;
    }
}

void rtsp_clt_rtcpfb(struct RTSP* rtsp)
{
    if(rtsp->clk - rtsp->rtcp_clk > 30 || rtsp->rtcp_clk == 0) {
        rtsp->rtcp_clk = mid_10ms();
        rtsp_op_rtcpfb(rtsp);
    }
}

void rtsp_clt_rtcpapp(void* arg)
{
    struct RTSP *rtsp = (struct RTSP *)arg;
    rtsp_cmd_push(rtsp, STREAM_CMD_INTERNAL, RTSP_CMD_RTSP_RTCPAPP);
}

void rtsp_clt_rtcp_open(struct RTSP* rtsp)
{
    struct RETArg arg;

    PRINTF("#%d open_play = %d, rtcp_flag  = %d, rtcp_handle = %p\n", rtsp->index, rtsp->open_play, rtsp->rtcp_flag, rtsp->rtcp_handle);
    if (rtsp->open_play == OPEN_PLAY_CLOSE || rtsp->rtcp_flag < 2)
        return;
    rtsp->ret_cache_iptv = 0;
    if (rtsp->rtcp_flag >= 2 && rtsp->rtcp_handle) {
        rtcp_port_reset(rtsp->rtcp_handle);
        return;
    }

    memset(&arg, 0, sizeof(arg));
    arg.rtsp = rtsp;
    arg.sin = rtsp->serv_sin;
    arg.bitrate = rtsp->bitrate;
    arg.fd_regist = rtsp_fd_regist;
    arg.fd_unregist = rtsp_fd_unregist;
    arg.fd_datasize = rtsp_fd_datasize;
    arg.fd_writedata = rtsp_fd_writedata;
    arg.fd_writeable = rtsp_fd_writeable;

    rtsp_clt_fcc_reset(rtsp);

    arg.clt_port = 0;
    arg.srv_port = 0;
    arg.cache_on = NULL;
    arg.cache_off = NULL;

    //arg.rtsp_pktfb = rtsp_clt_rtcp_callback;
    arg.rtsp_pktfb = NULL;

    rtsp->rtcp_handle = rtcp_port_open(&arg);

    rtsp->rtcp_flag = 3;
}

void rtsp_clt_rtcp_close(struct RTSP* rtsp)
{
    DBG_PRN("#%d rtcp_flag = %d\n", rtsp->index, rtsp->rtcp_flag);
    if (rtsp->rtcp_flag >= 2) {
        rtcp_port_close(rtsp->rtcp_handle);
        rtsp->rtcp_handle = NULL;
        rtsp->rtcp_flag = 1;
    }
}

void rtsp_clt_reset_play(struct RTSP* rtsp, int caReset)
{
    strm_bufOrder_reset(rtsp->strm_bufOrder);
    strm_play_reset(rtsp->strm_play, rtsp->index, caReset);
}


int rtsp_multicast_play(struct RTSP* rtsp, u_short port)
{
    int sock;
    struct ind_sin mult_sin = rtsp->mult_sin;

    mult_sin.port = port;
    if (rtsp->igmp_play_call) {
        if (rtsp->igmp_size > 0)
            sock = rtsp->igmp_play_call(rtsp->igmp, rtsp->igmp_size, &mult_sin);
        else
            sock = rtsp->igmp_play_call(NULL, 0, &mult_sin);
    } else {
        sock = stream_port_multicast_play(mult_sin.in_addr.s_addr, port);
    }

    return sock;
}

void rtsp_multicast_stop(struct RTSP* rtsp, int sock)
{
    struct ind_sin mult_sin = rtsp->mult_sin;

    if (rtsp->igmp_play_call) {
        if (rtsp->igmp_size > 0)
            rtsp->igmp_stop_call(rtsp->igmp, rtsp->igmp_size, sock, &mult_sin);
        else
            rtsp->igmp_stop_call(NULL, 0, sock, &mult_sin);
    } else {
        stream_port_multicast_stop(sock, mult_sin.in_addr.s_addr);
    }
}

int rtsp_clt_iptv_open(struct RTSP* rtsp)
{
    rtsp_clt_post_valid(rtsp, 0);

    {//igmp info
        char *buf = rtsp->igmp_info;
        int len = sprintf(buf, "igmp://");
        len += ind_net_ntop_ex(&rtsp->mult_sin, buf + len, IGMP_INFO_LEN);

        if (rtsp->call_rtspinfo)
            rtsp->call_rtspinfo(1, buf, len);
    }

    LOG_STRM_PRINTF("#%d %s, fcc_flag = %d, igmp_play_call = %p\n", rtsp->index, rtsp->igmp_info, rtsp->fcc_flag, rtsp->igmp_play_call);

    rtsp_clt_ret_close(rtsp);
    rtsp_clt_arq_close(rtsp);
    rtsp_clt_rtcp_close(rtsp);

    rtsp_clt_fcc_reset(rtsp);

    if (rtsp->fcc_flag) {
        struct FCCArg arg;

        memset(&arg, 0, sizeof(arg));
        arg.rtsp = rtsp;
        arg.type = rtsp->fcc_type;
        arg.flag = rtsp->psi_view_play;

        arg.sin = rtsp->mult_sin;

        arg.fd_regist = rtsp_fd_regist;
        arg.fd_unregist = rtsp_fd_unregist;
        arg.fd_datasize = rtsp_fd_datasize;
        arg.fd_writedata = rtsp_fd_writedata;
        arg.fd_writeable = rtsp_fd_writeable;

        arg.rtsp_datastat = rtsp_clt_datastat;

        if (rtsp->index == STREAM_INDEX_PIP)
            arg.pip = 1;
        else
            arg.pip = 0;
        rtsp->fcc_handle = fcc_port_open(&arg);
        if (rtsp->fcc_handle) {
            rtsp->fcc_flag = 2;
            if (rtsp->index < STREAM_INDEX_PIP && rtsp->multicast_unicast != 0 && IPTV_TYPE_MULTICAST == rtsp->iptvtype && -1 == rtsp->fcc_timeout)
                rtsp->fcc_timeout = 0;
        } else {
            LOG_STRM_ERROR("#%d fcc_port_open\n", rtsp->index);
            rtsp->fcc_flag = 0;
        }
    }
    if (!rtsp->fcc_flag) {
        uint16_t port = rtsp->mult_sin.port;
        rtsp->mult_sock = rtsp_multicast_play(rtsp, port);
    }

    rtsp_clt_fdset(rtsp);

    rtsp->mult_flag = 1;

    if (rtsp->open_play) {
        int idx = 0;
        if (STREAM_INDEX_PIP == rtsp->index)
            idx = 1;
        int_back_rtspURL(idx, rtsp->igmp_info);
        int_steam_setTransportProtocol(idx, RTSP_TRANSPORT_RTP_UDP);

        if (STREAM_INDEX_PIP != rtsp->index)
            rtsp_clt_reset_play(rtsp, 1);
        rtsp_stat_set(rtsp, 1);

        if (rtsp->iframe_flag == 1)
            strm_play_tplay(rtsp->strm_play, rtsp->index, -64);
        else
            strm_play_resume(rtsp->strm_play, rtsp->index, 1);
        strm_play_set_idle(rtsp->strm_play, rtsp->index, 0);
    }

    if (rtsp->open_play)
        rtsp->post_clk = rtsp->clk;

    if (rtsp->open_play == OPEN_PLAY_END)
        rtsp->open_play = OPEN_PLAY_RUNNING;

    if (rtsp->mult_save == 0)
        ind_timer_create(rtsp->tlink, rtsp->clk + INTERVAL_CLK_SAVE, 0, rtsp_clt_save, rtsp);

    if (rtsp->cmdsn == 1)    //第一次播放，表面打开成功
        rtsp_clt_cmdback(rtsp);

    if (rtsp->state != STRM_STATE_IPTV) {
        LOG_STRM_PRINTF("#%d STRM_STATE_IPTV\n", rtsp->index);
        rtsp_clt_state(rtsp, STRM_STATE_IPTV, 1);
    }

    rtsp_clt_post_valid(rtsp, 1);

    return 0;
}

int rtsp_clt_iptv_close(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    rtsp_clt_post_valid(rtsp, 0);

    if (rtsp->open_play)
        rtsp_stat_set(rtsp, 0);

    if (rtsp->mult_save == 0)
        ind_timer_delete(rtsp->tlink, rtsp_clt_save, rtsp);

    rtsp->fcc_timeout = -2;
    if (rtsp->fcc_flag == 2) {
        if (rtsp->fcc_handle) {
            fcc_port_close(rtsp->fcc_handle);
            rtsp->fcc_handle = NULL;
        }
        rtsp->fcc_flag = 1;
    } else {
        if (rtsp->mult_sock != -1) {
            rtsp_multicast_stop(rtsp, rtsp->mult_sock);
            rtsp->mult_sock = -1;
        }
    }

    rtsp->mult_flag = 0;
    rtsp_clt_fdset(rtsp);

    return 0;
}

void rtsp_clt_resume(struct RTSP* rtsp)
{
    rtsp->voddelay_clk = 0;
    if (STRM_STATE_PLAY == rtsp->state && rtsp->psi_view_play && rtsp->voddelay_level > 0) {
        int diff = strm_play_buffer(rtsp->strm_play);
        if (diff < 0)
            diff = 0;
        if (diff < rtsp->voddelay_level)
            rtsp->voddelay_clk = rtsp->clk + 100 + (rtsp->voddelay_level - diff);
    }
    if (rtsp->voddelay_clk)
        strm_play_pause(rtsp->strm_play, rtsp->index);
    else
        strm_play_resume(rtsp->strm_play, rtsp->index, 0);
}

static void rtsp_clt_recv_check(struct RTSP* rtsp, uint32_t clk)
{
    int length, space, timeBuffer, byterate;

    if (rtsp->index == STREAM_INDEX_PIP)
        return;

    space = strm_play_space(rtsp->strm_play);
    length = strm_play_length(rtsp->strm_play);
    if (rtsp->recv_safe == 3 && length < space / 3) {
        LOG_STRM_PRINTF("#%d recv unsafe\n", rtsp->index);
        rtsp->recv_safe = 1;
        rtsp_clt_fdset(rtsp);
    }

    if (rtsp->ret_flag >= 2)
        return;

    //2012-3-19 14:28:11 by ljh 依据华为新规范，支持时移快速缓冲，去掉rtsp->apptype != APP_TYPE_VOD的限制
    if (rtsp->state != STRM_STATE_PLAY)
        return;

    if (rtsp->cache == CACHE_STATE_UNSPPORT || rtsp->op != RTSP_OP_NONE)
        return;

    timeBuffer = rtsp->time_buffer;
    if (rtsp->cache == CACHE_STATE_ON) {
        byterate = strm_play_byte_rate(rtsp->strm_play);
        if (byterate + byterate >= space && rtsp->cache_clk_level > timeBuffer - 200) {
            LOG_STRM_WARN("#%d byterate = %d, space = %d, level = %d, timeBuffer = %d\n", rtsp->index, byterate, space, rtsp->cache_clk_level, timeBuffer);
            rtsp->cache_clk_level = timeBuffer - 200;
            rtsp->cache_clk_range = 200;
        }
    }

    if (timeBuffer >= rtsp->cache_clk_level + rtsp->cache_clk_range) {
        if (rtsp->cache == CACHE_STATE_ON && rtsp->clk_full < clk) {
            LOG_STRM_PRINTF("#%d EP_RNG_BUF_NOMAL timeBuffer = %d, length = %d, cache_off = %d\n", rtsp->index, timeBuffer, length, rtsp->cache_clk_level + rtsp->cache_clk_range);
            rtsp_msg_back(rtsp, STRM_MSG_BUF_FULL, 0);
            rtsp->clk_full = clk + INTERVAL_CLK_MESSAGE;
            rtsp->clk_empty = clk;
        }
        return;
    }
    if (timeBuffer <= rtsp->cache_clk_level) {
        if ((rtsp->cache == CACHE_STATE_OFF_INIT || rtsp->cache == CACHE_STATE_OFF) && rtsp->clk_empty < clk) {
            LOG_STRM_PRINTF("#%d EP_RNG_BUF_EMPTY timeBuffer = %d, length = %d, cache_on = %d\n", rtsp->index, timeBuffer, length, rtsp->cache_clk_level);
            rtsp_msg_back(rtsp, STRM_MSG_BUF_EMPTY, 0);
            rtsp->clk_empty = clk + INTERVAL_CLK_MESSAGE;
            rtsp->clk_full = clk;
        }
    }
}

void rtsp_clt_ext_100ms(struct RTSP *rtsp)
{
    double timeNow;

    if (3 == rtsp->arq_flag)
        arq_port_100ms(rtsp->arq_ctc_handle);

    if (2 == rtsp->fcc_flag)
        fcc_port_100ms(rtsp->fcc_handle);

    if (3 == rtsp->rtcp_flag)
        rtcp_port_100ms(rtsp->rtcp_handle);

    if (3 == rtsp->ret_flag) {
        int len;

        ret_port_100ms(rtsp->ret_handle);

        if (rtsp->open_play == OPEN_PLAY_END || rtsp->data_clk + 50 < rtsp->clk/* 断流时播放完缓冲里面数据 */) {
            timeNow = int_timeNow( );
            for (;;) {
                if (rtsp->fcc_times >= rtsp->fcc_counter)
                    break;
                len = ret_port_pop(rtsp->ret_handle);
                if (len <= 0) {
                    if (OPEN_PLAY_END == rtsp->open_play) {
                        LOG_STRM_PRINTF("#%d ret_port_pop\n", rtsp->index);
                        strm_play_end(rtsp->strm_play, rtsp->index);
                        rtsp->ret_flag = 4;
                    }
                    break;
                }
            }
        }
    }

    rtsp_clt_fcc_reset(rtsp);
}

#ifdef INCLUDE_PVR
static void rtsp_clt_tstv_read(struct RTSP* rtsp, uint32_t clk)
{
    int ret;
    StrmBuffer *sb;

    if (NULL == rtsp->pvr)
        return;

    if (STRM_STATE_PAUSE == rtsp->pvr_state && 0 == rtsp->pvr_smooth_pause)
        return;

    for (;;) {
        if (strm_play_space(rtsp->strm_play) < TS_TIMESHIFT_SIZE)
            break;
        sb = rtsp->pvr_sb;
        sb->off = 0;
        if (rtsp->pvr_state == STRM_STATE_PLAY)
            ret = ind_pvr_read(rtsp->pvr, clk * 10 / 9, sb->buf, TS_TIMESHIFT_SIZE);//匈牙利一线出现开始播放卡顿，所以暂时不用
        else
            ret = ind_pvr_read(rtsp->pvr, clk, sb->buf, TS_TIMESHIFT_SIZE);

        if (ret > 0) {
            if (ret % 188) {
                LOG_STRM_WARN("#%d ret = %d\n", rtsp->index, ret);
            } else {
                int i;
                for (i = 0; i < ret; i += 188) {
                    if (sb->buf[i] != 0x47)
                        LOG_STRM_WARN("#%d buf[%d] sync!\n", rtsp->index, i);
                }
            }
            sb->len = ret;
            strm_play_push(rtsp->strm_play, rtsp->index, &rtsp->pvr_sb);
            continue;
        }
        switch(ret) {
        case PVR_ANNOUNCE_NONE:
            //LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_NONE\n", rtsp->index);
            return;
        case PVR_ANNOUNCE_WRITE:
            if (STRM_STATE_PAUSE == rtsp->pvr_state)
                return;
            if (rtsp->pvr_scale != 1) {
                LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_WRITE > STRM_MSG_STREAM_END\n", rtsp->index);
                rtsp_msg_back(rtsp, STRM_MSG_STREAM_END, 0);
                rtsp->pvr_end = 1;
            }
            return;
        case PVR_ANNOUNCE_BEGIN:
            LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_BEGIN > STRM_MSG_STREAM_BEGIN\n", rtsp->index);
            rtsp_msg_back(rtsp, STRM_MSG_STREAM_BEGIN, 0);
            return;
        case PVR_ANNOUNCE_END:
            LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_END\n", rtsp->index);
            strm_play_end(rtsp->strm_play, rtsp->index);
            rtsp->pvr_end = 1;
            return;
        case PVR_ANNOUNCE_ERROR:
            LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_ERROR\n", rtsp->index);
            rtsp_msg_back(rtsp, STRM_MSG_OPEN_ERROR, 0);
            return;
        default:
            return;
        }
    }
}
#endif

void rtsp_clt_period_100ms(void* arg)
{
    uint32_t clk;
    struct RTSP *rtsp = (struct RTSP *)arg;

    if (rtsp->state != STRM_STATE_CLOSE && rtsp->state != STRM_STATE_OPEN) {
    #ifdef INCLUDE_PVR
        if (rtsp->open_shift || rtsp->rec_mix)
            strm_record_push(rtsp->strm_record, NULL, 0, rtsp->clk);
    #endif

        if (rtsp->open_play) {
            rtsp->time_buffer = strm_play_diff(rtsp->strm_play) + strm_play_buffer(rtsp->strm_play);
            if (rtsp->apptype != APP_TYPE_TSTV && STRM_STATE_FAST == rtsp->state)
                int_clt_time_fast(rtsp, 1);
        }

        if (STRM_STATE_PLAY == rtsp->state && rtsp->voddelay_clk) {
            if (rtsp->open_play == OPEN_PLAY_END || rtsp->voddelay_clk < rtsp->clk || rtsp->time_buffer >= rtsp->voddelay_level) {
                LOG_STRM_PRINTF("#%d open_play = %d/%d, clk = %u/%u, diff = %d/%d\n", rtsp->index, rtsp->open_play, rtsp->ret_flag, rtsp->voddelay_clk, rtsp->clk, rtsp->time_buffer, rtsp->voddelay_level);
                rtsp->voddelay_clk = 0;
                strm_play_resume(rtsp->strm_play, rtsp->index, 0);
                if (rtsp->open_play == OPEN_PLAY_END && 3 != rtsp->ret_flag)
                    strm_play_end(rtsp->strm_play, rtsp->index);
            }
        }

        rtsp_clt_ext_100ms(rtsp);

        if (rtsp->clear_flg == 0
            && rtsp->open_play
            && rtsp->state != STRM_STATE_PAUSE
            && rtsp->state != STRM_STATE_ADVERTISE) {

            if (rtsp->open_play == OPEN_PLAY_RUNNING || rtsp->open_play == OPEN_PLAY_TIMESHIFT) {
                clk = rtsp->clk;
                rtsp_clt_recv_check(rtsp, clk);

#ifdef INCLUDE_PVR
                if (APP_TYPE_TSTV == rtsp->apptype && STRM_STATE_IPTV != rtsp->pvr_state && rtsp->pvr_end == 0)
                    rtsp_clt_tstv_read(rtsp, clk);
#endif

                if (rtsp->burst_flag == 3 && rtsp->ret_flag < 3 && rtsp->state == STRM_STATE_PLAY && rtsp->op == RTSP_OP_NONE) {
                    if (rtsp->burst_clk < clk) {
                        if (rtsp->time_buffer <= CACHE_CLK_LEVEL) {
                            int size, bytrate;
                            bytrate = rtsp->stat.stat_bitrate / 8;
                            size = strm_play_space(rtsp->strm_play);
                            LOG_STRM_PRINTF("#%d time_buffer = %d, size = %d, bytrate = %d\n", rtsp->index, rtsp->time_buffer, size, bytrate);
                            rtsp->burst_clk = clk + TIMEOUT_CLK_BURST;
                            if (size > bytrate)
                                rtsp_cmd_push(rtsp, STREAM_CMD_INTERNAL, RTSP_CMD_RTSP_BURST_UP);
                        /* 去掉BURST DOWN，避免出现抖动
                        } else if (rtsp->time_buffer >= CACHE_CLK_LEVEL + CACHE_CLK_RANGE) {
                            LOG_STRM_PRINTF("#%d time_buffer = %d\n", rtsp->index, rtsp->time_buffer);
                            rtsp->burst_clk = clk + TIMEOUT_CLK_BURST;
                            rtsp_cmd_push(rtsp, STREAM_CMD_INTERNAL, RTSP_CMD_RTSP_BURST_DOWN);
                         */
                        } else {
                            rtsp->burst_clk += TIMEOUT_CLK_BURST_INT;
                        }
                    }
                }
            }
        }
    }

    if (RTSP_OP_RESOLVE == rtsp->op) {
        uint32_t ip;
        StrmRRS_t strmRRS = &rtsp->strmRRS;

        ip = strm_tool_dns_find(strmRRS->rrs_name);
        if (INADDR_ANY != ip) {
            if (INADDR_NONE == ip) {
                rtsp_op_failed(rtsp);
            } else {
                strmRRS->rrs_sins[0].in_addr.s_addr = ip;
                rtsp_op_succeed(rtsp);
            }
        }
    }
}

void rtsp_clt_heartbit(void* arg)
{
    struct RTSP *rtsp = (struct RTSP *)arg;
    rtsp_cmd_push(rtsp, STREAM_CMD_INTERNAL, RTSP_CMD_RTSP_HEARTBIT);
}

void rtsp_clt_natheartbit(void* arg)
{
    struct RTSP *rtsp = (struct RTSP *)arg;
    rtsp_cmd_push(rtsp, STREAM_CMD_INTERNAL, RTSP_CMD_RTSP_NATHEARTBIT);
}

int rtsp_clt_trickmode(struct RTSP* rtsp)
{
    if (rtsp->trickmode)
        return 1;

    if (rtsp->apptype <= APP_TYPE_IPTV2)
        rtsp_post_state(rtsp, rtsp->state, rtsp->scale);
    rtsp_post_msg(rtsp, STRM_MSG_UNSUPPORT_OP, 0);
    LOG_STRM_WARN("#%d app = %d, trickmode = %d\n", rtsp->index, rtsp->apptype, rtsp->trickmode);
    return 0;
}

void rtsp_clt_shiftcheck(void* arg)
{
    int sec, begin;
    struct RTSP *rtsp = (struct RTSP *)arg;

    sec = rtsp->time_current;
    begin = (int)((int)mid_time( ) + rtsp->servtime_diff - rtsp->time_length);
/*
    {
        char buf[32];
        rtsp_clt_time_local(sec, buf);
        LOG_STRM_PRINTF("@@@@@@@@@@: sec = %s\n", buf);
        rtsp_clt_time_local(begin, buf);
        LOG_STRM_PRINTF("@@@@@@@@@@: begin = %s\n", buf);
    }
*/
    if (sec <= begin) {
        LOG_STRM_PRINTF("#%d timeshift timeout!\n", rtsp->index);
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_BEGIN\n", rtsp->index);
        rtsp_msg_back(rtsp, STRM_MSG_STREAM_BEGIN, 0);
        ind_timer_delete(rtsp->tlink, rtsp_clt_shiftcheck, rtsp);
    }
}

void rtsp_clt_rangeout(void* arg)
{
    struct RTSP *rtsp = (struct RTSP *)arg;
    rtsp_msg_back(rtsp, STRM_MSG_STREAM_END, 0);
}

int rtsp_clt_datasocket(struct RTSP* rtsp)
{
    struct ind_sin* serv_sin = &rtsp->serv_sin;
    int i, opt, sock = -1;
    u_short port;

    sock = socket(serv_sin->family, SOCK_DGRAM, 0);
    if (sock < 0)
        LOG_STRM_ERROUT("#%d socket failed\n", rtsp->index);

    opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt)) < 0)
        LOG_STRM_ERROUT("#%d setsockopt SO_REUSEADDR", rtsp->index);

#if SUPPORTE_HD == 1
    opt = 1024 * 1024;
#else
    opt = 1024 * 200;
#endif
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&opt, sizeof(opt)) < 0)
        LOG_STRM_PRINTF("#%d Can't change system network size (wanted size = %d)\n", rtsp->index, opt);

    LOG_STRM_DEBUG("#%d port_udp = %d\n", rtsp->index, rtsp->port_udp);

    {
        struct ind_sin sin;

        memset(&sin, 0, sizeof(sin));
        sin.family = serv_sin->family;

        if (rtsp->port_udp) {
            port = (unsigned short)rtsp->port_udp;
            sin.port = port;
            ind_net_bind(sock, &sin);
        } else {
            port = 0;
            for (i = 0; i < 5 && port == 0; i ++) {
                port = (u_short)rtsp_alloc_port( );
                sin.port = port;
                if (ind_net_bind(sock, &sin))
                    port = 0;
            }
            if (port == 0)
                LOG_STRM_ERROUT("#%d bind port %hd failed %d! %s\n", rtsp->index, port, errno, strerror(errno));
        }
        rtsp->data_port = port;
    }

    LOG_STRM_PRINTF("#%d: udp socket %d port = %hu\n", rtsp->index, sock, port);
    if (rtsp->data_sock != -1)
        close(rtsp->data_sock);

    rtsp->data_sock = sock;
    rtsp_clt_fdset(rtsp);

    return 0;
Err:
    if (sock >= 0)
        close(sock);
    return -1;
}

int rtsp_clt_rtcpsocket(struct RTSP* rtsp)
{
    struct ind_sin* serv_sin = &rtsp->serv_sin;
    int opt, sock = -1;
    u_short port;

    sock = socket(serv_sin->family, SOCK_DGRAM, 0);
    if (sock < 0)
        ERR_OUT("#%d socket failed\n", rtsp->index);

    opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt)) < 0)
        ERR_OUT("#%d setsockopt SO_REUSEADDR", rtsp->index);

#if SUPPORTE_HD == 1
    opt = 1024 * 1024;
#else
    opt = 1024 * 200;
#endif
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&opt, sizeof(opt)) < 0)
        PRINTF("#%d Can't change system network size (wanted size = %d)\n", rtsp->index, opt);

    DBG_PRN("#%d data_port = %d\n", rtsp->index, rtsp->data_port);

    {
        struct ind_sin sin;

        memset(&sin, 0, sizeof(sin));
        sin.family = serv_sin->family;

        if (rtsp->data_port) {
            port = (unsigned short)rtsp->data_port + 1;
            sin.port = port;
            if (ind_net_bind(sock, &sin))
                ERR_OUT("#%d rtcp port %hd failed %d! %s\n", rtsp->index, port, errno, strerror(errno));
        } else {
            ERR_OUT("#%d data port %hd failed %d! %s\n", rtsp->index, rtsp->data_port, errno, strerror(errno));
        }
    }

    PRINTF("#%d: rtcp socket %d port = %hu\n", rtsp->index, sock, port);

    if (rtsp->rtcp_sock != -1)
        close(rtsp->rtcp_sock);

    rtsp->rtcp_sock = sock;
    rtsp_clt_fdset(rtsp);

    return 0;
Err:
    if (sock >= 0)
        close(sock);
    return -1;
}

int rtsp_clt_send(struct RTSP* rtsp, int len)
{
    char* buf = rtsp->send_buf;

    LOG_STRM_PRINTF("#%d len = %d\n", rtsp->index, len);
    rtsp->rtsp_code = 0;

    if (rtsp->sock == -1)
        LOG_STRM_ERROUT("#%d sock = -1\n", rtsp->index);

    if (send(rtsp->sock, buf, len, MSG_NOSIGNAL) != len)
        LOG_STRM_ERROUT("#%d send len = %d, errno = %d! %s\n", rtsp->index, len, errno, strerror(errno));

    if (rtsp->call_rtspinfo)
        rtsp->call_rtspinfo(0, buf, len);

    return 0;
Err:
    return -1;
}

void rtsp_clt_advertise_select(struct RTSP* rtsp, int inserted, int sec)
{
    int i, insert;
    int adv_num;
    struct Advertise *adv_array;

    LOG_STRM_PRINTF("#%d inserted = %d, sec = %d\n", rtsp->index, inserted, sec);

    adv_num = rtsp->adv_num;
    adv_array = rtsp->adv_array;

    if (adv_num <= 0)
        return;

    rtsp->adv_inserted = inserted;
    rtsp->adv_insert = 0;

    for (i = 0; i < adv_num; i ++) {
        insert = adv_array[i].insert;
        if (insert > rtsp->adv_inserted
            && insert >= sec
            && (insert < rtsp->adv_insert || 0 == rtsp->adv_insert))
            rtsp->adv_insert = insert;
    }
}

struct Advertise* rtsp_clt_advertise_elem(struct RTSP* rtsp, int insert)
{
    int i, adv_num;
    struct Advertise *adv_array;

    adv_num = rtsp->adv_num;
    adv_array = rtsp->adv_array;

    LOG_STRM_DEBUG("#%d adv_num = %d, insert = %d\n", rtsp->index, adv_num, insert);

    for (i = 0; i < adv_num; i ++) {
        if (insert == adv_array[i].insert)
            return &adv_array[i];
    }

    return NULL;
}

void rtsp_clt_close_play(struct RTSP* rtsp)
{
    strm_play_close(rtsp->strm_play, rtsp->index, rtsp->clear_flg);
    rtsp->open_play = OPEN_PLAY_CLOSE;
    rtsp->clear_flg = 0;

    {
        RTSPStat_t stat = &rtsp->stat;
        if (stat->stat_vodstop) {
            int ms = (int)(rtsp->clk - stat->stat_vodstop) * 10;

            stream_port_post_vodstop(ms);

            stat->stat_vodstop = 0;
            LOG_STRM_DEBUG("#%d vodstop = %d\n", rtsp->index, ms);
        }
    }
}

void rtsp_clt_close_vod(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);
    rtsp_op_none(rtsp);

    if (rtsp->sock != -1) {
        close(rtsp->sock);
        rtsp->sock = -1;
    }
    if (rtsp->data_sock != -1) {
        close(rtsp->data_sock);
        rtsp->data_sock = -1;
    }
    if (rtsp->rtcp_sock != -1) {
        close(rtsp->rtcp_sock);
        rtsp->rtcp_sock = -1;
    }

    rtsp_clt_ret_close(rtsp);
    rtsp_clt_arq_close(rtsp);

    if (3 == rtsp->rtcp_flag) {
        rtsp_clt_rtcp_close(rtsp);
        ind_timer_delete(rtsp->tlink, rtsp_clt_rtcpapp, rtsp);
    }

    ind_timer_delete(rtsp->tlink, rtsp_clt_heartbit, rtsp);
    ind_timer_delete(rtsp->tlink, rtsp_clt_natheartbit, rtsp);

    if (rtsp->rtsp_state != RTSP_STATE_TEARDOWN)
        rtsp->rtsp_state = RTSP_STATE_TEARDOWN;
    rtsp_clt_fdset(rtsp);
}

static void rtsp_clt_post_datasock(struct RTSP* rtsp)
{
     struct sockaddr_in peername, sockname;

     peername.sin_family = AF_INET;
     peername.sin_port = htons(rtsp->peername.port);
     peername.sin_addr = rtsp->peername.in_addr;

     sockname.sin_family = AF_INET;
     sockname.sin_port = htons(rtsp->sockname.port);
     sockname.sin_addr = rtsp->sockname.in_addr;

     stream_port_post_datasock(0, rtsp->state, &peername, &sockname, rtsp->url);
}

static void rtsp_clt_post_timer(struct RTSP *rtsp)
{
    //LOG_STRM_PRINTF("#%d postvalid = %d, mult_flag = %d, state = %d\n", rtsp->index, rtsp->postvalid, rtsp->mult_flag, rtsp->state);

    if (rtsp->state == STRM_STATE_PLAY || rtsp->state == STRM_STATE_IPTV)
        rtsp->postvalid = 1;
    else
        rtsp->postvalid = 2;

    if (rtsp->mult_flag) {
        if (rtsp->mult_sin.family == AF_INET) {
            struct sockaddr_in mult_sin;
            mult_sin.sin_family = rtsp->mult_sin.family;
            mult_sin.sin_addr = rtsp->mult_sin.in_addr;
            mult_sin.sin_port = htons(rtsp->mult_sin.port);
            LOG_STRM_DEBUG("#%d post multicast sqm_flag = %d\n", rtsp->index, g_rtsp_sqm_flag);
            if (g_rtsp_sqm_flag == 1)
                stream_port_post_datasock(0, 0, NULL, &mult_sin, rtsp->igmp_info);
        }
    } else {
    /*
        if (rtsp->data_sock != -1) {
            LOG_STRM_DEBUG("#%d post open udp\n", rtsp->index);
            if (g_rtsp_sqm_flag == 1)
                stream_port_post_datasock(0, 0, &rtsp->peername, &rtsp->sockname, rtsp->url);
        } else {
            LOG_STRM_DEBUG("#%d post open tcp\n", rtsp->index);
            if (g_rtsp_sqm_flag == 1)
                stream_port_post_datasock(0, 1, &rtsp->peername, &rtsp->sockname, rtsp->url);
        }
     */
        LOG_STRM_DEBUG("#%d post unicast postvalid = %d, sqm_flag = %d\n", rtsp->index, rtsp->postvalid, g_rtsp_sqm_flag);
        if (g_rtsp_sqm_flag == 1)
            rtsp_clt_post_datasock(rtsp);
    }
}

void rtsp_clt_post_valid(struct RTSP* rtsp, int valid)
{
    LOG_STRM_DEBUG("#%d valid = %d, open_play = %d, music_flg = %d\n", rtsp->index, valid, rtsp->open_play, rtsp->music_flg);

    if ((rtsp->open_play == 0 || rtsp->index == STREAM_INDEX_PIP || rtsp->music_flg != 0) && valid == 1)
        return;

    LOG_STRM_DEBUG("#%d postvalid = %d, valid = %d, state = %d\n", rtsp->index, rtsp->postvalid, valid, rtsp->state);

    if (valid) {
        if (rtsp->postvalid > 0) {
            if (rtsp->state == STRM_STATE_PLAY || rtsp->state == STRM_STATE_IPTV)
                rtsp->postvalid = 1;
            else
                rtsp->postvalid = 2;
            if (0 == rtsp->mult_flag) {
                LOG_STRM_DEBUG("#%d post postvalid = %d\n", rtsp->index, rtsp->postvalid);
                if (g_rtsp_sqm_flag == 1)
                    rtsp_clt_post_datasock(rtsp);
            }
        } else {
            rtsp->postvalid = 0;
            rtsp_clt_post_timer(rtsp);
        }
    } else {
        if (rtsp->postvalid > 0) {
            LOG_STRM_DEBUG("#%d post close. op = %d, mult = %d, clear = %d\n", rtsp->index, rtsp->op, rtsp->mult_flag, rtsp->clear_flg);
            if (g_rtsp_sqm_flag == 1) {
                if (RTSP_OP_TEARDOWN == rtsp->op && rtsp->mult_flag && 0 == rtsp->clear_flg)
                    stream_post_datasock(0, 1);
                else
                    stream_port_post_datasock(0, -1, NULL, NULL, NULL);
            }
        }
        rtsp->postvalid = -1;
    }
}

#ifdef INCLUDE_PVR
int rtsp_clt_tstv_calc(struct RTSP* rtsp)
{
    uint32_t now;
    struct PVRInfo *info = &rtsp->pvr_info;

    now = mid_time( );

    if (rtsp->open_shift == 0) {
        rtsp_clt_time_set_current(rtsp, now);
        LOG_STRM_DEBUG("#%d time_current = %d\n", rtsp->index, rtsp->time_current);
        goto Err;
    }

    IND_MEMSET(info, 0, sizeof(struct PVRInfo));
    if (rtsp->pvr) {
        if (ind_pvr_get_info(rtsp->shift_id, info)) {
            rtsp_clt_time_set_total(rtsp, 0);
            LOG_STRM_ERROUT("#%d ind_pvr_get_info\n", rtsp->index);
        }

        if (info->time_len >= rtsp->timeshift_len) {
            rtsp->rec_time = now - rtsp->timeshift_len;
            rtsp_clt_time_set_total(rtsp, rtsp->timeshift_len);
        } else {
            if (rtsp->rec_time == 0)
                rtsp->rec_time = now;
            rtsp_clt_time_set_total(rtsp, info->time_len);
        }
    }

    switch(rtsp->pvr_state) {
    case STRM_STATE_IPTV:
        rtsp_clt_time_set_current(rtsp, mid_time( ));
        LOG_STRM_DEBUG("time_current = %d\n", rtsp->time_current);
        LOG_STRM_DEBUG("#%d: time_len = %d, timeshift_len = %d, time_length = %d\n", rtsp->index, info->time_len, rtsp->timeshift_len, rtsp->time_length);
        break;
    case STRM_STATE_PAUSE:
    case STRM_STATE_PLAY:
    case STRM_STATE_FAST:
        if (STRM_STATE_FAST != rtsp->pvr_state) {
            if (rtsp->pvr == NULL) {
                int announce = ind_pvr_open(rtsp->shift_id, &rtsp->pvr);
                if (announce == 0) {
                    rtsp->time_start = mid_time( );
                    LOG_STRM_PRINTF("#%d time_start = %s, pvr_smooth = %d\n", rtsp->index, rtsp_clt_time_fmt(rtsp, 0, rtsp->time_start), rtsp->pvr_smooth);

                    if (1 == rtsp->pvr_smooth) {
                        if (ind_pvr_play(rtsp->pvr, -2, 1)) {
                            LOG_STRM_ERROR("#%d ind_pvr_play smooth\n", rtsp->index);
                            rtsp->pvr_smooth = 0;
                        } else {
                            rtsp->pvr_smooth_clk = strm_play_time(rtsp->strm_play);
                        }
                    }
                    if (0 == rtsp->pvr_smooth)
                        ind_pvr_play(rtsp->pvr, 0, 1);
                }
                break;
            }
            if (STRM_STATE_PAUSE == rtsp->pvr_state)
                break;
            if (1 == rtsp->pvr_smooth_pause) {
                LOG_STRM_PRINTF("#%d time_len = %d\n", rtsp->index, info->time_len);
                if (info->time_len > 0) {
                    strm_play_resume(rtsp->strm_play, rtsp->index, 0);
                    rtsp->pvr_smooth_pause = 0;
                }
            }
        }
        if (rtsp->pvr == NULL) {
            LOG_STRM_DEBUG("#%d pvr is NULL\n", rtsp->index);
            goto Err;
        }
        {
            uint32_t begin, current;
            int sec = 0;

            begin = rtsp->rec_time;
            if (STRM_STATE_PLAY == rtsp->pvr_state && 1 == rtsp->pvr_smooth) {
                int clk = strm_play_time(rtsp->strm_play);
                if (sec < rtsp->pvr_smooth_clk)
                    sec = 0;
                else
                    sec = (clk - rtsp->pvr_smooth_clk) / 100;
            } else {
                sec = strm_play_time(rtsp->strm_play) / 100;
            }
            current = (uint32_t)((int)rtsp->time_start + sec);

            LOG_STRM_DEBUG("#%d: sec = %d, begin = %d, end = %d, rec = %d\n", rtsp->index, sec, begin, now, rtsp->rec_time);
            if (current < begin)
                current = begin;
            else if (current > now)
                current = now;

            rtsp_clt_time_set_current(rtsp, current);
            LOG_STRM_DEBUG("time_current = %d\n", rtsp->time_current);
        }
        break;
    default:
        LOG_STRM_DEBUG("#%d: pvr_state = %d\n", rtsp->index, rtsp->pvr_state);
        break;
    }

    return 0;
Err:
    return -1;
}
#endif

void rtsp_clt_time_1000ms(struct RTSP* rtsp)
{
#ifdef INCLUDE_PVR
    if (rtsp->apptype == APP_TYPE_TSTV)
        rtsp_clt_tstv_calc(rtsp);
#endif

    if (rtsp->ppv_end) {
        uint32_t now;

        now = mid_time( );
        if (rtsp->ppv_end < now) {
            rtsp_msg_back(rtsp, STRM_MSG_PPV_END, 0);
            return;
        }
        if (rtsp->open_play == OPEN_PLAY_PPVWAIT) {
            if (rtsp->ppv_begin > now)
                return;
            rtsp->open_play = OPEN_PLAY_RUNNING;
        }
    }
    if (rtsp->apptype == APP_TYPE_TSTV)
        return;

    if (rtsp->op == RTSP_OP_PLAY || rtsp->op == RTSP_OP_PLAY_INIT || rtsp->op == RTSP_OP_PAUSE)
        return;

    switch(rtsp->state) {
    case STRM_STATE_PLAY:
        int_clt_time_play(rtsp);
        break;
    case STRM_STATE_IPTV:
#ifdef DEBUG_BUILD
        if (rtsp->iframe_flag) {
            int sec = strm_play_time(rtsp->strm_play);
            LOG_STRM_PRINTF("@@@@@@@@: sec = %d\n", sec);
        }
#endif
        rtsp_clt_time_set_current(rtsp, rtsp_clt_time_server(rtsp));
        LOG_STRM_DEBUG("time_current = %d\n", rtsp->time_current);
        break;
    default:
        break;
    }
}

static void int_clt_time_play(struct RTSP* rtsp)
{
    uint32_t time_begin;
    int sec, length, start;

    if (rtsp->state != STRM_STATE_PLAY)
        return;

    sec = strm_play_time(rtsp->strm_play) / 100;
    LOG_STRM_DEBUG("#%d: state = %d, sec = %d\n", rtsp->index, rtsp->state, sec);

    length = (int)rtsp->time_length;
    if (rtsp->apptype == APP_TYPE_VOD) {
        LOG_STRM_DEBUG("#%d: time_start = %d\n", rtsp->index, rtsp->time_start);
        start = (int)rtsp->time_start;
        time_begin = 0;
    } else {
        time_begin = rtsp_clt_time_server(rtsp) - (uint32_t)length;
        LOG_STRM_DEBUG("#%d: time_start = %s,  time_begin = %s\n", rtsp->index, rtsp_clt_time_fmt(rtsp, 0, rtsp->time_start), rtsp_clt_time_fmt(rtsp, 1, time_begin));
        start = (int)(rtsp->time_start - time_begin);
    }

    sec += start;
    if (sec < 0)
        sec = 0;
    else if (sec > length)
        sec = length;

    LOG_STRM_DEBUG("#%d: time_length = %d, scale = %d, start = %d, sec = %d\n", rtsp->index, rtsp->time_length, rtsp->scale, start, sec);

    rtsp_clt_time_set_current(rtsp, time_begin + (uint32_t)sec);
    if (APP_TYPE_VOD == rtsp->apptype && rtsp->index < STREAM_INDEX_PIP) {
        int buffer = strm_play_length(rtsp->strm_play);
        int_steam_setCurBufferSize(buffer);
        int_steam_setRemainPlaytime(length - sec);
    }

    if (rtsp->op == RTSP_OP_NONE && rtsp->state == STRM_STATE_PLAY
        && rtsp->adv_num > 0
        && rtsp->adv_insert > 0 && rtsp->time_current >= (uint32_t)rtsp->adv_insert)
        rtsp_cmd_queuecmd(rtsp, STREAM_CMD_ADVERTISE);
}

static void int_clt_time_fast(struct RTSP* rtsp, int align)
{
    uint32_t time_begin;
    int sec, length, start;

    if (rtsp->state != STRM_STATE_FAST)
        return;

    sec = strm_play_time(rtsp->strm_play) / 100;
    if (align) {
        int a = abs(sec);

        a = a - a % abs(rtsp->scale);
        if (sec < 0)
            sec = -a;
        else
            sec = a;
    }

    length = (int)rtsp->time_length;
    if (rtsp->apptype == APP_TYPE_VOD) {
        start = (int)rtsp->time_start;
        time_begin = 0;
    } else {
        time_begin = rtsp_clt_time_server(rtsp) - (uint32_t)length;
        start = (int)(rtsp->time_start - time_begin);
    }

    sec += start;
    if (sec < 0)
        sec = 0;
    else if (sec > length)
        sec = length;
    if (sec == rtsp->time_fast && (APP_TYPE_VOD == rtsp->apptype || 0 != sec)) {
        LOG_STRM_DEBUG("#%d: apptype = %d, sec = %d\n", rtsp->index, rtsp->apptype, sec);
        return;
    }
    rtsp->time_fast = sec;

    LOG_STRM_DEBUG("#%d: time_length = %d, scale = %d, start = %d, sec = %d\n", rtsp->index, rtsp->time_length, rtsp->scale, start, sec);

    rtsp_clt_time_set_current(rtsp, time_begin + (uint32_t)sec);
    if (APP_TYPE_VOD == rtsp->apptype && rtsp->index < STREAM_INDEX_PIP) {
        int buffer = strm_play_length(rtsp->strm_play);
        int_steam_setCurBufferSize(buffer);
        int_steam_setRemainPlaytime(length - sec);
    }
}

void rtsp_clt_time_sync(struct RTSP* rtsp)
{
    if (STRM_STATE_FAST == rtsp->state)
        int_clt_time_fast(rtsp, 0);
    else
        int_clt_time_play(rtsp);
}

uint32_t rtsp_clt_time_server(struct RTSP* rtsp)
{
    return (uint32_t)((int)mid_time( ) + rtsp->servtime_diff);
}

void rtsp_clt_time_set_total(struct RTSP* rtsp, uint32_t length)
{
    rtsp->time_length = length;
    if (rtsp->apptype == APP_TYPE_TSTV) {
        LOG_STRM_DEBUG("#%d record time = %d\n", rtsp->index, rtsp->rec_time);
        length = rtsp->rec_time;
    } else {
        LOG_STRM_DEBUG("#%d totaltime = %d\n", rtsp->index, length);
    }
    stream_back_totaltime(rtsp->index, length);
}

void rtsp_clt_time_set_current(struct RTSP* rtsp, uint32_t current)
{
    rtsp->time_current = current;
    if (rtsp->apptype == APP_TYPE_IPTV) {
        current -= rtsp->servtime_diff;
        LOG_STRM_DEBUG("#%d currenttime = %d, now = %d\n", rtsp->index, current, mid_time( ));
    } else {
        LOG_STRM_DEBUG("#%d currenttime = %d\n", rtsp->index, current);
    }
    stream_back_currenttime(rtsp->index, current);
}

uint32_t rtsp_clt_time_make(char *string)
{
    unsigned int t;
    struct ind_time tp;

    if (sscanf(string, "%04d%02d%02dT%02d%02d%02d",
            &tp.year, &tp.mon, &tp.day, &tp.hour, &tp.min, &tp.sec) != 6)
        LOG_STRM_ERROUT("sscanf failed!\n");

    t = ind_time_make(&tp);

    return t;
Err:
    return 0;
}

//20081115T115630.00Z-20081115T125630.00Z
//20081115T025928Z-20081115T035928Z
int rtsp_clt_time_range(struct RTSP* rtsp, char *buf)
{
    int diff;
    uint32_t begin, end, now;

    begin = rtsp_clt_time_make(buf);
    if (begin == 0)
        LOG_STRM_ERROUT("rtsp_clt_time_make begin\n");
    buf = strchr(buf, '-');
    if (buf == NULL)
        LOG_STRM_ERROUT("#%d '-' not found\n", rtsp->index);
    buf ++;

    end = rtsp_clt_time_make(buf);
    if (end == 0)
        LOG_STRM_ERROUT("rtsp_clt_time_make end\n");

    rtsp->timeshift_real = end - begin;

    if (rtsp->timeshift_len > 0 && rtsp->timeshift_len < rtsp->timeshift_real) {
        LOG_STRM_PRINTF("#%d timeshift_len = %d, timeshift_real = %d\n", rtsp->index, rtsp->timeshift_len, rtsp->timeshift_real);
        rtsp_clt_time_set_total(rtsp, rtsp->timeshift_len);
    } else {
        rtsp_clt_time_set_total(rtsp, rtsp->timeshift_real);
    }

    now = mid_time( );
    diff = (int)end - (int)now;
    LOG_STRM_PRINTF("#%d time diff = %d, end = %d, now = %d\n", rtsp->index, diff, end, now);
    LOG_STRM_DEBUG("#%d apptype = %d, servtime_diff = %d, diff = %d, rtsp->retry_op_off = %d\n", rtsp->index, rtsp->apptype, rtsp->servtime_diff, diff, rtsp->retry_op_off);
    if (rtsp->apptype != APP_TYPE_VOD && rtsp->servtime_diff == 0 && rtsp->retry_op_off)
        rtsp->retry_op_off += diff;

    rtsp->servtime_diff = diff;
    if (rtsp->open_play == OPEN_PLAY_RUNNING && rtsp->index < STREAM_INDEX_PIP)
        g_difftime = diff;
    if (rtsp->apptype != APP_TYPE_VOD && (rtsp->state == STRM_STATE_OPEN || rtsp->state == STRM_STATE_IPTV)) {
        rtsp_clt_time_set_current(rtsp, rtsp_clt_time_server(rtsp));
        LOG_STRM_DEBUG("time_current = %d\n", rtsp->time_current);
    }

    rtsp->servtime_sync = 1;

    return 0;
Err:
    return -1;
}

int rtsp_clt_time_local(uint32_t sec, char *buf)
{
    struct ind_time tp;

    ind_time_local(sec, &tp);

    sprintf(buf, "%04d%02d%02dT%02d%02d%02d",
            tp.year, tp.mon, tp.day,
            tp.hour, tp.min, tp.sec);

    return 15;
}

char *rtsp_clt_time_fmt(struct RTSP* rtsp, int idx, uint32_t sec)
{
    char *buf;
    struct ind_time tp;

    if (idx < 0 || idx >= LINE_NUM_2)
        return NULL;
    buf = rtsp->fmt_line[idx];
    ind_time_local(sec, &tp);
    sprintf(buf, "%04d%02d%02dT%02d%02d%02d",
            tp.year, tp.mon, tp.day,
            tp.hour, tp.min, tp.sec);

    return buf;
}

void rtsp_clt_time_print(uint32_t sec, char* buf)
{
    struct ind_time tp;

    ind_time_local(sec, &tp);
    sprintf(buf, "%04d%02d%02dT%02d%02d%02d",
            tp.year, tp.mon, tp.day,
            tp.hour, tp.min, tp.sec);
}

char *rtsp_clt_addr_fmt(struct RTSP* rtsp, struct ind_sin* sin)
{
    char *buf = rtsp->fmt_line[0];

    ind_net_ntop(sin, buf, LINE_LEN_128);

    return buf;
}

void rtsp_clt_set_rtspsock(int sock)
{
	g_sock = sock;
}

int mid_stream_get_rtspsock(void)
{
	return g_sock;
}

unsigned int mid_stream_get_difftime(int idx)
{
    return g_difftime;
}
