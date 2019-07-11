
/**************************************--**************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **************************************--**************************************/

#include "http_apple.h"

#if SUPPORTE_HD == 1

static int  slice_recv_begin(void* handle);
static void slice_recv_sync(void* handle);
static void slice_recv_end(void* handle);

static void int_end(HAppleTrack *track)
{
    AppleTask *task = &track->switch_task;

    track->switch_sequence = -1;
    strm_http_reset(task->http);
    apple_switch_clear(task);
}

static void int_error(void* handle, HTTP_MSG msgno)
{
    HAppleTrack *track = (HAppleTrack*)handle;

    int_end(track);
}

static int m3u8_recv_begin(void* handle)
{
    int length;
    AppleTask *task;
    HAppleTrack *track = (HAppleTrack*)handle;

    LOG_STRM_PRINTF("\n");

    task = &track->switch_task;
    apple_switch_clear_crypt(task);

    length = (int)strm_http_get_contentLength(task->http);
    if (length <= 0 || length >= HTTP_APPLE_M3U8_SIZE)
        LOG_STRM_ERROUT("length = %d\n", length);

    length = length - length % 188 + 188;

    task->recv_ts_buf = ts_buf_create(length);
    if (!task->recv_ts_buf)
        goto Err;

    ts_buf_reset(task->recv_ts_buf);
    strm_http_buf_replace(task->http, task->recv_ts_buf);

    return 0;
Err:
    return -1;
}

static void m3u8_recv_sync(void* handle)
{
}

void apple_switch_clear_crypt(AppleTask* task)
{
#ifdef USE_VERIMATRIX_OTT
    if (task->crypt_ts_buf) {
        ts_buf_delete(task->crypt_ts_buf);
        task->crypt_ts_buf = NULL;
    }
    if (task->crypt_buffer) {
        yhw_mem_free(task->crypt_buffer);
        task->crypt_buffer = NULL;
    }
    if (task->crypt_handle) {
        ymm_stream_destoryCryptM2MKeyHandle(task->crypt_handle);
        task->crypt_handle = NULL;
    }
#endif
}

void apple_switch_clear(AppleTask* task)
{
    if (task->recv_ts_buf) {
        ts_buf_delete(task->recv_ts_buf);
        task->recv_ts_buf = NULL;
    }
    if (task->recv_buffer) {
        IND_FREE(task->recv_buffer);
        task->recv_buffer = NULL;
    }
    apple_switch_clear_crypt(task);
}

static int slice_request(HAppleTrack *track)
{
    char *uri, url[STREAM_URL_SIZE];
    AppleTask *task;
    HLiveSlice *slice;

    task = &track->switch_task;

    slice = http_live_slice(task->media, track->switch_sequence);
    if (!slice)
        LOG_STRM_ERROUT("switch_sequence = %d\n", track->switch_sequence);

    uri = slice->uri;
    if (0 == strncasecmp("http://", uri, 7)) {
        IND_STRCPY(url, uri);
    } else {
        int act = strm_http_get_active(task->http);

        if (act) {
            if ('/' == uri[0])
                sprintf(url, "%s", uri);
            else
                sprintf(url, "%s/%s", task->m3u8_path, uri);
        } else {
            if ('/' == uri[0])
                sprintf(url, "http://%s%s", task->m3u8_host, uri);
            else
                sprintf(url, "http://%s%s/%s", task->m3u8_host, task->m3u8_path, uri);
        }
    }

    strm_http_set_etag(task->http, 0);
    if (strm_http_request(task->http, url, 0, 0))
        LOG_STRM_ERROUT("local_connect\n");

    {
        HttpOp op;

        memset(&op, 0, sizeof(HttpOp));
        op.deal_error = int_error;

        op.recv_begin   = slice_recv_begin;
        op.recv_sync    = slice_recv_sync;
        op.recv_end     = slice_recv_end;

        strm_http_set_opset(task->http, &op, track);
    }

    return 0;
Err:
    return -1;
}

static void m3u8_recv_end(void* handle)
{
    int len;
    char *buf;
    HLiveMedia *media;
    AppleTask *task;
    HAppleTrack *track = (HAppleTrack*)handle;

    task = &track->switch_task;

    len = 0;
    ts_buf_read_get(task->recv_ts_buf, &buf, &len);
    if (len <= 0)
        goto Err;
    media = task->media;

    buf[len] = 0;
    //LOG_STRM_PRINTF("%s\n\n", buf);
    http_live_parse_slice(media, buf, len);
    ts_buf_read_pop(task->recv_ts_buf, len);

#ifdef USE_VERIMATRIX_OTT
    if (media->key_uri) {
        task->crypt_handle = apple_audio_load_key(media);
        if (!task->crypt_handle)
            LOG_STRM_ERROUT("apple_audio_load_key\n");
    }
#endif
    if (slice_request(track))
        LOG_STRM_ERROUT("slice_request\n");

    return;
Err:
    int_end(track);
}

static int slice_recv_begin(void* handle)
{
    int length;
    AppleTask *task;
    HAppleTrack *track = (HAppleTrack*)handle;

    LOG_STRM_PRINTF("#%d\n", track->index);

    task = &track->switch_task;
    length = (int)strm_http_get_contentLen(task->http);

    if (task->media->key_uri) {
#ifdef USE_VERIMATRIX_OTT
        if (task->crypt_ts_buf) {
            if (length > ts_buf_size(task->crypt_ts_buf)) {
                ts_buf_delete(task->crypt_ts_buf);
                task->crypt_ts_buf = NULL;
                yhw_mem_free(task->crypt_buffer);
                task->crypt_buffer = NULL;
            } else {
                ts_buf_reset(task->crypt_ts_buf);
            }
        }
        if (!task->crypt_ts_buf) {
            length = length - length % 16 + 16;
            yhw_mem_alloc(length, 0, (void **)&task->crypt_buffer);
            if(!task->crypt_buffer)
                LOG_STRM_ERROUT("yhw_mem_alloc size = %d\n", length);
            task->crypt_ts_buf = ts_buf_reload(task->crypt_buffer, length);
            if (!task->crypt_ts_buf)
                LOG_STRM_ERROUT("ts_buf_reload\n");
        }
        strm_http_buf_replace(task->http, task->crypt_ts_buf);
#else
        LOG_STRM_ERROUT("key_uri\n");
#endif
    } else {
        if (length > ts_buf_size(task->recv_ts_buf)) {
            ts_buf_delete(task->recv_ts_buf);
            task->recv_ts_buf = ts_buf_create(length);
            if (!task->recv_ts_buf)
                LOG_STRM_ERROUT("ts_buf_create size = %dn", length);
        } else {
            ts_buf_reset(task->recv_ts_buf);
        }
        strm_http_buf_replace(task->http, task->recv_ts_buf);
    }

    return 0;
Err:
    return -1;
}

static void slice_recv_sync(void* handle)
{
}

static void slice_recv_end(void* handle)
{
    int len;
    char *buf;

    HLiveMedia *media;
    AppleTask *task;
    HAppleTrack *track = (HAppleTrack*)handle;

    LOG_STRM_PRINTF("#%d\n", track->index);

    task = &track->switch_task;
    len = (int)strm_http_get_contentLen(task->http);

    media = task->media;
    if (media->key_uri) {
#ifdef USE_VERIMATRIX_OTT
        ymm_stream_cryptM2MStream(task->crypt_handle, task->crypt_buffer, len, task->crypt_buffer);
        buf = task->crypt_buffer;
#else
        LOG_STRM_ERROUT("key_uri\n");
#endif
    } else {
        buf = task->recv_buffer;
    }

    {
        int i;
        uint32_t pid;
        uint8_t *ubuf;
        struct ts_psi *psi = &task->ts_psi;

        task->ts_pid = 0;
        ts_parse_reset(task->ts_parse);
        IND_MEMSET(psi, 0, sizeof(struct ts_psi));
        task->ts_psi.dr_subtitle = task->ts_subtitle;

        if (1 != ts_parse_psi(task->ts_parse, (uint8_t*)buf, len, NULL))
            LOG_STRM_ERROUT("ts_parse_psi\n");

        switch(track->index) {
        case HAPPLE_TRACK_AUDIO:
            task->ts_pid = psi->audio_pid[0];
             break;
        case HAPPLE_TRACK_SUBTITLE:
            task->ts_pid = psi->dr_subtitle->subtitle_pid[0];
            break;
        default:
            LOG_STRM_ERROUT("index = %d\n", track->index);
            break;
        }
        LOG_STRM_PRINTF("ts_pid = %d\n", task->ts_pid);

        for (i = 0; i < len; i += 188) {
            ubuf = (uint8_t*)(buf + i);
            pid = (((uint)ubuf[1] & 0x1f) << 8) + ubuf[2];//program_number
            if (pid && pid == task->ts_pid) {
                ubuf[1] = (ubuf[1] & 0xe0) | (unsigned char)(track->pid >> 8);
                ubuf[2] = (unsigned char)track->pid;
            } else {
                ubuf[1] = 0x1F;
                ubuf[2] = 0xFF;
                ubuf[3] = 0x00;
                memset (ubuf + 4, 0xff, 184);
            }
        }
    }

    apple_buffer_switch(track, buf, len);

    if (track->switch_sequence >= 0) {
        if (slice_request(track))
            LOG_STRM_ERROUT("slice_request\n");
    } else {
        apple_switch_end(track);
    }

    return;
Err:
    int_end(track);
}

void apple_switch_begin(HAppleTrack* track)
{
    AppleTask *task = &track->switch_task;

    if (!task->http) {
       task->http = strm_http_create(track->ha->loop, 0);
       if (!task->http)
           goto Err;
    }

   if (apple_audio_m3u8_request(track, task->http, track->ltrack->array[task->index]->uri, 0))
        LOG_STRM_ERROUT("#%d apple_audio_m3u8_request\n", track->index);

    {
        HttpOp op;

        memset(&op, 0, sizeof(HttpOp));
        op.deal_error = int_error;

        op.recv_begin   = m3u8_recv_begin;
        op.recv_sync    = m3u8_recv_sync;
        op.recv_end     = m3u8_recv_end;

        strm_http_set_opset(task->http, &op, track);
    }

    return;
Err:
    apple_switch_end(track);
    return;
}

void apple_switch_end(HAppleTrack* track)
{
    AppleTask *task = &track->switch_task;

    track->switch_sequence = -1;

    if (task->media)
        http_live_reset_slice(task->media);

    if (task->http)
        strm_http_reset(task->http);
    apple_switch_clear(task);
}

#endif//#if SUPPORTE_HD == 1
