/**************************************--**************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2012-3-20 23:56:17 by liujianhua

 **************************************--**************************************/

#include "http.h"
#include "http_ts.h"

#define FAST_BUFFER_SIZE        (2 * 1024 * 1316)

struct tagTsPcrTime {
    uint32_t        ifrm_clk;

    int         clk_diff;
    uint32_t        clk_last;
    uint32_t        clk_base;
    uint32_t        clk_time;
};
typedef struct tagTsPcrTime     TsPcrTime;

struct tagHttpTsPlay {
    HttpTs*         ht;
    HTTP_t          http;

    int             index;

    int             state;
    int             scale;

    int             diff;
    int             buffer;

    int             chunked;
    int             webradio;//服务器发送数据有间断

    int             end_flg;
    long long       strm_key;
    int             recv_times;

    StreamMsgQ*     strm_msgq;
    StreamPlay*     strm_play;
    StreamRecord*   strm_record;

    int             byte_rate;

    int             time_base;
    int             time_length;
    int             time_current;

    int             fast_showing;
    int             fast_receiving;

    int             fast_time_0;
    int             fast_time_1;
    ts_buf_t        fast_buf_0;
    ts_buf_t        fast_buf_1;

    StrmBuffer*     fast_sb;

    char*           fast_array[6];
    int             fast_index;
    uint32_t            fast_clk;

    ts_parse_t      ts_parse;
    struct ts_psi   ts_psi;

    int             delay_flg;
    uint32_t            delay_clk;
    int             delay_cmd;
    int             delay_arg;
};

typedef struct tagHttpTsPlay    HttpTsPlay;

static void int_fast_receive(HttpTsPlay* htp);
static int int_seek_byte(HttpTsPlay* htp, int seek);

static void int_clk_pcr(TsPcrTime* tpt, uint32_t pcr)
{
    //LOG_STRM_PRINTF("PTS: %d / %d\n", pts,  pts / 1000);
    if (tpt->clk_diff == -1) {
        LOG_STRM_PRINTF("base: %d\n", pcr);
        tpt->clk_base = pcr;
        tpt->clk_diff = 0;
    } else {
        if (pcr == tpt->clk_last)
            return;

        if (pcr < tpt->clk_last || pcr > tpt->clk_last + 200) {
            LOG_STRM_PRINTF("WARN! %u > %u\n",  tpt->clk_last, pcr);

            tpt->clk_time += tpt->clk_last - tpt->clk_base;
            tpt->clk_time += tpt->clk_diff;

            tpt->clk_base = pcr;
        } else {
            tpt->clk_diff = pcr - tpt->clk_last;
        }
    }
    tpt->clk_last = pcr;
}

static uint32_t int_clk_time(TsPcrTime* tpt)
{
    return (tpt->clk_time + (tpt->clk_last - tpt->clk_base));
}

static void int_clk_reset(TsPcrTime* tpt)
{
    tpt->clk_diff = -1;
}

static void int_state(HttpTsPlay* htp , STRM_STATE state, int scale)
{
    htp->state = state;
    htp->scale = scale;
    stream_post_state(htp->index, state, scale);
}

static void int_msgback(void* handle, STRM_MSG msgno, int arg)
{
    StreamMsgQ* strm_msgq = (StreamMsgQ*)handle;
    strm_msgq_queue(strm_msgq, msgno, arg);
}

static int int_recv_space(void* handle)
{
    int space;
    HttpTsPlay *htp = (HttpTsPlay*)handle;

    if (STRM_STATE_FAST == htp->state)
        return 0;

    space = strm_play_space(htp->strm_play);

    return space;
}

static int int_recv_push(void* handle)
{
    int l;
    HttpTs *ht;
    StrmBuffer* sb;
    HttpTsPlay *htp = (HttpTsPlay*)handle;

    ht = htp->ht;
    sb = ht->sb;

    l = sb->len;
    if (l <= 0)
        return 0;

#ifdef INCLUDE_PVR
    if (htp->strm_key != 0)
        record_port_strm_decrypt(sb->buf, sb->len);
#endif
    strm_play_push(htp->strm_play, htp->index, &ht->sb);

    ht->sb->len = 0;

    return l;
}

static int file_recv_begin(void* handle)
{
    long long content_length;
    HttpTsPlay *htp = (HttpTsPlay*)handle;

    if (STRM_STATE_FAST == htp->state) {
        ts_buf_reset(htp->fast_buf_1);
        strm_http_buf_replace(htp->http, htp->fast_buf_1);
    } else {
        strm_http_buf_replace(htp->http, NULL);

        content_length = strm_http_get_contentLength(htp->ht->http);
        if (0LL == content_length)
            htp->chunked = 1;

        if (htp->time_length > 0) {
            htp->byte_rate = (int)(content_length / htp->time_length);
            LOG_STRM_PRINTF("#%d byte_rate = %d\n", htp->index, htp->byte_rate);
        }
    }

    return 0;
}

static void file_recv_end(void* handle, int end)
{
    HttpTsPlay *htp = (HttpTsPlay*)handle;

    LOG_STRM_PRINTF("#%d end = %d, state = %d\n", htp->index, end, htp->state);
    if (1 == end && STRM_STATE_FAST == htp->state) {
        htp->fast_receiving = 0;
        if (0 == htp->fast_showing) {
            htp->fast_clk = strm_httploop_clk(htp->ht->loop);
            int_fast_receive(htp);
        }
    } else if (2 == end && STRM_STATE_PLAY == htp->state) {
        htp->end_flg = 1;
        strm_play_end(htp->strm_play, htp->index);
    }
}

static void slice_recv_begin(void* handle)
{
}

static void slice_recv_end(void* handle)
{
}

static int int_pause(HttpTsPlay* htp)
{
    LOG_STRM_PRINTF("#%d state = %d\n", htp->index, htp->state);

    switch (htp->state) {
    case STRM_STATE_PLAY:
        break;
    case STRM_STATE_FAST:
        int_seek_byte(htp, htp->time_current);
        htp->time_base = htp->time_current;
        strm_play_reset(htp->strm_play, htp->index, 1);
        break;
    default:
        LOG_STRM_ERROUT("#%d state = %d\n", htp->index, htp->state);
    }

    strm_play_pause(htp->strm_play, htp->index);

    int_state(htp, STRM_STATE_PAUSE, 0);

    return 0;
Err:
    return -1;
}

static int int_resume(HttpTsPlay* htp)
{
    LOG_STRM_PRINTF("#%d state = %d\n", htp->index, htp->state);

    switch (htp->state) {
    case STRM_STATE_PAUSE:
        break;
    case STRM_STATE_FAST:
        int_seek_byte(htp, htp->time_current);
        htp->time_base = htp->time_current;
        strm_play_reset(htp->strm_play, htp->index, 1);
        break;
    default:
        LOG_STRM_ERROUT("#%d state = %d\n", htp->index, htp->state);
    }

    strm_play_resume(htp->strm_play, htp->index, 0);

    int_state(htp, STRM_STATE_PLAY, 1);

    return 0;
Err:
    return -1;
}

static int int_seek_byte(HttpTsPlay* htp, int seek)
{
    int length;
    long long seek_byte, content_length;

    length = htp->time_length;
    content_length = strm_http_get_contentLength(htp->http);

    if (content_length <= 0 || length <= 0)
        LOG_STRM_ERROUT("#%d content_length = %lld, time_length = %d\n", htp->index, content_length, length);
    if (seek < 0)
        seek = 0;
    else if (seek >= length)
        seek = length - 1;

    seek_byte = content_length * seek / length;
    seek_byte = seek_byte - seek_byte % 188;
    LOG_STRM_PRINTF("#%d content_length = %lld, time_length = %d, seek = %d\n", htp->index, content_length, length, seek);
    strm_http_reset(htp->http);
    if (strm_http_request(htp->http, htp->ht->url, seek_byte, 0))
        LOG_STRM_ERROUT("#%d local_connect\n", htp->index);

    htp->end_flg = 0;
    http_ts_reset(htp->ht);

    return 0;
Err:
    return -1;
}

static int int_seek(HttpTsPlay* htp, int seek)
{
    int length = htp->time_length;

    LOG_STRM_PRINTF("#%d state = %d, seek = %d\n", htp->index, htp->state, seek);

    if (length <= 0)
        LOG_STRM_ERROUT("#%d length = %d\n", htp->index, length);

    if (seek < 0)
        seek = 0;
    else if (seek >= length)
        seek = length - 1;

    if (int_seek_byte(htp, seek))
        LOG_STRM_ERROUT("#%d int_seek_byte\n", htp->index);

    http_ts_reset(htp->ht);

    htp->time_base = seek;
    htp->time_current = seek;
    stream_back_currenttime(htp->index, htp->time_current);

    strm_play_reset(htp->strm_play, htp->index, 1);
    strm_play_resume(htp->strm_play, htp->index, 0);

    int_state(htp, STRM_STATE_PLAY, 1);

    return 0;
Err:
    return -1;
}

static void int_fast_parse(HttpTsPlay* htp)
{
    int i, off, len;
    uint8_t *p;
    char *buf;
    uint32_t pid, clk, mark;
    ts_psi_t psi;
    TsPcrTime tpt;

    for (i = 0; i < 6; i ++)
        htp->fast_array[i] = NULL;
    htp->fast_index = 0;

    len = 0;
    ts_buf_read_get(htp->fast_buf_0, &buf, &len);
    len = len - len % 188;
    if (len <= 0)
        LOG_STRM_ERROUT("ts_buf_read_get\n");

#ifdef INCLUDE_PVR
    if (htp->strm_key != 0)
        record_port_strm_decrypt(buf, len);
#endif

    memset(&htp->ts_psi, 0, sizeof(htp->ts_psi));
    ts_parse_reset(htp->ts_parse);
    for (off = 0; off + 1316 <= len; off += 1316) {
        if (1 == ts_parse_psi(htp->ts_parse, (u_char *)(buf + off), 1316, NULL))
            break;
    }
    if (off + 1316 > len)
        LOG_STRM_ERROUT("#%d ts_parse_psi\n", htp->index);

    psi = &htp->ts_psi;
    if (0 == psi->video_pid) {
        strm_play_set_idle(htp->strm_play, htp->index, 1);
        goto Err;
    }

    memset(&tpt, 0, sizeof(tpt));
    int_clk_reset(&tpt);

    mark = 0;
    for (i = 0, off = 0; off < len && i < 6; off += 188) {
        p = (uint8_t *)buf + off;
        if (p[0] != 0x47)
            LOG_STRM_ERROUT("ts sync byte!\n");

        pid =(((uint32_t)p[1] & 0x1f) << 8) + p[2];

        if (pid == psi->pcr_pid) {
            uint32_t pcr = ts_pcr_parse188(p);
            if (pcr)
                int_clk_pcr(&tpt, pcr / 450);
        }

        if ((p[1] & 0x40) == 0) //start indicator
            continue;

        if (pid != psi->video_pid)
            continue;
        htp->fast_array[i] = buf + off;

        if (0 == i) {
            i ++;
            continue;
        }

        clk = int_clk_time(&tpt);

        if (clk >= mark + 40) {
            htp->fast_array[i] = buf + off;
            i ++;
            mark = clk;
        }
    }

Err:
    return;
}

static void int_fast_show(HttpTsPlay* htp)
{
    int i, l, len;
    char *buf, *end;
    StrmBuffer* sb;

    htp->fast_clk += 20;

    i = htp->fast_index;
    htp->fast_index ++;

    if (i >= 5)
        return;

    htp->fast_showing = 1;

    buf = htp->fast_array[i];
    i ++;
    end = htp->fast_array[i];
    if (!buf || !end)
        return;
    len = (int)(end - buf);

    while (len > 0) {
        sb = htp->fast_sb;
        l = sb->size;
        if (l > len)
            l = len;
        IND_MEMCPY(sb->buf, buf, l);
        buf += l;
        len -= l;

        sb->len = l;
        sb->off = 0;
        strm_play_push(htp->strm_play, htp->index, &htp->fast_sb);
    }
}

static void int_fast_receive(HttpTsPlay* htp)
{
    HttpTs *ht;
    int fast_time_0;
    long long bytes, start_byte, content_length;

    ht = htp->ht;

    fast_time_0 = htp->fast_time_0;
    if (-1 == fast_time_0) {
        htp->fast_time_0 = htp->time_current;
    } else {
        ts_buf_t ts_buf;

        htp->fast_time_0 = htp->fast_time_1;

        ts_buf = htp->fast_buf_0;
        htp->fast_buf_0 = htp->fast_buf_1;
        htp->fast_buf_1 = ts_buf;

        int_fast_parse(htp);
    }

    if (fast_time_0 >= 0) {
        htp->fast_clk = strm_httploop_clk(ht->loop);
        int_fast_show(htp);
    }

    if ((htp->scale < 0 && htp->fast_time_0 <= 0) || (htp->scale > 0 && htp->fast_time_0 >= htp->time_length))
        return;

    content_length = strm_http_get_contentLength(ht->http);
    htp->fast_time_1 = htp->fast_time_0 + htp->scale;

    if (htp->scale < 0) {
        if (htp->fast_time_1 < 0)
            htp->fast_time_1 = 0;
        start_byte = content_length * htp->fast_time_1 /  htp->time_length;
    } else {
        if (htp->fast_time_1 > htp->time_length)
            htp->fast_time_1 = htp->time_length;
        start_byte = content_length * (htp->fast_time_1 - 2) /  htp->time_length;
    }
    start_byte = start_byte - start_byte % 188;

    bytes = (long long)htp->byte_rate * 2;
    if (bytes > FAST_BUFFER_SIZE)
        bytes = FAST_BUFFER_SIZE;
    if (content_length < start_byte + bytes)
        bytes = content_length - start_byte;

    strm_http_reset(htp->http);
    if (-1 == fast_time_0)
        strm_http_request(htp->http, NULL, start_byte, bytes);
    else
        strm_http_request(htp->http, NULL, start_byte, bytes);

    htp->end_flg = 0;
    htp->fast_receiving = 1;
}

static int int_fast(HttpTsPlay* htp, int scale)
{
    LOG_STRM_PRINTF("#%d state = %d, scale = %d\n", htp->index, htp->state, scale);

    if (!htp->fast_buf_0)
        htp->fast_buf_0  = ts_buf_create(FAST_BUFFER_SIZE);
    if (!htp->fast_buf_1)
        htp->fast_buf_1  = ts_buf_create(FAST_BUFFER_SIZE);
    if (!htp->fast_sb)
        htp->fast_sb = strm_buf_malloc(TS_TIMESHIFT_SIZE);
    if (!htp->ts_parse)
        htp->ts_parse = ts_parse_create(&htp->ts_psi);

    htp->fast_showing = 0;
    htp->fast_receiving = 0;

    htp->fast_time_0 = -1;
    htp->fast_clk = strm_httploop_clk(htp->ht->loop);

    strm_play_reset(htp->strm_play, htp->index, 1);
    strm_play_tplay(htp->strm_play, htp->index, scale);

    int_state(htp, STRM_STATE_FAST, scale);

    int_fast_receive(htp);

    return 0;
}

static int int_stop(HttpTsPlay* htp)
{
    if (int_seek(htp, htp->time_length - 3))
        LOG_STRM_ERROUT("#%d int_seek\n", htp->index);

    return 0;
Err:
    return -1;
}

static void int_trickmode(HttpTsPlay* htp, int cmd, int arg)
{
    if (STREAM_CMD_SEEK == cmd || STREAM_CMD_STOP == cmd) {
        if (htp->delay_flg)
            htp->delay_flg = 0;

        switch(cmd) {
        case STREAM_CMD_SEEK:
            LOG_STRM_PRINTF("#%d STREAM_CMD_SEEK\n", htp->index);
            int_seek(htp, (uint32_t)arg);
            break;
        case STREAM_CMD_STOP:
            LOG_STRM_PRINTF("#%d STREAM_CMD_STOP\n", htp->index);
            int_stop(htp);
            break;
        default:
            break;
        }
    } else {
        uint32_t clk = strm_httploop_clk(htp->ht->loop);

        if (htp->delay_flg == 0 || htp->delay_flg == 3) {
            switch(cmd) {
            case STREAM_CMD_RESUME:
                LOG_STRM_PRINTF("#%d STREAM_CMD_RESUME\n", htp->index);
                int_resume(htp);
                break;
        
            case STREAM_CMD_PAUSE:
                LOG_STRM_PRINTF("#%d STREAM_CMD_PAUSE\n", htp->index);
                int_pause(htp);
                break;
        
            case STREAM_CMD_FAST:
                LOG_STRM_PRINTF("#%d STREAM_CMD_FAST\n", htp->index);
                int_fast(htp, arg);
                break;

            default:
                break;
            }

            htp->delay_flg = 1;
            htp->delay_clk = clk + INTERVAL_CLK_1000MS;
        } else {
            switch(cmd) {
            case STREAM_CMD_RESUME:
                LOG_STRM_PRINTF("#%d STREAM_CMD_RESUME_\n", htp->index);
                stream_post_state(htp->index, STRM_STATE_PLAY, 1);
                break;

            case STREAM_CMD_PAUSE:
                LOG_STRM_PRINTF("#%d STREAM_CMD_PAUSE_\n", htp->index);
                stream_post_state(htp->index, STRM_STATE_PAUSE, 0);
                break;
        
            case STREAM_CMD_FAST:
                LOG_STRM_PRINTF("#%d STREAM_CMD_FAST_\n", htp->index);
                stream_post_state(htp->index, STRM_STATE_FAST, arg);
                break;

            default:
                break;
            }

            htp->delay_flg = 2;
            htp->delay_cmd = cmd;
            htp->delay_arg = arg;
            htp->delay_clk = clk + INTERVAL_CLK_1000MS;
        }
    }
}

static int int_error_play(HttpTsPlay* htp, int msgno)
{
    strm_httploop_break(htp->ht->loop);
    return 0;
}

static void int_deal_error(void* handle, STRM_MSG msgno)
{
    HttpTsPlay *htp = (HttpTsPlay*)handle;

    if (htp->state) {
        int_error_play(htp, 0);
        stream_post_msg(htp->index, STRM_MSG_OPEN_ERROR, msgno);
    }
}


static void int_deal_cmd(void* handle, StreamCmd* strmCmd)
{
    HttpTsPlay *htp = (HttpTsPlay*)handle;

    int cmd = strmCmd->cmd;

    switch(cmd) {
    case STREAM_CMD_RESUME:
    case STREAM_CMD_PAUSE:
    case STREAM_CMD_FAST:
    case STREAM_CMD_SEEK:
    case STREAM_CMD_STOP:
        {
            int cmdsn = strmCmd->arg3;
            int_trickmode(htp, cmd, strmCmd->arg0);
            if (cmdsn)
                stream_back_cmd(htp->index, cmdsn);
        }
        break;
        
    case STREAM_CMD_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE\n", htp->index);
        strm_httploop_break(htp->ht->loop);
        break;

    default:
        LOG_STRM_ERROUT("#%d Unkown CMD %d\n", htp->index, cmd);
    }

Err:
    return;
}

static void int_deal_msg(void* handle, STRM_MSG msgno, int arg)
{
    HttpTsPlay *htp = (HttpTsPlay*)handle;

    LOG_STRM_PRINTF("#%d msgno = %d\n", htp->index, msgno);

    switch(msgno) {
    case STRM_MSG_PTS_VIEW:
        LOG_STRM_PRINTF("#%d STRM_MSG_PTS_VIEW\n", htp->index);
        stream_post_msg(htp->index, msgno, 0);
        break;
    case STRM_MSG_PSI_VIEW:
        LOG_STRM_PRINTF("#%d STRM_MSG_PSI_VIEW\n", htp->index);
        break;
   case  STRM_MSG_RECV_TIMEOUT:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_TIMEOUT\n", htp->index);
        stream_post_msg(htp->index, msgno, 0);
        break;
    case STRM_MSG_RECV_RESUME:
        LOG_STRM_PRINTF("#%d STRM_MSG_RECV_RESUME\n", htp->index);
        stream_post_msg(htp->index, msgno, 0);
        break;
    case STRM_MSG_STREAM_MUSIC:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_MUSIC chunked = %d\n", htp->index, htp->chunked);
        if (htp->chunked) {
            LOG_STRM_PRINTF("#%d: temp pause\n", htp->index);
            strm_play_pause(htp->strm_play, htp->index);
            htp->webradio = 2;
        }
        break;
    case STRM_MSG_STREAM_VIDEO:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_VIDEO\n", htp->index);
        break;
    case STRM_MSG_UNSUPPORT_MEDIA:
    case STRM_MSG_PLAY_ERROR:
        LOG_STRM_ERROR("#%d STRM_MSG_PLAY_ERROR\n", htp->index);
        int_error_play(htp, 0);
        break;
    case STRM_MSG_STREAM_END:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", htp->index);
        stream_post_msg(htp->index, STRM_MSG_STREAM_END, 0);
        break;

    default:
        LOG_STRM_ERROUT("#%d unknown play message %d\n", htp->index, msgno);
    }

Err:
    return;
}

//DLNA共享播放
static int int_open_play(HttpTsPlay* htp, PlayArg *arg, HttpTsArg *httparg)
{
    long long startbyte;
    LOG_STRM_PRINTF("#%d url = %s\n", htp->index, httparg->url);

    htp->strm_key = httparg->key;
    htp->time_length = httparg->length;
    LOG_STRM_PRINTF("#%d key = %lld time_start = %d, bitrate = %d, time_length = %d\n", htp->index, htp->strm_key, httparg->start, httparg->bitrate, htp->time_length);
    stream_back_totaltime(htp->index, htp->time_length);

    if (httparg->start > 0 && httparg->bitrate > 0 && httparg->length > 0) {
        htp->time_base = httparg->start;
        htp->time_current = httparg->start;
        startbyte = (long long)httparg->bitrate / 8 * httparg->start;
    } else {
        startbyte = 0;
    }
    startbyte = startbyte - startbyte % 188;
    if (strm_http_request(htp->http, httparg->url, startbyte, 0))
        LOG_STRM_ERROUT("#%d local_connect\n", htp->index);

    strm_play_open(htp->strm_play, htp->index, htp->strm_msgq, int_msgback, APP_TYPE_HTTP, TS_TIMESHIFT_SIZE);
#ifdef INCLUDE_PVR
    if (htp->strm_key != 0) {
        LOG_STRM_PRINTF("#%d key = %lld\n", htp->index, htp->strm_key);
        record_port_strm_setkey(htp->strm_key);
    }
#endif

    int_state(htp, STRM_STATE_PLAY, 1);

    return 0;
Err:
    return -1;
}

static void int_close_play(HttpTsPlay* htp)
{
    LOG_STRM_PRINTF("#%d state = %d\n", htp->index, htp->state);
    if (htp->state == 0)
        return;

    strm_play_close(htp->strm_play, htp->index, 1);

#ifdef INCLUDE_PVR
    if (htp->strm_key != 0) {
        LOG_STRM_PRINTF("#%d key clr\n", htp->index);
        record_port_strm_setkey(0);
    }
    htp->state = 0;
#endif

    strm_httploop_break(htp->ht->loop);
}

static void int_100ms(void* handle)
{
    uint32_t clk;
    HttpTsPlay *htp = (HttpTsPlay*)handle;

    clk = strm_httploop_clk(htp->ht->loop);
    if (STRM_STATE_FAST == htp->state && 0 == htp->end_flg && htp->fast_clk < clk) {
        if (htp->fast_index < 5) {
            int_fast_show(htp);
        } else {
            htp->fast_showing = 0;

            htp->time_current = htp->fast_time_0;
            stream_back_currenttime(htp->index, htp->time_current);

            if (htp->scale < 0) {
                if (htp->time_current <= 0) {
                    LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_BEGIN\n", htp->index);
                    stream_post_msg(htp->index, STRM_MSG_STREAM_BEGIN, 0);
                    int_seek(htp, 0);
                    return;
                }
            } else {
                if (htp->time_current >= htp->time_length) {
                    LOG_STRM_PRINTF("#%d current = %d / %d\n", htp->index, htp->time_current, htp->time_length);
                    htp->end_flg = 1;
                    LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", htp->index);
                    stream_post_msg(htp->index, STRM_MSG_STREAM_END, 0);
                    return;
                }
            }
            if (0 == htp->fast_receiving)
                int_fast_receive(htp);
        }
    }

    if (htp->delay_flg && htp->delay_clk < clk) {
        if (htp->delay_flg == 2) {
            htp->delay_flg = 3;
            int_trickmode(htp, htp->delay_cmd, htp->delay_arg);
        } else {
            htp->delay_flg = 0;
            htp->delay_clk = 0;
        }
    }
    if (STRM_STATE_FAST != htp->state) {
        htp->diff = strm_play_buffer(htp->strm_play);
        htp->recv_times = 0;
    }
}

static void int_1000ms(void* handle)
{
    int current;
    HttpTsPlay *htp = (HttpTsPlay*)handle;

    if (STRM_STATE_FAST == htp->state)
        return;

    current = strm_play_time(htp->strm_play);
#ifdef DEBUG_BUILD
    {
        int space, length;

        space = strm_play_space(htp->strm_play);
        length = strm_play_length(htp->strm_play);

        LOG_STRM_DEBUG("#%d: sec = %d, diff = %d, length = %d, space = %d\n", htp->index, current / 100, htp->diff, length, space);
    }
#endif
    if (current < 0)
        current = 0;

    if (STRM_STATE_PLAY == htp->state && htp->webradio) {
        if (1 == htp->webradio && htp->diff < 100) {
            LOG_STRM_PRINTF("#%d: temp pause\n", htp->index);
            htp->webradio = 2;
            strm_play_pause(htp->strm_play, htp->index);
        } else if (2 == htp->webradio && htp->diff > 300) {
            LOG_STRM_PRINTF("#%d: temp resume\n", htp->index);
            htp->webradio = 1;
            strm_play_resume(htp->strm_play, htp->index, 0);
        }
    }

    htp->time_current = htp->time_base + current / 100;
    stream_back_currenttime(htp->index, htp->time_current);
    return;
}

static void int_destroy(void* handle)
{
    HttpTsPlay *htp = (HttpTsPlay*)handle;

    int_close_play(htp);

    htp->ht->ts_handle = NULL;

    if (htp->fast_buf_0)
        ts_buf_delete(htp->fast_buf_0);
    if (htp->fast_buf_1)
        ts_buf_delete(htp->fast_buf_1);

    if (htp->ts_parse)
        ts_parse_delete(htp->ts_parse);

    IND_FREE(htp);
}

void* http_ts_play_create(HttpTs* ht, PlayArg *arg, HttpTsArg *httparg)
{
    HttpTsPlay *htp = (HttpTsPlay*)IND_CALLOC(sizeof(HttpTsPlay), 1);
    if (!htp)
        LOG_STRM_ERROUT("#%d malloc failed!\n", ht->index);

    htp->ht = ht;
    htp->http = ht->http;
    htp->index = ht->index;

    htp->strm_msgq = int_strm_msgq(htp->index);
    htp->strm_play = int_strm_play(htp->index);
#ifdef INCLUDE_PVR
    htp->strm_record = int_strm_record(htp->index);
#endif

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

    if (int_open_play(htp, arg, httparg))
        LOG_STRM_ERROUT("#%d int_open_play\n", htp->index);

    return htp;
Err:
    if (htp)
        int_destroy(htp);

    ht->ts_handle = htp;

    return NULL;
}
