
/**************************************--**************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **************************************--**************************************/

#include "http_apple.h"

#if SUPPORTE_HD == 1

#define HTTP_APPLE_BUFFER_SIZE      (10 * 1024 * 1316)
#define HTTP_APPLE_PUSH_SIZE        (64 * 1316)

static int g_buffer_size = HTTP_APPLE_BUFFER_SIZE;

/********************************** HTTP TS ***********************************/

static HAppleTrack* http_track_create(HttpApple* ha, int idx);
static void http_track_delete(HttpApple* ha, int idx);

static int gHistoryBandwidth = 0;
static unsigned int gHistoryValidClock = 0;

static void int_state(HttpApple* ha, STRM_STATE state, int scale)
{
    ha->state = state;
    ha->scale = scale;
    stream_post_state(ha->index, state, scale);
    LOG_STRM_PRINTF("#%d state = %d, scale = %d\n", ha->index, state, scale);
}

static void int_msgback(void* handle, STRM_MSG msgno, int arg)
{
    StreamMsgQ* strm_msgq = (StreamMsgQ*)handle;
    strm_msgq_queue(strm_msgq, msgno, arg);
}

static void int_100ms(void* handle)
{
    HttpApple *ha = (HttpApple*)handle;

    if (ha->m3u8_clk || ha->slice_clk) {
        uint32_t clk = strm_httploop_clk(ha->loop);

        if (ha->m3u8_clk) {
            if (ha->m3u8_clk <= clk) {
                LOG_STRM_PRINTF("#%d m3u8_clk\n", ha->index);
                apple_audio_m3u8_refresh(ha, 1);
            }
        } else {
            if (ha->slice_clk <= clk) {
                if (apple_buffer_delay(ha)) {
                    ha->slice_clk = strm_httploop_clk(ha->loop) + ha->slice_duration * 100;
                } else {
                    HLiveMedia *media = ha->track_array[HAPPLE_TRACK_VIDEO]->task.media;
                    LOG_STRM_PRINTF("#%d slice_clk\n", ha->index);
                    if (1 == ha->servicetype || ha->load_sequence >= media->x_sequence + media->slice_num)
                        apple_audio_m3u8_refresh(ha, 1);
                    else
                        apple_audio_slice_download(ha);
                }
            }
        }
    }

    apple_buffer_push(ha);

    if (STRM_STATE_BUFFER == ha->state) {
        int buffer = apple_buffer_duration(ha);

        if (buffer > 500) {
            strm_play_resume(ha->strm_play, ha->index, 0);
            if (1 == ha->servicetype) {
                LOG_STRM_PRINTF("#%d STRM_STATE_IPTV\n", ha->index);
                int_state(ha, STRM_STATE_IPTV, 0);
            } else {
                LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", ha->index);
                int_state(ha, STRM_STATE_PLAY, 0);
            }
        }
    }
}

static void int_1000ms(void* handle)
{
    uint32_t current;
    HttpApple *ha;

    ha = (HttpApple*)handle;

    {
        int space = apple_bufque_space(ha->abq);
        int_steam_setCurBufferSize(space);
    }
    if (-1 == ha->load_sequence) //seek过程中不上报时间
        return;

    current = ha->time_start + strm_play_time(ha->strm_play) / 100;

    if (0 == ha->servicetype || 3 == ha->servicetype) {
        if (current > ha->time_length)
            current = ha->time_length;
        int_steam_setRemainPlaytime(ha->time_length - current);
    } else {
        uint32_t now = mid_time( );

        if (STRM_STATE_IPTV == ha->state) {
            current = now;
        } else {
            if (current < now - ha->time_length)
                current = now - ha->time_length;
            else if (current > now)
                current = now;
        }
    }

    ha->time_current = current;
    stream_back_currenttime(ha->index, ha->time_current);

    apple_buffer_1000ms(ha);
}

static void int_error(void* handle, HTTP_MSG msgno)
{
    HttpApple *ha = (HttpApple*)handle;

    LOG_STRM_DEBUG("#%d\n", ha->index);

    strm_httploop_break(ha->loop);
    stream_post_msg(ha->index, STRM_MSG_OPEN_ERROR, 0);
}

static int index_recv_begin(void* handle)
{
    AppleTask *task;
    HttpApple *ha = (HttpApple*)handle;

    task = &ha->track_array[HAPPLE_TRACK_VIDEO]->task;

    ts_buf_reset(task->recv_ts_buf);
    strm_http_buf_replace(task->http, task->recv_ts_buf);

    return 0;
}

static void index_recv_sync(void* handle)
{
}

static void index_recv_end(void* handle)
{
    HttpApple *ha = (HttpApple*)handle;
    HttpLive *hls;
    AppleTask *task;
    HAppleTrack *track;
    HLiveStream *stream;
    int i, num;

    hls = ha->hls;
	track = ha->track_array[HAPPLE_TRACK_VIDEO];
    task = &track->task;

    ha->stream = NULL;
    {
        int len;
        char *buf;
        ts_buf_t ts_buf = task->recv_ts_buf;

        len = 0;
        ts_buf_reload_get(ts_buf, &buf, &len);
        if (len > 0)
            ts_buf_reload_mark(ts_buf, len);

        len = 0;
        ts_buf_read_get(ts_buf, &buf, &len);
        if (len > 0) {
            buf[len] = 0;
            if (ind_memstr(buf, len, "#EXT-X-MEDIA-SEQUENCE")) {
                http_live_parse_slice(task->media, buf, len);
                ts_buf_read_pop(ts_buf, len);

                strm_http_get_host(task->http, task->m3u8_host);
                strm_http_get_uri(task->http, task->m3u8_path);

                track->state = LIVE_STATE_IDLE;
                apple_audio_m3u8_end(ha);
                return;
            }
            http_live_parse_stream(hls, buf, len);
            ts_buf_read_pop(ts_buf, len);
        }
    }

    if (NULL == hls->stream_queue)
        LOG_STRM_ERROUT("#%d stream_queue is empty!\n", ha->index);

    strm_http_get_host(task->http, ha->index_host);
    strm_http_get_uri(task->http, ha->index_path);

    if (ha->index != STREAM_INDEX_PIP) {
        num = 0;
        stream = hls->stream_queue;
        while (stream) {
            num++;
            stream = stream->next;
        }
        int_steam_setStreamNum(num);

        i = 0;
        stream = hls->stream_queue;
        while (stream) {
            int_steam_setStreamInfo(i, stream->bandwidth, stream->uri);
            i++;
            stream = stream->next;
        }
    }
    stream = hls->stream_queue;
    {
        int bandwidth = 0;
        unsigned int clk = strm_httploop_clk(ha->loop);

        //每次进入播放的首个分片，取最近一次历史分片下载速率×80%的分片
        if (gHistoryValidClock && gHistoryValidClock > clk)
            bandwidth = gHistoryBandwidth * 4 / 5;

        if (bandwidth > 0) {
            HLiveStream *next = stream->next;

            while (next && next->bandwidth <= bandwidth) {
                stream = next;
                next = stream->next;
            }
        }
    }
    if (!stream)
        LOG_STRM_ERROUT("#%d stream is NULL\n", ha->index);

    ha->stream = stream;

    LOG_STRM_PRINTF("#%d num = %d / %d\n", ha->index, hls->audio.num, hls->subtitle.num);
    if (!ha->alternative_audio && hls->audio.num > 0)
        ha->alternative_audio = 1;

    if (ha->alternative_audio) {
        struct ts_psi *psi;
        HLiveElem* elem;
        HAppleTrack *track;
        HLiveTrack* ltrack;

        track = ha->track_array[HAPPLE_TRACK_VIDEO];

        if (STREAM_INDEX_PIP != ha->index)
            codec_alternative_set(1);

        psi = &ha->ts_psi;
        IND_MEMSET(psi, 0, sizeof(ha->ts_psi));

        psi->pmt_pid = 0x40;

        psi->pcr_pid = 0x45;
        psi->video_pid = 0x45;
        psi->video_type = ISO_IEC_H264;

        track->pid = 0x45;

        num = hls->audio.num;
        if (num > 0) {
            psi->audio_num = num;

            track = ha->track_array[HAPPLE_TRACK_AUDIO];
            ltrack = &hls->audio;
            if (!track) {
                track = http_track_create(ha, HAPPLE_TRACK_AUDIO);
                if (!track)
                    LOG_STRM_ERROUT("#%d http_track_create audio\n", ha->index);

                track->task.media = &ltrack->media;
                track->switch_task.media = &ltrack->switch_media;
                track->ltrack = ltrack;
            }
            track->pid = 0x50;

            for (i = 0; i < num; i ++) {//language
                psi->audio_pid[i] = 0x50 + i;
                elem = ltrack->array[i];

                IND_MEMCPY(psi->audio_iso693[i].language, elem->language, 3);
                psi->audio_iso693[i].type = 0;
            }
            if (track->task.index >= num)
                track->task.index = 0;
        }

        num = hls->subtitle.num;

        if (num > 0) {
            ts_dr_subtitle_t dr_subtitle = &ha->dr_subtitle;

            psi->dr_subtitle = dr_subtitle;

            dr_subtitle->subtitle_num = num;

            track = ha->track_array[HAPPLE_TRACK_SUBTITLE];
            ltrack = &hls->subtitle;
            if (!track) {
                track = http_track_create(ha, HAPPLE_TRACK_SUBTITLE);
                if (!track)
                    LOG_STRM_ERROUT("#%d http_track_create subtitle\n", ha->index);

                track->task.media = &ltrack->media;
                track->switch_task.media = &ltrack->switch_media;
                track->ltrack = ltrack;
            }
            track->pid = 0x60;

            for (i = 0; i < num; i ++) {//language
                dr_subtitle->subtitle_pid[i] = 0x60 + i;
                elem = ltrack->array[i];

                IND_MEMCPY(dr_subtitle->subtitle[i].language, elem->language, 3);
                dr_subtitle->subtitle[i].type = 0;
            }
            if (track->task.index >= num)
                track->task.index = 0;
        }
    }

    apple_audio_m3u8_refresh(ha, 0);

    return;
Err:
    int_error(ha, STRM_MSG_OPEN_ERROR);
    return;
}

int http_apple_index_request(HttpApple* ha, int servicetype)//从直播转时移
{
    char *url;
    AppleTask *task;
    HAppleTrack *track;

    track = ha->track_array[HAPPLE_TRACK_VIDEO];
    task = &track->task;

    url = ha->url;
    if (ha->huaweimode) {
        if (1 == servicetype || 2 == servicetype) {
            char *p = strstr(url, "servicetype=");
            if (NULL == p)
                LOG_STRM_ERROUT("#%d servicetype\n", ha->index);
            if (2 == servicetype)
                p[12] = '2';
            else
                p[12] = '1';
        }
    }

    strm_http_set_etag(task->http, 0);
    strm_http_set_gzip(task->http, 1);
    strm_http_reset(task->http);
    if (strm_http_request(task->http, url, 0, 0))
        LOG_STRM_ERROUT("#%d strm_http_request\n", ha->index);

    LOG_STRM_PRINTF("#%d LIVE_STATE_INDEX\n", ha->index);
    {
        HttpOp op;

        memset(&op, 0, sizeof(op));

        op.deal_error   = int_error;
        op.recv_begin   = index_recv_begin;
        op.recv_sync    = index_recv_sync;
        op.recv_end     = index_recv_end;

        strm_http_set_opset(task->http, &op, ha);
    }

    track->state = LIVE_STATE_INDEX;

    return 0;
Err:
    return -1;
}

static void int_msg(void* handle, STRM_MSG msgno, int arg)
{
    HttpApple* ha = (HttpApple*)handle;

    LOG_STRM_DEBUG("#%d msgno = %d\n", ha->index, msgno);

    switch(msgno) {
    case STRM_MSG_PTS_VIEW:
        LOG_STRM_PRINTF("#%d STRM_MSG_PTS_VIEW\n", ha->index);
        stream_post_msg(ha->index, STRM_MSG_PTS_VIEW, 0);
        break;
    case STRM_MSG_PSI_VIEW:
        LOG_STRM_PRINTF("#%d STRM_MSG_PSI_VIEW\n", ha->index);
        break;
    case STRM_MSG_STREAM_MUSIC:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_MUSIC\n", ha->index);
        stream_post_msg(ha->index, STRM_MSG_STREAM_MUSIC, 0);
        break;
    case STRM_MSG_STREAM_VIDEO:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_VIDEO\n", ha->index);
        stream_post_msg(ha->index, STRM_MSG_STREAM_VIDEO, 0);
        break;
    case STRM_MSG_UNSUPPORT_MEDIA:
    case STRM_MSG_PLAY_ERROR:
        LOG_STRM_ERROR("#%d STRM_MSG_PLAY_ERROR\n", ha->index);
        strm_httploop_break(ha->loop);
        stream_post_msg(ha->index, STRM_MSG_OPEN_ERROR, 0);
        break;
    case STRM_MSG_UNDERFLOW:
        strm_play_pause(ha->strm_play, ha->index);
        LOG_STRM_PRINTF("#%d STRM_STATE_BUFFER\n", ha->index);
        int_state(ha, STRM_STATE_BUFFER, 0);
        break;
    case STRM_MSG_RECV_TIMEOUT:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_TIMEOUT\n", ha->index);
        ha->data_flag = 0;
        if (ha->error_code >= 0) {
            strm_httploop_break(ha->loop);
            stream_post_msg(ha->index, STRM_MSG_OPEN_ERROR, ha->error_code);
        } else {
            strm_play_pause(ha->strm_play, ha->index);
            LOG_STRM_PRINTF("#%d STRM_STATE_BUFFER\n", ha->index);
            int_state(ha, STRM_STATE_BUFFER, 0);
            //stream_post_msg(ha->index, STRM_MSG_RECV_TIMEOUT, 0);
        }
        break;
    case STRM_MSG_RECV_RESUME:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_RESUME\n", ha->index);
        //stream_post_msg(ha->index, STRM_MSG_RECV_RESUME, 0);
        ha->data_flag = 1;
        break;
    case STRM_MSG_STREAM_END:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", ha->index);
        stream_post_msg(ha->index, STRM_MSG_STREAM_END, 0);
        break;

    default:
        LOG_STRM_ERROUT("#%d unknown play message %d\n", ha->index, msgno);
    }

Err:
    return;
}

static int int_pause(HttpApple* ha)
{
    switch (ha->state) {
    case STRM_STATE_IPTV://只有频道采用IPTV状态
        ha->time_current = mid_time( );
        ha->time_start = ha->time_current - strm_play_time(ha->strm_play) / 100;
        break;
    case STRM_STATE_PLAY:
    case STRM_STATE_BUFFER:
        break;
    default:
        LOG_STRM_ERROUT("#%d state = %d\n", ha->index, ha->state);
    }

    strm_play_pause(ha->strm_play, ha->index);
    LOG_STRM_PRINTF("#%d STRM_STATE_PAUSE\n", ha->index);
    int_state(ha, STRM_STATE_PAUSE, 0);

    return 0;
Err:
    return -1;
}

static int int_resume(HttpApple* ha)
{
    strm_play_resume(ha->strm_play, ha->index, 0);
    LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", ha->index);
    int_state(ha, STRM_STATE_PLAY, 1);

    return 0;
}

static int int_seek(HttpApple* ha, uint32_t seek)
{
    int i;
    AppleTask *task;
    HAppleTrack *track = ha->track_array[HAPPLE_TRACK_VIDEO];

    if (1 == ha->servicetype && (ha->time_length <= 0 || 0 == track->load_datetime))
        LOG_STRM_ERROUT("#%d time_length = %d, load_datetime = %08x\n", ha->index, ha->time_length, track->load_datetime);

    ha->data_flag = 1;
    ha->error_code = -1;
    ha->load_sequence = -1;

    ha->slice_duration = 0;

    ha->end_flg = 0;
    for (i = 0; i < HAPPLE_TRACK_NUM; i ++) {
        track = ha->track_array[i];
        if (!track)
            continue;
        task = &track->task;
        if (i != HAPPLE_TRACK_VIDEO)
            track->state = LIVE_STATE_IDLE;

        strm_http_reset(task->http);
        ts_buf_reset(task->recv_ts_buf);

        track->load_length = 0;
        track->load_duration = 0;

        if (ha->alternative_audio)
            apple_switch_end(track);
    }
    apple_bufque_reset(ha->abq);
    if (ha->index != STREAM_INDEX_PIP)
        int_steam_setCurSegment(NULL);

    ha->time_seek = seek;

    if (1 == ha->servicetype || 2 == ha->servicetype) {
        uint32_t now = mid_time( );

        if (ha->time_seek > now)
            ha->time_seek = now;
        else if (ha->time_seek < now - ha->time_length)
            ha->time_seek = now - ha->time_length;

        for (i = 0; i < HAPPLE_TRACK_NUM; i ++) {
            track = ha->track_array[i];
            if (track)
                track->load_datetime = ha->time_seek + track->serv_diff;
        }
    }

    track = ha->track_array[HAPPLE_TRACK_VIDEO];

    strm_play_reset(ha->strm_play, ha->index, 1);
    strm_play_resume(ha->strm_play, ha->index, 0);

    if (0 == ha->servicetype || 3 == ha->servicetype) {
        if (0 == seek) {
            LOG_STRM_PRINTF("#%d STRM_MSG_SEEK_BEGIN\n", ha->index);
            stream_post_msg(ha->index, STRM_MSG_SEEK_BEGIN, 0);
        } else if (ha->time_length > 0 && seek >= ha->time_length) {
            seek = ha->time_length;
            LOG_STRM_PRINTF("#%d STRM_MSG_SEEK_END\n", ha->index);
            stream_post_msg(ha->index, STRM_MSG_SEEK_END, 0);
        }

        LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", ha->index);
        int_state(ha, STRM_STATE_PLAY, 1);

        if (seek < ha->time_length && LIVE_STATE_INDEX != track->state) {
            HLiveMedia* media = &ha->hls->media;

            LOG_STRM_PRINTF("#%d state = %d, slice_complete = %d, total_duration = %d\n", 
                    ha->index, track->state, media->slice_complete, media->total_duration);
            if (LIVE_STATE_M3U8 == track->state || (!media->slice_complete && seek >= media->total_duration)) {
                if (apple_audio_m3u8_refresh(ha, 1))
                    LOG_STRM_ERROUT("#%d m3u8_request_all\n", ha->index);
            } else {
                track->state = LIVE_STATE_IDLE;
                apple_audio_m3u8_end(ha);
            }
        }
    } else {
        uint32_t now, duration;

        now = mid_time( );
        duration = ha->hls->media.x_duration;
        if (duration > 0 && seek > now - duration * 3) {//果达到或超过了右边界最新时间倒数第三个分片时，转直播
            LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", ha->index);
            stream_post_msg(ha->index, STRM_MSG_STREAM_END, 0);

            if (http_apple_index_request(ha, 1))
                LOG_STRM_ERROUT("#%d http_apple_index_request\n", ha->index);
            ha->servicetype = 1;
            LOG_STRM_PRINTF("#%d STRM_STATE_IPTV\n", ha->index);
            int_state(ha, STRM_STATE_IPTV, 1);
        } else {
            if (http_apple_index_request(ha, 2))
                LOG_STRM_ERROUT("#%d http_apple_index_request\n", ha->index);
            ha->servicetype = 2;
            LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", ha->index);
            int_state(ha, STRM_STATE_PLAY, 1);
        }
    }

    return 0;
Err:
    return -1;
}

static int int_stop(HttpApple* ha)
{
    uint32_t seek;

    if (0 == ha->servicetype || 3 == ha->servicetype)
        seek = ha->time_length;
    else
        seek = mid_time( );

    if (int_seek(ha, seek))
        LOG_STRM_ERROUT("#%d int_seek\n", ha->index);

    return 0;
Err:
    return -1;
}

static void int_trickmode(HttpApple* ha, int cmd, int arg)
{
    switch(cmd) {
    case STREAM_CMD_PAUSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_PAUSE open_play = %d\n", ha->index, ha->state);
        int_pause(ha);
        break;
    case STREAM_CMD_SEEK:
        LOG_STRM_PRINTF("#%d STREAM_CMD_SEEK open_play = %d\n", ha->index, ha->state);
        int_seek(ha, (uint32_t)arg);
        break;
    case STREAM_CMD_STOP:
        LOG_STRM_PRINTF("#%d STREAM_CMD_STOP open_play = %d\n", ha->index, ha->state);
        int_stop(ha);
        break;
    case STREAM_CMD_FAST:
        LOG_STRM_WARN("#%d STREAM_CMD_FAST open_play = %d\n", ha->index, ha->state);
        break;
    case STREAM_CMD_RESUME:
        LOG_STRM_PRINTF("#%d STREAM_CMD_RESUME open_play = %d\n", ha->index, ha->state);
        int_resume(ha);
        break;
    default:
        break;
    }
}

static void int_cmd(void* handle, StreamCmd* strmCmd)
{
    int cmd;
    HttpApple* ha = (HttpApple*)handle;

    cmd = strmCmd->cmd;
    LOG_STRM_DEBUG("#%d cmd = %d\n", ha->index, cmd);

    switch(cmd) {
    case STREAM_CMD_RESUME:
    case STREAM_CMD_PAUSE:
    case STREAM_CMD_FAST:
    case STREAM_CMD_SEEK:
    case STREAM_CMD_STOP:
        {
            int cmdsn = strmCmd->arg3;
            int_trickmode(ha, cmd, strmCmd->arg0);
            if (cmdsn)
                stream_back_cmd(ha->index, cmdsn);
        }
        break;

    case STREAM_CMD_TEST_BANDWIDTH:
        LOG_STRM_PRINTF("#%d STREAM_CMD_TEST_BANDWIDTH bandwidth = %d\n", ha->index, strmCmd->arg0);
        strm_http_set_limit(ha->track_array[HAPPLE_TRACK_VIDEO]->task.http, strmCmd->arg0 / 8);
        break;

    case STREAM_CMD_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE\n", ha->index);
        strm_httploop_break(ha->loop);
        break;

    default:
        LOG_STRM_ERROUT("#%d Unkown CMD %d\n", ha->index, cmd);
    }

Err:
    return;
}

static int int_open_play(HttpApple* ha, PlayArg *arg, HttpAppleArg *httparg)
{
    HAppleTrack *video = ha->track_array[HAPPLE_TRACK_VIDEO];

    IND_STRCPY(ha->url, httparg->url);
    ha->huaweimode = httparg->huaweimode;
    ha->servicetype = httparg->servicetype;

    ha->data_flag = 1;
    ha->error_code = -1;
    ha->load_sequence = -1;

    strm_http_set_sqm(video->task.http, 1);
    http_apple_index_request(ha, ha->servicetype);

    strm_play_open(ha->strm_play, ha->index, ha->strm_msgq, int_msgback, APP_TYPE_HTTP_LIVE, STREAM_BUFFER_SIZE);

    if (1 == ha->servicetype) {
        ha->time_seek = 0;
        ha->time_length = httparg->shiftlen;
        LOG_STRM_PRINTF("#%d STRM_STATE_IPTV time_length = %d\n", ha->index, ha->time_length);
        int_state(ha, STRM_STATE_IPTV, 1);
    } else {
        ha->time_seek = httparg->shiftlen;
        if (3 == ha->servicetype) {
            ha->time_length = ha->time_end - ha->time_begin;
            ha->time_begin = httparg->begin;
            ha->time_end = httparg->end;
        }
        LOG_STRM_PRINTF("#%d STRM_STATE_PLAY time_seek = %d\n", ha->index, ha->time_seek);
        int_state(ha, STRM_STATE_PLAY, 1);
    }

    return 0;
}

static void int_close_play(HttpApple* ha)
{
    LOG_STRM_PRINTF("#%d state = %d\n", ha->index, ha->state);
    if (ha->state == 0)
        return;

    if (ha->index != STREAM_INDEX_PIP) {
        int_steam_setStreamNum(0);
        int_steam_setCurSegment(NULL);
    }

    if (ha->servicetype == 0 || ha->servicetype == 3)
        strm_play_close(ha->strm_play, ha->index, 1);
    else
        strm_play_close(ha->strm_play, ha->index, 0);

    strm_httploop_break(ha->loop);
    LOG_STRM_PRINTF("#%d STRM_STATE_CLOSE\n", ha->index);
    int_state(ha, STRM_STATE_CLOSE, 1);
}

static HAppleTrack* http_track_create(HttpApple* ha, int idx)
{
    AppleTask *task;
    HAppleTrack *track = IND_CALLOC(sizeof(HAppleTrack), 1);
    if (!track)
        goto Err;
    track->switch_sequence = -1;
    track->index = idx;
    ha->track_array[idx] = track;

    track->ha = ha;
    task = &track->task;

    task->recv_buffer = (char*)IND_MALLOC(HTTP_APPLE_M3U8_SIZE);
    if (!task->recv_buffer)
        goto Err;

    task->ts_parse = ts_parse_create(&task->ts_psi);
    if (!task->ts_parse)
        goto Err;
    if (HAPPLE_TRACK_SUBTITLE == idx)
        task->ts_subtitle = (ts_dr_subtitle_t)IND_MALLOC(sizeof(struct ts_dr_subtitle));

    task->recv_ts_buf = ts_buf_reload(task->recv_buffer, HTTP_APPLE_M3U8_SIZE);
    if (!task->recv_ts_buf)
        goto Err;

    task->http = strm_http_create(ha->loop, 0);
    if (!task->http)
        goto Err;
    strm_http_set_retry(task->http, 1);

    return track;
Err:
    if (track)
        http_track_delete(ha, idx);
    return NULL;
}

static void http_track_delete(HttpApple* ha, int idx)
{
    AppleTask *task;
    HAppleTrack *track;

    track = ha->track_array[idx];
    if (!track)
        return;
    ha->track_array[idx] = NULL;

    task = &track->task;
    apple_switch_clear(task);
    apple_switch_clear(&track->switch_task);

    if (task->ts_parse)
        ts_parse_delete(task->ts_parse);
    if (task->ts_subtitle)
        IND_FREE(task->ts_subtitle);

    if (track->uri)
        IND_FREE(track->uri);

    if (track->key_uri)
        IND_FREE(track->key_uri);

    IND_FREE(track);
}

static HttpApple* http_apple_create(int idx, mid_msgq_t msgq)
{
    HttpApple *ha;
    HttpLoopOp op;
    HAppleTrack *track;

    ha = (HttpApple*)IND_CALLOC(sizeof(HttpApple), 1);
    if (!ha)
        LOG_STRM_ERROUT("#%d malloc failed!\n", idx);

    ha->index = idx;

#ifdef ENABLE_SAVE_APPLE
    ha->save_fp = fopen(DEFAULT_EXTERNAL_DATAPATH"/apple.ts", "wb");
    LOG_STRM_PRINTF("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@: save_fp = %p\n", ha->save_fp);
#endif

    ha->hls = http_live_create( );
    if (NULL == ha->hls)
        LOG_STRM_ERROUT("#%d hls_create\n", idx);

    memset(&op, 0, sizeof(op));

    op.deal_cmd = int_cmd;
    op.deal_msg = int_msg;
    op.local_100ms = int_100ms;
    op.local_1000ms = int_1000ms;

    ha->sb = strm_buf_malloc(STREAM_BUFFER_SIZE);
    if (!ha->sb)
        LOG_STRM_ERROUT("#%d strm_buf_malloc\n", ha->index);
    ha->abq = apple_bufque_create(STREAM_BUFFER_SIZE, g_buffer_size / STREAM_BUFFER_SIZE);
    if (!ha->abq)
        LOG_STRM_ERROUT("#%d apple_bufque_create size = %d\n", ha->index, g_buffer_size);
    int_steam_setCurBufferSize(g_buffer_size);
    int_steam_setToalBufferSize(g_buffer_size);

    ha->loop = strm_httploop_create(idx, &op, ha, msgq);
    if (!ha->loop)
        LOG_STRM_ERROUT("strm_httploop_create!\n");

    track = http_track_create(ha, HAPPLE_TRACK_VIDEO);
    if (!track)
        LOG_STRM_ERROUT("#%d http_track_create\n", idx);
    track->task.media = &ha->hls->media;

    return ha;
Err:
    if (ha) {
        if (ha->hls)
            http_live_delete(ha->hls);

        if (ha->loop)
            strm_httploop_delete(ha->loop);
        IND_FREE(ha);
    }
    return NULL;
}

static void http_apple_delete(HttpApple* ha)
{
    int i;

    if (!ha)
        return;

#ifdef ENABLE_SAVE_APPLE
    LOG_STRM_PRINTF("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@: save_fp = %p\n", ha->save_fp);
    fclose(ha->save_fp);
#endif

    if (ha->hls)
        http_live_delete(ha->hls);
    for (i = 0; i < HAPPLE_TRACK_NUM; i++)
        http_track_delete(ha, i);

    if (ha->sb)
        strm_buf_free(ha->sb);
    if (ha->abq)
        apple_bufque_delete(ha->abq);

    if (ha->loop)
        strm_httploop_delete(ha->loop);

    IND_FREE(ha);
}

//IPTV STB WebTV 特性设计.docx 分片下载速率记在内存中，只记录最近一次，历史记录5分钟内有效。
void http_apple_bandwidth(HttpApple* ha, int bandwidth)
{
    gHistoryBandwidth = bandwidth;
    gHistoryValidClock = strm_httploop_clk(ha->loop) + 60 * 5 * 100;
}

static int int_play_argparse(int idx, PlayArg* arg, char *argbuf, const char* url, int shiftlen, int begin, int end)
{
    int servicetype;
    const char *p;
    HttpAppleArg *httparg = (HttpAppleArg *)argbuf;

    if (strncasecmp(url, "http://", 7))
        LOG_STRM_ERROUT("#%d\n", idx);
    LOG_STRM_PRINTF("#%d\n", idx);

    p = strstr(url, "servicetype=");
    if (p) {
        servicetype = atoi(p + 12);
        switch (arg->apptype) {
        case APP_TYPE_APPLE_VOD:
            if (servicetype != 0 && servicetype != 3)
                LOG_STRM_ERROUT("#%d servicetype = %d\n", idx, servicetype);
            break;
        case APP_TYPE_APPLE_IPTV:
            if (servicetype != 1)
                LOG_STRM_ERROUT("#%d APP_TYPE_APPLE_IPTV servicetype = %d\n", idx, servicetype);
            break;
        default:
            LOG_STRM_ERROUT("#%d apptype = %d\n", idx, arg->apptype);
        }
        httparg->huaweimode = 1;
    } else {
        switch (arg->apptype) {
        case APP_TYPE_APPLE_VOD:
            servicetype = 0;
            break;
        case APP_TYPE_APPLE_IPTV:
            servicetype = 1;
            break;
        default:
            LOG_STRM_ERROUT("#%d apptype = %d\n", idx, arg->apptype);
        }
        httparg->huaweimode = 0;
    }

    httparg->servicetype = servicetype;

    IND_STRCPY(httparg->url, url);
    if (shiftlen > 0)
        httparg->shiftlen = shiftlen;
    else
        httparg->shiftlen = 0;

    LOG_STRM_PRINTF("#%d magic = %u, shiftlen = %d, begin = %d, end = %d\n", idx, arg->magic, shiftlen, begin, end);

    httparg->begin = 0;
    httparg->end = 0;
    if (3 == servicetype) {
        p = strstr(url, "playseek=");
        if (!p) {
            if (begin < 0 || end < begin)
                LOG_STRM_ERROUT("#%d begin = %d, end = %d\n", idx, begin, end);
            httparg->begin = begin;
            httparg->end = end;
        }
    }
    int_back_hls_playrate(0);

    return 0;
Err:
    return -1;
}

/********************************hls*********************************************/
static void int_play_loop(void *handle, int idx, mid_msgq_t msgq, PlayArg *arg, char *argbuf)
{
    HttpApple *ha;
    HttpAppleArg *httparg;

    ha = http_apple_create(idx, msgq);
    if (!ha)
        LOG_STRM_ERROUT("#%d http is NULL\n", idx);

    ha->strm_msgq = int_strm_msgq(idx);
    ha->strm_play = int_strm_play(idx);

    httparg = (HttpAppleArg *)argbuf;

    if (int_open_play(ha, arg, httparg))
        LOG_STRM_ERROUT("#%d int_open_play\n", idx);

    strm_httploop_loop(ha->loop);

    LOG_STRM_PRINTF("#%d exit loop!\n", idx);
    int_close_play(ha);

Err:
    if (idx != STREAM_INDEX_PIP) {
        int_steam_setDownloadRate(0);
        int_steam_setRemainPlaytime(0);

        int_steam_setCurBufferSize(0);
        int_steam_setToalBufferSize(0);
    }
    if (ha)
        http_apple_delete(ha);
    return;
}

int apple_create_stream(StreamCtrl *ctrl)
{
    ctrl->handle = ctrl;

    ctrl->loop_play = int_play_loop;

    ctrl->argsize = sizeof(HttpAppleArg);
    ctrl->argparse_play = int_play_argparse;

    return 0;
}

void mid_stream_set_apple_buffersize(int size)
{
    LOG_STRM_PRINTF("size = %d\n", size);
    g_buffer_size = size;
}

#endif//#if SUPPORTE_HD == 1
