
/**************************************--**************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **************************************--**************************************/

#include "http_apple.h"

#if SUPPORTE_HD == 1

#define TIME_UNDERFLOW      10

#define APPLE_CRIPT_SIZE    (1316 * 196)

static void int_state(HAppleTrack* track, int livestate);
static int slice_request(HAppleTrack* track);

int apple_audio_m3u8_request(HAppleTrack* track, HTTP_t http, char* uri, int etag)
{
    char *p;
    char url[STREAM_URL_SIZE];
    HttpApple *ha = track->ha;

    if (0 == strncasecmp("http://", uri, 7)) {
        IND_STRCPY(url, uri);
    } else {
        int act = strm_http_get_active(http);

        if (act) {
            if ('/' == uri[0])
                sprintf(url, "%s", uri);
            else
                sprintf(url, "%s/%s", ha->index_path, uri);
        } else {
            if ('/' == uri[0])
                sprintf(url, "http://%s%s", ha->index_host, uri);
            else
                sprintf(url, "http://%s%s/%s", ha->index_host, ha->index_path, uri);
        }
    }

    if (2 == ha->servicetype) {
        int len;
        struct ind_time tp;

        memset(&tp, 0, sizeof(tp));
        len = strlen(url);
        ind_time_local(track->load_datetime, &tp);
        if (strchr(url, '?'))
            url[len] = '&';
        else
            url[len] = '?';
        len ++;
        sprintf(url + len, "playseek=%04d%02d%02d%02d%02d%02d-", 
            tp.year, tp.mon, tp.day, tp.hour, tp.min, tp.sec);
    } else if (3 == ha->servicetype) {
        int len;
        struct ind_time tp;

        len = strlen(url);
        p = strchr(url, '?');
        if (p) {
            p = strstr(url, "playseek=");
            if (!p)
                url[len] = '&';
        } else {
            url[len] = '?';
        }
        len ++;

        if (!p) {
            ind_time_local(ha->time_begin, &tp);
            len += sprintf(url + len, "playseek=%04d%02d%02d%02d%02d%02d-", 
                tp.year, tp.mon, tp.day, tp.hour, tp.min, tp.sec);
    
            ind_time_local(ha->time_end, &tp);
            sprintf(url + len, "%04d%02d%02d%02d%02d%02d", 
                tp.year, tp.mon, tp.day, tp.hour, tp.min, tp.sec);
        }
    }

    strm_http_set_etag(http, etag);
    strm_http_set_gzip(http, 1);
    if (strm_http_request(http, url, 0, 0))
        LOG_STRM_ERROUT("#%d strm_http_request\n", ha->index);

    return 0;
Err:
    return -1;
}

static int m3u8_request(HAppleTrack* track, char* uri, int etag)
{
    HttpApple *ha;
    AppleTask *task;

    ha = track->ha;
    task = &track->task;

    if (1 == ha->servicetype && HAPPLE_TRACK_VIDEO == track->index)
        ha->retry_clk = strm_httploop_clk(ha->loop);

    if (uri) {
        if (track->uri)
            IND_FREE(track->uri);
        track->uri = IND_STRDUP(uri);
    }
    if (!track->uri)
        LOG_STRM_ERROUT("#%d url is NULL\n", ha->index);

    if (apple_audio_m3u8_request(track, task->http, track->uri, etag))
        LOG_STRM_ERROUT("#%d apple_audio_m3u8_request\n", ha->index);

    LOG_STRM_PRINTF("#%d LIVE_STATE_M3U8\n", ha->index);
    int_state(track, LIVE_STATE_M3U8);

    return 0;
Err:
    return -1;
}

static int m3u8_recv_begin(void* handle)
{
    AppleTask *task;
    HAppleTrack *track = (HAppleTrack*)handle;

    LOG_STRM_PRINTF("\n");

    task = &track->task;
    ts_buf_reset(task->recv_ts_buf);
    strm_http_buf_replace(task->http, task->recv_ts_buf);

    return 0;
}

static void m3u8_recv_sync(void* handle)
{
}

#ifdef USE_VERIMATRIX_OTT
static int g_key_valid = 0;
static int write_data(void* ptr, size_t size, size_t nmemb, void* stream)
{
    int len;
    char *buf;

    len = size * nmemb;
    if(len != 16)
        LOG_STRM_ERROUT("len = %d\n", len);
    buf = (char*)stream;

    IND_MEMCPY(buf, ptr, 16);	
    g_key_valid = 1;

Err:	
	return len;
}

static int load_key(char *url, char *key_buf)
{
    CURL *http = NULL;

    //curl_global_init(CURL_GLOBAL_DEFAULT);
    http = curl_easy_init();

    g_key_valid = 0;

    curl_easy_setopt(http, CURLOPT_URL, url);
    curl_easy_setopt(http, CURLOPT_SSL_VERIFYPEER,0L); 
    //curl_easy_setopt(http, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(http, CURLOPT_WRITEFUNCTION, (void *)write_data);
    curl_easy_setopt(http, CURLOPT_WRITEDATA, key_buf);
    curl_easy_setopt(http, CURLOPT_CONNECTTIMEOUT_MS, 3000);
    curl_easy_setopt(http, CURLOPT_TIMEOUT_MS, 2000);
    /*
    if(cookie && strlen(cookie) > 0)
        res = curl_easy_setopt(http, CURLOPT_COOKIE, (void *)cookie);
    */
    curl_easy_perform(http);

    curl_easy_cleanup(http);

    if (g_key_valid)
        return 0;

	return -1;
}

void* apple_audio_load_key(HLiveMedia* media)
{
    void *handle;
    char key[16], iv[16];

    //获取key
    if (load_key(media->key_uri, key))
        LOG_STRM_ERROUT("yhw_mem_alloc size = %d\n", APPLE_CRIPT_SIZE);

    handle = ymm_stream_createCryptM2MHandle(YX_SECURITY_ALGORITHM_AES, YX_SECURITY_ALGORITHMVARIANT_CBC, 0, 1);
    if (!handle)
        LOG_STRM_ERROUT("ymm_stream_createCryptM2MHandle\n");

    if (media->key_iv) {
        IND_MEMCPY(iv, media->key_iv, 16);
    } else {
        uint32_t seq;
        uint8_t *ubuf;

        IND_MEMSET(iv, 0, 12);

        seq = media->x_sequence;
        ubuf = (uint8_t*)iv;

        ubuf[12] = seq >> 24;
        ubuf[13] = seq >> 16;
        ubuf[14] = seq >> 8;
        ubuf[15] = seq >> 0;
    }
    {
        uint8_t *ubuf;

        ubuf = (uint8_t *)key;
        printf("key: %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n", 
            ubuf[0], ubuf[1], ubuf[2],  ubuf[3],  ubuf[4],  ubuf[5],  ubuf[6],  ubuf[7], 
            ubuf[8], ubuf[9], ubuf[10], ubuf[11], ubuf[12], ubuf[13], ubuf[14], ubuf[15]);

        ubuf = (uint8_t *)iv;
        printf("iv:  %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n", 
            ubuf[0], ubuf[1], ubuf[2],  ubuf[3],  ubuf[4],  ubuf[5],  ubuf[6],  ubuf[7], 
            ubuf[8], ubuf[9], ubuf[10], ubuf[11], ubuf[12], ubuf[13], ubuf[14], ubuf[15]);
    }

    ymm_stream_loadM2MKeys(handle, key, 16, iv, 16);

    return handle;
Err:
    return NULL;
}
#endif

//直播基准分片
static int iptv_sequence(HttpApple *ha, HLiveMedia* media)
{
    int load_sequence;

    if (!ha->stream && media->slice_num > 5)
        load_sequence = media->x_sequence + 2;
    else if (media->slice_num > 3)
        load_sequence = media->x_sequence + (media->slice_num - 3);
    else if (media->slice_num == 3)
        load_sequence = media->x_sequence + (media->slice_num - 2);
    else
        load_sequence = media->x_sequence + (media->slice_num - 1);

    return load_sequence;
}

void apple_audio_m3u8_delay(HttpApple *ha)
{
    unsigned int clk;

    clk = strm_httploop_clk(ha->loop);

    if (1 == ha->servicetype) {
        unsigned int m3u8_clk;

        ha->retry_times++;
        m3u8_clk = ha->retry_base + ha->slice_duration * ha->retry_times * 50;
        LOG_STRM_PRINTF("retry_times = %d, m3u8_clk = %d / %d\n", ha->retry_times, m3u8_clk, clk);
        if (m3u8_clk < clk) {
            ha->retry_base = clk;
            ha->retry_times = 0;
            ha->m3u8_clk = clk;
        } else {
            ha->m3u8_clk = m3u8_clk;
        }
    } else {
        ha->m3u8_clk = clk + ha->slice_duration * 100;
    }
}

void apple_audio_m3u8_end(HttpApple *ha)
{
    int i, load_sequence;
    HLiveMedia *media;
    HAppleTrack* track;
    HAppleTrack **track_array;

    track_array = ha->track_array;

    for(i = 0; i < HAPPLE_TRACK_NUM; i++) {
        track = track_array[i];
        if (!track)
            continue;
        if (LIVE_STATE_IDLE != track->state)
            return;

        media = track->task.media;
        if (media->slice_num <= 0)
            LOG_STRM_ERROUT("track = %d, slice_num = %d\n", i, media->slice_num);
        if (0 == ha->servicetype && 0 == media->slice_complete)
            LOG_STRM_ERROUT("VOD invalid m3u8!\n");
    }

    load_sequence = ha->load_sequence;

    if (-1 == load_sequence) {
        for(i = 0; i < HAPPLE_TRACK_NUM; i++) {
            track = track_array[i];
            if (!track)
                continue;

            media = track->task.media;

            if (-1 == load_sequence) {
                if (0 == ha->servicetype || 3 == ha->servicetype) {
                    uint32_t seek;

                    ha->time_length = media->total_duration;
                    stream_back_totaltime(ha->index, ha->time_length);

                    seek = ha->time_seek;
                    if (seek > ha->time_length)
                        seek = ha->time_length;
                    if (seek > 0)
                        load_sequence = http_live_vodIndex(media, &seek);
                    else
                        load_sequence = 0;
                    load_sequence += media->x_sequence;

                    LOG_STRM_PRINTF("seek = %d / %d, load_sequence = %d\n", seek, ha->time_seek, load_sequence);
                    ha->time_start = seek;
                    ha->time_current = seek;
                    stream_back_currenttime(ha->index, ha->time_current);
                } else {
                    uint32_t now = mid_time( );

                    if (1 == ha->servicetype) {
                        HLiveSlice *slice;

                        load_sequence = iptv_sequence(ha, media);

                        slice = http_live_slice(media, load_sequence);
                        if (NULL == slice)
                            LOG_STRM_ERROUT("slice not exist! load_sequence = %d\n", load_sequence);
                        track->serv_diff = slice->datetime - now;
                        LOG_STRM_PRINTF("serv_diff = %d\n", track->serv_diff);
                    } else {
                        int idx;

                        LOG_STRM_DEBUG("datetime = %d\n", track->load_datetime);
                        idx = http_live_iptvIndex(media, &track->load_datetime);
                        if (idx < 1)
                            LOG_STRM_ERROUT("idx = %d\n", idx);
                        LOG_STRM_PRINTF("datetime = %d, idx\n", track->load_datetime);
                        ha->time_start = track->load_datetime - track->serv_diff;
                        load_sequence = media->x_sequence + idx;
                    }
                }
            } else {
                if (load_sequence < media->x_sequence || load_sequence >= media->x_sequence + media->slice_num)
                    LOG_STRM_ERROUT("track = %d, load_sequence = %d / %d / %d\n", i, load_sequence, media->x_sequence, media->slice_num);
            }
        }
    } else {
        int refresh = 0;

        for(i = 0; i < HAPPLE_TRACK_NUM; i++) {
            track = track_array[i];
            if (!track)
                continue;

            media = track->task.media;
            if (load_sequence < media->x_sequence) {
                if (1 == ha->servicetype && 0 == i)
                    load_sequence = iptv_sequence(ha, media);
                else
                    LOG_STRM_ERROUT("track = %d, load_sequence = %d / %d\n", i, load_sequence, media->x_sequence);
            }
            if (load_sequence >= media->x_sequence + media->slice_num) {
                if (0 == ha->servicetype)
                    LOG_STRM_ERROUT("track = %d, load_sequence = %d / %d / %d\n", i, load_sequence, media->x_sequence, media->slice_num);

                refresh = 1;
            }
        }

        if (refresh) {
            apple_audio_m3u8_delay(ha);
            return;
        }
    }

    ha->load_sequence = load_sequence;
    LOG_STRM_PRINTF("LIVE_STATE_SLICE servicetype = %d, load_sequence = %d\n", ha->servicetype, load_sequence);

    if (slice_request(track_array[HAPPLE_TRACK_VIDEO]))
        LOG_STRM_ERROUT("slice_request\n");

    return;
Err:
    strm_httploop_break(ha->loop);
    stream_post_msg(ha->index, STRM_MSG_OPEN_ERROR, 0);
}

#ifdef USE_VERIMATRIX_OTT
int create_crypt(HAppleTrack *track)
{
    AppleTask* task = &track->task;

    if (task->crypt_handle)
        ymm_stream_destoryCryptM2MKeyHandle(task->crypt_handle);
    task->crypt_handle = apple_audio_load_key(task->media);
    if (!task->crypt_handle)
        LOG_STRM_ERROUT("apple_audio_load_key\n");

    if (!task->crypt_ts_buf) {
        yhw_mem_alloc(APPLE_CRIPT_SIZE, 0, (void **)&task->crypt_buffer);
        if(!task->crypt_buffer)
            LOG_STRM_ERROUT("yhw_mem_alloc size = %d\n", APPLE_CRIPT_SIZE);

        task->crypt_ts_buf = ts_buf_reload(task->crypt_buffer, APPLE_CRIPT_SIZE);
        if (!task->crypt_ts_buf)
            LOG_STRM_ERROUT("ts_buf_reload\n");
    }

    return 0;
Err:
    apple_switch_clear_crypt(task);
    return -1;
}
#endif

static void m3u8_recv_end(void* handle)
{
    int len;
    char *buf;
    ts_buf_t ts_buf;
    HttpApple *ha;
    AppleTask *task;
    HAppleTrack *track = (HAppleTrack*)handle;

    ha = track->ha;
    task = &track->task;
    ts_buf = task->recv_ts_buf;

    len = 0;
    ts_buf_reload_get(ts_buf, &buf, &len);
    if (len > 0)
        ts_buf_reload_mark(ts_buf, len);

    len = 0;
    strm_http_buf_get(task->http, &buf, &len);
    if (len > 0) {
        HLiveMedia *media = task->media;

        buf[len] = 0;
        http_live_parse_slice(media, buf, len);
        strm_http_buf_pop(task->http, len);

        if (media->key_uri) {
#ifdef USE_VERIMATRIX_OTT
            if (!track->key_uri || strcmp(media->key_uri, track->key_uri)) {
                if (create_crypt(track))
                    LOG_STRM_ERROUT("create_crypt\n");
            }
            if (track->key_uri)
                IND_FREE(track->key_uri);
            track->key_uri = media->key_uri;
            media->key_uri = NULL;
#else
            LOG_STRM_ERROUT("key_uri\n");
#endif
        } else {
            if (track->key_uri) {
                IND_FREE(track->key_uri);
                track->key_uri = NULL;
            }
        }
    }

    strm_http_get_host(task->http, task->m3u8_host);
    strm_http_get_uri(task->http, task->m3u8_path);

    track->state = LIVE_STATE_IDLE;
    if (HAPPLE_TRACK_VIDEO == track->index) {
        int load_sequence;
        HLiveMedia *media;

        media = track->task.media;
        load_sequence = ha->load_sequence;

        if (ha->slice_duration <= 0)
            ha->slice_duration = media->x_duration;

        if (track->isSkipSlice) {
            HLiveStream *stream;

            track->isSkipSlice = 0;

            stream = ha->stream;
            if (stream) {
                stream = stream->prev;
                if (stream && load_sequence >= media->x_sequence && load_sequence < media->x_sequence + media->slice_num) {
                    //降码率
                    ha->stream = stream;
                    if (!m3u8_request(track, stream->uri, 0))
                        LOG_STRM_ERROUT("#%d m3u8_request", ha->index);
                    return;
                }
            }
            //跳片
            load_sequence = iptv_sequence(ha, media);
            ha->load_sequence = load_sequence;
        }

        if (1 == ha->servicetype && (-1 == load_sequence || load_sequence < media->x_sequence + media->slice_num)) {
            ha->retry_base = ha->retry_clk;
            ha->retry_times = 0;
        }
    }

    apple_audio_m3u8_end(ha);

    return;
Err:
    strm_httploop_break(track->ha->loop);
    stream_post_msg(track->ha->index, STRM_MSG_OPEN_ERROR, 0);
}

static void m3u8_error(void* handle, HTTP_MSG msgno)
{
    HttpApple *ha;
    HAppleTrack *track = (HAppleTrack*)handle;

    ha = track->ha;
    ha->error_code = strm_http_get_code(track->task.http);

    if (!ha->data_flag) { //数据播放完毕再报错
        strm_httploop_break(ha->loop);
        stream_post_msg(ha->index, STRM_MSG_OPEN_ERROR, ha->error_code);
    }
}

static int slice_recv_begin(void* handle)
{
    HttpApple *ha;
    AppleTask *task;
    HAppleTrack *track = (HAppleTrack*)handle;

    ha = track->ha;
    task = &track->task;
    track->load_length = (int)strm_http_get_contentLen(task->http);

    task->ts_pid = 0;
    ts_parse_reset(task->ts_parse);
    IND_MEMSET(&task->ts_psi, 0, sizeof(struct ts_psi));
    task->ts_psi.dr_subtitle = task->ts_subtitle;

    if (track->key_uri) {
#ifdef USE_VERIMATRIX_OTT
        ts_buf_reset(task->crypt_ts_buf);
        strm_http_buf_replace(task->http, task->crypt_ts_buf);
#endif
    } else {
        ts_buf_reset(task->recv_ts_buf);
        strm_http_buf_replace(task->http, task->recv_ts_buf);
    }

    track->isSliceBegun = 1;
    apple_buffer_slice_valid(track, 1);

    if (HAPPLE_TRACK_VIDEO == track->index) {
        track = ha->track_array[HAPPLE_TRACK_AUDIO];
        if (track) {
            if (slice_request(track))
                LOG_STRM_ERROUT("slice_request audio\n");
        }
        track = ha->track_array[HAPPLE_TRACK_SUBTITLE];
        if (track) {
            if (slice_request(track))
                LOG_STRM_ERROUT("slice_request subtitle\n");
        }
    }

    return 0;
Err:
    strm_httploop_break(ha->loop);
    stream_post_msg(ha->index, STRM_MSG_OPEN_ERROR, 0);
    return -1;
}

static void slice_recv_sync(void* handle)
{
}

static void slice_recv_end(void* handle)
{
    HAppleTrack *track = (HAppleTrack*)handle;

    apple_buffer_slice_valid(track, 2);

    if (HAPPLE_TRACK_VIDEO == track->index) {
        HttpApple* ha = track->ha;
        int bandwidth;
        long long length;

        if (track->load_duration > 0)
            ha->slice_duration = track->load_duration;

        length = track->load_length;
        bandwidth = (int)(length * 100 / (mid_10ms( ) - track->load_clk)) * 8;
        ha->load_bandwidth = bandwidth;
        if (track->ha->index != STREAM_INDEX_PIP)
            int_steam_setDownloadRate(bandwidth);

        http_apple_bandwidth(track->ha, bandwidth);

        //Bsmooth = 0.3 * Bsmooth' + 0.7 * Breal);
        track->load_bandsmooth = (track->load_bandsmooth * 3 + bandwidth * 7) / 10;
        LOG_STRM_PRINTF("bandwidth = %d, bandsmooth = %d\n", bandwidth, track->load_bandsmooth);
    }

    track->state = LIVE_STATE_IDLE;
    apple_buffer_slice_end(track);
}

static void slice_error(void* handle, HTTP_MSG msgno)
{
    int code;
    HttpApple *ha;
    HAppleTrack *track = (HAppleTrack*)handle;

    ha = track->ha;
    code = strm_http_get_code(track->task.http);
    LOG_STRM_PRINTF("#%d code = %d\n", ha->index, code);

    //isSliceBegun 为真表示不完整分片，直接报错
    if (1 == ha->servicetype && HAPPLE_TRACK_VIDEO == track->index && !track->isSliceBegun) {
        HLiveStream *stream = ha->stream;

        if (!stream && HTTP_CODE_Internal_Server_Error == code) {
            ha->slice_clk = strm_httploop_clk(ha->loop) + ha->slice_duration * 100;
            return;
        }

        if (HTTP_CODE_Not_Found == code) {
            track->isSkipSlice = 1;
            if (!m3u8_request(track, NULL, 0))
                return;
            track->isSkipSlice = 0;
            LOG_STRM_ERROR("#%d m3u8_request", ha->index);
        }

        //降码率
        if (stream) {
            stream = stream->prev;
            if (stream) {
                ha->stream = stream;
                if (!m3u8_request(track, stream->uri, 0))
                    return;
                LOG_STRM_ERROR("#%d m3u8_request", ha->index);
            }
        }
    }
    ha->error_code = code;
    if (!ha->data_flag) { //数据播放完毕再报错
        strm_httploop_break(ha->loop);
        stream_post_msg(ha->index, STRM_MSG_OPEN_ERROR, ha->error_code);
    }
}

static void int_state(HAppleTrack* track, int livestate)
{
    HttpOp op;

    memset(&op, 0, sizeof(HttpOp));

    switch(livestate) {
    case LIVE_STATE_M3U8:
        op.recv_begin   = m3u8_recv_begin;
        op.recv_sync    = m3u8_recv_sync;
        op.recv_end     = m3u8_recv_end;
        op.deal_error   = m3u8_error;
        break;
    case LIVE_STATE_SLICE:
        op.recv_begin   = slice_recv_begin;
        op.recv_sync    = slice_recv_sync;
        op.recv_end     = slice_recv_end;
        op.deal_error   = slice_error;
        break;
    default:
        LOG_STRM_ERROR("livestate = %d\n", livestate);
        return;
    }

    strm_http_set_opset(track->task.http, &op, track);

    track->state = livestate;
}

void apple_audio_error(HAppleTrack* track)
{
    HttpApple *ha;

    ha = track->ha;
    LOG_STRM_DEBUG("state = %d, stream = %p\n", track->state, ha->stream);

    if (STRM_STATE_IPTV != ha->state || -1 == ha->load_sequence) {
        strm_httploop_break(ha->loop);
        stream_post_msg(ha->index, STRM_MSG_OPEN_ERROR, 0);
    } else {
        strm_http_reset(track->task.http);

        LOG_STRM_PRINTF("LIVE_STATE_IDLE\n");
        track->state = LIVE_STATE_IDLE;
        ha->m3u8_clk = strm_httploop_clk(ha->loop) + ha->slice_duration * 100;
    }
}

//Bbitrate = [1 + (Tbuf - Tunderflow) / Tseg] * Bbandwidth
static HLiveStream* select_stream(HttpApple* ha)
{
    int bitrate, buffer;
    HAppleTrack *video;
    HLiveStream *stream;

    stream = ha->stream;
    if (!stream)
        return NULL;
    video = ha->track_array[HAPPLE_TRACK_VIDEO];
    buffer = apple_buffer_duration(ha);

    {//bitrate = (1 + (Tbuf - Tunderflow) / Tseg) / bndwidth;
        int duration;
        HLiveSlice *slice;
        HLiveMedia *media;
        long long bandwidth;

        buffer = apple_buffer_duration(ha) / 100;
        media = video->task.media;
        slice = http_live_slice(media, ha->load_sequence);
        if (slice)
            duration = slice->duration;
        else
            duration = media->x_duration;

        bandwidth = video->load_bandsmooth;
        if (buffer <= TIME_UNDERFLOW)
            bitrate = (int)(bandwidth * 4 / 5);
        else
            bitrate = (int)(bandwidth * (duration + buffer - TIME_UNDERFLOW) / duration);
        LOG_STRM_PRINTF("#%d duration = %d, buffer = %d, bitrate = %d, bandwidth = %d\n", ha->index, duration, buffer, bitrate, stream->bandwidth);
    }

    if (bitrate > stream->bandwidth) {
        HLiveStream *next = stream->next;

        if (next && next->bandwidth <= bitrate && next->bandwidth < ha->load_bandwidth)
            stream = next;
    } else if (bitrate < stream->bandwidth) {
        HLiveStream *prev = stream->prev;

        while (stream->bandwidth > bitrate && prev) {
            stream = prev;
            prev = stream->prev;
        }
    }
    if (stream != ha->stream)
        LOG_STRM_PRINTF("#%d bandwidth = %d\n", ha->index, stream->bandwidth);

    return stream;
}

int apple_audio_m3u8_refresh(HttpApple* ha, int etag)
{
    int i;
    char *uri;
    HAppleTrack *track;
    HLiveStream *stream;

    if (-1 == ha->load_sequence || LIVE_STATE_INDEX == ha->track_array[0]->state)
        stream = ha->stream;
    else
        stream = select_stream(ha);

    ha->m3u8_clk = 0;
    ha->slice_clk = 0;

    for (i = 0; i < HAPPLE_TRACK_NUM; i++) {
        track = ha->track_array[i];
        if (!track)
            continue;

        switch(i) {
        case HAPPLE_TRACK_VIDEO:
            if (stream)
                uri = stream->uri;
            else
                uri = ha->url;
            break;
        case HAPPLE_TRACK_AUDIO:
        case HAPPLE_TRACK_SUBTITLE:
            uri = track->ltrack->array[track->task.index]->uri;
            break;
        default:
            goto Err;
        }

        track->isSkipSlice = 0;
        if (HAPPLE_TRACK_VIDEO == i && stream && stream != ha->stream) {
            ha->stream = stream;
            if (m3u8_request(track, uri, 0))
                LOG_STRM_ERROUT("#%d m3u8_request", ha->index);
        } else {
            if (m3u8_request(track, uri, etag))
                LOG_STRM_ERROUT("#%d m3u8_request\n", ha->index);
        }
    }

    return 0;
Err:
    strm_httploop_break(ha->loop);
    stream_post_msg(ha->index, STRM_MSG_OPEN_ERROR, 0);
    return -1;
}

//分片请求
static int slice_request(HAppleTrack* track)
{
    char *uri, url[STREAM_URL_SIZE];
    HttpApple *ha;
    AppleTask *task;
    HLiveSlice *slice;

    ha = track->ha;
    task = &track->task;

    apple_buffer_slice_valid(track, 0);

    slice = http_live_slice(task->media, ha->load_sequence);
    if (!slice)
        LOG_STRM_ERROUT("slice is NULL! load_sequence = %d\n", ha->load_sequence);

    uri = slice->uri;
    if (HAPPLE_TRACK_VIDEO == track->index && STREAM_INDEX_PIP != ha->index)
        int_steam_setCurSegment(uri);

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
    strm_http_set_gzip(task->http, 0);
    track->isSliceBegun = 0;
    if (strm_http_request(task->http, url, 0, 0))
        LOG_STRM_ERROUT("local_connect\n");

    track->load_duration = slice->duration;
    track->load_datetime = slice->datetime;

    LOG_STRM_PRINTF("LIVE_STATE_SLICE url = %s\n", url);
    int_state(track, LIVE_STATE_SLICE);

    track->load_clk = mid_10ms( );

    return 0;
Err:    
    return -1;
}

void apple_audio_slice_download(HttpApple* ha)
{
    HLiveStream *stream;
    HAppleTrack *track, **track_array;

    track_array = ha->track_array;
    track = track_array[HAPPLE_TRACK_VIDEO];

    ha->slice_clk = 0;

    stream = select_stream(ha);
    if (stream && stream != ha->stream) {
        ha->stream = stream;
        track->isSkipSlice = 0;
        m3u8_request(track, stream->uri, 0);
        return;
    }

    if (ha->alternative_audio) {
        int change;

        change = 0;

        track = track_array[HAPPLE_TRACK_AUDIO];
        if (track && track->switch_index != track->task.index) {
            track->task.index = track->switch_index;
            m3u8_request(track, track->ltrack->array[track->switch_index]->uri, 0);
            change = 1;
        }
        track = track_array[HAPPLE_TRACK_SUBTITLE];
        if (track && track->switch_index != track->task.index) {
            track->task.index = track->switch_index;
            m3u8_request(track, track->ltrack->array[track->switch_index]->uri, 0);
            change = 1;
        }

        if (change)
            return;
    }

    if (slice_request(track_array[HAPPLE_TRACK_VIDEO]))
        LOG_STRM_ERROUT("slice_request\n");

    return;
Err:
    strm_httploop_break(ha->loop);
    stream_post_msg(ha->index, STRM_MSG_OPEN_ERROR, 0); 
}

#endif//#if SUPPORTE_HD == 1
