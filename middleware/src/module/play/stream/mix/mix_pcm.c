/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/
#include <sys/select.h>

#include "mix.h"
#include "config/pathConfig.h"

#define MIX_ELEM_SIZE            (4 * 1024)
#define MIX_PCM_RNG_SIZE        (128 * 1024)

struct LOCAL
{
    uint32_t            clk;
    uint32_t            warnclk;

    int                 index;
    uint32_t            magic;

    int                 codec;

    STRM_STATE          state;
    ind_tlink_t         tlink;

    mid_msgq_t          msgq;
    int                 msgfd;

    StrmBufQue*         sbq;
    StrmBuffer*         popSb;
    StrmBuffer*         pushSb;

#ifdef ENABLE_SAVE_PLAY
    FILE*               save_fp;
#endif
};

typedef struct {
    struct MixPCM mixpcm;
} PcmArg;

//#define ENABLE_SAVE_PLAY

static mid_mutex_t g_mutex = NULL;

static struct LOCAL* g_array[2] = {NULL, NULL};

static void local_push(void *arg);

static int local_open(struct LOCAL *local, uint32_t magic, PcmArg *pcmarg)
{
    struct MixPCM* mixpcm = &pcmarg->mixpcm;

    if (codec_pcm_open(local->codec, mixpcm->sampleRate, mixpcm->bitWidth, mixpcm->channels))
        LOG_STRM_ERROUT("codec_pcm_open\n");

    ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_100MS, INTERVAL_CLK_100MS, local_push, local);

    mid_mutex_lock(g_mutex);

    local->magic = magic;
    local->state = STRM_STATE_PLAY;

    mid_mutex_unlock(g_mutex);

#ifdef ENABLE_SAVE_PLAY
    local->save_fp = fopen(DEFAULT_EXTERNAL_DATAPATH"/pcm.wav", "wb");
    LOG_STRM_PRINTF("@@@@@@@@: fp = %p\n", sp->save_fp);
#endif

    LOG_STRM_PRINTF("#%d STRM_MSG_PTS_VIEW\n", local->index);
    stream_post_msg(local->index, STRM_MSG_PTS_VIEW, 0);

    return 0;
Err:
    return -1;
}

static int local_close(struct LOCAL *local)
{
    LOG_STRM_PRINTF("#%d\n", local->index);

    if (local->state == STRM_STATE_CLOSE)
        LOG_STRM_ERROUT("STRM_STATE_CLOSE\n");

    #ifdef ENABLE_SAVE_PLAY
        LOG_STRM_PRINTF("@@@@@@@@: save_len = %d\n", local->save_len);
        if (local->save_fp) {
            fclose(local->save_fp);
            local->save_fp = NULL;
        }
    #endif

    codec_pcm_close(local->codec);

    ind_timer_delete(local->tlink, local_push, local);

    mid_mutex_lock(g_mutex);
    local->state = STRM_STATE_CLOSE;
    mid_mutex_unlock(g_mutex);

    return 0;
Err:
    return -1;
}


static void local_cmd(struct LOCAL* local, StreamCmd* strmCmd)
{
    int cmd = strmCmd->cmd;

    LOG_STRM_PRINTF("#%d %d %d\n", local->index, local->state, cmd);

    if (stream_deal_cmd(local->index, strmCmd) == 1)
        return;

    switch(cmd) {
    case STREAM_CMD_CLOSE:
        LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE\n", local->index);
        local_close(local);
        break;
    default:
        LOG_STRM_PRINTF("#%d cmd = %d\n", local->index, cmd);
        break;
    }
}

static void local_push(void *arg)
{
    int len;
    StrmBuffer* sb;
    struct LOCAL *local = (struct LOCAL *)arg;

    for (;;) {
        sb = local->popSb;
        if (sb->len <= 0) {
            mid_mutex_lock(g_mutex);
            strm_bufque_pop(local->sbq, &local->popSb);
            mid_mutex_unlock(g_mutex);
            sb = local->popSb;
            if (sb->len <= 0)
                return;
            sb->off = 0;
        }
        len = codec_pcm_push(local->codec, sb->buf + sb->off, sb->len);
        if (len <= 0)
            return;
        sb->off += len;
        sb->len -= len;
    }
}

static void local_loop(struct LOCAL *local)
{
    uint32_t            clk, clks, out;
    fd_set            rset;
    struct timeval    tv;

    while (local->state != STRM_STATE_CLOSE) {
        clk = mid_10ms( );
        local->clk = clk;
        out = ind_timer_clock(local->tlink);
        if (out <= clk) {
            ind_timer_deal(local->tlink, clk);
            continue;
        }

        clks = out - clk;
        tv.tv_sec = clks / 100;
        tv.tv_usec = clks % 100 * 10000;

        FD_ZERO(&rset);
        FD_SET((uint32_t)local->msgfd, &rset);
        if (select(local->msgfd + 1, &rset , NULL,  NULL, &tv) <= 0)
            continue;

        if (FD_ISSET((uint32_t)local->msgfd, &rset)) {
            StreamCmd strmCmd;

            LOG_STRM_PRINTF("#%d\n", local->index);
            memset(&strmCmd, 0, sizeof(strmCmd));
            mid_msgq_getmsg(local->msgq, (char *)(&strmCmd));

            local_cmd(local, &strmCmd);

            LOG_STRM_PRINTF("#%d cmd = %d\n", local->index, strmCmd.cmd);
        }
    }
}

static void local_loop_play(void *handle, int idx, mid_msgq_t msgq, PlayArg* arg, char* argbuf)
{
    int codec;
    struct LOCAL *local;

    local = (struct LOCAL *)IND_CALLOC(sizeof(struct LOCAL), 1);
    if (local == NULL)
        LOG_STRM_ERROUT("#%d malloc failed!\n", idx);

    local->pushSb = strm_buf_malloc(MIX_ELEM_SIZE);
    local->popSb = strm_buf_malloc(MIX_ELEM_SIZE);
    local->popSb->len = 0;

    local->sbq = strm_bufque_create(MIX_ELEM_SIZE, MIX_PCM_RNG_SIZE / MIX_ELEM_SIZE);

    local->index = idx;
    local->msgq = msgq;
    local->msgfd = mid_msgq_fd(local->msgq);
    local->tlink = int_stream_tlink(idx);

    if (idx == STREAM_INDEX_PIP)
        codec = 1;
    else
        codec = 0;
    local->codec = codec;

    mid_mutex_lock(g_mutex);
    g_array[codec] = local;
    mid_mutex_unlock(g_mutex);

    if (local_open(local, arg->magic, (PcmArg *)argbuf))
        LOG_STRM_ERROUT("#%d local_open\n", idx);

    local_loop(local);
    LOG_STRM_PRINTF("#%d exit local_loop!\n", idx);

Err:
    if (local) {
        mid_mutex_lock(g_mutex);
        g_array[codec] = NULL;
        mid_mutex_unlock(g_mutex);

        if (local->popSb)
            strm_buf_free(local->popSb);
        if (local->pushSb)
            strm_buf_free(local->pushSb);
        if (local->sbq)
            strm_bufque_delete(local->sbq);
        IND_FREE(local);
    }
    return;
}

static int local_argparse_play(int idx, PlayArg* arg, char *argbuf, const char* url, int shiftlen, int begin, int end)
{
    struct MixPCM *mixpcm;
    PcmArg *pcmarg = (PcmArg *)argbuf;

    if (url == NULL)
        LOG_STRM_ERROUT("#%d url is NULL\n", idx);

    mixpcm = (struct MixPCM *)url;
    pcmarg->mixpcm.sampleRate = mixpcm->sampleRate;
    pcmarg->mixpcm.bitWidth    = mixpcm->bitWidth;
    pcmarg->mixpcm.channels    = mixpcm->channels;
    LOG_STRM_PRINTF("#%d sampleRate = %d, bitWidth = %d, channels = %d\n", idx, mixpcm->sampleRate, mixpcm->bitWidth, mixpcm->channels);

    return 0;
Err:
    return -1;
}

int mix_pcm_create_stream(StreamCtrl *ctrl)
{
    if (!g_mutex)
        g_mutex = mid_mutex_create( );

    ctrl->handle = ctrl;

    ctrl->loop_play = local_loop_play;

    ctrl->argsize = sizeof(PcmArg);
    ctrl->argparse_play = local_argparse_play;

    return 0;
}

int stream_mix_pcm_space(int idx, uint32_t magic)
{
    int space = -1;
    struct LOCAL *local;

    mid_mutex_lock(g_mutex);

    if (idx != 0 && idx != 1)
        LOG_STRM_ERROUT("index = %d\n", idx);

    local = g_array[idx];
    if (!local) {
        space = 0;
    } else if (magic == local->magic && local->state == STRM_STATE_PLAY) {
        space = strm_bufque_space(local->sbq);
        //LOG_STRM_PRINTF("index = %d, space = %d\n", index, space);
    } else {
        if (local->warnclk < local->clk) {
            LOG_STRM_WARN("#%d, magic = %u/%u, state = %d\n", idx, magic, local->magic, local->state);
            local->warnclk = local->clk + 500;
        }
        space = -2;
    }

Err:
    mid_mutex_unlock(g_mutex);
    return space;
}

int stream_mix_pcm_push(int idx, uint32_t magic, char* buf, int len)
{
    int l, bytes = -1;
    struct LOCAL *local;

    mid_mutex_lock(g_mutex);

    if (idx != 0 && idx != 1)
        LOG_STRM_ERROUT("index = %d\n", idx);
    if (buf == NULL || len <= 0)
        LOG_STRM_ERROUT("buf = %p, len = %d\n", buf, len);

    local = g_array[idx];
    if (!local)
        LOG_STRM_ERROUT("#%d local is NULL\n", idx);

    bytes = 0;
    if (memcmp(buf, "RIFF", 4) == 0) {
        l = ind_wav_parse((uint8_t*)buf, len, NULL, NULL, NULL);
        if (l > 0) {
            LOG_STRM_DEBUG("#%d WAVE\n", idx);
            buf += l;
            len -= l;
        }
    }
    if (magic == local->magic && local->state == STRM_STATE_PLAY) {
        StrmBuffer* sb;

        while (len > 0 && strm_bufque_space(local->sbq) >= MIX_ELEM_SIZE) {
            l = len;
            if (l > MIX_ELEM_SIZE)
                l = MIX_ELEM_SIZE;
            sb = local->pushSb;
            memcpy(sb->buf, buf, l);
            sb->off = 0;
            sb->len = l;
            strm_bufque_push(local->sbq, &local->pushSb);
            bytes += l;
            buf += l;
            len -= l;
        }
    }
    //LOG_STRM_PRINTF("index = %d, bytes = %d\n", index, bytes);

Err:
    mid_mutex_unlock(g_mutex);
    return bytes;
}
