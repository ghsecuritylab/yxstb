/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#include "stream.h"
#include "config/pathConfig.h"


#ifdef INCLUDE_PVR

//#define ENABLE_SAVE_LOCAL

#define PVR_BUFFER_SEC          15//支持下载.pdf
#define INTERVAL_SYNC_REC       200//2秒
#define INTERVAL_SYNC_BUFFER    500

struct LOCAL {
    int             index;
    STRM_STATE      state;
    int             scale;

#ifdef ENABLE_SAVE_LOCAL
    FILE*           fp;
#endif

    uint32_t            id;
    char            url[STREAM_URL_SIZE];
    uint32_t            clk;
    uint32_t            clk_fast;//开始一段时间快发
    uint32_t            clk_pause;
    int             diff;

    PlayArg         arg;
    ind_tlink_t     tlink;
    StreamPlay*     strm_play;
    StreamMsgQ*     strm_msgq;

    StrmBuffer*     sb;

    mid_msgq_t      msgq;
    int             msgfd;

    int             psi_flag_play;
    int             music_flg;
    int             signal_flg;

    int             buffer_flg;
    uint32_t            buffer_clk;
    uint32_t            buffer_length;

    int             push_end;

    PvrElem_t       pvr;

    uint32_t            time_length;
    uint32_t            time_current;
    uint32_t            time_start;
    uint32_t            time_bmark;

    //approximate 近似值
    uint32_t            approx_elems;
    uint32_t            approx_length;

    int             quiet_flg;//一个录制节目，数据错误，播放静帧且PTS不变
    uint32_t            quiet_clk;
    int             quiet_time;

    uint32_t            stat_times;
    int             stat_bytes;

    int             delay_flg;
    int             delay_cmd;
    int             delay_arg;
};
typedef struct LOCAL* LOCAL_t;

typedef struct {
    char    url[STREAM_URL_SIZE];
    int     starttime;
} PvrArg;

static int local_open(struct LOCAL *local, PlayArg *arg, PvrArg *pvrarg);
static int local_close(struct LOCAL *local);
static void local_msgback(void *handle, STRM_MSG msgno, int arg);
static void local_msg(struct LOCAL *local, int code);
static void local_100ms(void *arg);
static void local_delay_timer(void *arg);

static void local_sync_time(struct LOCAL* local, int recording);

static void local_set_totaltime(struct LOCAL* local, uint32_t length);
static void local_set_currenttime(struct LOCAL* local, uint32_t current);

static void local_state(struct LOCAL *local, STRM_STATE state, int scale)
{
    local->diff = 0;
    local->state = state;
    local->scale = scale;
    stream_post_state(local->index, state, scale);
}

static void local_buffer(struct LOCAL *local)
{
    local_sync_time(local, 0);
    LOG_STRM_PRINTF("#%d STRM_MSG_BUFFER_BEGIN\n", local->index);
    stream_post_msg(local->index, STRM_MSG_BUFFER_BEGIN, 0);
    local->buffer_flg = 1;
    local->clk_pause = local->clk;
    strm_play_pause(local->strm_play, local->index);
    local->buffer_length = local->time_length;
}

static void local_buffer_fail(void *arg)
{
    struct LOCAL* local = (struct LOCAL*)arg;

    local_close(local);
    LOG_STRM_PRINTF("#%d STRM_MSG_OPEN_ERROR\n", local->index);
    stream_post_msg(local->index, STRM_MSG_OPEN_ERROR, PVR_CODE_BUFFER_FAILED);
}

static void local_sync_end(struct LOCAL* local)
{
    strm_play_reset(local->strm_play, local->index, 1);
    strm_play_set_idle(local->strm_play, local->index, 1);

    ind_pvr_play(local->pvr, local->time_length, 1);
    local->push_end = 1;

    LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", local->index);
    local_msg(local, STRM_MSG_STREAM_END);
}

static void local_sync_time(struct LOCAL* local, int recording)
{
    uint32_t sec;

    if (local->state != STRM_STATE_PLAY && local->state != STRM_STATE_FAST) {
        LOG_STRM_DEBUG("#%d: state = %d\n", local->index, local->state);
        return;
    }

    {
        int t = (int)local->time_start + strm_play_time(local->strm_play) / 100;
        if (t < 0)
            sec = 0;
        else
            sec = (uint32_t)t;
    }

    if (sec >= local->time_length) {
        sec = local->time_length;

        if (0 == recording && 0 == local->push_end && local->scale > 0) {
            LOG_STRM_WARN("#%d: time_length = %d, time_start = %d\n", local->index, local->time_length, local->time_start);
            local_sync_end(local);
            return;
        }
    }

    local->diff = strm_play_diff(local->strm_play);
    LOG_STRM_DEBUG("#%d: time_length = %d, scale = %d, start = %d, sec = %d, diff = %d\n", local->index, local->time_length, local->scale, local->time_start, sec, local->diff);

    if (local->push_end) {
        local_set_currenttime(local, local->time_length);
    } else {
        if (0 == recording && local->scale > 0 && 0 == local->buffer_flg && local->quiet_flg >= 0 && sec == local->quiet_time) {//容错处理
            int clks = local->clk - local->quiet_clk;
            local->quiet_flg ++;

            if (clks < 190)//延迟两秒
                clks = 0;
            else
                clks -= 190;
            sec = local->quiet_time + clks * local->scale / 100;
            if (local->quiet_flg >= 3 && sec >= local->time_length) {
                local_set_currenttime(local, local->time_length);

                LOG_STRM_WARN("#%d: time_length = %d, quiet_time = %d, sec = %d\n", local->index, local->time_length, local->quiet_time, sec);
                local_sync_end(local);
                return;
            }
            if (local->quiet_flg > 1 && (local->scale > 1 || local->quiet_flg > 2)) {
                if (sec >= local->time_length)
                    sec = local->time_length;
                LOG_STRM_WARN("#%d: time_length = %d, quiet_time = %d, sec = %d\n", local->index, local->time_length, local->quiet_time, sec);
                local_set_currenttime(local, (uint32_t)sec);
            }
        } else {
            local->quiet_flg = 0;
            local->quiet_clk = local->clk;
            local->quiet_time = sec;

            local_set_currenttime(local, (uint32_t)sec);
        }
    }
}

static void local_1000ms(void *arg)
{
    int recording;
    struct LOCAL* local = (struct LOCAL*)arg;

    recording = record_back_exist(local->id);
    if (recording) {
        struct PVRInfo info;

        if (ind_pvr_get_info(local->id, &info)) {
            LOG_STRM_ERROR("#%d ind_pvr_get_info\n", local->index);
            local_close(local);
            LOG_STRM_PRINTF("#%d STRM_MSG_OPEN_ERROR\n", local->index);
            stream_post_msg(local->index, STRM_MSG_OPEN_ERROR, 0);
            return;
        }

        local_set_totaltime(local, info.time_len);

        if (local->buffer_flg)
            LOG_STRM_DEBUG("#%d time_length = %d, buffer_length = %d\n", local->index, local->time_length, local->buffer_length);
    }

    if (local->buffer_flg && (0 == recording || local->time_length >= local->buffer_length + PVR_BUFFER_SEC)) {
        LOG_STRM_PRINTF("#%d STRM_MSG_BUFFER_END recording = %d\n", local->index, recording);
        stream_post_msg(local->index, STRM_MSG_BUFFER_END, 0);
        local->buffer_flg = 0;
        if (STRM_STATE_PLAY == local->state)
            strm_play_resume(local->strm_play, local->index, 0);

        if (STRM_STATE_OPEN == local->state)
            local_open(local, NULL, NULL);
    }

    if (STRM_STATE_OPEN != local->state)
        local_sync_time(local, recording);
}

static int local_open(struct LOCAL *local, PlayArg *arg, PvrArg *pvrarg)
{
    uint32_t id;
    int msgarg, announce, recording;
    struct PVRInfo info;

    msgarg = 0;
    local->state = STRM_STATE_OPEN;

    local->music_flg = -1;
    local->quiet_flg = -1;

    ind_timer_create(local->tlink, local->clk + INTERVAL_CLK_1000MS, INTERVAL_CLK_1000MS, local_1000ms, local);

    if (arg) {
        IND_STRCPY(local->url, pvrarg->url);
        memcpy(&local->arg, arg, sizeof(PlayArg));
    } else {
        arg = &local->arg;
    }
    if (ind_str8tohex(local->url, &id))
        LOG_STRM_ERROUT("#%d ind_str8tohex\n", local->index);
    recording = record_back_exist(local->id);
    LOG_STRM_PRINTF("#%d id = 0x%x recording = %d\n", local->index, id, recording);

    if (arg != &local->arg) {
        local->id = id;

        if (pvrarg->starttime < 0)
            local->time_start = 0;
        else
            local->time_start = pvrarg->starttime;

        if (ind_pvr_get_info(id, &info)) {
            if (record_back_wait(local->index, id))
                LOG_STRM_ERROUT("#%d record_back_wait\n", local->index);
            ind_timer_create(local->tlink, local->clk + INTERVAL_SYNC_BUFFER, 0, local_buffer_fail, local);
            LOG_STRM_PRINTF("#%d STRM_MSG_BUFFER_BEGIN recording = %d\n", local->index, recording);
            stream_post_msg(local->index, STRM_MSG_BUFFER_BEGIN, 0);
            local->buffer_flg = 1;
            local->buffer_length = 0;
            goto End;
        }

        if (2 == info.recording && 0 == info.time_bmark && info.time_len < info.time_length && info.time_len + 5 >= info.time_length - info.time_length / 100) {
            local->approx_length = info.time_length;
            local->approx_elems = info.time_len / (info.time_length - info.time_len);
            LOG_STRM_PRINTF("#%d time_len = %d, approx_length = %d, approx_elems = %d\n", local->index, info.time_len, local->approx_length, local->approx_elems);
        }

        announce = ind_pvr_open(id, &local->pvr);
        if (announce) {
            if (announce == PVR_ANNOUNCE_LOST)
                msgarg = PVR_CODE_NOT_FOUND;
            LOG_STRM_ERROUT("#%d ind_pvr_open\n", local->index);
        }
    } else {
        if (ind_pvr_get_info(id, &info))
            LOG_STRM_ERROUT("#%d ind_pvr_get_info\n", local->index);

        announce = ind_pvr_open(id, &local->pvr);
        if (announce) {
            if (announce == PVR_ANNOUNCE_LOST)
                msgarg = PVR_CODE_NOT_FOUND;
            LOG_STRM_ERROUT("#%d ind_pvr_open\n", local->index);
        }
    }
    local->time_bmark = info.time_bmark;

    LOG_STRM_PRINTF("#%d time_start = %d, time_bmark = %d\n", local->index, local->time_current, local->time_bmark);
    if (local->time_start > 0 && local->time_bmark > 0) {
        if (local->time_start > local->time_bmark)
            local->time_start -= local->time_bmark;
        else
            local->time_start = 0;
    }
    local_set_totaltime(local, info.time_len);
    local_set_currenttime(local, local->time_start);

    if (ind_pvr_play(local->pvr, local->time_current, 1))
        LOG_STRM_ERROUT("#%d ind_pvr_play\n", local->index);
    local->time_start = local->time_current;

    strm_play_open(local->strm_play, local->index, local, local_msgback, APP_TYPE_PVR, TS_TIMESHIFT_SIZE);

    if (info.networkid) {
        DvbCaParam ca_param;
        memset(&ca_param, 0, sizeof(ca_param));
        ca_param.networkid = info.networkid;

        if (STREAM_INDEX_PIP == local->index)
            codec_dvbs(1, &ca_param);
        else
            codec_dvbs(0, &ca_param);
    }

    strm_play_resume(local->strm_play, local->index, 0);

#ifdef ENABLE_SAVE_LOCAL
    local->fp = fopen(DEFAULT_EXTERNAL_DATAPATH"/local.ts", "wb");
    LOG_STRM_PRINTF("@@@@@@@@: fp = %p\n", local->fp);
#endif

    local_state(local, STRM_STATE_PLAY, 1);

    if (recording && local->time_length < PVR_BUFFER_SEC) {
        if (local->buffer_flg == 0) {
            LOG_STRM_PRINTF("#%d STRM_MSG_BUFFER_BEGIN\n", local->index);
            stream_post_msg(local->index, STRM_MSG_BUFFER_BEGIN, 0);
            local->buffer_flg = 1;
            local->clk_pause = local->clk;
            strm_play_pause(local->strm_play, local->index);
            local->buffer_length = 0;
        }
        goto End;
    }

    if (local->buffer_flg) {
        LOG_STRM_PRINTF("#%d STRM_MSG_BUFFER_END recording = %d\n", local->index, recording);
        stream_post_msg(local->index, STRM_MSG_BUFFER_END, 0);
        local->buffer_flg = 0;
    }

End:
    local->stat_times = 0;
    local->stat_bytes = 0;
    ind_timer_create(local->tlink, local->clk + INTERVAL_CLK_100MS, INTERVAL_CLK_100MS, local_100ms, local);
    ind_timer_create(local->tlink, local->clk + INTERVAL_CLK_1000MS, INTERVAL_CLK_1000MS, local_1000ms, local);

    return 0;
Err:
    LOG_STRM_PRINTF("#%d STRM_MSG_OPEN_ERROR\n", local->index);
    stream_post_msg(local->index, STRM_MSG_OPEN_ERROR, msgarg);
    local_close(local);
    return -1;
}

static int local_pause(struct LOCAL *local)
{
    LOG_STRM_PRINTF("#%d\n", local->index);

    switch(local->state) {
    case STRM_STATE_PLAY:
        break;
    case STRM_STATE_FAST:
        local_sync_time(local, 0);
        local->time_start = local->time_current;
        strm_play_reset(local->strm_play, local->index, 1);
        if (ind_pvr_play(local->pvr, -1, 1))
            LOG_STRM_ERROUT("#%d ind_pvr_play\n", local->index);
        break;
    default:
        LOG_STRM_ERROUT("#%d state = %d\n", local->index, local->state);
    }

    if (!local->buffer_flg) {
        local->clk_pause = local->clk;
        strm_play_pause(local->strm_play, local->index);
    }
    local_state(local, STRM_STATE_PAUSE, 0);

    return 0;
Err:
    stream_post_state(local->index, local->state, local->scale);
    return -1;
}

static int local_resume(struct LOCAL *local)
{
    LOG_STRM_PRINTF("#%d state = %d\n", local->index, local->state);

    switch(local->state) {
    case STRM_STATE_PAUSE:
        break;
    case STRM_STATE_FAST:
        local_sync_time(local, 0);
        local->time_start = local->time_current;
        strm_play_reset(local->strm_play, local->index, 1);
        if (ind_pvr_play(local->pvr, -1, 1))
            LOG_STRM_ERROUT("#%d ind_pvr_play\n", local->index);
        break;
    default:
        LOG_STRM_ERROUT("#%d %d\n", local->index, local->state);
    }

    if (!local->buffer_flg)
        strm_play_resume(local->strm_play, local->index, 0);

    local_state(local, STRM_STATE_PLAY, 1);

    local->push_end = 0;

    return 0;
Err:
    stream_post_state(local->index, local->state, local->scale);
    return -1;
}

static int local_fast(struct LOCAL *local, int scale)
{
    int ret;
    uint32_t off;
    LOG_STRM_PRINTF("#%d\n", local->index);

    switch(local->state) {
    case STRM_STATE_PLAY:   break;
    case STRM_STATE_PAUSE:  break;
    case STRM_STATE_FAST:   break;
    default:                LOG_STRM_ERROUT("#%d %d\n", local->index, local->state);
    }

    local_sync_time(local, 0);

    off = local->time_current;
    {//避免画面回退
        LOG_STRM_PRINTF("#%d scale = %d, off = %d, time_len = %d\n", local->index, scale, off, local->time_length);
        if (scale < 0 && off > 0)
            off --;
        if (scale > 0 && off < local->time_length)
            off ++;
        LOG_STRM_PRINTF("#%d off = %d\n", local->index, off);
    }
    ret = ind_pvr_play(local->pvr, off, scale);
    if (ret) {
        if (ret == PVR_ANNOUNCE_LOST_INDEX)
            stream_post_msg(local->index, STRM_MSG_INDEX_DAMAGE, 0);
        LOG_STRM_ERROUT("#%d ind_pvr_play\n", local->index);
    }
    local->time_start = local->time_current;

    if (local->buffer_flg) {
        LOG_STRM_PRINTF("#%d STRM_MSG_BUFFER_END\n", local->index);
        stream_post_msg(local->index, STRM_MSG_BUFFER_END, 0);
        local->buffer_flg = 0;
    }
    local->quiet_flg = -1;
    strm_play_reset(local->strm_play, local->index, 0);
    strm_play_tplay(local->strm_play, local->index, scale);
    local_state(local, STRM_STATE_FAST, scale);

    local->push_end = 0;

    return 0;
Err:
    stream_post_state(local->index, local->state, local->scale);
    return -1;
}

static int local_seek(struct LOCAL *local, int offset, int cmd)
{
    int ret, approx_length, approx_elems;
    LOG_STRM_PRINTF("#%d offset = %d\n", local->index, offset);

    switch(local->state) {
    case STRM_STATE_PLAY:   break;
    case STRM_STATE_PAUSE:  break;
    case STRM_STATE_FAST:   break;
    default:                LOG_STRM_ERROUT("#%d %d\n", local->index, local->state);
    }

    approx_elems = local->approx_elems;
    approx_length = local->approx_length;
    if (approx_length > 0) {
        int off0, off1;

        off0 = offset;
        offset = (long long)off0 * approx_elems / (approx_elems + 1);

        //避免经换算后，比原先值小 2012-5-11 1:00:34 by 柳建华
        off1 = offset + offset / approx_elems;
        if (off1 < off0) {
            int i, max;

            max = off0 - off1;
            for (i = 1; i <= max; i ++) {//避免死循环
                offset ++;
                off1 = offset + offset / approx_elems;
                if (off1 >= off0)
                    break;
            }
        }
        LOG_STRM_PRINTF("#%d approx offset = %d / (%d, %d)\n", local->index, offset, off0, off1);
    }

    if (offset <= 0) {
        if (cmd)
            stream_post_msg(local->index, STRM_MSG_SEEK_BEGIN, 0);
    } else if (offset >= (int)local->time_length) {
        stream_post_msg(local->index, STRM_MSG_SEEK_END, 0);
    }

    if (offset < 0 || offset >= (int)local->time_length) {
        LOG_STRM_WARN("#%d offset = %d / length = %d\n", local->index, offset, local->time_length);
        if (offset < 0 || local->time_length <= 0)
            offset = 0;
        else
            offset = (int)local->time_length - 1;
    }

    ret = ind_pvr_play(local->pvr, offset, 1);
    if (ret) {
        if (ret == PVR_ANNOUNCE_LOST_INDEX)
            stream_post_msg(local->index, STRM_MSG_INDEX_DAMAGE, 0);
        LOG_STRM_ERROUT("#%d ind_pvr_play\n", local->index);
    }
    local->time_start = offset;
    local_set_currenttime(local, offset);

    if (local->buffer_flg) {
        LOG_STRM_PRINTF("#%d STRM_MSG_BUFFER_END\n", local->index);
        stream_post_msg(local->index, STRM_MSG_BUFFER_END, 0);
        local->buffer_flg = 0;
    }

    local->quiet_flg = -1;
    strm_play_reset(local->strm_play, local->index, 1);
    strm_play_resume(local->strm_play, local->index, 0);
    local_state(local, STRM_STATE_PLAY, 1);

    local->push_end = 0;

    return 0;
Err:
    return -1;
}

static int local_close(struct LOCAL *local)
{
    LOG_STRM_PRINTF("#%d\n", local->index);

    if (local->buffer_flg) {
        LOG_STRM_PRINTF("#%d STRM_MSG_BUFFER_END\n", local->index);
        stream_post_msg(local->index, STRM_MSG_BUFFER_END, 0);
        local->buffer_flg = 0;
    }

#ifdef ENABLE_SAVE_LOCAL
    if (local->fp) {
        fclose(local->fp);
        local->fp = NULL;
    }
#endif

    if (local->pvr) {
        ind_pvr_close(local->pvr);
        local->pvr = NULL;
    }
    if (local->state != STRM_STATE_OPEN)
        strm_play_close(local->strm_play, local->index, 1);

    ind_timer_delete(local->tlink, local_100ms, local);
    ind_timer_delete(local->tlink, local_delay_timer, local);
    local_state(local, STRM_STATE_CLOSE, 0);


    return 0;
}

static void local_msgback(void *handle, STRM_MSG msgno, int arg)
{
    struct LOCAL *local = (struct LOCAL *)handle;

    strm_msgq_queue(local->strm_msgq, msgno, arg);
}

static void local_msg(struct LOCAL *local, int code)
{
    LOG_STRM_PRINTF("#%d code = %d\n", local->index, code);

    switch(code) {
    case STRM_MSG_OPEN_ERROR:
    case STRM_MSG_PLAY_ERROR:
        LOG_STRM_PRINTF("#%d STRM_MSG_PLAY_ERROR > STRM_MSG_OPEN_ERROR music_flg = %d\n", local->index, local->music_flg);
        stream_post_msg(local->index, STRM_MSG_OPEN_ERROR, 0);
        local_close(local);
        break;

    case STRM_MSG_STREAM_BEGIN:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_BEGIN\n", local->index);
        local_seek(local, 0, 0);
        stream_post_msg(local->index, STRM_MSG_STREAM_BEGIN, 0);
        break;

    case STRM_MSG_STREAM_END:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", local->index);
        stream_post_msg(local->index, STRM_MSG_STREAM_END, 0);
        LOG_STRM_DEBUG("#%d time_current = %d\n", local->index, local->time_current);
        local_set_currenttime(local, local->time_length);
        break;

    case STRM_MSG_PTS_VIEW:
        LOG_STRM_PRINTF("#%d STRM_MSG_PTS_VIEW\n", local->index);
        stream_post_msg(local->index, STRM_MSG_PTS_VIEW, 0);
        break;
    case STRM_MSG_STREAM_MUSIC:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_MUSIC music_flg = %d\n", local->index, local->music_flg);
        if (1 != local->music_flg) {
            if (-1 == local->music_flg)
                stream_post_msg(local->index, STRM_MSG_STREAM_VIEW, 0);
            stream_post_msg(local->index, STRM_MSG_STREAM_MUSIC, 0);
            local->music_flg = 1;
        }
        if (local->state == STRM_STATE_FAST)
            local_resume(local);
        break;
    case STRM_MSG_STREAM_VIDEO:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_VIDEO music_flg = %d\n", local->index, local->music_flg);
        if (0 != local->music_flg) {
            if (-1 == local->music_flg)
                stream_post_msg(local->index, STRM_MSG_STREAM_VIEW, 0);
            stream_post_msg(local->index, STRM_MSG_STREAM_VIDEO, 0);
            local->music_flg = 0;
        }
        break;
    case STRM_MSG_RECV_FIRST:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_FIRST\n", local->index);
        break;
    case STRM_MSG_RECV_TIMEOUT:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_TIMEOUT\n", local->index);
        stream_post_msg(local->index, STRM_MSG_RECV_TIMEOUT, 0);
        break;
    case STRM_MSG_RECV_RESUME:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_RESUME\n", local->index);
        stream_post_msg(local->index, STRM_MSG_RECV_RESUME, 0);
        break;

    case STRM_MSG_PSI_ERROR:
        LOG_STRM_PRINTF("#%d STRM_MSG_PSI_ERROR\n", local->index);
        stream_post_msg(local->index, STRM_MSG_PSI_ERROR, 0);
        break;
    case STRM_MSG_PSI_RESUME:
        LOG_STRM_PRINTF("#%d STRM_MSG_PSI_RESUME\n", local->index);
        stream_post_msg(local->index, STRM_MSG_PSI_RESUME, 0);
        break;

    default:
        LOG_STRM_ERROUT("#%d code = %d\n", local->index, code);
    }

Err:
    return;
}

static void local_trickmode(struct LOCAL* local, int cmd, int arg)
{
    if (STREAM_CMD_SEEK == cmd || STREAM_CMD_STOP == cmd) {
        if (local->delay_flg) {
            ind_timer_delete(local->tlink, local_delay_timer, local);
            local->delay_flg = 0;
        }

        if (STREAM_CMD_STOP == cmd) {
            int seek;

            if (local->approx_length > 0)
                seek = local->approx_length;
            else
                seek = local->time_length;
            if (record_back_exist(local->id)) {
                if (seek > PVR_BUFFER_SEC)
                    seek -= PVR_BUFFER_SEC;
                else
                    seek = 0;
            } else {
                if (seek > 3)
                    seek -= 3;
                else
                    seek = 0;
            }

            LOG_STRM_PRINTF("#%d STREAM_CMD_STOP approx_length = %d, time_length = %d, seek = %d\n", local->index, local->approx_length, local->time_length, seek);
            local_seek(local, seek, 1);
        } else {
            LOG_STRM_PRINTF("#%d STREAM_CMD_SEEK\n", local->index);
            if (local->time_bmark > 0) {
                if (arg < local->time_bmark)
                    arg = 0;
                else
                    arg -= local->time_bmark;
            }
            local_seek(local, arg, 1);
        }
    } else {
        if (local->delay_flg == 0 || local->delay_flg == 3) {
            switch(cmd) {
            case STREAM_CMD_PAUSE:
                LOG_STRM_PRINTF("#%d STREAM_CMD_PAUSE\n", local->index);
                local_pause(local);
                break;
            case STREAM_CMD_RESUME:
                LOG_STRM_PRINTF("#%d STREAM_CMD_RESUME\n", local->index);
                local_resume(local);
                break;
            case STREAM_CMD_FAST:
                LOG_STRM_PRINTF("#%d STREAM_CMD_FAST\n", local->index);
                local_fast(local, arg);
                break;
            default:
                break;
            }
            if (local->delay_flg == 0) {
                local->delay_flg = 1;
                ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_1000MS, 0, local_delay_timer, local);
            }
        } else {
            switch(cmd) {
            case STREAM_CMD_RESUME:
                LOG_STRM_PRINTF("#%d STREAM_CMD_RESUME_\n", local->index);
                stream_post_state(local->index, STRM_STATE_PLAY, 1);
                break;

            case STREAM_CMD_PAUSE:
                LOG_STRM_PRINTF("#%d STREAM_CMD_PAUSE_\n", local->index);
                stream_post_state(local->index, STRM_STATE_PAUSE, 0);
                break;

            case STREAM_CMD_FAST:
                LOG_STRM_PRINTF("#%d STREAM_CMD_FAST_\n", local->index);
                stream_post_state(local->index, STRM_STATE_FAST, arg);
                break;

            default:
                break;
            }

            local->delay_flg = 2;
            local->delay_cmd = cmd;
            local->delay_arg = arg;
            ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_1000MS, 0, local_delay_timer, local);
        }
    }
}

static void local_delay_timer(void *arg)
{
    struct LOCAL* local = (struct LOCAL*)arg;

    if (local->delay_flg == 2) {
        local->delay_flg = 3;
        local_trickmode(local, local->delay_cmd, local->delay_arg);
    }
    local->delay_flg = 0;
}

static void local_cmd(struct LOCAL* local, StreamCmd* strmCmd)
{
    int cmd = strmCmd->cmd;

    LOG_STRM_PRINTF("#%d %d %d\n", local->index, local->state, cmd);

    if (stream_deal_cmd(local->index, strmCmd) == 1)
        return;

    //处理可能需要延迟处理的命令
    switch(cmd) {
    case STREAM_CMD_WAKE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_WAKE\n", local->index);
        if (local->buffer_flg == 0) {
            LOG_STRM_WARN("#%d buffer_flg = %d\n", local->index, local->buffer_flg);
            break;
        }

        ind_timer_delete(local->tlink, local_buffer_fail, local);
        if (strmCmd->arg0 == local->id && local->state == STRM_STATE_OPEN)
            break;

        local_close(local);
        LOG_STRM_PRINTF("#%d STRM_MSG_OPEN_ERROR\n", local->index);
        stream_post_msg(local->index, STRM_MSG_OPEN_ERROR, 0);
        break;
    case STREAM_CMD_RESUME:
    case STREAM_CMD_PAUSE:
    case STREAM_CMD_FAST:
    case STREAM_CMD_SEEK:
    case STREAM_CMD_STOP:

        if (STRM_STATE_OPEN == local->state)
            LOG_STRM_ERROUT("#%d cmd = %d, buffering!\n", local->index, cmd);

        {
            int cmdsn = strmCmd->arg3;

            local_trickmode(local, cmd, strmCmd->arg0);

            if (cmdsn)
                stream_back_cmd(local->index, cmdsn);
        }
        break;

    case STREAM_CMD_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE\n", local->index);
        local_close(local);
        return;

    default:
        LOG_STRM_ERROUT("#%d Unkown CMD %d\n", local->index, cmd);
    }

Err:
    return;
}

static void local_100ms(void *arg)
{
    int len;
    uint32_t clk;
    StrmBuffer* sb;
    struct LOCAL *local = (struct LOCAL *)arg;

    local->stat_times ++;

    if (local->stat_times >= 50) {
        int space, length;
        space = strm_play_space(local->strm_play);
        length = strm_play_length(local->strm_play);
        LOG_STRM_PRINTF("#%d buffer = %d, state = %d, diff = %d, end = %d, length = %d / %d, bytes = %d\n", local->index, local->buffer_flg, local->state, local->diff, local->push_end, length, space, local->stat_bytes);
        local->stat_times = 0;
        local->stat_bytes = 0;
    }

    if (local->buffer_flg)
        return;
    if (local->state != STRM_STATE_PLAY && local->state != STRM_STATE_FAST)
        return;

    if (strm_msgq_valid(local->strm_msgq))
        return;
    if (local->push_end)
        return;

    clk = local->clk;
    //strm_buf_print(sb);

    for (;;) {
        if (strm_play_space(local->strm_play) < TS_TIMESHIFT_SIZE)
            break;

        sb = local->sb;
        if (STRM_STATE_PLAY == local->state)
            len = ind_pvr_read(local->pvr, clk * 10 / 9, sb->buf, TS_TIMESHIFT_SIZE);//加速取数据
        else
            len = ind_pvr_read(local->pvr, clk, sb->buf, TS_TIMESHIFT_SIZE);

        if (len > 0) {
            local->stat_bytes += len;

#ifdef ENABLE_SAVE_LOCAL
            if (local->fp) {
                if (fwrite(sb->buf, 1, len, local->fp) != len)
                    LOG_STRM_WARN("#%d fwrite\n", local->index);
            }
#endif
            sb->off = 0;
            sb->len = len;
            strm_play_push(local->strm_play, local->index, &local->sb);

            if (strm_msgq_valid(local->strm_msgq))
                goto End;

            continue;
        }

        switch(len) {
        case PVR_ANNOUNCE_NONE:
            //LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_NONE\n", local->index);
            goto End;
        case PVR_ANNOUNCE_WRITE:
            if (local->scale != 1) {
                LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_WRITE scale = %d\n", local->index, local->scale);
                local_resume(local);
            } else {
                LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_WRITE\n", local->index);
                local_buffer(local);
            }
            goto End;
        case PVR_ANNOUNCE_BEGIN:
            LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_BEGIN\n", local->index);
            LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_BEGIN\n", local->index);
            local_msgback(local, STRM_MSG_STREAM_BEGIN, 0);
            goto End;
        case PVR_ANNOUNCE_END:
            LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_END scale = %d\n", local->index, local->scale);
            if (local->scale != 1) {
                strm_play_set_idle(local->strm_play, local->index, 1);
                LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", local->index);
                local_msgback(local, STRM_MSG_STREAM_END, 0);
            } else {
                strm_play_end(local->strm_play, local->index);
            }
            local->push_end = 1;
            goto End;
        case PVR_ANNOUNCE_ERROR:
            LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_ERROR\n", local->index);
            local_sync_time(local, 0);
            local_msg(local, STRM_MSG_OPEN_ERROR);
            goto End;
        default:
            LOG_STRM_PRINTF("#%d ret = %d\n", local->index, len);
            goto End;
        }
    }
End:
    {
        uint32_t diff = mid_10ms( ) - clk;
        if (diff > 50)
            LOG_STRM_WARN("#%d diff = %d\n", local->index, diff);
    }
}


static void local_loop(struct LOCAL* local)
{
    uint32_t            clk, clks, out;
    fd_set            rset;
    struct timeval    tv;

    while (local->state != STRM_STATE_CLOSE) {

        clk = mid_10ms( );
        local->clk = clk;
        out = ind_timer_clock(local->tlink);
        if (out <= clk) {
            ind_timer_deal(local->tlink, clk);
            continue;
        }
        if (strm_msgq_valid(local->strm_msgq)) {
            StreamMsg msg;
            strm_msgq_print(local->strm_msgq);
            if (strm_msgq_pump(local->strm_msgq, &msg) == 1)
                local_msg(local, msg.msg);
            continue;
        }

        clks = out - clk;
        tv.tv_sec = clks / 100;
        tv.tv_usec = clks % 100 * 10000;

        FD_ZERO(&rset);
        FD_SET((uint32_t)local->msgfd, &rset);
        if (select(local->msgfd + 1, &rset , NULL,  NULL, &tv) != 1)
            continue;

        if (FD_ISSET((uint32_t)local->msgfd, &rset)) {
            StreamCmd strmCmd;

            memset(&strmCmd, 0, sizeof(strmCmd));
            mid_msgq_getmsg(local->msgq, (char *)(&strmCmd));
            local_cmd(local, &strmCmd);
        }
    }
    ind_timer_delete_all(local->tlink);
}

static LOCAL_t local_create(int idx, mid_msgq_t msgq)
{
    LOCAL_t local = (LOCAL_t)IND_CALLOC(sizeof(struct LOCAL), 1);
    if (local == NULL)
        LOG_STRM_ERROUT("malloc failed!\n");

    local->state = STRM_STATE_CLOSE;

    local->index = idx;
    local->msgq = msgq;
    local->msgfd = mid_msgq_fd(local->msgq);
    local->tlink = int_stream_tlink(idx);
    local->strm_play = int_strm_play(idx);
    local->strm_msgq = int_strm_msgq(idx);

    local->buffer_flg = 0;

    local->clk = mid_10ms( );

    local->sb = strm_buf_malloc(TS_TIMESHIFT_SIZE);

    return local;
Err:
    return NULL;
}

static void local_delete(struct LOCAL *local)
{
    if (local == NULL)
        return;

    strm_buf_free(local->sb);
    IND_FREE(local);
}

static void local_loop_play(void *handle, int idx, mid_msgq_t msgq, PlayArg *arg, char *argbuf)
{
    LOCAL_t local = local_create(idx, msgq);
    if (local == NULL)
        LOG_STRM_ERROUT("#%d local is NULL\n", idx);

    if (local_open(local, arg, (PvrArg *)argbuf))
        LOG_STRM_ERROUT("#%d pvr_recplay_local_open\n", idx);

    local_loop(local);
    LOG_STRM_PRINTF("#%d exit local_loop!\n", idx);

Err:
    local_delete(local);
    return;
}

static void local_set_totaltime(struct LOCAL* local, uint32_t length)
{
    local->time_length = length;

    if (local->approx_length > 0) {
        length = local->approx_length;
    } else {
        if (local->time_bmark > 0)
        length += local->time_bmark;
    }
    stream_back_totaltime(local->index, length);
}

static void local_set_currenttime(struct LOCAL* local, uint32_t current)
{
    local->time_current = current;
    if (local->approx_length > 0) {
        if (current >= local->time_length)
            current = local->approx_length;
        else
            current = current + current / local->approx_elems;
    } else {
        if (local->time_bmark > 0)
            current += local->time_bmark;
    }
    stream_back_currenttime(local->index, current);
}

static int local_argparse_play(int idx, PlayArg* arg, char *argbuf, const char* url, int shiftlen, int begin, int end)
{
    PvrArg *pvrarg = (PvrArg *)argbuf;

    if (url == NULL)
        LOG_STRM_ERROUT("#%d url is NULL\n", idx);

    LOG_STRM_PRINTF("#%d url = %s\n", idx, url);

    IND_STRCPY(pvrarg->url, url);
    pvrarg->starttime = shiftlen;

    return 0;
Err:
    return -1;
}

int pvr_create_stream(StreamCtrl *ctrl)
{
    ctrl->handle = ctrl;

    ctrl->loop_play = local_loop_play;

    ctrl->argsize = sizeof(PvrArg);
    ctrl->argparse_play = local_argparse_play;

    return 0;
}

#endif//INCLUDE_PVR
