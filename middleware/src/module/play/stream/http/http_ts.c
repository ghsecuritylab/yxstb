/**************************************--**************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **************************************--**************************************/

#include "http.h"
#include "http_ts.h"
#include "config/pathConfig.h"

//2012-11-3 15:07:09 youtube退出只剩20M内存，所以把内存由50M降到20M
#define HTTP_SBQ_NUM_LIVE   (16 * 1024)//华为要求100M，这么大内存没必要，所以减半
#define HTTP_SBQ_NUM_NORMAL 1024

//#define ENABLE_SAVE_HTTP

/*************************** HTTP Live Streaming ******************************/

enum {
    LIVE_STATE_FILE = 0,
    LIVE_STATE_M3U8,
    LIVE_STATE_SLICE,
};

/********************************** HTTP TS ***********************************/

typedef struct {
    uint32_t magic;
    long long key;
}  HttpPvr;

HttpPvr g_httppvr = {0, 0};

int g_bitrate = 0;

static void int_error(void* handle, HTTP_MSG msgno);

static void slice_end(HttpTs* ht);

static void file_recv_sync(void* handle);

static int slice_select(HttpTs* ht);
static void slice_ontimer(HttpTs* ht);

static int int_hls(HttpTs* ht, int livestate);

static void int_100ms(void* handle)
{
    uint32_t clk;
    HttpTs *ht = (HttpTs*)handle;

    if (ht->hls_state != LIVE_STATE_M3U8)
        file_recv_sync(ht);

    clk = strm_httploop_clk(ht->loop);

    if (ht->ts_handle)
        ht->ts_op.ht_100ms(ht->ts_handle);

    if (ht->hls_clk && ht->hls_clk < clk) {
        ht->hls_clk = 0;
        slice_ontimer(ht);
    }

    if (2 == ht->order_flg && strm_bufque_length(ht->sbq) <= 0) {
        ht->order_flg = 1;

        ht->hls_index ++;
        slice_select(ht);
    }
}

static void int_sync(void* handle)
{
    HttpTs *ht = (HttpTs*)handle;

    if (ht->ts_handle)
        ht->ts_op.ht_1000ms(ht->ts_handle);
}

static void int_open_init(HttpTs* ht, char *url)
{
    int_hls(ht, LIVE_STATE_FILE);

    IND_STRCPY(ht->url, url);
}

static int file_recv_begin(void* handle)
{
    long long content_length;
    char content_type[CONTENT_TYPE_SIZE];
    HttpTs *ht = (HttpTs*)handle;

    content_length = strm_http_get_contentLength(ht->http);
    strm_http_get_contentType(ht->http, content_type);
    LOG_STRM_PRINTF("#%d content_length = %lld, content_type = %s\n", ht->index, content_length, content_type);
    if (0 == strcasecmp(content_type, "application/x-mpegurl") || 0 == strcasecmp(content_type, "application/vnd.apple.mpegurl")) {
        LOG_STRM_PRINTF("#%d m3u8\n", ht->index);
        ht->hls = http_live_create( );
        if (!ht->hls) {
            LOG_STRM_ERROR("hls_create\n");
            int_error(ht, 0);
            return -1;
        }
        int_hls(ht, LIVE_STATE_M3U8);
        return 0;
    }
    strm_http_sbq_replace(ht->http, ht->sbq, TS_TIMESHIFT_SIZE);

    if (ht->ts_handle)
        return ht->ts_op.file_recv_begin(ht->ts_handle);

    return 0;
}

static void file_recv_sync(void* handle)
{
    int l;
    StrmBuffer* sb;
    HttpTs *ht = (HttpTs*)handle;

    while (1) {
        if (ht->ts_op.ht_recv_space(ht->ts_handle) < 188)
            break;

        if (ht->sb->len <= 0) {
            if (strm_bufque_length(ht->sbq) <= 0) {
                if (1 == ht->end_flg) {
                    if (LIVE_STATE_SLICE == ht->hls_state) {
                        ht->end_flg = 0;
                        slice_end(ht);
                    } else {
                        ht->end_flg = 2;
                        if (ht->ts_handle)
                            ht->ts_op.file_recv_end(ht->ts_handle, 2);
                    }
                }
                return;
            }

            strm_bufque_pop(ht->sbq, &ht->sb);
            sb = ht->sb;

#ifdef ENABLE_SAVE_HTTP
            if (ht->save_fp) {
                if (fwrite(sb->buf, 1, sb->len, ht->save_fp) != len)
                    LOG_STRM_WARN("#%d fwrite\n", ht->index);
                ht->save_len += sb->len;
            }
#endif
            if (ht->byte_skip > 0) {
                if (sb->len <= ht->byte_skip) {
                    ht->byte_skip -= sb->len;
                    sb->len = 0;
                    continue;
                }
                sb->off += ht->byte_skip;
                sb->len -= ht->byte_skip;
            }
        }

        l = 0;
        if (ht->ts_handle)
            l = ht->ts_op.ht_recv_push(ht->ts_handle);
        if (l <= 0)
            break;
    }
}

static void file_recv_end(void* handle)
{
    HttpTs *ht = (HttpTs*)handle;

    LOG_STRM_PRINTF("#%d\n", ht->index);

    ht->end_flg = 1;
    if (ht->ts_handle)
        ht->ts_op.file_recv_end(ht->ts_handle, 1);
}

static int m3u8_recv_begin(void* handle)
{
#ifdef DEBUG_BUILD
    HttpTs *ht = (HttpTs*)handle;

    LOG_STRM_PRINTF("#%d\n", ht->index);
#endif
    return 0;
}

static void m3u8_recv_sync(void* handle)
{
}

static void m3u8_recv_end(void* handle)
{
    HttpTs *ht = (HttpTs*)handle;
    HLiveMedia *media;

    media = &ht->hls->media;

    {
        int len;
        char *buf;

        len = 0;
        strm_http_buf_get(ht->http, &buf, &len);
        http_live_parse_slice(media, buf, len);
        strm_http_buf_pop(ht->http, len);
    }

    if (media->slice_num <= 0)
        LOG_STRM_ERROUT("#%d slice_num = %d\n", ht->index, media->slice_num);

    LOG_STRM_PRINTF("#%d slice_num = %d, slice_complete = %d, duration = %d / %d\n", ht->index, media->slice_num, media->slice_complete, media->x_duration, media->total_duration);

    if (ht->time_start > 0) {
        if (ht->time_start >= media->total_duration) {
            LOG_STRM_WARN("#%d time_start = %d, total_duration = %d\n", ht->index, ht->time_start, media->total_duration);
            ht->time_start = media->total_duration - 1;
        }
        ht->hls_index = http_live_vodIndex(media, &ht->time_start);
        if (ht->hls_index <= 0)
            LOG_STRM_ERROUT("#%d hls_slce_index, start = %d, total = %d\n", ht->hls_index, ht->time_start, media->total_duration);
    }
    ht->byte_skip = 0;

    strm_http_get_uri(ht->http, ht->uri);
    {
        char *p = strchr(ht->uri, '?');
        if (p)
            *p = 0;
        p = strrchr(ht->uri, '/');
        if (!ht->uri)
            LOG_STRM_ERROUT("#%d\n", ht->index);
        ht->uri_base = (int)(p - ht->uri) + 1;
    }

    if (slice_select(ht))
        LOG_STRM_ERROUT("#%d slice_select\n", ht->index);

    int_hls(ht, LIVE_STATE_SLICE);
    return;
Err:
    int_error(ht, 0);
    return;
}

static int slice_select(HttpTs* ht)
{
    HLiveSlice *slice;
    HLiveMedia *media = &ht->hls->media;

    if (ht->hls_index >= media->slice_num) {
        if (media->slice_complete) {
            if (ht->ts_handle)
                ht->ts_op.file_recv_end(ht->ts_handle, 2);
        } else {
            strm_http_reset(ht->http);
            //等待下一分片完成
            ht->hls_clk = strm_httploop_clk(ht->loop) + media->x_duration * 100;
        }
    } else {
        char *uri;

        slice = http_live_slice(media, media->x_sequence + ht->hls_index);
        if (!slice)
            LOG_STRM_ERROUT("#%d index = %d, slice_num = %d\n", ht->index, ht->hls_index, media->slice_num);

        strm_http_set_etag(ht->http, 0);
        if (0 == strncasecmp(slice->uri, "http://", 7) || '/' == slice->uri[0]) {
            LOG_STRM_PRINTF("\n");
            uri = slice->uri;
        } else {
            LOG_STRM_PRINTF("\n");
            IND_STRCPY(ht->uri + ht->uri_base, slice->uri);
            uri = ht->uri;
        }
        if (strm_http_request(ht->http, uri, 0, 0))
            LOG_STRM_ERROUT("#%d local_connect\n", ht->index);
    }

    return 0;
Err:
    return -1;
}

static void slice_ontimer(HttpTs* ht)
{
    int_hls(ht, LIVE_STATE_M3U8);

    strm_http_set_etag(ht->http, 1);
    strm_http_request(ht->http, ht->url, 0, 0);
}

static int slice_recv_begin(void* handle)
{
    HttpTs *ht = (HttpTs*)handle;
    HLiveMedia *media;

    media = &ht->hls->media;
    if (ht->time_start > 0) {
        HLiveSlice *slice;
        long long len = strm_http_get_contentLen(ht->http);

        slice = http_live_slice(media, media->x_sequence + ht->hls_index);
        if (slice) {
            LOG_STRM_PRINTF("#%d time_start = %d, duration = %d\n", ht->index, ht->time_start, slice->duration);
            ht->byte_skip = (int)(len * ht->time_start / slice->duration);
            ht->byte_skip = ht->byte_skip - ht->byte_skip % 188;
        }
        ht->time_start = 0;
    }
    strm_http_sbq_replace(ht->http, ht->sbq, TS_TIMESHIFT_SIZE);

    if (ht->ts_handle)
        ht->ts_op.slice_recv_begin(ht->ts_handle);

    return 0;
}

static void slice_end(HttpTs* ht)
{
#ifdef DEBUG_BUILD
    HLiveMedia *media = &ht->hls->media;
    LOG_STRM_PRINTF("#%d slice index = %d, num = %d, complete = %d\n", ht->index, ht->hls_index, media->slice_num, media->slice_complete);
#endif

    if (ht->ts_handle)
        ht->ts_op.slice_recv_end(ht->ts_handle);

    if (1 == ht->order_flg && strm_bufque_length(ht->sbq) > 0) {
        ht->order_flg = 2;
    } else {
        ht->hls_index ++;
        slice_select(ht);
    }
}

static void slice_recv_end(void* handle)
{
    HttpTs *ht = (HttpTs*)handle;

    LOG_STRM_PRINTF("#%d sync_flg = %d\n", ht->index, ht->sync_flg);

    if (ht->sync_flg)
        ht->end_flg = 1;
    else
        slice_end(ht);
}

static void int_error(void* handle, HTTP_MSG httpmsg)
{
    STRM_MSG strmmsg;
    HttpTs *ht = (HttpTs*)handle;

    LOG_STRM_DEBUG("#%d ts_handle = %p, httpmsg = %d\n", ht->index, ht->ts_handle, httpmsg);

    switch (httpmsg) {
    case HTTP_MSG_NET_TIMEOUT:      strmmsg = RECORD_MSG_NET_TIMEOUT;       break;
    case HTTP_MSG_REFUSED_SOCKET:   strmmsg = RECORD_MSG_REFUSED_SOCKET;    break;
    case HTTP_CODE_Forbidden:   	strmmsg = RECORD_MSG_REFUSED_SERVER;    break;
    case HTTP_CODE_Not_Found:       strmmsg = RECORD_MSG_NOT_FOUND;         break;
    default:                        strmmsg = RECORD_MSG_ERROR;             break;
    }

    if (ht->ts_handle)
        ht->ts_op.ht_deal_error(ht->ts_handle, strmmsg);
}

static void int_msg(void* handle, STRM_MSG msgno, int arg)
{
    HttpTs *ht = (HttpTs*)handle;

    LOG_STRM_DEBUG("#%d ts_handle = %p, msgno = %d\n", ht->index, ht->ts_handle, msgno);

    if (ht->ts_handle)
        ht->ts_op.ht_deal_msg(ht->ts_handle, msgno, arg);
}

static void int_cmd(void* handle, StreamCmd* strmCmd)
{
    HttpTs *ht = (HttpTs*)handle;
    LOG_STRM_DEBUG("#%d ts_handle = %p, cmd = %d\n", ht->index, ht->ts_handle, strmCmd->cmd);

    if (ht->ts_handle)
        ht->ts_op.ht_deal_cmd(ht->ts_handle, strmCmd);
}

/*
 APP_TYPE_HLS 播放需要申请大内存
 */
static HttpTs* httpts_create(int idx, mid_msgq_t msgq, APP_TYPE apptype, int size, int num)
{
    HttpTs* ht;
    HttpLoopOp op;

    ht = (HttpTs*)IND_CALLOC(sizeof(HttpTs), 1);
    if (!ht)
        LOG_STRM_ERROUT("malloc failed!\n");

    ht->index = idx;

    memset(&op, 0, sizeof(op));

    op.deal_cmd = int_cmd;
    op.deal_msg = int_msg;
    op.local_100ms = int_100ms;
    op.local_1000ms = int_sync;

    ht->loop = strm_httploop_create(idx, &op, ht, msgq);
    if (!ht->loop)
        LOG_STRM_ERROUT("strm_httploop_create!\n");

    ht->http = strm_http_create(ht->loop, size);
    if (!ht->http)
        LOG_STRM_ERROUT("strm_http_create!\n");

    ht->sb = strm_buf_malloc(TS_TIMESHIFT_SIZE);
    ht->sbq = strm_bufque_create(TS_TIMESHIFT_SIZE, num);
    if (!ht->sbq)
        LOG_STRM_ERROUT("strm_bufque_create!\n");

    return ht;
Err:
    if (ht) {
        if (ht->sb)
            strm_buf_free(ht->sb);
        if (ht->loop)
            strm_httploop_delete(ht->loop);
        IND_FREE(ht);
    }
    return NULL;
}

static void httpts_delete(HttpTs* ht)
{
    if (!ht)
        return;

    if (ht->sb)
        strm_buf_free(ht->sb);
    if (ht->sbq)
        strm_bufque_delete(ht->sbq);
    if (ht->hls)
        http_live_delete(ht->hls);
    if (ht->loop)
        strm_httploop_delete(ht->loop);

    IND_FREE(ht);
}

static void int_loop(HttpTs* ht)
{
#ifdef ENABLE_SAVE_HTTP
    ht->save_len = 0;
    ht->save_fp = fopen(DEFAULT_EXTERNAL_DATAPATH"/ht.ts", "wb");
    LOG_STRM_PRINTF("@@@@@@@@: fp = %p\n", ht->save_fp);
#endif

    strm_httploop_loop(ht->loop);

#ifdef ENABLE_SAVE_HTTP
    LOG_STRM_PRINTF("@@@@@@@@: save = %lld\n", ht->save_len);
    if (ht->save_fp) {
        fclose(ht->save_fp);
        ht->save_fp = NULL;
    }
#endif
}

static void int_loop_play(void *handle, int idx, mid_msgq_t msgq, PlayArg *arg, char *argbuf)
{
    int num, size;
    HttpTs *ht;
    HttpTsArg *httparg;

#ifdef INCLUDE_PVR
    if (arg->shiftid) {
        num = HTTP_SBQ_NUM_NORMAL;
        size = HTTP_M3U8_SIZE;
    } else
#endif
    {
        if (APP_TYPE_HLS == arg->apptype) {
            num = HTTP_SBQ_NUM_LIVE;
            size = HTTP_M3U8_SIZE;
        } else {
            num = HTTP_SBQ_NUM_NORMAL;
            size = 0;
        }
    }

    ht = httpts_create(idx, msgq, arg->apptype, size, num);
    if (!ht)
        LOG_STRM_ERROUT("#%d http is NULL\n", idx);

    httparg = (HttpTsArg *)argbuf;
    int_open_init(ht, httparg->url);

#ifdef INCLUDE_PVR
    if (arg->shiftid) {
        ht->ts_handle = http_ts_record_create(ht, arg, NULL, httparg);
    } else
#endif
    {
        if (APP_TYPE_HLS == arg->apptype) {
            ht->ts_handle = http_ts_live_create(ht, arg, httparg);
        } else {
            ht->order_flg = 1;
            ht->ts_handle = http_ts_play_create(ht, arg, httparg);
        }
    }
    if (!ht->ts_handle)
        LOG_STRM_ERROUT("#%d http_ts_create apptype = %d\n", idx, arg->apptype);

    int_loop(ht);
    LOG_STRM_PRINTF("#%d exit local_loop!\n", idx);

    if (ht->ts_handle)
        ht->ts_op.ht_destroy(ht->ts_handle);

Err:
    if (ht)
        httpts_delete(ht);
    return;
}

#ifdef INCLUDE_PVR
static void int_loop_record(void *handle, int idx, mid_msgq_t msgq, RecordArg *arg, char *argbuf)
{
    HttpTs *ht;
    HttpTsArg *httparg;

    ht = httpts_create(idx, msgq, 0, HTTP_M3U8_SIZE, HTTP_SBQ_NUM_NORMAL);
    if (!ht)
        LOG_STRM_ERROUT("#%d http is NULL\n", idx);

    httparg = (HttpTsArg *)argbuf;
    int_open_init(ht, httparg->url);

    ht->order_flg = 1;
    ht->ts_handle = http_ts_record_create(ht, NULL, arg, httparg);
    if (NULL == ht->ts_handle)
        LOG_STRM_ERROUT("#%d http_ts_record_create\n", idx);

    int_loop(ht);
    LOG_STRM_PRINTF("#%d exit local_loop!\n", idx);

    if (ht->ts_handle)
        ht->ts_op.ht_destroy(ht->ts_handle);

Err:
    httpts_delete(ht);
}
#endif


static int int_argparse_play(int idx, PlayArg* arg, char *argbuf, const char* url, int shiftlen, int begin, int end)
{
    HttpTsArg *httparg = (HttpTsArg *)argbuf;

    if (!url)
        LOG_STRM_ERROUT("#%d url is NULL\n", idx);
    if (strncasecmp(url, "http://", 7))
        LOG_STRM_ERROUT("#%d url\n", idx);

    IND_STRCPY(httparg->url, url);

    LOG_STRM_PRINTF("#%d magic = %u / %u, bitrate = %d, length = %d, shiftlen = %d\n", idx, arg->magic, g_httppvr.magic, begin, end, shiftlen);
    httparg->key = 0;
    httparg->start = shiftlen;

    httparg->bitrate = 0;
    if (begin > 0)
        httparg->bitrate = begin;

    httparg->length = 0;
    if (end > 0)
        httparg->length = end;

    if (arg->magic == g_httppvr.magic)
        httparg->key = g_httppvr.key;
    g_httppvr.magic = 0;

    return 0;
Err:
    return -1;
}

#ifdef INCLUDE_PVR
static int int_argparse_record(int idx, RecordArg* arg, char *argbuf, const char* url)
{
    HttpTsArg *httparg = (HttpTsArg *)argbuf;

    if (!url)
        LOG_STRM_ERROUT("#%d url is NULL\n", idx);
    if (strncasecmp(url, "http://", 7))
        LOG_STRM_ERROUT("#%d url\n", idx);

    IND_STRCPY(httparg->url, url);
    httparg->key = 0;
    httparg->bitrate = 0;

    LOG_STRM_PRINTF("#%d bitrate = %d\n", idx, g_bitrate);
    httparg->bitrate = g_bitrate;

    return 0;
Err:
    return -1;
}

static int int_urlcmp(char *argbuf, const char* url, APP_TYPE apptype)
{
    const char *p;
    int ret, len, l;
    HttpTsArg *httparg = (HttpTsArg *)argbuf;

    len = strlen(httparg->url);
    p = strchr(url, '?');
    if (p)
        l = p - url;
    else
        l = strlen(url);

    if (len < l)
        ret = -1;
    else if (len > l && '?' != httparg->url[l])
        ret = -1;
    else
        ret = strncmp(httparg->url, url, l);
    LOG_STRM_PRINTF("ret = %d\n", ret);

    return ret;
}
#endif

static int int_hls(HttpTs* ht, int livestate)
{
    HttpOp op;

    memset(&op, 0, sizeof(op));

    op.deal_error = int_error;

    switch(livestate) {
    case LIVE_STATE_FILE:
        LOG_STRM_PRINTF("#%d LIVE_STATE_FILE\n", ht->index);
        op.recv_begin = file_recv_begin;
        op.recv_sync = file_recv_sync;
        op.recv_end = file_recv_end;
        break;
    case LIVE_STATE_M3U8:
        LOG_STRM_PRINTF("#%d LIVE_STATE_M3U8\n", ht->index);
        op.recv_begin    = m3u8_recv_begin;
        op.recv_sync    = m3u8_recv_sync;
        op.recv_end    = m3u8_recv_end;
        break;
    case LIVE_STATE_SLICE:
        LOG_STRM_PRINTF("#%d LIVE_STATE_SLICE\n", ht->index);
        op.recv_begin    = slice_recv_begin;
        op.recv_sync    = file_recv_sync;
        op.recv_end    = slice_recv_end;
        break;
    default:
        LOG_STRM_ERROUT("#%d livestate = %d\n", ht->index, livestate);
    }
    strm_http_set_opset(ht->http, &op, ht);

    ht->hls_state = livestate;

    return 0;
Err:
    return -1;
}

void http_ts_reset(HttpTs* ht)
{
    ht->end_flg = 0;

    ht->sb->len = 0;
}

int http_ts_slice_duration(HttpTs* ht)
{
    HLiveMedia *media = &ht->hls->media;
    HLiveSlice *slice;

    slice = http_live_slice(media, media->x_sequence + ht->hls_index);
    if (slice)
        return slice->duration;
    else
        return media->x_duration;
}

int http_create_stream(StreamCtrl *ctrl)
{
    ctrl->handle = ctrl;

    ctrl->loop_play = int_loop_play;
#ifdef INCLUDE_PVR
    ctrl->loop_record = int_loop_record;
    ctrl->urlcmp = int_urlcmp;
#endif

    ctrl->argsize = sizeof(HttpTsArg);
    ctrl->argparse_play = int_argparse_play;
#ifdef INCLUDE_PVR
    ctrl->argparse_record = int_argparse_record;
#endif

    return 0;
}

void mid_stream_http_pvrkey(int idx, long long key)
{
    LOG_STRM_PRINTF("#%d key = %lld\n", idx, key);
    if (idx == 0) {
        g_httppvr.magic = int_stream_nextmagic( );
        g_httppvr.key = key;
    }
}

void mid_record_bitrate(int idx, int bitrate)
{
    LOG_STRM_PRINTF("#%d bitrate = %d\n", idx, bitrate);
    g_bitrate = bitrate;
}
