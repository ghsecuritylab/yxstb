/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#include "../http/http.h"

#define HTTP_MIX_RNG_SIZE        (32 * 1024)

typedef struct {
    char url[STREAM_URL_SIZE];
}  HttpPCMArg;

struct LOCAL {
    int         index;

    HTTPLoop_t  loop;
    HTTP_t      http;

    int         end_flg;
    uint32_t        end_clk;

    ind_pcm_t   pcmbuf;

    int         codec;
    int         codec_open;
};

static void int_sync(struct LOCAL *local)
{
    char *buf;
    int httplen, pcmlen, len;

    if (local->codec_open == 0)
        return;

    if (local->end_flg == 3) {
        uint32_t clk = strm_httploop_clk(local->loop);
        if (local->end_clk <= clk) {
            LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", local->index);
            stream_post_msg(local->index, STRM_MSG_STREAM_END, 0);
            //strm_httploop_break(local->loop);
            local->end_flg = 4;
        }
        return;
    }

    for (;;) {
        pcmlen = 0;
        httplen = 0;

        len = 0;
        strm_http_buf_get(local->http, &buf, &len);
        if (len > 0) {
            httplen = ind_pcm_write(local->pcmbuf, buf, len);
            if (httplen > 0)
                strm_http_buf_pop(local->http, httplen);
        } else {
            if (1 == local->end_flg) {
                LOG_STRM_PRINTF("#%d SET end_flag 2\n", local->index);
                local->end_flg = 2;
            }
        }

        len = 0;
        ind_pcm_read_get(local->pcmbuf, &buf, &len);
        if (len > 0) {
            pcmlen = codec_pcm_push(local->codec, buf, len);
            if (pcmlen > 0)
                ind_pcm_read_pop(local->pcmbuf, pcmlen);
        } else {
            if (2 == local->end_flg) {
                LOG_STRM_PRINTF("#%d SET end_flag 3\n", local->index);
                local->end_flg = 3;
                local->end_clk = strm_httploop_clk(local->loop) + 200;
            }
        }

        if (pcmlen <= 0 && httplen <= 0)
            break;
    }
}

static void local_100ms(void* handle)
{
    struct LOCAL *local = (struct LOCAL *)handle;

    int_sync(local);
}

static void local_1000ms(void *handle)
{
}

static int local_open(struct LOCAL *local, PlayArg *arg, HttpPCMArg *pcmarg)
{
    LOG_STRM_PRINTF("#%d url = %s\n", local->index, pcmarg->url);

    ind_pcm_src_set(local->pcmbuf, 0, 0, 0);

    if (strm_http_request(local->http, pcmarg->url, 0, 0))
        LOG_STRM_ERROUT("#%d local_connect\n", local->index);

    return 0;
Err:
    return -1;
}

static int local_recv_begin(void* handle)
{
    return 0;
}

static void local_recv_sync(void* handle)
{
    char *buf;
    int len, ret;
    struct LOCAL *local = (struct LOCAL *)handle;

    if (local->codec_open == 0) {
        struct MixPCM mixpcm;

        len = 0;
        strm_http_buf_get(local->http, &buf, &len);
        if (len < 188)
            return;

        ret = ind_wav_parse((unsigned char*)buf, len, &mixpcm.sampleRate, &mixpcm.bitWidth, &mixpcm.channels);
        if (ret <= 0) {
            LOG_STRM_ERROR("#%d ind_wav_parse\n", local->index);
            strm_httploop_break(local->loop);
            return;
        }

        ind_pcm_src_set(local->pcmbuf, mixpcm.sampleRate, mixpcm.bitWidth, mixpcm.channels);
        LOG_STRM_PRINTF("#%d src: sampleRate = %d, bitWidth = %d, channels = %d\n", local->index, mixpcm.sampleRate, mixpcm.bitWidth, mixpcm.channels);
        ind_pcm_dst_get(local->pcmbuf, &mixpcm.sampleRate, &mixpcm.bitWidth, &mixpcm.channels);
        LOG_STRM_PRINTF("#%d dst: sampleRate = %d, bitWidth = %d, channels = %d\n", local->index, mixpcm.sampleRate, mixpcm.bitWidth, mixpcm.channels);

        codec_pcm_open(local->codec, mixpcm.sampleRate, mixpcm.bitWidth, mixpcm.channels);
        local->codec_open = 1;

        strm_http_buf_pop(local->http, ret);
    }

    int_sync(local);
}

static void local_recv_end(void* handle)
{
    struct LOCAL *local = (struct LOCAL *)handle;
    LOG_STRM_PRINTF("#%d SET end_flag 1\n", local->index);
    local->end_flg = 1;
}

static int local_close(void* handle)
{
    struct LOCAL *local = (struct LOCAL *)handle;

    if (local->codec_open) {
        codec_pcm_close(local->codec);
        local->codec_open = 0;
    }

    return 0;
}

static void local_error(void* handle, HTTP_MSG msgno)
{
    struct LOCAL *local = (struct LOCAL *)handle;

    LOG_STRM_PRINTF("#%d STRM_MSG_OPEN_ERROR\n", local->index);
    stream_post_msg(local->index, STRM_MSG_OPEN_ERROR, 0);
    strm_httploop_break(local->loop);
}

static void local_cmd(void* handle, StreamCmd* strmCmd)
{
    struct LOCAL *local = (struct LOCAL *)handle;
    int cmd = strmCmd->cmd;

    LOG_STRM_PRINTF("#%d cmd = %d\n", local->index, cmd);

    if (stream_deal_cmd(local->index, strmCmd) == 1)
        return;

    switch(cmd) {
    case STREAM_CMD_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE\n", local->index);
        strm_httploop_break(local->loop);
        break;

    default:
        LOG_STRM_ERROUT("#%d Unkown CMD %d\n", local->index, cmd);
        break;
    }

Err:
    return;
}

static void local_msg(void *handle, STRM_MSG msgno, int arg)
{
}

static void local_loop_play(void *handle, int idx, mid_msgq_t msgq, PlayArg* arg, char* argbuf)
{
    struct LOCAL *local;

    local = (struct LOCAL *)IND_CALLOC(sizeof(struct LOCAL), 1);
    if (!local)
        LOG_STRM_ERROUT("#%d malloc failed!\n", idx);

    local->pcmbuf = ind_pcm_create(HTTP_MIX_RNG_SIZE);
    if (!local->pcmbuf)
        LOG_STRM_ERROUT("#%d ind_pcm_create\n", idx);

    {
        HttpLoopOp op;

        memset(&op, 0, sizeof(op));

        op.deal_cmd = local_cmd;
        op.deal_msg = local_msg;
        op.local_100ms = local_100ms;
        op.local_1000ms = local_1000ms;

        local->loop = strm_httploop_create(idx, &op, local, msgq);
        if (!local->loop)
            LOG_STRM_ERROUT("strm_httploop_create!\n");
    }
    local->http = strm_http_create(local->loop, HTTP_RECV_SIZE);
    if (!local->http)
        LOG_STRM_ERROUT("#%d strm_http_create!\n", idx);

    local->index = idx;

    {
        HttpOp op;

        memset(&op, 0, sizeof(op));
    
        op.recv_begin = local_recv_begin;
        op.recv_sync = local_recv_sync;
        op.recv_end = local_recv_end;
        op.deal_error = local_error;
        strm_http_set_opset(local->http, &op, local);
    }

    if (idx == STREAM_INDEX_PIP)
        local->codec = 1;
    else
        local->codec = 0;

    if (local_open(local, arg, (HttpPCMArg *)argbuf))
        LOG_STRM_ERROUT("#%d local_open\n", idx);

    strm_httploop_loop(local->loop);
    LOG_STRM_PRINTF("#%d exit local_loop!\n", idx);

    local_close(local);

Err:
    if (local) {
        if (local->pcmbuf)
            ind_pcm_delete(local->pcmbuf);
        if (local->loop)
            strm_httploop_delete(local->loop);
        IND_FREE(local);
    }
    return;
}

static int local_argparse_play(int idx, PlayArg* arg, char *argbuf, const char* url, int shiftlen, int begin, int end)
{
    HttpPCMArg *pcmarg = (HttpPCMArg *)argbuf;

    if (url == NULL)
        LOG_STRM_ERROUT("#%d url is NULL\n", idx);

    IND_STRCPY(pcmarg->url, url);

    return 0;
Err:
    return -1;
}

int http_pcm_create_stream(StreamCtrl *ctrl)
{
    ctrl->handle = ctrl;

    ctrl->loop_play = local_loop_play;

    ctrl->argsize = sizeof(HttpPCMArg);
    ctrl->argparse_play = local_argparse_play;

    return 0;
}
