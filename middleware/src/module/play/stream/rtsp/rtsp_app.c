/*********************************************************************/
/*        FileName        yxvod.c                                    */
/*        Author          xiaole                                     */
/*        Mail            xiaole@yuxing.com                          */
/*        Create Date     2005-11-2 17:02                            */
/*        Version         1.0                                        */
/*        Brief           Bitband Server enginer for Conexent        */
/*********************************************************************/

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

#include "sys_basic_macro.h"

#include "rtsp_app.h"

static int rtsp_stop(struct RTSP* rtsp);

//流量控制
static int g_burst_flag = 0;

//华为新SQA库的标志位
static int g_arq_flag = 0;

typedef struct {
    int fcc_type;
    char igmp[IGMP_INFO_SIZE];
    int igmp_size;

    MultiPlayCall play_call;
    MultiStopCall stop_call;
} ArgExt;

typedef struct {
    char url[STREAM_URL_SIZE];
    char tmshift_url[STREAM_URL_SIZE];

    int adv_num;
    struct Advertise adv_array[ADVERTISE_NUM];
    MultiPlayCall    igmp_play_call;
    MultiStopCall    igmp_stop_call;
    char*            igmp[IGMP_INFO_SIZE];
    int                igmp_size;

    int fcc_type;
    int shiftlen;
    int    record;
    int begin;
    int end;
} RtspArg;

static ArgExt *g_argextArray = NULL;

static unsigned int g_port_tcp = 0;
static unsigned int g_port_udp = 0;

static int g_iframe_flag = 0;

//心跳周期
static int g_heartbit_period = HEARTBIT_PERIOD_SEC;
static int g_heartbit_standard = 0;

static int g_transport = 2;
static int g_standard = RTSP_STANDARD_YUXING;

static int g_multicast_unicast = 2;//IPTV 国内 STB设计规格.doc 3.3.5 G03-05 组播无流转单播
static uint32_t g_multicast_clock = 0;//组播转单播超时时间

static int g_multicast_forbid = 0;
//提前打开解码器
static int g_timeshift_second = 0;

static int g_cache_clk_level = CACHE_CLK_LEVEL;
static int g_cache_clk_range = CACHE_CLK_RANGE;

static int g_size_play = STREAM_BLOCK_LEVEL_PLAY;
static int g_size_cache = STREAM_BLOCK_LEVEL_CACHE;
/*
    安全接收，TCP传输方式下，缓冲快满时停止接收
 */
static int g_recv_safe = 1;

static uint32_t g_port_off = 0;

typedef struct {
    uint32_t    clk;
    uint32_t    magic;
} ChannelZap;

static ChannelZap g_channelZap[2] = {{0, 0}, {0, 0}};

static uint32_t g_skipfast_clks = 0;
static uint32_t g_nat_heartbitperiod = NAT_HEARTBIT_PERIOD_SEC;
static uint32_t g_nat_flag = 0;

static CallBack_RtspInfo g_call_rtspinfo = NULL;
static char *g_useragent = NULL;

static struct RTSP* g_rtsp = NULL;

int g_rtsp_sqm_flag = 1;

/*
    VOD积累一定数据才开始播放
 */
static int g_vodlevel = 0;
static int g_pushsync = 1;

#ifdef INCLUDE_PVR
static void tstv_state      (struct RTSP* rtsp, STRM_STATE state, int scale);
static int  tstv_pause      (struct RTSP* rtsp);
static int  tstv_resume     (struct RTSP* rtsp);
static int  tstv_fast       (struct RTSP* rtsp, int scale);
static int  tstv_seek       (struct RTSP* rtsp, uint32_t offset);
static int  tstv_stop       (struct RTSP* rtsp);
static int  tstv_record     (struct RTSP* rtsp);
static int  tstv_open_timeshift     (struct RTSP* rtsp);
static int  tstv_close_timeshift    (struct RTSP* rtsp);

static int  rtsp_close_record   (struct RTSP *rtsp, uint32_t id, int end);
static void rtsp_record_end     (struct RTSP* rtsp, uint32_t id, uint32_t endtime);
#endif//INCLUDE_PVR

void rtsp_post_msg(struct RTSP* rtsp, STRM_MSG msgno, int arg)
{
    if (rtsp->open_play == OPEN_PLAY_CLOSE)
        return;
    stream_post_msg(rtsp->index, msgno, arg);
}

#ifdef INCLUDE_PVR
static void rtsp_post_record(struct RTSP* rtsp, uint32_t id, STRM_MSG msgno, int arg)
{
    RecordMix_t rec_mix;

    rec_mix = rtsp->rec_mix;
    while (rec_mix) {
        if (!id || id == rec_mix->id)
            record_post_msg(rtsp->rec_index, rec_mix->id, msgno, arg);
        rec_mix = rec_mix->next;
    }
}
#endif//INCLUDE_PVR

void rtsp_post_state(struct RTSP* rtsp, STRM_STATE state, int scale)
{
    if (rtsp->open_play == OPEN_PLAY_CLOSE || state == STRM_STATE_ADVERTISE)
        return;
    stream_post_state(rtsp->index, state, scale);
}

static int rtsp_open(struct RTSP* rtsp, APP_TYPE apptype, char* url, char* tmshift_url)
{
#ifdef DEBUG_BUILD
    ind_sin_t rrs_sins;
    StrmRRS_t strmRRS = &rtsp->strmRRS;
#endif

    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    if (rtsp->state != STRM_STATE_CLOSE)
        LOG_STRM_ERROUT("#%d %d\n", rtsp->index, rtsp->state);

    if (APP_TYPE_VOD == apptype)
        rtsp->voddelay_level = g_vodlevel;
    else
        rtsp->voddelay_level = 0;
    rtsp->voddelay_clk = 0;

    rtsp->standard = g_standard;

    if (g_useragent) {
        IND_STRCPY(rtsp->UserAgent, g_useragent);
    } else {
        LOG_STRM_DEBUG("#%d standard = %d\n", rtsp->index, rtsp->standard);
        if (rtsp->standard == RTSP_STANDARD_YUXING)
            IND_STRCPY(rtsp->UserAgent, "YX_STB");
        else if (rtsp->standard == RTSP_STANDARD_HUAWEI)
            IND_STRCPY(rtsp->UserAgent, "HUAWEI");//Huawei IPTV
        else if (rtsp->standard == RTSP_STANDARD_CTC_SHANGHAI)
            IND_STRCPY(rtsp->UserAgent, "HW_STB; CTC/2.0");
        else
            IND_STRCPY(rtsp->UserAgent, "CTC RTSP 1.0");
    }

    if (rtsp->index != STREAM_INDEX_PIP) {
        rtsp->cache_clk_level = g_cache_clk_level;
        rtsp->cache_clk_range = g_cache_clk_range;
        rtsp->size_play = g_size_play;
        rtsp->size_cache = g_size_cache;
        LOG_STRM_DEBUG("#%d cache_time = %d / %d, size = %d / %d\n", rtsp->index, rtsp->cache_clk_level, rtsp->cache_clk_range, rtsp->size_play, rtsp->size_cache);
    }

    rtsp->port_tcp = g_port_tcp;
    rtsp->port_udp = g_port_udp;

    rtsp->burst_flag = 0;
    if (RTSP_MAIN_PLAY(rtsp) && g_burst_flag == 1)
        rtsp->burst_flag = 1;

    LOG_STRM_DEBUG("#%d burst_flag = %d\n", rtsp->index, rtsp->burst_flag);

    rtsp->arq_flag = g_arq_flag;
    rtsp->iframe_flag = g_iframe_flag;

    rtsp->heartbit_period = g_heartbit_period;
    rtsp->heartbit_standard = g_heartbit_standard;
    rtsp->transport = g_transport;

    rtsp->recv_safe = 0;
    if (RTSP_MAIN_PLAY(rtsp) && g_recv_safe)
        rtsp->recv_safe = 1;

    LOG_STRM_DEBUG("#%d heartbit_period = %d, transport = %d, recv_safe = %d\n", rtsp->index, rtsp->heartbit_period, rtsp->transport, rtsp->recv_safe);

    rtsp_clt_reset(rtsp);

    rtsp->timeshift_second = g_timeshift_second;

    if (g_multicast_unicast == 3 && mid_10ms( ) - g_multicast_clock > 30000)
        g_multicast_unicast = 2;
    rtsp->multicast_unicast = g_multicast_unicast;
    rtsp->multicast_forbid = g_multicast_forbid;

    rtsp->apptype = apptype;
    rtsp->iptvtype = IPTV_TYPE_UNICAST;
    rtsp->timeshift_len = 0;

    if (rtsp->transport == RTSP_TRANSPORT_UDP || rtsp->transport == RTSP_TRANSPORT_RTP_UDP)
        rtsp->nat_flag = g_nat_flag;
    else
        rtsp->nat_flag = 0;
    rtsp->nat_heartbitperiod = g_nat_heartbitperiod;

    if (rtsp->apptype == APP_TYPE_VOD) {
        if (rtsp_clt_parse_url(rtsp, url))
            LOG_STRM_ERROUT("#%d rtsp_clt_parse_url\n", rtsp->index);
    } else {
        if (rtsp_clt_parse_iptv(rtsp, url, tmshift_url))
            LOG_STRM_ERROUT("#%d rtsp_clt_parse_iptv\n", rtsp->index);
    }

#ifdef DEBUG_BUILD
    {
        int i;
        rrs_sins = strmRRS->rrs_sins;
        for (i = 0; i < strmRRS->rrs_num; i ++)
            LOG_STRM_PRINTF("#%d rrs[%d] = %s:%hd\n", rtsp->index, i, rtsp_clt_addr_fmt(rtsp, &rrs_sins[i]), rrs_sins[i].port);
    }
#endif

    if (3 == rtsp->multicast_unicast && IPTV_TYPE_MULTICAST == rtsp->iptvtype) {
        LOG_STRM_DEBUG("#%d MULTICAST_TIMESHIFT to UNICAST\n", rtsp->index);
        rtsp->iptvtype = IPTV_TYPE_UNICAST;
    }

    if (rtsp->iptvtype == IPTV_TYPE_UNICAST)
        rtsp_op_resolve(rtsp, 0);//rtsp_clt_parse_url(rtsp, rtsp->channel_url);会导致VOD也使用channel_url

    LOG_STRM_DEBUG("#%d STRM_STATE_OPEN iptvtype = %d\n", rtsp->index, rtsp->iptvtype);
    rtsp->state = STRM_STATE_OPEN;

    if (rtsp->iptvtype != IPTV_TYPE_UNICAST)
        rtsp_clt_iptv_open(rtsp);

    ind_timer_create(rtsp->tlink, rtsp->clk + INTERVAL_CLK_100MS, INTERVAL_CLK_100MS, rtsp_clt_period_100ms, rtsp);

    return 0;
Err:
    rtsp->err_no = RTSP_ERROR_URL;
    return -1;
}

static void rtsp_close(struct RTSP* rtsp)
{
    ind_timer_delete(rtsp->tlink, rtsp_clt_period_100ms, rtsp);
    rtsp_op_immediate_reg(rtsp, RTSP_OP_TEARDOWN);
}

static void rtsp_unicast(struct RTSP* rtsp)
{
    rtsp_clt_iptv_close(rtsp);
    rtsp->iptvtype = IPTV_TYPE_UNICAST;
    rtsp->retry_op = RTSP_OP_NONE;
    rtsp->retry_flg = 1;
    rtsp->timeshift_status = 0;

    if (rtsp->multicast_unicast == 2) {
        g_multicast_unicast = 3;
        g_multicast_clock = mid_10ms( );
        rtsp->multicast_unicast = g_multicast_unicast;
    }

    if (rtsp->sock != -1)
        rtsp_clt_close_vod(rtsp);

    rtsp_clt_parse_url(rtsp, rtsp->channel_url);
    rtsp_op_resolve(rtsp, 0);
}

static void rtsp_1000ms(void* arg)
{
    struct RTSP* rtsp = (struct RTSP*)arg;

    if (rtsp->open_play)
        rtsp_clt_time_1000ms(rtsp);

    if (rtsp->recv_clk <= rtsp->clk) {
        if (rtsp->open_play) {
            int diff, buffer, length, byterate;

            diff = strm_play_diff(rtsp->strm_play);
            buffer = strm_play_buffer(rtsp->strm_play);
            length = strm_play_length(rtsp->strm_play);
            byterate = strm_play_byte_rate(rtsp->strm_play);
            LOG_STRM_PRINTF("#%d length = %d, diff = %d, buffer = %d, recv_times = %d, byterate = %d, cache = %d (%d, %d)\n", rtsp->index, length, diff, buffer, rtsp->recv_times, byterate, rtsp->cache, rtsp->cache_clk_level, rtsp->cache_clk_range);
            LOG_STRM_DEBUG("#%d open_play = %d, open_shift = %d, rec_mix = %p\n", rtsp->index, rtsp->open_play, rtsp->open_shift, rtsp->rec_mix);
        } else {
            LOG_STRM_PRINTF("#%d rec_mix = %p, recv_times = %d\n", rtsp->index, rtsp->rec_mix, rtsp->recv_times);
        }

        while(rtsp->recv_clk <= rtsp->clk)
            rtsp->recv_clk += INTERVAL_CLK_RECV_TIMEOUT;
        rtsp->recv_times = 0;
    }

#ifdef INCLUDE_PVR
    {
        RecordMix_t rec_mix = rtsp->rec_mix;
        while (rec_mix) {
            if (rec_mix->clk && rec_mix->clk <= rtsp->clk) {
                uint32_t id = rec_mix->id;

                LOG_STRM_PRINTF("#%d id = 0x%08x\n", rtsp->index, id);
                rtsp_close_record(rtsp, id, 1);
                record_back_close(id);
                break;
            }
            rec_mix = rec_mix->next;
        }
    }
#endif

    if (rtsp->fcc_timeout >= 0) {
#if defined(Jiangsu) || defined(ANDROID)//Android版本暂不支持fcc，后续支持fcc功能后在放开;江苏不支持fcc
        rtsp->fcc_timeout = -2;
#else
        int mflag = fcc_port_mflag(rtsp->fcc_handle);
        LOG_STRM_DEBUG("#%d timeout = %d, mflag = %d\n", rtsp->index, rtsp->fcc_timeout, mflag);
        if (mflag < 2) {
            if (1 == mflag) {
                rtsp->fcc_timeout ++;
                if (rtsp->fcc_timeout >= 5)
                    rtsp_unicast(rtsp);
            }
        } else {
            rtsp->fcc_timeout = -2;
        }
#endif
    }
}

static int rtsp_open_play(struct RTSP* rtsp, PlayArg *arg, RtspArg *rtsparg)
{
    LOG_STRM_PRINTF("#%d arg->shiftlen = %d, rec_mix = %p\n", rtsp->index, rtsparg->shiftlen, rtsp->rec_mix);

    int_steam_postPlay( );

    if (0 == rtsparg->url[0]) {
        rtsp->rtsp_code = RTSP_CODE_URL_FORMAT_Error;
        LOG_STRM_ERROUT("#%d RTSP_CODE_URL_FORMAT_Error\n", rtsp->index);
    }

    if (STREAM_INDEX_PIP != rtsp->index) {
        rtsp->call_rtspinfo = g_call_rtspinfo;
        rtsp_stat_init(rtsp);
    }

    rtsp->psi_view_play = 0;
    rtsp->trickmode = 0;

    rtsp->ppv_begin = 0;
    rtsp->ppv_end = 0;

    rtsp->rec_time = 0;
    rtsp->cmdsn = 1;

    rtsp->time_length = 0;
#ifdef INCLUDE_PVR
    memset(&rtsp->pvr_info, 0, sizeof(rtsp->pvr_info));
#endif

    rtsp->ply_magic = arg->magic;

    if (!rtsp->rec_mix) {
        rtsp->fcc_flag = 0;
        rtsp->fcc_type = rtsparg->fcc_type;
        if (rtsp->fcc_type)
            rtsp->fcc_flag = 1;

        rtsp->ret_flag = 1;

        rtsp->igmp_play_call = rtsparg->igmp_play_call;
        rtsp->igmp_stop_call = rtsparg->igmp_stop_call;

        rtsp->igmp_size = rtsparg->igmp_size;
        if (rtsp->igmp_size > 0)
            IND_MEMCPY(rtsp->igmp, rtsparg->igmp, rtsp->igmp_size);
    }
    LOG_STRM_DEBUG("#%d fcc_flag = %d\n", rtsp->index, rtsp->fcc_flag);

    strm_play_open(rtsp->strm_play, rtsp->index, rtsp, rtsp_msg_back, arg->apptype, RTP_BUFFER_LENGTH);
    strm_play_set_idle(rtsp->strm_play, rtsp->index, 1);
    strm_play_set_timeout(rtsp->strm_play);
    if (rtsp->stat.call_statflow)
        strm_play_set_flow(rtsp->strm_play);

    rtsp->open_play = OPEN_PLAY_RUNNING;
    rtsp->psi_view_play = 0;

    if (!rtsp->rec_mix) {
        if (rtsp_open(rtsp, arg->apptype, rtsparg->url, rtsparg->tmshift_url))
            LOG_STRM_ERROUT("#%d rtsp_open\n", rtsp->index);
    } else {
        rtsp->apptype = arg->apptype;
        if (rtsp->apptype == APP_TYPE_IPTV)
            IND_STRCPY(rtsp->tmshift_url, rtsparg->tmshift_url);
    }

    if (rtsp->apptype == APP_TYPE_VOD && rtsparg->adv_num > 0) {
        int i;
        for (i = 0; i < rtsparg->adv_num; i ++)
            rtsp->adv_array[i] = rtsparg->adv_array[i];
        rtsp->adv_num = rtsparg->adv_num;
        rtsp->apptype = APP_TYPE_VOD;
    } else {
        rtsp->adv_num = 0;
    }

    if (rtsp->apptype == APP_TYPE_VOD) {
        if (rtsparg->begin >= 0) {
            rtsp->ctc_begin = rtsparg->begin;
            rtsp->ctc_end = rtsparg->end;
        } else {
            rtsp->ctc_begin = -1;
        }
        rtsp->time_length = 0;
        if (rtsparg->shiftlen <= 0) {
            rtsp_clt_time_set_current(rtsp, 0);
        } else {
            rtsp_clt_time_set_current(rtsp, rtsparg->shiftlen);
        }
        if (rtsp->index < STREAM_INDEX_PIP) {
            int size = strm_play_space(rtsp->strm_play);
            int_steam_setToalBufferSize(size);
        }
        LOG_STRM_DEBUG("#%d time_current = %d\n", rtsp->index, rtsp->time_current);
    } else {
        uint32_t now, begin, end;

        rtsp_clt_time_set_current(rtsp, 0);
        if (rtsparg->shiftlen > 0) {
            rtsp->timeshift_len = rtsparg->shiftlen;
            rtsp_clt_time_set_total(rtsp, rtsparg->shiftlen);
        }

        now = mid_time( );
        begin = rtsparg->begin;
        end = rtsparg->end;
        if (end != 0 && end != (uint32_t)-1) {
            if (end < now)
                LOG_STRM_ERROUT("#%d end = %u, now = %u\n", rtsp->index, end, now);
            rtsp->ppv_end = end;
            rtsp->time_length = 0;

            if (begin != 0 && begin != (uint32_t)-1) {
                if (end <= begin)
                    LOG_STRM_ERROUT("#%d begin = %u, end = %u, now = %u\n", rtsp->index, begin, end, now);
                rtsp->ppv_begin = begin;
                if (rtsp->ppv_begin > now)
                    rtsp->open_play = OPEN_PLAY_PPVWAIT;
            }
        }
        rtsp_clt_time_set_current(rtsp, rtsp_clt_time_server(rtsp));
        LOG_STRM_DEBUG("#%d time_current = %d\n", rtsp->index, rtsp->time_current);
    }

    if (rtsp->apptype == APP_TYPE_VOD || rtsparg->shiftlen != 0)
        rtsp->trickmode = 1;
    if (rtsp->apptype == APP_TYPE_IPTV) {
        if (rtsparg->tmshift_url[0] == 0)
            rtsp->trickmode = 0;
        LOG_STRM_DEBUG("#%d apptype = %d, shiftlen = %d, trickmode = %d\n", rtsp->index, rtsp->apptype, rtsparg->shiftlen, rtsp->trickmode);
    }

#ifdef INCLUDE_PVR
    if (rtsp->apptype == APP_TYPE_TSTV) {
        int timeshift = strm_record_shifttime( );

        rtsp->shift_id = arg->shiftid;

        if (rtsparg->shiftlen > timeshift) {
            LOG_STRM_ERROR("#%d: shiftlen = %d, timeshift = %d\n", rtsp->index, rtsparg->shiftlen, timeshift);
            rtsp->trickmode = 0;
        }
        rtsp->pvr_state = STRM_STATE_IPTV;
        rtsp->pvr_scale = 1;
    }
#endif//INCLUDE_PVR

    if (RTSP_MAIN_PLAY(rtsp) && rtsp->iptvtype == IPTV_TYPE_UNICAST)
        rtsp->stat.stat_vodreq = rtsp->clk;

    LOG_STRM_DEBUG("#%d arg->shiftlen = %d, rtsp->trickmode = %d\n", rtsp->index, rtsparg->shiftlen, rtsp->trickmode);

    if (rtsp->apptype == APP_TYPE_VOD)
        rtsp->skipfast_clks = g_skipfast_clks;

    return 0;
Err:

    if (rtsp->open_play) {
        rtsp_post_state(rtsp, STRM_STATE_CLOSE, 0);
#ifdef INCLUDE_PVR
        if (rtsp->open_shift)
            tstv_close_timeshift(rtsp);
#endif
        rtsp_clt_close_play(rtsp);
    }

    if (!rtsp->rec_mix) {
        LOG_STRM_PRINTF("#%d STRM_STATE_CLOSE\n", rtsp->index);
        rtsp->state = STRM_STATE_CLOSE;
    }
    LOG_STRM_PRINTF("#%d STRM_MSG_OPEN_ERROR\n", rtsp->index);
    stream_post_msg(rtsp->index, STRM_MSG_OPEN_ERROR, rtsp->rtsp_code);

    return -1;
}

static void rtsp_close_play(struct RTSP* rtsp, int clear)
{
    LOG_STRM_PRINTF("#%d clear = %d state = %d\n", rtsp->index, clear, rtsp->state);

    if (RTSP_MAIN_PLAY(rtsp) && rtsp->iptvtype == IPTV_TYPE_UNICAST)
        rtsp->stat.stat_vodstop = rtsp->clk;

    if (rtsp->state == STRM_STATE_ADVERTISE) {
        strm_play_leader_set(rtsp->strm_play, rtsp->index);
        mid_stream_close(STREAM_INDEX_ADV, 0);
    }

    ind_timer_delete(rtsp->tlink, rtsp_clt_shiftcheck, rtsp);
    ind_timer_delete(rtsp->tlink, rtsp_clt_rangeout, rtsp);

    rtsp->call_rtspinfo = NULL;

    rtsp_stat_set(rtsp, 0);
    rtsp->clear_flg = clear;

#ifdef INCLUDE_PVR
    if (rtsp->apptype == APP_TYPE_TSTV) {
        tstv_state(rtsp, STRM_STATE_CLOSE, 0);
        if (rtsp->pvr) {
            ind_pvr_close(rtsp->pvr);
            rtsp->pvr = NULL;
        }
    }
#endif//INCLUDE_PVR

    if (!rtsp->rec_mix) {
        rtsp_close(rtsp);
    } else {
        LOG_STRM_PRINTF("#%d STRM_STATE_CLOSE\n", rtsp->index);
        rtsp_post_state(rtsp, STRM_STATE_CLOSE, 0);
#ifdef INCLUDE_PVR
        if (rtsp->open_shift)
            tstv_close_timeshift(rtsp);
#endif
        rtsp_clt_close_play(rtsp);
    }

     if (rtsp->open_play) {
        if (STREAM_INDEX_PIP == rtsp->index)
            int_steam_setPacketLost(1, 0);
        else
            int_steam_setPacketLost(0, 0);
    }
}

#ifdef INCLUDE_PVR
static int rtsp_open_record(struct RTSP *rtsp, RecordArg *arg, RtspArg *rtsparg)
{
    uint32_t id;
    RecordMix_t rec_mix;
    PvrArgument_t pvrarg;

    int open_record = 0;

    rec_mix = rtsp->rec_mix;
    while (rec_mix) {
        LOG_STRM_PRINTF("#%d record = 0x%08x\n", rtsp->index, rec_mix->id);
        rec_mix = rec_mix->next;
    }
    rec_mix = NULL;

    pvrarg = &arg->pvrarg;
    id = pvrarg->id;

    if (rtsp->open_play && rtsp->state != STRM_STATE_IPTV) {
        LOG_STRM_PRINTF("#%d RECORD_MSG_PVR_CONFLICT\n", rtsp->index);
        record_post_msg(arg->index, pvrarg->id, RECORD_MSG_PVR_CONFLICT, 0);
        LOG_STRM_ERROR("#%d open_play = %d, state = %d\n", rtsp->index, rtsp->open_play, rtsp->state);
        return -1;
    }

    if (!rtsp->rec_mix && !rtsp->open_play) {
        rtsp->fcc_flag = 0;
        rtsp->fcc_type = rtsparg->fcc_type;
        if (rtsp->fcc_type)
            rtsp->fcc_flag = 1;

        rtsp->ret_flag = 1;

        rtsp->igmp_play_call = rtsparg->igmp_play_call;
        rtsp->igmp_stop_call = rtsparg->igmp_stop_call;

        rtsp->igmp_size = rtsparg->igmp_size;
        if (rtsp->igmp_size > 0)
            IND_MEMCPY(rtsp->igmp, rtsparg->igmp, rtsp->igmp_size);
    }
    LOG_STRM_PRINTF("#%d id = 0x%08x open_play = %d rec_mix = %p, fcc_flag = %d, ret_flag = %d\n", rtsp->index, id, rtsp->open_play, rtsp->rec_mix, rtsp->fcc_flag, rtsp->ret_flag);

    pvrarg->realtime = 1;
    if (arg->begin > 0 && arg->begin < arg->end)
        arg->pvrarg.time_length = arg->end - arg->begin;

    if (!rtsp->rec_mix && rtsp->open_shift == 0 && arg->add == 1)
        ind_pvr_rec_rebreak(arg->pvrarg.id);

    if (arg->rcall_open) {
        if (rtsp->rec_mix || rtsp->open_play)
            LOG_STRM_ERROUT("#%d rec_mix = %p, open_play = %d\n", rtsp->index, rtsp->rec_mix, rtsp->open_play);

        rtsp->rcall_open = arg->rcall_open;
        rtsp->rcall_push = arg->rcall_push;
        rtsp->rcall_close = arg->rcall_close;

        rtsp->rcall_handle = rtsp->rcall_open(pvrarg->info_buf, pvrarg->info_len, pvrarg->id);
        if (-1 == rtsp->rcall_handle)
            LOG_STRM_ERROUT("#%d rcall_open\n", rtsp->index);
    } else {
        if (rtsp->rec_mix || rtsp->open_shift)
            strm_record_mix_open(rtsp->strm_record, arg);
        else
            strm_record_open(rtsp->strm_record, rtsp, rtsp_msg_back, arg);
    }
    open_record = 1;

    rec_mix = (RecordMix_t)IND_CALLOC(sizeof(RecordMix), 1);
    if (!rec_mix)
        LOG_STRM_ERROUT("#%d malloc RecordMix\n", rtsp->index);
    rec_mix->id = pvrarg->id;

    if (!rtsp->rec_mix) {
        if (rtsp->open_play == OPEN_PLAY_CLOSE) {
            if (rtsp_open(rtsp, arg->apptype, rtsparg->url, NULL))
                LOG_STRM_ERROUT("#%d rtsp_open\n", rtsp->index);
        }
        rtsp->rec_index = arg->index;
    }

    {
	    RecordMix_t mix, prev;

        prev = NULL;
        mix = rtsp->rec_mix;
        while (mix && mix->clk) {
            prev = mix;
            mix = mix->next;
        }
        rec_mix->next = mix;
        if (prev)
            prev->next = rec_mix;
        else
            rtsp->rec_mix = rec_mix;
    }

    if (arg->end)
        rtsp_record_end(rtsp, pvrarg->id, arg->end);

    return 0;
Err:

    if (rtsp->rcall_open && rtsp->rcall_open == arg->rcall_open)
        rtsp->rcall_open = NULL;
    if (rec_mix && rec_mix != rtsp->rec_mix)
        IND_FREE(rec_mix);

    if (open_record) {
        if (arg->rcall_open) {
             rtsp->rcall_close(rtsp->rcall_handle);
             rtsp->rcall_handle = -1;
        } else {
            if (rtsp->open_play == OPEN_PLAY_CLOSE) {
                LOG_STRM_PRINTF("#%d STRM_STATE_CLOSE\n", rtsp->index);
                rtsp->state = STRM_STATE_CLOSE;
            }
            if (rtsp->open_shift)
                strm_record_mix_close(rtsp->strm_record, pvrarg->id, 0);
            else
                strm_record_close(rtsp->strm_record, 0);
        }
    }
    LOG_STRM_PRINTF("#%d RECORD_MSG_ERROR\n", rtsp->index);
    record_post_msg(arg->index, pvrarg->id, RECORD_MSG_ERROR, 0);
    return -1;
}

static int rtsp_close_record(struct RTSP *rtsp, uint32_t id, int end)
{
    RecordMix_t prev, rec_mix;

    LOG_STRM_PRINTF("#%d end = %d\n", rtsp->index, end);

    prev = NULL;
    rec_mix = rtsp->rec_mix;
    if (id) {
        while (rec_mix) {
            if (id == rec_mix->id)
                break;
            prev = rec_mix;
            rec_mix = rec_mix->next;
        }
    }

    if (!rec_mix)
        LOG_STRM_ERROUT("#%d id = 0x%08x\n", rtsp->index, id);

    if (rtsp->rcall_open) {
        rtsp->rcall_open = NULL;

        rtsp->rcall_close(rtsp->rcall_handle);
        rtsp->rcall_handle = -1;

        if (1 == end) {
            LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_END\n", rtsp->index);
            record_post_msg(rtsp->rec_index, rec_mix->id, RECORD_MSG_SUCCESS_END, 0);
        }
    } else {
        if (!id || (!prev && !rec_mix->next)) {
            if (rtsp->open_shift) {
                RecordMix_t mix = rtsp->rec_mix;
                while (mix) {
                    strm_record_mix_close(rtsp->strm_record, mix->id, end);
                    mix = mix->next;
                }
            } else {
                strm_record_close(rtsp->strm_record, end);
            }
        } else {
            strm_record_mix_close(rtsp->strm_record, id, end);
        }
    }

    if (id) {
        if (1 != end) {
            LOG_STRM_PRINTF("#%d RECORD_MSG_CLOSE id = 0x%08x\n", rtsp->index, rec_mix->id);
            record_post_msg(rtsp->rec_index, rec_mix->id, RECORD_MSG_CLOSE, 0);
        }
        if (prev)
            prev->next = rec_mix->next;
        else
            rtsp->rec_mix = rec_mix->next;
        IND_FREE(rec_mix);
    } else {
        while (rtsp->rec_mix) {
            rec_mix = rtsp->rec_mix;
            rtsp->rec_mix = rec_mix->next;
            if (1 != end) {
                LOG_STRM_PRINTF("#%d RECORD_MSG_CLOSE id = 0x%08x\n", rtsp->index, rec_mix->id);
                record_post_msg(rtsp->rec_index, rec_mix->id, RECORD_MSG_CLOSE, 0);
            }
            IND_FREE(rec_mix);
        }
    }

    if (!rtsp->rec_mix && OPEN_PLAY_CLOSE == rtsp->open_play)
        rtsp_close(rtsp);

    return 0;
Err:
    return -1;
}

static void rtsp_record_end(struct RTSP* rtsp, uint32_t id, uint32_t endtime)
{
    uint32_t clk, now;
    RecordMix_t rec_mix;

    now = mid_time( );
    LOG_STRM_PRINTF("#%d now = %s, end = %s\n", rtsp->index, rtsp_clt_time_fmt(rtsp, 0, now), rtsp_clt_time_fmt(rtsp, 1, endtime));
    if (endtime <= now) {
        LOG_STRM_WARN("#%d endtime = %u, now = %u\n", rtsp->index, endtime, now);
        clk = rtsp->clk;
    } else {
        clk = rtsp->clk + (endtime - now) * 100;
    }

    rec_mix = rtsp->rec_mix;
    while (rec_mix) {
        if (!id || id == rec_mix->id)
            rec_mix->clk = clk;
        rec_mix = rec_mix->next;
    }
}
#endif

static void rtsp_view(struct RTSP* rtsp, int msgno)
{
#if defined(ANHUI_CTC_SD)
    if (3 == rtsp->multicast_unicast && rtsp->mult_flag) {
        LOG_STRM_DEBUG("#%d MULTICAST_TIMESHIFT to RESET\n", rtsp->index);
        rtsp->multicast_unicast = 2;
    }
#endif
    if (!rtsp->signal_flg) {
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_RESUME\n", rtsp->index);
        rtsp_msg_back(rtsp, STRM_MSG_RECV_RESUME, 0);
        rtsp->signal_flg = 1;
    }

    if (rtsp->psi_view_play == 1)//广告播放会产生多个STRM_MSG_STREAM_VIEW
        return;
    LOG_STRM_DEBUG("#%d STRM_MSG_STREAM_VIEW\n", rtsp->index);
    rtsp->psi_view_play = 1;
    rtsp_post_msg(rtsp, STRM_MSG_STREAM_VIEW, 0);

    {
        RTSPStat_t stat = &rtsp->stat;
        if (stat->stat_vodreq) {
            int ms = (int)(rtsp->clk - stat->stat_vodreq) * 10;

            stream_port_post_vodreq(ms);

            stat->stat_vodreq = 0;
            LOG_STRM_DEBUG("#%d vodreq = %d\n", rtsp->index, ms);
        }
    }
}

static int rtsp_msg(struct RTSP* rtsp, STRM_MSG msgno, int arg)
{
    int idx = 0;

    LOG_STRM_DEBUG("#%d msgno = %d, arg = %d\n", rtsp->index, msgno, arg);
    if (STREAM_INDEX_PIP == rtsp->index)
        idx = 1;

    switch(msgno) {
    case STRM_MSG_OPEN_ERROR:
        LOG_STRM_PRINTF("#%d STRM_MSG_OPEN_ERROR rtsp_code = %d, psi_view_play = %d\n", rtsp->index, rtsp->rtsp_code, rtsp->psi_view_play);
        rtsp_post_msg(rtsp, STRM_MSG_OPEN_ERROR, rtsp->rtsp_code);

        int_steam_postAbnormal( );

        if (rtsp->apptype == APP_TYPE_TSTV && rtsp->state == STRM_STATE_IPTV) {
            rtsp_clt_time_set_current(rtsp, rtsp_clt_time_server(rtsp));
            LOG_STRM_DEBUG("#%d time_current = %d\n", rtsp->index, rtsp->time_current);
        }
        if (APP_TYPE_IPTV == rtsp->apptype && rtsp->state > STRM_STATE_OPEN) {
            LOG_STRM_PRINTF("#%d state = %d, mult_flag = %d, timeshift = %d, %d\n", rtsp->index, rtsp->state, rtsp->mult_flag, rtsp->timeshift_status, rtsp->timeshift_support);
            if (rtsp->timeshift_status && !rtsp->timeshift_support) {
                if (rtsp->state != STRM_STATE_IPTV) {
                    rtsp_stop(rtsp);
                    return 0;
                }
                if (rtsp->mult_flag) {
                    rtsp->timeshift_status = 0;
                    rtsp_clt_close_vod(rtsp);
                    return 0;
                }
            }
        }

        rtsp_close_play(rtsp, 1);
        break;
    case STRM_MSG_PLAY_ERROR:
        rtsp_close_play(rtsp, 1);
        int_back_rtspMethod(idx, "");
        LOG_STRM_PRINTF("#%d STRM_MSG_PLAY_ERROR\n", rtsp->index);
        rtsp_post_msg(rtsp, STRM_MSG_OPEN_ERROR, arg);
        break;
    case STRM_MSG_UNSUPPORT_MEDIA:
        LOG_STRM_DEBUG("#%d STRM_MSG_UNSUPPORT_MEDIA\n", rtsp->index);
        rtsp_post_msg(rtsp, STRM_MSG_UNSUPPORT_MEDIA, 0);
        break;

    case STRM_MSG_CHANGE_PSI:
        LOG_STRM_DEBUG("#%d STRM_MSG_CHANGE_PSI\n", rtsp->index);
        break;

    case STRM_MSG_CHANGE_CRC:
        LOG_STRM_DEBUG("#%d STRM_MSG_CHANGE_CRC\n", rtsp->index);
        break;

    case RECORD_MSG_PSI_VIEW:
        LOG_STRM_DEBUG("#%d RECORD_MSG_PSI_VIEW\n", rtsp->index);
        rtsp->psi_view_record = 1;
        if (rtsp->rec_mix && rtsp->rec_wait == 1) {
            stream_back_wake(rtsp->rec_wait_id);
            rtsp->rec_wait = 0;
        }
        break;
    case STRM_MSG_PSI_VIEW:
        LOG_STRM_PRINTF("#%d STRM_MSG_PSI_VIEW\n", rtsp->index);
        break;
    case STRM_MSG_PTS_VIEW:
        rtsp_post_msg(rtsp, STRM_MSG_PTS_VIEW, arg);
        if (APP_TYPE_IPTV != rtsp->apptype && APP_TYPE_TSTV != rtsp->apptype) {
            LOG_STRM_DEBUG("#%d STRM_MSG_PTS_VIEW apptype = %d\n", rtsp->index, rtsp->apptype);
            break;
        }

        {
            int ms;

            LOG_STRM_DEBUG("#%d STRM_MSG_PTS_VIEW apptype = %d, magic = %08x / %08x\n", rtsp->index, rtsp->apptype, g_channelZap[idx].magic, rtsp->ply_magic);

            if (g_channelZap[idx].clk && g_channelZap[idx].magic + 1 == rtsp->ply_magic) {
                ms = (int)(mid_10ms( ) - g_channelZap[idx].clk) * 10;

                stream_port_post_channelzap(ms);

                g_channelZap[idx].clk = 0;
                g_channelZap[idx].magic = 0;
                LOG_STRM_DEBUG("#%d channelzap = %d\n", rtsp->index, ms);
            }
        }
        break;
    case STRM_MSG_STREAM_BEGIN:
        LOG_STRM_DEBUG("#%d STRM_MSG_STREAM_BEGIN\n", rtsp->index);
        rtsp_post_msg(rtsp, STRM_MSG_STREAM_BEGIN, 0);
#ifdef INCLUDE_PVR
        if (rtsp->apptype == APP_TYPE_TSTV) {
            tstv_seek(rtsp, 0);
        } else
#endif//INCLUDE_PVR
        {
            rtsp_stat_set(rtsp, 0);

            rtsp->op_off = 0;
            rtsp->op_type = PLAY_TYPE_SET;
            rtsp->op_scale = 1;

            rtsp->retry_flg = 0;
            rtsp->retry_op = RTSP_OP_PLAY;
            rtsp->retry_op_off = 0;

            rtsp_op_immediate_reg(rtsp, RTSP_OP_PLAY);
        }
        break;
    case STRM_MSG_STREAM_END:
        LOG_STRM_DEBUG("#%d STRM_MSG_STREAM_END apptype = %d\n", rtsp->index, rtsp->apptype);
#ifdef INCLUDE_PVR
        if (rtsp->apptype == APP_TYPE_TSTV) {
            LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", rtsp->index);
            rtsp_post_msg(rtsp, STRM_MSG_STREAM_END, 0);
            tstv_stop(rtsp);
        } else
#endif//INCLUDE_PVR
        {
            if (rtsp->apptype == APP_TYPE_VOD) {
                rtsp_clt_time_set_current(rtsp, rtsp->time_length);
                LOG_STRM_DEBUG("#%d time_current = %d\n", rtsp->index, rtsp->time_current);
            } else {
                rtsp_clt_time_set_current(rtsp, rtsp_clt_time_server(rtsp));
                LOG_STRM_DEBUG("#%d time_current = %d\n", rtsp->index, rtsp->time_current);
            }
            rtsp->time_start = rtsp->time_current;
            rtsp_stat_set(rtsp, 0);
            if (rtsp->index == STREAM_INDEX_ADV) {
                rtsp_close_play(rtsp, 0);
                break;
            }

            if (rtsp->adv_num > 0 && rtsp_clt_advertise_elem(rtsp, -2)) {
                rtsp->adv_insert = -2;
                rtsp_cmd_queuecmd(rtsp, STREAM_CMD_ADVERTISE);
                break;
            }

            if (RTSP_MAIN_PLAY(rtsp) && SEEK_END_DUMMY != rtsp->seek_end)
                rtsp_post_msg(rtsp, STRM_MSG_STREAM_END, 0);

            if (rtsp->apptype == APP_TYPE_VOD) {
            } else {
                rtsp_stop(rtsp);
            }
        }
        break;

    case STRM_MSG_PPV_END:
        LOG_STRM_DEBUG("#%d STRM_MSG_PPV_END\n", rtsp->index);
        rtsp_post_msg(rtsp, STRM_MSG_PPV_END, 0);
        rtsp_close_play(rtsp, 0);
        break;

    case STRM_MSG_STREAM_MUSIC:
        LOG_STRM_DEBUG("#%d STRM_MSG_STREAM_MUSIC\n", rtsp->index);
        rtsp_view(rtsp, msgno);
        if (rtsp->music_flg == 1)
            break;
        rtsp->music_flg = 1;
        rtsp_clt_post_valid(rtsp, 0);

        rtsp_post_msg(rtsp, STRM_MSG_STREAM_MUSIC, 0);
        break;
    case STRM_MSG_STREAM_VIDEO:
        LOG_STRM_DEBUG("#%d STRM_MSG_STREAM_VIDEO\n", rtsp->index);
        rtsp_view(rtsp, msgno);
        if (rtsp->music_flg == 0)
            break;
        rtsp->music_flg = 0;
        rtsp_clt_post_valid(rtsp, 1);

        rtsp_post_msg(rtsp, STRM_MSG_STREAM_VIDEO, 0);
        break;
    case STRM_MSG_RECV_FIRST:
        LOG_STRM_DEBUG("#%d STRM_MSG_RECV_FIRST\n", rtsp->index);
        rtsp_post_msg(rtsp, STRM_MSG_RECV_FIRST, 0);
        break;
    case STRM_MSG_FIRST_TIMEOUT:
        LOG_STRM_DEBUG("#%d STRM_MSG_FIRST_TIMEOUT\n", rtsp->index);
        if (rtsp->signal_flg)
         rtsp->signal_flg = -1;
#ifdef ENABLE_IPV6
        if (rtsp->mult_flag && rtsp->mult_sin.family == AF_INET6)
            stream_port_mld_report(&rtsp->mult_sin);
#endif
        if (rtsp->mult_flag && rtsp->mult_sin.family == AF_INET)
            stream_port_multicast_report(rtsp->mult_sin.in_addr.s_addr);
        break;

    case STRM_MSG_FREEZE:
        LOG_STRM_PRINTF("#%d STRM_MSG_FREEZE state = %d\n", rtsp->index, rtsp->state);
        if (STRM_STATE_IPTV == rtsp->state) {
            rtsp_clt_reset_play(rtsp, 0);
            strm_play_resume(rtsp->strm_play, rtsp->index, 1);
        }
        break;

    case STRM_MSG_RECV_TIMEOUT:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_TIMEOUT state = %d, multicast_unicast = %d, apptype = %d, trickmode = %d\n", rtsp->index, rtsp->state, rtsp->multicast_unicast, rtsp->apptype, rtsp->trickmode);

        int_steam_postAbnormal( );

        if (rtsp->post_clk) {
            if (rtsp->open_play) {
                if (rtsp->mult_flag)
                    stream_port_post_fail(1, rtsp->igmp_info, RTSP_ERROR_TIMEOUT);
                else
                    stream_port_post_fail(0, rtsp->url, RTSP_ERROR_TIMEOUT);
            }
            rtsp->post_clk = 0;
        }

        if (STRM_STATE_IPTV == rtsp->state && rtsp->index < STREAM_INDEX_PIP && rtsp->multicast_unicast != 0 && rtsp->iptvtype == IPTV_TYPE_MULTICAST && rtsp->channel_url[0]) {
            int unicast = 0;
            if (rtsp->fcc_flag >= 2) {
#if defined(Jiangsu) || defined(ANDROID)// Android版本暂不支持fcc，后续支持fcc功能后在放开;江苏不支持fcc
                 unicast = 1;
#else
                int mflag = fcc_port_mflag(rtsp->fcc_handle);
                LOG_STRM_PRINTF("#%d mflag = %d\n", rtsp->index, mflag);
                if (mflag < 2)
                    unicast = 1;
#endif
            } else {
                if (0 == rtsp->psi_view_play)
                    unicast = 1;
            }
            if (unicast) {
                rtsp_unicast(rtsp);
                break;
            }
        }

        if (rtsp->rtsp_state != RTSP_STATE_TEARDOWN) {
            int interval = rtsp->heartbit_period * 100;
            ind_timer_create(rtsp->tlink, rtsp->clk + INTERVAL_CLK_SESSION_TIMEOUT, interval, rtsp_clt_heartbit, rtsp);
        }

        rtsp->signal_flg = 0;
        int_back_rtspMethod(idx, "");
        if (rtsp->mult_flag)
            rtsp_post_msg(rtsp, STRM_MSG_RECV_TIMEOUT, 1);
        else
            rtsp_post_msg(rtsp, STRM_MSG_RECV_TIMEOUT, 0);

        if (rtsp->ret_flag > 2)
            rtsp_clt_ret_open(rtsp);

        if (rtsp->mult_flag) {
            #ifdef ENABLE_IPV6
            if (rtsp->mult_sin.family == AF_INET6)
                stream_port_mld_report(&rtsp->mult_sin);
            #endif
            if (rtsp->mult_sin.family == AF_INET)
                stream_port_multicast_report(rtsp->mult_sin.in_addr.s_addr);

            stream_port_post_abend_fail(rtsp->igmp_info);
        } else {
            stream_port_post_abend_fail(rtsp->url);
        }

        stream_port_post_streamgap( );

        break;

    case STRM_MSG_RECV_TIMEOUT15:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_TIMEOUT15\n", rtsp->index);
        if (rtsp->mult_flag)
            rtsp_post_msg(rtsp, STRM_MSG_RECV_TIMEOUT15, 1);
        else
            rtsp_post_msg(rtsp, STRM_MSG_RECV_TIMEOUT15, 0);
        break;

    case STRM_MSG_RECV_RESUME:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_RESUME signal = %d\n", rtsp->index, rtsp->signal_flg);
        if (!rtsp->signal_flg) {
            stream_port_post_abend_end( );
            if (rtsp->state != STRM_STATE_CLOSE)
                rtsp_post_msg(rtsp, STRM_MSG_RECV_RESUME, 0);
        }
        rtsp->signal_flg = 1;
        break;

    case STRM_MSG_OVERFLOW:
        {
            int width = strm_play_get_width(rtsp->strm_play);
            LOG_STRM_PRINTF("#%d STRM_MSG_OVERFLOW width = %d\n", rtsp->index, width);
            if (width > 0 && rtsp->stat.call_statflow) {
                if (rtsp->mult_flag)
                    rtsp->stat.call_statflow(1, width, 1);
                else
                    rtsp->stat.call_statflow(0, width, 1);
            }
        }
        break;

    case STRM_MSG_UNDERFLOW:
        {
            int width = strm_play_get_width(rtsp->strm_play);
            LOG_STRM_PRINTF("#%d STRM_MSG_UNDERFLOW\n", rtsp->index);
            if (width <= 0)
                width = 720;
            if (rtsp->stat.call_statflow) {
                if (rtsp->mult_flag)
                    rtsp->stat.call_statflow(1, width, -1);
                else
                    rtsp->stat.call_statflow(0, width, -1);
            }
        }
        break;

    case STRM_MSG_PSI_ERROR:
        LOG_STRM_PRINTF("#%d STRM_MSG_PSI_ERROR\n", rtsp->index);
        rtsp_post_msg(rtsp, STRM_MSG_PSI_ERROR, 0);
        break;
    case STRM_MSG_PSI_RESUME:
        LOG_STRM_PRINTF("#%d STRM_MSG_PSI_RESUME\n", rtsp->index);
        rtsp_post_msg(rtsp, STRM_MSG_PSI_RESUME, 0);
        break;

    case STRM_MSG_BUF_FULL:
        LOG_STRM_DEBUG("#%d STRM_MSG_BUF_FULL\n", rtsp->index);
        if (CACHE_STATE_ON == rtsp->cache) {
            rtsp->retry_op = RTSP_OP_NONE;
            rtsp_op_immediate_reg(rtsp, RTSP_OP_CACHE_OFF);
        }
        break;
    case STRM_MSG_BUF_EMPTY:
        if (STRM_STATE_PLAY != rtsp->state)
            break;
        LOG_STRM_DEBUG("#%d STRM_MSG_BUF_EMPTY cache = %d, burst = %d\n", rtsp->index, rtsp->cache, rtsp->burst_flag);
        if (rtsp->burst_flag >= 3)
            break;
        if (CACHE_STATE_OFF_INIT == rtsp->cache || CACHE_STATE_OFF == rtsp->cache)
            rtsp_op_immediate_reg(rtsp, RTSP_OP_CACHE_ON);
        break;

#ifdef INCLUDE_PVR
    case RECORD_MSG_ERROR:
    case RECORD_MSG_DISK_ERROR:
    case RECORD_MSG_DISK_FULL:
    case RECORD_MSG_PVR_CONFLICT:
    case RECORD_MSG_DATA_DAMAGE:
        {
            RecordMix_t rec_mix;
            uint32_t id = (uint32_t)arg;

            switch(msgno) {
            case RECORD_MSG_DISK_ERROR:     LOG_STRM_ERROR("#%d RECORD_MSG_DISK_ERROR...msgno = %d, rec_mix = %p\n", rtsp->index, msgno, rtsp->rec_mix);    break;
            case RECORD_MSG_DISK_FULL:      LOG_STRM_ERROR("#%d RECORD_MSG_DISK_FULL....msgno = %d, rec_mix = %p\n", rtsp->index, msgno, rtsp->rec_mix);    break;
            case RECORD_MSG_PVR_CONFLICT:   LOG_STRM_ERROR("#%d RECORD_MSG_PVR_CONFLICT.msgno = %d, rec_mix = %p\n", rtsp->index, msgno, rtsp->rec_mix);    break;
            case RECORD_MSG_DATA_DAMAGE:    LOG_STRM_ERROR("#%d RECORD_MSG_DATA_DAMAGE..msgno = %d, rec_mix = %p\n", rtsp->index, msgno, rtsp->rec_mix);    break;
            default:                        LOG_STRM_ERROR("#%d RECORD_MSG_ERROR........msgno = %d, rec_mix = %p\n", rtsp->index, msgno, rtsp->rec_mix);    break;
            }
            rec_mix = rtsp->rec_mix;
            if (id) {
                while (rec_mix) {
                    if (id == rec_mix->id)
                        break;
                }
            }
            if (rec_mix) {
                rtsp_post_record(rtsp, id, msgno, 0);
                rtsp_close_record(rtsp, id, 0);
            }
            if (rtsp->open_shift && (0 == id || id == rtsp->shift_id)) {
                if (msgno == RECORD_MSG_ERROR) {
                    if (rtsp->pvr == NULL)
                        rtsp_post_msg(rtsp, RECORD_MSG_DATA_DAMAGE, 0);
                    else if (strm_record_size( ) == -1)
                        rtsp_post_msg(rtsp, RECORD_MSG_DISK_DETACHED, 0);
                    else
                        rtsp_post_msg(rtsp, RECORD_MSG_ERROR, 0);
                } else {
                    rtsp_post_msg(rtsp, msgno, 0);
                }
                tstv_close_timeshift(rtsp);
            }
        }
        break;
    case RECORD_MSG_DISK_WARN:
        LOG_STRM_PRINTF("#%d RECORD_MSG_DISK_WARN\n", rtsp->index);
        if (rtsp->rec_mix)
            rtsp_post_record(rtsp, 0, RECORD_MSG_DISK_WARN, 0);
        break;
    case RECORD_MSG_NET_TIMEOUT:
        LOG_STRM_PRINTF("#%d RECORD_MSG_NET_TIMEOUT\n", rtsp->index);
        if (rtsp->rec_mix)
            rtsp_post_record(rtsp, 0, RECORD_MSG_NET_TIMEOUT, 0);
        break;
    case RECORD_MSG_SUCCESS_BEGIN:
        LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_BEGIN\n", rtsp->index);
        if (rtsp->rec_mix)
            rtsp_post_record(rtsp, (uint32_t)arg, RECORD_MSG_SUCCESS_BEGIN, 0);
        else
            rtsp_post_msg(rtsp, RECORD_MSG_SUCCESS_BEGIN, 0);
        break;
    case RECORD_MSG_SUCCESS_END:
        LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_END\n", rtsp->index);
        break;
#endif

    default:
        LOG_STRM_ERROUT("#%d msgno = %d\n", rtsp->index, msgno);
    }

    return 0;
Err:
    return -1;
}

#define ASSESS_PLAY_NORMAL( )\
    if (rtsp->open_play == OPEN_PLAY_CLOSE)\
        LOG_STRM_ERROUT("#%d open_play is false\n", rtsp->index);\
    if (rtsp->rec_mix)\
        LOG_STRM_ERROUT("#%d open_record is true\n", rtsp->index)

static int rtsp_pause(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d state = %d, open_play = %d\n", rtsp->index, rtsp->state, rtsp->open_play);
    ASSESS_PLAY_NORMAL( );

    switch(rtsp->state) {
    case STRM_STATE_PLAY:
    case STRM_STATE_FAST:
    case STRM_STATE_IPTV:
        break;
    default:
        LOG_STRM_ERROUT("#%d %d\n", rtsp->index, rtsp->state);
    }

    if (rtsp->apptype == APP_TYPE_VOD && rtsp->state == STRM_STATE_PLAY && rtsp->open_play == OPEN_PLAY_END) {
        LOG_STRM_PRINTF("#%d STRM_STATE_PAUSE\n", rtsp->index);
        strm_play_pause(rtsp->strm_play, rtsp->index);
        rtsp_clt_state(rtsp, STRM_STATE_PAUSE, 0);
        return 0;
    }

    if (rtsp->apptype == APP_TYPE_VOD) {
        /*
        if (rtsp->time_length <= 0)
            LOG_STRM_ERROUT("#%dtime_length = %d\n", rtsp->index, rtsp->time_length);
        */
        rtsp_op_immediate_reg(rtsp, RTSP_OP_PAUSE);
    } else {
        LOG_STRM_PRINTF("#%d timeshift_second = %d, timeshift_support = %d\n", rtsp->index, rtsp->timeshift_second, rtsp->timeshift_support);

        rtsp->timeshift_status = 1;
        if (rtsp->state == STRM_STATE_IPTV
            && ((rtsp->iptvtype != IPTV_TYPE_UNICAST && rtsp->sock == -1)
            ||  (rtsp->iptvtype == IPTV_TYPE_UNICAST && rtsp->timeshift_second == 1 && rtsp->timeshift_support == 0))) {

            rtsp->servtime_diff = 0;
            rtsp_clt_time_set_current(rtsp, mid_time( ));

            rtsp->retry_op_off = mid_time( );

            rtsp->retry_op = RTSP_OP_PAUSE;
            rtsp->retry_flg = 1;
            rtsp_op_immediate_reg(rtsp, RTSP_OP_TIMESHIFT);
        } else {
            rtsp_op_immediate_reg(rtsp, RTSP_OP_PAUSE);
        }
    }

    return 0;
Err:
    rtsp_post_state(rtsp, rtsp->state, rtsp->scale);
    return -1;
}

static int rtsp_resume(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d state = %d, open_play = %d\n", rtsp->index, rtsp->state, rtsp->open_play);
    ASSESS_PLAY_NORMAL( );

    switch(rtsp->state) {
    case STRM_STATE_PLAY:   return 0;
    case STRM_STATE_PAUSE:  break;
    case STRM_STATE_FAST:
        if (OPEN_PLAY_END == rtsp->open_play) {
            rtsp_clt_reset_play(rtsp, 0);
            strm_play_resume(rtsp->strm_play, rtsp->index, 0);
            strm_play_set_idle(rtsp->strm_play, rtsp->index, 1);
            LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", rtsp->index);
            rtsp_clt_state(rtsp, STRM_STATE_PLAY, 1);
            return 0;
        }
        if (rtsp->adv_num > 0)
            rtsp_clt_advertise_select(rtsp, 0, rtsp->time_current);
        break;
    default:
        LOG_STRM_ERROUT("#%d %d\n", rtsp->index, rtsp->state);
    }

    if (rtsp->apptype == APP_TYPE_VOD && rtsp->state == STRM_STATE_PAUSE && rtsp->open_play == OPEN_PLAY_END) {
        LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", rtsp->index);
        strm_play_resume(rtsp->strm_play, rtsp->index, 0);
        strm_play_end(rtsp->strm_play, rtsp->index);
        rtsp_clt_state(rtsp, STRM_STATE_PLAY, 1);
        return 0;
    }

    rtsp_op_immediate_reg(rtsp, RTSP_OP_RESUME);

    return 0;
Err:
    rtsp_post_state(rtsp, rtsp->state, rtsp->scale);
    return -1;
}

static int rtsp_seek(struct RTSP* rtsp, uint32_t seek)
{
    LOG_STRM_PRINTF("#%d seek = %d / %d\n", rtsp->index, seek, rtsp->time_length);

    ASSESS_PLAY_NORMAL( );

    switch(rtsp->state) {
    case STRM_STATE_PLAY:   break;
    case STRM_STATE_PAUSE:  break;
    case STRM_STATE_FAST:   break;
    case STRM_STATE_IPTV:   break;
    default: LOG_STRM_ERROUT("#%d %d\n", rtsp->index, rtsp->state);
    }

    rtsp->op_scale = -1;//表示seek命令操作
    rtsp->op_off = seek;
    if (rtsp->apptype == APP_TYPE_VOD) {
        if (rtsp->adv_num > 0)
            rtsp_clt_advertise_select(rtsp, 0, seek);

        if (rtsp->time_length <= 0)
            LOG_STRM_ERROUT("#%d time_length = %d\n", rtsp->index, rtsp->time_length);

        rtsp_op_immediate_reg(rtsp, RTSP_OP_SEEK);
    } else {
        if (rtsp->ppv_begin && seek < rtsp->ppv_begin)
            seek = rtsp->ppv_begin;

        rtsp->timeshift_status = 1;
        LOG_STRM_DEBUG("#%d: now = %s,  seek = %s\n", rtsp->index, rtsp_clt_time_fmt(rtsp, 0, mid_time( )), rtsp_clt_time_fmt(rtsp, 1, seek));
        if (rtsp->state == STRM_STATE_IPTV
            && ((rtsp->iptvtype != IPTV_TYPE_UNICAST && rtsp->sock == -1)
            ||  (rtsp->iptvtype == IPTV_TYPE_UNICAST && rtsp->timeshift_second == 1 && rtsp->timeshift_support == 0))) {

            rtsp->servtime_diff = 0;
            rtsp_clt_time_set_current(rtsp, mid_time( ));

            rtsp->retry_op = RTSP_OP_SEEK;
            rtsp->retry_op_off = seek;
            rtsp->retry_op_scale = -1;
            rtsp->retry_flg = 1;
            rtsp_op_immediate_reg(rtsp, RTSP_OP_TIMESHIFT);
        } else {
            if (seek != 0) {
                rtsp->op_off += rtsp->servtime_diff;
                LOG_STRM_DEBUG("#%d seek = %d\n", rtsp->index, rtsp->op_off);
            }

            rtsp_op_immediate_reg(rtsp, RTSP_OP_SEEK);
        }
    }

    return 0;
Err:
    rtsp_post_state(rtsp, rtsp->state, rtsp->scale);
    return -1;
}

static int rtsp_fast(struct RTSP* rtsp, int scale)
{
    LOG_STRM_PRINTF("#%d scale = %d\n", rtsp->index, scale);
    ASSESS_PLAY_NORMAL( );

    switch(rtsp->state) {
    case STRM_STATE_PLAY:    break;
    case STRM_STATE_PAUSE:    break;
    case STRM_STATE_FAST:    break;
    case STRM_STATE_IPTV:    if (scale < 0)    break;    LOG_STRM_ERROUT("#%d STRM_STATE_IPTV scale = %d\n", rtsp->index, scale);
    default:                LOG_STRM_ERROUT("#%d %d\n", rtsp->index, rtsp->state);
    }

    if (rtsp->music_flg == 1)
        LOG_STRM_ERROUT("#%d music_flg is true!\n", rtsp->index);

    rtsp->op_off = 0;
    rtsp->op_scale = scale;

    if (rtsp->apptype == APP_TYPE_VOD) {
        if (rtsp->adv_num > 0)
            rtsp_clt_advertise_select(rtsp, 0, rtsp->time_current);

        if (rtsp->time_length <= 0)
            LOG_STRM_ERROUT("#%d time_length = %d\n", rtsp->index, rtsp->time_length);

        if (CACHE_STATE_ON == rtsp->cache) {
            if (rtsp->ret_flag >= 3) {
                LOG_STRM_DEBUG("#%d ret_port_cache off\n", rtsp->index);
                ret_port_cache(rtsp->ret_handle, 0);
            }
            rtsp->cache = CACHE_STATE_OFF;
        }
        rtsp_op_immediate_reg(rtsp, RTSP_OP_FAST);

    } else {
        rtsp->timeshift_status = 1;
        if (rtsp->state == STRM_STATE_IPTV
            && ((rtsp->iptvtype != IPTV_TYPE_UNICAST && rtsp->sock == -1)
            ||  (rtsp->iptvtype == IPTV_TYPE_UNICAST && rtsp->timeshift_second == 1 && rtsp->timeshift_support == 0))) {

            rtsp->servtime_diff = 0;
            rtsp_clt_time_set_current(rtsp, mid_time( ));

            rtsp->retry_op_off = mid_time( );

            rtsp->retry_op_scale = scale;
            rtsp->retry_op = RTSP_OP_FAST;
            rtsp->retry_flg = 1;
            rtsp_op_immediate_reg(rtsp, RTSP_OP_TIMESHIFT);
        } else {
            rtsp_op_immediate_reg(rtsp, RTSP_OP_FAST);
        }
    }

    return 0;
Err:
    rtsp_post_state(rtsp, rtsp->state, rtsp->scale);
    return -1;
}

static int rtsp_stop(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    ASSESS_PLAY_NORMAL( );

    switch(rtsp->state) {
    case STRM_STATE_PLAY:   break;
    case STRM_STATE_PAUSE:  break;
    case STRM_STATE_FAST:   break;
    default:                LOG_STRM_ERROUT("#%d %d\n", rtsp->index, rtsp->state);
    }

    if (rtsp->apptype == APP_TYPE_VOD)
        LOG_STRM_ERROUT("#%d apptype = %d\n", rtsp->index, rtsp->apptype);

    LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", rtsp->index);
    rtsp_post_msg(rtsp, STRM_MSG_STREAM_END, 0);
    if (rtsp->state == STRM_STATE_IPTV)
        return 0;

    rtsp->timeshift_status = 0;

    if (rtsp->apptype == APP_TYPE_IPTV && rtsp->iptvtype == IPTV_TYPE_UNICAST && rtsp->timeshift_second == 1 && rtsp->timeshift_support == 0) {
        rtsp->retry_op = RTSP_OP_STOP;
        rtsp->retry_flg = 1;
        rtsp_op_immediate_reg(rtsp, RTSP_OP_TIMESHIFT);
        return 0;
    }

    rtsp_op_immediate_reg(rtsp, RTSP_OP_STOP);

    return 0;
Err:
    rtsp_post_state(rtsp, rtsp->state, rtsp->scale);
    return -1;
}

static int rtsp_advertised(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d\n", rtsp->index);

    ASSESS_PLAY_NORMAL( );

    if (rtsp->state != STRM_STATE_ADVERTISE)
        LOG_STRM_ERROUT("#%d %d\n", rtsp->index, rtsp->state);
    if (rtsp->apptype != APP_TYPE_VOD)
        LOG_STRM_ERROUT("#%d apptype = %d\n", rtsp->index, rtsp->apptype);

    strm_play_open(rtsp->strm_play, rtsp->index, rtsp, rtsp_msg_back, APP_TYPE_VOD, RTP_BUFFER_LENGTH);
    strm_play_set_timeout(rtsp->strm_play);
    if (rtsp->stat.call_statflow)
        strm_play_set_flow(rtsp->strm_play);

    LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", rtsp->index);
    rtsp_clt_state(rtsp, STRM_STATE_PLAY, 1);

    if (rtsp->adv_insert == -2) {
        strm_play_set_idle(rtsp->strm_play, rtsp->index, 1);
        rtsp_post_msg(rtsp, STRM_MSG_STREAM_END, 1);//存在片尾广告
        return 0;
    }
    if (rtsp->adv_insert == -1) {
        rtsp_clt_advertise_select(rtsp, 0, 0);
    } else {
        rtsp_clt_advertise_select(rtsp, rtsp->adv_insert, rtsp->time_current);
    }

    rtsp->op_off = rtsp->time_current;
    rtsp_op_immediate_reg(rtsp, RTSP_OP_SEEK);

    return 0;
Err:
    return -1;
}

static int rtsp_trickmode(struct RTSP* rtsp, int cmd, int arg)
{
    if (rtsp->open_play == OPEN_PLAY_CLOSE || rtsp->trickmode == 0)
        LOG_STRM_ERROUT("#%d open_play = %d trickmode = %d\n", rtsp->index, rtsp->open_play, rtsp->trickmode);

#ifdef INCLUDE_PVR
    if (rtsp->apptype == APP_TYPE_TSTV) {
        if (cmd == STREAM_CMD_SEEK && arg == rtsp->time_current && rtsp->pvr_state == STRM_STATE_PAUSE) {
            LOG_STRM_PRINTF("#%d STREAM_CMD_SEEK > STREAM_CMD_RESUME\n", rtsp->index);
            cmd = STREAM_CMD_RESUME;
        }
        switch(cmd) {
        case STREAM_CMD_PAUSE:  tstv_pause(rtsp);       break;
        case STREAM_CMD_FAST:   tstv_fast(rtsp, arg);   break;
        case STREAM_CMD_RESUME: tstv_resume(rtsp);      break;
        case STREAM_CMD_SEEK:   tstv_seek(rtsp, arg);   break;
        case STREAM_CMD_STOP:   tstv_stop(rtsp);        break;
        default:                                        break;
        }
    } else
#endif//INCLUDE_PVR
    {
        if (rtsp->rec_mix || rtsp->open_shift || rtsp->open_play == OPEN_PLAY_PPVWAIT)
            LOG_STRM_ERROUT("#%d open_play = %d rec_mix = %p\n", rtsp->index, rtsp->open_play, rtsp->rec_mix);

        if (rtsp->trickmode == 0)
            LOG_STRM_ERROUT("#%d trickmode is zero! apptype = %d, iptvtype = %d\n", rtsp->index, rtsp->apptype, rtsp->iptvtype);

        if (cmd == STREAM_CMD_SEEK && arg == (int)rtsp->time_current && rtsp->state == STRM_STATE_PAUSE) {
            LOG_STRM_PRINTF("#%d STREAM_CMD_SEEK > STREAM_CMD_RESUME\n", rtsp->index);
            cmd = STREAM_CMD_RESUME;
        }

        switch(cmd) {
        case STREAM_CMD_PAUSE:
            rtsp_pause(rtsp);
            break;
        case STREAM_CMD_FAST:
            rtsp_fast(rtsp, arg);
            if (arg < 0)
                rtsp->seek_end = SEEK_END_NONE;
            break;
        case STREAM_CMD_RESUME:
            rtsp_resume(rtsp);
            break;
        case STREAM_CMD_SEEK:
            rtsp->seek_end = SEEK_END_NONE;
            rtsp_seek(rtsp, arg);
            break;
        case STREAM_CMD_STOP:
            if (rtsp->apptype == APP_TYPE_VOD) {
                int seek = rtsp->time_length;
                LOG_STRM_PRINTF("#%d STREAM_CMD_STOP > STREAM_CMD_SEEK\n", rtsp->index);
                if (seek > 3)
                    seek -= 3;
                rtsp_seek(rtsp, seek);
            } else {
                rtsp_stop(rtsp);
            }
            break;
        default:
            break;
        }
    }

    return 0;
Err:
    return -1;
}

static void rtsp_cmd(void *handle, StreamCmd* strmCmd)
{
    int cmd;
    struct RTSP *rtsp = (struct RTSP *)handle;

    if (!rtsp || !strmCmd)
        LOG_STRM_ERROUT("handle = %p, msg = %p\n", handle, strmCmd);

    cmd = strmCmd->cmd;
    LOG_STRM_DEBUG("#%d %d %d\n", rtsp->index, rtsp->state, cmd);

    switch(cmd) {
    case STREAM_CMD_OPEN:
        LOG_STRM_PRINTF("#%d STREAM_CMD_OPEN\n", rtsp->index);
        if (rtsp->open_play)
            LOG_STRM_ERROUT("#%d open_play = %d\n", rtsp->index, rtsp->open_play);
        {
            PlayArg *arg;
            char *argbuf;

            if (stream_back_get_arg(rtsp->index, strmCmd->arg0, &arg, &argbuf))
                LOG_STRM_ERROUT("#%d open_play = %d\n", rtsp->index, rtsp->open_play);
            strm_play_leader_set(rtsp->strm_play, rtsp->index);
            rtsp_open_play(rtsp, arg, (RtspArg *)argbuf);
        }
        break;
#ifdef INCLUDE_PVR
    case STREAM_CMD_RECORD_OPEN:
        LOG_STRM_PRINTF("#%d STREAM_CMD_RECORD_OPEN rec_mix = %p, open_play = %d\n", rtsp->index, rtsp->rec_mix, rtsp->open_play);
        {
            RecordArg *arg;
            char *argbuf;
            if (record_back_get_arg(rtsp->index, strmCmd->arg0, &arg, &argbuf))
                LOG_STRM_ERROUT("#%d stream_back_get_arg\n", rtsp->index);
            if (rtsp_open_record(rtsp, arg, (RtspArg*)argbuf))
                LOG_STRM_ERROUT("#%d rtsp_open_record\n", rtsp->index);
        }
        break;

    case STREAM_CMD_WAIT_RECORD:
        LOG_STRM_PRINTF("#%d STREAM_CMD_WAIT_RECORD\n", rtsp->index);
        {
            RecordMix_t rec_mix;
            uint32_t id = (uint32_t)strmCmd->arg0;

            rec_mix = rtsp->rec_mix;
            while (rec_mix) {
                if (id == rec_mix->id)
                    break;
                rec_mix = rec_mix->next;
            }
            if (!rec_mix) {
                LOG_STRM_PRINTF("#%d rec_mix = %p, id = 0x%08x\n", rtsp->index, rtsp->rec_mix, (uint32_t)strmCmd->arg0);
                stream_back_wake(0);
                break;
            }
            if (rtsp->psi_view_record == 1) {
                stream_back_wake(id);
                break;
            }
            rtsp->rec_wait = 1;
            rtsp->rec_wait_id = id;
        }
        break;

    case STREAM_CMD_RECORD_END:
        LOG_STRM_PRINTF("#%d STREAM_CMD_RECORD_END 0x%08x %u\n", rtsp->index, (uint32_t)strmCmd->arg0, (uint32_t)strmCmd->arg1);
        rtsp_record_end(rtsp, (uint32_t)strmCmd->arg0, (uint32_t)strmCmd->arg1);
        break;

    case STREAM_CMD_TIMESHIFT_OPEN:
        LOG_STRM_PRINTF("#%d STREAM_CMD_TIMESHIFT_OPEN\n", rtsp->index);
        tstv_open_timeshift(rtsp);
        break;

    case STREAM_CMD_TIMESHIFT_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_TIMESHIFT_CLOSE\n", rtsp->index);
        tstv_close_timeshift(rtsp);
        break;
#endif

    case STREAM_CMD_RESUME:
    case STREAM_CMD_PAUSE:
    case STREAM_CMD_FAST:
    case STREAM_CMD_SEEK:
    case STREAM_CMD_STOP:
        rtsp->cmdsn = strmCmd->arg3;

        if (rtsp_trickmode(rtsp, cmd, strmCmd->arg0)) {
            rtsp_post_msg(rtsp, STRM_MSG_UNSUPPORT_OP, 0);
            LOG_STRM_ERROUT("#%d trickmode = %d\n", rtsp->index, rtsp->trickmode);
        }
        break;
    case STREAM_CMD_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE clear = %d, open_play = %d, rec_mix = %p\n", rtsp->index, strmCmd->arg0, rtsp->open_play, rtsp->rec_mix);
        if (0 == rtsp->open_play)
            LOG_STRM_ERROUT("#%d play closed!\n", rtsp->index);
        rtsp_close_play(rtsp, strmCmd->arg0);
        if (rtsp->index <= STREAM_INDEX_PIP && (APP_TYPE_IPTV == rtsp->apptype || APP_TYPE_TSTV == rtsp->apptype)) {
            int idx = 0;
            if (STREAM_INDEX_PIP == rtsp->index)
                idx = 1;
            if (strmCmd->arg0 == 0) {//clear
                g_channelZap[idx].clk = mid_10ms( );
                g_channelZap[idx].magic = rtsp->ply_magic;
            } else {
                g_channelZap[idx].clk = 0;
                g_channelZap[idx].magic = 0;
            }
        }
        break;

#ifdef INCLUDE_PVR
    case STREAM_CMD_RECORD_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_RECORD_CLOSE open_play = %d, rec_mix = %p\n", rtsp->index, rtsp->open_play, rtsp->rec_mix);
        if (!rtsp->rec_mix)
            LOG_STRM_ERROUT("#%d record closed!\n", rtsp->index);
        if (strmCmd->arg0 != -1)
            strmCmd->arg0 = 0;
        rtsp_close_record(rtsp, (uint32_t)strmCmd->arg1, strmCmd->arg0);
        break;
#endif

    case STREAM_CMD_ADVERTISE:
        if (rtsp->state != STRM_STATE_ADVERTISE)
            rtsp_op_immediate_reg(rtsp, RTSP_OP_ADVERTISE);
        break;
    case STREAM_CMD_ADVERTISED:
        LOG_STRM_PRINTF("#%d STRM_MSG_ADVERTISE_END adv_insert - %d\n", rtsp->index, rtsp->adv_insert);
        rtsp_post_msg(rtsp, STRM_MSG_ADVERTISE_END, rtsp->adv_insert);
        strm_play_leader_set(rtsp->strm_play, rtsp->index);
        rtsp_advertised(rtsp);
        break;

    case STREAM_CMD_INTERNAL:
        LOG_STRM_PRINTF("#%d STREAM_CMD_INTERNAL arg = %d\n", rtsp->index, strmCmd->arg0);
        switch(strmCmd->arg0) {
        case RTSP_CMD_HEARTBIT:
        case RTSP_CMD_RTSP_HEARTBIT:
            if (RTSP_CMD_HEARTBIT == strmCmd->arg0 && rtsp->ply_magic != (uint32_t)(strmCmd->arg3))
                break;
            rtsp_op_immediate_reg(rtsp, RTSP_OP_HEARTBIT);
            break;
        case RTSP_CMD_RTSP_NATHEARTBIT:
            rtsp_op_immediate_reg(rtsp, RTSP_OP_KEEPLIVE);
            break;
        case RTSP_CMD_RTSP_BURST_UP:
            LOG_STRM_PRINTF("#%d RTSP_CMD_RTSP_BURST_UP\n", rtsp->index);
            rtsp_op_immediate_reg(rtsp, RTSP_OP_BURST_UP);
            break;
        case RTSP_CMD_RTSP_BURST_DOWN:
            LOG_STRM_PRINTF("#%d RTSP_CMD_RTSP_BURST_DOWN\n", rtsp->index);
            rtsp_op_immediate_reg(rtsp, RTSP_OP_BURST_DOWN);
            break;
        case RTSP_CMD_RTSP_ARQ:
            LOG_STRM_PRINTF("#%d RTSP_CMD_RTSP_ARQ\n", rtsp->index);
            if (abs(rtsp->scale) > 1)
                break;
            rtsp_op_immediate_reg(rtsp, RTSP_OP_ARQ);
            break;
        default:
            break;
        }
        break;

    default:
        LOG_STRM_ERROUT("#%d Unkown CMD %d\n", rtsp->index, cmd);
    }

    if (rtsp->cmdsn && rtsp->op == RTSP_OP_NONE)
        rtsp_clt_cmdback(rtsp);
    return;
Err:
    if (rtsp && rtsp->cmdsn)
        rtsp_clt_cmdback(rtsp);
    return;
}

static int local_argparse_play(int idx, PlayArg* arg, char* argbuf, const char* url, int shiftlen, int begin, int end)
{
    APP_TYPE apptype = arg->apptype;
    RtspArg *rtsparg = (RtspArg *)argbuf;

    if (g_port_off == 0) {
        g_port_off = mid_time( );
        g_port_off %= PORT_MAX_OFFSET;
        g_port_off &= 0xfffffe;
    }

    rtsparg->igmp_play_call = g_argextArray[idx].play_call;
    rtsparg->igmp_stop_call = g_argextArray[idx].stop_call;

    rtsparg->igmp_size = g_argextArray[idx].igmp_size;
    if (rtsparg->igmp_size > 0)
        IND_MEMCPY(rtsparg->igmp, g_argextArray[idx].igmp, rtsparg->igmp_size);

    g_argextArray[idx].play_call = NULL;
    g_argextArray[idx].stop_call = NULL;
    g_argextArray[idx].igmp_size = 0;

    rtsparg->fcc_type = g_argextArray[idx].fcc_type;
    LOG_STRM_DEBUG("#%d fcc_type = %d\n", idx, rtsparg->fcc_type);
    g_argextArray[idx].fcc_type = 0;

    if (apptype == APP_TYPE_IPTV) {
        IND_STRCPY(rtsparg->url, url);

        if (strncmp(url, "igmp://", 7) == 0) {
            const char *p = strchr(url, '|');
            if (p) {
                LOG_STRM_PRINTF("#%d igmp|rtsp\n", idx);
                IND_STRCPY(rtsparg->tmshift_url, p + 1);
            } else {
                LOG_STRM_PRINTF("#%d igmp\n", idx);
                rtsparg->tmshift_url[0] = 0;
            }
        } else {
            LOG_STRM_PRINTF("#%d rtsp\n", idx);
            IND_STRCPY(rtsparg->tmshift_url, url);
        }
    } else if (apptype == APP_TYPE_IPTV2) {
        struct IPTVUrl *iurl = (struct IPTVUrl *)url;

        IND_STRCPY(rtsparg->url, iurl->channel_url);
        if (strncmp(rtsparg->url, "igmp://", 7) == 0) {
            if (strchr(rtsparg->url, '|'))
                LOG_STRM_PRINTF("#%d igmp|rtsp\n", idx);
            else
                LOG_STRM_PRINTF("#%d igmp\n", idx);
        } else {
            LOG_STRM_PRINTF("#%d rtsp\n", idx);
        }
        IND_STRCPY(rtsparg->tmshift_url, iurl->tmshift_url);
        apptype = APP_TYPE_IPTV;
    } else {
        if (apptype == APP_TYPE_VODADV) {
            int i, num;
            struct Advertise* array;
            struct VODAdv* vodadv = (struct VODAdv*)url;

            num = vodadv->adv_num;
            LOG_STRM_PRINTF("#%d adv_num = %d\n", idx, vodadv->adv_num);

            array = vodadv->adv_array;
            if (num < 0 || num > ADVERTISE_NUM)
                LOG_STRM_ERROUT("#%d adv_num = %d\n", idx, num);

            for (i = 0; i < num; i ++) {
                LOG_STRM_PRINTF("#%d adv_insert = %d\n", idx, array[i].insert);
                memcpy(&rtsparg->adv_array[i], &array[i], sizeof(struct Advertise));
            }
            rtsparg->adv_num = num;
            IND_STRCPY(rtsparg->url, vodadv->url);
            apptype = APP_TYPE_VOD;
        } else {
            rtsparg->adv_num = 0;
            IND_STRCPY(rtsparg->url, url);
            rtsparg->tmshift_url[0] = 0;
        }
    }

    if (idx == 0 && (apptype == APP_TYPE_IPTV || apptype == APP_TYPE_TSTV) && strstr(rtsparg->tmshift_url, "igmp://")) {
        rtsparg->tmshift_url[0] = 0;
        LOG_STRM_WARN("#%d tmshift_url\n", idx);
    }

    rtsparg->shiftlen = shiftlen;
    rtsparg->begin = begin;
    rtsparg->end = end;

    rtsparg->record = 0;

    arg->apptype = apptype;

    return 0;
Err:
    rtsparg->url[0] = 0;//延迟到播放线程报错
    return 0;
}

#ifdef INCLUDE_PVR
static int local_argparse_record(int idx, RecordArg* arg, char *argbuf, const char* url)
{
    int i;
    RtspArg *rtsparg = (RtspArg *)argbuf;

    switch (idx) {
    case 0:     i = STREAM_INDEX_REC0;    break;
    case 1:     i = STREAM_INDEX_REC1;    break;
    default:    LOG_STRM_ERROUT("#%d\n", idx);
    }

    rtsparg->igmp_play_call = g_argextArray[i].play_call;
    rtsparg->igmp_stop_call = g_argextArray[i].stop_call;

    rtsparg->igmp_size = g_argextArray[i].igmp_size;
    if (rtsparg->igmp_size > 0)
        IND_MEMCPY(rtsparg->igmp, g_argextArray[i].igmp, rtsparg->igmp_size);

    g_argextArray[i].play_call = NULL;
    g_argextArray[i].stop_call = NULL;
    g_argextArray[i].igmp_size = 0;

    rtsparg->fcc_type = g_argextArray[i].fcc_type;
    LOG_STRM_PRINTF("#%d fcc_type = %d\n", idx, rtsparg->fcc_type);
    g_argextArray[i].fcc_type = 0;

    IND_STRCPY(rtsparg->url, url);

    rtsparg->record = 1;

    return 0;
Err:
    return -1;
}

static int local_urlcmp(char *argbuf, const char* url, APP_TYPE apptype)
{
    RtspArg *rtsparg = (RtspArg *)argbuf;

    return strcmp(rtsparg->url, url);
}
#endif//INCLUDE_PVR

static void rtsp_loop(struct RTSP* rtsp)
{
    int i, ret, magic;
    uint32_t out, clk, clks;
    fd_set rset;
    struct timeval tv;
    struct RegistFd *rfd;

    rtsp_cmd_empty(rtsp);
    ind_timer_create(rtsp->tlink, rtsp->clk + INTERVAL_CLK_1000MS, INTERVAL_CLK_1000MS, rtsp_1000ms, rtsp);

    while (rtsp->state != STRM_STATE_CLOSE) {
        clk = mid_10ms( );//其他位置可能导致clk无法更新
        rtsp->clk = clk;
        out = ind_timer_clock(rtsp->tlink);
        if (out <= clk) {
            ind_timer_deal(rtsp->tlink, clk);
            continue;
        }
        if (rtsp->op == RTSP_OP_NONE) {
            if (rtsp->cmd_queue) {
                StreamCmd *strmCmd = rtsp_cmd_pump(rtsp);
                if (strmCmd) {
                    rtsp_cmd(rtsp, strmCmd);
                    rtsp_cmd_free(rtsp, strmCmd);
                }
                //LOG_STRM_PRINTF("@2\n");
                continue;
            }
            if (strm_msgq_valid(rtsp->strm_msgq)) {
                StreamMsg msg;
                strm_msgq_print(rtsp->strm_msgq);
                if (strm_msgq_pump(rtsp->strm_msgq, &msg) == 1)
                    rtsp_msg(rtsp, msg.msg, msg.arg);
                continue;
            }
        }

        clks = out - clk;
        tv.tv_sec = 0;
        if (clks >= 99)
            tv.tv_usec = 990000;
        else
            tv.tv_usec = (long)clks * 10000;
        if (rtsp->index == 0)
            tv.tv_usec += 99;

        rset = rtsp->rset;
        if (rtsp->cmd_close == 1) {
            FD_CLR((uint32_t)rtsp->msgfd, &rset);
        } else if (rtsp->index == STREAM_INDEX_ADV && strm_play_leader_get(rtsp->strm_play) != STREAM_INDEX_ADV) {
            //关闭广告播放
            LOG_STRM_PRINTF("#%d leader = %d\n", rtsp->index, strm_play_leader_get(rtsp->strm_play));
            rtsp_cmd_empty(rtsp);
            rtsp_op_none(rtsp);
            rtsp->cmd_close = 1;
            rtsp_close_play(rtsp, 0);
            continue;
        }

        if (rtsp->op == RTSP_OP_CONNECT) {
            fd_set wset;

            FD_ZERO(&wset);
            FD_SET((uint32_t)rtsp->sock, &wset);

            rtsp->readset = rset;
            ret = select(rtsp->maxfd, &rset, &wset,  NULL, &tv);
            if (ret <= 0)
                continue;

            if (FD_ISSET((uint32_t)rtsp->sock, &rset)) {
                LOG_STRM_ERROR("#%d select errno = %d! %s\n", rtsp->index, errno, strerror(errno));
                rtsp_op_failed(rtsp);
                continue;
            }

            if (FD_ISSET((uint32_t)rtsp->sock, &wset)) {
                socklen_t len;
                int error;

                LOG_STRM_DEBUG("#%d select errno = %d! %s\n", rtsp->index, errno, strerror(errno));
                error = -1;
                len = sizeof(error);
                getsockopt(rtsp->sock, SOL_SOCKET, SO_ERROR, (void*)&error, &len);
                LOG_STRM_DEBUG("#%d getsockopt error = %d! %s\n", rtsp->index, error, strerror(error));
                if (error == 0)
                    rtsp_op_succeed(rtsp);
                else
                    rtsp_op_failed(rtsp);
                continue;
            }
        } else {
            if (rtsp->maxfd == 0) {
                LOG_STRM_PRINTF("#%d maxfd is zero\n", rtsp->index);
                rtsp_op_teardown_ok(rtsp);
                break;
            }

            rtsp->readset = rset;
            ret = select(rtsp->maxfd, &rset, NULL,  NULL, &tv);
            if (ret <= 0)
                continue;
        }

        rtsp->clk = mid_10ms( );
        if (rtsp->cmd_close == 0 && FD_ISSET((uint32_t)rtsp->msgfd, &rset)) {
            StreamCmd strmCmd;

            memset(&strmCmd, 0, sizeof(strmCmd));
            mid_msgq_getmsg(rtsp->msgq, (char *)(&strmCmd));

            ret = stream_deal_cmd(rtsp->index, &strmCmd);
            if (ret == 1)
                continue;

            if (rtsp->open_play && !rtsp->rec_mix && strmCmd.cmd == STREAM_CMD_CLOSE) {
                LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE\n", rtsp->index);
                rtsp_cmd_empty(rtsp);
                rtsp_op_none(rtsp);
                rtsp->cmd_close = 1;
                rtsp_cmd(rtsp, &strmCmd);
            } else {
                rtsp_cmd_queue(rtsp, &strmCmd);
            }
            continue;
        }

        if (rtsp->op != RTSP_OP_CONNECT) {
            if (rtsp->op == RTSP_OP_NAT) {
                if (rtsp->data_sock != -1 && FD_ISSET((uint32_t)rtsp->data_sock, &rset)) {
                    rtsp_recv_nat(rtsp);
                    continue;
                }
            }

            if (rtsp->sock != -1 && FD_ISSET((uint32_t)rtsp->sock, &rset)) {
                rtsp_recv_tcp(rtsp);//data_sock和mult_sock的值可能发生变化
                continue;
            }
            if (rtsp->data_sock != -1 && FD_ISSET((uint32_t)rtsp->data_sock, &rset))
                rtsp_recv_unicast(rtsp);
        }

        if (rtsp->mult_sock != -1 && FD_ISSET((uint32_t)rtsp->mult_sock, &rset))
            rtsp_recv_multicast(rtsp);

        magic = rtsp->regist_magic;
        for (i = 0; i < rtsp->regist_num; i ++) {
            rfd = &rtsp->regist_fd[i];
            if (FD_ISSET((uint32_t)rfd->fd, &rset))
                rfd->recv(rfd->handle, rfd->fd);
            if (magic != rtsp->regist_magic)
                break;
        }
    }
    rtsp_cmd_empty(rtsp);
    ind_timer_delete_all(rtsp->tlink);
#ifdef INCLUDE_PVR
    {
        RecordMix_t rec_mix;
        while (rtsp->rec_mix) {
            rec_mix = rtsp->rec_mix;
            rtsp->rec_mix = rec_mix->next;
            IND_FREE(rec_mix);
        }
    }
#endif
}

static void rtsp_loop_begin(struct RTSP *rtsp, int idx, mid_msgq_t msgq)
{
    rtsp->index = idx;

    rtsp->msgq = msgq;
    rtsp->msgfd = mid_msgq_fd(rtsp->msgq);
    rtsp->tlink = int_stream_tlink(idx);
    rtsp->strm_msgq = int_strm_msgq(idx);
    rtsp->strm_play = int_strm_play(idx);

    rtsp->cmd_close = 0;

    rtsp->tcp_buf = ts_buf_create(TCP_BUFFER_LENGTH);

    rtsp->rtp_sb = strm_buf_malloc(RTP_BUFFER_LENGTH);
    rtsp->strm_bufOrder = strm_bufOrder_create(RTP_BUFFER_LENGTH);

#ifdef INCLUDE_PVR
    rtsp->strm_record = int_strm_record(idx);
    rtsp->pvr_sb = strm_buf_malloc(TS_TIMESHIFT_SIZE);
#endif

    rtsp->clk = mid_10ms( );
}

static void rtsp_loop_end(struct RTSP *rtsp)
{
    ts_buf_delete(rtsp->tcp_buf);

    strm_buf_free(rtsp->rtp_sb);
    strm_bufOrder_delete(rtsp->strm_bufOrder);

#ifdef INCLUDE_PVR
    strm_buf_free(rtsp->pvr_sb);
#endif

    rtsp->open_play = OPEN_PLAY_CLOSE;
    rtsp->open_shift = 0;
}

static void rtsp_loop_play(void *handle, int idx, mid_msgq_t msgq, PlayArg *arg, char *argbuf)
{
    struct RTSP *rtsp = (struct RTSP *)handle;

    if (rtsp == NULL)
        LOG_STRM_ERROUT("rtsp is NULL\n");

    rtsp_loop_begin(rtsp, idx, msgq);

    if (rtsp_open_play(rtsp, arg, (RtspArg *)argbuf))
        LOG_STRM_ERROUT("#%d rtsp_open_play\n", idx);

    rtsp_loop(rtsp);
    LOG_STRM_PRINTF("#%d exit rtsp_loop! cmd_close = %d\n", idx, rtsp->cmd_close);
    if (rtsp->cmd_close == 0)
        int_stream_cmd(STREAM_CMD_ADVERTISED, 0, 0, 0, 0);

Err:
    if (rtsp) {
        if (APP_TYPE_VOD == rtsp->apptype && rtsp->index < STREAM_INDEX_PIP) {
            int_steam_setRemainPlaytime(0);
            int_steam_setCurBufferSize(0);
            int_steam_setToalBufferSize(0);
        }

        rtsp_loop_end(rtsp);
    }

    return;
}

#ifdef INCLUDE_PVR
static void rtsp_loop_record(void *handle, int idx, mid_msgq_t msgq, RecordArg *arg, char *argbuf)
{
    struct RTSP *rtsp = (struct RTSP *)handle;
    if (rtsp == NULL)
        LOG_STRM_ERROUT("rtsp is NULL\n");

    rtsp_loop_begin(rtsp, idx, msgq);

    if (rtsp_open_record(rtsp, arg, (RtspArg*)argbuf))
        LOG_STRM_ERROUT("#%d rtsp_open_record\n", idx);

    rtsp_loop(rtsp);
    LOG_STRM_PRINTF("#%d exit rtsp_loop!\n", idx);

Err:
    if (rtsp)
        rtsp_loop_end(rtsp);

    return;
}
#endif//INCLUDE_PVR

int rtsp_create_stream(StreamCtrl *ctrl)
{
    int i;
    struct RTSP *rtsp;

    if (g_argextArray == NULL) {
        g_argextArray = (ArgExt *)IND_MALLOC(sizeof(ArgExt) * STREAM_INDEX_NUM);
        if (g_argextArray)
            IND_MEMSET(g_argextArray, 0, sizeof(ArgExt) * STREAM_INDEX_NUM);
    }

    rtsp = (struct RTSP *)IND_MALLOC(sizeof(struct RTSP));
    if (rtsp == NULL)
        LOG_STRM_ERROUT("malloc failed!\n");
    IND_MEMSET(rtsp, 0, sizeof(struct RTSP));

    rtsp->mutex = mid_mutex_create( );

    rtsp_clt_reset(rtsp);
    rtsp_clt_fdset(rtsp);

    rtsp->cmd_pool = NULL;
    for (i = 0; i < CMD_QUEUE_LENGTH; i ++) {
        rtsp->cmd_array[i].next = rtsp->cmd_pool;
        rtsp->cmd_pool = &rtsp->cmd_array[i];
    }
    rtsp->cmd_queue = NULL;

    rtsp_op_init(rtsp);

    rtsp->standard = RTSP_STANDARD_YUXING;
    rtsp->transport = 2;

    ctrl->handle = rtsp;
    if (g_rtsp == NULL)
        g_rtsp = rtsp;

    ctrl->loop_play = rtsp_loop_play;
#ifdef INCLUDE_PVR
    ctrl->loop_record = rtsp_loop_record;
#endif

    ctrl->argsize = sizeof(RtspArg);
    ctrl->argparse_play = local_argparse_play;
#ifdef INCLUDE_PVR
    ctrl->argparse_record = local_argparse_record;
    ctrl->urlcmp = local_urlcmp;
#endif

    return 0;
Err:
    return -1;
}

#ifdef INCLUDE_PVR
static void tstv_state(struct RTSP* rtsp, STRM_STATE state, int scale)
{
    rtsp->pvr_state = state;
    rtsp->pvr_scale = scale;

    rtsp_post_state(rtsp, state, scale);
}

static int tstv_pause(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d state = %d\n", rtsp->index, rtsp->pvr_state);
    if (0 == rtsp->open_shift)
        LOG_STRM_ERROUT("#%d open_shift is zero\n", rtsp->index);

    if (rtsp->pvr && rtsp_clt_tstv_calc(rtsp))
        LOG_STRM_ERROUT("#%d tstv_time_calc\n", rtsp->index);

    rtsp->pvr_smooth_pause = 0;

    switch(rtsp->pvr_state) {
    case STRM_STATE_IPTV:
        rtsp->pvr_smooth = 0;
        if (rtsp->open_shift) {
            strm_record_stamp(rtsp->strm_record);
            rtsp->pvr_smooth = 1;
            rtsp->pvr_smooth_pause = 1;
        }
        break;
    case STRM_STATE_PLAY:
        break;
    case STRM_STATE_FAST:
        rtsp->time_start = rtsp->time_current;
        rtsp_clt_reset_play(rtsp, 0);
        break;
    default:
        LOG_STRM_ERROUT("#%d state = %d\n", rtsp->index, rtsp->pvr_state);
    }

    strm_play_pause(rtsp->strm_play, rtsp->index);
    rtsp->open_play = OPEN_PLAY_TIMESHIFT;
    tstv_state(rtsp, STRM_STATE_PAUSE, 0);

    rtsp->pvr_end = 0;

    return 0;
Err:
    if (rtsp->pvr_state == STRM_STATE_IPTV && rtsp->pvr) {
        ind_pvr_close(rtsp->pvr);
        rtsp->pvr = NULL;
    }
    return -1;
}

static int tstv_resume(struct RTSP* rtsp)
{
    int sec;

    LOG_STRM_PRINTF("#%d state = %d\n", rtsp->index, rtsp->pvr_state);
    if (0 == rtsp->open_shift)
        LOG_STRM_ERROUT("#%d open_shift is zero\n", rtsp->index);

    rtsp->pvr_smooth_pause = 0;

    switch(rtsp->pvr_state) {
    case STRM_STATE_PAUSE:    break;
    case STRM_STATE_FAST:    break;
    default:                LOG_STRM_ERROUT("#%d state = %d\n", rtsp->index, rtsp->pvr_state);
    }

    sec = ind_pvr_get_time(rtsp->shift_id);
    if (sec <= 0) {
        LOG_STRM_WARN("#%d sec = %d\n", rtsp->index, sec);
        strm_play_pause(rtsp->strm_play, rtsp->index);
        rtsp->pvr_smooth_pause = 1;
    } else {
        if (NULL == rtsp->pvr) {
            ind_pvr_open(rtsp->shift_id, &rtsp->pvr);
            if (NULL == rtsp->pvr)
                LOG_STRM_ERROUT("#%d pvr is NULL\n", rtsp->index);
        }

        if (STRM_STATE_FAST == rtsp->pvr_state) {
            rtsp->time_start = rtsp->time_current;
            rtsp_clt_reset_play(rtsp, 1);
        }

        if (ind_pvr_play(rtsp->pvr, -1, 1))
            LOG_STRM_ERROUT("#%d ind_pvr_play\n", rtsp->index);

        strm_play_resume(rtsp->strm_play, rtsp->index, 0);
    }

    rtsp->open_play = OPEN_PLAY_TIMESHIFT;
    tstv_state(rtsp, STRM_STATE_PLAY, 1);

    rtsp->pvr_end = 0;

    return 0;
Err:
    return -1;
}

static int tstv_fast(struct RTSP* rtsp, int scale)
{
    uint32_t now;
    int off;
    struct PVRInfo *info = &rtsp->pvr_info;

    if (rtsp->music_flg == 1)
        LOG_STRM_ERROUT("#%d music_flg is true!\n", rtsp->index);

    info->time_len = 0;
    ind_pvr_get_info(rtsp->shift_id, info);
    LOG_STRM_PRINTF("#%d state = %d, scale = %d, time_len = %d\n", rtsp->index, rtsp->pvr_state, scale, info->time_len);
    if (0 == rtsp->open_shift)
        LOG_STRM_ERROUT("#%d open_shift is zero\n", rtsp->index);

    if (info->time_len < 2) {//处理录制刚开始就时移
        if (scale < 0) {
            LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_BEGIN\n", rtsp->index);
            rtsp_post_msg(rtsp, STRM_MSG_STREAM_BEGIN, 0);
            LOG_STRM_PRINTF("#%d tstv_fast > tstv_resume\n", rtsp->index);
            tstv_resume(rtsp);
        } else {
            LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", rtsp->index);
            rtsp_post_msg(rtsp, STRM_MSG_STREAM_END, 0);
            LOG_STRM_PRINTF("#%d tstv_fast > tstv_stop\n", rtsp->index);
            tstv_stop(rtsp);
        }
        return 0;
    }

    if (NULL == rtsp->pvr) {
        ind_pvr_open(rtsp->shift_id, &rtsp->pvr);
        if (NULL == rtsp->pvr)
            LOG_STRM_ERROUT("#%d pvr is NULL\n", rtsp->index);
    }
    rtsp_clt_tstv_calc(rtsp);

    now = mid_time( );
    off = (int)info->time_len - (int)(now - rtsp->time_current);
    if (off < 0)
        off = 0;
    {//避免画面回退
        LOG_STRM_PRINTF("#%d scale = %d, off = %d, time_len = %d\n", rtsp->index, scale, off, info->time_len);
        if (scale < 0 && off > 0)
            off --;
        if (scale > 0 && off < (int)info->time_len)
            off ++;
        LOG_STRM_PRINTF("#%d off = %d\n", rtsp->index, off);
    }
    if (ind_pvr_play(rtsp->pvr, off, scale))
        LOG_STRM_ERROUT("#%d ind_pvr_play\n", rtsp->index);
    rtsp->time_start = rtsp->time_current;

    rtsp_clt_reset_play(rtsp, 0);
    strm_play_tplay(rtsp->strm_play, rtsp->index, scale);
    rtsp->open_play = OPEN_PLAY_TIMESHIFT;
    tstv_state(rtsp, STRM_STATE_FAST, scale);

    rtsp->pvr_end = 0;
    rtsp->pvr_smooth = 0;

    return 0;
Err:
    if (rtsp->pvr_state == STRM_STATE_IPTV && rtsp->pvr) {
        ind_pvr_close(rtsp->pvr);
        rtsp->pvr = NULL;
    }
    return -1;
}

static int tstv_seek(struct RTSP* rtsp, uint32_t offset)
{
    uint32_t now, begin;
    int off;
    struct PVRInfo    *info = &rtsp->pvr_info;

    info->time_len = 0;
    ind_pvr_get_info(rtsp->shift_id, info);
    LOG_STRM_PRINTF("#%d state = %d, offset = %d, time_len = %d\n", rtsp->index, rtsp->pvr_state, offset, info->time_len);
    if (0 == rtsp->open_shift)
        LOG_STRM_ERROUT("#%d open_shift is zero\n", rtsp->index);

    if (info->time_len < 2) {//处理录制刚开始就时移
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", rtsp->index);
        rtsp_post_msg(rtsp, STRM_MSG_STREAM_END, 0);
        LOG_STRM_PRINTF("#%d tstv_seek > tstv_stop\n", rtsp->index);
        tstv_stop(rtsp);
        return 0;
    }

    if (NULL == rtsp->pvr) {
        ind_pvr_open(rtsp->shift_id, &rtsp->pvr);
        if (NULL == rtsp->pvr)
            LOG_STRM_ERROUT("#%d pvr is NULL\n", rtsp->index);
    }
    rtsp_clt_tstv_calc(rtsp);

    now = mid_time( );
    begin = rtsp->rec_time;
    if (offset > 0 && offset <= rtsp->timeshift_len) {
        LOG_STRM_WARN("#%d offset = %d\n", rtsp->index, offset);
        offset += begin;
    }
    if (offset < begin)
        offset = begin;
    if (offset > now)
        offset = now;

    off = (int)(info->time_len - (now - offset));
    if (off < 0)
        off = 0;
    if (ind_pvr_play(rtsp->pvr, off, 1))
        LOG_STRM_ERROUT("#%d ind_pvr_play off = %d\n", rtsp->index, off);
    rtsp->time_start = offset;

    rtsp_clt_reset_play(rtsp, 1);
    strm_play_resume(rtsp->strm_play, rtsp->index, 0);
    rtsp->open_play = OPEN_PLAY_TIMESHIFT;
    tstv_state(rtsp, STRM_STATE_PLAY, 1);

    rtsp->pvr_end = 0;
    rtsp->pvr_smooth = 0;

    return 0;
Err:
    if (rtsp->pvr_state == STRM_STATE_IPTV && rtsp->pvr) {
        ind_pvr_close(rtsp->pvr);
        rtsp->pvr = NULL;
    }
    return -1;
}

static int tstv_stop(struct RTSP* rtsp)
{
    LOG_STRM_PRINTF("#%d state = %d\n", rtsp->index, rtsp->pvr_state);

    if (rtsp->pvr) {
        ind_pvr_close(rtsp->pvr);
        rtsp->pvr = NULL;
    }

    if (rtsp->pvr_state == STRM_STATE_IPTV)
        return 0;

    rtsp->open_play = OPEN_PLAY_RUNNING;
    tstv_state(rtsp, STRM_STATE_IPTV, 1);
    rtsp_clt_tstv_calc(rtsp);
    rtsp_clt_reset_play(rtsp, 1);
    strm_play_resume(rtsp->strm_play, rtsp->index, 1);

    return 0;
}

static int tstv_record(struct RTSP* rtsp)
{
    RecordArg arg;

    memset(&arg, 0, sizeof(arg));
    arg.pvrarg.id = rtsp->shift_id;
    arg.pvrarg.realtime = 1;
    arg.pvrarg.time_shift = rtsp->timeshift_len;
    if (rtsp->rec_mix)
        strm_record_mix_open(rtsp->strm_record, &arg);
    else
        strm_record_open(rtsp->strm_record, rtsp, rtsp_msg_back, &arg);

    rtsp->open_shift = 1;

    return 0;
}

static int tstv_open_timeshift(struct RTSP* rtsp)
{
    int size;

    if (rtsp->apptype != APP_TYPE_TSTV
        || rtsp->open_shift
        || rtsp->open_play != OPEN_PLAY_RUNNING
        || rtsp->trickmode != 1) {
        LOG_STRM_ERROR("#%d open_shift = %d open_play = %d, trickmode = %d\n", rtsp->index, rtsp->open_shift, rtsp->open_play, rtsp->trickmode);
        LOG_STRM_PRINTF("#%d RECORD_MSG_PVR_CONFLICT\n", rtsp->index);
        rtsp_post_msg(rtsp, RECORD_MSG_PVR_CONFLICT, 0);
        return -1;
    }

    size = strm_timeshift_size( );
    if (size < 0) {
        LOG_STRM_ERROR("#%d RECORD_MSG_DISK_ERROR\n", rtsp->index);
        rtsp_post_msg(rtsp, RECORD_MSG_DISK_ERROR, 0);
        return -1;
    }
    if (size == 0) {
        LOG_STRM_PRINTF("#%d RECORD_MSG_NOT_ENOUGH\n", rtsp->index);
        rtsp_post_msg(rtsp, RECORD_MSG_NOT_ENOUGH, 0);
        return -1;
    }

    tstv_record(rtsp);
    rtsp->rec_time = 0;

    return 0;
}

static int tstv_close_timeshift(struct RTSP* rtsp)
{
    if (rtsp->open_shift != 1)
        LOG_STRM_ERROUT("#%d open_shift = %d\n", rtsp->index, rtsp->open_shift);

    if (rtsp->open_play) {
        if (rtsp->pvr_state == STRM_STATE_PLAY || rtsp->pvr_state == STRM_STATE_PAUSE || rtsp->pvr_state == STRM_STATE_FAST) {
            LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", rtsp->index);
            rtsp_post_msg(rtsp, STRM_MSG_STREAM_END, 0);
            tstv_stop(rtsp);
        }

        if (rtsp->rec_mix)
            strm_record_mix_close(rtsp->strm_record, rtsp->shift_id, 0);
        else
            strm_record_close(rtsp->strm_record, 0);
        rtsp->open_shift = 0;
    } else {
        if (rtsp->rec_mix)
            strm_record_mix_close(rtsp->strm_record, rtsp->shift_id, 0);
        else
            rtsp_close(rtsp);
    }

    return 0;
Err:
    return -1;
}
#endif//INCLUDE_PVR

uint32_t rtsp_alloc_port(void)
{
    uint32_t port;
    mid_mutex_t mutex;

    mutex = int_stream_mutex( );
    mid_mutex_lock(mutex);
    port = g_port_off + PORT_BASE_VALUE;
    g_port_off += 2;
    if (g_port_off >= PORT_MAX_OFFSET)
        g_port_off = 0;
    mid_mutex_unlock(mutex);

    return port;
}

void mid_stream_set_sqm(int sqm_flag)
{
    LOG_STRM_PRINTF("sqm_flag = %d\n", sqm_flag);
    g_rtsp_sqm_flag = sqm_flag;
}

void mid_stream_set_arq(int flag)
{
    LOG_STRM_PRINTF("ARQ flag = %d\n", flag);
    g_arq_flag = flag;
}

void mid_stream_set_fcc(int idx, int fcc_type)
{
    int i;

    LOG_STRM_PRINTF("#%d fcc fcc_type = %d\n", idx, fcc_type);

    switch (idx) {
    case 0:                 i = STREAM_INDEX_PLAY;  break;
    case 1:                 i = STREAM_INDEX_PIP;   break;
    case STREAM_INDEX_ADV:  i = STREAM_INDEX_ADV;   break;
    default:                LOG_STRM_ERROUT("#%d\n", idx);
    }
    g_argextArray[i].fcc_type = fcc_type;

Err:
    return;
}

#ifdef INCLUDE_PVR
void mid_record_set_fcc(int idx, int fcc_type)
{
    int i;

    LOG_STRM_PRINTF("#%d fcc fcc_type = %d\n", idx, fcc_type);

    switch (idx) {
    case 0:     i = STREAM_INDEX_REC0;  break;
    case 1:     i = STREAM_INDEX_REC1;  break;
    default:    LOG_STRM_ERROUT("#%d\n", idx);
    }
    g_argextArray[i].fcc_type = fcc_type;

Err:
    return;
}
#endif

void mid_stream_set_burst(int flag)
{
    LOG_STRM_PRINTF("Burst flag  = %d\n", flag);
    g_burst_flag = flag;
}

void mid_stream_set_pushsync(int flag)
{
    LOG_STRM_PRINTF("flag = %d\n", flag);
    g_pushsync = flag;
}

void mid_stream_recv_safe(int safe)
{
    LOG_STRM_PRINTF("safe = %d\n", safe);

    g_recv_safe = safe;
}

void mid_stream_set_size(int playSize, int cacheSize)
{
    if (playSize < STREAM_BLOCK_LEVEL_PLAY || cacheSize < STREAM_BLOCK_LEVEL_CACHE)
        LOG_STRM_ERROUT("playSize = %d, cacheSize = %d\n", playSize, cacheSize);
    LOG_STRM_PRINTF("playSize = %d, cacheSize = %d\n", playSize, cacheSize);

    g_size_play = playSize;
    g_size_cache = cacheSize;

Err:
    return;
}

void mid_stream_cache(int on, int off)
{
    LOG_STRM_PRINTF("cache on = %d, off = %d\n", on, off);

    if (on < 0) {
        g_cache_clk_level = 0;
        g_cache_clk_range = 0;
    } else if (on == 0) {
        g_cache_clk_level = CACHE_CLK_LEVEL;
        g_cache_clk_range = CACHE_CLK_RANGE;
    } else {
        on /= 10;
        off /= 10;
        if (off <= on)
            LOG_STRM_ERROUT("on = %d, off = %d\n", on, off);
        g_cache_clk_level = on;
        g_cache_clk_range = off - on;
    }

Err:
    return;
}

void mid_stream_transport(int trasnport)
{
    LOG_STRM_PRINTF("transport = %d\n", trasnport);
    g_transport = trasnport;
    int_steam_setTransportProtocol(0, trasnport);
    int_steam_setTransportProtocol(1, trasnport);
}

void mid_stream_skipfast(int ms)
{
    LOG_STRM_PRINTF("skipfast = %d\n", ms);
    g_skipfast_clks = ms / 10;
}

void mid_stream_port(int idx, int port_tcp, int port_udp)
{
    LOG_STRM_PRINTF("#%d port_tcp = %d port_udp = %d\n", idx, port_tcp, port_udp);

    if (port_tcp < 0 || port_tcp > 0xffff || port_udp < 0 || port_udp > 0xffff)
        LOG_STRM_ERROUT("#%d arg0 = %d, arg1 = %d\n", idx, port_tcp, port_udp);

    g_port_tcp = port_tcp;
    g_port_udp = port_udp;

Err:
    return;
}

void mid_stream_iframe(int flag)
{
    LOG_STRM_PRINTF("Iframe flag  = %d\n", flag);

    g_iframe_flag = flag;
}

void mid_stream_standard(int standard)
{
    LOG_STRM_PRINTF("standard = %d\n", standard);

    switch(standard) {
    case RTSP_STANDARD_CTC_SHANGHAI:
    case RTSP_STANDARD_CTC_GUANGDONG:
    case RTSP_STANDARD_HUAWEI:
    case RTSP_STANDARD_UNICOM:
        break;
    default:
        LOG_STRM_ERROUT("standard = %d\n", standard);
    }

    g_standard = standard;

Err:
    return;
}

void mid_stream_timeshift_second(int enable)
{
    LOG_STRM_PRINTF("enable = %d\n", enable);

    g_timeshift_second = enable;
}

void mid_stream_multicast_unicast(int enable)
{
    LOG_STRM_PRINTF("enable = %d\n", enable);

    g_multicast_unicast = enable;
}

void mid_stream_multicast_forbid(int forbid)
{
    LOG_STRM_PRINTF("forbid = %d\n", forbid);

    g_multicast_forbid = forbid;
}

void mid_stream_heartbit_period(int interval)
{
    LOG_STRM_PRINTF("interval = %d\n", interval);

    g_heartbit_period = interval;
}

void mid_stream_heartbit_standard(int standard)
{
    LOG_STRM_PRINTF("standard = %d\n", standard);

    g_heartbit_standard = standard;
}

void mid_stream_set_igmp(int idx, MultiPlayCall play_call, MultiStopCall stop_call, void* igmp, int igmp_size)
{
    int i;

    if (igmp_size < 0 || igmp_size > IGMP_INFO_SIZE)
        LOG_STRM_ERROR("#%d igmp_size = %d\n", idx, igmp_size);

    LOG_STRM_PRINTF("#%d play_call = %p, stop_call = %p, igmp_size = %d\n", idx, play_call, stop_call, igmp_size);
    switch (idx) {
    case 0:                 i = STREAM_INDEX_PLAY;  break;
    case 1:                 i = STREAM_INDEX_PIP;   break;
    case STREAM_INDEX_ADV:  i = STREAM_INDEX_ADV;   break;
    default:                LOG_STRM_ERROUT("#%d\n", idx);
    }
    g_argextArray[i].play_call = play_call;
    g_argextArray[i].stop_call = stop_call;

    g_argextArray[i].igmp_size = igmp_size;
    if (igmp_size > 0)
        IND_MEMCPY(g_argextArray[i].igmp, igmp, igmp_size);

Err:
    return;
}

#ifdef INCLUDE_PVR
void mid_record_set_igmp(int idx, MultiPlayCall play_call, MultiStopCall stop_call, void* igmp, int igmp_size)
{
    int i;

    if (igmp_size < 0 || igmp_size > IGMP_INFO_SIZE)
        LOG_STRM_ERROR("#%d igmp_size = %d\n", idx, igmp_size);

    LOG_STRM_PRINTF("#%d play_call = %p, stop_call = %p, igmp_size = %d\n", idx, play_call, stop_call, igmp_size);
    switch (idx) {
    case 0:      i = STREAM_INDEX_REC0; break;
    case 1:     i = STREAM_INDEX_REC1;  break;
    default:    LOG_STRM_ERROUT("#%d\n", idx);
    }

    g_argextArray[i].play_call = play_call;
    g_argextArray[i].stop_call = stop_call;

    g_argextArray[i].igmp_size = igmp_size;
    if (igmp_size > 0)
        IND_MEMCPY(g_argextArray[i].igmp, igmp, igmp_size);

Err:
    return;
}
#endif//INCLUDE_PVR

void mid_stream_heartbit_active(void)
{
    uint32_t magic = int_stream_playmagic( );
    int_stream_cmd(STREAM_CMD_INTERNAL, RTSP_CMD_HEARTBIT, 0, 0, (int)magic);
}

void mid_stream_nat(int mode)
{
    LOG_STRM_PRINTF("nat_flag = %d\n", mode);

    if(mode >= 0 && mode <= 1) {
        g_nat_flag = mode;
    } else {
        g_nat_flag = 0;
    }
}

void mid_stream_nat_heartbitperiod(int sec)
{
    extern void yx_FCC_NatInterval_set (int Interval_time);

    LOG_STRM_PRINTF("nat_keepalive = %d\n", sec);

    if(sec < 0)
        return;
    g_nat_heartbitperiod = sec;
#ifdef INCLUDE_SQA
    yx_FCC_NatInterval_set(sec);
#endif
}

void mid_stream_vodlevel(int ms)
{
    LOG_STRM_PRINTF("vodlevel = %d\n", ms);
    g_vodlevel = ms / 10;
}

void mid_stream_callback_rtspinfo(CallBack_RtspInfo rtspinfo)
{
    LOG_STRM_PRINTF("rtspinfo = %p\n", rtspinfo);
    g_call_rtspinfo = rtspinfo;
}

void mid_stream_set_useragent(char *useragent)
{
    if (!useragent)
        LOG_STRM_ERROUT("useragent is NULL\n");
    if (strlen(useragent) >= USERAGENT_LENGTH)
        LOG_STRM_ERROUT("useragent too large\n");
    if (g_useragent)
        IND_FREE(g_useragent);
    g_useragent = IND_STRDUP(useragent);

Err:
    return;
}
