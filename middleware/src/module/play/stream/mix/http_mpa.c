/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#include "../http/http.h"



typedef struct {
    char url[STREAM_URL_SIZE];
    char cookie[HTTP_COOKIE_SIZE];
    int loop;
    int start;
} HttpMpaArg;

struct LOCAL {
    int             index;
    int             state;

    HTTPLoop_t      loop;
    HTTP_t          http;

    char            url[STREAM_URL_SIZE];

    int             loop_flg;
    int             open_play;
    int             open_http;

    int             end_flg;
    uint32_t            end_clk;

    int             sync_times;

    ts_audio_t      audio;
    int             starttime;
    int             totaltime;
    int             currenttime;

    StrmBuffer*     play_sb;

    StreamPlay*     strm_play;
    StreamMsgQ*     strm_msgq;
};

static CookieArg* g_cookieArg = NULL;

static void local_error(void *handle, HTTP_MSG msgno);
static void local_seek(struct LOCAL* local, int sec);

static void local_state(struct LOCAL *local, int state, int scale)
{
    local->state = state;
    stream_post_state(local->index, state, scale);
}

static int int_mpa_push(struct LOCAL *local)
{
    char *buf;
    int len, ret;

    len = 0;
    strm_http_buf_get(local->http, &buf, &len);
    if (len <= 0) {
        if (local->end_flg == 1) {
            LOG_STRM_PRINTF("#%d SET end_flag 2\n", local->index);
            local->end_flg = 2;
        }
        return 0;
    }

    ret = ts_audio_write(local->audio, buf, len);
    if (ret < 0) {
        LOG_STRM_ERROR("#%d ret = %d, len = %d\n", local->index, ret, len);
        strm_httploop_break(local->loop);
        local->open_http = 0;
    } else {
        if (ret > 0)
            strm_http_buf_pop(local->http, ret);
    }

    return ret;
}

static int int_mpa_pop(struct LOCAL *local)
{
    int ret;
    uint32_t clk;
    StrmBuffer* sb = local->play_sb;
    static int err = 0;

    clk = strm_httploop_clk(local->loop);

    if (strm_play_buffer(local->strm_play) > 200)
        return 0;
    if (strm_play_space(local->strm_play) < STREAM_BLOCK_LEVEL_PIP / 4)
        return 0;

    if (err >= 3) {
        LOG_STRM_ERROR("#%d ts_audio_read\n", local->index);
        local_error(local, 0);
        return 0;
    }

    if (sb->len <= 0) {
        sb->off = 0;
        sb->len = ts_audio_read(local->audio, sb->buf, 1316);
        if (sb->len < 0) {
            err ++;
            LOG_STRM_WARN("#%d ts_audio_read\n", local->index);
            ts_audio_reset(local->audio);
            return 0;
        }
    }
    err = 0;

    if (sb->len == 0) {
        if (local->end_flg == 2) {
            LOG_STRM_PRINTF("#%d SET end_flag 3\n", local->index);
            local->end_flg = 3;
            local->end_clk = clk + 200;
            if (local->loop_flg == 0)
                strm_play_end(local->strm_play, local->index);
        }
        return 0;
    }
    if (local->totaltime == -1) {
        local->totaltime = ts_audio_second(local->audio, (int)strm_http_get_contentLength(local->http));
        LOG_STRM_PRINTF("#%d totaltime = %d, starttime = %d\n", local->index, local->totaltime, local->starttime);
        if (local->totaltime)
            stream_back_totaltime(local->index, local->totaltime);
        if (local->starttime) {
            if (local->starttime >= local->totaltime) {
                LOG_STRM_WARN("#%dstarttime = %d / %d\n", local->index, local->starttime, local->totaltime);
                local->starttime = local->totaltime - 1;
            }
            local_seek(local, local->starttime);
            return 0;
        }
    }

    ret = sb->len;
    strm_play_push(local->strm_play, local->index, &local->play_sb);
    local->play_sb->len = 0;

    return ret;
}

static void int_sync(struct LOCAL *local)
{
    int pushlen, poplen;

    for (;;) {
        pushlen = int_mpa_push(local);
        poplen = int_mpa_pop(local);

        if (pushlen <= 0 && poplen <= 0)
            break;
    }
}

static void local_100ms(void *handle)
{
    struct LOCAL *local = (struct LOCAL *)handle;

    if (STRM_STATE_PLAY == local->state && local->end_flg >= 3) {
        if (local->loop_flg) {
            strm_http_request(local->http, local->url, 0, 0);
            LOG_STRM_PRINTF("#%d CLR end_flag\n", local->index);
            local->end_flg = 0;
            local->starttime = 0;
        }
        return;
    }

    int_sync(local);
}

static void local_1000ms(void *handle)
{
    struct LOCAL *local = (struct LOCAL *)handle;

#ifdef DEBUG_BUILD
    local->sync_times ++;
    if (local->sync_times >= 5) {
        int diff, buffer;
        diff = strm_play_diff(local->strm_play);
        buffer = strm_play_buffer(local->strm_play);
        LOG_STRM_PRINTF("#%d diff = %d, buffer = %d\n", local->index, diff, buffer);
        local->sync_times = 0;
    }
#endif
    if (local->end_flg <= 3) {
        local->currenttime = local->starttime + strm_play_time(local->strm_play) / 100;
        if (local->currenttime)
            stream_back_currenttime(local->index, local->currenttime);
    }
}

static void local_msgback(void* handle, STRM_MSG msgno, int arg)
{
    struct LOCAL *local = (struct LOCAL *)handle;

    strm_msgq_queue(local->strm_msgq, msgno, arg);
}

static int local_open(struct LOCAL *local, PlayArg *arg, HttpMpaArg *mpaarg)
{
    LOG_STRM_PRINTF("#%d buildtime : "__DATE__" "__TIME__" : stream url = %s\n", local->index, mpaarg->url);

    local->end_flg = 0;
    local->open_play = 0;
    local->open_http = 0;
    local->totaltime = -1;
    local->currenttime = 0;

    ts_audio_reset(local->audio);

    local->loop_flg = mpaarg->loop;
    local->starttime = mpaarg->start;

    if (mpaarg->cookie[0])
        strm_http_set_cookie(local->http, mpaarg->cookie);

    strcpy(local->url, mpaarg->url);
    if (strm_http_request(local->http, local->url, 0, 0))
        LOG_STRM_ERROUT("#%d local_connect\n", local->index);

    strm_play_open(local->strm_play, local->index, local, local_msgback, APP_TYPE_HTTP_MPA, 1316);
    local->open_play = 1;

    local_state(local, STRM_STATE_PLAY, 1);

    return 0;
Err:
    stream_post_msg(local->index, STRM_MSG_OPEN_ERROR, 0);
    return -1;
}

static int local_recv_begin(void *handle)
{
    return 0;
}

static void local_recv_sync(void *handle)
{
    struct LOCAL *local = (struct LOCAL *)handle;

    int_sync(local);
}

static void local_recv_end(void *handle)
{
    struct LOCAL *local = (struct LOCAL *)handle;
    LOG_STRM_PRINTF("#%d SET end_flag 1\n", local->index);
    local->end_flg = 1;
}

static void local_close(struct LOCAL *local)
{
    if (local == NULL)
        return;
    LOG_STRM_PRINTF("#%d\n", local->index);

    if (local->open_play) {
        strm_play_close(local->strm_play, local->index, 0);
        local->open_play = 0;
    }

    if (local->http) {
        strm_httploop_break(local->loop);
        local->http = NULL;
    }
    local->open_http = 0;

    local_state(local, STRM_STATE_CLOSE, 0);
}

static void local_error(void *handle, HTTP_MSG msgno)
{
    struct LOCAL *local = (struct LOCAL *)handle;
    strm_httploop_break(local->loop);
    local->open_http = 0;
    stream_post_msg(local->index, STRM_MSG_OPEN_ERROR, 0);
}

static void local_seek(struct LOCAL* local, int sec)
{
    int length, offset;

    if (local->totaltime <= 0)
        LOG_STRM_ERROUT("#%d totaltime = %d\n", local->index, local->totaltime);
    if (sec < 0)
        sec = 0;
    if (sec >= local->totaltime)
        sec = local->totaltime - 1;

    length = (int)strm_http_get_contentLength(local->http);

    if (length <= 0)
        LOG_STRM_ERROUT("#%d length = %d\n", local->index, length);
    strm_play_reset(local->strm_play, local->index, 1);
    strm_play_resume(local->strm_play, local->index, 0);

    offset = length * sec / local->totaltime;
    LOG_STRM_PRINTF("#%d offset = %d, length = %d\n", local->index, offset, length);
    strm_http_reset(local->http);
    strm_http_request(local->http, local->url, (long long)offset, 0);

    local->end_flg = 0;
    local->starttime = sec;
    ts_audio_reset(local->audio);

    local_state(local, STRM_STATE_PLAY, 1);

Err:
    return;
}

static void local_pause(struct LOCAL* local)
{
    if (local->state != STRM_STATE_PLAY)
        LOG_STRM_ERROUT("#%d state = %d\n", local->index, local->state);
    strm_play_pause(local->strm_play, local->index);
    LOG_STRM_PRINTF("#%d STRM_STATE_PAUSE\n", local->index);

    local_state(local, STRM_STATE_PAUSE, 0);
Err:
    return;
}

static void local_resume(struct LOCAL* local)
{
    if (local->state != STRM_STATE_PAUSE)
        LOG_STRM_ERROUT("#%d state = %d\n", local->index, local->state);
    strm_play_resume(local->strm_play, local->index, 0);
    LOG_STRM_PRINTF("#%d STRM_STATE_PLAY\n", local->index);

    local_state(local, STRM_STATE_PLAY, 1);
Err:
    return;
}

static void local_trackmode(struct LOCAL* local, int cmd, int arg)
{
    switch(cmd) {
    case STREAM_CMD_SEEK:
        LOG_STRM_PRINTF("#%d STREAM_CMD_SEEK arg = %d\n", local->index, arg);
        local_seek(local, arg);
        break;
    case STREAM_CMD_PAUSE:
    LOG_STRM_PRINTF("#%d STREAM_CMD_PAUSE\n", local->index);
        local_pause(local);
        break;
    case STREAM_CMD_RESUME:
        LOG_STRM_PRINTF("#%d STREAM_CMD_RESUME\n", local->index);
        local_resume(local);
        break;

    default:
        LOG_STRM_ERROR("#%d Unkown CMD %d\n", local->index, cmd);
        break;
    }
}

static void local_cmd(void *handle, StreamCmd* strmCmd)
{
    struct LOCAL *local = (struct LOCAL *)handle;
    int cmd = strmCmd->cmd;

    LOG_STRM_PRINTF("#%d cmd = %d, state = %d\n", local->index, cmd, local->state);

    if (stream_deal_cmd(local->index, strmCmd) == 1)
        return;

    switch(cmd) {
    case STREAM_CMD_FAST:
    case STREAM_CMD_SEEK:
    case STREAM_CMD_PAUSE:
    case STREAM_CMD_RESUME:
        {
            int cmdsn = strmCmd->arg3;
            local_trackmode(local, cmd, strmCmd->arg0);
            if (cmdsn)
                stream_back_cmd(local->index, cmdsn);
        }
        break;
    case STREAM_CMD_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE\n", local->index);
        strm_httploop_break(local->loop);
        local->open_http = 0;
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
    struct LOCAL *local = (struct LOCAL *)handle;

    switch(msgno) {
    case STRM_MSG_FIRST_TIMEOUT:
        break;
    case STRM_MSG_STREAM_END:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END loop = %d\n", local->index, local->loop_flg);
        local->end_flg = 4;
        if (local->totaltime > 0)
            stream_back_currenttime(local->index, local->totaltime);
        stream_post_msg(local->index, STRM_MSG_STREAM_END, 0);
        break;
    case STRM_MSG_PLAY_ERROR:
        LOG_STRM_PRINTF("#%d STRM_MSG_PLAY_ERROR\n", local->index);
        local_error(local, 0);
        break;

    case STRM_MSG_STREAM_MUSIC:
        LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_MUSIC\n", local->index);
        stream_post_msg(local->index, STRM_MSG_STREAM_VIEW, 0);
        stream_post_msg(local->index, STRM_MSG_STREAM_MUSIC, 0);
        break;
    case STRM_MSG_PTS_VIEW:
        LOG_STRM_PRINTF("#%d STRM_MSG_PTS_VIEW\n", local->index);
        break;

    default:
        LOG_STRM_ERROUT("#%d Unkown MSG %d\n", local->index, msgno);
        break;
    }

Err:
    return;
}

static void local_loop_play(void* handle, int idx, mid_msgq_t msgq, PlayArg* arg, char* argbuf)
{
    struct LOCAL *local;

    local = (struct LOCAL *)IND_CALLOC(sizeof(struct LOCAL), 1);
    if (!local)
        LOG_STRM_ERROUT("#%d malloc failed!\n", idx);

    local->play_sb = strm_buf_malloc(1316);
    local->play_sb->len = 0;

    local->audio = ts_audio_create( );
    if (!local->audio)
        LOG_STRM_ERROUT("#%d ts_audio_create\n", idx);

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

    {
        HttpOp op;

        memset(&op, 0, sizeof(op));
    
        op.recv_begin = local_recv_begin;
        op.recv_sync = local_recv_sync;
        op.recv_end = local_recv_end;
        op.deal_error = local_error;
        strm_http_set_opset(local->http, &op, local);
    }

    local->index = idx;
    local->open_http = 1;
    local->strm_msgq = int_strm_msgq(idx);
    local->strm_play = int_strm_play(idx);

    if (local_open(local, arg, ( HttpMpaArg *)argbuf))
        LOG_STRM_ERROUT("#%d local_open\n", idx);

    strm_httploop_loop(local->loop);
    LOG_STRM_PRINTF("#%d exit loop!\n", idx);

    local_close(local);

Err:
    if (local) {
        if (local->play_sb)
            strm_buf_free(local->play_sb);
        if (local->loop)
            strm_httploop_delete(local->loop);
        if (local->audio)
            ts_audio_delete(local->audio);
        IND_FREE(local);
    }
    LOG_STRM_PRINTF("#%d exit play\n", idx);
    return;
}

static int local_argparse_play(int idx, PlayArg* arg, char *argbuf, const char* url, int shiftlen, int begin, int end)
{
     HttpMpaArg *mpaarg = ( HttpMpaArg *)argbuf;

    if (url == NULL)
        LOG_STRM_ERROUT("#%d url is NULL\n", idx);
    LOG_STRM_PRINTF("#%d url = %s, loop = %d\n", idx, url, shiftlen);

    IND_STRCPY(mpaarg->url, url);
    if (shiftlen == 1)
        mpaarg->loop = 1;
    else
        mpaarg->loop = 0;
    if (begin > 0)
        mpaarg->start = begin;
    else
        mpaarg->start = 0;

    if (g_cookieArg && g_cookieArg->magic == arg->magic)
        IND_STRCPY(mpaarg->cookie, g_cookieArg->cookie);
    else
        mpaarg->cookie[0] = 0;

    return 0;
Err:
    return -1;
}

int http_mpa_create_stream(StreamCtrl *ctrl)
{
    if (g_cookieArg == NULL)
        g_cookieArg = (CookieArg*)IND_CALLOC(sizeof(CookieArg), 1);
    if (g_cookieArg)
        g_cookieArg->magic = 0;

    ctrl->handle = ctrl;

    ctrl->loop_play = local_loop_play;

    ctrl->argsize = sizeof(HttpMpaArg);
    ctrl->argparse_play = local_argparse_play;

    return 0;
}

void mid_stream_hmpa_cookie(int idx, char* cookie)
{
    int len;

    if (idx != 0 || cookie == NULL)
        LOG_STRM_ERROUT("index = %d, cookie = %p\n", idx, cookie);
    if (g_cookieArg == NULL)
        LOG_STRM_ERROUT("cookieArg is NULL\n");

    len = strlen(cookie);
    if (len >= HTTP_COOKIE_SIZE)
        LOG_STRM_ERROUT("cookie too large! len = %d\n", len);

    g_cookieArg->magic = int_stream_nextmagic( );
    IND_STRCPY(g_cookieArg->cookie, cookie);

Err:
    return;
}
