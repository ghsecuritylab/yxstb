
/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#include "unistd.h"

#include "stream.h"
#include "stream_port.h"
#include "config/pathConfig.h"


enum {
    PLAY_STATE_CLOSE = 0,
    PLAY_STATE_PLAY,
    PLAY_STATE_TPLAY,
    PLAY_STATE_PAUSE
};

//#define ENABLE_SAVE_PLAY

#define INTERVAL_CLK_CA_WAIT    100
#define INTERVAL_CLK_CA_PTS     5
#define ACCELERATED_FACTOR      0.2
#define DECODE_DIFF_CLKS        150

static int g_ca_wait = INTERVAL_CLK_CA_WAIT;
static int g_pcr_flag = 0;

static int g_vodsync_clks = 0;
static int g_voddelay_clks = 0;
static int g_iptvdelay_clks = 0;
static int g_decodediff_clks = DECODE_DIFF_CLKS;//解码器内缓冲数据最小量

//MPEG2-TS statistics “IPTV STB V100R002C20 STB与EPG接口规范－MT定制 .doc” 3.1.49-3
typedef struct tagTSContinuity TSContinuity;
struct tagTSContinuity {
    TSContinuity *next;
    TSContinuity *prev;

    uint16_t pid;
    uint16_t counter;
    uint32_t times;//出现次数
};

static unsigned int gTSSyncClock = 0;
static unsigned int gTSSyncInterval = 0;
static TSSyncNotifyCall gTSSyncNotifyCall = 0;

//流统计
struct _ByteStat {
    uint32_t clk_begin;
    uint32_t clk_end;

    int bytes;
};

typedef struct _ByteStat ByteStat;

#define STAT_ARRAY_SIZE        5

#define EXTRA_ARRAY_SIZE       10

struct tagPlayPush {
    long long   tsPacketCount;

    int         firstPcrOff;
    uint32_t    firstPcrValue;
    int         secondPcrOff;
    uint32_t    secondPcrValue;

    int         pushPcrNum;
    uint32_t    pushPcrValue;

    int         forecast;
    int         adjustment;

    double      tsPacketDuration;

    int         extraNum;
    int         extraPcrNum;
    uint32_t    extraPcrValue[EXTRA_ARRAY_SIZE];

    double      baseClock;
    double      firstClock;

    double      lastClock;

    double      pauseTime;
    double      firstRealTime;

    double      transmitDuration;
};
typedef struct tagPlayPush    PlayPush;

struct _StreamPlay {
    int         codec;

    int         leader;
    int         enable;

    uint32_t    clk;

    APP_TYPE    apptype;

    mid_mutex_t loop_mutex;
    mid_mutex_t data_mutex;

    void*       msg_handle;
    MsgCallBack msg_callback;

#ifdef ENABLE_SAVE_PLAY
    FILE*       save_fp;
    long long   save_len;
#endif

    ByteStat    stat_array[STAT_ARRAY_SIZE];
    int         stat_byterate;

    uint32_t    stat_clk;
    int         stat_time;
    int         stat_numArray[TS_AUDIO_NUM];

    //OTT播放判断播放缓冲下溢
    int         underflow_flag;
    double      underflow_time0;
    double      underflow_duration0;
    double      underflow_time1;
    double      underflow_duration1;

    int         data_flg;
    uint32_t    data_clk;
    uint32_t    data_pts;

    uint32_t    time_clk;
    int         time_diff;
    int         time_buffer;
    int         time_play;

    uint32_t    push_clk;
    uint32_t    delay_clk;
    uint32_t    delay_dbg;
    uint32_t    tskip_clk;//丢弃切换之间的数据，避免花屏

    int         reset_flg;

    uint32_t    pts_clk;
    uint32_t    pts_clk_base;

    int         state;
    int         scale;

    int         psi_flag;
    int         iptv_flag;
    int         shift_flag;

    int         open_flg;
    int         still_flg;

    int         temp_flg;

    int         flow_flg;
    int         timeout_flg;

    //检测DVBS音视频数据是否下发
    uint32_t    load_clk;
    int         load_valid;
    int         load_check;

    int         pic_chk;
    int         pic_width;

    int         isIdle;

    uint32_t    end_flg;
    uint32_t    end_clk;
    uint32_t    end_pts;

    uint32_t    ply_clk;
    uint32_t    ply_pts;
    uint32_t    ply_spy_pts;
    uint32_t    ply_spy_clk;

    int         bit_base;
    int         bit_last;
    uint32_t    bit_bytes;

    StrmBufPlay* sbp;
    StrmBufPlay* shift_sbp;

    int         sbq_rawsize;

    ts_parse_t  ts_parse;
    ts_pts_t    ts_pts;

    uint32_t    pcr_clk;
    int         pcr_mode;

    ts_pcr_t    ts_pcr;
    uint32_t    ts_pcr_base;
    uint32_t    ts_pcr_pause;
    uint32_t    ts_pcr_debug;

    int         psi_exceps;
    int         crc_exceps;

    struct ts_psi           ps_psi;
    struct ts_dr_subtitle   ps_subtitle;
    struct ts_dr_teletext   ps_teletext;

    struct ts_psi           ts_psi;
    struct ts_dr_subtitle   ts_subtitle;
    struct ts_dr_teletext   ts_teletext;

    struct ts_ca            ca;

    uint32_t    cat_wait_clk;
    uint32_t    ca_wait_clk;

    char        section_buf[TS_SECTION_SIZE];
    int         section_len;

    int         decode_valid;//打印解码器内存空间大小

    int         conax_flag;

    PlayPush    pp;

    double      pp_speedTime;//开始播放一段时间加速发送
    double      pp_emptyTime;

    TSContinuity *tsContinuity;
};

static StreamPlay*  g_sp[2] = {NULL, NULL};

static uint8_t      g_ff[8] = {0x47, 0x1F, 0xFF, 0x30, 0x03, 0x00, 'F', '0'};//fast stuffing

static int g_bytes = 0;

static void int_play_task(void *arg);
static void int_play_error(StreamPlay* sp, int arg);
static void int_play_cls(StreamPlay* sp, int clear);

static void local_ca_close(StreamPlay* sp);
static void local_ca_update(StreamPlay* sp);

static void int_play_end(StreamPlay* sp);

double int_timeNow(void)
{
    double timeNow;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    timeNow = tp.tv_sec;
    timeNow += tp.tv_nsec / 1000000000.0;

    return timeNow;
}

#define TIME_ADJUSTMENT_FACTOR 0.8
#define MAX_PLAYOUT_BUFFER_DURATION 0.2 // (seconds)

static void int_updateDuration(PlayPush* pp, unsigned char* pkt, double timeNow)
{
    uint32_t pcr;
    double tClock, durationPerPacket;

    pp->tsPacketCount++;
    pp->pushPcrNum++;

    pcr = pp->firstPcrValue;
    if (0 == pcr)
        return;

    if (pp->firstPcrOff > 0) {
        pp->firstPcrOff--;
        if (1 == pp->forecast && pp->pushPcrValue && pcr > pp->pushPcrValue && pcr < pp->pushPcrValue + 45000) {
            double duration;

            duration = (pcr - pp->pushPcrValue) / 45000.0 - pp->tsPacketDuration * pp->pushPcrNum;
            if (duration > 0.0) {
                durationPerPacket = duration / (pp->firstPcrOff + 1);

                if (pp->adjustment > 0)
                    pp->tsPacketDuration = durationPerPacket / TIME_ADJUSTMENT_FACTOR;
                else if (pp->adjustment < 0)
                    pp->tsPacketDuration = durationPerPacket * TIME_ADJUSTMENT_FACTOR;
                else
                    pp->tsPacketDuration = durationPerPacket;

                pp->forecast = 0;
            } else {
                pp->transmitDuration += duration;
                pp->tsPacketDuration = 0;
            }
            //printf("++++ %lld: d = %f / %f, o = %d, n = %d\n", pp->tsPacketCount, pp->tsPacketDuration, duration, pp->firstPcrOff, pp->pushPcrNum);
        }
        return;
    }

    tClock = pcr / 45000.0;
    pp->firstPcrValue = 0;

    if (0 == pp->pushPcrValue) {
        pp->pushPcrValue = pcr;
        pp->pushPcrNum = 0;
        pp->forecast = 1;

        pp->firstClock = tClock;
        pp->firstRealTime = timeNow;
    } else {
        double playoutDuration;

        if (pcr <= pp->pushPcrValue || pcr > pp->pushPcrValue + 45000) {
            uint32_t extra;

            if (0 == pp->extraNum) {
                pp->extraPcrValue[0] = pcr;
                pp->extraNum = 1;
                pp->extraPcrNum = pp->pushPcrNum;
                return;
            }

            extra = pp->extraPcrValue[pp->extraNum - 1];
            if (pcr <= extra || pcr > extra + 45000) {
                pp->extraPcrValue[0] = pcr;
                pp->extraNum = 1;
                pp->extraPcrNum = pp->pushPcrNum;
                return;
            }

            if (pp->extraNum < EXTRA_ARRAY_SIZE) {
                pp->extraPcrValue[pp->extraNum] = pcr;
                pp->extraNum ++;
                return;
            }

            LOG_STRM_WARN("clock = %f / %f, extraNum = %d\n", tClock, pp->lastClock, pp->extraNum);

            extra = pp->extraPcrValue[0];
            pp->pushPcrValue = extra;
            pp->pushPcrNum -= pp->extraPcrNum;

            pp->baseClock += pp->lastClock - pp->firstClock;
            pp->lastClock = extra / 45000.0;
            pp->firstClock = pp->lastClock;
        }
        pp->extraNum = 0;

        playoutDuration = tClock - pp->firstClock + pp->baseClock;
        if (pp->transmitDuration > playoutDuration)
            pp->adjustment = -1;
        else if (pp->transmitDuration + MAX_PLAYOUT_BUFFER_DURATION < playoutDuration)
            pp->adjustment = 1;
        else
            pp->adjustment = 0;

        //printf("++++ pcr = %u\n", pp->secondPcrValue - pcr);
        if (pp->secondPcrValue && pp->secondPcrValue > pcr && pp->secondPcrValue < pcr + 45000) {
            durationPerPacket = (double)(pp->secondPcrValue - pcr) / 45000.0 / pp->secondPcrOff;
            //printf("++++ %lld: pcr = %u, off = %d\n", pp->tsPacketCount, pp->secondPcrValue - pcr, pp->secondPcrOff);
            pp->forecast = 0;
        } else {
            durationPerPacket = (pcr - pp->pushPcrValue) / 45000.0 / pp->pushPcrNum;
            //printf("++++ %lld: pcr = %u, num = %d\n", pp->tsPacketCount, pcr - pp->pushPcrValue, pp->pushPcrNum);
            pp->forecast = 1;
        }

        if (pp->adjustment > 0)
            pp->tsPacketDuration = durationPerPacket / TIME_ADJUSTMENT_FACTOR;
        else if (pp->adjustment < 0)
            pp->tsPacketDuration = durationPerPacket * TIME_ADJUSTMENT_FACTOR;
        else
            pp->tsPacketDuration = durationPerPacket;
        //printf("++++ %lld: d = %f, t = %f, p = %f, f = %d, a = %d\n", pp->tsPacketCount, pp->tsPacketDuration, pp->transmitDuration, playoutDuration, pp->forecast, pp->adjustment);

        pp->pushPcrValue = pcr;
        pp->pushPcrNum = 0;
    }
    pp->lastClock = tClock;
}

StreamPlay* strm_play_create(int codec, int size)
{
    StreamPlay *sp;

    if (codec != 0 && codec != 1)
        LOG_STRM_ERROUT("codec = %d\n", codec);

    sp = (StreamPlay *)IND_CALLOC(sizeof(StreamPlay), 1);
    if (NULL == sp)
        LOG_STRM_ERROUT("codec = %d, malloc\n", codec);

    LOG_STRM_PRINTF("size = %d\n", size);
    sp->sbq_rawsize = size;

    sp->loop_mutex = mid_mutex_create( );
    sp->data_mutex = mid_mutex_create( );

    sp->codec = codec;

    g_sp[codec] = sp;

    stream_port_task_create(int_play_task, sp);

    return sp;
Err:
    return NULL;
}

int strm_play_leader_get(StreamPlay* sp)
{
    return sp->leader;
}

void strm_play_leader_set(StreamPlay* sp, int leader)
{
    LOG_STRM_DEBUG("codec = %d, leader = %d\n", sp->codec, leader);
    mid_mutex_lock(sp->loop_mutex);
    sp->leader = leader;
    mid_mutex_unlock(sp->loop_mutex);
}

static void int_stat_reset(StreamPlay* sp)
{
    int i;
    ByteStat *array;

    array = sp->stat_array;
    for (i = 0; i < STAT_ARRAY_SIZE; i ++)
        array[i].clk_begin = 0;

    memset(&sp->stat_array[0], 0, sizeof(ByteStat));
}

static void int_play_reset(StreamPlay* sp)
{
    mid_mutex_lock(sp->data_mutex);
    strm_bufplay_reset(sp->sbp);
    if (sp->shift_sbp)
        strm_bufplay_reset(sp->shift_sbp);

    {
        TSContinuity *tsContinuity, *next;

        tsContinuity = sp->tsContinuity;
        sp->tsContinuity = NULL;
        while (tsContinuity) {
            next = tsContinuity->next;
            IND_FREE(tsContinuity);
            tsContinuity = next;
        }
    }
    mid_mutex_unlock(sp->data_mutex);

    ts_pts_reset(sp->ts_pts, 1);
    ts_parse_reset(sp->ts_parse);

    memset(&sp->pp, 0, sizeof(sp->pp));

    ts_pcr_reset(sp->ts_pcr, 0);
    sp->ts_pcr_base = sp->clk;
    sp->ts_pcr_pause = 0;

    sp->pcr_clk = 0;
    sp->delay_clk = 0;

    int_stat_reset(sp);
    sp->stat_byterate = 0;

    sp->stat_clk = 0;
    sp->stat_time = 0;
    memset(sp->stat_numArray, 0, sizeof(int) * TS_AUDIO_NUM);

    sp->decode_valid = 0;
    sp->reset_flg = 1;

    sp->load_clk = 0;
    sp->load_valid = 0;
    sp->load_check = 0;

    sp->ply_clk = sp->clk;
    sp->ply_pts = 0;
    sp->data_pts = 0;

    sp->isIdle = 0;
    sp->end_flg = 0;

    sp->time_clk = sp->clk;
    sp->time_diff = 0;
    sp->time_buffer = 0;
    sp->time_play = 0;

    sp->ply_spy_pts = 0;
    sp->ply_spy_clk = 0;

    sp->pp_speedTime = 0.0;
    sp->pp_emptyTime = 0.0;
}

static void int_play_clean(StreamPlay* sp);

int strm_play_open(StreamPlay* sp, int leader, void* msg_handle, MsgCallBack msg_callback, APP_TYPE apptype, int size)
{
    int result = -1;

    mid_mutex_lock(sp->loop_mutex);

    if (leader != sp->leader)
        LOG_STRM_ERROUT("leader = %d / %d\n", leader, sp->leader);

    LOG_STRM_PRINTF("#%d ---- rawsize = %d\n", leader, sp->sbq_rawsize);

    if (sp->state != PLAY_STATE_CLOSE)
        codec_close(sp->codec, 0);
    sp->msg_handle = msg_handle;
    sp->msg_callback = msg_callback;

    sp->enable = 1;
    sp->apptype = apptype;

    if (APP_TYPE_IPTV == apptype || APP_TYPE_TSTV == apptype || APP_TYPE_DVBS == apptype)
        sp->iptv_flag = 1;
    else
        sp->iptv_flag = 0;
    codec_open(sp->codec, sp->iptv_flag * g_pcr_flag);
    sp->open_flg = 1;

    sp->clk = mid_10ms( );

    sp->psi_flag = 0;
    sp->shift_flag = 0;

    sp->pic_width = 0;

    sp->scale = 0;

    sp->end_pts = 0;

    sp->pts_clk = 1;
    sp->pts_clk_base = 0;

    sp->push_clk = sp->clk;
    sp->tskip_clk = 0;

    sp->ca.system_id = 0;
    sp->ca_wait_clk = 0;
    sp->cat_wait_clk = 0;

    sp->temp_flg = 0;

    sp->flow_flg = 0;
    sp->timeout_flg = 0;
    sp->underflow_flag = 0;

    memset(&sp->ts_subtitle,    0, sizeof(struct ts_dr_subtitle));
    memset(&sp->ts_teletext,    0, sizeof(struct ts_dr_teletext));

    memset(&sp->ps_subtitle,    0, sizeof(struct ts_dr_subtitle));
    memset(&sp->ps_teletext,    0, sizeof(struct ts_dr_teletext));

    memset(&sp->ts_psi, 0, sizeof(struct ts_psi));
    sp->ts_psi.dr_subtitle = &sp->ts_subtitle;
    sp->ts_psi.dr_teletext = &sp->ts_teletext;

    memset(&sp->ps_psi, 0, sizeof(struct ts_psi));
    sp->ps_psi.dr_subtitle = &sp->ps_subtitle;
    sp->ps_psi.dr_teletext = &sp->ps_teletext;

    sp->ts_psi.emm_num = 0;
    sp->ps_psi.emm_num = 0;

    sp->pcr_mode = 0;

    sp->psi_exceps = 0;
    sp->crc_exceps = 0;

    sp->state = PLAY_STATE_PLAY;
    LOG_STRM_DEBUG("#%d PLAY_STATE_OPEN\n", leader);

    mid_mutex_lock(sp->data_mutex);

    int_play_clean(sp);

    {
        int num;

        if (RTP_BUFFER_LENGTH == size)
            num = sp->sbq_rawsize / 1316;
        else
            num = sp->sbq_rawsize / size;

        sp->sbp = strm_bufplay_create(size, num);
        LOG_STRM_PRINTF("#%d ---- sbp = %p, size = %d, num = %d\n", leader, sp->sbp, size, num);

        if (APP_TYPE_TSTV == apptype || APP_TYPE_DVBS == apptype) {
            num = sp->sbq_rawsize / TS_TIMESHIFT_SIZE;
            sp->shift_sbp = strm_bufplay_create(TS_TIMESHIFT_SIZE, num);
        }
    }
    mid_mutex_unlock(sp->data_mutex);

    sp->ts_parse = ts_parse_create(&sp->ps_psi);
    sp->ts_pts = ts_pts_create( );
    sp->ts_pcr = ts_pcr_create( );

    int_play_reset(sp);

#ifdef ENABLE_SAVE_PLAY
    sp->save_len = 0;
    sp->save_fp = fopen(DEFAULT_EXTERNAL_DATAPATH"/play.ts", "wb");
    LOG_STRM_PRINTF("@@@@@@@@: fp = %p\n", sp->save_fp);
#endif

    sp->data_flg = 1;

    //sp->still_flg = 1; 播放音频时，该值不能置为0

    result = 0;
Err:
    mid_mutex_unlock(sp->loop_mutex);
    return result;
}

static int int_play_byte_rate(StreamPlay* sp)
{
    int i, byterate, bytes;
    int clks;
    ByteStat *stat;

    byterate = 0;

    if (sp->state != PLAY_STATE_PLAY && sp->state != PLAY_STATE_PAUSE)
        goto Err;

    clks = 0;
    bytes = 0;

    for (i = 0; i < STAT_ARRAY_SIZE; i ++) {
        stat = &sp->stat_array[i];
        if (0 == stat->clk_begin)
            break;
        clks += (int)(stat->clk_end - stat->clk_begin);
        bytes += stat->bytes;
    }

    if (clks > 300)
        byterate = bytes * 100 / clks;

Err:
    return byterate;
}

int strm_play_byte_rate(StreamPlay* sp)
{
    int byterate;

    if (!sp)
        sp = g_sp[0];
    mid_mutex_lock(sp->loop_mutex);
    byterate = int_play_byte_rate(sp);
    mid_mutex_unlock(sp->loop_mutex);

    return byterate;
}

int strm_play_byte_percent(StreamPlay* sp, int leader, uint32_t pktpercent)
{
    int last, byterate;

    byterate = 0;
    mid_mutex_lock(sp->loop_mutex);

    if (sp->state != PLAY_STATE_PLAY && sp->state != PLAY_STATE_PAUSE)
        goto Err;

    last = ts_pts_time_last(sp->ts_pts, 1);
    if (sp->stat_clk && sp->stat_clk < sp->clk) {
        float clkpercent;

        clkpercent = (float)(last - sp->stat_time) / (float)(sp->clk - sp->stat_clk);
        byterate = (int)(clkpercent * (float)pktpercent);
    }

    if (sp->psi_flag) {
        int i;
        ts_psi_t psi = &sp->ts_psi;

        if (last > sp->stat_time) {
            long long bitRate;
            for (i = 0; i < psi->audio_num; i++) {
                bitRate = sp->stat_numArray[i];
                sp->stat_numArray[i] = 0;
                bitRate = bitRate * 188 * 8 * 100 / (last - sp->stat_time);
                int_steam_setAudioBitRate(sp->codec, i, (int)bitRate);
            }
        } else {
            for (i = 0; i < psi->audio_num; i++)
                int_steam_setAudioBitRate(sp->codec, i, 0);
        }
    }

    sp->stat_clk = sp->clk;
    sp->stat_time = last;

Err:
    mid_mutex_unlock(sp->loop_mutex);
    return byterate;
}

static void int_play_psi(StreamPlay* sp)
{
    ts_psi_t psi = &sp->ts_psi;

    psi->emm_num = 0;

    sp->ply_pts = 0;

    if (0 == psi->video_pid) {
        sp->pic_width = -1;
        LOG_STRM_DEBUG("#%d STRM_MSG_STREAM_MUSIC\n", sp->leader);
        sp->msg_callback(sp->msg_handle, STRM_MSG_STREAM_MUSIC, 0);
    } else {
        LOG_STRM_DEBUG("#%d STRM_MSG_STREAM_VIDEO\n", sp->leader);
        sp->msg_callback(sp->msg_handle, STRM_MSG_STREAM_VIDEO, 0);
    }

    LOG_STRM_PRINTF("#%d video = %d/%d, audio = %d/%d, pcr = %d\n", sp->leader,
            psi->video_pid, psi->video_type, psi->audio_pid[0], psi->audio_type[0], psi->pcr_pid);

    if (psi->dr_subtitle)
        LOG_STRM_DEBUG("#%d subtitle_num = %d\n", sp->leader, psi->dr_subtitle->subtitle_num);
}

static void int_play_pts(StreamPlay* sp)
{
    if (sp->ply_clk + 50 < sp->clk) {
        codec_pts(sp->codec, &sp->ply_pts);
        sp->ply_clk = sp->clk;
    }
}

static void int_play_resume(StreamPlay* sp)
{
    if (sp->iptv_flag && g_pcr_flag)
        return;
    if (sp->reset_flg) {
        uint32_t delay_clks = 0;
        if (1 == sp->iptv_flag)
            delay_clks = g_iptvdelay_clks;
        else if (APP_TYPE_VOD == sp->apptype || APP_TYPE_IPTV == sp->apptype)
            delay_clks = g_vodsync_clks;
        if (delay_clks) {
            uint32_t clk = sp->clk + delay_clks;
            LOG_STRM_PRINTF("#%d iptv_flag = %d, delay clks = %d\n", sp->leader, sp->iptv_flag, delay_clks);
            if (sp->delay_clk < clk)
                sp->delay_clk = clk;
        }
        sp->reset_flg = 0;
    }
}

static int int_play_first(StreamPlay* sp)
{
    ts_psi_t psi = &sp->ts_psi;

    LOG_STRM_DEBUG("#%d apptype = %d, video_pid = %d\n", sp->leader, sp->apptype, psi->video_pid);

    if (sp->load_valid < 0) {
        LOG_STRM_WARN("#%d normal media\n", sp->leader);
        sp->load_valid = 0;
    }

    sp->psi_flag = 1;
    if (sp->shift_sbp)
        strm_bufplay_psi_set(sp->shift_sbp, psi);
    int_play_psi(sp);

    int_steam_setAudioChannels(sp->codec, psi->audio_num);

    if (0 == psi->video_pid && sp->still_flg) {
        codec_close(sp->codec, 1);
        sp->still_flg = 0;
        codec_open(sp->codec, sp->iptv_flag * g_pcr_flag);
    }
    if (codec_psi(sp->codec, psi)) {
        int_play_error(sp, RTSP_CODE_Unsupported_Media);
        LOG_STRM_ERROUT("#%d unsupport media type 1\n", sp->leader);
    }

    local_ca_update(sp);

    int_play_pts(sp);
    sp->end_pts = sp->ply_pts;
    if (psi->video_pid)
        sp->still_flg = 1;

    if (0 == sp->codec && sp->ca.system_id)
        sp->ca_wait_clk = 1;

    if (PLAY_STATE_TPLAY == sp->state) {
        codec_tplay(sp->codec);

        if (0x0b00 == (sp->ca.system_id & 0xff00))
            sp->conax_flag = 1;
    } else if (PLAY_STATE_PAUSE == sp->state) {
        codec_pause(sp->codec);
    } else {
        if (APP_TYPE_VOD == sp->apptype && 0 == sp->codec && g_voddelay_clks > 0) {
            LOG_STRM_PRINTF("#%d delay clks = %d\n", sp->leader, g_voddelay_clks);
            sp->delay_clk = sp->clk + g_voddelay_clks;
        }
        if (1 != sp->iptv_flag)
            int_play_resume(sp);
        if (sp->underflow_flag)
            sp->underflow_flag = 1;
    }

    LOG_STRM_DEBUG("#%d STRM_MSG_PSI_VIEW\n", sp->leader);
    sp->msg_callback(sp->msg_handle, STRM_MSG_PSI_VIEW, 0);

    if (0x0b00 == (sp->ca.system_id & 0xff00))
        sp->pcr_mode = -1;
    else
        sp->pcr_mode = 0;

    return 0;
Err:
    return -1;
}

void strm_play_reset(StreamPlay* sp, int leader, int caReset)
{
    LOG_STRM_PRINTF("#%d ---- state = %d, ca_system_id = %d\n", leader, sp->state, (uint32_t)sp->ca.system_id);

    mid_mutex_lock(sp->loop_mutex);
    if (leader != sp->leader)
        LOG_STRM_ERROUT("leader = %d / %d\n", leader, sp->leader);
    if (PLAY_STATE_CLOSE == sp->state || !sp->sbp)
        goto Err;

    if (sp->psi_flag)
        codec_reset(sp->codec, caReset);

    int_play_reset(sp);
    if (sp->scale)
        ts_pts_reset(sp->ts_pts, sp->scale);
    else
        ts_pts_reset(sp->ts_pts, 1);
    sp->pcr_clk = 0;

    if (0 == sp->codec && caReset && sp->ca.system_id)
        sp->ca_wait_clk = 1;

Err:
    mid_mutex_unlock(sp->loop_mutex);
}

void strm_play_pause(StreamPlay* sp, int leader)
{
    LOG_STRM_PRINTF("#%d ---- state = %d\n", leader, sp->state);

    mid_mutex_lock(sp->loop_mutex);
    if (leader != sp->leader)
        LOG_STRM_ERROUT("leader = %d / %d\n", leader, sp->leader);

    if (abs(sp->scale) > 1)
        ts_pts_reset(sp->ts_pts, 1);
    sp->scale = 0;

    if (APP_TYPE_TSTV == sp->apptype || APP_TYPE_DVBS == sp->apptype)
        sp->shift_flag = 1;
    else
        sp->shift_flag = 0;

    if (sp->psi_flag)
        codec_pause(sp->codec);

    if (sp->pp.firstRealTime)
        sp->pp.pauseTime = int_timeNow( );

    sp->state = PLAY_STATE_PAUSE;
    LOG_STRM_DEBUG("#%d PLAY_STATE_PAUSE\n", leader);

    if (ts_pcr_time(sp->ts_pcr) > 0)
        sp->ts_pcr_pause = sp->clk;
    else
        sp->ts_pcr_pause = 0;

    sp->pp_speedTime = 0.0;
    sp->pp_emptyTime = 0.0;

Err:
    mid_mutex_unlock(sp->loop_mutex);
}

/*
    0 vod
    1 iptv
    2 无流清缓冲
 */
void strm_play_resume(StreamPlay* sp, int leader, int iptv_flag)
{
    LOG_STRM_PRINTF("#%d ---- state = %d end_flg = %d, iptv_flag = %d, pcr_flag = %d\n", leader, sp->state, sp->end_flg, iptv_flag, g_pcr_flag);

    mid_mutex_lock(sp->loop_mutex);
    if (leader != sp->leader)
        LOG_STRM_ERROUT("leader = %d / %d\n", leader, sp->leader);

    if (abs(sp->scale) > 1)
        ts_pts_reset(sp->ts_pts, 1);
    sp->scale = 1;
    sp->data_clk = mid_10ms( );

    if (iptv_flag) {
        sp->iptv_flag = 1;
        if (2 == iptv_flag)
            sp->data_clk = sp->clk - INTERVAL_CLK_DATA_TIMEOUT;
    } else {
        sp->iptv_flag = 0;
        if (APP_TYPE_TSTV == sp->apptype || APP_TYPE_DVBS == sp->apptype)
            sp->shift_flag = 1;
        else
            sp->shift_flag = 0;
    }

    if (1 == sp->iptv_flag)
        sp->end_flg = 0;
    if (sp->end_flg)
        int_play_end(sp);

    if (sp->psi_flag) {
        codec_resume(sp->codec, sp->iptv_flag * g_pcr_flag);
        int_play_resume(sp);
        if (sp->underflow_flag)
            sp->underflow_flag = 1;
    }

    if (sp->pp.firstRealTime && sp->pp.pauseTime) {
        double timeNow = int_timeNow( );
        sp->pp.firstRealTime += timeNow - sp->pp.pauseTime;
        sp->pp.pauseTime = 0;
    }

    sp->state = PLAY_STATE_PLAY;
    LOG_STRM_DEBUG("#%d PLAY_STATE_PLAY\n", sp->leader);

    if (ts_pcr_time(sp->ts_pcr) > 0 && sp->ts_pcr_pause) {
        sp->ts_pcr_base = sp->ts_pcr_base + (sp->clk - sp->ts_pcr_pause);
        sp->ts_pcr_pause = 0;
    }

    if (0 == sp->codec && sp->ca.system_id && sp->ca_wait_clk)
        sp->ca_wait_clk = 1;

Err:
    mid_mutex_unlock(sp->loop_mutex);
}

void strm_play_tplay(StreamPlay* sp, int leader, int scale)
{
    LOG_STRM_PRINTF("#%d ---- state = %d, scale = %d\n", leader, sp->state, scale);

    mid_mutex_lock(sp->loop_mutex);
    if (leader != sp->leader)
        LOG_STRM_ERROUT("leader = %d / %d\n", leader, sp->leader);

    if (APP_TYPE_TSTV == sp->apptype || APP_TYPE_DVBS == sp->apptype)
        sp->shift_flag = 1;
    else
        sp->shift_flag = 0;

    sp->end_flg = 0;
    sp->conax_flag = 0;
    sp->ca_wait_clk = 0;

    if (sp->psi_flag) {
        codec_tplay(sp->codec);

        if (0x0b00 == (sp->ca.system_id & 0xff00))
            sp->conax_flag = 1;
    }
    sp->state = PLAY_STATE_TPLAY;
    LOG_STRM_DEBUG("#%d PLAY_STATE_TPLAY\n", leader);

    sp->scale = scale;
    sp->data_clk = mid_10ms( );

    ts_pts_reset(sp->ts_pts, scale);

    if (0 == sp->codec && sp->ca.system_id && sp->ca_wait_clk)
        sp->ca_wait_clk = 1;

Err:
    mid_mutex_unlock(sp->loop_mutex);
}

void strm_play_tskip(StreamPlay* sp, int leader)
{
    LOG_STRM_PRINTF("#%d ---- state = %d, scale = %d\n", leader, sp->state, sp->scale);

    mid_mutex_lock(sp->loop_mutex);
    if (leader != sp->leader)
        LOG_STRM_ERROUT("leader = %d / %d\n", leader, sp->leader);

     sp->tskip_clk = 1;

Err:
    mid_mutex_unlock(sp->loop_mutex);
}

void strm_play_rect(StreamPlay* sp, int leader, int x, int y, int width, int height)
{
    LOG_STRM_PRINTF("#%d ---- %d\n", leader, sp->leader);

    mid_mutex_lock(sp->loop_mutex);

    codec_rect(sp->codec, x, y, width, height);

    mid_mutex_unlock(sp->loop_mutex);
}

static void int_play_end(StreamPlay* sp)
{
    sp->end_flg = 1;
    sp->end_pts = 0;
    sp->end_clk = 0;

    codec_pts(sp->codec, &sp->end_pts);
}

void strm_play_end(StreamPlay* sp, int leader)
{
    LOG_STRM_PRINTF("#%d ---- state = %d\n", leader, sp->state);

    mid_mutex_lock(sp->loop_mutex);
    if (leader != sp->leader)
        LOG_STRM_ERROUT("leader = %d / %d\n", leader, sp->leader);

    int_play_end(sp);

Err:
    mid_mutex_unlock(sp->loop_mutex);
}

//已经完全播放结束，不需要再检测判断
void strm_play_set_idle(StreamPlay* sp, int leader, int idle)
{
    LOG_STRM_PRINTF("#%d ---- state = %d, idle = %d\n", leader, sp->state, idle);

    mid_mutex_lock(sp->loop_mutex);
    if (leader != sp->leader)
        LOG_STRM_ERROUT("leader = %d / %d\n", leader, sp->leader);

    sp->isIdle = idle;

Err:
    mid_mutex_unlock(sp->loop_mutex);
}

static void int_play_clean(StreamPlay* sp)
{
    if (sp->ts_parse) {
        ts_parse_delete(sp->ts_parse);
        sp->ts_parse = NULL;
    }
    if (sp->ts_pts) {
        ts_pts_delete(sp->ts_pts);
        sp->ts_pts = NULL;
    }
    if (sp->ts_pcr) {
        ts_pcr_delete(sp->ts_pcr);
        sp->ts_pcr = NULL;
    }

    if (sp->sbp) {
        strm_bufplay_delete(sp->sbp);
        sp->sbp = NULL;
    }
    if (sp->shift_sbp) {
        strm_bufplay_delete(sp->shift_sbp);
        sp->shift_sbp = NULL;
    }
}

static void int_play_cls(StreamPlay* sp, int clear)
{
    if (PLAY_STATE_CLOSE == sp->state && 0 == clear)
        return;

    if (PLAY_STATE_CLOSE == sp->state) {
        if (sp->still_flg || (1 == sp->codec && sp->open_flg) || 0 == sp->codec) {
            codec_close(sp->codec, 1);
            if (sp->open_flg)
                sp->open_flg = 0;
            if (sp->still_flg)
                sp->still_flg = 0;
        }
    } else {
        local_ca_close(sp);
        codec_close(sp->codec, clear);
        if (clear) {
            sp->open_flg = 0;
            sp->still_flg = 0;
        }

        mid_mutex_lock(sp->data_mutex);
        int_play_clean(sp);
        mid_mutex_unlock(sp->data_mutex);

        int_steam_setAudioChannels(sp->codec, 0);
        int_steam_setContinuityError(sp->codec, 0);

        sp->state = PLAY_STATE_CLOSE;
        LOG_STRM_DEBUG("#%d PLAY_STATE_CLOSE\n", sp->leader);
    }
}

void strm_play_close(StreamPlay* sp, int leader, int clear)
{
    LOG_STRM_PRINTF("#%d ---- state = %d, open_flg = %d, still_flg = %d, clear = %d\n", leader, sp->state, sp->open_flg, sp->still_flg, clear);

    mid_mutex_lock(sp->loop_mutex);
    if (leader != sp->leader)
        LOG_STRM_ERROUT("leader = %d / %d\n", leader, sp->leader);

    if (sp->state != PLAY_STATE_CLOSE) {

    #ifdef ENABLE_SAVE_PLAY
        LOG_STRM_PRINTF("@@@@@@@@: save_len = %d\n", sp->save_len);
        if (sp->save_fp) {
            fclose(sp->save_fp);
            sp->save_fp = NULL;
        }
    #endif

        int_play_cls(sp, clear);
    }

Err:
    mid_mutex_unlock(sp->loop_mutex);
}

static void int_play_error(StreamPlay* sp, int arg)
{
    LOG_STRM_PRINTF("#%d STRM_MSG_PLAY_ERROR\n", sp->leader);
    sp->msg_callback(sp->msg_handle, STRM_MSG_PLAY_ERROR, arg);
    int_play_cls(sp, 1);
}

void strm_play_clear(StreamPlay* sp, int leader)
{
    if (leader == sp->leader) {
        LOG_STRM_PRINTF("#%d ---- state = %d, still_flg = %d\n", leader, sp->state, sp->still_flg);
        sp->enable = -1;
    }
}

int strm_play_time(StreamPlay* sp)
{
    return sp->time_play;
}

int strm_play_diff(StreamPlay* sp)
{
    return sp->time_diff;
}

int strm_play_buffer(StreamPlay* sp)
{
    return sp->time_buffer;
}

void strm_play_resize(StreamPlay* sp, int leader, int size, int resize)
{
    mid_mutex_lock(sp->loop_mutex);
    mid_mutex_lock(sp->data_mutex);
    if (leader != sp->leader)
        LOG_STRM_ERROUT("leader = %d / %d\n", leader, sp->leader);
    if (PLAY_STATE_CLOSE == sp->state || !sp->sbp)
        goto Err;

    if (resize != sp->sbq_rawsize) {
        int num;

        sp->sbq_rawsize = resize;

        strm_bufplay_delete(sp->sbp);

        if (RTP_BUFFER_LENGTH == size)
            num = sp->sbq_rawsize / 1316;
        else
            num = sp->sbq_rawsize / size;

        sp->sbp = strm_bufplay_create(size, num);
        LOG_STRM_PRINTF("#%d ---- sbp = %p, size = %d, num = %d\n", leader, sp->sbp, size, num);

        if (sp->psi_flag)
            strm_bufplay_psi_set(sp->sbp, &sp->ts_psi);
    }

Err:
    mid_mutex_unlock(sp->data_mutex);
    mid_mutex_unlock(sp->loop_mutex);
}

int strm_play_space(StreamPlay* sp)
{
    int result = 0;
    StrmBufQue* sbq;

    mid_mutex_lock(sp->data_mutex);
    if (PLAY_STATE_CLOSE == sp->state || !sp->sbp)
        goto Err;

    sbq = sp->sbp->sbq;
    if (sp->shift_flag)
        sbq = sp->shift_sbp->sbq;

    result = strm_bufque_space(sbq);

Err:
    mid_mutex_unlock(sp->data_mutex);
    return result;
}

int strm_play_length(StreamPlay* sp)
{
    int result = 0;
    StrmBufQue* sbq;

    mid_mutex_lock(sp->data_mutex);
    if (PLAY_STATE_CLOSE == sp->state || !sp->sbp)
        goto Err;

    sbq = sp->sbp->sbq;
    if (sp->shift_flag)
        sbq = sp->shift_sbp->sbq;

    result = strm_bufque_length(sbq);

Err:
    mid_mutex_unlock(sp->data_mutex);
    return result;
}

static void local_ca_open(StreamPlay* sp, ts_ca_t ca)
{
    uint32_t system_id = (uint32_t)ca->system_id;

    LOG_STRM_PRINTF("#%d ca system_id = 0x%04x, pid = %d\n", sp->leader, system_id, (uint32_t)ca->pid);

    switch(system_id) {
    case 0x0604: /* Irdeto CA */
        LOG_STRM_PRINTF("#%d Irdeto CA\n", sp->leader);
        sp->section_len = ts_parse_getpmt(sp->ts_parse, sp->section_buf, TS_SECTION_SIZE);
        codec_ca_update(sp->codec, ca, sp->section_buf, sp->section_len);
        break;
    case 0x4a02:/* 永新视博 */
        LOG_STRM_PRINTF("#%d NOVEL-SUPERTV\n", sp->leader);
        codec_ca_update(sp->codec, ca, NULL, 0);
        break;
    case 0x5601:/* VERIMATRIX */
        LOG_STRM_PRINTF("#%d VERIMATRIX\n", sp->leader);
        codec_ca_update(sp->codec, ca, NULL, 0);
        break;
    case 0x0666:/* SecureMedia */
        LOG_STRM_PRINTF("#%d SecureMedia\n", sp->leader);
        codec_ca_update(sp->codec, ca, NULL, 0);
        break;
    default:
        {
            uint32_t mask = (uint32_t)(system_id & 0xff00);
            if (0x0b00 == mask || 0x1800 == mask) {
                if (0x0b00 == mask)
                    LOG_STRM_PRINTF("#%d CONAX apptype = %d\n", sp->leader, sp->apptype);
                else
                    LOG_STRM_PRINTF("#%d NAGRA apptype = %d\n", sp->leader, sp->apptype);
                sp->section_len = ts_parse_getpmt(sp->ts_parse, sp->section_buf, TS_SECTION_SIZE);
                codec_ca_update(sp->codec, ca, sp->section_buf, sp->section_len);

                if (APP_TYPE_DVBS == sp->apptype)
                    sp->cat_wait_clk = sp->clk + 50;
                else
                    codec_ca_cat(sp->codec, NULL, 0);
            } else {
                LOG_STRM_PRINTF("#%d Unknown CA 0x%04x!\n", sp->leader, system_id);
            }
        }
        break;
    }

    memcpy(&sp->ca, ca, sizeof(sp->ca));
}

static void local_ca_close(StreamPlay* sp)
{
    LOG_STRM_PRINTF("#%d ca system_id = 0x%04x\n", sp->leader, (uint32_t)sp->ca.system_id);

    if (0 == sp->ca.system_id)
        return;

    switch(sp->ca.system_id) {
    case 0x0604: /* Irdeto CA */
        LOG_STRM_PRINTF("#%d Irdeto CA\n", sp->leader);
        codec_ca_update(sp->codec, NULL, NULL, 0);
        break;
    case 0x4a02:/* 永新视博 */
        LOG_STRM_PRINTF("#%d NOVEL-SUPERTV\n", sp->leader);
        codec_ca_update(sp->codec, NULL, NULL, 0);
        break;
    case 0x5601:/* VERIMATRIX */
        LOG_STRM_PRINTF("#%d VERIMATRIX\n", sp->leader);
        codec_ca_update(sp->codec, NULL, NULL, 0);
        break;
    case 0x0666:/* SecureMedia */
        LOG_STRM_PRINTF("#%d SecureMedia\n", sp->leader);
        codec_ca_update(sp->codec, NULL, NULL, 0);
        break;
    default:
        {
            uint32_t mask = (uint32_t)(sp->ca.system_id & 0xff00);
            if (0x0b00 == mask || 0x1800 == mask) {
                if (0x0b00 == mask)
                    LOG_STRM_PRINTF("#%d CONAX\n", sp->leader);
                else
                    LOG_STRM_PRINTF("#%d NAGRA\n", sp->leader);
                codec_ca_update(sp->codec, NULL, NULL, 0);
            } else {
                LOG_STRM_PRINTF("#%d Unknown CA!\n", sp->leader);
            }
        }
        break;
    }

    sp->ca.system_id = 0;
}

static void local_ca_update(StreamPlay* sp)
{
    uint32_t mask;
    int i, num;
    struct ts_ca ca, *ecm_array;

    ca.system_id = 0;
    num = sp->ts_psi.ecm_num;
    ecm_array = sp->ts_psi.ecm_array;
    if (num > 0) {
        for (i = 0; i < num; i ++) {
            ca = ecm_array[i];
            mask = (uint32_t)(ca.system_id & 0xff00);
            if (0x0b00 == mask || 0x1800 == mask)
                break;
        }
        if (i >= num)
            ca = ecm_array[0];
    }

    if (ca.system_id == sp->ca.system_id)
        return;
    LOG_STRM_PRINTF("#%d system_id = %04x / %04x\n", sp->leader, (uint32_t)ca.system_id, (uint32_t)sp->ca.system_id);

    if (0 == sp->ca.system_id) {
        local_ca_open(sp, &ca);
    } else {
        local_ca_close(sp);
        if (0 == ca.system_id)
            return;
        local_ca_open(sp, &ca);
    }
}

static int local_ca_check(StreamPlay* sp)
{
    if (0 == sp->ca_wait_clk)
        return 0;

    if (sp->ca_wait_clk < sp->clk) {
        LOG_STRM_WARN("#%d wait CA timeout! clk = %u / %u\n", sp->leader, sp->ca_wait_clk, sp->clk);
        sp->ca_wait_clk = 0;
        return 0;
    }
    {
        int ret = codec_ca_check(sp->codec);
        if (1 == ret) {
            LOG_STRM_PRINTF("#%d wait CA OK!\n", sp->leader);
            sp->ca_wait_clk = 0;
        }
        return ret;
    }
}

void strm_play_emm(StreamPlay* sp, int *emm_pid, int *emm_num)
{
    int i;
    ts_psi_t psi = &sp->ts_psi;

    if (psi->emm_num > 0) {
        int num = psi->emm_num;
        LOG_STRM_PRINTF("#%d emm_num = %d! pcr_pid = 0x%x / (0x%x, 0x%x)\n", sp->leader, psi->emm_num, psi->pcr_pid, psi->video_pid, psi->audio_pid[0]);

        if (psi->pcr_pid != psi->video_pid && psi->pcr_pid != psi->audio_pid[0]) {
            if (num >= TS_EMM_NUM)
                num = TS_EMM_NUM - 1;
            for (i = 0; i < num; i ++)
                emm_pid[i] = (int)((uint32_t)psi->emm_array[i].pid);
            emm_pid[num] = psi->pcr_pid;
            *emm_num = num + 1;
        } else {
            *emm_num = num;
            for (i = 0; i < psi->emm_num; i ++)
                emm_pid[i] = (int)((uint32_t)psi->emm_array[i].pid);
        }
    } else {
        *emm_num = 0;
    }
}

static int int_play_parse(StreamPlay* sp, const char *buf, int len)
{
    int ret;
    ts_psi_t psi = &sp->ps_psi;
    uint32_t pmt_crc = sp->ts_psi.pmt_crc;

    ret = ts_parse_psi(sp->ts_parse, (u_char *)buf, len, sp->ts_pts);

    if (sp->cat_wait_clk) {
        sp->section_len = ts_parse_getcat(sp->ts_parse, sp->section_buf, TS_SECTION_SIZE);
        if (sp->section_len > 0) {
            LOG_STRM_PRINTF("#%d section_len = %d\n", sp->leader, sp->section_len);
            codec_ca_cat(sp->codec, sp->section_buf, sp->section_len);
            sp->cat_wait_clk = 0;
        } else if (sp->cat_wait_clk < sp->clk) {
            LOG_STRM_PRINTF("#%d cat_wait_clk timeout!\n", sp->leader);
            codec_ca_cat(sp->codec, NULL, 0);
            sp->cat_wait_clk = 0;
        }
    }

    if (1 == ret) {
        if (0 == psi->video_pid && 0 == psi->audio_num) {
            if (sp->load_valid >= 0)
                LOG_STRM_WARN("#%d empty media\n", sp->leader);

            sp->load_valid = -1;
            return 0;
        }
        if (sp->load_valid < 0) {
            LOG_STRM_WARN("#%d normal media\n", sp->leader);
            sp->load_valid = 0;
        }

        if (0 == ts_psi_equal(&sp->ts_psi, psi)) {
            sp->psi_exceps ++;
            sp->crc_exceps = 0;
            if (sp->psi_exceps >= 5) {
                sp->psi_exceps = 0;
                LOG_STRM_PRINTF("#%d STRM_MSG_CHANGE_PSI\n", sp->leader);
                sp->msg_callback(sp->msg_handle, STRM_MSG_CHANGE_PSI, 0);

                int_steam_setAudioChannels(sp->codec, psi->audio_num);
                memset(sp->stat_numArray, 0, sizeof(int) * TS_AUDIO_NUM);

                ts_psi_copy(&sp->ts_psi, psi);

                strm_bufplay_psi_set(sp->sbp, psi);
                if (sp->shift_sbp)
                    strm_bufplay_psi_set(sp->shift_sbp, psi);

                int_play_psi(sp);

                local_ca_close(sp);

                if (0 == psi->video_pid)
                    sp->pic_width = -1;
                else if (sp->pic_width > 0)
                    sp->pic_width = 0;
                if (0 == psi->video_pid && sp->still_flg) {
                    codec_close(sp->codec, 1);
                    sp->still_flg = 0;
                } else {
                    codec_close(sp->codec, -1);
                }
                codec_open(sp->codec, sp->iptv_flag * g_pcr_flag);

                if (codec_psi(sp->codec, psi)) {
                    int_play_error(sp, RTSP_CODE_Unsupported_Media);
                    LOG_STRM_ERROUT("#%d unsupport media type 2\n", sp->leader);
                }

                local_ca_update(sp);

                if (psi->video_pid)
                    sp->still_flg = 1;

                if (PLAY_STATE_TPLAY == sp->state) {
                    codec_tplay(sp->codec);
                } else if (PLAY_STATE_PAUSE == sp->state) {
                    codec_pause(sp->codec);
                } else {
                    if (sp->underflow_flag)
                        sp->underflow_flag = 1;
                }

                return 1;
            }
        } else {
            sp->psi_exceps = 0;

            if (psi->pmt_crc != sp->ts_psi.pmt_crc) {
                sp->crc_exceps ++;
                if (sp->crc_exceps >= 5) {
                    sp->crc_exceps = 0;

                    LOG_STRM_PRINTF("#%d change crc: %08x > %08x\n", sp->leader, sp->ts_psi.pmt_crc, psi->pmt_crc);
                    ts_psi_copy(&sp->ts_psi, psi);
                    LOG_STRM_PRINTF("#%d PMT STRM_MSG_CHANGE_CRC\n", sp->leader);
                    sp->msg_callback(sp->msg_handle, STRM_MSG_CHANGE_CRC, 2);
                }
            } else {
                sp->crc_exceps = 0;

                if (psi->emm_num > 0) {
                    if (ts_cat_equal(psi, &sp->ts_psi) == 0) {
                        LOG_STRM_PRINTF("#%d emm_num: %d > %d\n", sp->leader, sp->ts_psi.emm_num, psi->emm_num);
                        ts_psi_copy(&sp->ts_psi, psi);
                        LOG_STRM_PRINTF("#%d CAT STRM_MSG_CHANGE_CRC\n", sp->leader);
                        sp->msg_callback(sp->msg_handle, STRM_MSG_CHANGE_CRC, 1);
                    }
                    psi->emm_num = 0;
                }
            }

            local_ca_update(sp);

            if (pmt_crc != psi->pmt_crc && 0x0604 == sp->ca.system_id/* Irdeto CA */) {
                sp->section_len = ts_parse_getpmt(sp->ts_parse, sp->section_buf, TS_SECTION_SIZE);
                if (sp->section_len > 0)
                    codec_ca_update(sp->codec, &sp->ca, sp->section_buf, sp->section_len);
            }
        }
    }
#ifdef ENABLE_SAVE_PLAY
    if (sp->save_fp) {
        ret = fwrite(buf, 1, len, sp->save_fp);
        if (ret != len)
            LOG_STRM_WARN("#%d fwrite ret = %d, len = %d\n", sp->leader, ret, len);
        sp->save_len += len;
    }
#endif

Err:
    return 0;
}

//const char *data_buf, int data_len
void strm_play_push(StreamPlay* sp, int leader, StrmBuffer **psb)
{
    char *data_buf;
    int data_len, len;
    StrmBuffer *sb;
    StrmBufPlay *sbp;

    sb = *psb;
    g_bytes += sb->len;
    mid_mutex_lock(sp->data_mutex);

    if (leader != sp->leader)
        goto Err;

    sbp = sp->sbp;
    if (sp->shift_flag)
        sbp = sp->shift_sbp;

    if (PLAY_STATE_CLOSE == sp->state || !sbp)
        goto Err;

    data_buf = sb->buf + sb->off;
    data_len = sb->len;

    if (0 == sp->codec) {
        if (gTSSyncInterval) {
            if (gTSSyncClock < sp->clk) {
                gTSSyncClock = sp->clk + gTSSyncInterval;
                gTSSyncNotifyCall( );
            }
        }
    }

    {
        int continuityError, synchronizationError;
        uint8_t *ubuf;
        uint16_t pid, counter;
        TSContinuity *tsContinuity, *prev;

        continuityError = 0;
        synchronizationError = 0;
        ubuf = (unsigned char *)data_buf;
        for (len = 0; len < data_len; len += 188, ubuf += 188) {
            if (0x47 == ubuf[0]) {
                pid = ((uint16_t)(ubuf[1] & 0x1f) << 8) + ubuf[2];
                if (0x1fff != pid) {
                    counter = ubuf[3] & 0xf;

                    prev = NULL;
                    tsContinuity = sp->tsContinuity;
                    while (tsContinuity) {
                        if (pid == tsContinuity->pid)
                            break;
                        prev = tsContinuity;
                        tsContinuity = tsContinuity->next;
                    }

                    if (tsContinuity) {
                        if (((tsContinuity->counter + 1) & 0xf) != counter)
                            continuityError++;
                        tsContinuity->times++;
                        tsContinuity->counter = counter;

                        prev = tsContinuity->prev;
                        if (prev && tsContinuity->times > prev->times + 7) {
                            TSContinuity *prevPrev, *next;

                            prevPrev = prev->prev;
                            next = tsContinuity->next;

                            tsContinuity->prev = prevPrev;
                            if (prevPrev)
                                prevPrev->next = tsContinuity;
                            else
                                sp->tsContinuity = tsContinuity;

                            tsContinuity->next = prev;
                            prev->prev = tsContinuity;

                            prev->next = next;
                            if (next)
                                next->prev = prev;
                        }
                    } else {
                        LOG_STRM_DEBUG("#%d new pid = %u\n", leader, (uint32_t)pid);
                        tsContinuity = (TSContinuity*)IND_CALLOC(sizeof(TSContinuity), 1);
                        if (tsContinuity) {
                            tsContinuity->pid = pid;
                            tsContinuity->times = 1;
                            tsContinuity->counter = counter;

                            if (prev) {
                                prev->next = tsContinuity;
                                tsContinuity->prev = prev;
                            } else {
                                sp->tsContinuity = tsContinuity;
                            }
                        }
                    }
                }
            } else {
                synchronizationError++;
            }
        }
        if (continuityError)
            int_steam_setContinuityError(sp->codec, continuityError);

        //检查数据有效性
        if (data_len % 188)
            LOG_STRM_ERROUT("#%d data_len = %d\n", leader, data_len);

        if (synchronizationError) {
            static int clk = 0;
            if (clk < sp->clk) {
                clk =  sp->clk + 100;
                LOG_STRM_ERROR("#%d sync[%d] = 0x%02x\n", leader, len, (uint32_t)((uint8_t)data_buf[len]));
            }
            goto Err;
        }
    }

    if (strm_bufque_space(sbp->sbq) <= 0) {
        LOG_STRM_WARN("#%d ########: OVERFLOW! flg = %d\n", sp->leader, sp->flow_flg);
        if (0 == sp->underflow_flag && sp->flow_flg)
            sp->msg_callback(sp->msg_handle, STRM_MSG_OVERFLOW, 0);
        strm_bufplay_reset(sbp);
    }
    strm_bufque_push(sbp->sbq, psb);

Err:
    mid_mutex_unlock(sp->data_mutex);
}

static void int_play_push_ca(StreamPlay* sp, StrmBufPlay* sbp)
{
    int len, decode_len;
    char *decode_buf;
    StrmBuffer *sb;

    sb = sbp->sb;

    for (;;) {
        decode_len = 0;
        codec_buf_get(sp->codec, &decode_buf, &decode_len);
        if (decode_len <= 0)
            break;

        sb->len = 0;
        mid_mutex_lock(sp->data_mutex);
        strm_bufplay_ca(sp->sbp, sb);
        mid_mutex_unlock(sp->data_mutex);
        if (sb->len <= 0)
            break;

        len = sb->len;
        if (len > decode_len)
            len = decode_len;

        IND_MEMCPY(decode_buf, sb->buf, len);
        sp->push_clk = sp->clk;
        codec_buf_put(sp->codec, len);
        {
            static int clk = 0;
            if (clk < sp->clk) {
                clk =  sp->clk + 40;
                LOG_STRM_PRINTF("#%d ECM len = %d\n", sp->leader, len);
            }
        }
    }
    sb->len = 0;
}

static void int_play_push_put(StreamPlay* sp, char *buf, int len)
{
    int ret;

#ifdef ENABLE_SAVE_PLAY
    if (sp->save_fp) {
        ret = fwrite(buf, 1, len, sp->save_fp);
        if (ret != len)
            LOG_STRM_WARN("#%d fwrite ret = %d, len = %d\n", sp->leader, ret, len);
        sp->save_len += len;
    }
#endif

    if (PLAY_STATE_PLAY == sp->state || PLAY_STATE_PAUSE == sp->state) {
        int last;
        ByteStat *stat = &sp->stat_array[0];

        mid_mutex_lock(sp->data_mutex);
        stat->bytes += len;
        last = ts_pts_time_last(sp->ts_pts, 1);
        if (last > 0) {
            stat->clk_end = last;
            if (last >= stat->clk_begin + 100) {
                int i;
                ByteStat *array = sp->stat_array;

                for (i = STAT_ARRAY_SIZE - 1; i > 0; i --)
                    array[i] = array[i - 1];

                stat->bytes = 0;
                stat->clk_begin = last;
                stat->clk_end = last;
            }
        }
        mid_mutex_unlock(sp->data_mutex);
    }

    if (-1 == sp->pcr_mode) {
        ts_psi_t psi = &sp->ts_psi;

        ret = ts_index_check(buf, len, psi->video_pid, psi->audio_pid[0]);
        if (ret) {
            if (-1 == ret)
                sp->pcr_mode = 1;
            else
                sp->pcr_mode = 0;
            LOG_STRM_PRINTF("#%d ret = %d, pcr_mode = %d\n", sp->leader, ret, sp->pcr_mode);
        }
    }

    sp->push_clk = sp->clk;
    if (1 == int_play_parse(sp, buf, len))
        return;
    codec_buf_put(sp->codec, len);
}

static void int_play_push_tplay(StreamPlay* sp, StrmBufPlay* sbp)
{
    int decode_len, data_len;
    char *decode_buf;
    StrmBuffer *sb;

    sb = sbp->sb;

    if (sp->tskip_clk) {
        sp->push_clk = sp->clk;

        if (1 == sp->tskip_clk) {
            sp->tskip_clk = sp->clk;
        } else if (sp->clk - sp->tskip_clk > 10) {
            int length;

            mid_mutex_lock(sp->data_mutex);
            length = strm_bufque_length(sbp->sbq);
            strm_bufplay_reset(sbp);
            mid_mutex_unlock(sp->data_mutex);

            LOG_STRM_PRINTF("#%d SKIP = %d\n", sp->leader, length);
            sb->len = 0;
            sp->tskip_clk = 0;
        }
        return;
    }

    decode_len = 0;
    codec_buf_get(sp->codec, &decode_buf, &decode_len);
    if (decode_len <= 0)
        return;

    data_len = 0;
    while (data_len < decode_len) {
        if (sb->len <= 0) {
            char *buf;

            mid_mutex_lock(sp->data_mutex);
            if (strm_bufque_length(sbp->sbq) > 0)
                strm_bufplay_pop(sbp);
            mid_mutex_unlock(sp->data_mutex);
            sb = sbp->sb;
            if (sb->len <= 0)
                break;

            buf = sb->buf + sb->off;
            if (sp->conax_flag) {
                if (0 == memcmp(buf, g_ff, 7)) {
                    if ('1' == buf[7])
                        sp->conax_flag = 2;
                    else if ('2' == buf[7])
                        sp->conax_flag = 3;
                }
            }
        }

        if (sp->conax_flag > 1) {
            uint32_t pid;
            uint8_t *ubuf;
            int i, num, flag;
            ts_ca_t array;
            ts_psi_t psi = &sp->ts_psi;

            while (sb->len > 0 && data_len < decode_len) {
                flag = 0;
                ubuf = (uint8_t *)(sb->buf + sb->off);
                pid =(((uint32_t)ubuf[1] & 0x1f) << 8) + ubuf[2];

                num = psi->ecm_num;
                if (num > 0) {
                    array = psi->ecm_array;
                    for (i = 0; i < num; i ++) {
                        if (pid == (uint32_t)array[i].pid) {
                            flag = 1;
                            break;
                        }
                    }
                }

                if (0 == flag) {
                    if (0 == pid || 1 == pid) {//PAT CAT
                        flag = 1;
                    } else if (pid == (uint32_t)psi->pmt_pid) {//PMT
                        flag = 1;
                    } else {
                        num = psi->emm_num;
                        if (num > 0) {
                            array = psi->emm_array;
                            for (i = 0; i < num; i ++) {
                                if (pid == (uint32_t)array[i].pid) {
                                    flag = 1;
                                    break;
                                }
                            }
                        }
                    }
                }

                if ((2 == sp->conax_flag && 1 == flag) || (2 != sp->conax_flag && 0 == flag)) {
                    IND_MEMCPY(decode_buf + data_len, ubuf, 188);
                    data_len += 188;
                }
                sb->off += 188;
                sb->len -= 188;
            }
        } else {
            int len = decode_len - data_len;

            if (len > sb->len)
                len = sb->len;
            IND_MEMCPY(decode_buf + data_len, sb->buf + sb->off, len);
            data_len += len;

            sb->off += len;
            sb->len -= len;
        }
    }

    if (data_len > 0)
        int_play_push_put(sp, decode_buf, data_len);
}

static void int_play_push_dvbs(StreamPlay* sp, char *data_buf, int data_len)
{
    if (sp->load_valid >= 0 && sp->psi_flag) {
        uint32_t clk = sp->clk;

        if (sp->load_clk < clk) {
            if (sp->load_valid && !sp->load_check)
                sp->load_valid = 0;
            sp->load_check = 0;
            sp->load_clk = clk + 100;
        }

        if (!sp->load_check) {
            uint32_t pid;
            uint8_t *ubuf, *uend;
            ts_psi_t psi = &sp->ts_psi;

            ubuf = (uint8_t *)data_buf;
            uend = (uint8_t *)(data_buf + data_len);

            for (; ubuf < uend; ubuf += 188) {
                pid =(((uint32_t)ubuf[1] & 0x1f) << 8) + ubuf[2];
                if ((psi->video_pid && pid == psi->video_pid) || (psi->audio_num > 0 && pid == psi->audio_pid[0])) {
                    sp->load_valid = 1;
                    sp->load_check = 1;
                    break;
                }
            }
        }
    }
}

static void int_play_push_normal(StreamPlay* sp, StrmBufPlay* sbplay)
{
    char *buf, *decode_buf;
    int l, off, len, bytes, empty, enough, decode_len;
    uint32_t pcr;
    double timeNow;

    PlayPush *pp;
    StrmBuffer *sb;
    StrmBufPlay* sbp;

    pp = &sp->pp;
    timeNow = int_timeNow( );

    if (1 == sp->iptv_flag && 0 == sp->decode_valid)
        sp->pp_speedTime = timeNow + 2.0;

    bytes = 0;
    empty = 0;
    enough = 0;
    decode_len = 0;
    for (;;) {
        if (-1 == sp->enable)
            break;
        if (decode_len <= 0) {
            codec_buf_get(sp->codec, &decode_buf, &decode_len);
            if (decode_len <= 0)
                break;
            if (0 == sp->decode_valid) {
                LOG_STRM_PRINTF("#%d decode_size = %d!\n", sp->leader, decode_len);
                sp->decode_valid = 1;
            }
            decode_len = decode_len - decode_len % 188;
        }

        sbp = sbplay;
        if (1 == sp->shift_flag) {
            mid_mutex_lock(sp->data_mutex);
            if (strm_bufque_length(sp->sbp->sbq) <= 0)
                sp->shift_flag = 2;
            mid_mutex_unlock(sp->data_mutex);
            if (1 == sp->shift_flag)//直播直接转本地时移
                sbp = sp->sbp;
        }
        sb = sbp->sb;

        if (sb->len <= 0) {
            pp->firstPcrValue = 0;
            pp->secondPcrValue = 0;
            mid_mutex_lock(sp->data_mutex);
            pcr = 0;
            if (strm_bufque_length(sbp->sbq) > 0) {
                strm_bufplay_pcr(sbp, &off, &pcr);
                strm_bufplay_pop(sbp);
            }
            sb = sbp->sb;
            if (pcr) {
                pp->firstPcrOff = off;
                //printf("++++ %lld: foff = %d\n", pp->tsPacketCount, pp->firstPcrOff);
                pp->firstPcrValue = pcr;
                len = sb->len / 188;
                if (off < len) {
                    pcr = 0;
                    strm_bufplay_pcr(sbp, &off, &pcr);
                    if (pcr) {
                        pp->secondPcrOff = off + len - pp->firstPcrOff;
                        pp->secondPcrValue = pcr;
                        //printf("++++ %lld: soff = %d\n", pp->tsPacketCount, pp->secondPcrOff);
                    }
                }
            }
            mid_mutex_unlock(sp->data_mutex);
        } else {
            if (0 == pp->secondPcrValue) {
                pcr = 0;
                mid_mutex_lock(sp->data_mutex);
                strm_bufplay_pcr(sbp, &off, &pcr);
                mid_mutex_unlock(sp->data_mutex);
                if (pcr) {
                    len = sb->len / 188;
                    if (0 == pp->firstPcrValue) {
                        pp->firstPcrOff = off + len;
                        pp->firstPcrValue = pcr;
                        //printf("++++ %lld: foff1 = %d\n", pp->tsPacketCount, pp->firstPcrOff);
                    } else {
                        pp->secondPcrOff = off + len - pp->firstPcrOff;
                        //printf("++++ %lld: soff1 = %d\n", pp->tsPacketCount, pp->secondPcrOff);

                        pp->secondPcrValue = pcr;
                    }
                }
            }
        }

        if (sb->len > 0) {
            empty = 0;
            sp->pp_emptyTime = 0.0;

            len = decode_len - bytes;
            if (len > sb->len)
                len = sb->len;
            buf = sb->buf + sb->off;

            enough = 0;
            if (1) {
                ts_psi_t psi = &sp->ts_psi;
                double playTime;
                uint8_t *ubuf;
                uint32_t pid;
                int i;

                for (l = 0; l < len; l += 188) {
                    ubuf = (uint8_t*)(buf + l);

                    pid =(((uint32_t)ubuf[1] & 0x1f) << 8) + ubuf[2];
                    for (i = 0; i < psi->audio_num; i++) {
                        if (pid == psi->audio_pid[i]) {
                            sp->stat_numArray[i]++;
                            break;
                        }
                    }

                    int_updateDuration(pp, ubuf, timeNow);
                    pp->transmitDuration += pp->tsPacketDuration;

                    if ((0 == sp->iptv_flag || 0 == g_pcr_flag) && pp->tsPacketDuration > 0.0) {
                        playTime = pp->firstRealTime + pp->transmitDuration;
                        if (sp->pp_speedTime) {
                            if (sp->pp_speedTime <= timeNow) {
                                LOG_STRM_PRINTF("#%d speed timeout!\n", sp->leader);
                                sp->pp_speedTime = 0;
                            }
                            if (playTime + 0.2 > timeNow)
                                pp->firstRealTime -= pp->tsPacketDuration * ACCELERATED_FACTOR;
                        } else {
                            if (sp->time_diff < g_decodediff_clks) {
                                if (playTime + 0.2 > timeNow)
                                    pp->firstRealTime -= pp->tsPacketDuration * ACCELERATED_FACTOR;
                            }
                        }
                    }
                    playTime = pp->firstRealTime + pp->transmitDuration;
                    if (playTime > timeNow) {
                        enough = 1;
                        l += 188;
                        break;
                    }
                }
                //printf("++++ t = %f, d = %f / %d\n", pp->transmitDuration, pp->tsPacketDuration, l / 188);
            } else {
                l = len;
            }

            IND_MEMCPY(decode_buf + bytes, buf, l);
            bytes += l;
            sb->off += l;
            sb->len -= l;
        } else {
            if (sp->pp_emptyTime > 0.0) {
                if (timeNow - sp->pp_emptyTime > 1.0 && pp->firstRealTime > 0.0) {
                    LOG_STRM_WARN("#%d reset playpush!\n", sp->leader);
                    IND_MEMSET(pp, 0, sizeof(sp->pp));
                }
            } else {
                sp->pp_emptyTime = timeNow;
            }
            empty = 1;
        }

        if (bytes > 0 && (bytes >= decode_len || empty || enough)) {
            if (APP_TYPE_DVBS == sp->apptype && sp->iptv_flag)
                int_play_push_dvbs(sp, decode_buf, bytes);
            else
                sp->load_valid = 1;

            int_play_push_put(sp, decode_buf, bytes);
            decode_len = 0;
            bytes = 0;
        }

        if (empty || enough)
            break;
    }

    //OTT播放缓冲下溢检测
    if (sp->underflow_flag && sp->underflow_flag < 4) {
        if (enough || !empty) {
            sp->underflow_flag = 1;
        } else {
            if (1 == sp->underflow_flag) {
                if (pp->transmitDuration > 0.0) {
                    sp->underflow_flag = 2;
                    sp->underflow_time0 = timeNow;
                    sp->underflow_duration0 = pp->transmitDuration;
                }
            } else if (2 == sp->underflow_flag) {
                if (timeNow > sp->underflow_time0 + 1.0) {
                    sp->underflow_time1 = timeNow;
                    sp->underflow_duration1 = pp->transmitDuration;
                    sp->underflow_flag = 3;
                }
            } else if (3 == sp->underflow_flag) {
                if (timeNow > sp->underflow_time1 + 1.0) {
                    if (pp->transmitDuration - sp->underflow_duration0 + 0.2 < timeNow - sp->underflow_time0) {
                        LOG_STRM_PRINTF("#%d STRM_MSG_UNDERFLOW flg = %d\n", sp->leader, sp->flow_flg);
                        sp->msg_callback(sp->msg_handle, STRM_MSG_UNDERFLOW, 0);
                        sp->underflow_flag = 4;
                    } else {
                        sp->underflow_time0 = sp->underflow_time1;
                        sp->underflow_duration0 = sp->underflow_duration1;
                        sp->underflow_time1 = timeNow;
                        sp->underflow_duration1 = pp->transmitDuration;
                    }
                }
            }
        }
    }
}

static void int_play_push_pip(StreamPlay* sp)
{
    char *buf, *decode_buf;
    int len, bytes, empty, decode_len;

    StrmBuffer *sb;
    StrmBufPlay *sbp = sp->sbp;

    empty = 0;
    bytes = 0;
    decode_len = 0;
    while (0 == empty) {
        if (decode_len <= 0) {
            codec_buf_get(sp->codec, &decode_buf, &decode_len);
            if (decode_len <= 0)
                break;
            if (0 == sp->decode_valid) {
                LOG_STRM_PRINTF("#%d decode_size = %d!\n", sp->leader, decode_len);
                sp->decode_valid = 1;
            }
            decode_len = decode_len - decode_len % 188;
        }

        sb = sbp->sb;
        if (sb->len <= 0) {
            mid_mutex_lock(sp->data_mutex);
            if (strm_bufque_length(sbp->sbq) > 0)
                strm_bufplay_pop(sbp);
            mid_mutex_unlock(sp->data_mutex);
            sb = sbp->sb;
        }

        if (sb->len > 0) {
            len = decode_len - bytes;
            if (len > sb->len)
                len = sb->len;
            buf = sb->buf + sb->off;

            IND_MEMCPY(decode_buf + bytes, buf, len);
            bytes += len;
            sb->off += len;
            sb->len -= len;
        } else {
            empty = 1;
        }

        if (bytes > 0 && (bytes >= decode_len || empty)) {
            if (APP_TYPE_DVBS == sp->apptype && sp->iptv_flag)
                int_play_push_dvbs(sp, decode_buf, bytes);
            else
                sp->load_valid = 1;

            int_play_push_put(sp, decode_buf, bytes);
            decode_len = 0;
            bytes = 0;
        }
    }
}

static void int_play_push(StreamPlay* sp)
{
    StrmBufPlay *sbp;

    sbp = sp->sbp;
    if (sp->shift_flag)
        sbp = sp->shift_sbp;

    if (sp->ca_wait_clk) {
        if (1 == sp->ca_wait_clk) {
            if (strm_bufque_length(sbp->sbq) <= 0)
                return;

            sp->ca_wait_clk = mid_10ms( ) + g_ca_wait;//sp->clk有时不能及时跟新导致超时提前，改用用mid_10ms
            LOG_STRM_PRINTF("#%d wait for CA ......! timeout = %d, clk = %u / %u\n", sp->leader, g_ca_wait, sp->ca_wait_clk, sp->clk);
        }
        if (local_ca_check(sp) < 0) {
            LOG_STRM_ERROR("#%d CA_Uninitiated!\n", sp->leader);
            int_play_error(sp, RTSP_CODE_CA_Uninitiated);
            return;
        }
        if (sp->ca_wait_clk) {
            int_play_push_ca(sp, sbp);
            return;
        }
    }

    if (PLAY_STATE_TPLAY == sp->state) {
        int_play_push_tplay(sp, sbp);
        return;
    }

    if (sp->delay_clk > 0) {
        if (sp->delay_clk < sp->clk) {
            LOG_STRM_PRINTF("#%d delay finished\n", sp->leader);
            sp->delay_clk = 0;
        } else {
            if (sp->delay_dbg < sp->clk) {
                LOG_STRM_PRINTF("delay = %d\n", sp->clk - sp->delay_clk);
                sp->delay_dbg = sp->clk + 25;
            }
            sp->push_clk = sp->clk;
            return;
        }
    }

    if (1 == sp->codec)
        int_play_push_pip(sp);
    else
        int_play_push_normal(sp, sbp);

    if (sp->pts_clk) {
        if (0 == sp->pts_clk_base) {
            LOG_STRM_PRINTF("First data!\n");
#ifdef DEBUG_BUILD
            printf("play thread [%d] pid = %d\n", sp->leader, (uint32_t)getpid());
#endif
            sp->pts_clk = sp->clk + INTERVAL_CLK_CA_PTS;
            sp->pts_clk_base = sp->clk;
        } else if (sp->clk > sp->pts_clk) {
            int_play_pts(sp);
            if (sp->ply_pts && sp->ply_pts != sp->end_pts) {
                int clks = sp->clk - sp->pts_clk_base;
                sp->pts_clk = 0;
                sp->msg_callback(sp->msg_handle, STRM_MSG_PTS_VIEW, clks * 10);
                if (0 == sp->pic_width) {
                    sp->pic_width = codec_video_width(sp->codec);
                    LOG_STRM_PRINTF("#%d pic_width = %d\n", sp->leader, sp->pic_width);
                }
            } else {
                sp->pts_clk = sp->clk + INTERVAL_CLK_CA_PTS;;
            }
        }
    }
}

static void int_play_check(StreamPlay* sp)
{
    int length;

    if (10 == sp->end_flg || sp->isIdle)
        return;

    if (sp->shift_flag)
        length = strm_bufque_length(sp->shift_sbp->sbq);
    else
        length = strm_bufque_length(sp->sbp->sbq);

    if ((APP_TYPE_DVBS == sp->apptype && sp->iptv_flag && 0 == sp->load_valid) || (sp->push_clk + 50 < sp->clk && (length <= 0 || !sp->psi_flag))) {
        if (sp->end_flg) {
            int_play_pts(sp);

            if (sp->ply_pts != sp->end_pts || 1 == sp->end_flg) {
                sp->end_flg = 2;
                sp->end_pts = sp->ply_pts;
                sp->end_clk = sp->clk;
            } else {
                if (sp->end_flg == 2 && sp->clk - sp->end_clk >= 200) {
                    LOG_STRM_PRINTF("#%d end play! 10\n", sp->leader);
                    sp->end_flg = 10;
                    if (sp->state == PLAY_STATE_TPLAY && sp->scale < 0) {
                        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_BEGIN\n", sp->leader);
                        sp->msg_callback(sp->msg_handle, STRM_MSG_STREAM_BEGIN, 0);
                    } else {
                        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", sp->leader);
                        sp->msg_callback(sp->msg_handle, STRM_MSG_STREAM_END, 0);
                    }
                }
            }
        } else {
            if (1 == sp->data_flg) {
                sp->data_clk = sp->clk;
                sp->data_flg = 0;
                sp->data_pts = 0;
                codec_pts(sp->codec, &sp->data_pts);
            } else {
                int_play_pts(sp);
                if (sp->ply_pts != sp->data_pts) {//缓冲区数据播放完毕后再开始计算超时时间
                    if (sp->data_flg <= -3) {
                        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_RESUME\n", sp->leader);
                        sp->msg_callback(sp->msg_handle, STRM_MSG_RECV_RESUME, 0);
                    }
                    sp->data_flg = 1;
                }
                if (0 == sp->data_flg) {
                    if (sp->clk > sp->data_clk + 100) {
                        LOG_STRM_PRINTF("#%d STRM_MSG_UNDERFLOW flg = %d\n", sp->leader, sp->flow_flg);
                        if (0 == sp->underflow_flag && sp->flow_flg)
                            sp->msg_callback(sp->msg_handle, STRM_MSG_UNDERFLOW, 0);
                        sp->data_flg = -1;
                    }
                } else if (-1 == sp->data_flg) {
                    if (sp->clk > sp->data_clk + 250) {
                        LOG_STRM_PRINTF("#%d STRM_MSG_FIRST_TIMEOUT flg = %d\n", sp->leader, sp->timeout_flg);
                        if (sp->timeout_flg && sp->ps_psi.video_pid)
                            sp->msg_callback(sp->msg_handle, STRM_MSG_FIRST_TIMEOUT, 0);

                        sp->data_flg = -2;

                        if (!sp->psi_flag && sp->still_flg) {
                            codec_close(sp->codec, 2);
                            sp->still_flg = 0;
                            codec_open(sp->codec, sp->iptv_flag * g_pcr_flag);
                        }
                    }
                } else if (-2 == sp->data_flg) {
                    if (sp->clk > sp->data_clk + INTERVAL_CLK_DATA_TIMEOUT) {
                        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_TIMEOUT\n", sp->leader);
                        sp->msg_callback(sp->msg_handle, STRM_MSG_RECV_TIMEOUT, 0);

                        sp->data_flg = -3;
                    }
                } else if (-3 == sp->data_flg) {
                    if (sp->clk > sp->data_clk + INTERVAL_CLK_DATA_TIMEOUT15) {
                        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_TIMEOUT15 flg = %d\n", sp->leader, sp->timeout_flg);
                        if (sp->timeout_flg)
                            sp->msg_callback(sp->msg_handle, STRM_MSG_RECV_TIMEOUT15, 0);
                        sp->data_flg = -4;
                    }
                }
            }
        }
    } else {
        if (sp->data_flg <= -3) {
            LOG_STRM_PRINTF("#%d STRM_MSG_RECV_RESUME\n", sp->leader);
            sp->msg_callback(sp->msg_handle, STRM_MSG_RECV_RESUME, 0);
        }
        sp->data_flg = 1;
    }
}

static void int_play_task(void *arg)
{
    StreamPlay* sp = (StreamPlay*)arg;

    while (1) {
        sp->clk = mid_10ms( );

        mid_mutex_lock(sp->loop_mutex);

        if (-1 == sp->enable) {
            LOG_STRM_PRINTF("#%d ---- state = %d, still_flg = %d\n", sp->leader, sp->state, sp->still_flg);
            int_play_cls(sp, 1);
            sp->enable = 0;
        }

        if ((PLAY_STATE_PLAY == sp->state || PLAY_STATE_TPLAY == sp->state) && sp->sbp) {
            int_play_check(sp);

            if (!sp->psi_flag) {
                int psi_flag;

                mid_mutex_lock(sp->data_mutex);
                psi_flag = strm_bufplay_psi_get(sp->sbp, &sp->ts_psi);
                mid_mutex_unlock(sp->data_mutex);
                if (1 == psi_flag)
                    int_play_first(sp);
            }

            if (sp->psi_flag)
                int_play_push(sp);

            if (sp->time_clk < sp->clk && PLAY_STATE_CLOSE != sp->state) {
                int pcr_mode = sp->pcr_mode;

                if (0 != pcr_mode)
                    pcr_mode = 1;
                sp->time_clk = sp->clk + 50;

                if (!sp->psi_flag) {
                    sp->time_play = 0;
                    sp->time_diff = 0;
                    sp->time_buffer = 0;
                } else if (PLAY_STATE_TPLAY == sp->state) {
                    sp->time_play = ts_pts_time_play(sp->ts_pts, pcr_mode, sp->clk, 0);//ts_pts_time_last只有在正常播放时才有效
                    sp->time_diff = 0;
                    sp->time_buffer = 0;
                } else {
                    int_play_pts(sp);
                    if (PLAY_STATE_PLAY == sp->state && sp->iptv_flag && sp->ply_pts) {
                        if (sp->data_flg < 0) {
                            sp->ply_spy_clk = 0;
                        } else {
                            if (0 == sp->ply_spy_clk || sp->ply_spy_pts != sp->ply_pts) {
                                sp->ply_spy_clk = sp->clk;
                                sp->ply_spy_pts = sp->ply_pts;
                            } else {
                                if (sp->clk - sp->ply_spy_clk > 500) {
                                    sp->ply_spy_clk = 0;
                                    LOG_STRM_PRINTF("#%d STRM_MSG_FREEZE\n", sp->leader);
                                    sp->msg_callback(sp->msg_handle, STRM_MSG_FREEZE, 0);
                                }
                            }
                        }
                    }
                    sp->time_play = ts_pts_time_play(sp->ts_pts, pcr_mode, sp->clk, sp->ply_pts);
                    sp->time_diff = ts_pts_time_last(sp->ts_pts, pcr_mode) - sp->time_play;

                    if (sp->pp.transmitDuration > 0.0) {
                        int byteate = (int)(sp->pp.tsPacketCount * 188 / sp->pp.transmitDuration);
                        if (byteate > 0) {
                            StrmBufPlay *sbp = sp->sbp;
                            if (sp->shift_flag)
                                sbp = sp->shift_sbp;

                            mid_mutex_lock(sp->data_mutex);
                            sp->time_buffer = strm_bufque_length(sbp->sbq) * 100 / byteate;
                            mid_mutex_unlock(sp->data_mutex);
                        }
                    }
                }
            }
        }

        mid_mutex_unlock(sp->loop_mutex);

        usleep(10*1000);
    }

    return;
}

int strm_play_get_psi(StreamPlay* sp)
{
    return sp->psi_flag;
}

int strm_play_get_width(StreamPlay* sp)
{
    return sp->pic_width;
}

/*
    STRM_MSG_RECV_TIMEOUT15
    STRM_MSG_FIRST_TIMEOUT
 */
void strm_play_set_timeout(StreamPlay* sp)
{
    sp->timeout_flg = 1;
}

void strm_play_set_underflow(StreamPlay* sp)
{
    sp->underflow_flag = 1;
}

void strm_play_set_flow(StreamPlay* sp)
{
    sp->flow_flg = 1;
    sp->pic_width = 0;
}

void mid_stream_set_pcr(int flag)
{
    LOG_STRM_PRINTF("flag = %d\n", flag);
    g_pcr_flag = flag;
}

void mid_stream_cawait(int ms)
{
    LOG_STRM_PRINTF("cawait = %d\n", ms);
    g_ca_wait = ms / 10;
}

void mid_stream_vodsync(int ms)
{
    LOG_STRM_PRINTF("syncelay = %d\n", ms);
    g_vodsync_clks = ms / 10;
}

void mid_stream_voddelay(int ms)
{
    LOG_STRM_PRINTF("voddelay = %d\n", ms);
    g_voddelay_clks = ms / 10;
    if (g_voddelay_clks < DECODE_DIFF_CLKS)
        g_decodediff_clks = DECODE_DIFF_CLKS;
    else
        g_decodediff_clks = g_voddelay_clks;
}

void mid_stream_iptvdelay(int ms)
{
    LOG_STRM_PRINTF("syncelay = %d\n", ms);
    g_iptvdelay_clks = ms / 10;
}

void mid_stream_set_tsSyncNotify(unsigned int interval, TSSyncNotifyCall notifyCall)
{
    if (gTSSyncInterval) {
        if (interval > 3600 || !notifyCall)
            return;
    }
    gTSSyncClock = 0;
    gTSSyncInterval = interval * 100;
    gTSSyncNotifyCall = notifyCall;
}

