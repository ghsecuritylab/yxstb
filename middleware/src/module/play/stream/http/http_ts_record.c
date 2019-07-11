/**************************************--**************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2012-3-21 12:24:58 by liujianhua

 **************************************--**************************************/

#ifdef INCLUDE_PVR

#include "http.h"
#include "http_ts.h"

#include "ind_pvr.h"

/*
    暂时录制不能转播放原因：
        书签播放进入录制，然后关闭播放，再从头播放，如果复用录制，那么就不可能播VOD开头部分。
 */

struct tagHttpTsRecord {
    HttpTs*         ht;
    HTTP_t          http;

    int             index;
    int             state;
    int             scale;

    int             diff;
    int             open_play;
    int             open_record;

    PvrElem_t       pvr;
    StrmBuffer*     pvr_sb;

    int             msg_flg;
    int             psi_flg;
    int             end_flg;//1 recv end, 2 push end
    int             first_flg;//第一次缓冲
    int             buffer_flg;//1 缓冲 2 缓冲并上报缓冲消息

    uint32_t            warn_clk;

    long long       bmark_byte;
    uint32_t            bmark_time;

    StreamMsgQ*     strm_msgq;
    StreamPlay*     strm_play;
    StreamRecord*   strm_record;

    int             rec_index;
    uint32_t            rec_id;
    uint32_t            rec_wait;
    uint32_t            rec_time;
    RecordArg       rec_arg;
    int             rec_end;

    uint32_t            recv_clk;
    uint32_t            recv_times;

    int             check_bytes;
    uint32_t            check_clk;
    int             stat_bytes;

    int             bitrate;
    int             bandwidth;
    uint32_t            adjust_clk;

    uint32_t            time_base;
    uint32_t            time_length;
    uint32_t            time_current;

    int                 rcall_handle;
    RecordCall_open     rcall_open;
    RecordCall_push     rcall_push;
    RecordCall_close    rcall_close;
};
typedef struct tagHttpTsRecord  HttpTsRecord;

static void int_1000ms(void* handle);
static int  int_error_record(HttpTsRecord* htr, STRM_MSG msgno);
static void int_close_record(HttpTsRecord* htr, int complete);

static void int_state(HttpTsRecord* htr, STRM_STATE state, int scale)
{
    htr->state = state;
    htr->scale = scale;
    stream_post_state(htr->index, state, scale);
}

static void int_msgback(void* handle, STRM_MSG msgno, int arg)
{
    StreamMsgQ* strm_msgq = (StreamMsgQ*)handle;
    strm_msgq_queue(strm_msgq, msgno, arg);
}

//上报缓冲百分比
static void int_bufferrate(HttpTsRecord* htr)
{
    int diff = htr->rec_time - (htr->time_current - htr->bmark_time);
    if (diff < 0)
        diff = 0;

    if (0 == htr->buffer_flg || diff >= 15)
        int_back_hls_bufferrate(100);
    else
        int_back_hls_bufferrate(diff * 100 / 15);
}

static void int_buffer_begin(HttpTsRecord* htr, int rate)
{
    htr->recv_clk = strm_httploop_clk(htr->ht->loop);
    htr->recv_times = strm_http_get_recvTimes(htr->http);

    htr->buffer_flg = 2;
    if (rate)
        int_bufferrate(htr);
    stream_post_msg(htr->index, HLS_MSG_BUFFER_BEGIN, 0);
}

static void int_buffer_end(HttpTsRecord* htr)
{
    if (!htr->recv_clk) {
        LOG_STRM_ERROR("#%d STRM_MSG_RECV_RESUME\n", htr->index);
        stream_post_msg(htr->index, STRM_MSG_RECV_RESUME, 0);
    }
    htr->first_flg = 0;
    htr->buffer_flg = 0;
    int_bufferrate(htr);
    stream_post_msg(htr->index, HLS_MSG_BUFFER_END, 0);
}

static int int_recv_space(void* handle)
{
    int space;
    HttpTsRecord *htr = (HttpTsRecord*)handle;

    if (3 == htr->open_record)
        space = 1316;
    else
        space = strm_record_space(htr->strm_record);

    return space;
}

static int int_recv_push(void* handle)
{
    int len;
    HttpTs *ht;
    StrmBuffer* sb;
    HttpTsRecord *htr = (HttpTsRecord*)handle;

    ht = htr->ht;
    sb = ht->sb;

    if (sb->len <= 0)
        return 0;

    if (htr->check_clk)
        htr->check_bytes += sb->len;

    htr->stat_bytes += sb->len;

    len = sb->len;
    if (3 == htr->open_record) {
        if (len > 1316)
            len = 1316;
        htr->rcall_push(htr->rcall_handle, sb->buf + sb->off, len);
    } else {
        int space = strm_record_space(htr->strm_record);
        if (len > space)
            len = space;

        strm_record_push(htr->strm_record, sb->buf + sb->off, len, 0);
    }
    sb->off += len;
    sb->len -= len;

    return len;
}

static int int_record_arg(HttpTsRecord* htr)
{
    if (1 != htr->open_record)
        LOG_STRM_ERROUT("#%d open_record = %d\n", htr->index, htr->open_record);

    if (htr->rec_arg.pvrarg.byte_length > 0) {
        int length, size;

        length = (int)(htr->rec_arg.pvrarg.byte_length / 1024 / 1024);
        size = strm_record_size( );
        if (size == -1) {
            LOG_STRM_ERROR("#%d RECORD_MSG_DISK_ERROR\n", htr->index);
            int_error_record(htr, RECORD_MSG_DISK_ERROR);
            return -1;
        }
        if (size == 0) {
            LOG_STRM_ERROR("#%d RECORD_MSG_DISK_FULL\n", htr->index);
            int_error_record(htr, RECORD_MSG_DISK_FULL);
            return -1;
        }
        if (length >= size) {
            LOG_STRM_ERROR("#%d RECORD_MSG_NOT_ENOUGH %d / %d\n", htr->index, length, size);
            int_error_record(htr, RECORD_MSG_NOT_ENOUGH);
            return -1;
        }
    }

    strm_record_arg(htr->strm_record, &htr->rec_arg.pvrarg);
    htr->open_record = 2;

    return 0;
Err:
    return -1;
}

static int file_recv_begin(void* handle)
{
    long long content_length;
    HttpTsRecord *htr = (HttpTsRecord*)handle;

    content_length = strm_http_get_contentLength(htr->http);
    if (1 == htr->open_play) {
        uint32_t totalMB, freeMB = 0;

        record_port_disk_size(&totalMB, &freeMB);

        if (freeMB <= (uint32_t)(content_length / 1024 / 1024) + ind_pvr_fragment( ))
            stream_post_msg(htr->index, HLS_MSG_BUFFER_DISKERROR, 0);
    }
    if (0 < content_length) {//断点续传
        htr->rec_arg.pvrarg.byte_length = content_length;
        LOG_STRM_PRINTF("#%d content_length = %lld!\n", htr->index, content_length);

        if (htr->open_play && 0 < htr->bitrate) {
            htr->time_length = content_length / (htr->bitrate / 8);
            LOG_STRM_PRINTF("#%d time_length = %d!\n", htr->index, htr->time_length);
            stream_back_totaltime(htr->index, htr->time_length);
        }
    }

    if (1 == htr->open_record)
        int_record_arg(htr);

    if (1 == htr->buffer_flg) {
        LOG_STRM_PRINTF("#%d HLS_MSG_BUFFER_BEGIN\n", htr->index);
        int_buffer_begin(htr, 0);
    }

    return 0;
}

static void file_recv_end(void* handle, int end)
{
    HttpTsRecord *htr = (HttpTsRecord*)handle;

    LOG_STRM_PRINTF("#%d end = %d, open_record = %d\n", htr->index, end, htr->open_record);
    if (end != 2)
        return;
    htr->end_flg = 1;

    if (3 == htr->open_record)
        int_msgback(htr->strm_msgq, RECORD_MSG_SUCCESS_END, 0);
    else if (htr->open_record)
        strm_record_close(htr->strm_record, 1);
    else
        strm_record_end(htr->strm_record);
}

static void slice_recv_begin(void* handle)
{
    HttpTs *ht;
    HLiveMedia *media;
    HttpTsRecord *htr = (HttpTsRecord*)handle;

    ht = htr->ht;
    media = &ht->hls->media;

    if (htr->open_play && htr->time_length < media->total_duration) {
        htr->time_length = media->total_duration;
        if (htr->time_length < htr->bmark_time) {
            LOG_STRM_ERROR("#%d time_length = %d, time_start = %d!\n", htr->index, htr->time_length, htr->bmark_time);
            int_error_record(htr, RECORD_MSG_ERROR);
            return;
        }
        LOG_STRM_PRINTF("#%d time_length = %d, time_start = %d!\n", htr->index, htr->time_length, htr->bmark_time);
        stream_back_totaltime(htr->index, htr->time_length);
    }

    //ht->hls_index
    if (0 == htr->rec_arg.pvrarg.breaktype) {
        uint32_t byterate;
        int duration;
        long long content_length;

        htr->rec_arg.pvrarg.time_length = media->total_duration;

        content_length = strm_http_get_contentLen(htr->http);
        duration = http_ts_slice_duration(htr->ht);
        if (content_length <= 0 || duration <= 0) {
            byterate = 8 * 1024 * 1024;
            LOG_STRM_PRINTF("#%d Unknown byterate!\n", htr->index);
        } else  {
            byterate = (uint32_t)(content_length / duration);
            LOG_STRM_PRINTF("#%d byterate = %u\n", htr->index, byterate);
        }
        htr->rec_arg.pvrarg.byte_rate = byterate;
        htr->rec_arg.pvrarg.breaktype = 1;
    }

    if (1 == htr->open_record)
        int_record_arg(htr);

    if (1 == htr->buffer_flg) {
        LOG_STRM_PRINTF("#%d HLS_MSG_BUFFER_BEGIN\n", htr->index);
        int_buffer_begin(htr, 0);
    }
}

static void slice_recv_end(void* handle)
{
    HttpTsRecord *htr = (HttpTsRecord*)handle;

    LOG_STRM_PRINTF("#%d open_record = %d\n", htr->index, htr->open_record);
    if (1 == htr->open_record || 2 == htr->open_record)
            strm_record_break_point(htr->strm_record);
}

static int int_pause(HttpTsRecord* htr)
{
    LOG_STRM_PRINTF("#%d state = %d\n", htr->index, htr->state);

    switch(htr->state) {
    case STRM_STATE_PLAY:
        if (htr->buffer_flg)
            break;
        strm_play_pause(htr->strm_play, htr->index);
        break;
    case STRM_STATE_FAST:
        int_1000ms(htr);
        strm_play_reset(htr->strm_play, htr->index, 1);
        strm_play_pause(htr->strm_play, htr->index);

        htr->time_base = htr->time_current - htr->bmark_time;
        if (htr->pvr)
            ind_pvr_play(htr->pvr, htr->time_base, 1);
        break;
    default:
        LOG_STRM_ERROUT("#%d state = %d\n", htr->index, htr->state);
    }

    LOG_STRM_DEBUG("#%d STRM_STATE_PAUSE\n", htr->index);
    int_state(htr, STRM_STATE_PAUSE, 1);

    return 0;
Err:
    return -1;
}

static int int_resume(HttpTsRecord* htr)
{
    LOG_STRM_PRINTF("#%d state = %d\n", htr->index, htr->state);

    switch(htr->state) {
    case STRM_STATE_PAUSE:
        if (htr->buffer_flg)
            break;
        strm_play_resume(htr->strm_play, htr->index, 0);
        if (htr->pvr)
            ind_pvr_play(htr->pvr, -1, 1);
        break;
    case STRM_STATE_FAST:
        int_1000ms(htr);
        strm_play_reset(htr->strm_play, htr->index, 1);
        strm_play_resume(htr->strm_play, htr->index, 0);

        htr->time_base = htr->time_current - htr->bmark_time;
        if (htr->pvr)
            ind_pvr_play(htr->pvr, htr->time_base, 1);
        break;
    default:
        LOG_STRM_ERROUT("#%d state = %d\n", htr->index, htr->state);
    }

    if (htr->end_flg)
        htr->end_flg = 1;
    LOG_STRM_DEBUG("#%d STRM_STATE_PLAY end_flg = %d\n", htr->index, htr->end_flg);
    int_state(htr, STRM_STATE_PLAY, 1);

    return 0;
Err:
    return -1;
}

static int int_fast(HttpTsRecord* htr, int scale)
{
    LOG_STRM_PRINTF("#%d state = %d, scale = %d\n", htr->index, htr->state, scale);

    if (htr->buffer_flg) {
        LOG_STRM_DEBUG("#%d STRM_STATE_PLAY buffer_flg = %d\n", htr->index, htr->buffer_flg);
        int_state(htr, STRM_STATE_PLAY, 1);
        return 0;
    }

    int_1000ms(htr);
    strm_play_reset(htr->strm_play, htr->index, 1);
    strm_play_tplay(htr->strm_play, htr->index, scale);

    htr->time_base = htr->time_current - htr->bmark_time;
    if (htr->pvr)
        ind_pvr_play(htr->pvr, htr->time_base, scale);

    if (htr->end_flg)
        htr->end_flg = 1;
    LOG_STRM_DEBUG("#%d STRM_STATE_FAST scale = %d, end_flg = %d\n", htr->index, htr->end_flg, scale);
    int_state(htr, STRM_STATE_FAST, scale);

    return 0;
}

static int int_seek(HttpTsRecord* htr, uint32_t seek)
{
    uint32_t time_length;

    time_length = htr->bmark_time + htr->rec_time;
    LOG_STRM_PRINTF("#%d state = %d, seek = %d / (%d, %d / %d)\n", htr->index, htr->state, seek, htr->bmark_time, time_length, htr->time_length);

    if (seek + 3 >= time_length) { //一键到尾
        if (htr->rec_time > 15)
            seek = time_length - 15;
        else
            seek = htr->bmark_time;
    } else if (seek < htr->bmark_time) {
        LOG_STRM_WARN("seek = %d / (%d, %d) pvr = %p\n", seek, htr->bmark_time, time_length, htr->pvr);
        seek = htr->bmark_time;
    }

    strm_play_reset(htr->strm_play, htr->index, 1);

    htr->time_base = seek - htr->bmark_time;
    if (htr->pvr)
        ind_pvr_play(htr->pvr, htr->time_base, 1);

    if (htr->end_flg)
        htr->end_flg = 1;
    LOG_STRM_DEBUG("#%d STRM_STATE_PLAY end_flg = %d\n", htr->index, htr->end_flg);
    int_state(htr, STRM_STATE_PLAY, 1);

    {
        int diff = htr->rec_time - (seek - htr->bmark_time);
        if (diff < 15 && 0 == htr->end_flg) {
            strm_play_pause(htr->strm_play, htr->index);
            LOG_STRM_PRINTF("#%d HLS_MSG_BUFFER_BEGIN\n", htr->index);
            int_buffer_begin(htr, 1);
        } else {
            strm_play_resume(htr->strm_play, htr->index, 0);
            if (htr->buffer_flg) {
                LOG_STRM_PRINTF("#%d HLS_MSG_BUFFER_END\n", htr->index);
                int_buffer_end(htr);
            }
        }
    }

    return 0;
}

static int int_stop(HttpTsRecord* htr)
{
    uint32_t seek;

    if (htr->rec_time <= 3)
        seek = 0;
    else
        seek = htr->rec_time;

    LOG_STRM_DEBUG("#%d seek = %d, rec_time = %d\n", htr->index, seek, htr->rec_time);

    int_seek(htr, seek);

    return 0;
}

static void int_trickmode(HttpTsRecord* htr, int cmd, int arg)
{
    if (0 == htr->open_play) {
        LOG_STRM_ERROR("#%d open_play = 0\n", htr->index);
        return;
    }

    switch(cmd) {
    case STREAM_CMD_PAUSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_PAUSE open_play = %d\n", htr->index, htr->state);
        int_pause(htr);
        break;
    case STREAM_CMD_SEEK:
        LOG_STRM_PRINTF("#%d STREAM_CMD_SEEK open_play = %d\n", htr->index, htr->state);
        int_seek(htr, (uint32_t)arg);
        break;
    case STREAM_CMD_STOP:
        LOG_STRM_PRINTF("#%d STREAM_CMD_STOP open_play = %d\n", htr->index, htr->state);
        int_stop(htr);
        break;
    case STREAM_CMD_FAST:
        LOG_STRM_PRINTF("#%d STREAM_CMD_FAST open_play = %d\n", htr->index, htr->state);
        int_fast(htr, arg);
        break;
    case STREAM_CMD_RESUME:
        LOG_STRM_PRINTF("#%d STREAM_CMD_RESUME open_play = %d\n", htr->index, htr->state);
        int_resume(htr);
        break;
    default:
        break;
    }
}

//本地硬盘播放
static int int_open_play(HttpTsRecord* htr, PlayArg *arg, HttpTsArg *httparg)
{
    char *p;
    LOG_STRM_PRINTF("#%d\n", htr->index);

    htr->bmark_time = 0;
    htr->bmark_byte = 0;
    p = strstr(httparg->url, "starttime=");
    if(p)
        htr->bmark_time = atoi(p + 10);
    htr->time_base = 0;
    htr->time_current = htr->bmark_time + htr->time_base;

    htr->bitrate = 0;
    int_back_hls_bufferrate(0);
    p = strstr(httparg->url, "biteRate=");
    if(p)
        htr->bitrate = atoi(p + 9) * 1024;

    p = strstr(httparg->url, "m3u8");
    if (p)
        strm_http_set_etag(htr->http, 1);
    if (htr->bmark_time > 0) {
        if (p) {
            htr->ht->time_start = htr->bmark_time;
        } else {
            if (htr->bitrate <= 0)
                LOG_STRM_ERROUT("#%d time_start = %d, bitrate = %d\n", htr->index, htr->bmark_time, htr->bitrate);
            htr->bmark_byte = ((long long)htr->bmark_time * (htr->bitrate / 188) / 8) * 188;
        }
    }

    if (strm_http_request(htr->http, httparg->url, htr->bmark_byte, 0))
        LOG_STRM_ERROUT("#%d local_connect\n", htr->index);

    strm_play_open(htr->strm_play, htr->index, htr->strm_msgq, int_msgback, APP_TYPE_HLS, TS_TIMESHIFT_SIZE);
    htr->open_play = 1;

    strm_play_pause(htr->strm_play, htr->index);
    htr->first_flg = 1;
    htr->buffer_flg = 1;

    htr->rec_id = arg->shiftid;
    if (0 == htr->open_record) {
        RecordArg recArg;

        memset(&recArg, 0, sizeof(recArg));
        recArg.pvrarg.id = htr->rec_id;
        strm_record_open(htr->strm_record, htr->strm_msgq, int_msgback, &recArg);
    }

    htr->pvr = NULL;
    LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", htr->index);
    int_state(htr, STRM_STATE_PLAY, 1);

    return 0;
Err:
    LOG_STRM_PRINTF("#%d STRM_MSG_OPEN_ERROR\n", htr->index);
    stream_post_msg(htr->index, STRM_MSG_OPEN_ERROR, 0);
    return -1;
}

static void int_close_play(HttpTsRecord* htr)
{
    if (0 == htr->open_play)
        return;
    strm_play_close(htr->strm_play, htr->index, 1);
    htr->open_play = 0;
    int_back_hls_bufferrate(0);

    LOG_STRM_PRINTF("#%d STRM_STATE_CLOSE\n", htr->index);
    int_state(htr, STRM_STATE_CLOSE, 1);

    if (htr->pvr) {
        ind_pvr_close(htr->pvr);
        htr->pvr = NULL;
    }

    if (0 == htr->open_record)
        strm_record_close(htr->strm_record, 0);
}

static int int_open_record(HttpTsRecord* htr, RecordArg *arg, HttpTsArg *httparg)
{
    HttpTs *ht;
    struct PVRInfo info;

    LOG_STRM_PRINTF("#%d\n", htr->index);

    ht = htr->ht;
    htr->rec_index = arg->index;

    htr->rec_wait = 0;
    htr->rec_time = 0;
    htr->rec_end = 0;

    htr->rec_arg.add = arg->add;
    htr->rec_arg.pvrarg.id = arg->pvrarg.id;
    htr->rec_arg.pvrarg.info_len = arg->pvrarg.info_len;
    IND_MEMCPY(htr->rec_arg.pvrarg.info_buf, arg->pvrarg.info_buf, arg->pvrarg.info_len);

    htr->rec_arg.pvrarg.realtime = 0;

    strm_http_set_bitrate(htr->http, httparg->bitrate);

    if (arg->rcall_open && (htr->open_record || htr->open_play))
        LOG_STRM_ERROUT("#%d open_record = %d, open_play = %d\n", htr->index, htr->open_record, htr->open_play);

    if (htr->open_play) {
        if (arg->pvrarg.id != htr->rec_id) 
            LOG_STRM_ERROUT("#%d id = %08x / %08x\n", htr->index, arg->pvrarg.id, htr->rec_id);

        htr->open_record = 1;
        htr->rec_arg.pvrarg.time_bmark = htr->bmark_time;
        htr->rec_arg.pvrarg.byte_bmark = htr->bmark_byte;
        if (htr->rec_arg.pvrarg.byte_length > 0 || htr->rec_arg.pvrarg.time_length > 0) {
            if (int_record_arg(htr))
                LOG_STRM_ERROUT("#%d int_record_arg\n", htr->index);
        }
    } else {
        htr->rec_id = arg->pvrarg.id;

        if (htr->rec_arg.add == 1) {
            int breaktype;

            ind_pvr_rec_rebreak(htr->rec_id);

            if (ind_pvr_get_info(htr->rec_id, &info))
                LOG_STRM_ERROUT("#%d ind_pvr_get_info\n", htr->index);

            if (info.recording == 2) {
                LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_END\n", htr->index);
                record_back_close(htr->rec_id);
                record_post_msg(htr->rec_index, htr->rec_id, RECORD_MSG_SUCCESS_END, 0);
                return 1;
            }

            breaktype = ind_pvr_breaktype(htr->rec_id);
            LOG_STRM_PRINTF("#%d breaktype = %d\n", htr->index, breaktype);
            if (breaktype) {
                htr->bmark_byte = 0;
                if (breaktype == 1) {
                    ht->hls_index = ind_pvr_breaknum(htr->rec_id);
                    LOG_STRM_PRINTF("#%d hls_index = %d\n", htr->index, ht->hls_index);
                }
            } else {
                htr->bmark_byte = info.byte_bmark + info.byte_len;
            }
        } else {
            htr->bmark_byte = 0;
        }

        {
            char *p = strstr(httparg->url, "m3u8");
            if (p)
                strm_http_set_etag(htr->http, 1);
        }
        if (strm_http_request(htr->http, httparg->url, htr->bmark_byte, 0))
            LOG_STRM_ERROUT("#%d local_connect\n", htr->index);

        if (arg->rcall_open) {
            htr->rcall_open = arg->rcall_open;
            htr->rcall_push = arg->rcall_push;
            htr->rcall_close = arg->rcall_close;

            htr->rcall_handle = htr->rcall_open(arg->pvrarg.info_buf, arg->pvrarg.info_len, arg->pvrarg.id);
            if (-1 == htr->rcall_handle) {
                strm_httploop_break(ht->loop);
                LOG_STRM_ERROUT("#%d rcall_open\n", htr->index);
            }

            htr->open_record = 3;
        } else {
            int len = htr->rec_arg.pvrarg.info_len;
            htr->rec_arg.pvrarg.info_len = 0;
            strm_record_open(htr->strm_record, htr->strm_msgq, int_msgback, &htr->rec_arg);
            htr->rec_arg.pvrarg.info_len = len;

            htr->open_record = 1;
        }
    }

    if (htr->psi_flg) {
        LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_BEGIN\n", htr->index);
        record_post_msg(htr->rec_index, htr->rec_id, RECORD_MSG_SUCCESS_BEGIN, 0);
    }

    return 0;
Err:
    LOG_STRM_PRINTF("#%d RECORD_MSG_ERROR\n", htr->index);
    record_post_msg(htr->rec_index, htr->rec_id, RECORD_MSG_ERROR, 0);
    return -1;
}

static void int_close_record(HttpTsRecord* htr, int complete)
{
    if (0 == htr->open_record)
        return;

    if (htr->rec_end == -1)
        complete = -1;
    if (0 == htr->open_play) {
        if (3 == htr->open_record) {
            if (-1 != htr->rcall_handle) {
                htr->rcall_close(htr->rcall_handle);
                htr->rcall_handle = -1;
            }
        } else {
            strm_record_close(htr->strm_record, complete);
        }
    }

    htr->open_record = 0;
}

static int int_error_record(HttpTsRecord* htr, STRM_MSG msgno)
{
    if (htr->open_play) {
        stream_post_msg(htr->index, STRM_MSG_OPEN_ERROR, msgno);
        int_close_play(htr);
    }
    if (htr->open_record) {
        record_post_msg(htr->rec_index, htr->rec_id, msgno, 0);
        int_close_record(htr, 0);
    }
    strm_httploop_break(htr->ht->loop);

    return 0;
}

static void int_deal_error(void* handle, STRM_MSG msgno)
{
    HttpTsRecord *htr = (HttpTsRecord*)handle;

    int_error_record(htr, msgno);
}

static void int_deal_cmd(void* handle, StreamCmd* strmCmd)
{
    int cmd;
    HttpTs *ht;
    HttpTsRecord *htr = (HttpTsRecord*)handle;

    ht = htr->ht;
    cmd = strmCmd->cmd;

    switch(cmd) {
    case STREAM_CMD_RESUME:
    case STREAM_CMD_PAUSE:
    case STREAM_CMD_FAST:
    case STREAM_CMD_SEEK:
    case STREAM_CMD_STOP:
        {
            int cmdsn = strmCmd->arg3;
            int_trickmode(htr, cmd, strmCmd->arg0);
            if (cmdsn)
                stream_back_cmd(htr->index, cmdsn);
        }
        break;

    case STREAM_CMD_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE\n", htr->index);
        int_close_play(htr);
        if (0 == htr->open_record)
            strm_httploop_break(ht->loop);
        break;

    case STREAM_CMD_RECORD_OPEN:
        LOG_STRM_PRINTF("#%d STREAM_CMD_RECORD_OPEN play = %d, record = %d\n", htr->index, htr->open_play, htr->open_record);
        if (htr->open_record)
            break;
        {
            RecordArg *arg;
            char *argbuf;
            if (record_back_get_arg(htr->index, strmCmd->arg0, &arg, &argbuf))
                LOG_STRM_ERROUT("#%d stream_back_get_arg\n", htr->index);
            if (int_open_record(htr, arg, (HttpTsArg*)argbuf))
                LOG_STRM_ERROUT("#%d int_open_record\n", htr->index);
        }
        break;

    case STREAM_CMD_RECORD_CHECK_BANDWIDTH:
        LOG_STRM_PRINTF("#%d STREAM_CMD_RECORD_CHECK_BANDWIDTH\n", htr->index);
        htr->check_bytes = 0;
        htr->check_clk = strm_httploop_clk(ht->loop) + 500;
        break;

    case STREAM_CMD_RECORD_LIMIT_BANDWIDTH:
        LOG_STRM_PRINTF("#%d STREAM_CMD_RECORD_LIMIT_BANDWIDTH bitrate = %d\n", htr->index, strmCmd->arg0);
        htr->bandwidth = strmCmd->arg0;
        strm_http_set_limit(htr->http, htr->bandwidth / 8);
        break;

    case STREAM_CMD_RECORD_ADJUST_BANDWIDTH:
        {
            int bitrate, percent, second;

            percent = strmCmd->arg0;
            second = strmCmd->arg1;
            LOG_STRM_PRINTF("#%d STREAM_CMD_RECORD_ADJUST_BANDWIDTH bandwidth = %d, percent = %d, second = %d\n", htr->index, htr->bandwidth, percent, second);
            if (percent < 0 ||  percent > 100 || second <= 0)
                LOG_STRM_ERROUT("#%d percent = %d, second = %d", htr->index, percent, second);
            bitrate = htr->bandwidth * percent / 100;
            strm_http_set_limit(htr->http, bitrate / 8);

            if (second > 24 * 3600)
                second = 24 * 3600;
            htr->adjust_clk = strm_httploop_clk(ht->loop) + second * 100;
        }

        if (1 == htr->open_record || 2 == htr->open_record)
            strm_record_pause(htr->strm_record);
        break;

    case STREAM_CMD_WAIT_RECORD:
        LOG_STRM_PRINTF("#%d STREAM_CMD_WAIT_RECORD\n", htr->index);
        if (htr->rec_id != (uint32_t)strmCmd->arg0) {
            LOG_STRM_PRINTF("#%d id = 0x%x / 0x%x\n", htr->index, htr->rec_id, (uint32_t)strmCmd->arg0);
            stream_back_wake(0);
            break;
        }
        if (htr->psi_flg == 1) {
            stream_back_wake(htr->rec_id);
            break;
        }
        htr->rec_wait = 1;
        break;
    case STREAM_CMD_RECORD_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_RECORD_CLOSE arg = %d\n", htr->index, strmCmd->arg0);
        if (htr->open_record)
            int_close_play(htr);

        if (strmCmd->arg0 == -1)
            htr->rec_end = -1;
        if (0 == htr->open_play)
            strm_httploop_break(ht->loop);
        break;

    default:
        LOG_STRM_ERROR("#%d Unkown CMD %d\n", htr->index, cmd);
        break;
    }
Err:
    return;
}

static void int_deal_msg(void* handle, STRM_MSG msgno, int arg)
{
    HttpTsRecord *htr = (HttpTsRecord*)handle;

    switch(msgno) {
    case RECORD_MSG_ERROR:
        LOG_STRM_ERROR("#%d RECORD_MSG_ERROR\n", htr->index);
        int_error_record(htr, msgno);
        break;
    case RECORD_MSG_PVR_CONFLICT:
        LOG_STRM_ERROR("#%d RECORD_MSG_PVR_CONFLICT\n", htr->index);
        int_error_record(htr, msgno);
        break;
    case RECORD_MSG_DATA_DAMAGE:
        LOG_STRM_ERROR("#%d RECORD_MSG_DATA_DAMAGE\n", htr->index);
        int_error_record(htr, msgno);
        break;
    case RECORD_MSG_DISK_ERROR:
        LOG_STRM_ERROR("#%d RECORD_MSG_DISK_ERROR\n", htr->index);
        int_error_record(htr, msgno);
        break;
    case RECORD_MSG_DISK_FULL:
        LOG_STRM_ERROR("#%d RECORD_MSG_DISK_FULL\n", htr->index);
        int_error_record(htr, msgno);
        break;
    case RECORD_MSG_DISK_WARN:
        LOG_STRM_ERROR("#%d RECORD_MSG_DISK_WARN\n", htr->index);
        if (htr->open_record)
            record_post_msg(htr->rec_index, htr->rec_id, RECORD_MSG_DISK_WARN, 0);
        break;
    case RECORD_MSG_DATA_TIMEOUT:
        LOG_STRM_ERROR("#%d RECORD_MSG_DATA_TIMEOUT\n", htr->index);
        if (htr->open_record)
            record_post_msg(htr->rec_index, htr->rec_id, RECORD_MSG_DATA_TIMEOUT, 0);
        break;
    case RECORD_MSG_DATA_RESUME:
        LOG_STRM_ERROR("#%d RECORD_MSG_DATA_RESUME\n", htr->index);
        if (htr->open_record)
            record_post_msg(htr->rec_index, htr->rec_id, RECORD_MSG_DATA_RESUME, 0);
        break;
    case RECORD_MSG_SUCCESS_BEGIN:
        LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_BEGIN\n", htr->index);
        if (htr->open_record)
            record_post_msg(htr->rec_index, htr->rec_id, RECORD_MSG_SUCCESS_BEGIN, 0);

        htr->psi_flg = 1;
        if (htr->rec_wait == 1) {
            stream_back_wake(htr->rec_id);
            htr->rec_wait = 0;
            break;
        }

        if (htr->open_play) {
            if (ind_pvr_open(htr->rec_id, &htr->pvr)) {
                int_close_play(htr);
                LOG_STRM_ERROUT("#%d ind_pvr_open\n", htr->index);
            }
            ind_pvr_play(htr->pvr, htr->time_base, 1);
            stream_post_msg(htr->index, RECORD_MSG_SUCCESS_BEGIN, 0);
        }

        break;
    case RECORD_MSG_SUCCESS_END:
        LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_END\n", htr->index);
        if (0 == htr->open_play)
            strm_httploop_break(htr->ht->loop);
        if (htr->open_record) {
            record_back_close(htr->rec_id);
            record_post_msg(htr->rec_index, htr->rec_id, RECORD_MSG_SUCCESS_END, 0);
        }
        break;

    case RECORD_MSG_CLOSE:
        LOG_STRM_PRINTF("#%d RECORD_MSG_CLOSE\n", htr->index);
        if (htr->open_record)
            record_post_msg(htr->rec_index, htr->rec_id, msgno, 0);
        break;

    case RECORD_MSG_PSI_VIEW:
        LOG_STRM_PRINTF("#%d RECORD_MSG_PSI_VIEW\n", htr->index);
        break;

    case STRM_MSG_STREAM_BEGIN:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_BEGIN\n", htr->index);
        stream_post_msg(htr->index, STRM_MSG_STREAM_BEGIN, 0);
        break;
    case STRM_MSG_STREAM_END:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", htr->index);
        stream_post_msg(htr->index, STRM_MSG_STREAM_END, 0);
        break;

    case STRM_MSG_RECV_TIMEOUT:
    case STRM_MSG_RECV_RESUME:
    case STRM_MSG_STREAM_MUSIC:
    case STRM_MSG_STREAM_VIDEO:
    case STRM_MSG_PSI_VIEW:
    case STRM_MSG_PTS_VIEW:
        break;

    case STRM_MSG_CHANGE_PSI:
        LOG_STRM_PRINTF("#%d STRM_MSG_CHANGE_PSI\n", htr->index);
        break;
    case STRM_MSG_CHANGE_CRC:
        LOG_STRM_PRINTF("#%d STRM_MSG_CHANGE_CRC\n", htr->index);
        break;

    default:
        LOG_STRM_ERROR("#%d unknown record message %d\n", htr->index, msgno);
        break;
    }
Err:
    return;
}

static void int_100ms(void* handle)
{
    uint32_t clk;
    HttpTsRecord *htr = (HttpTsRecord*)handle;

    clk = strm_httploop_clk(htr->ht->loop);

    if (htr->adjust_clk && htr->adjust_clk < clk) {
        strm_http_set_limit(htr->http, htr->bandwidth / 8);
        htr->adjust_clk = 0;
        if (1 == htr->open_record || 2 == htr->open_record)
            strm_record_resume(htr->strm_record);
    }

    if (htr->check_clk && htr->check_clk < clk) {
        htr->check_clk = 0;
        record_post_msg(htr->rec_index, htr->rec_id, RECORD_MSG_BANDWIDTH, htr->check_bytes / 5);
    }

    {
        int ret;
        StrmBuffer* sb;

        if (htr->buffer_flg)
            return;

        if (htr->state != STRM_STATE_PLAY && htr->state != STRM_STATE_FAST)
            return;

        if (htr->end_flg == 2)
            return;

        htr->diff = strm_play_buffer(htr->strm_play);
        for (;;) {
            if (strm_play_space(htr->strm_play) < TS_TIMESHIFT_SIZE)
                break;

            sb = htr->pvr_sb;
            if (htr->diff < 300)
                ret = ind_pvr_read(htr->pvr, clk * 10 / 9, sb->buf, sb->size);//加速取数据
            else
                ret = ind_pvr_read(htr->pvr, clk, sb->buf, sb->size);

            if (ret > 0) {
                sb->off = 0;
                sb->len = ret;
                strm_play_push(htr->strm_play, htr->index, &htr->pvr_sb);

                if (strm_msgq_valid(htr->strm_msgq))
                    return;
                continue;
            }

            switch(ret) {
            case PVR_ANNOUNCE_NONE:
                //LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_NONE\n", local->index);
                return;
            case PVR_ANNOUNCE_BEGIN:
                LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_BEGIN > STRM_MSG_STREAM_BEGIN\n", htr->index);
                stream_post_msg(htr->index, STRM_MSG_STREAM_BEGIN, 0);
                int_seek(htr, 0);
                return;
            case PVR_ANNOUNCE_END:
            case PVR_ANNOUNCE_WRITE:
                if (htr->end_flg) {
                    if (htr->scale != 1) {
                        strm_play_set_idle(htr->strm_play, htr->index, 1);
                        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", htr->index);
                        stream_post_msg(htr->index, STRM_MSG_STREAM_END, 0);
                    } else {
                        strm_play_end(htr->strm_play, htr->index);
                    }
                    htr->end_flg = 2;
                    LOG_STRM_PRINTF("#%d end_flg = %d\n", htr->index, htr->end_flg);
                } else {
                    if (STRM_STATE_FAST == htr->state) {
                        LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_WRITE > STRM_STATE_PLAY\n", htr->index);
                        int_seek(htr, htr->bmark_time + htr->rec_time);
                    }
                }
                return;
            case PVR_ANNOUNCE_ERROR:
                LOG_STRM_PRINTF("#%d PVR_ANNOUNCE_ERROR > STRM_MSG_OPEN_ERROR\n", htr->index);
                stream_post_msg(htr->index, STRM_MSG_OPEN_ERROR, 0);
                int_close_play(htr);
                return;
            default:
                LOG_STRM_PRINTF("#%d ret = %d\n", htr->index, ret);
                return;
            }
        }
    }
}

static void int_1000ms(void* handle)
{
    uint32_t clk;
    HttpTsRecord *htr = (HttpTsRecord*)handle;

    if (htr->open_record) {
        int rate = htr->stat_bytes / 10 / INTERVAL_CLK_1000MS;
        htr->stat_bytes = 0;
        record_port_byterate(htr->rec_index, htr->rec_id, rate);
        if (3 == htr->open_record)
            return;
    }

    clk = strm_httploop_clk(htr->ht->loop);

    htr->rec_time = ind_pvr_get_time(htr->rec_id);

    if (htr->open_play) {
        int current, pdiff, rdiff;

        if (htr->end_flg)
            int_back_hls_recordrate(htr->time_length);
        else
            int_back_hls_recordrate(htr->bmark_time + htr->rec_time);

        current = strm_play_time(htr->strm_play);
        pdiff = strm_play_buffer(htr->strm_play);
        current = htr->time_base + current / 100;
        if (current < 0)
            current = 0;

        htr->time_current = htr->bmark_time + current;
        stream_back_currenttime(htr->index, htr->time_current);

        rdiff = htr->rec_time - (htr->time_current - htr->bmark_time);
        LOG_STRM_DEBUG("#%d: sec = %d, pdiff = %d, rdiff = %d, time_base = %d, rec_time = %d\n", htr->index, current, pdiff, rdiff, htr->time_base, htr->rec_time);
        if (rdiff < 15 && 0 == htr->end_flg) {
			if (STRM_STATE_FAST == htr->state) {
				LOG_STRM_PRINTF("#%d rdiff = %d > STRM_STATE_PLAY\n", htr->index, rdiff);
				int_seek(htr, htr->bmark_time + htr->rec_time - 15);
				return;
			}
            if (htr->warn_clk) {
                if (htr->first_flg) {
                    if (0 == htr->msg_flg && clk - htr->warn_clk >= 1600 && rdiff < 12) {
                        LOG_STRM_PRINTF("#%d: HLS_MSG_BUFFER_BANDWIDTH\n", htr->index);
                        stream_post_msg(htr->index, HLS_MSG_BUFFER_BANDWIDTH, 0);
                        htr->msg_flg = 1;
                    }
                } else {
                    if (0 == htr->msg_flg && clk - htr->warn_clk >= 1200) {
                        LOG_STRM_PRINTF("#%d: HLS_MSG_BUFFER_LIMITED\n", htr->index);
                        stream_post_msg(htr->index, HLS_MSG_BUFFER_LIMITED, 0);
                        htr->msg_flg = 1;
                    }
                }
            } else {
                htr->warn_clk = clk;
            }
        } else {
            htr->msg_flg = 0;
            htr->warn_clk = 0;
        }
        if (htr->buffer_flg) {
            int_bufferrate(htr);
            if (htr->pvr) {
                LOG_STRM_PRINTF("#%d buffer second = %d\n", htr->index, rdiff);
                if (rdiff >= 15 || 1 == htr->end_flg) {
                    if (STRM_STATE_PLAY == htr->state) {
                        strm_play_resume(htr->strm_play, htr->index, 0);
                        ind_pvr_play(htr->pvr, -1, 1);
                    }
                    LOG_STRM_PRINTF("#%d HLS_MSG_BUFFER_END\n", htr->index);
                    int_buffer_end(htr);
                }
            } else {
                LOG_STRM_PRINTF("#%d buffer second = 0\n", htr->index);
            }
        } else {
            if (rdiff <= 3 && 0 == htr->end_flg) {
                if (STRM_STATE_FAST == htr->state || STRM_STATE_PLAY == htr->state) {
                    if (STRM_STATE_FAST == htr->state) {
                        htr->time_base = htr->time_current - htr->bmark_time;
                        if (htr->time_base >= htr->rec_time)
                            htr->time_base = htr->rec_time;
                        if (htr->pvr)
                            ind_pvr_play(htr->pvr, htr->time_base, 1);
                        strm_play_reset(htr->strm_play, htr->index, 1);

                        LOG_STRM_DEBUG("#%d STRM_STATE_PLAY\n", htr->index);
                        int_state(htr, STRM_STATE_PLAY, 1);
                    }
                    strm_play_pause(htr->strm_play, htr->index);
                    LOG_STRM_PRINTF("#%d HLS_MSG_BUFFER_BEGIN\n", htr->index);
                    int_buffer_begin(htr, 1);
                }
            }
        }
        if (2 == htr->buffer_flg) {
            uint32_t recv_times;

            recv_times = strm_http_get_recvTimes(htr->http);
            if (htr->recv_clk) {
                if (recv_times == htr->recv_times) {
                    if (clk - htr->recv_clk >= 500) {
                        htr->recv_clk = 0;
                        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_TIMEOUT\n", htr->index);
                        stream_post_msg(htr->index, STRM_MSG_RECV_TIMEOUT, 0);
                    }
                } else {
                    htr->recv_clk = clk;
                    htr->recv_times = recv_times;
                }
            } else {
                if (recv_times != htr->recv_times) {
                    htr->recv_clk = clk;
                    htr->recv_times = recv_times;
                    LOG_STRM_PRINTF("#%d STRM_MSG_RECV_RESUME\n", htr->index);
                    stream_post_msg(htr->index, STRM_MSG_RECV_RESUME, 0);
                }
            }
        }
    }
}

static void int_destroy(void* handle)
{
    HttpTsRecord *htr = (HttpTsRecord*)handle;

    if (htr->rec_wait) {
        stream_back_wake(0);
        htr->rec_wait = 0;
    }

    int_close_record(htr, 0);

    LOG_STRM_PRINTF("#%d RECORD_MSG_CLOSE\n", htr->index);
    record_post_msg(htr->rec_index, htr->rec_id, RECORD_MSG_CLOSE, 0);

    {
        HttpTs *ht = htr->ht;

        if (htr->pvr_sb)
            strm_buf_free(htr->pvr_sb);
        IND_FREE(htr);
        ht->ts_handle = NULL;
    }
}

void* http_ts_record_create(HttpTs* ht, PlayArg *plyArg, RecordArg *recArg, HttpTsArg *httparg)
{
    HttpTsRecord *htr = (HttpTsRecord*)IND_CALLOC(sizeof(HttpTsRecord), 1);
    if (!htr)
        LOG_STRM_ERROUT("#%d malloc failed!\n", ht->index);

    htr->ht = ht;
    htr->http = ht->http;
    htr->index = ht->index;

    htr->bandwidth = 8 * 1024 * 1024;

    htr->strm_msgq = int_strm_msgq(htr->index);
    htr->strm_play = int_strm_play(htr->index);
    htr->strm_record = int_strm_record(htr->index);

    htr->rcall_handle = -1;

    ht->sync_flg = 1;

    ht->ts_op.ht_recv_space = int_recv_space;
    ht->ts_op.ht_recv_push = int_recv_push;

    ht->ts_op.file_recv_begin = file_recv_begin;
    ht->ts_op.file_recv_end = file_recv_end;
    ht->ts_op.slice_recv_begin = slice_recv_begin;
    ht->ts_op.slice_recv_end = slice_recv_end;

    ht->ts_op.ht_deal_error = int_deal_error;
    ht->ts_op.ht_deal_cmd = int_deal_cmd;
    ht->ts_op.ht_deal_msg = int_deal_msg;

    ht->ts_op.ht_100ms = int_100ms;
    ht->ts_op.ht_1000ms = int_1000ms;

    ht->ts_op.ht_destroy = int_destroy;

    htr->pvr_sb = strm_buf_malloc(TS_TIMESHIFT_SIZE);

    if (plyArg) {
        if (int_open_play(htr, plyArg, httparg))
            LOG_STRM_ERROUT("#%d int_open_play\n", htr->index);
    } else if (recArg) {
        if (int_open_record(htr, recArg, httparg))
            LOG_STRM_ERROUT("#%d int_open_record\n", htr->index);
    } else {
        LOG_STRM_ERROUT("#%d\n", htr->index);
    }

    return htr;
Err:
    if (htr)
        int_destroy(htr);
    return NULL;
}
#endif//INCLUDE_PVR
