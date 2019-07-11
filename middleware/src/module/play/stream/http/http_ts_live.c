/**************************************--**************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2012-3-20 23:56:17 by liujianhua

 **************************************--**************************************/

#include "http.h"
#include "http_ts.h"

struct tagHttpTsLive {
    HttpTs*         ht;
    HTTP_t          http;

    int             index;
    int             state;
    int             scale;

    int             diff;

    uint32_t            recv_clk;
    uint32_t            recv_times;

    int             stop_flg;//
    int             first_flg;//第一次缓冲
    int             buffer_flg;//1 缓冲 2 缓冲并上报缓冲消息

    int             msg_flg;
    uint32_t            warn_clk;

    StreamMsgQ*     strm_msgq;
    StreamPlay*     strm_play;
    StreamRecord*   strm_record;

    int             byte_rate;

    uint32_t            time_base;
    uint32_t            time_length;
    uint32_t            time_current;
};
typedef struct tagHttpTsLive    HttpTsLive;

static void int_state(HttpTsLive* htl, STRM_STATE state, int scale)
{
    htl->state = state;
    htl->scale = scale;
    stream_post_state(htl->index, state, scale);
}

static void int_msgback(void* handle, STRM_MSG msgno, int arg)
{
    StreamMsgQ* strm_msgq = (StreamMsgQ*)handle;
    strm_msgq_queue(strm_msgq, msgno, arg);
}

//上报缓冲百分比
static void int_bufferrate(HttpTsLive* htl, int diff)
{
    if (0 == htl->buffer_flg || diff >= 15)
        int_back_hls_bufferrate(100);
    else
        int_back_hls_bufferrate(diff * 100 / 15);
}

static void int_buffer_begin(HttpTsLive* htl, int diff)
{
    htl->recv_clk = strm_httploop_clk(htl->ht->loop);
    htl->recv_times = strm_http_get_recvTimes(htl->http);

    htl->buffer_flg = 2;
    int_bufferrate(htl, diff);
    if (!htl->stop_flg)
        stream_post_msg(htl->index, HLS_MSG_BUFFER_BEGIN, 0);
}

static void int_buffer_end(HttpTsLive* htl)
{
    if (!htl->recv_clk) {
        LOG_STRM_ERROR("#%d STRM_MSG_RECV_RESUME\n", htl->index);
        stream_post_msg(htl->index, STRM_MSG_RECV_RESUME, 0);
    }

    htl->first_flg = 0;
    htl->buffer_flg = 0;
    if (!htl->stop_flg)
        stream_post_msg(htl->index, HLS_MSG_BUFFER_END, 0);
}

static int int_recv_space(void* handle)
{
    int space = 0;

    HttpTsLive* htl = (HttpTsLive*)handle;

    if (!htl->buffer_flg && STRM_STATE_PLAY == htl->state)
        space = strm_play_space(htl->strm_play);

    return space;
}

static int int_recv_push(void* handle)
{
    int l;
    HttpTs *ht;
    HttpTsLive* htl = (HttpTsLive*)handle;

    ht = htl->ht;

    l = ht->sb->len;
    if (l > 0) {
        strm_play_push(htl->strm_play, htl->index, &ht->sb);
        ht->sb->len = 0;
    }

    return l;
}

static int file_recv_begin(void* handle)
{
    HttpTsLive* htl = (HttpTsLive*)handle;

    if (htl->byte_rate > 0) {
        long long content_length = strm_http_get_contentLength(htl->http);

        htl->time_length = content_length / htl->byte_rate;
        LOG_STRM_PRINTF("#%d time_length = %d!\n", htl->index, htl->time_length);
        stream_back_totaltime(htl->index, htl->time_length);
    }
    if (1 == htl->buffer_flg) {
        LOG_STRM_PRINTF("#%d HLS_MSG_BUFFER_BEGIN stop_flg = %d\n", htl->index, htl->stop_flg);
        int_buffer_begin(htl, 0);
    }

    return 0;
}

static void file_recv_end(void* handle, int end)
{
    HttpTsLive* htl = (HttpTsLive*)handle;

    LOG_STRM_PRINTF("#%d end = %d\n", htl->index, end);
    if (end != 2)
        return;

    strm_play_end(htl->strm_play, htl->index);
}

static void slice_recv_begin(void* handle)
{
    HttpTsLive *htl = (HttpTsLive*)handle;

    {
        HLiveMedia *media;
        int duration, content_length;

        media = &htl->ht->hls->media;

        if (htl->time_length < media->total_duration) {
            htl->time_length = media->total_duration;
            LOG_STRM_PRINTF("#%d time_length = %d!\n", htl->index, htl->time_length);
            stream_back_totaltime(htl->index, htl->time_length);
        }

        content_length = (int)strm_http_get_contentLen(htl->http);
        duration = http_ts_slice_duration(htl->ht);
        if (content_length <= 0 || duration <= 0) {
            htl->byte_rate = 8 * 1024 * 1024;
            LOG_STRM_PRINTF("#%d Unknown byterate!\n", htl->index);
        } else  {
            htl->byte_rate = (uint32_t)(content_length / duration);
            LOG_STRM_PRINTF("#%d byterate = %d\n", htl->index, htl->byte_rate);
        }
    }

    if (1 == htl->buffer_flg) {
        LOG_STRM_PRINTF("#%d HLS_MSG_BUFFER_BEGIN stop_flg = %d\n", htl->index, htl->stop_flg);
        int_buffer_begin(htl, 0);
    }
}

static void slice_recv_end(void* handle)
{
}

static int int_pause(HttpTsLive* htl)
{
    if (STRM_STATE_PAUSE == htl->state)
        LOG_STRM_ERROUT("#%d state = %d\n", htl->index, htl->state);

    if (0 == htl->buffer_flg)
        strm_play_pause(htl->strm_play, htl->index);

    LOG_STRM_DEBUG("#%d STRM_STATE_PAUSE\n", htl->index);
    int_state(htl, STRM_STATE_PAUSE, 0);

    return 0;
Err:
    return -1;
}

static int int_resume(HttpTsLive* htl)
{
    if (STRM_STATE_PLAY == htl->state)
        LOG_STRM_ERROUT("#%d open_play = %d\n", htl->index, htl->state);

    if (0 == htl->buffer_flg)
        strm_play_resume(htl->strm_play, htl->index, 0);

    LOG_STRM_DEBUG("#%d STRM_STATE_PLAY\n", htl->index);
    int_state(htl, STRM_STATE_PLAY, 1);

    return 0;
Err:
    return -1;
}

static int int_seek(HttpTsLive* htl, int seek)
{
    int length = htl->time_length;
    long long content_length = strm_http_get_contentLength(htl->http);
    if (content_length <= 0 || length <= 0)
        LOG_STRM_ERROUT("#%d content_length = %lld, time_length = %d\n", htl->index, content_length, length);
    if (seek < 0 || length <= 3)
        seek = 0;
    if (length > 3 && seek >= length - 3) {
        htl->stop_flg = 1;
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", htl->index);
        stream_post_msg(htl->index, STRM_MSG_STREAM_END, 1);
        seek = length - 3;
    } else {
        htl->stop_flg = 0;
    }

    content_length = content_length * seek / length;
    content_length = content_length - content_length % 188;
    LOG_STRM_PRINTF("#%d content_length = %lld, time_length = %d, seek = %d\n", htl->index, content_length, length, seek);
    strm_http_reset(htl->http);
    if (strm_http_request(htl->http, htl->ht->url, content_length, 0))
        LOG_STRM_ERROUT("#%d local_connect\n", htl->index);

    http_ts_reset(htl->ht);

    htl->time_base = seek;
    htl->time_current = seek;
    stream_back_currenttime(htl->index, htl->time_current);

    strm_play_reset(htl->strm_play, htl->index, 1);
    strm_play_resume(htl->strm_play, htl->index, 0);

    LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", htl->index);
    int_state(htl, STRM_STATE_PLAY, 1);

    strm_play_pause(htl->strm_play, htl->index);
    htl->warn_clk = 0;
    htl->first_flg = 0;
    LOG_STRM_PRINTF("#%d HLS_MSG_BUFFER_BEGIN stop_flg = %d\n", htl->index, htl->stop_flg);
    int_buffer_begin(htl, 0);

    return 0;
Err:
    return -1;
}

static int int_stop(HttpTsLive* htl)
{
    if (int_seek(htl, htl->time_length - 3))
        LOG_STRM_ERROUT("#%d int_seek\n", htl->index);

    return 0;
Err:
    return -1;
}

static void int_trickmode(HttpTsLive* htl, int cmd, int arg)
{
    switch(cmd) {
    case STREAM_CMD_PAUSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_PAUSE open_play = %d\n", htl->index, htl->state);
        int_pause(htl);
        break;
    case STREAM_CMD_SEEK:
        LOG_STRM_PRINTF("#%d STREAM_CMD_SEEK open_play = %d\n", htl->index, htl->state);
        int_seek(htl, arg);
        break;
    case STREAM_CMD_STOP:
        LOG_STRM_PRINTF("#%d STREAM_CMD_STOP open_play = %d\n", htl->index, htl->state);
        int_stop(htl);
        break;
    case STREAM_CMD_FAST:
        LOG_STRM_PRINTF("#%d STREAM_CMD_FAST open_play = %d\n", htl->index, htl->state);
        break;
    case STREAM_CMD_RESUME:
        LOG_STRM_PRINTF("#%d STREAM_CMD_RESUME open_play = %d\n", htl->index, htl->state);
        int_resume(htl);
        break;
    default:
        break;
    }
}

static int int_error_play(HttpTsLive* htl, int msgno)
{
    stream_post_msg(htl->index, STRM_MSG_OPEN_ERROR, 0);
    strm_httploop_break(htl->ht->loop);
    return 0;
}

static void int_deal_error(void* handle, STRM_MSG msgno)
{
    HttpTsLive* htl = (HttpTsLive*)handle;

    LOG_STRM_PRINTF("#%d state = %d\n", htl->index, htl->state);
    if (htl->state)
        int_error_play(htl, 0);
}


static void int_deal_cmd(void* handle, StreamCmd* strmCmd)
{
    HttpTsLive* htl = (HttpTsLive*)handle;

    int cmd = strmCmd->cmd;

    switch(cmd) {
    case STREAM_CMD_RESUME:
    case STREAM_CMD_PAUSE:
    case STREAM_CMD_FAST:
    case STREAM_CMD_SEEK:
    case STREAM_CMD_STOP:
        {
            int cmdsn = strmCmd->arg3;
            int_trickmode(htl, cmd, strmCmd->arg0);
            if (cmdsn)
                stream_back_cmd(htl->index, cmdsn);
        }
        break;
        
    case STREAM_CMD_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE\n", htl->index);
        strm_httploop_break(htl->ht->loop);
        break;

    default:
        LOG_STRM_ERROUT("#%d Unkown CMD %d\n", htl->index, cmd);
    }

Err:
    return;
}

static void int_deal_msg(void* handle, STRM_MSG msgno, int arg)
{
    HttpTsLive* htl = (HttpTsLive*)handle;

    LOG_STRM_PRINTF("#%d msgno = %d\n", htl->index, msgno);

    switch(msgno) {
    case STRM_MSG_PTS_VIEW:
        LOG_STRM_PRINTF("#%d STRM_MSG_PTS_VIEW\n", htl->index);
        stream_post_msg(htl->index, STRM_MSG_PTS_VIEW, 0);
        break;
    case STRM_MSG_PSI_VIEW:
        LOG_STRM_PRINTF("#%d STRM_MSG_PSI_VIEW\n", htl->index);
        break;
    case STRM_MSG_STREAM_MUSIC:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_MUSIC\n", htl->index);
        break;
    case STRM_MSG_STREAM_VIDEO:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_VIDEO\n", htl->index);
        break;
    case STRM_MSG_UNSUPPORT_MEDIA:
    case STRM_MSG_PLAY_ERROR:
        LOG_STRM_ERROR("#%d STRM_MSG_PLAY_ERROR\n", htl->index);
        int_error_play(htl, 0);
        break;
    case STRM_MSG_STREAM_END:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END stop_flg = %d\n", htl->index, htl->stop_flg);
        if (!htl->stop_flg)
            stream_post_msg(htl->index, STRM_MSG_STREAM_END, 0);
        break;

    case STRM_MSG_RECV_TIMEOUT:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_TIMEOUT\n", htl->index);
        break;
    case STRM_MSG_RECV_RESUME:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_RESUME\n", htl->index);
        break;

    default:
        LOG_STRM_ERROUT("#%d unknown play message %d\n", htl->index, msgno);
    }

Err:
    return;
}

static int int_open_play(HttpTsLive* htl, PlayArg *arg, HttpTsArg *httparg)
{
    char *p;
    long long startbyte;
    LOG_STRM_PRINTF("#%d\n", htl->index);

    htl->time_base = 0;
    p = strstr(httparg->url, "starttime=");
    if(p)
        htl->time_base = atoi(p + 10);
    htl->time_current = htl->time_base;

    htl->byte_rate = 0;
    p = strstr(httparg->url, "biteRate=");
    if(p)
        htl->byte_rate = atoi(p + 9) * 1024 / 8;

    startbyte = 0;
    p = strstr(httparg->url, "m3u8");
    if (p)
        strm_http_set_etag(htl->http, 1);
    else if (htl->byte_rate <= 0)
        LOG_STRM_ERROUT("#%d bitrate = %d\n", htl->index, htl->byte_rate);

    if (htl->time_base > 0 && NULL == p) {
        startbyte = (long long)htl->time_base * htl->byte_rate;
        startbyte -= startbyte % 188;
    }

    if (strm_http_request(htl->http, httparg->url, startbyte, 0))
        LOG_STRM_ERROUT("#%d local_connect\n", htl->index);

    strm_play_open(htl->strm_play, htl->index, htl->strm_msgq, int_msgback, APP_TYPE_HLS, TS_TIMESHIFT_SIZE);
    LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", htl->index);
    int_state(htl, STRM_STATE_PLAY, 1);

    strm_play_pause(htl->strm_play, htl->index);
    htl->first_flg = 1;
    htl->buffer_flg = 1;

    return 0;
Err:
    return -1;
}

static void int_close_play(HttpTsLive* htl)
{
    LOG_STRM_PRINTF("#%d state = %d\n", htl->index, htl->state);
    if (htl->state == 0)
        return;

    strm_play_close(htl->strm_play, htl->index, 1);

    strm_httploop_break(htl->ht->loop);
}

static void int_100ms(void* handle)
{
    HttpTsLive* htl = (HttpTsLive*)handle;

    htl->diff = strm_play_buffer(htl->strm_play);
}

static void int_1000ms(void* handle)
{
    int current, diff;
    HttpTs *ht;
    HttpTsLive* htl = (HttpTsLive*)handle;

    ht = htl->ht;

    current = strm_play_time(htl->strm_play);
    diff = htl->diff;
    LOG_STRM_DEBUG("#%d: current = %d, diff = %d\n", htl->index, current / 100, diff);
    if (current < 0)
        current = 0;

    htl->time_current = htl->time_base + current / 100;
    stream_back_currenttime(htl->index, htl->time_current);

    diff /= 100;
    if (htl->byte_rate > 0)
        diff += strm_bufque_length(ht->sbq) / htl->byte_rate;

    if (diff < 15 && !ht->end_flg) {
        uint32_t clk = strm_httploop_clk(ht->loop);
        if (htl->warn_clk) {
            if (htl->first_flg) {
                if (0 == htl->msg_flg && clk - htl->warn_clk >= 1600 && diff < 12) {
                    LOG_STRM_PRINTF("#%d: HLS_MSG_BUFFER_BANDWIDTH stop_flg = %d\n", htl->index, htl->stop_flg);
                    if (!htl->stop_flg)
                        stream_post_msg(htl->index, HLS_MSG_BUFFER_BANDWIDTH, 0);
                    htl->msg_flg = 1;
                }
            } else {
                if (0 == htl->msg_flg && clk - htl->warn_clk >= 1200) {
                    LOG_STRM_PRINTF("#%d: HLS_MSG_BUFFER_LIMITED stop_flg = %d\n", htl->index, htl->stop_flg);
                    if (!htl->stop_flg)
                        stream_post_msg(htl->index, HLS_MSG_BUFFER_LIMITED, 0);
                    htl->msg_flg = 1;
                }
            }
        } else {
            htl->warn_clk = clk;
        }
    } else {
        htl->msg_flg = 0;
        htl->warn_clk = 0;
    }

    if (htl->buffer_flg) {
        int_bufferrate(htl, diff);

        LOG_STRM_PRINTF("#%d buffer second = %d\n", htl->index, diff);
        if (diff >= 15 || ht->end_flg) {
            if (STRM_STATE_PLAY == htl->state)
                strm_play_resume(htl->strm_play, htl->index, 0);

            LOG_STRM_PRINTF("#%d HLS_MSG_BUFFER_END stop_flg = %d\n", htl->index, htl->stop_flg);
            int_buffer_end(htl);
        }
    } else {
        if (diff <= 3 && !ht->end_flg) {
            if (STRM_STATE_PLAY == htl->state)
                strm_play_pause(htl->strm_play, htl->index);

            LOG_STRM_PRINTF("#%d HLS_MSG_BUFFER_BEGIN stop_flg = %d\n", htl->index, htl->stop_flg);
            int_buffer_begin(htl, diff);
        }
    }

    if (2 == htl->buffer_flg) {
        uint32_t clk, recv_times;

        clk = strm_httploop_clk(ht->loop);
        recv_times = strm_http_get_recvTimes(htl->http);
        if (htl->recv_clk) {
            if (recv_times == htl->recv_times) {
                if (clk - htl->recv_clk >= 500) {
                    htl->recv_clk = 0;
                    LOG_STRM_PRINTF("#%d STRM_MSG_RECV_TIMEOUT\n", htl->index);
                    stream_post_msg(htl->index, STRM_MSG_RECV_TIMEOUT, 0);
                }
            } else {
                htl->recv_clk = clk;
                htl->recv_times = recv_times;
            }
        } else {
            if (recv_times != htl->recv_times) {
                htl->recv_clk = clk;
                htl->recv_times = recv_times;
                LOG_STRM_PRINTF("#%d STRM_MSG_RECV_RESUME\n", htl->index);
                stream_post_msg(htl->index, STRM_MSG_RECV_RESUME, 0);
            }
        }
    }
}

static void int_destroy(void* handle)
{
    HttpTsLive* htl = (HttpTsLive*)handle;

    int_close_play(htl);

    htl->ht->ts_handle = NULL;

    IND_FREE(htl);
}

void* http_ts_live_create(HttpTs* ht, PlayArg *arg, HttpTsArg *httparg)
{
    HttpTsLive* htl = (HttpTsLive*)IND_CALLOC(sizeof(HttpTsLive), 1);
    if (!htl)
        LOG_STRM_ERROUT("#%d malloc failed!\n", ht->index);

    htl->ht = ht;
    htl->http = ht->http;
    htl->index = ht->index;

    htl->strm_msgq = int_strm_msgq(htl->index);
    htl->strm_play = int_strm_play(htl->index);

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

    if (int_open_play(htl, arg, httparg))
        LOG_STRM_ERROUT("#%d int_open_play\n", htl->index);

    return htl;
Err:
    if (htl)
        int_destroy(htl);

    ht->ts_handle = htl;

    return NULL;
}
