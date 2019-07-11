/**************************************--**************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **************************************--**************************************/

#include "http.h"
#include "http_live.h"

typedef struct tagHParseBuffer HParseBuffer;
typedef struct tagHParseBuffer* HParseBuffer_t;
struct tagHParseBuffer {
    int         m3u8_off;
    int         m3u8_len;
    char*       m3u8_buf;
};

typedef struct tagHParseSlice HParseSlice;
typedef struct tagHParseSlice* HParseSlice_t;
struct tagHParseSlice {
    uint32_t    slice_datetime;
    int         slice_duration;
};

typedef struct tagHParseStream HParseStream;
typedef struct tagHParseStream* HParseStream_t;
struct tagHParseStream {
    int         stream_bandwidth;
};

HttpLive* http_live_create(void)
{
    HttpLive *hls = NULL;

    hls = (HttpLive*)IND_CALLOC(sizeof(HttpLive), 1);
    if(NULL == hls)
        LOG_STRM_ERROUT("malloc HttpLive\n");

    return hls;
Err:
    return NULL;
}

static void hls_reset_stream(HttpLive* hls)
{
    HLiveStream *stream, *next;

    stream = hls->stream_queue;
    while(stream) {
        next = stream->next;
        IND_FREE(stream);
        stream = next;
    }
    hls->stream_queue = NULL;
}

void http_live_reset_slice(HLiveMedia* media)
{
    int i;
    HLiveSlice *slice, **slice_array, **next_array;

    slice_array = media->slice_array;
    while (slice_array) {
        next_array = (HLiveSlice**)slice_array[HLS_ARRAY_SIZE];
        for (i = 0; i < HLS_ARRAY_SIZE; i++) {
            slice = slice_array[i];
            if (NULL == slice)
                break;
            IND_FREE(slice);
        }
        IND_FREE(slice_array);
        if (NULL == slice)
            break;
        slice_array = next_array;
    }
    media->slice_num = 0;
    media->slice_array = NULL;

    media->slice_complete = 0;
    media->total_duration = 0;

    media->x_duration = 0;
    media->x_sequence = 0;

    if (media->key_iv) {
        IND_FREE(media->key_iv);
        media->key_iv = NULL;
    }
    if (media->key_uri) {
        IND_FREE(media->key_uri);
        media->key_uri = NULL;
    }
}

static void hls_reset_track(HLiveTrack* track)
{
    int i, num;
    HLiveElem **array;

    http_live_reset_slice(&track->media);
    http_live_reset_slice(&track->switch_media);

    num = track->num;
    array = track->array;
    for (i = 0; i < num; i++)
        IND_FREE(array[i]);
    track->num = 0;
}

void http_live_delete(HttpLive* hls)
{
    if (!hls)
        return;

    http_live_reset_slice(&hls->media);
    hls_reset_stream(hls);

    hls_reset_track(&hls->audio);
    hls_reset_track(&hls->subtitle);

    IND_FREE(hls);
}

static int hls_parse_line(HParseBuffer* m3u8)
{
    char *buf, *end;
    int l, len;

    //找行首
    buf = m3u8->m3u8_buf + m3u8->m3u8_off;
    len = m3u8->m3u8_len;
    while (len > 0 && (buf[0] == '\r' || buf[0] == '\n' || buf[0] == ' ')) {
        buf++;
        len--;
    }
    l = m3u8->m3u8_len - len;
    if (l > 0) {
        m3u8->m3u8_off += l;
        m3u8->m3u8_len = len;
    }
    len = m3u8->m3u8_len;
    if (len <= 0)
        return 0;

    //找行尾
    buf = m3u8->m3u8_buf + m3u8->m3u8_off;
    end = buf + len;
    while (buf < end && buf[0] != '\r' && buf[0] != '\n')
        buf++;

    len = (int)(buf - (m3u8->m3u8_buf + m3u8->m3u8_off));

    return len;
}

typedef struct tagAttribute Attribute;
struct tagAttribute {
    char*   name_buf;
    int     name_len;
    char*   value_buf;
    int     value_len;
    int     value_type;
};

static int hls_parse_attribute(char* buf, int len, Attribute* attr)
{
    int i;

    i = 0;
    while (i < len && ' ' == buf[i])
        i++;
    if (i >= len)
        return -1;
    attr->name_buf = buf + i;

    while (i < len && buf[i] != '=')
        i++;
    if (i + 2 >= len)
        return -1;
    attr->name_len = buf + i - attr->name_buf;

    i++;
    if ('"' == buf[i]) {
        i++;
        attr->value_type = 1;
    } else {
        attr->value_type = 0;
    }
    attr->value_buf = buf + i;

    if (1 == attr->value_type) {
        while (i < len && buf[i] != '"')
            i++;
        if (i >= len)
            return -1;
        attr->value_len = buf + i - attr->value_buf;
    }

    while (i < len && buf[i] != ',')
        i++;
    if (0 == attr->value_type)
        attr->value_len = buf + i - attr->value_buf;

    if (i >= len)
        return i;
    return (i + 1);
}

static void hls_parse_key(HLiveMedia* media, char* buf, int len)
{
    Attribute attr;

    int l, method;
    uint8_t *iv;
    char  *uri;

    if (media->key_iv) {
        IND_FREE(media->key_iv);
        media->key_iv = NULL;
    }
    if (media->key_uri) {
        IND_FREE(media->key_uri);
        media->key_uri = NULL;
    }

    method = 0;
    iv = NULL;
    uri = NULL;

    while (len > 0) {
        l = hls_parse_attribute(buf, len, &attr);
        if (l <= 0)
            break;
        buf += l;
        len -= l;
        if (6 == attr.name_len && 0 == memcmp(attr.name_buf, "METHOD", 6)) {
            if (7 == attr.value_len && 0 == memcmp(attr.value_buf, "AES-128", 7))
                method = 1;
        } else if (3 == attr.name_len && 0 == memcmp(attr.name_buf, "URI", 3)) {
            if (uri)
                IND_FREE(uri);
            uri = (char*)IND_MALLOC(attr.value_len + 1);
            if (uri) {
                IND_MEMCPY(uri, attr.value_buf, attr.value_len);
                uri[attr.value_len] = '\0';
            }
        } else if (2 == attr.name_len && 0 == memcmp(attr.name_buf, "IV", 2)) {
            if (34 == attr.value_len) {
                uint8_t *ubuf = (uint8_t *)attr.value_buf;
                if (ubuf[0] == '0' && ubuf[1] == 'x') {
                    ubuf += 2;

                    if (iv)
                        IND_FREE(iv);
                    iv = (uint8_t*)IND_MALLOC(16);
                    if (iv) {
                        int i;
                        uint8_t ch, v;

                        for (i = 0; i < 32; i++) {
                            ch = ubuf[i];
                            if (ch >= '0' && ch <= '9')
                                v = ch - '0';
                            else if (ch >= 'a' && ch <= 'f')
                                v = ch - 'a' + 10;
                            else if (ch >= 'A' && ch <= 'F')
                                v = ch - 'A' + 10;
                            else
                                break;
                            v <<= 4;
                            i++;

                            ch = ubuf[i];
                            if (ch >= '0' && ch <= '9')
                                v += ch - '0';
                            else if (ch >= 'a' && ch <= 'f')
                                v += ch - 'a' + 10;
                            else if (ch >= 'A' && ch <= 'F')
                                v += ch - 'A' + 10;
                            else
                                break;
                            iv[i / 2] = v;
                        }
                        if (i < 32) {
                            IND_FREE(iv);
                            iv = NULL;
                        }
                    }
                }
            }
        }
    }

    if (method) {
        if (uri) {
            media->key_iv = iv;
            media->key_uri = uri;
        } else {
            if (iv)
                IND_FREE(iv);
        }
    } else {
        if (iv)
            IND_FREE(iv);
        if (uri)
            IND_FREE(uri);
    }
}

static void hls_parse_media(HttpLive* hls, char* buf, int len)
{
    int l, type, group_index, isdefault, uri_len, lang_len;
    char *uri_buf, *lang_buf;

    Attribute attr;
    HLiveElem *elem;
    HLiveTrack *track;

    isdefault = 0;
    type = 0;
    group_index = TS_AUDIO_NUM;
    uri_len = 0;
    uri_buf = NULL;
    lang_len = 0;
    lang_buf = NULL;

    while (len > 0) {
        l = hls_parse_attribute(buf, len, &attr);
        if (l <= 0)
            break;
        buf += l;
        len -= l;
        if (4 == attr.name_len && 0 == memcmp(attr.name_buf, "TYPE", 4)) {
            if (5 == attr.value_len && 0 == memcmp(attr.value_buf, "AUDIO", 5))
                type = 1;
            else if (9 == attr.value_len && 0 == memcmp(attr.value_buf, "SUBTITLES", 9))
                type = 2;
        } else if (7 == attr.name_len && 0 == memcmp(attr.name_buf, "DEFAULT", 7)) {
            if (3 == attr.value_len && 0 == memcmp(attr.value_buf, "YES", 3))
                isdefault = 1;
        } else if (8 == attr.name_len && 0 == memcmp(attr.name_buf, "LANGUAGE", 8)) {
            lang_buf = attr.value_buf;
            lang_len = attr.value_len;
        } else if (3 == attr.name_len && 0 == memcmp(attr.name_buf, "URI", 3)) {
            uri_buf = attr.value_buf;
            uri_len = attr.value_len;
        }
    }
    if (!type || !uri_len)
        LOG_STRM_ERROUT("TYPE = %d, URI = %d\n", type, uri_len);

    if (1 == type)
        track = &hls->audio;
    else
        track = &hls->subtitle;
    if (track->num >= TS_AUDIO_NUM)
        LOG_STRM_ERROUT("num = %d\n", track->num);

    elem = (HLiveElem*)IND_MALLOC(sizeof(HLiveElem) + uri_len);
    if (!elem)
        LOG_STRM_ERROUT("malloc HLiveMedia\n");
    IND_MEMCPY(elem->uri, uri_buf, uri_len);
    elem->uri[uri_len] = 0;

    if (2 == lang_len || 3 == lang_len) {
        IND_MEMCPY(elem->language, lang_buf, lang_len);
        if (2 == lang_len)
            elem->language[2] = '\0';
    } else {
        elem->language[0] = '\0';
    }

    if (isdefault) {
        int i;
        for (i = track->num; i > 0; i--)
            track->array[i] = track->array[i - 1];
        track->array[0] = elem;
    } else {
        track->array[track->num] = elem;
    }
    track->num++;

Err:
    return;
}

static void hls_parse_stream_inf(HttpLive* hls, HParseStream* parse, char* buf, int len)
{
    int l, bandwidth;
    Attribute attr;

    bandwidth = 0;
    while (len > 0) {
        l = hls_parse_attribute(buf, len, &attr);
        if (l <= 0)
            break;
        buf += l;
        len -= l;
        if (9 == attr.name_len && 0 == memcmp(attr.name_buf, "BANDWIDTH", 9)) {
            if (bandwidth >= 0)
                bandwidth = atoi(attr.value_buf);
        } else if (6 == attr.name_len && 0 == memcmp(attr.name_buf, "CODECS", 6)) {
            if (attr.value_len < 4 || 0 != memcmp(attr.value_buf, "avc1", 4))
                bandwidth = -1;
        }
    }

    if (bandwidth > 0)
        parse->stream_bandwidth = bandwidth;
    else
        parse->stream_bandwidth = 0;
}

static void hls_parse_stream_uri(HttpLive* hls, HParseStream* parse, char* buf, int len)
{
    HLiveStream *stream, *next, *prev;

    prev = NULL;
    stream = hls->stream_queue;
    while (stream && stream->bandwidth < parse->stream_bandwidth) {
        prev = stream;
        stream = stream->next;
    }

    stream = (HLiveStream*)IND_MALLOC(sizeof(HLiveStream) + len);
    if (!stream)
        return;

    stream->bandwidth = parse->stream_bandwidth;

    while (len > 0 && buf[len - 1] == ' ')
        len--;
    IND_MEMCPY(stream->uri, buf, len);
    stream->uri[len] = 0;

    if (prev) {
        next = prev->next;
        prev->next = stream;
    } else {
        next = hls->stream_queue;
        hls->stream_queue = stream;
    }
    stream->prev = prev;
    if (next)
        next->prev = stream;
    stream->next = next;
}

void http_live_parse_stream(HttpLive* hls, char* m3u8_buf, int m3u8_len)
{
    char *buf;
    int len;
    HParseBuffer m3u8;
    HParseStream parse;

    m3u8.m3u8_off = 0;
    m3u8.m3u8_len = m3u8_len;
    m3u8.m3u8_buf = m3u8_buf;

    parse.stream_bandwidth = 0;

    hls_reset_stream(hls);

    while (1) {
        len = hls_parse_line(&m3u8);
        if (len <= 0)
            return;
        buf = m3u8.m3u8_buf + m3u8.m3u8_off;
        m3u8.m3u8_off += len;
        m3u8.m3u8_len -= len;

        if (buf[0] == '#') {
            buf += 1;
            len -= 1;

            parse.stream_bandwidth = 0;
            if (memcmp(buf, "EXT-X-", 6) == 0) {
                buf += 6;
                len -= 6;
                if ('M' == buf[0] && memcmp(buf, "MEDIA:", 6) == 0)//#EXT-X-MEDIA:
                    hls_parse_media(hls, buf + 6, len - 6);
                else if ('S' == buf[0] && memcmp(buf, "STREAM-INF:", 11) == 0)//#EXT-X-STREAM-INF:
                    hls_parse_stream_inf(hls, &parse, buf + 11, len - 11);
            }
        } else {
            if (len >= STREAM_URL_SIZE - 64) {
                LOG_STRM_ERROR("stream url too large = %d\n", len);
            } else {
                if (parse.stream_bandwidth)
                    hls_parse_stream_uri(hls, &parse, buf, len);
            }
        }
    }
}

void http_live_parse_slice(HLiveMedia* media, char *m3u8_buf, int m3u8_len)
{
    char *buf;
    int line;
    HParseBuffer m3u8;
    HParseSlice parse;

    parse.slice_duration = 0;
    parse.slice_datetime = 0;
    m3u8.m3u8_off = 0;
    m3u8.m3u8_len = m3u8_len;
    m3u8.m3u8_buf = m3u8_buf;

    http_live_reset_slice(media);

    while (1) {
        line = hls_parse_line(&m3u8);
        if (line <= 0)
            return;

        buf = m3u8.m3u8_buf + m3u8.m3u8_off;
        m3u8.m3u8_off += line;
        m3u8.m3u8_len -= line;

        if (buf[0] == '#') {
            if ('I' == buf[4] && memcmp(buf, "#EXTINF:", 8) == 0) {
                parse.slice_duration = ind_atoui(buf + 8);

                //C28LUS0415初步0418确认.xls 规定 逻辑分片时长范围为5s-50s
                if (0 == media->x_duration) {
                    media->x_duration = parse.slice_duration;
                    LOG_STRM_PRINTF("x_duration = %d from duration\n", media->x_duration);
                }
            } else if ('P' == buf[7] && memcmp(buf, "#EXT-X-PROGRAM-DATE-TIME:", 25) == 0) {
                    struct ind_time tp;

                    memset(&tp, 0, sizeof(struct ind_time));
                    if (sscanf(buf + 25, "%04d-%02d-%02dT%02d:%02d:%02d", &tp.year, &tp.mon, &tp.day, &tp.hour, &tp.min, &tp.sec) == 6)
                        parse.slice_datetime = ind_time_make(&tp);
                /*{
                    ind_time_local(parse_slice->datetime, &tp);
                    LOG_STRM_PRINTF("EXT-X-PROGRAM-DATE-TIME = %08x %04d-%02d-%02dT%02d:%02d:%02d\n", 
                        parse_slice->datetime, tp.year, tp.mon, tp.day, tp.hour, tp.min, tp.sec);
                }*/
            } else if (memcmp(buf, "#EXT-X-ENDLIST", 14) == 0) {
                media->slice_complete = 1;
                LOG_STRM_PRINTF("EXT-X-ENDLIST\n");
            } else if ('T' == buf[7] && memcmp(buf, "#EXT-X-TARGETDURATION:", 22) == 0) {
                media->x_duration = ind_atoui(buf + 22);
                if (media->x_duration <= 0)
                    LOG_STRM_ERROR("EXT-X-TARGETDURATION = %d\n", media->x_duration);
                else
                    LOG_STRM_PRINTF("EXT-X-TARGETDURATION = %d\n", media->x_duration);
            } else if ('M' == buf[7] && memcmp(buf, "#EXT-X-MEDIA-SEQUENCE:", 22) == 0) {
                media->x_sequence = ind_atoui(buf + 22);
                LOG_STRM_PRINTF("EXT-X-MEDIA-SEQUENCE = %d\n", media->x_sequence);
            } else if ('K' == buf[7] && memcmp(buf, "#EXT-X-KEY:", 11) == 0) {
                buf += 11;
                line -= 11;
                hls_parse_key(media, buf, line);
            }
        } else {
            if (line >= STREAM_URL_SIZE - 64) {
                LOG_STRM_ERROR("slice url too large = %d\n", line);
            } else {
                HLiveSlice *slice;

                if (0 == parse.slice_duration)
                    LOG_STRM_WARN("slice duration = %d\n", parse.slice_duration);
    
                slice = (HLiveSlice*)IND_MALLOC(sizeof(HLiveSlice) + line);
                if (NULL == slice) {
                    LOG_STRM_ERROR("malloc HttpSlice\n");
                } else {
                    int len;
                    int slice_num;
                    HLiveSlice **slice_array, **next_array;
    
                    slice_num = media->slice_num;
                    slice_array = media->slice_array;
                    slice->datetime = parse.slice_datetime;
                    slice->duration = parse.slice_duration;
    
                    media->total_duration += slice->duration;
    
                    len = line;
                    while (len > 0 && buf[len - 1] == ' ')
                        len--;
                    IND_MEMCPY(slice->uri, buf, len);
                    slice->uri[len] = 0;
    
                    while (slice_num >= HLS_ARRAY_SIZE) {
                        slice_num -= HLS_ARRAY_SIZE;
                        if (slice_num <= 0)
                            break;
                        slice_array = (HLiveSlice**)slice_array[HLS_ARRAY_SIZE];
                    }
                    if (slice_num > 0) {
                        slice_array[slice_num] = slice;
                        media->slice_num++;
                    } else {
                        next_array = (HLiveSlice**)IND_CALLOC(sizeof(HLiveSlice*) * (HLS_ARRAY_SIZE + 1), 1);
                        LOG_STRM_PRINTF("slice_num = %d, next_array = %p\n", media->slice_num, next_array);
                        if (!next_array) {
                            LOG_STRM_ERROR("malloc slice_array\n");
                            IND_FREE(slice);
                        } else {
                            if (media->slice_num == 0)
                                media->slice_array = next_array;
                            else
                                slice_array[HLS_ARRAY_SIZE] = (HLiveSlice*)next_array;
                            next_array[0] = slice;
                            media->slice_num++;
                        }
                    }
                }
            }
            parse.slice_duration = 0;
            parse.slice_datetime = 0;
        }
    }
}

int http_live_vodIndex(HLiveMedia* media, uint32_t *psec)
{
    HLiveSlice *slice, **slice_array;
    uint32_t src, dst;
    int i, idx, slice_num = media->slice_num;

    src = *psec;
    if (src > media->total_duration)
        src = media->total_duration;

    dst = 0;
    idx = 0;
    slice_array = media->slice_array;
    for (i = 0; idx < slice_num; i++, idx++) {
        if (i >= HLS_ARRAY_SIZE) {
            slice_array = (HLiveSlice**)slice_array[HLS_ARRAY_SIZE];
            i = 0;
        }
        slice = slice_array[i];
        if (idx >= slice_num - 1)
            break;
        if (dst + slice->duration > src)
            break;
        dst += slice->duration;
    }

    *psec = dst;
    return idx;
}

int http_live_iptvIndex(HLiveMedia* media, uint32_t *pdatetime)
{
    HLiveSlice *slice, **slice_array;
    uint32_t datetime;
    int i, idx, slice_num = media->slice_num;

    datetime = *pdatetime;

    idx = 1;
    slice_array = media->slice_array;
    for (i = 1; idx < slice_num; i++, idx++) {
        if (i >= HLS_ARRAY_SIZE) {
            slice_array = (HLiveSlice**)slice_array[HLS_ARRAY_SIZE];
            i = 0;
        }
        slice = slice_array[i];
        if (datetime >= slice->datetime && datetime < slice->datetime + slice->duration) {
            *pdatetime = slice->datetime;
            return idx;
        } else if (idx >= slice_num - 1 || datetime < slice->datetime) {
            LOG_STRM_PRINTF("idx = %d / %d, datetime = %d / %d\n", idx, slice_num, datetime, slice->datetime);
            *pdatetime = slice->datetime;
            return idx;
        }
    }

    return -1;
}

HLiveSlice* http_live_slice(HLiveMedia* media, int sequence)
{
    int idx, slice_num;
    HLiveSlice *slice, **slice_array;

    slice_num = media->slice_num;
    idx = sequence - media->x_sequence;
    if (idx < 0 || idx >= slice_num)
        goto Err;
        //LOG_STRM_ERROUT("index = %d, slice_num = %d\n", idx, slice_num);

    slice_array = media->slice_array;

    while (idx >= HLS_ARRAY_SIZE) {
        idx -= HLS_ARRAY_SIZE;
        slice_array = (HLiveSlice**)slice_array[HLS_ARRAY_SIZE];
    }
    slice = slice_array[idx];

    return slice;
Err:
    return NULL;
}

HLiveStream* http_live_stream(HttpLive* hls, int bandwidth)
{
    HLiveStream *stream = hls->stream_queue;
    while (stream && stream->bandwidth != bandwidth)
        stream = stream->next;
    return stream;
}
