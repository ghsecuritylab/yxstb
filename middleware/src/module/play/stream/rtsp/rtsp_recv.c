
/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "rtsp_app.h"

enum {
    TCP_FLAG_UNKNOWN = 0,
    TCP_FLAG_DATA_SYNC,
    TCP_FLAG_DATA,
    TCP_FLAG_CTRL_SYNC,
    TCP_FLAG_CTRL,
};

static void deal_recv_announce(struct RTSP* rtsp)
{
    char *buf, *p;

    buf = rtsp->ctrl_buf;
    if (rtsp->standard == RTSP_STANDARD_ELECARD) {
        p = ind_stristr(buf, "Notice:");
        if (p)
            p += 7;
    } else {
        p = ind_stristr(buf, "x-notice:");
        if (p)
            p += 9;
    }
    if (p) {
        int code = 0;
        while(*p == ' ')
            p ++;
        if (sscanf(p, "%d", &code) == 1) {
            LOG_STRM_PRINTF("#%d code = %d\n", rtsp->index, code);
            switch(code) {
            case 2101://End of Stream
                if (rtsp->open_play) {
                    rtsp->open_play = OPEN_PLAY_END;

                    if (STRM_STATE_FAST == rtsp->state || APP_TYPE_IPTV == rtsp->apptype) {
                        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", rtsp->index);
                        rtsp_msg_back(rtsp, STRM_MSG_STREAM_END, 0);

                        strm_play_set_idle(rtsp->strm_play, rtsp->index, 1);
                        if (rtsp->ret_flag == 3)
                            rtsp->ret_flag = 4;
                    } else {
                        LOG_STRM_PRINTF("#%d End of Stream ret_flag = %d\n", rtsp->index, rtsp->ret_flag);
                        if (rtsp->ret_flag < 3)
                            strm_play_end(rtsp->strm_play, rtsp->index);
                        else
                            rtsp->ret_flag = 3;
                    }
                }
                break;
            case 2102://Beginning of Stream
                if (rtsp->open_play) {
                    if (rtsp->state == STRM_STATE_FAST) {
                        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_BEGIN\n", rtsp->index);
                        rtsp_msg_back(rtsp, STRM_MSG_STREAM_BEGIN, 0);
                    } else {
                        LOG_STRM_PRINTF("#%d Beginning of Stream\n", rtsp->index);
                        strm_play_end(rtsp->strm_play, rtsp->index);
                    }
                }
                break;
            default:
                LOG_STRM_WARN("#%d unknown ANNOUNCE!\n", rtsp->index);
                break;
            }
        }
    } else {
        LOG_STRM_WARN("#%d Notice not fond!\n", rtsp->index);
    }
}

static int deal_recv_response(struct RTSP* rtsp)
{
    char *buf, *p;
    int code;
    RTSP_OP op;

    op = rtsp->op;

    buf = rtsp->ctrl_buf;
    p = ind_stristr(buf, "CSeq: ");
    if (p == NULL)
        LOG_STRM_WARNOUT("#%d CSeq not found\n", rtsp->index);
    if (sscanf(p + 6, "%d", &code) != 1)
        LOG_STRM_WARNOUT("#%d CSeq error\n", rtsp->index);

    if (op == RTSP_OP_NONE || code != rtsp->op_cseq) {
        p = ind_stristr(rtsp->ctrl_buf, "x-Fast_Cache:");
        if (p) {
            p += 13;
            while (*p == ' ')
                p ++;
            if (ind_memicmp(p, "off", 3) == 0) {
                LOG_STRM_PRINTF("#%d CACHE_STATE_OFF ret_flag == %d\n", rtsp->index, rtsp->ret_flag);
                if (rtsp->ret_flag >= 3) {
                    LOG_STRM_DEBUG("#%d ret_port_cache off\n", rtsp->index);
                    ret_port_cache(rtsp->ret_handle, 0);
                }
                rtsp->cache = CACHE_STATE_OFF;
            } else {
                if (ind_memicmp(p, "on", 2))
                    LOG_STRM_WARN("#%d invalid!\n", rtsp->index);
                LOG_STRM_DEBUG("#%d CACHE_STATE_ON ret_flag == %d\n", rtsp->index, rtsp->ret_flag);
                if (rtsp->ret_flag >= 3) {
                    LOG_STRM_DEBUG("#%d ret_port_cache on\n", rtsp->index);
                    ret_port_cache(rtsp->ret_handle, 1);
                }
                rtsp->cache = CACHE_STATE_ON;
            }
        }
    }

    if (op == RTSP_OP_NONE)
        LOG_STRM_WARNOUT("#%d RTSP_OP_NONE\n", rtsp->index);

    if (code != rtsp->op_cseq) {
        if (code == rtsp->ig_cseq)
            goto Warn;
        LOG_STRM_WARNOUT("#%d RECV CSeq = %d / %d\n", rtsp->index, code, rtsp->op_cseq);
    }

    p = strstr(buf, "RTSP/1.0");
    if (p == NULL)
        LOG_STRM_ERROUT("#%d 'RTSP/1.0' not found\n", rtsp->index);
    if (sscanf(p + 9, "%d", &code) != 1)
        LOG_STRM_ERROUT("#%d 'code sscanf failed\n", rtsp->index);
    rtsp->rtsp_code = code;
    switch(code) {
    case 200:
        rtsp->err_no = 0;
        break;
    case 302:
        if (ind_stristr(buf, "Location: rtsp") == NULL) {
            rtsp->err_no = RTSP_ERROR_REDIRECT;
            LOG_STRM_ERROUT("#%d op = %d, 302\n", rtsp->index, op);
        }
        rtsp_op_location(rtsp);
        return 0;
    default:
        rtsp->err_no = code;
        LOG_STRM_ERROUT("#%d op = %d, code = %d\n", rtsp->index, op, code);
    }
    rtsp_op_succeed(rtsp);
Warn:
    return 0;
Err:
    if (rtsp->err_no == 0)
        rtsp->err_no = RTSP_ERROR_ANALYZE;
    rtsp_op_failed(rtsp);
    return -1;
}

#define ANNOUNCE_STRING         "ANNOUNCE"
#define REDIRECT_STRING         "REDIRECT"
#define RTSP_1_0_STRING         "RTSP/1.0"

#define MSG_MARK_LENGTH         8

void deal_recv_ctrl(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\r\n%s\n", rtsp->index, rtsp->ctrl_buf);

    if (strncmp(rtsp->ctrl_buf, ANNOUNCE_STRING, MSG_MARK_LENGTH) == 0) {
        if (rtsp->op != RTSP_OP_PLAY_INIT && rtsp->op != RTSP_OP_PLAY)
            deal_recv_announce(rtsp);
    } else {
        deal_recv_response(rtsp);
    }
}

static int int_recv_sync_check(struct RTSP* rtsp, char* buf)
{
    if (buf[0] == 0x24) {
        if (buf[1] == 0 || buf[1] == 1) {
            uint8_t *ubuf;
            uint32_t ulen;

            ubuf = (uint8_t*)buf;
            ulen = ((uint32_t)ubuf[2] << 8) + (uint32_t)ubuf[3];

            if (rtsp->rtp_flag) {
                if (ulen > 0 && ulen < 1500 && ulen % 4 == 0)
                    return (int)ulen;
            } else {
                if (ulen > 0 && ulen < TCP_BUFFER_LENGTH - 12 && 0 == ulen % 188)//12 = 8字节同步 + 4字节tcp头
                    return (int)ulen;
            }
            LOG_STRM_WARN("len = %u\n", ulen);
        }
    } else {
        if (buf[0] == 'A' && 0 == memcmp(buf, ANNOUNCE_STRING, MSG_MARK_LENGTH))
            return 0;
        if (buf[0] == 'R') {
            if (buf[1] == 'E' && 0 == memcmp(buf, REDIRECT_STRING, MSG_MARK_LENGTH))
                return 0;
            if (buf[1] == 'T' && 0 == memcmp(buf, RTSP_1_0_STRING, MSG_MARK_LENGTH))
                return 0;
        }
    }

    return -1;
}

static void int_recv_sync_buf(struct RTSP* rtsp, char* buf, int len)
{
    int skip, length;

    length = len;
    while (len >= MSG_MARK_LENGTH) {
        rtsp->tcp_match = int_recv_sync_check(rtsp, buf);
        if (rtsp->tcp_match >= 0)
            break;
        buf++;
        len--;
    }
    skip = length - len;
    if (skip > 0) {
        LOG_STRM_WARN("Skip %d\n", skip);
        ts_buf_read_pop(rtsp->tcp_buf, skip);
    }
}

static int int_recv_sync(struct RTSP* rtsp)
{
    int len, length;
    char *buf, *sb_buf;

    len = 0;
    ts_buf_read_get(rtsp->tcp_buf, &buf, &len);
    int_recv_sync_buf(rtsp, buf, len);
    if (rtsp->tcp_match >= 0)
        goto Ok;

    length = ts_buf_length(rtsp->tcp_buf);
    if (length < MSG_MARK_LENGTH)
        return 0;

    len = 0;
    ts_buf_read_get(rtsp->tcp_buf, &buf, &len);

    sb_buf = rtsp->rtp_sb->buf;
    IND_MEMCPY(sb_buf, buf, len);

    len += ts_buf_peek(rtsp->tcp_buf, 0, sb_buf + len, MSG_MARK_LENGTH - 1);
    int_recv_sync_buf(rtsp, sb_buf, len);
    if (rtsp->tcp_match >= 0)
        goto Ok;

    len = ts_buf_length(rtsp->tcp_buf);
    if (len < MSG_MARK_LENGTH)
        return 0;

    len = 0;
    ts_buf_read_get(rtsp->tcp_buf, &buf, &len);
    int_recv_sync_buf(rtsp, buf, len);
    if (rtsp->tcp_match >= 0)
        goto Ok;

    return 0;
Ok:
    if (rtsp->tcp_match > 0) {
        rtsp->tcp_flag = TCP_FLAG_DATA_SYNC;
        ts_buf_read(rtsp->tcp_buf, NULL, 4);
        rtsp->tcp_len = rtsp->tcp_match;
    } else {
        rtsp->tcp_flag = TCP_FLAG_CTRL_SYNC;
    }
    return 1;
}

static int int_recv_data_syunc(struct RTSP* rtsp)
{
    int len, length;
    char *buf, *sb_buf;

    length = ts_buf_length(rtsp->tcp_buf);
    if (length < rtsp->tcp_len + MSG_MARK_LENGTH) {
        if (RTSP_OP_PAUSE == rtsp->op || RTSP_OP_TEARDOWN == rtsp->op || RTSP_OP_TIMESHIFT == rtsp->op) {
            if (ts_buf_memstr(rtsp->tcp_buf, "RTSP/1.0") >= 0) {
                ts_buf_read_pop(rtsp->tcp_buf, 2);
                rtsp->tcp_flag = TCP_FLAG_UNKNOWN;
                return 1;
            }
        }
        return 0;
    }

    len = 0;
    ts_buf_read_get(rtsp->tcp_buf, &buf, &len);
    if (len >= rtsp->tcp_len + MSG_MARK_LENGTH) {
        rtsp->tcp_match = int_recv_sync_check(rtsp, buf + rtsp->tcp_len);
    } else {
        sb_buf = rtsp->rtp_sb->buf;
        if (len > rtsp->tcp_len) {
            len -= rtsp->tcp_len;
            IND_MEMCPY(sb_buf, buf + rtsp->tcp_len, len);

            ts_buf_peek(rtsp->tcp_buf, 0, sb_buf + len, MSG_MARK_LENGTH - len);
        } else {
            len = rtsp->tcp_len - len;
            ts_buf_peek(rtsp->tcp_buf, len, sb_buf, MSG_MARK_LENGTH);
        }
        rtsp->tcp_match = int_recv_sync_check(rtsp, sb_buf);
    }
    if (rtsp->tcp_match >= 0)
        goto Ok;

    ts_buf_read_pop(rtsp->tcp_buf, 2);
    rtsp->tcp_flag = TCP_FLAG_UNKNOWN;

    return 1;
Ok:
    rtsp->tcp_flag = TCP_FLAG_DATA;
    return 1;
}

static void int_recv_data_push(struct RTSP* rtsp)
{
    StrmBuffer* sb = rtsp->rtp_sb;

    if (rtsp->post_clk) {
        int rrt = rtsp->clk - rtsp->post_clk;
        if (rtsp->open_play) {
            if (rtsp->mult_flag)
                stream_port_post_ok(1, rrt);
            else
                stream_port_post_ok(0, rrt);
        }
        rtsp->post_clk = 0;
    }

    rtsp->stat.stat_bytes += sb->len;
    if (sb->len <= 0)
        return;

    if (rtsp->state == STRM_STATE_FAST && rtsp->skipfast_clk) {
        if (rtsp->skipfast_clk <= rtsp->clk)
            rtsp->skipfast_clk = 0;
        return;
    }

    rtsp->recv_times ++;

    if (rtsp->clear_flg || rtsp->op == RTSP_OP_STOP)
        return;

#ifdef INCLUDE_PVR
    if (rtsp->open_shift || rtsp->rec_mix) {
        if (rtsp->rec_mix && rtsp->rcall_open) {
            rtsp->rcall_push(rtsp->rcall_handle, sb->buf + sb->off, sb->len);
            if (!rtsp->psi_view_record) {
                LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_BEGIN\n", rtsp->index);
                record_post_msg(rtsp->rec_index, rtsp->rec_mix->id, RECORD_MSG_SUCCESS_BEGIN, 0);
                rtsp->psi_view_record = 1;
            }
        } else {
            strm_record_push(rtsp->strm_record, sb->buf + sb->off, sb->len, rtsp->clk);
        }
    }
#endif//INCLUDE_PVR

    if (rtsp->open_play == OPEN_PLAY_RUNNING || (rtsp->open_play == OPEN_PLAY_END && rtsp->ret_flag == 3)) {
        int space, length;

        if (SEEK_END_DUMMY != rtsp->seek_end)
            strm_play_push(rtsp->strm_play, rtsp->index, &rtsp->rtp_sb);
        space = strm_play_space(rtsp->strm_play);
        length = strm_play_length(rtsp->strm_play);

        if (space < length / 7 && rtsp->state == STRM_STATE_PLAY && rtsp->transtype == SOCK_STREAM && rtsp->recv_safe == 1) {
            LOG_STRM_PRINTF("#%d recv safe\n", rtsp->index);
            rtsp->recv_safe = 3;
            rtsp_clt_fdset(rtsp);
        }
    }
}

static int int_recv_data(struct RTSP* rtsp)
{
    int hdr, len;
    StrmBuffer *sb = rtsp->rtp_sb;

    len = rtsp->tcp_len;
    if (len % 188) {
        ts_buf_read(rtsp->tcp_buf, sb->buf, len);

        hdr = ind_rtp_parse(sb->buf, 188, NULL);
        len -= hdr;
        if (hdr < 0 || hdr > 188 || len <= 0 || len % 188) {
            LOG_STRM_WARN("Skip hdr = %d, len = %d\n", hdr, len);
        } else {
            sb->off = hdr;
            sb->len = len;

            stream_port_post_datapush(0, sb->buf + sb->off, len, -1);
            int_recv_data_push(rtsp);
        }
    } else {
        while (rtsp->tcp_len > 0) {
            sb = rtsp->rtp_sb;

            len = rtsp->tcp_len;
            if (len > 1316)
                len = 1316;

            ts_buf_read(rtsp->tcp_buf, sb->buf, len);
            sb->len = len;
            sb->off = 0;
            stream_port_post_datapush(0, sb->buf, len, -1);
            int_recv_data_push(rtsp);

            rtsp->tcp_len -= len;
        }
    }

    if (rtsp->tcp_match > 0) {
        rtsp->tcp_flag = TCP_FLAG_DATA_SYNC;
        ts_buf_read(rtsp->tcp_buf, NULL, 4);
        rtsp->tcp_len = rtsp->tcp_match;
        rtsp->tcp_match = -1;
    } else if (rtsp->tcp_match == 0) {
        rtsp->tcp_flag = TCP_FLAG_CTRL_SYNC;
    } else {
        rtsp->tcp_flag = TCP_FLAG_UNKNOWN;
    }
    return 1;
}

int int_recv_ctrl(struct RTSP* rtsp)
{
    int len;
    char *p;

    if (TCP_FLAG_CTRL_SYNC == rtsp->tcp_flag) {
        len = ts_buf_memstr(rtsp->tcp_buf, "\r\n\r\n");//命令结束符
        if (len < 0) {
            if (ts_buf_length(rtsp->tcp_buf) >= CTRL_BUFFER_LENGTH) {
                ts_buf_read_pop(rtsp->tcp_buf, 1);
                goto End;
            }
            return 0;
        }

        len += 4;
        ts_buf_read(rtsp->tcp_buf, rtsp->ctrl_buf, len);
        rtsp->ctrl_len = len;
        rtsp->ctrl_buf[len] = 0;

        p = ind_memistr(rtsp->ctrl_buf, len, "Content-Length: ");
        if (!p) {
            deal_recv_ctrl(rtsp);
            goto End;
        }

        len = 0;
        sscanf(p + 16, "%d", &len);
        if (rtsp->ctrl_len + len > CTRL_BUFFER_LENGTH)
            goto End;

        rtsp->tcp_len = len;
        rtsp->tcp_flag = TCP_FLAG_CTRL;
    }

    len = ts_buf_length(rtsp->tcp_buf);
    if (len > rtsp->tcp_len)
        len = rtsp->tcp_len;
    ts_buf_read(rtsp->tcp_buf, rtsp->ctrl_buf + rtsp->ctrl_len, len);
    rtsp->tcp_len -= len;
    rtsp->ctrl_len += len;

    if (rtsp->tcp_len > 0)
        return 0;

    rtsp->ctrl_buf[rtsp->ctrl_len] = 0;
    deal_recv_ctrl(rtsp);
End:
    rtsp->tcp_flag = TCP_FLAG_UNKNOWN;
    return 1;
}

int rtsp_recv_tcp(struct RTSP* rtsp)
{
    int ret, len;
    char* buf;

    len = 0;
    ts_buf_write_get(rtsp->tcp_buf, &buf, &len);
    if (len > 0) {
        ret = recv(rtsp->sock, buf, len, 0);
        if (ret <= 0) {
            rtsp->err_no = RTSP_ERROR_SOCKET;
            if (rtsp->signal_flg <= 0)
                rtsp->rtsp_code = RTSP_CODE_Socket_Error;
            else
                rtsp->rtsp_code = RTSP_CODE_server_Error;
            if (errno == EINPROGRESS) {
                LOG_STRM_ERROUT("#%d result = %d, EINPROGRESS\n", rtsp->index, ret);
                return 0;//注意：上一行必须ERR_OUT，遇到很多情况下socket错误就是EINPROGRESS，by ljh 2012-3-1 13:39:28
            }
            LOG_STRM_ERROUT("#%d result = %d, errno = %d! %s\n", rtsp->index, ret, errno, strerror(errno));
        }
        ts_buf_write_put(rtsp->tcp_buf, ret);
    }

    ret = 1;
    while (1 == ret) {
        len = ts_buf_length(rtsp->tcp_buf);
        if (len < MSG_MARK_LENGTH)
            break;
        switch (rtsp->tcp_flag) {
        case TCP_FLAG_DATA_SYNC:    ret = int_recv_data_syunc(rtsp);break;
        case TCP_FLAG_DATA:         ret = int_recv_data(rtsp);      break;
        case TCP_FLAG_CTRL_SYNC:
        case TCP_FLAG_CTRL:         ret = int_recv_ctrl(rtsp);      break;
        default:                    ret = int_recv_sync(rtsp);      break;
        }
    }

    return 0;
Err:
    rtsp_op_failed(rtsp);
    return -1;
}

static void rtsp_recv_seq(struct RTSP* rtsp, RTSPLost_t lost, uint32_t seq)
{
    StrmBuffer* sb = rtsp->rtp_sb;

    if (RTP_INVALID_SEQ != seq) {
        uint32_t pkl = lost->rtp_stat_lost;

        rtsp_stat_seq(rtsp, lost, seq);

        if (rtsp->open_play && lost->rtp_stat_lost > pkl) {
            pkl = lost->rtp_stat_lost - pkl;

            if (STREAM_INDEX_PIP == rtsp->index)
                int_steam_setPacketLost(1, pkl);
            else
                int_steam_setPacketLost(0, pkl);
        }
    }

    //if (hdr <= 0)的判断不能移到下面，否则RTP头会被去掉
    if (rtsp->postvalid == 1 && g_rtsp_sqm_flag == 1) {
        if (RTP_INVALID_SEQ != seq)
            stream_port_post_datapush(0, sb->buf, sb->off + sb->len, (int)seq);
        else
            stream_port_post_datapush(0, sb->buf, sb->len, -1);
    }

    int_recv_data_push(rtsp);
}

int rtsp_recv_rtp(struct RTSP* rtsp)
{
    int ret, hdr, stat_lost, stat_total;
    uint32_t seq;
    RTSPLost_t lost;
    StrmBuffer* sb = rtsp->rtp_sb;

    if (RTSP_STANDARD_CTC_SHANGHAI == rtsp->standard && (2 == rtsp->fcc_flag ||3 == rtsp->arq_flag))
        lost = &rtsp->stat.lost_correct;
    else
        lost = &rtsp->stat.lost_rtp;

    ret = 0;

    stat_lost = lost->rtp_stat_lost;
    stat_total = lost->rtp_stat_total;

    if (sb->len % 188) {
        hdr = ind_rtp_parse(sb->buf, sb->len, &seq);
        if (hdr < 0)
            LOG_STRM_ERROUT("#%d ind_rtp_header\n", rtsp->index);
    } else {
        hdr = 0;
    }

    if (1 == rtsp->mult_flag) {
        if (0 == hdr && rtsp->open_play) {
            int idx = 0;
            if (STREAM_INDEX_PIP == rtsp->index)
                idx = 1;
            int_steam_setTransportProtocol(idx, RTSP_TRANSPORT_UDP);
        }
        rtsp->mult_flag = 2;
    }
    if (hdr > 0) {
        sb->off = hdr;
        sb->len -= hdr;

        rtsp->rtp_seq = seq;
        rtsp_recv_seq(rtsp, lost, seq);

        {
            RTSPStat_t stat = &rtsp->stat;
            stat->stat_bitrate_lost += lost->rtp_stat_lost - stat_lost;
            stat->stat_bitrate_total += lost->rtp_stat_total - stat_total;
        }
    } else {
        sb->off = 0;
        rtsp_recv_seq(rtsp, lost, RTP_INVALID_SEQ);
    }

    return ret;
Err:
    return -1;
}

static void rtsp_recv_udp(struct RTSP* rtsp, int fd)
{
    StrmBuffer* sb = rtsp->rtp_sb;

    sb->len = recv(fd, sb->buf, sb->size, 0);
    if (sb->len <= 0) {
        //rtsp_msg_back(rtsp, STRM_MSG_OPEN_ERROR, 0);
        //LOG_STRM_ERROUT("#%d recv len = %d, errno = %d! %s\n", rtsp->index, len, errno, strerror(errno));
        return;
    }

    rtsp->data_clk = rtsp->clk;

#if 1
    rtsp_recv_rtp(rtsp);
#else
    if (0x47 == sb->buf[0]) {//无RTP头
        rtsp_recv_rtp(rtsp);
    } else {
        strm_bufOrder_push(rtsp->strm_bufOrder, &rtsp->rtp_sb);
        while (1) {
            strm_bufOrder_pop(rtsp->strm_bufOrder, &rtsp->rtp_sb);
            if (rtsp->rtp_sb->len <= 0)
                break;
            rtsp_recv_rtp(rtsp);
        }
        //printf("\n");
    }
#endif
}

static void rtsp_recv_ret(struct RTSP* rtsp, int fd)
{
    StrmBuffer* sb = rtsp->rtp_sb;

    if (RTSP_OP_SERVER_ZTE == rtsp->op_server) {
        struct sockaddr_in from;
        uint32_t s_addr;

        size_t size = sizeof(from);
        from.sin_addr.s_addr = 0;
        sb->len = recvfrom(fd, sb->buf, RTP_BUFFER_LENGTH, 0, (struct sockaddr*)&from, &size);
        s_addr = rtsp->playname.in_addr.s_addr;
        if (s_addr && s_addr != from.sin_addr.s_addr)
            return;
    } else {
        sb->len = recv(fd, sb->buf, RTP_BUFFER_LENGTH, 0);
    }
    if (sb->len <= 0)
        LOG_STRM_ERROUT("#%d recv ret = %d %s\n", rtsp->index, sb->len, strerror(errno));

    rtsp->data_clk = rtsp->clk;
    ret_port_push(rtsp->ret_handle, sb->buf, sb->len);
Err:
    return;
}

static void rtsp_recv_arq_ctc(struct RTSP* rtsp, int fd)
{
    StrmBuffer* sb = rtsp->rtp_sb;

    if (RTSP_OP_SERVER_ZTE == rtsp->op_server) {
        struct sockaddr_in from;
        uint32_t s_addr;

        size_t size = sizeof(from);
        from.sin_addr.s_addr = 0;
        sb->len = recvfrom(fd, sb->buf, RTP_BUFFER_LENGTH, 0, (struct sockaddr*)&from, &size);
        s_addr = rtsp->playname.in_addr.s_addr;
        if (s_addr && s_addr != from.sin_addr.s_addr)
            return;
    } else {
        sb->len = recv(fd, sb->buf, RTP_BUFFER_LENGTH, 0);
    }
    if (sb->len <= 0)
        LOG_STRM_ERROUT("#%d recv ret = %d %s\n", rtsp->index, sb->len, strerror(errno));

    if (RTSP_STANDARD_CTC_SHANGHAI == rtsp->standard && 3 == rtsp->arq_flag) {
        uint32_t seq = 0;
        int hdr = ind_rtp_parse(sb->buf, sb->len, &seq);
        if (hdr > 0)
            rtsp_stat_seq(rtsp, &rtsp->stat.lost_correct, seq);
    }
    arq_port_push(rtsp->arq_ctc_handle, sb->buf, sb->len);
Err:
    return;
}

static void rtsp_recv_rtcp(struct RTSP* rtsp, int fd)
{
	StrmBuffer* sb = rtsp->rtp_sb;

    {
        struct sockaddr_in from;
        uint32_t s_addr;

        size_t size = sizeof(from);
        from.sin_addr.s_addr = 0;
        sb->len = recvfrom(fd, sb->buf, RTP_BUFFER_LENGTH, 0, (struct sockaddr*)&from, &size);
        s_addr = rtsp->playname.in_addr.s_addr;
        if (s_addr && s_addr != from.sin_addr.s_addr)
            return;
    }
    if (sb->len <= 0)
        ERR_OUT("#%d recv ret = %d %s\n", rtsp->index, sb->len, strerror(errno));

    uint32_t seq = 0;
    uint32_t ssrc = 0;
	int hdr = ind_rtp_parse_ex(sb->buf, sb->len, &seq, &ssrc);
    //caculate packet
	if (hdr > 0) {
        if(ssrc == 1) {
            uint8_t rtp_buf[RTP_BUFFER_LENGTH];
            memcpy(rtp_buf, sb->buf, hdr);
            memcpy(rtp_buf + hdr, sb->buf + hdr + 2, sb->len - hdr - 2);
            *(rtp_buf + 1) = 0x21;//type
            unsigned short cseq = (unsigned short)seq;
            *(rtp_buf + 2) = cseq >> 8;
            *(rtp_buf + 3) = cseq;
            //*(rtp_buf + 4) = ;//stamp
            *(rtp_buf + 11) = 0x00;
            rtcp_port_push(rtsp->rtcp_handle, (char*)rtp_buf, sb->len - 2);
        } else if (ssrc == 0){
            rtsp_stat_seq_ex(rtsp, seq);
            rtcp_port_push(rtsp->rtcp_handle, sb->buf, sb->len);
        }
	}

Err:
    return;
}

void rtsp_recv_unicast(struct RTSP* rtsp)
{
    StrmBuffer* sb = rtsp->rtp_sb;

    if (rtsp->iptvtype != IPTV_TYPE_UNICAST
        && (rtsp->state == STRM_STATE_IPTV || rtsp->op == RTSP_OP_STOP))
        recv(rtsp->data_sock, sb->buf, RTP_BUFFER_LENGTH, 0);
    else if (rtsp->state != STRM_STATE_FAST && rtsp->ret_flag == 3)
        rtsp_recv_ret(rtsp, rtsp->data_sock);
    else if(rtsp->state != STRM_STATE_FAST && rtsp->arq_flag == 3)
        rtsp_recv_arq_ctc(rtsp, rtsp->data_sock);
    else if(rtsp->state != STRM_STATE_FAST && rtsp->rtcp_flag == 3)
        rtsp_recv_rtcp(rtsp, rtsp->data_sock);
    else
        rtsp_recv_udp(rtsp, rtsp->data_sock);
}

void rtsp_recv_multicast(struct RTSP* rtsp)
{
    StrmBuffer* sb = rtsp->rtp_sb;

    if (rtsp->state != STRM_STATE_IPTV)
        recv(rtsp->mult_sock, sb->buf, RTP_BUFFER_LENGTH, 0);
    else
        rtsp_recv_udp(rtsp, rtsp->mult_sock);
}

int rtsp_recv_nat(struct RTSP* rtsp)
{
    int ret;
    struct natpkt pkt;

    ret = recv(rtsp->data_sock, rtsp->nat_buf, NAT_BUFFER_LENGTH, 0);
    if (ret <= 0) {
        rtsp->err_no = RTSP_ERROR_SOCKET;
        if (errno == EINPROGRESS)
            LOG_STRM_ERROUT("#%d result = %d, EINPROGRESS\n", rtsp->index, ret);
        LOG_STRM_ERROUT("#%d result = %d, errno = %d! %s\n", rtsp->index, ret, errno, strerror(errno));
    }

    if(ret == sizeof(struct natpkt)) {
        memcpy(&pkt, rtsp->nat_buf, sizeof(struct natpkt));
        LOG_STRM_PRINTF("pkt.ssrc = %d, nat_ssrc = %d, ntohl = %d\n", pkt.ssrc, rtsp->nat_ssrc, ntohl(pkt.ssrc));
        if(pkt.version == 0x00 && pkt.type == NAT_MSG_RESPONSE && ntohl(pkt.ssrc) == rtsp->nat_ssrc)
            rtsp_op_succeed(rtsp);
        else
            LOG_STRM_ERROUT("#%d pkt msg err, version = %d, type = %d, ssrc = %d\n", rtsp->index, pkt.version, pkt.type, pkt.ssrc);
    } else
        LOG_STRM_ERROUT("#%d pkt len err, len = %d\n", rtsp->index, ret);
    return 0;
Err:
    return -1;
}
