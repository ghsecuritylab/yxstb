#include <sys/types.h>
#include <sys/socket.h>

#include "rtsp_app.h"
#include "StatisticRoot.h"

static int g_stat_int_bitrate = 100 * 5;
static int g_stat_int_pklosts = 100 * 10;

static CallBack_StatFlow g_call_statflow = NULL;

void rtsp_stat_init(struct RTSP* rtsp)
{
    rtsp->stat.stat_int_bitrate = g_stat_int_bitrate;
    rtsp->stat.stat_int_pklosts = g_stat_int_pklosts;

    LOG_STRM_DEBUG("statflow = %p\n", g_call_statflow);
    rtsp->stat.call_statflow = g_call_statflow;
}

static void rtsp_stat_bitrate(void* arg)
{
    uint32_t clk, clks, rate, bitrate, bitpercent, pktpercent;
    RTSPStat_t stat;
    struct RTSP *rtsp = (struct RTSP *)arg;

    stat = &rtsp->stat;

    clk = rtsp->clk;
    clks = clk - stat->stat_clock;
    stat->stat_clock = clk;

    if (clks < 100)
        return;

    rate = stat->stat_bytes * 8 / (clks * 10);
    stat->stat_bytes = 0;

    bitrate = strm_play_byte_rate(rtsp->strm_play) * 8;

    if(stat->stat_bitrate_lost > 0 && stat->stat_bitrate_lost < stat->stat_bitrate_total)
        pktpercent = (stat->stat_bitrate_total - stat->stat_bitrate_lost) * 1000 / stat->stat_bitrate_total;
    else
        pktpercent = 1000;

    bitpercent = strm_play_byte_percent(rtsp->strm_play, rtsp->index, pktpercent);
    LOG_STRM_DEBUG("#%d bitpercent = %d, pktpercent = %d\n", rtsp->index, bitpercent, pktpercent);

    if (rtsp->open_play && STREAM_INDEX_PIP != rtsp->index) {
        int width, mult;

        width = strm_play_get_width(rtsp->strm_play);
        if (rtsp->mult_flag)
            mult = 1;
        else
            mult = 0;
        stream_port_post_bandwidth(mult, width, rate);
        stream_port_post_bitrate(mult, width, bitrate);
        stream_port_post_bitrate_percent(mult, width, bitpercent);
    }

    stat->stat_bitrate = bitrate;
    stat->stat_bitrate_total = 0;
    stat->stat_bitrate_lost = 0;
}

static void rtsp_stat_pklosts(void* arg)
{
    RTSPStat_t stat;
    RTSPLost_t lost;
    int width, stat_total, stat_lost;
    struct RTSP *rtsp = (struct RTSP *)arg;

    if (rtsp->transtype == SOCK_STREAM && rtsp->mult_sock == -1)
        return;

    stat = &rtsp->stat;
    stat_lost = 0;
    stat_total = 0;

    lost = &stat->lost_rtp;
    width = strm_play_get_width(rtsp->strm_play);
    if (lost->rtp_stat_total > 0) {
        LOG_STRM_DEBUG("#%d mult_flag = %d lost_rtp lost = %d / %d\n", rtsp->index, rtsp->mult_flag, lost->rtp_stat_lost, lost->rtp_stat_total);

        if (rtsp->open_play && STREAM_INDEX_PIP != rtsp->index) {
            if (0 == rtsp->mult_flag)
                stream_port_post_pklosts(PcketsLost_Flag_VOD, width, lost->rtp_stat_total, lost->rtp_stat_lost);
            else
                stream_port_post_pklosts(PcketsLost_Flag_MULT, width, lost->rtp_stat_total, lost->rtp_stat_lost);
        }

        if (0 == stat_total) {
            stat_lost = lost->rtp_stat_lost;
            stat_total = lost->rtp_stat_total;
        }
        rtsp_stat_reset(lost, 0);
    }

    lost = &stat->lost_correct;
    if (lost->rtp_stat_total > 0) {
        LOG_STRM_DEBUG("#%d mult_flag = %d lost_correct lost = %d / %d\n", rtsp->index, rtsp->mult_flag, lost->rtp_stat_lost, lost->rtp_stat_total);
        if (rtsp->open_play && STREAM_INDEX_PIP != rtsp->index) {
            if (0 == rtsp->mult_flag)
                stream_port_post_pklosts(PcketsLost_Flag_ARQ, width, lost->rtp_stat_total, lost->rtp_stat_lost);
            else
                stream_port_post_pklosts(PcketsLost_Flag_FEC, width, lost->rtp_stat_total, lost->rtp_stat_lost);
        }

        if (0 == stat_total) {
            stat_lost = lost->rtp_stat_lost;
            stat_total = lost->rtp_stat_total;
        }
        rtsp_stat_reset(lost, 0);
    }

    if (stat_total > 0) {
        LOG_STRM_DEBUG("#%d lost = %d / %d\n", rtsp->index, stat_lost, stat_total);
        if (stat_lost > stat_total / 100)
            stat->lost_warn++;
        else
            stat->lost_warn = 0;

        if (stat->lost_warn >= 2) {
            stat->lost_warn = 0;
            if (rtsp->open_play && STREAM_INDEX_PIP != rtsp->index) {
                LOG_STRM_PRINTF("#%d STRM_MSG_PACKETS_LOST\n", rtsp->index);
                rtsp_post_msg(rtsp, STRM_MSG_PACKETS_LOST, 0);
            }
        }
    }
}

void rtsp_stat_reset(RTSPLost_t lost, int clear)
{
    lost->rtp_stat_total = 0;
    lost->rtp_stat_lost = 0;

    if (clear) {
        lost->rtp_lost_num = 0;
        lost->rtp_lost_clk = 0;

        lost->rtp_seq = RTP_INVALID_SEQ;
    }
}

//快进或暂停时不统计丢包率，
//播放状态发生变化，必须重置统计信息，错计丢包率
void rtsp_stat_set(struct RTSP* rtsp, int flag)
{
    RTSPStat_t stat;

    if (!rtsp->open_play)
        return;
    stat = &rtsp->stat;

#ifdef DEBUG_BUILD
    {
        unsigned int seq = stat->lost_rtp.rtp_seq;
        LOG_STRM_DEBUG("#%d flag = %d, seq = 0x%04x/%d\n", rtsp->index, flag, seq, seq);
    }
#endif

    stat->lost_warn = 0;

    rtsp_stat_reset(&stat->lost_rtp, 1);
    rtsp_stat_reset(&stat->lost_correct, 1);

    stat->stat_bitrate_total = 0;
    stat->stat_bitrate_lost = 0;
    if (flag) {
        unsigned int clk = rtsp->clk;
        stat->stat_clock = clk;
        stat->stat_bytes = 0;
        ind_timer_create(rtsp->tlink, clk + stat->stat_int_bitrate, stat->stat_int_bitrate, rtsp_stat_bitrate, rtsp);
        ind_timer_create(rtsp->tlink, clk + stat->stat_int_pklosts, stat->stat_int_pklosts, rtsp_stat_pklosts, rtsp);

    } else {
        ind_timer_delete(rtsp->tlink, rtsp_stat_bitrate, rtsp);
        ind_timer_delete(rtsp->tlink, rtsp_stat_pklosts, rtsp);
    }
}

void rtsp_stat_seq(struct RTSP* rtsp, RTSPLost_t lost, uint32_t seq)
{
    int n;

    if (lost->rtp_seq == RTP_INVALID_SEQ) {
        lost->rtp_lost_num = 0;
        n = 1;
    } else if (seq <= lost->rtp_seq)
        n = seq + 0x10000 - lost->rtp_seq;
    else
        n = seq - lost->rtp_seq;

    lost->rtp_stat_total += (uint32_t)n;
    if (n > 1) {
        n --;
        lost->rtp_stat_lost += (uint32_t)n;

        lost->rtp_lost_seq0 = lost->rtp_seq;
        lost->rtp_lost_seq1 = seq;
        if (lost->rtp_lost_num == 0) {
            LOG_STRM_WARN("#%d lost = %d, rtp_seq = %d / %d\n", rtsp->index, n, lost->rtp_seq, seq);
            lost->rtp_lost_clk = rtsp->clk + INTERVAL_CLK_LOST;
        }
        lost->rtp_lost_num += n;
    }
    if (lost->rtp_lost_num > 0) {
        if (lost->rtp_lost_clk < rtsp->clk) {
            LOG_STRM_WARN("#%d total lost = %d, rtp_seq = %d/%d\n", rtsp->index, lost->rtp_lost_num, lost->rtp_lost_seq0, lost->rtp_lost_seq1);
            lost->rtp_lost_num = 0;
        }
    }
    lost->rtp_seq = seq;
}

void rtsp_stat_seq_ex(struct RTSP* rtsp, uint seq)
{
    int n;
    int i = 0;

    if (rtsp->rtcp_seq == RTP_INVALID_SEQ) {
        n = 1;
    } else if (seq <= rtsp->rtcp_seq) {
        n = seq + RTP_INVALID_SEQ - rtsp->rtcp_seq;
    } else {
        n = seq - rtsp->rtcp_seq;
    }

    n--;
    if (n < 0) {
        ERR_PRN("same or err packet:%d\n", n);
    } else if (n == 0) {
        if(seq - rtsp->rtcp_lostseq > RTCP_SEQ_NUM && rtsp->rtcp_lostseq) {
            rtsp_clt_rtcpfb(rtsp);
            rtsp->rtcp_lostseq = 0;
            rtsp->rtcp_lostnum = 0;
            rtsp->rtcp_lostblp = 0;
        }
    } else {//have lost 1-17
        if(rtsp->rtcp_lostseq == 0) {//no lost            
            rtsp->rtcp_lostseq = (seq - n) >= 0 ? (seq - n) : (RTP_INVALID_SEQ + seq - n);
            rtsp->rtcp_lostnum = n;
            if(n == 1) {
                rtsp->rtcp_lostblp = 0;
            } 
            else {
                for(i = 0; i < n - 1; i++) {
                    if(i >= 16) {
                        rtsp_clt_rtcpfb(rtsp);
                        rtsp->rtcp_lostseq = 0;
                        rtsp->rtcp_lostnum = 0;
                        rtsp->rtcp_lostblp = 0;
                        break;
                    }
                    rtsp->rtcp_lostblp += 1 << i;
                }                
            }
        } else {
            if(seq - rtsp->rtcp_lostseq > RTCP_SEQ_NUM) {
                rtsp_clt_rtcpfb(rtsp);
                rtsp->rtcp_lostseq = 0;
                rtsp->rtcp_lostnum = 0;
                rtsp->rtcp_lostblp = 0;
            } else {
                int other_lost = (seq - n) >= 0 ? (seq - n) : (RTP_INVALID_SEQ + seq - n);
                int pos = ((other_lost - rtsp->rtcp_lostseq) >= 0 ? (other_lost - rtsp->rtcp_lostseq) : (RTP_INVALID_SEQ + other_lost - rtsp->rtcp_lostseq)) - 1;
                for(i = 0; i < n; i++)
                    rtsp->rtcp_lostblp += 0x8000 >> (pos + i);
            }
        }
    }

    rtsp->rtcp_seq = seq;
}

void mid_stream_statint_pklosts(int interval)
{
    if (interval < 5 || interval > 3600)
        LOG_STRM_ERROUT("interval = %d\n", interval);
    LOG_STRM_PRINTF("interval = %d\n", interval);
    g_stat_int_pklosts = interval * 100;

Err:
    return;
}

void mid_stream_statint_bitrate(int interval)
{
    if (interval < 5 || interval > 3600)
        LOG_STRM_ERROUT("interval = %d\n", interval);
    LOG_STRM_PRINTF("interval = %d\n", interval);
    g_stat_int_bitrate = interval * 100;

Err:
    return;
}

void mid_stream_callback_statflow(CallBack_StatFlow statflow)
{
    g_call_statflow = statflow;
    LOG_STRM_PRINTF("statflow = %p\n", g_call_statflow);
}
