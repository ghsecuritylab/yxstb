
/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#ifdef INCLUDE_DVBS

#include "stream.h"

#include "dvbs_port.h"
#include "config/pathConfig.h"

#define PVR_FAST_CLK            2000
#define DVBS_PRINT_CLK          500
#define DVBS_SHIFT_CLK          800

#define DVBS_INSTANCE_NUM       4

struct __DVBS {
    int             index;
    STRM_STATE      state;
    int             scale;
    int             tuner;
    int             diff;

    uint32_t        clk;
    uint32_t        clk_fast;
    uint32_t        clk_pause;

    FILE*           fp;
    int             fp_sn;

    int             rec_index;
    uint32_t        rec_time;

    int             open_play;
    int             open_shift;
    int             open_filter;

    uint32_t        shift_id;
	RecordMix_t     rec_mix;

    ind_tlink_t     tlink;

    char            dvbs_url[STREAM_URL_SIZE];
    mid_mutex_t     dvbs_mutex;
    uint32_t        dvbs_magic;

    StreamPlay*     strm_play;
    StreamRecord*   strm_record;

    StreamMsgQ*     strm_msgq;

    mid_msgq_t      msgq;
    int             msgfd;

    int             music_flg;

    int             push_end;

    uint32_t        print_clk;
    uint32_t        write_clk;
    uint32_t        write_pmts;
    uint32_t        write_times;

    uint32_t        stat_clk;
    int             stat_bytes;

    StrmBuffer*     sb;

    PvrElem_t       pvr;
    StrmBuffer*     pvr_sb;
    struct PVRInfo  pvr_info;

    int             pvr_smooth;
    int             pvr_smooth_clk;
    int             pvr_smooth_pause;//从直播到暂停继续给解码器送数据

    uint32_t        shiftlen;

    uint32_t        pat_counter;

    DvbCaParam      ca_param;

    char            pat_buf[188];

    uint32_t        time_length;
    uint32_t        time_start;
    uint32_t        time_current;

    int             emm_num;
    int             emm_pid[TS_EMM_NUM];

    int             delay_flg;
    int             delay_cmd;
    int             delay_arg;
};

typedef struct __DVBS   DVBS;
typedef struct __DVBS*  DVBS_t;

typedef struct {
    char    url[STREAM_URL_SIZE];
    int     tuner;
    uint32_t    shiftlen;
} DvbsArg;

static int g_inited = 0;
static int* g_tunerArray = NULL;

static DVBS_t g_array[DVBS_INSTANCE_NUM] = {NULL, NULL, NULL, NULL};

#ifdef INCLUDE_PVR
static void local_record_end(DVBS_t dvbs, uint32_t id, uint32_t endtime);
static void local_record_close(DVBS_t dvbs, uint32_t id, int end);

static void local_delay_timer(void *arg);

static int local_close_timeshift(DVBS_t dvbs);

static int local_seek(DVBS_t dvbs, uint32_t offset);
static int local_stop(DVBS_t dvbs);
#endif

static void dvbs_callback_writepmt(uint32_t magic, DvbCaParam_t param);
static void dvbs_callback_writedata(uint32_t magic, uint8_t *data_buf, int data_len);
static void local_msg(DVBS_t dvbs, STRM_MSG msgno, int arg);
static void local_msg_back(void *handle, STRM_MSG msgno, int arg);
static void local_push(void *arg);

static void local_set_recordtime(DVBS_t dvbs, uint32_t rectime);
static void local_set_currenttime(DVBS_t dvbs, uint32_t current);

static void local_state(DVBS_t dvbs, STRM_STATE state, int scale)
{
    dvbs->state = state;
    dvbs->scale = scale;

    if (dvbs->open_play == 1)
        stream_post_state(dvbs->index, state, scale);
}

static void local_time_calc(DVBS_t dvbs, uint32_t* pnow)
{
    uint32_t now;
    struct PVRInfo *info = &dvbs->pvr_info;

    now = mid_time( );
    if (pnow)
        *pnow = now;

    if (dvbs->open_play && STRM_STATE_IPTV == dvbs->state)
        local_set_currenttime(dvbs, now);

    if (dvbs->open_shift == 0)
        return;

    if (ind_pvr_get_info(dvbs->shift_id, info)) {
        LOG_STRM_WARN("#%d: ind_pvr_get_info\n", dvbs->index);
        return;
    }

    if (info->time_len > dvbs->shiftlen) {
        dvbs->time_length = dvbs->shiftlen;
        local_set_recordtime(dvbs, now - dvbs->time_length);
    } else {
        if (dvbs->rec_time == 0)
            local_set_recordtime(dvbs, now - info->time_len);
        dvbs->time_length = info->time_len;
    }

    if (STRM_STATE_PLAY == dvbs->state || STRM_STATE_PAUSE == dvbs->state) {
        if (NULL == dvbs->pvr) {
            int announce = ind_pvr_open(dvbs->shift_id, &dvbs->pvr);
            if (announce == 0) {
                dvbs->time_start = mid_time( );
                LOG_STRM_PRINTF("#%d time_start = %d, pvr_smooth = %d\n", dvbs->index, dvbs->time_start, dvbs->pvr_smooth);

                if (dvbs->pvr_smooth) {
                    if (ind_pvr_play(dvbs->pvr, -2, 1)) {
                        LOG_STRM_ERROR("#%d ind_pvr_play smooth\n", dvbs->index);
                        dvbs->pvr_smooth = 0;
                    } else {
                        dvbs->pvr_smooth_clk = strm_play_time(dvbs->strm_play);
                    }
                }
                if (0 == dvbs->pvr_smooth)
                    ind_pvr_play(dvbs->pvr, info->time_len, 1);
            }
        }
    }

    if (STRM_STATE_PLAY == dvbs->state || STRM_STATE_FAST == dvbs->state) {
        if (dvbs->pvr == NULL) {
            LOG_STRM_WARN("#%d pvr is NULL\n", dvbs->index);
            return;
        }

        {
            uint32_t begin, current;
            int sec;

            begin = dvbs->rec_time;

            if (STRM_STATE_PLAY == dvbs->state && dvbs->pvr_smooth) {
                int clk = strm_play_time(dvbs->strm_play);
                if (clk < dvbs->pvr_smooth_clk)
                    sec = 0;
                else
                    sec = (clk - dvbs->pvr_smooth_clk) / 100;
            } else {
                sec = strm_play_time(dvbs->strm_play) / 100;
            }
            current = (uint32_t)((int)dvbs->time_start + sec);

            //LOG_STRM_PRINTF("#%d: current = %d, begin = %d, end = %d, rec = %d, start = %d\n", dvbs->index, current, begin, now, dvbs->rec_time, dvbs->time_start);
            if (current < begin)
                current = begin;
            else if (current > now)
                current = now;

            LOG_STRM_DEBUG("#%d: scale = %d, time_length = %d, sec = %d, time_current = %u / %u\n", dvbs->index, dvbs->scale, dvbs->time_length, sec, dvbs->time_current, now);

            local_set_currenttime(dvbs, current);
        }
    }
}

static void local_time_sync(DVBS_t dvbs)
{
    uint32_t now;

    local_time_calc(dvbs, &now);

    if (dvbs->state == STRM_STATE_FAST && dvbs->scale < 0 && dvbs->time_current <= dvbs->rec_time) {
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_BEGIN\n", dvbs->index);
        local_msg(dvbs, STRM_MSG_STREAM_BEGIN, 0);
    }

    if (STRM_STATE_PAUSE == dvbs->state && int_stream_timeshift_jump( )) {
        if (dvbs->time_current + dvbs->time_length <= now) {
            LOG_STRM_PRINTF("#%d STRM_STATE_PAUSE > STRM_STATE_PLAY\n", dvbs->index);
            local_seek(dvbs, 0);
        }
    }
}

static void local_1000ms(void *arg)
{
    DVBS_t dvbs = (DVBS_t)arg;

    if (dvbs->open_play)
        local_time_sync(dvbs);

#ifdef INCLUDE_PVR
    {
        RecordMix_t rec_mix = dvbs->rec_mix;
        while (rec_mix) {
            if (rec_mix->clk && rec_mix->clk <= dvbs->clk) {
                uint32_t id = rec_mix->id;

                LOG_STRM_PRINTF("#%d id = 0x%08x local_record_close 1\n", dvbs->index, id);
                local_record_close(dvbs, id, 1);
                record_back_close(id);
                break;
            }
            rec_mix = rec_mix->next;
        }
    }
#endif
}

#ifdef INCLUDE_PVR
static void local_filter_timer(void *arg)
{
    DVBS_t dvbs = (DVBS_t)arg;

    if (dvbs_port_record_open(dvbs->index) == 0) {
        dvbs->open_filter = 1;
        ind_timer_delete(dvbs->tlink, local_filter_timer, dvbs);
    }
}

static void local_filter_open(DVBS_t dvbs)
{
    if (dvbs_port_record_open(dvbs->index) == 0)
        dvbs->open_filter = 1;
    else
        ind_timer_create(dvbs->tlink, mid_10ms( ) + INTERVAL_CLK_1000MS, INTERVAL_CLK_1000MS, local_filter_timer, dvbs);
}

static void local_filter_close(DVBS_t dvbs)
{
    ind_timer_delete(dvbs->tlink, local_filter_timer, dvbs);
    if (dvbs->open_filter == 1) {
        dvbs->open_filter = 0;
        dvbs_port_record_close(dvbs->index);
    }
}

static int local_shift(DVBS_t dvbs)
{
    int size;
    RecordArg arg;

    LOG_STRM_PRINTF("#%d\n", dvbs->index);

    size = strm_timeshift_size( );
    if (size < 0) {
        LOG_STRM_ERROR("#%d RECORD_MSG_DISK_ERROR\n", dvbs->index);
        stream_post_msg(dvbs->index, RECORD_MSG_DISK_ERROR, 0);
        return -1;
    }
    if (size == 0) {
        LOG_STRM_PRINTF("#%d RECORD_MSG_NOT_ENOUGH\n", dvbs->index);
        stream_post_msg(dvbs->index, RECORD_MSG_NOT_ENOUGH, 0);
        return -1;
    }

    memset(&arg, 0, sizeof(arg));
    arg.pvrarg.realtime = 1;
    arg.pvrarg.id = dvbs->shift_id;
    arg.pvrarg.time_shift = dvbs->shiftlen;
    if (dvbs->rec_mix) {
        strm_record_mix_open(dvbs->strm_record, &arg);
    } else {
        strm_record_open(dvbs->strm_record, dvbs, local_msg_back, &arg);
        if (dvbs->ca_param.networkid)
            strm_record_netwkid(dvbs->strm_record, dvbs->ca_param.networkid);
        local_filter_open(dvbs);
    }

    dvbs->open_shift = 1;

    return 0;
}
#endif

static void local_save(DVBS_t dvbs, int flag)
{
    LOG_STRM_PRINTF("#%d flag = %d\n", dvbs->index, flag);

    if (dvbs->fp) {
        fclose(dvbs->fp);
        dvbs->fp = NULL;
        sync( );
    }

    if (flag) {
        FILE* fp;
        char filename[64];

        sprintf(filename, DEFAULT_EXTERNAL_DATAPATH"/dvbs_%d.ts", dvbs->fp_sn);
        dvbs->fp_sn ++;

        fp = fopen(filename, "wb");
        LOG_STRM_PRINTF("#%d fp = %p filename = %s\n", dvbs->index, fp, filename);
        dvbs->fp = fp;
    }
}

static int local_open(DVBS_t dvbs, DvbsArg* dvbsarg)
{
    uint32_t clk, magic;

    clk = mid_10ms( );

    dvbs->tuner = dvbsarg->tuner;
    dvbs->print_clk = clk + DVBS_PRINT_CLK;

    dvbs->write_clk = 0;
    dvbs->write_pmts = 0;
    dvbs->write_times = 0;

    dvbs->stat_clk = clk;
    dvbs->stat_bytes = 0;

    dvbs->emm_num = 0;

    IND_STRCPY(dvbs->dvbs_url, dvbsarg->url);

    memset(&dvbs->ca_param, 0, sizeof(dvbs->ca_param));

    dvbs->clk = mid_10ms( );

    mid_mutex_lock(dvbs->dvbs_mutex);
    dvbs->dvbs_magic ++;
    mid_mutex_unlock(dvbs->dvbs_mutex);

    magic = ((uint32_t)dvbs->index << 16) | (dvbs->dvbs_magic & 0xffff);

    if (dvbs_port_play(dvbs->index, dvbs->tuner, dvbs->dvbs_url, dvbs_callback_writepmt, dvbs_callback_writedata, magic))
        LOG_STRM_ERROUT("#%d dvbs_port_play url = %s\n", dvbs->index, dvbs->dvbs_url);

    dvbs->pat_counter = 0;

    return 0;
Err:
    return -1;
}

static void local_close(DVBS_t dvbs)
{
    dvbs_port_stop(dvbs->index);

    mid_mutex_lock(dvbs->dvbs_mutex);
    dvbs->dvbs_magic ++;
    mid_mutex_unlock(dvbs->dvbs_mutex);

    local_state(dvbs, STRM_STATE_CLOSE, 0);
}

static int local_play_open(DVBS_t dvbs, PlayArg *arg, DvbsArg *dvbsarg)
{
    uint32_t clk;
    int code = 0;

    LOG_STRM_PRINTF("#%d buildtime : "__DATE__" "__TIME__" : stream url = %s\n", dvbs->index, dvbsarg->url);

    clk = mid_10ms( );

    dvbs->open_play = 0;
    dvbs->state = STRM_STATE_IPTV;

    dvbs->rec_time = 0;
    dvbs->shift_id = arg->shiftid;

    strm_play_open(dvbs->strm_play, dvbs->index, dvbs, local_msg_back, APP_TYPE_DVBS, 1316);
    strm_play_resume(dvbs->strm_play, dvbs->index, 1);

    dvbs->sb->len = 0;
    dvbs->open_play = 1;

    if (!dvbs->rec_mix) {
        if (local_open(dvbs, dvbsarg)) {
            code = DVBS_CODE_TUNER_ERROR;
            LOG_STRM_ERROUT("#%d: local_open\n", dvbs->index);
        }
    }

    ind_timer_create(dvbs->tlink, clk + INTERVAL_CLK_100MS, INTERVAL_CLK_100MS, local_push, dvbs);

    local_state(dvbs, STRM_STATE_IPTV, 1);

    dvbs->shiftlen = 0;
#ifdef INCLUDE_PVR
    if (dvbsarg->shiftlen > 0) {
        uint32_t timeshift = strm_record_shifttime( );

        if (dvbsarg->shiftlen > timeshift) {
            LOG_STRM_ERROR("#%d shiftlen = %d, timeshift = %d\n", dvbs->index, dvbsarg->shiftlen, timeshift);
        } else {
            dvbs->shiftlen = dvbsarg->shiftlen;
        }
    }
#endif

    dvbs->time_start = 0;

    dvbs->music_flg = -1;

    dvbs->push_end = 0;

    dvbs->delay_flg = 0;

    dvbs->time_length = 0;
    local_set_currenttime(dvbs, mid_time( ));

    return 0;
Err:
    if (dvbs->open_play) {
        strm_play_close(dvbs->strm_play, dvbs->index, 1);
        dvbs->open_play = 0;
    }
    dvbs->state = STRM_STATE_CLOSE;
    LOG_STRM_PRINTF("#%d STRM_MSG_OPEN_ERROR code = %d\n", dvbs->index, code);
    stream_post_msg(dvbs->index, STRM_MSG_OPEN_ERROR, code);
    return -1;
}

static void local_play_close(DVBS_t dvbs, int clear)
{
    LOG_STRM_PRINTF("#%d open_play = %d, open_shift = %d, rec_mix = %p\n", dvbs->index, dvbs->open_play, dvbs->open_shift, dvbs->rec_mix);
    if (dvbs->open_play == 0)
        return;

    if (dvbs->fp) {
        local_save(dvbs, 0);
        sync( );
    }

#ifdef INCLUDE_PVR
    if (dvbs->open_shift)
        local_close_timeshift(dvbs);

    ind_timer_delete(dvbs->tlink, local_delay_timer, dvbs);

    if (dvbs->rec_mix)
        local_state(dvbs, STRM_STATE_IPTV, 0);
    else
#endif
        local_close(dvbs);

    mid_mutex_lock(dvbs->dvbs_mutex);
    strm_play_close(dvbs->strm_play, dvbs->index, clear);
    dvbs->open_play = 0;
    mid_mutex_unlock(dvbs->dvbs_mutex);
}

#ifdef INCLUDE_PVR
static void local_record_msg(DVBS_t dvbs, uint32_t id, STRM_MSG msgno, int arg)
{
	RecordMix_t rec_mix = dvbs->rec_mix;

    while (rec_mix) {
        if (!id || id == rec_mix->id)
            record_post_msg(dvbs->index, rec_mix->id, msgno, arg);
        rec_mix = rec_mix->next;
    }
}

static int local_record_open(DVBS_t dvbs, RecordArg *arg, DvbsArg *dvbsarg)
{
    uint32_t id, clk;
	RecordMix_t rec_mix;

    LOG_STRM_PRINTF("#%d buildtime : "__DATE__" "__TIME__" : stream\n", dvbs->index);

    dvbs->rec_index = arg->index;

    id = arg->pvrarg.id;
    {
        int size = strm_record_size( );
        if (size == -1) {
            LOG_STRM_ERROR("#%d RECORD_MSG_DISK_ERROR\n", dvbs->index);
            record_post_msg(dvbs->index, id, RECORD_MSG_DISK_ERROR, DVBS_CODE_TUNER_ERROR);
            return -1;
        }
        if (size == 0) {
            LOG_STRM_ERROR("#%d RECORD_MSG_DISK_FULL\n", dvbs->index);
            record_post_msg(dvbs->index, id, RECORD_MSG_DISK_FULL, DVBS_CODE_TUNER_ERROR);
            return -1;
        }
    }

    clk = mid_10ms( );

    arg->pvrarg.realtime = 1;

    rec_mix = (RecordMix_t)IND_CALLOC(sizeof(RecordMix), 1);
    if (!rec_mix)
        LOG_STRM_ERROUT("#%d malloc RecordMix\n", dvbs->index);
    rec_mix->id = id;

    if (!dvbs->rec_mix && dvbs->open_shift == 0 && arg->add == 1)
        ind_pvr_rec_rebreak(id);
    if (dvbs->rec_mix || dvbs->open_shift) {
        strm_record_mix_open(dvbs->strm_record, arg);
    } else {
        strm_record_open(dvbs->strm_record, dvbs, local_msg_back, arg);
        local_filter_open(dvbs);
    }
    if (dvbs->ca_param.networkid)
        strm_record_netwkid(dvbs->strm_record, dvbs->ca_param.networkid);

    if (0 == dvbs->open_play && !dvbs->rec_mix) {
        if (local_open(dvbs, dvbsarg)) {

            LOG_STRM_ERROR("#%d: RECORD_MSG_ERROR local_open\n", dvbs->index);
            record_post_msg(dvbs->index, id, RECORD_MSG_ERROR, DVBS_CODE_TUNER_ERROR);

            if (dvbs->open_shift) {
                strm_record_mix_close(dvbs->strm_record, id, 0);
            } else {
                local_filter_close(dvbs);
                strm_record_close(dvbs->strm_record, 0);
            }
            IND_FREE(rec_mix);
            return -1;
        }
        local_state(dvbs, STRM_STATE_IPTV, 0);
    }

    {
	    RecordMix_t mix, prev;

        prev = NULL;
        mix = dvbs->rec_mix;
        while (mix && mix->clk) {
            prev = mix;
            mix = mix->next;
        }
        rec_mix->next = mix;
        if (prev)
            prev->next = rec_mix;
        else
            dvbs->rec_mix = rec_mix;
    }

    ind_timer_create(dvbs->tlink, clk + INTERVAL_CLK_100MS, INTERVAL_CLK_100MS, local_push, dvbs);

    if (arg->end > 0)
        local_record_end(dvbs, id, arg->end);

    return 0;
Err:
    return -1;
}

static void local_record_close(DVBS_t dvbs, uint32_t id, int end)
{
	RecordMix_t rec_mix;
    LOG_STRM_PRINTF("#%d open_play = %d, open_shift = %d, rec_mix = %p\n", dvbs->index, dvbs->open_play, dvbs->open_shift, dvbs->rec_mix);

    rec_mix = dvbs->rec_mix;
    if (id) {
        while (rec_mix && rec_mix->id != id)
            rec_mix = rec_mix->next;
    }

    if (!rec_mix) {
       LOG_STRM_ERROR("#%d id = 0x%08x cosed!\n", dvbs->index, id);
       return;
    }

    if (end == 2) {
        end = 0;
    } else {
        if (id == 0 || (rec_mix == dvbs->rec_mix && !rec_mix->next)) {
            if (dvbs->open_shift) {
                rec_mix = dvbs->rec_mix;
                while (rec_mix) {
                    strm_record_mix_close(dvbs->strm_record, rec_mix->id, end);
                    rec_mix = rec_mix->next;
                }
            } else {
                local_filter_close(dvbs);
                strm_record_close(dvbs->strm_record, end);
            }
        } else {
            strm_record_mix_close(dvbs->strm_record, id, end);
        }
    }

    if (end == 0) {
        RecordMix_t prev, next;

        prev = NULL;
        rec_mix = dvbs->rec_mix;
        while (rec_mix) {
            next = rec_mix->next;
            if (id == 0 || id == rec_mix->id) {
                LOG_STRM_PRINTF("#%d RECORD_MSG_CLOSE id = 0x%08x\n", dvbs->index, rec_mix->id);
                record_post_msg(dvbs->index, rec_mix->id, RECORD_MSG_CLOSE, 0);
                IND_FREE(rec_mix);
                if (prev)
                    prev->next = next;
                else
                    dvbs->rec_mix = next;
            } else {
                prev = rec_mix;
            }
            rec_mix = next;
        }
        if (!dvbs->rec_mix && !dvbs->open_play) {
            local_close(dvbs);
        }
    }
}

static void local_record_end(DVBS_t dvbs, uint32_t id, uint32_t endtime)
{
    uint32_t clk, now;
    RecordMix_t rec_mix;

    {
        struct ind_time tp;
        char *buf = dvbs->pat_buf;

        now = mid_time( );
        ind_time_local(now, &tp);
        sprintf(buf, "%04d%02d%02dT%02d%02d%02d", tp.year, tp.mon, tp.day, tp.hour, tp.min, tp.sec);
        LOG_STRM_PRINTF("now = %d / %s\n", now, buf);

        ind_time_local(endtime, &tp);
        sprintf(buf, "%04d%02d%02dT%02d%02d%02d", tp.year, tp.mon, tp.day, tp.hour, tp.min, tp.sec);
        LOG_STRM_PRINTF("end = %d / %s\n", endtime, buf);
    }

    if (endtime <= now) {
        LOG_STRM_WARN("#%d endtime = %u, now = %u\n", dvbs->index, endtime, now);
        clk = dvbs->clk;
    } else {
        clk = dvbs->clk + (endtime - now) * 100;
    }

    rec_mix = dvbs->rec_mix;
    while (rec_mix) {
        if (!id || id == rec_mix->id)
            rec_mix->clk = clk;
        rec_mix = rec_mix->next;
    }
}

static int local_open_timeshift(DVBS_t dvbs)
{
    if (dvbs->open_play == 0 || dvbs->open_shift || dvbs->shiftlen <= 0) {
        LOG_STRM_ERROR("#%d open_play = %d, rec_mix = %p, shiftlen = %d\n", dvbs->index, dvbs->open_play, dvbs->rec_mix, dvbs->shiftlen);
        stream_post_msg(dvbs->rec_index, RECORD_MSG_DATA_DAMAGE, 0);
        return -1;
    }

    if (local_shift(dvbs)) {
        LOG_STRM_ERROR("#%d local_shift\n", dvbs->index);
        return -1;
    }

    dvbs->rec_time = 0;

    return 0;
}

static int local_close_timeshift(DVBS_t dvbs)
{
    if (dvbs->open_play != 1 || dvbs->open_shift == 0)
        LOG_STRM_ERROUT("#%d open_play = %d, open_shift = %d\n", dvbs->index, dvbs->open_play, dvbs->open_shift);

    if (dvbs->state == STRM_STATE_PLAY || dvbs->state == STRM_STATE_PAUSE || dvbs->state == STRM_STATE_FAST)
        local_stop(dvbs);

    if (dvbs->rec_mix) {
        strm_record_mix_close(dvbs->strm_record, dvbs->shift_id, 0);
    } else {
        local_filter_close(dvbs);
        strm_record_close(dvbs->strm_record, 0);
    }
    dvbs->open_shift = 0;

    return 0;
Err:
    return -1;
}

static int local_pause(DVBS_t dvbs)
{
    LOG_STRM_PRINTF("#%d\n", dvbs->index);
    if (0 == dvbs->open_shift)
        LOG_STRM_ERROUT("#%d open_shift is zero\n", dvbs->index);

    if (dvbs->pvr)
        local_time_calc(dvbs, NULL);

    dvbs->pvr_smooth_pause = 0;

    switch(dvbs->state) {
    case STRM_STATE_IPTV:
        local_set_currenttime(dvbs, mid_time( ));

        dvbs->pvr_smooth = 0;
        strm_record_stamp(dvbs->strm_record);
        dvbs->pvr_smooth = 1;
        dvbs->pvr_smooth_pause = 1;
        dvbs->clk_fast = dvbs->clk + PVR_FAST_CLK;
        break;
    case STRM_STATE_PLAY:
        break;
    case STRM_STATE_FAST:
        if (dvbs->pvr == NULL)
            LOG_STRM_ERROUT("#%d pvr is NULL\n", dvbs->index);

        dvbs->time_start = dvbs->time_current;
        strm_play_reset(dvbs->strm_play, dvbs->index, 1);
        break;
    default:
        LOG_STRM_ERROUT("#%d state = %d\n", dvbs->index, dvbs->state);
    }

    mid_mutex_lock(dvbs->dvbs_mutex);
    local_state(dvbs, STRM_STATE_PAUSE, 0);
    dvbs->clk_pause = dvbs->clk;
    strm_play_pause(dvbs->strm_play, dvbs->index);
    mid_mutex_unlock(dvbs->dvbs_mutex);

    return 0;
Err:
    if (dvbs->state == STRM_STATE_IPTV && dvbs->pvr) {
        ind_pvr_close(dvbs->pvr);
        dvbs->pvr = NULL;
    }
    stream_post_state(dvbs->index, dvbs->state, dvbs->scale);

    return -1;
}

static int local_resume(DVBS_t dvbs)
{
    int reset = 0;
    LOG_STRM_PRINTF("#%d\n", dvbs->index);

    if (dvbs->pvr == NULL)
        LOG_STRM_ERROUT("#%d pvr is NULL\n", dvbs->index);

    switch(dvbs->state) {
    case STRM_STATE_PAUSE:
        if (dvbs->clk_fast && dvbs->clk_fast >= dvbs->clk_pause)
            dvbs->clk_fast = dvbs->clk + (dvbs->clk_fast - dvbs->clk_pause);
        else
            dvbs->clk_fast = 0;
        break;
    case STRM_STATE_FAST:
        local_time_calc(dvbs, NULL);
        dvbs->time_start = dvbs->time_current;
        reset = 1;
        dvbs->clk_fast = dvbs->clk + PVR_FAST_CLK;
        break;
    default:
        LOG_STRM_ERROUT("#%d state = %d\n", dvbs->index, dvbs->state);
    }

    if (dvbs->pvr && ind_pvr_play(dvbs->pvr, -1, 1))
        LOG_STRM_ERROUT("#%d ind_pvr_play\n", dvbs->index);

    mid_mutex_lock(dvbs->dvbs_mutex);
    local_state(dvbs, STRM_STATE_PLAY, 1);
    if (reset)
        strm_play_reset(dvbs->strm_play, dvbs->index, 1);
    strm_play_resume(dvbs->strm_play, dvbs->index, 0);
    mid_mutex_unlock(dvbs->dvbs_mutex);

    dvbs->push_end = 0;

    return 0;
Err:
    stream_post_state(dvbs->index, dvbs->state, dvbs->scale);
    return -1;
}

static int local_fast(DVBS_t dvbs, int scale)
{
    uint32_t now;
    int off;
    struct PVRInfo *info = &dvbs->pvr_info;

    LOG_STRM_PRINTF("#%d scale = %d\n", dvbs->index, scale);

    switch(scale) {
    case -32:   break;
    case -16:   break;
    case -8:    break;
    case -4:    break;
    case -2:    break;
    case 1:     break;
    case 2:     break;
    case 4:     break;
    case 8:     break;
    case 16:    break;
    case 32:    break;
    default:
        LOG_STRM_ERROUT("#%d scale = %d\n", dvbs->index, scale);
    }

    if (dvbs->open_shift == 0)
        LOG_STRM_ERROUT("#%d open_shift zero!\n", dvbs->index);

    if (NULL == dvbs->pvr) {
        if (ind_pvr_open(dvbs->shift_id, &dvbs->pvr))
            LOG_STRM_ERROR("#%d ind_pvr_open\n", dvbs->index);
    }

    if (NULL == dvbs->pvr || info->time_len <= 0) {
        LOG_STRM_WARN("#%d STRM_MSG_STREAM_END pvr = %p, scale = %d, time_len = %d\n", dvbs->index, dvbs->pvr, scale, info->time_len);

        stream_post_msg(dvbs->index, STRM_MSG_STREAM_END, 0);
        return local_stop(dvbs);
    }

    local_time_calc(dvbs, &now);

    if (1 == dvbs->music_flg)
        LOG_STRM_ERROUT("#%d music_flg is true!\n", dvbs->index);

    if (scale != dvbs->scale) {
        off = (int)info->time_len - (int)(now - dvbs->time_current);
        if (off < 0)
            off = 0;
        if (ind_pvr_play(dvbs->pvr, off, scale))
            LOG_STRM_ERROUT("#%d ind_pvr_play\n", dvbs->index);
        dvbs->time_start = dvbs->time_current;
    }

    mid_mutex_lock(dvbs->dvbs_mutex);
    if (scale != dvbs->scale) {
        strm_play_reset(dvbs->strm_play, dvbs->index, 1);
        strm_play_tplay(dvbs->strm_play, dvbs->index, scale);
    }
    local_state(dvbs, STRM_STATE_FAST, scale);
    mid_mutex_unlock(dvbs->dvbs_mutex);

    dvbs->push_end = 0;
    dvbs->pvr_smooth = 0;

    return 0;
Err:
    if (dvbs->state == STRM_STATE_IPTV && dvbs->pvr) {
        ind_pvr_close(dvbs->pvr);
        dvbs->pvr = NULL;
    }
    stream_post_state(dvbs->index, dvbs->state, dvbs->scale);
    return -1;
}

static int local_seek(DVBS_t dvbs, uint32_t offset)
{
    uint32_t now, begin;
    int off;
    struct PVRInfo    *info = &dvbs->pvr_info;

    if (dvbs->open_shift == 0)
        LOG_STRM_ERROUT("#%d open_shift zero!\n", dvbs->index);

    if (dvbs->pvr == NULL && ind_pvr_open(dvbs->shift_id, &dvbs->pvr))
        LOG_STRM_ERROUT("#%d ind_pvr_open state = %d\n", dvbs->index, dvbs->state);

    local_time_calc(dvbs, &now);

    LOG_STRM_PRINTF("#%d offset = %d, rec_time = %d, now = %d\n", dvbs->index, offset, dvbs->rec_time, now);

    begin = dvbs->rec_time;
    if (offset < begin)
        offset = begin;
    else if (offset > now)
        offset = now;

    off = (int)info->time_len - (int)(now - offset);
    if (off < 0)
        off = 0;
    if (ind_pvr_play(dvbs->pvr, off, 1))
        LOG_STRM_ERROUT("#%d ind_pvr_play\n", dvbs->index);
    dvbs->time_start = offset;
    local_set_currenttime(dvbs, offset);

    dvbs->clk_fast = dvbs->clk + PVR_FAST_CLK;

    mid_mutex_lock(dvbs->dvbs_mutex);
    dvbs->sb->len = 0;
    local_state(dvbs, STRM_STATE_PLAY, 1);
    strm_play_reset(dvbs->strm_play, dvbs->index, 1);
    strm_play_resume(dvbs->strm_play, dvbs->index, 0);
    mid_mutex_unlock(dvbs->dvbs_mutex);

    dvbs->push_end = 0;
    dvbs->pvr_smooth = 0;

    return 0;
Err:
    if (dvbs->state == STRM_STATE_IPTV && dvbs->pvr) {
        ind_pvr_close(dvbs->pvr);
        dvbs->pvr = NULL;
    }
    stream_post_state(dvbs->index, dvbs->state, dvbs->scale);
    return -1;
}

static int local_stop(DVBS_t dvbs)
{
    LOG_STRM_PRINTF("#%d\n", dvbs->index);

    if (dvbs->open_shift == 0)
        LOG_STRM_ERROUT("#%d open_shift zero!\n", dvbs->index);

    switch(dvbs->state) {
    case STRM_STATE_PAUSE:
        break;
    case STRM_STATE_PLAY:
        break;
    case STRM_STATE_FAST:
        break;
    default:
        LOG_STRM_ERROUT("#%d state = %d\n", dvbs->index, dvbs->state);
    }

    if (dvbs->pvr) {
        ind_pvr_close(dvbs->pvr);
        dvbs->pvr = NULL;
    }

    mid_mutex_lock(dvbs->dvbs_mutex);
    dvbs->sb->len = 0;
    local_state(dvbs, STRM_STATE_IPTV, 1);

    strm_play_reset(dvbs->strm_play, dvbs->index, 1);
    strm_play_resume(dvbs->strm_play, dvbs->index, 1);
    mid_mutex_unlock(dvbs->dvbs_mutex);

    local_time_calc(dvbs, NULL);

    return 0;
Err:
    stream_post_state(dvbs->index, dvbs->state, dvbs->scale);
    return -1;
}
#endif

static void local_emm(DVBS_t dvbs)
{
    int i;
    int add_num, del_num, emm_num, old_num;
    int add_pid[TS_EMM_NUM], del_pid[TS_EMM_NUM], old_pid[TS_EMM_NUM], *emm_pid;

    emm_pid = dvbs->emm_pid;

    old_num = dvbs->emm_num;
    for (i = 0; i < old_num; i ++)
        old_pid[i] = emm_pid[i];

    if (dvbs->open_play == 1)
        strm_play_emm(dvbs->strm_play, emm_pid, &emm_num);
#ifdef INCLUDE_PVR
    else if (dvbs->rec_mix)
        strm_record_emm(dvbs->strm_record, emm_pid, &emm_num);
#endif
    else
        return;

    if (emm_num > TS_EMM_NUM)
        emm_num = TS_EMM_NUM;

    add_num = 0;
    for (i = 0; i < emm_num; i++) {
        int j;
        for (j = 0; j < old_num; j++) {
            if (emm_pid[i] == old_pid[j]) {
                old_pid[j] = 0;
                break;
            }
        }
        if (j >= old_num) {
            add_pid[add_num] = emm_pid[i];
            add_num ++;
        }
    }

    del_num = 0;
    for (i = 0; i < old_num; i++) {
        if (old_pid[i]) {
            del_pid[del_num] = old_pid[i];
            del_num++;
        }
    }

    if (del_num >= 0 || add_num >= 0)
        dvbs_port_change(dvbs->index, 1, del_pid, del_num, add_pid, add_num);
}

static void local_msg_back(void *handle, STRM_MSG msgno, int arg)
{
    DVBS_t dvbs = (DVBS_t)handle;

    strm_msgq_queue(dvbs->strm_msgq, msgno, arg);
}

static void local_msg(DVBS_t dvbs, STRM_MSG msgno, int arg)
{
    LOG_STRM_DEBUG("#%d msgno = %d, arg = %d\n", dvbs->index, msgno, arg);

    switch(msgno) {
    case STRM_MSG_OPEN_ERROR:
    case STRM_MSG_PLAY_ERROR:
        LOG_STRM_PRINTF("#%d STRM_MSG_PLAY_ERROR music_flg = %d\n", dvbs->index, dvbs->music_flg);
        stream_post_msg(dvbs->index, STRM_MSG_OPEN_ERROR, 0);
        local_play_close(dvbs, 0);
        break;

#ifdef INCLUDE_PVR
    case RECORD_MSG_ERROR:
    case RECORD_MSG_DISK_FULL:
    case RECORD_MSG_DISK_ERROR:
    case RECORD_MSG_DATA_DAMAGE:
    case RECORD_MSG_PVR_CONFLICT:
        {
            RecordMix_t rec_mix;
            uint32_t id = (uint32_t)arg;

            switch(msgno) {
            case RECORD_MSG_DISK_FULL:      LOG_STRM_ERROR("#%d: RECORD_MSG_DISK_FULL... rec_mix = %p open_shift = %d\n", dvbs->index, dvbs->rec_mix, dvbs->open_shift);    break;
            case RECORD_MSG_DISK_ERROR:     LOG_STRM_ERROR("#%d: RECORD_MSG_DISK_ERROR.. rec_mix = %p open_shift = %d\n", dvbs->index, dvbs->rec_mix, dvbs->open_shift);    break;
            case RECORD_MSG_DATA_DAMAGE:    LOG_STRM_ERROR("#%d: RECORD_MSG_DATA_DAMAGE. rec_mix = %p open_shift = %d\n", dvbs->index, dvbs->rec_mix, dvbs->open_shift);    break;
            case RECORD_MSG_PVR_CONFLICT:   LOG_STRM_ERROR("#%d: RECORD_MSG_PVR_CONFLICT rec_mix = %p open_shift = %d\n", dvbs->index, dvbs->rec_mix, dvbs->open_shift);    break;
            default:                        LOG_STRM_ERROR("#%d: RECORD_MSG_ERROR....... rec_mix = %p open_shift = %d\n", dvbs->index, dvbs->rec_mix, dvbs->open_shift);    break;
            }

            rec_mix = dvbs->rec_mix;
            if (id) {
                while (rec_mix) {
                    if (id == rec_mix->id)
                        break;
                }
            }
            if (rec_mix) {
                local_record_msg(dvbs, id, msgno, 0);
                LOG_STRM_PRINTF("#%d id = 0x%08x local_record_close 0\n", dvbs->index, id);
                local_record_close(dvbs, id, 0);
            }
            if (dvbs->open_shift && (id == 0 || id == dvbs->shift_id)) {
                if (msgno == RECORD_MSG_ERROR) {
                    if (dvbs->pvr == NULL)
                        stream_post_msg(dvbs->index, RECORD_MSG_DATA_DAMAGE, 0);
                    else if (strm_record_size( ) == -1)
                        stream_post_msg(dvbs->index, RECORD_MSG_DISK_DETACHED, 0);
                    else
                        stream_post_msg(dvbs->index, RECORD_MSG_ERROR, 0);
                } else {
                    stream_post_msg(dvbs->index, msgno, 0);
                }
                local_close_timeshift(dvbs);
            }
        }
        break;
    case RECORD_MSG_FORBID:
        LOG_STRM_PRINTF("#%d RECORD_MSG_FORBID\n", dvbs->index);
        if (dvbs->rec_mix) {
            LOG_STRM_PRINTF("#%d local_record_close 0\n", dvbs->index);
            local_record_msg(dvbs, 0, RECORD_MSG_ERROR, 0);
            local_record_close(dvbs, 0, 0);
        }
        if (dvbs->open_shift)
            local_close_timeshift(dvbs);
        break;
    case RECORD_MSG_DISK_WARN:
        LOG_STRM_PRINTF("#%d RECORD_MSG_DISK_WARN\n", dvbs->index);
        if (dvbs->rec_mix)
            local_record_msg(dvbs, 0, RECORD_MSG_DISK_WARN, 0);
        break;
    case RECORD_MSG_NET_TIMEOUT:
        LOG_STRM_PRINTF("#%d RECORD_MSG_NET_TIMEOUT\n", dvbs->index);
        if (dvbs->rec_mix)
            local_record_msg(dvbs, 0, RECORD_MSG_NET_TIMEOUT, 0);
        break;
    case STRM_MSG_STREAM_BEGIN:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_BEGIN\n", dvbs->index);
        local_seek(dvbs, 0);
        stream_post_msg(dvbs->index, STRM_MSG_STREAM_BEGIN, 0);
        break;
    case STRM_MSG_STREAM_END:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", dvbs->index);
        local_stop(dvbs);
        stream_post_msg(dvbs->index, STRM_MSG_STREAM_END, 0);
        break;
#endif

    case STRM_MSG_CHANGE_PSI:
        LOG_STRM_PRINTF("#%d STRM_MSG_CHANGE_PSI\n", dvbs->index);
        dvbs_port_change(dvbs->index, 2, NULL, 0, NULL, 0);
        break;
    case STRM_MSG_CHANGE_CRC:
        LOG_STRM_PRINTF("#%d STRM_MSG_CHANGE_CRC play = %d, rec_mix = %p\n", dvbs->index, dvbs->open_play, dvbs->rec_mix);
        if (arg == 2)
            dvbs_port_change(dvbs->index, 2, NULL, 0, NULL, 0);
        local_emm(dvbs);
        break;
    case STRM_MSG_STREAM_MUSIC:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_MUSIC\n", dvbs->index);
        dvbs->music_flg = 1;
        break;
    case STRM_MSG_STREAM_VIDEO:
        LOG_STRM_DEBUG("#%d STRM_MSG_STREAM_VIDEO\n", dvbs->index);
        dvbs->music_flg = 0;
        break;
    case STRM_MSG_PSI_VIEW:
        LOG_STRM_PRINTF("#%d STRM_MSG_PSI_VIEW\n", dvbs->index);
        local_emm(dvbs);
        break;
    case STRM_MSG_PTS_VIEW:
        LOG_STRM_PRINTF("#%d STRM_MSG_PTS_VIEW\n", dvbs->index);
        stream_post_msg(dvbs->index, STRM_MSG_PTS_VIEW, 0);
        break;
    case STRM_MSG_RECV_FIRST:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_FIRST\n", dvbs->index);
        break;

    case STRM_MSG_FREEZE:
        LOG_STRM_PRINTF("#%d STRM_MSG_FREEZE state = %d\n", dvbs->index, dvbs->state);
        if (STRM_STATE_IPTV == dvbs->state) {
            strm_play_reset(dvbs->strm_play, dvbs->index, 0);
            strm_play_resume(dvbs->strm_play, dvbs->index, 1);
        }
        break;

    case STRM_MSG_RECV_TIMEOUT:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_TIMEOUT\n", dvbs->index);
        stream_post_msg(dvbs->index, STRM_MSG_RECV_TIMEOUT, 0);
        break;
    case STRM_MSG_RECV_RESUME:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_RESUME\n", dvbs->index);
        stream_post_msg(dvbs->index, STRM_MSG_RECV_RESUME, 0);
        break;
    case STRM_MSG_RECV_TIMEOUT15:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_TIMEOUT15\n", dvbs->index);
        break;
    case STRM_MSG_FIRST_TIMEOUT:
        LOG_STRM_PRINTF("#%d STRM_MSG_FIRST_TIMEOUT\n", dvbs->index);
        break;

    case STRM_MSG_PSI_ERROR:
        LOG_STRM_PRINTF("#%d STRM_MSG_PSI_ERROR\n", dvbs->index);
        stream_post_msg(dvbs->index, STRM_MSG_PSI_ERROR, 0);
        break;
    case STRM_MSG_PSI_RESUME:
        LOG_STRM_PRINTF("#%d STRM_MSG_PSI_RESUME\n", dvbs->index);
        stream_post_msg(dvbs->index, STRM_MSG_PSI_RESUME, 0);
        break;

#ifdef INCLUDE_PVR
    case RECORD_MSG_SUCCESS_BEGIN:
        {
            uint32_t id = (uint32_t)arg;

            LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_BEGIN id = %08x / %08x\n", dvbs->index, id, dvbs->shift_id);

            if (dvbs->rec_mix)
                local_record_msg(dvbs, id, RECORD_MSG_SUCCESS_BEGIN, 0);

            if (dvbs->open_shift && id == dvbs->shift_id)
                stream_post_msg(dvbs->index, RECORD_MSG_SUCCESS_BEGIN, 0);
        }
        local_emm(dvbs);
        break;
    case RECORD_MSG_SUCCESS_END:
        LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_END\n", dvbs->index);
        local_record_msg(dvbs, (uint32_t)arg, RECORD_MSG_SUCCESS_END, 0);
        local_record_close(dvbs, (uint32_t)arg, 2);
        break;
#endif

    default:
        LOG_STRM_ERROUT("#%d msgno = %d\n", dvbs->index, msgno);
    }

Err:
    return;
}

#ifdef INCLUDE_PVR
static void local_trickmode(DVBS_t dvbs, int cmd, int arg)
{
    if (cmd == STREAM_CMD_SEEK && arg == dvbs->time_current && dvbs->state == STRM_STATE_PAUSE) {
        LOG_STRM_PRINTF("#%d STREAM_CMD_SEEK > STREAM_CMD_RESUME\n", dvbs->index);
        cmd = STREAM_CMD_RESUME;
    }

    if (cmd == STREAM_CMD_SEEK) {
        if (dvbs->delay_flg) {
            ind_timer_delete(dvbs->tlink, local_delay_timer, dvbs);
            dvbs->delay_flg = 0;
        }

        LOG_STRM_PRINTF("#%d STREAM_CMD_SEEK\n", dvbs->index);
        local_seek(dvbs, (uint32_t)arg);
    } else {
        if (dvbs->delay_flg == 0 || dvbs->delay_flg == 3) {
            switch(cmd) {
            case STREAM_CMD_RESUME:
                LOG_STRM_PRINTF("#%d STREAM_CMD_RESUME\n", dvbs->index);
                local_resume(dvbs);
                break;

            case STREAM_CMD_PAUSE:
                LOG_STRM_PRINTF("#%d STREAM_CMD_PAUSE\n", dvbs->index);
                local_pause(dvbs);
                break;

            case STREAM_CMD_FAST:
                LOG_STRM_PRINTF("#%d STREAM_CMD_FAST\n", dvbs->index);
                local_fast(dvbs, arg);
                break;

            case STREAM_CMD_STOP:
                LOG_STRM_PRINTF("#%d STREAM_CMD_SEEK\n", dvbs->index);
                local_stop(dvbs);
                break;

            default:
                break;
            }

            dvbs->delay_flg = 1;
            ind_timer_create(dvbs->tlink, mid_10ms( ) + INTERVAL_CLK_1000MS, 0, local_delay_timer, dvbs);
        } else {
            switch(cmd) {
            case STREAM_CMD_RESUME:
                LOG_STRM_PRINTF("#%d STREAM_CMD_RESUME_\n", dvbs->index);
                stream_post_state(dvbs->index, STRM_STATE_PLAY, 1);
                break;

            case STREAM_CMD_PAUSE:
                LOG_STRM_PRINTF("#%d STREAM_CMD_PAUSE_\n", dvbs->index);
                stream_post_state(dvbs->index, STRM_STATE_PAUSE, 0);
                break;

            case STREAM_CMD_FAST:
                LOG_STRM_PRINTF("#%d STREAM_CMD_FAST_\n", dvbs->index);
                stream_post_state(dvbs->index, STRM_STATE_FAST, arg);
                break;

            case STREAM_CMD_STOP:
                LOG_STRM_PRINTF("#%d STREAM_CMD_SEEK_\n", dvbs->index);
                stream_post_state(dvbs->index, STRM_STATE_IPTV, 1);
                break;

            default:
                break;
            }

            dvbs->delay_flg = 2;
            dvbs->delay_cmd = cmd;
            dvbs->delay_arg = arg;
            ind_timer_create(dvbs->tlink, mid_10ms( ) + INTERVAL_CLK_1000MS, 0, local_delay_timer, dvbs);
        }
    }
}

static void local_delay_timer(void *arg)
{
    DVBS_t dvbs = (DVBS_t)arg;

    if (dvbs->delay_flg == 2) {
        dvbs->delay_flg = 3;
        local_trickmode(dvbs, dvbs->delay_cmd, dvbs->delay_arg);
    } else {
        dvbs->delay_flg = 0;
    }
}
#endif

static void local_cmd(DVBS_t dvbs, StreamCmd* strmCmd)
{
    int cmd = strmCmd->cmd;

    LOG_STRM_PRINTF("#%d %d %d\n", dvbs->index, dvbs->state, cmd);

    if (stream_deal_cmd(dvbs->index, strmCmd) == 1)
        return;

    switch(cmd) {
    case STREAM_CMD_OPEN:
        LOG_STRM_PRINTF("#%d STREAM_CMD_OPEN\n", dvbs->index);
        if (dvbs->open_play || !dvbs->rec_mix)
            LOG_STRM_ERROUT("#%d open_play = %d rec_mix = %p\n", dvbs->index, dvbs->open_play, dvbs->rec_mix);
        {
            PlayArg *arg;
            char *argbuf;
            if (stream_back_get_arg(dvbs->index, strmCmd->arg0, &arg, &argbuf))
                LOG_STRM_ERROUT("#%d open_play = %d\n", dvbs->index, dvbs->open_play);
            strm_play_leader_set(dvbs->strm_play, dvbs->index);
            local_play_open(dvbs, arg, (DvbsArg*)argbuf);
        }
        break;

    #ifdef INCLUDE_PVR
    case STREAM_CMD_RECORD_OPEN:
        LOG_STRM_PRINTF("#%d STREAM_CMD_RECORD_OPEN open_play = %d, rec_mix = %p\n", dvbs->index, dvbs->open_play, dvbs->rec_mix);
        {
            RecordArg *arg;
            char *argbuf;
            if (record_back_get_arg(dvbs->index, strmCmd->arg0, &arg, &argbuf))
                LOG_STRM_ERROUT("#%d stream_back_get_arg\n", dvbs->index);
            if (local_record_open(dvbs, arg, (DvbsArg*)argbuf))
                LOG_STRM_ERROUT("#%d local_record_open\n", dvbs->index);
        }
        break;
    #endif

    case STREAM_CMD_TEST_SAVE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_TEST_SAVE flag = %d\n", dvbs->index, strmCmd->arg0);
        local_save(dvbs, strmCmd->arg0);
        break;

    #ifdef INCLUDE_PVR
    case STREAM_CMD_RECORD_END:
        LOG_STRM_PRINTF("#%d STREAM_CMD_RECORD_END 0x%08x %u\n", dvbs->index, (uint32_t)strmCmd->arg0, (uint32_t)strmCmd->arg1);
        local_record_end(dvbs, (uint32_t)strmCmd->arg0, (uint32_t)strmCmd->arg1);
        break;

    case STREAM_CMD_TIMESHIFT_OPEN:
        LOG_STRM_PRINTF("#%d STREAM_CMD_TIMESHIFT_OPEN\n", dvbs->index);
        local_open_timeshift(dvbs);
        break;

    case STREAM_CMD_TIMESHIFT_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_TIMESHIFT_CLOSE\n", dvbs->index);
        local_close_timeshift(dvbs);
        break;

    case STREAM_CMD_WAKE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_WAKE\n", dvbs->index);
        break;

    case STREAM_CMD_RESUME:
    case STREAM_CMD_PAUSE:
    case STREAM_CMD_FAST:
    case STREAM_CMD_SEEK:
    case STREAM_CMD_STOP:
        {
            int cmdsn = strmCmd->arg3;
            if (dvbs->music_flg >= 0)
                local_trickmode(dvbs, cmd, strmCmd->arg0);
            if (cmdsn)
                stream_back_cmd(dvbs->index, cmdsn);
        }
        break;
    #endif

    case STREAM_CMD_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE clear = %d, open_play = %d, rec_mix = %p\n", dvbs->index, strmCmd->arg0, dvbs->open_play, dvbs->rec_mix);
        if (dvbs->open_play == 0)
            LOG_STRM_ERROUT("#%d play closed!\n", dvbs->index);
        local_play_close(dvbs, strmCmd->arg0);
        return;

    #ifdef INCLUDE_PVR
    case STREAM_CMD_RECORD_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_RECORD_CLOSE arg = %d, open_play = %d, rec_mix = %p\n", dvbs->index, strmCmd->arg0, dvbs->open_play, dvbs->rec_mix);
        if (!dvbs->rec_mix)
            LOG_STRM_ERROUT("#%d record closed!\n", dvbs->index);
        if (strmCmd->arg0 != -1)
            strmCmd->arg0 = 0;
        local_record_close(dvbs, (uint32_t)strmCmd->arg1, strmCmd->arg0);
        return;
    #endif

    default:
        LOG_STRM_ERROUT("#%d Unkown CMD %d\n", dvbs->index, cmd);
    }

Err:
    return;
}

static void local_push(void *arg)
{
    int len;
    DVBS_t dvbs;
    StrmBuffer* sb;

    dvbs = (DVBS_t)arg;

    if (dvbs->open_play && int_strm_playing(dvbs->index) == 0) {
        LOG_STRM_PRINTF("closed!\n");
        local_play_close(dvbs, 0);
        if (!dvbs->rec_mix)
            return;
    }

    if (dvbs->open_filter == 1 && dvbs_port_record_check(dvbs->index) == -1)
        local_msg_back(dvbs, RECORD_MSG_FORBID, 0);

    if (dvbs->print_clk < dvbs->clk) {
        int length, byteRate, stat_bytes;
        uint32_t stat_clk, write_pmts, write_times;

        mid_mutex_lock(dvbs->dvbs_mutex);

        stat_clk = dvbs->stat_clk;
        dvbs->stat_clk = dvbs->clk;
        stat_bytes = dvbs->stat_bytes;
        dvbs->stat_bytes = 0;

        write_pmts = dvbs->write_pmts;
        dvbs->write_pmts = 0;
        write_times = dvbs->write_times;
        dvbs->write_times = 0;

        mid_mutex_unlock(dvbs->dvbs_mutex);

        byteRate = stat_bytes * 25 / (int)(dvbs->clk - stat_clk) / 256;//25 = 100 / 4, 256 = 1024 / 4;
        if (dvbs->open_play) {
            int diff_fast = 0;
            if (STRM_STATE_PLAY == dvbs->state && dvbs->clk_fast)
                diff_fast = (int)(dvbs->clk_fast - dvbs->clk);
            length = strm_play_length(dvbs->strm_play);
            LOG_STRM_PRINTF("#%d write times = %d, write_pmts = %d, byteRate = %d kB/s, diff = %d / %d, length = %d\n", dvbs->index, write_times, write_pmts, byteRate, dvbs->diff, diff_fast, length);
        } else {
            LOG_STRM_PRINTF("#%d write times = %d, write_pmts = %d, byteRate = %d kB/s\n", dvbs->index, write_times, write_pmts, byteRate);
        }
        dvbs->print_clk  = dvbs->clk + DVBS_PRINT_CLK;
    }

    if (dvbs->open_play == 0 || dvbs->state == STRM_STATE_CLOSE)
        return;

    dvbs->diff = strm_play_buffer(dvbs->strm_play);

    if (STRM_STATE_IPTV == dvbs->state)
        return;

    if (STRM_STATE_PAUSE == dvbs->state && 0 == dvbs->pvr_smooth_pause)
        return;

    if (1 == dvbs->push_end || NULL == dvbs->pvr)
        return;

    for (;;) {
        if (strm_play_space(dvbs->strm_play) < 1316)
            break;

        sb = dvbs->pvr_sb;
        if (STRM_STATE_PLAY == dvbs->state) {
            if (dvbs->clk_fast && dvbs->clk_fast < dvbs->clk)
                dvbs->clk_fast = 0;

            if (dvbs->clk_fast || dvbs->diff < 300)
                len = ind_pvr_read(dvbs->pvr, dvbs->clk * 10 / 9, sb->buf, sb->size);//加速取数据
            else
                len = ind_pvr_read(dvbs->pvr, dvbs->clk, sb->buf, sb->size);
        } else {
            len = ind_pvr_read(dvbs->pvr, dvbs->clk, sb->buf, sb->size);
        }
        if (len > 0) {
            sb->off = 0;
            sb->len = len;
            strm_play_push(dvbs->strm_play, dvbs->index, &dvbs->pvr_sb);
            continue;
        }
        switch(len) {
        case PVR_ANNOUNCE_NONE:
            //LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_NONE\n", dvbs->index);
            return;
        case PVR_ANNOUNCE_WRITE:
            if (STRM_STATE_PAUSE == dvbs->state)
                return;
            if (dvbs->scale != 1) {
                LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_WRITE > STRM_MSG_STREAM_END\n", dvbs->index);
                local_msg_back(dvbs, STRM_MSG_STREAM_END, 0);
                dvbs->push_end = 1;
            }
            return;
        case PVR_ANNOUNCE_BEGIN:
            LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_BEGIN > STRM_MSG_STREAM_BEGIN\n", dvbs->index);
                local_msg_back(dvbs, STRM_MSG_STREAM_BEGIN, 0);
            dvbs->push_end = 1;
            return;
        case PVR_ANNOUNCE_END:
            LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_END\n", dvbs->index);
            strm_play_end(dvbs->strm_play, dvbs->index);
            dvbs->push_end = 1;
            return;
        case PVR_ANNOUNCE_ERROR:
            LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_ERROR > STRM_MSG_OPEN_ERROR\n", dvbs->index);
            local_msg(dvbs, STRM_MSG_OPEN_ERROR, 0);
            return;
        default:
            return;
        }
    }
}


static void local_loop(DVBS_t dvbs)
{
    uint32_t        clk, clks, out;
    fd_set          rset;
    struct timeval  tv;

    dvbs->clk = mid_10ms( );
    ind_timer_create(dvbs->tlink, dvbs->clk + INTERVAL_CLK_1000MS, INTERVAL_CLK_1000MS, local_1000ms, dvbs);

    while (dvbs->state != STRM_STATE_CLOSE) {

        clk = mid_10ms( );
        dvbs->clk = clk;

        out = ind_timer_clock(dvbs->tlink);
        if (out <= clk) {
            ind_timer_deal(dvbs->tlink, clk);
            continue;
        }
        if (strm_msgq_valid(dvbs->strm_msgq)) {
            StreamMsg msg;
            strm_msgq_print(dvbs->strm_msgq);
            if (strm_msgq_pump(dvbs->strm_msgq, &msg) == 1)
                local_msg(dvbs, msg.msg, msg.arg);
            continue;
        }

        clks = out - clk;
        tv.tv_sec = clks / 100;
        tv.tv_usec = clks % 100 * 10000;

        FD_ZERO(&rset);
        FD_SET((uint32_t)dvbs->msgfd, &rset);

        if (select(dvbs->msgfd + 1, &rset , NULL,  NULL, &tv) <= 0)
            continue;

        if (FD_ISSET((uint32_t)dvbs->msgfd, &rset)) {
            StreamCmd strmCmd;

            memset(&strmCmd, 0, sizeof(strmCmd));
            mid_msgq_getmsg(dvbs->msgq, (char *)(&strmCmd));
            local_cmd(dvbs, &strmCmd);
            continue;
        }
    }
    ind_timer_delete_all(dvbs->tlink);

    dvbs->open_play = 0;
    dvbs->open_shift = 0;
#ifdef INCLUDE_PVR
    {
        RecordMix_t rec_mix;
        while (dvbs->rec_mix) {
            rec_mix = dvbs->rec_mix;
            dvbs->rec_mix = rec_mix->next;
            IND_FREE(rec_mix);
        }
    }
#endif
    dvbs->open_filter = 0;
}

static void local_loop_begin(DVBS_t dvbs, int idx, mid_msgq_t msgq)
{
    dvbs->msgq = msgq;
    dvbs->msgfd = mid_msgq_fd(dvbs->msgq);

    dvbs->tlink = int_stream_tlink(idx);
    dvbs->strm_msgq = int_strm_msgq(idx);
    dvbs->strm_play = int_strm_play(idx);
#ifdef INCLUDE_PVR
    dvbs->strm_record = int_strm_record(idx);
#endif

    dvbs->sb = strm_buf_malloc(1316);
#ifdef INCLUDE_PVR
    dvbs->pvr_sb = strm_buf_malloc(1316);
#endif
}
static void local_loop_end(DVBS_t dvbs)
{
    if (dvbs) {
        if (dvbs->sb) {
            strm_buf_free(dvbs->sb);
            dvbs->sb = NULL;
        }
#ifdef INCLUDE_PVR
        if (dvbs->pvr_sb) {
            strm_buf_free(dvbs->pvr_sb);
            dvbs->pvr_sb = NULL;
        }
#endif
    }
}

static void local_loop_play(void *handle, int idx, mid_msgq_t msgq, PlayArg *arg, char* argbuf)
{
    DVBS_t dvbs = (DVBS_t)handle;

    if (dvbs == NULL)
        LOG_STRM_ERROUT("#%d dvbs is NULL\n", idx);

    if (idx < 0 || idx >= DVBS_INSTANCE_NUM)
        LOG_STRM_ERROUT("#%d\n", idx);

    local_loop_begin(dvbs, idx, msgq);

    dvbs->index = idx;
    g_array[idx] = dvbs;

    if (local_play_open(dvbs, arg, (DvbsArg*)argbuf))
        LOG_STRM_ERROUT("#%d local_play_open\n", dvbs->index);

    local_loop(dvbs);
    LOG_STRM_PRINTF("#%d exit local_loop!\n", idx);

Err:
    local_loop_end(dvbs);
    return;
}

#ifdef INCLUDE_PVR
static void local_loop_record(void *handle, int idx, mid_msgq_t msgq, RecordArg *arg, char *argbuf)
{
    DVBS_t dvbs = (DVBS_t)handle;

    if (dvbs == NULL)
        LOG_STRM_ERROUT("#%d dvbs is NULL\n", idx);

    dvbs->index = idx;
    g_array[idx] = dvbs;

    if (local_record_open(dvbs, arg, (DvbsArg *)argbuf))
        LOG_STRM_ERROUT("#%d local_record_open\n", idx);

    local_loop(dvbs);
    LOG_STRM_PRINTF("#%d exit local_loop!\n", idx);

Err:
    local_loop_end(dvbs);
    return;
}
#endif

static void local_set_recordtime(DVBS_t dvbs, uint32_t rectime)
{
    LOG_STRM_DEBUG("#%d rectime = %u!\n", dvbs->index, rectime);
    dvbs->rec_time = rectime;
    stream_back_totaltime(dvbs->index, rectime);
}

static void local_set_currenttime(DVBS_t dvbs, uint32_t current)
{
    dvbs->time_current = current;
    stream_back_currenttime(dvbs->index, current);
}

static void dvbs_write_data(DVBS_t dvbs, char *buf, int len)
{
    if (dvbs->ca_param.pmtpid == 0) {
        LOG_STRM_WARN("#%d pmtpid is NULL\n", dvbs->ca_param.pmtpid);
        return;
    }

    if (dvbs->open_play && dvbs->state == STRM_STATE_IPTV) {
        int l;
        StrmBuffer* sb;

        while (len > 0) {
            sb = dvbs->sb;

            l = sb->size - sb->len;
            if (l > len)
                l = len;
            IND_MEMCPY(sb->buf + sb->len, buf, l);
            buf += l;
            len -= l;

            sb->len += l;
            if (sb->len >= sb->size) {
                sb->off = 0;
                strm_play_push(dvbs->strm_play, dvbs->index, &dvbs->sb);
                sb = dvbs->sb;
                sb->len = 0;
            }
        }
    }

#ifdef INCLUDE_PVR
    if (dvbs->rec_mix || dvbs->open_shift) {
        if (dvbs->open_filter == 1)
            dvbs_port_record_push(dvbs->index, buf, len);
        strm_record_push(dvbs->strm_record, buf, len, 0);
    }
#endif

    if (dvbs->fp)
        fwrite(buf, 1, len, dvbs->fp);
}

static void dvbs_write_pat(DVBS_t dvbs)
{
    ind_ts_pat(dvbs->pat_buf, dvbs->pat_counter, dvbs->ca_param.prognum, dvbs->ca_param.pmtpid);
    dvbs_write_data(dvbs, dvbs->pat_buf, 188);
    dvbs->pat_counter ++;
}

static void dvbs_callback_writepmt(uint32_t magic, DvbCaParam_t param)
{
    int idx;
    DVBS_t dvbs;

    idx = (int)(magic >> 16);
    magic &= 0xffff;

    if (idx < 0 || idx >= DVBS_INSTANCE_NUM)
        LOG_STRM_ERROUT("#%d\n", idx);

    dvbs = g_array[idx];
    if (dvbs == NULL)
        LOG_STRM_ERROUT("#%d instance is NULL\n", idx);

    if (magic != (dvbs->dvbs_magic & 0xffff))
        LOG_STRM_ERROUT("#%d magic = %x, dvbs_magic = %x\n", idx, magic, dvbs->dvbs_magic);

    dvbs->ca_param.networkid = param->networkid;
    dvbs->ca_param.samefreq  = param->samefreq;
    dvbs->ca_param.prognum   = param->prognum;
    dvbs->ca_param.pmtpid    = param->pmtpid;

#ifdef INCLUDE_PVR
    if (param->networkid && (dvbs->rec_mix || dvbs->open_shift))
        strm_record_netwkid(dvbs->strm_record, param->networkid);
#endif

    if (dvbs->open_play) {
        if (STREAM_INDEX_PIP == dvbs->index)
            codec_dvbs(1, &dvbs->ca_param);
        else
            codec_dvbs(0, &dvbs->ca_param);
    }

Err:
    return;
}

static void dvbs_callback_writedata(uint32_t magic, uint8_t *data_buf, int data_len)
{
    uint32_t pid;
    int i, idx;
    DVBS_t dvbs;

    idx = (int)(magic >> 16);
    magic &= 0xffff;

    if (idx < 0 || idx >= DVBS_INSTANCE_NUM)
        LOG_STRM_ERROUT("#%d\n", idx);

    if (data_len % 188)
        LOG_STRM_ERROUT("#%d data_len = %d\n", idx, data_len);

    for (i = 0; i < data_len; i += 188) {
        if (data_buf[i] != 0x47)
            LOG_STRM_ERROUT("#%d data_buf[%d] = %u\n", idx, i, (uint32_t)((uint8_t)data_buf[i]));
    }

    dvbs = g_array[idx];
    if (dvbs == NULL)
        LOG_STRM_ERROUT("#%d instance is NULL\n", idx);

    mid_mutex_lock(dvbs->dvbs_mutex);

    if (magic != (dvbs->dvbs_magic & 0xffff))
        LOG_STRM_WARNOUT("#%d magic = %x, dvbs_magic = %x\n", idx, magic, dvbs->dvbs_magic);

    dvbs->write_times ++;
    dvbs->write_clk = dvbs->clk;

    dvbs->stat_bytes += data_len;

    for (i = 0; i < data_len; i += 188) {
        pid =(((uint32_t)data_buf[1] & 0x1f) << 8) + data_buf[2];
        if (pid == dvbs->ca_param.pmtpid && (data_buf[1] & 0x40)) {
            uint32_t prognum = ts_parse_pmt_prognum(data_buf);
            if (prognum == dvbs->ca_param.prognum) {
                dvbs->write_pmts ++;
                dvbs_write_pat(dvbs);
                break;
            }
        }
    }
    dvbs_write_data(dvbs, (char*)data_buf, data_len);

Warn:
    mid_mutex_unlock(dvbs->dvbs_mutex);

Err:
    return;
}

static int local_argparse_play(int idx, PlayArg* arg, char *argbuf, const char* url, int shiftlen, int begin, int end)
{
    DvbsArg *dvbsarg = (DvbsArg *)argbuf;

    if (url == NULL)
        LOG_STRM_ERROUT("#%d url is NULL\n", idx);

    LOG_STRM_PRINTF("#%d buildtime "__DATE__" "__TIME__"\n", idx);
    LOG_STRM_PRINTF("#%d url = %s, tuner = %d\n", idx, url, g_tunerArray[idx]);

    IND_STRCPY(dvbsarg->url, url);
    dvbsarg->shiftlen = shiftlen;

    dvbsarg->tuner = g_tunerArray[idx];
    g_tunerArray[idx] = 0;

    return 0;
Err:
    return -1;
}

static int local_argparse_record(int idx, RecordArg* arg, char *argbuf, const char* url)
{
    int i;
    DvbsArg *dvbsarg = (DvbsArg *)argbuf;

    switch (idx) {
    case 0:     i = STREAM_INDEX_REC0;    break;
    case 1:     i = STREAM_INDEX_REC1;    break;
    default:    LOG_STRM_ERROUT("#%d\n", idx);
    }

    dvbsarg->tuner = g_tunerArray[i];
    g_tunerArray[i] = 0;

    LOG_STRM_PRINTF("url = %s\n", url);
    IND_STRCPY(dvbsarg->url, url);

    return 0;
Err:
    return -1;
}

static int local_urlcmp(char *argbuf, const char* url, APP_TYPE apptype)
{
    DvbsArg *dvbsarg = (DvbsArg *)argbuf;

    return strcmp(dvbsarg->url, url);
}

int dvbs_create_stream(StreamCtrl *ctrl)
{
    DVBS_t dvbs;

    if (g_inited == 0) {
        dvbs_port_init( );
        g_inited = 1;
    }
    if (g_tunerArray == NULL) {
        g_tunerArray = (int*)IND_MALLOC(sizeof(int) * STREAM_INDEX_NUM);
        if (g_tunerArray)
            IND_MEMSET(g_tunerArray, 0, sizeof(int) * STREAM_INDEX_NUM);
    }

    dvbs = (DVBS_t)IND_MALLOC(sizeof(DVBS));
    if (dvbs == NULL)
        LOG_STRM_ERROUT("malloc failed!\n");
    IND_MEMSET(dvbs, 0, sizeof(DVBS));

    dvbs->state = STRM_STATE_CLOSE;

    dvbs->dvbs_mutex = mid_mutex_create( );

    ctrl->handle = dvbs;

    ctrl->loop_play = local_loop_play;
#ifdef INCLUDE_PVR
    ctrl->loop_record = local_loop_record;
#endif

    ctrl->argsize = sizeof(DvbsArg);
    ctrl->argparse_play = local_argparse_play;
    ctrl->argparse_record = local_argparse_record;
    ctrl->urlcmp = local_urlcmp;

    return 0;
Err:
    ctrl->handle = 0;
    return -1;
}

void mid_stream_set_tuner(int idx, int tuner)
{
    int i;

    LOG_STRM_PRINTF("#%d tuner = %d\n", idx, tuner);

    if (g_tunerArray == NULL)
        LOG_STRM_ERROUT("not inited!\n");

    switch (idx) {
    case 0:                 i = STREAM_INDEX_PLAY;  break;
    case 1:                 i = STREAM_INDEX_PIP;   break;
    case STREAM_INDEX_ADV:  i = STREAM_INDEX_ADV;   break;
    default:                LOG_STRM_ERROUT("#%d\n", idx);
    }
    g_tunerArray[i] = tuner;
    LOG_STRM_PRINTF("#%d tuner[%d] = %d\n", idx, i, g_tunerArray[i]);

Err:
    return;
}

void mid_record_set_tuner(int idx, int tuner)
{
    int i;

    LOG_STRM_PRINTF("#%d tuner = %d\n", idx, tuner);

    if (g_tunerArray == NULL)
        LOG_STRM_ERROUT("not inited!\n");

    switch (idx) {
    case 0:     i = STREAM_INDEX_REC0;  break;
    case 1:     i = STREAM_INDEX_REC1;  break;
    default:    LOG_STRM_ERROUT("#%d\n", idx);
    }

    g_tunerArray[i] = tuner;
    LOG_STRM_PRINTF("#%d tuner[%d] = %d\n", idx, i, g_tunerArray[i]);

Err:
    return;
}

#endif//INCLUDE_DVBS
