/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/
#include <sys/types.h>
#include <unistd.h>

#include "stream.h"
#include "stream_port.h"

#include "ind_gfx.h"
#include "ind_ts.h"

#include "config/pathConfig.h"

#define MOSAIC_NUM          9

#define FIST_CA_CLK         100
#define WAIT_ALL_CLK        250
#define WAIT_BUF_CLK        200

#define MOSIC_VIDEO_PID     0x100
#define MOSIC_AUDIO_PID     0x200
#define MOSIC_PCR_PID       0x300

enum {
    INT_STREAM_CMD_NONE = 0,
    INT_STREAM_CMD_SETKEY,
    INT_STREAM_CMD_OPEN,
    INT_STREAM_CMD_CLOSE,
};

typedef struct {
    int             key;
    short           x;
    short           y;
    short           width;
    short           height;
    struct ind_sin  mult_sin;

    char            igmp[IGMP_INFO_SIZE];
    int             igmp_size;

    StrmMsgCall     msgcall;
    int             callarg;
} Mosaic;
typedef Mosaic*     Mosaic_t;

typedef struct {
    MultiPlayCall   igmp_play_call;
    MultiStopCall   igmp_stop_call;

    Mosaic          msc_array[MOSAIC_NUM];
} MosaicArg;

typedef struct {
    struct ind_sin  mult_sin;
    int             mult_sock;

    char            igmp[IGMP_INFO_SIZE];
    int             igmp_size;

    FILE*           fp;
    uint32_t            clk;

    ts_parse_t      ts_parse;
    struct ts_psi   ts_psi;
    struct ts_psi   ps_psi;
    int             ts_psi_flag;
    int             ts_psi_excep;

    struct ts_mosaic ts_mosaic;

    StrmMsgCall     msgcall;
    int             callarg;
} MscElem;
typedef MscElem* MscElem_t;

typedef struct {
    int             index;
    uint32_t            clk;

    STRM_STATE      state;

    char            rtp_buf[RTP_BUFFER_LENGTH];

    MscElem         elem_array[MOSAIC_NUM];

    MultiPlayCall   igmp_play_call;
    MultiStopCall   igmp_stop_call;

    mid_msgq_t      msgq;
    int             msgfd;
    int             maxfd;

    ts_buf_t        ts_buf;

    ind_tlink_t     tlink;
} LOCAL;
typedef LOCAL* LOCAL_t;

static int g_save_flg = 0;

static int g_msg_key = -1;
static int g_msg_arg = 0;
static StrmMsgCall g_msg_call = NULL;

static int g_msc_key = -1;
static int g_msc_mute = 0;
static int g_msc_enable = 1;
static mid_mutex_t g_mutex = NULL;

static MosaicArg *g_mscarg = NULL;

static uint32_t g_ca_clk = 1;

static void local_mosaic_close(LOCAL_t local, int key);

static void local_state(LOCAL_t local, STRM_STATE state, int scale)
{
    local->state = state;
    //stream_post_state(local->index, state, scale);
}

static void local_message(MscElem_t elem, STRM_MSG msgno, int arg)
{
    if (elem->msgcall)
        elem->msgcall(elem->ts_mosaic.key, msgno, arg, 0, elem->callarg);
    else
        stream_port_message(elem->ts_mosaic.key, msgno, arg, 0);
}

static void local_sync(void *arg)
{
    int i;
    MscElem_t elem_array, elem;

    LOCAL_t local = (LOCAL_t)arg;

    elem_array = local->elem_array;

    for (i = 0; i < MOSAIC_NUM; i ++) {
        elem = &elem_array[i];
        if (-1 == elem->ts_mosaic.key)
            continue;
        if (elem->clk && elem->clk + INTERVAL_CLK_DATA_TIMEOUT < local->clk) {
            LOG_STRM_WARN("#%d key = %d MOSAIC_MSG_TIMEOUT\n", i, elem->ts_mosaic.key);
            local_message(elem, MOSAIC_MSG_TIMEOUT, 0);
            elem->clk = 0;
        }
    }
}

static void local_push(void *arg)
{
    int len, length;
    char *buf;
    ts_buf_t tb;

    LOCAL_t local = (LOCAL_t)arg;

    tb = local->ts_buf;
    for (;;) {
        length = ts_buf_length(tb);
        if (length <= 0)
            return;

        ts_buf_read_get(tb, &buf, &len);
        if (len <= 0 || len % 188)
            return;

        len = codec_mosaic_push(buf, len);
        if (len <= 0)
            break;
        ts_buf_read_pop(tb, len);
    }
}

static void local_mosaic_open(LOCAL_t local, int key)
{
    int i, idx, mult_sock;
    MscElem_t elem_array, elem;

    LOG_STRM_PRINTF("key = %d\n", key);

    local_mosaic_close(local, key);

    idx = -1;
    elem_array = local->elem_array;
    for (i = 0; i < MOSAIC_NUM; i ++) {
        if (-1 == elem_array[i].ts_mosaic.key) {
            idx = i;
            break;
        }
    }
    if (-1 == idx)
        LOG_STRM_ERROUT("elem_array is full!\n");

    elem = &elem_array[idx];
    elem->igmp_size = -1;
    elem->fp = NULL;
    mid_mutex_lock(g_mutex);
    {
        Mosaic_t msc, msc_array;

        msc_array = g_mscarg->msc_array;
        for (i = 0; i < MOSAIC_NUM; i ++) {
            msc = &msc_array[i];
            if (key == msc->key) {
                int igmp_size;

                elem->ts_mosaic.x = msc->x;
                elem->ts_mosaic.y = msc->y;
                elem->ts_mosaic.width = msc->width;
                elem->ts_mosaic.height = msc->height;

                elem->msgcall = msc->msgcall;
                elem->callarg = msc->callarg;

                elem->mult_sin = msc->mult_sin;

                igmp_size = msc->igmp_size;
                if (igmp_size > 0 && igmp_size <= IGMP_INFO_SIZE) {
                    IND_MEMCPY(elem->igmp, msc->igmp, igmp_size);
                    elem->igmp_size = igmp_size;
                } else {
                    elem->igmp_size = 0;
                }
                break;
            }
        }
    }
    mid_mutex_unlock(g_mutex);

    if (elem->igmp_size < 0)
        LOG_STRM_ERROUT("igmp_size = %d\n", elem->igmp_size);

    if (local->igmp_play_call) {
        struct ind_sin mult_sin = elem->mult_sin;

        if (elem->igmp_size > 0)
            mult_sock = local->igmp_play_call(elem->igmp, elem->igmp_size, &mult_sin);
        else
            mult_sock = local->igmp_play_call(NULL, 0, &mult_sin);
    } else {
        mult_sock = stream_port_multicast_play(elem->mult_sin.in_addr.s_addr, elem->mult_sin.port);
    }

    elem->ts_mosaic.key = key;

    if (mult_sock < 0) {
        local_message(elem, MOSAIC_MSG_ERROR, 0);
        elem->ts_mosaic.key = -1;
        LOG_STRM_ERROUT("#%d key = %d MOSAIC_MSG_ERROR %08x:%hd\n", idx, key, elem->mult_sin.in_addr.s_addr, elem->mult_sin.port);
    }

    if (g_save_flg) {
        char filename[16];

        sprintf(filename, DEFAULT_EXTERNAL_DATAPATH"/mosaic_%d.ts", key);
        elem->fp = fopen(filename, "wb");
        LOG_STRM_PRINTF("key = %d open fp = %p\n", key, elem->fp);
    }

    if (mult_sock >= local->maxfd)
        local->maxfd = mult_sock + 1;

    elem->clk = local->clk;

    elem->ts_psi_flag = 0;
    elem->ts_psi_excep = 0;
    memset(&elem->ps_psi,    0, sizeof(struct ts_psi));
    memset(&elem->ts_psi,    0, sizeof(struct ts_psi));

    ts_parse_reset(elem->ts_parse);

    {
        char mult_info[16];
        ind_net_stoa(elem->mult_sin.in_addr.s_addr, mult_info);
        LOG_STRM_PRINTF("#%d key = %d %s:%hd\n", idx, key, mult_info, elem->mult_sin.port);
    }

    elem->mult_sock = mult_sock;

    if (key == g_msc_key)
        codec_mosaic_set(key);

Err:
    return;
}

static void local_mosaic_close(LOCAL_t local, int key)
{
    int i;
    MscElem_t elem_array, elem;

    if (key < 0)
        return;

    elem = NULL;
    elem_array = local->elem_array;
    for (i = 0; i < MOSAIC_NUM; i ++) {
        elem = &elem_array[i];
        if (key == elem->ts_mosaic.key) {
            LOG_STRM_PRINTF("#%d key = %d\n", i, key);

            if (local->igmp_stop_call) {
                struct ind_sin mult_sin = elem->mult_sin;

                if (elem->igmp_size > 0)
                    local->igmp_stop_call(elem->igmp, elem->igmp_size, elem->mult_sock, &mult_sin);
                else
                    local->igmp_stop_call(NULL, 0, elem->mult_sock, &mult_sin);
            } else {
                stream_port_multicast_stop(elem->mult_sock, elem->mult_sin.in_addr.s_addr);
            }
            elem->mult_sock = -1;

            if (elem->fp) {
                LOG_STRM_PRINTF("key = %d close fp = %p\n", key, elem->fp);
                fclose(elem->fp);
                elem->fp = NULL;
            }

            if (elem->ts_psi_flag)
                codec_mosaic_elem_close(key);
            elem->ts_mosaic.key = -1;
        }
    }
}

static int local_open_play(LOCAL_t local, PlayArg *arg)
{
    int i;
    uint32_t clk;
    ts_mosaic_t ts_msc;
    MscElem_t elem_array;

    LOG_STRM_PRINTF("\n");

    local->maxfd = local->msgfd + 1;

    clk = mid_10ms();
    local->clk = clk;

    mid_mutex_lock(g_mutex);

    local->igmp_play_call =  g_mscarg->igmp_play_call;
    local->igmp_stop_call =  g_mscarg->igmp_stop_call;

    mid_mutex_unlock(g_mutex);

    elem_array = local->elem_array;

    for (i = 0; i < MOSAIC_NUM; i ++) {
        elem_array[i].mult_sock = -1;

        ts_msc = &elem_array[i].ts_mosaic;

        ts_msc->key = -1;

        ts_msc->pcr = MOSIC_PCR_PID + i;
        ts_msc->vpid = MOSIC_VIDEO_PID + i;
        ts_msc->apid = MOSIC_AUDIO_PID + i;

        ts_msc->vtype = ISO_IEC_H264;
        ts_msc->atype = ISO_IEC_13818_7_AUDIO;
    }

    ts_buf_reset(local->ts_buf);

    LOG_STRM_PRINTF("STRM_STATE_IPTV\n");
    local_state(local, STRM_STATE_IPTV, 1);
    ind_timer_create(local->tlink, clk + INTERVAL_CLK_1000MS, INTERVAL_CLK_1000MS, local_sync, local);
    ind_timer_create(local->tlink, clk + INTERVAL_CLK_100MS, INTERVAL_CLK_100MS, local_push, local);

    codec_mosaic_open( );

    return 0;
}

static void local_close_play(LOCAL_t local)
{
    int i;
    MscElem_t elem_array, elem;

    LOG_STRM_PRINTF("\n");

    elem_array = local->elem_array;
    for (i = 0; i < MOSAIC_NUM; i ++) {
        elem = &elem_array[i];
        if (elem->ts_mosaic.key >= 0)
            local_mosaic_close(local, elem->ts_mosaic.key);
        if (elem->ts_mosaic.key >= 0)
            LOG_STRM_ERROR("#%d local_mosaic_close\n", i);
    }

    codec_mosaic_close( );

    LOG_STRM_PRINTF("STRM_STATE_CLOSE\n");
    local_state(local, STRM_STATE_CLOSE, 0);
}

static void local_cmd(LOCAL_t local, StreamCmd* strmCmd)
{
    int cmd = strmCmd->cmd;

    if (stream_deal_cmd(local->index, strmCmd) == 1)
        return;

    switch(cmd) {
    case STREAM_CMD_INTERNAL:
        {
            int key = strmCmd->arg1;

            switch(strmCmd->arg0) {
            case INT_STREAM_CMD_SETKEY:
                LOG_STRM_PRINTF("INT_STREAM_CMD_SETKEY key = %d\n", key);
                codec_mosaic_set(key);
                break;
            case INT_STREAM_CMD_OPEN:
                LOG_STRM_PRINTF("INT_STREAM_CMD_OPEN key = %d\n", key);
                local_mosaic_open(local, key);
                break;
            case INT_STREAM_CMD_CLOSE:
                LOG_STRM_PRINTF("INT_STREAM_CMD_CLOSE key = %d\n", key);
                local_mosaic_close(local, key);
                break;
            default:
                LOG_STRM_ERROUT("Unkown internal CMD %d\n", strmCmd->arg0);
            }
        }
        break;
    case STREAM_CMD_CLOSE:
        LOG_STRM_PRINTF("STREAM_CMD_CLOSE\n");
        local_close_play(local);
        break;

    default:
        LOG_STRM_ERROUT("Unkown CMD %d\n", cmd);
    }

Err:
    return;
}

static void local_recv(LOCAL_t local, int idx)
{
    int len, hdr;
    ts_psi_t psi;
    MscElem_t elem;
    ts_mosaic_t ts_msc;

    uint8_t *buf = (uint8_t *)local->rtp_buf;

    elem = &local->elem_array[idx];

    ts_msc = &elem->ts_mosaic;

    len = recv(elem->mult_sock, buf, RTP_BUFFER_LENGTH, 0);
    if (g_msc_enable == 0)//测试使用
        return;

    hdr = ind_rtp_parse((char *)buf, len, NULL);
    if (hdr < 0)
        LOG_STRM_ERROUT("#%d key = %d ind_rtp_header\n", idx, ts_msc->key);
    len -= hdr;
    buf += hdr;

    if (elem->fp)
        fwrite(buf, len, 1, elem->fp);

    if (elem->clk == 0) {
        LOG_STRM_PRINTF("#%d key = %d MOSAIC_MSG_RESUME\n", idx, ts_msc->key);
        local_message(elem, MOSAIC_MSG_RESUME, 0);
    }

    elem->clk = local->clk;

    psi = &elem->ts_psi;

    if (ts_parse_psi(elem->ts_parse, buf, len, NULL) == 1) {
        if (elem->ts_psi_flag == 1) {
            if (ts_psi_equal(psi, &elem->ps_psi) == 0)
                elem->ts_psi_excep ++;
            else
                elem->ts_psi_excep = 0;
        }

        if (elem->ts_psi_flag == 0 || elem->ts_psi_excep >= 5) {

            ts_psi_copy(psi, &elem->ps_psi);
            elem->ts_psi_excep = 0;

            ts_msc->pcr = MOSIC_PCR_PID + idx;
            if (psi->video_pid) {
                ts_msc->vtype = psi->video_type;
                if (psi->pcr_pid == psi->video_pid)
                    ts_msc->pcr = ts_msc->vpid;
            }

            if (psi->audio_num) {
                ts_msc->atype = psi->audio_type[0];
                if (psi->pcr_pid == psi->audio_pid[0])
                    ts_msc->pcr = ts_msc->apid;
            }

            if (psi->video_pid)
                ts_msc->vtype = psi->video_type;

            if (psi->audio_num)
                ts_msc->atype = psi->audio_type[0];

            if (psi->ecm_num > 0)
                ts_msc->ts_ca = psi->ecm_array[0];
            else
                ts_msc->ts_ca.system_id = 0;

            LOG_STRM_PRINTF("#%d key = %d video = %d / %d, system_id = %04x\n", idx, ts_msc->key, ts_msc->vpid, ts_msc->vtype, (uint32_t)ts_msc->ts_ca.system_id);

            if (elem->ts_psi_flag == 0) {
                codec_mosaic_elem_open(ts_msc);

                LOG_STRM_PRINTF("#%d key = %d MOSAIC_MSG_SUCCESS\n", idx, ts_msc->key);
                local_message(elem, MOSAIC_MSG_SUCCESS, 0);
                elem->ts_psi_flag = 1;

                if (g_ca_clk)
                    g_ca_clk = local->clk + FIST_CA_CLK;
            }
        }
    }
    if (0 == elem->ts_psi_flag)
        return;

    if (ts_msc->ts_ca.system_id) {
        len = codec_mosaic_decript(ts_msc->key, (char *)buf, len);
        if (len <= 0 || len % 188)
            return;
        if (g_ca_clk) {
            if (g_ca_clk > local->clk)
                return;
            LOG_STRM_PRINTF("key = %d first CA end!\n", ts_msc->key);
            g_ca_clk = 0;
        }
    }

    {
        char *b;
        int l;
        uint32_t pid;

        l = ts_buf_length(local->ts_buf);
        if (l + len > ts_buf_size(local->ts_buf)) {
            LOG_STRM_WARN("key = %d ########: OVERFLOW!\n", ts_msc->key);
            ts_buf_reset(local->ts_buf);
        }

        for (; len >= 188; buf += 188, len -= 188) {
            pid =(((unsigned int)buf[1] & 0x1f) << 8) + buf[2];

            if (pid == 0)
                continue;
            if (pid == psi->video_pid)
                pid = ts_msc->vpid;
            else if (psi->audio_num > 0 && pid == psi->audio_pid[0])
                pid = ts_msc->apid;
            else if (pid == psi->pcr_pid)
                pid = ts_msc->pcr;
            else
                continue;

            buf[1] = (buf[1] & 0xe0) | (unsigned char)(pid >> 8);
            buf[2] = (unsigned char)pid;

            ts_buf_write_get(local->ts_buf, &b, &l);
            if (l < 188)
                LOG_STRM_ERROUT("key = %d l = %d\n", ts_msc->key, l);
            IND_MEMCPY(b, buf, 188);
            ts_buf_write_put(local->ts_buf, 188);
        }
    }

Err:
    return;
}


static void local_loop(LOCAL_t local)
{
    int i, sock;
    MscElem_t elem_array, elem;

    uint32_t            clk, clks, out;
    fd_set            rset;
    struct timeval    tv;

    elem_array = local->elem_array;

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
        for (i = 0; i < MOSAIC_NUM; i ++) {
            sock = elem_array[i].mult_sock;
            if (sock >= 0)
                FD_SET((uint32_t)sock, &rset);
        }

        if (select(local->maxfd, &rset , NULL,  NULL, &tv) <= 0)
            continue;

        if (FD_ISSET((uint32_t)local->msgfd, &rset)) {
            StreamCmd strmCmd;

            memset(&strmCmd, 0, sizeof(strmCmd));
            mid_msgq_getmsg(local->msgq, (char *)(&strmCmd));
            local_cmd(local, &strmCmd);
            continue;
        }

        for (i = 0; i < MOSAIC_NUM; i ++) {
            elem = &elem_array[i];

            if (elem->mult_sock >= 0 && FD_ISSET((uint32_t)elem->mult_sock, &rset))
                local_recv(local, i);
        }
    }
    ind_timer_delete_all(local->tlink);
}

static LOCAL_t local_create(int idx, mid_msgq_t msgq)
{
    int i;
    LOCAL_t local;
    MscElem_t elem_array, elem;

    local = (LOCAL_t)IND_MALLOC(sizeof(LOCAL));
    if (local == NULL)
        LOG_STRM_ERROUT("malloc failed!\n");
    IND_MEMSET(local, 0, sizeof(LOCAL));

    local->state = STRM_STATE_CLOSE;

    local->ts_buf = ts_buf_create(STREAM_BLOCK_LEVEL_CACHE);
    if (!local->ts_buf)
        LOG_STRM_ERROUT("ts_buf_create size = %d!\n", STREAM_BLOCK_LEVEL_CACHE);

    elem_array = local->elem_array;
    for (i = 0; i < MOSAIC_NUM; i ++) {
        elem = &elem_array[i];
        elem->ts_parse = ts_parse_create(&elem->ps_psi);
    }

    local->index = idx;
    local->msgq = msgq;
    local->msgfd = mid_msgq_fd(local->msgq);
    local->tlink = int_stream_tlink(idx);

    return local;
Err:
    if (local)
        IND_FREE(local);
    return NULL;
}

static void local_delete(LOCAL_t local)
{
    int i;
    MscElem_t elem_array;

    if (local == NULL)
        return;

    elem_array = local->elem_array;
    for (i = 0; i < MOSAIC_NUM; i ++)
        ts_parse_delete(elem_array[i].ts_parse);

    if (local->ts_buf)
        ts_buf_delete(local->ts_buf);

    IND_FREE(local);
}

static void local_loop_play(void *handle, int idx, mid_msgq_t msgq, PlayArg *arg, char *argbuf)
{
    LOCAL_t local = local_create(idx, msgq);
    if (local == NULL)
        LOG_STRM_ERROUT("#%d local is NULL\n", idx);

    if (local_open_play(local, arg))
        LOG_STRM_ERROUT("#%d pvr_recplay_local_open\n", idx);

    local_loop(local);
    LOG_STRM_PRINTF("#%d exit local_loop!\n", idx);

Err:
    local_delete(local);
    return;
}

static int local_argparse_play(int idx, PlayArg* arg, char *argbuf, const char* url, int shiftlen, int begin, int end)
{
    return 0;
}

int mosaic_create_stream(StreamCtrl *ctrl)
{
    if (g_mutex)
        LOG_STRM_ERROUT("already inited\n");
    g_mutex = mid_mutex_create( );

    g_mscarg = (MosaicArg*)IND_CALLOC(sizeof(MosaicArg),1);

    ctrl->handle = ctrl;

    ctrl->loop_play = local_loop_play;
    ctrl->loop_record = NULL;

    ctrl->argsize = 0;
    ctrl->argparse_play = local_argparse_play;

    return 0;
Err:
    return -1;
}

void mid_stream_mosaic_set(int key)
{
    LOG_STRM_PRINTF("key = %d\n", key);

    g_msc_key = key;

    int_stream_cmd(STREAM_CMD_INTERNAL, INT_STREAM_CMD_SETKEY, key, 0, 0);
}

int mid_stream_mosaic_get(void)
{
    return g_msc_key;
}

void mid_stream_mosaic_mute(int mute)
{
    LOG_STRM_PRINTF("mute = %d\n", mute);
    if (mute == 1 || mute == 0)
        g_msc_mute = mute;
}

void mid_stream_mosaic_enable(int enable)
{
    LOG_STRM_PRINTF("enable = %d\n", enable);
    g_msc_enable = enable;
}

void mid_stream_mosaic_igmp(MultiPlayCall play_call, MultiStopCall stop_call)
{
    mid_mutex_lock(g_mutex);
    g_mscarg->igmp_play_call = play_call;
    g_mscarg->igmp_stop_call = stop_call;
    mid_mutex_unlock(g_mutex);
}



void mid_stream_mosaic_setcall(int key, StrmMsgCall msgcall, int callarg)
{
    LOG_STRM_PRINTF("key = %d, msgcall = %p, callarg = %d\n", key, msgcall, callarg);

    mid_mutex_lock(g_mutex);

    g_msg_key = key;
    g_msg_arg = callarg;
    g_msg_call = msgcall;

    mid_mutex_unlock(g_mutex);
}

int mid_stream_mosaic_open(int key, const char* url, int x, int y, int width, int height, char* igmp, int igmp_size)
{
    int i, k, result, first;
    Mosaic_t msc, msc_array;
    struct ind_rect rect;

    result = -1;
    first = 0;
    if (APP_TYPE_MOSAIC != int_stream_get_apptype( )) {
        mid_stream_open_range(0, NULL, APP_TYPE_MOSAIC, 0, -1, 0);
        first = 1;
    }

    mid_mutex_lock(g_mutex);

    if (key < 0 || igmp_size < 0 || igmp_size > IGMP_INFO_SIZE)
        LOG_STRM_ERROUT("key = %d, igmp_size = %d\n", key, igmp_size);

    msc_array = g_mscarg->msc_array;
    if (first) {
        for (i = 0; i < MOSAIC_NUM; i ++)
            msc_array[i].key = -1;
    }

    LOG_STRM_PRINTF("first = %d, key = %d, rect = (%d, %d, %d, %d), url = %s\n", first, key, x, y, width, height, url);

    //检查key是否重复
    for (i = 0; i < MOSAIC_NUM; i ++) {
        msc = &msc_array[i];
        k = msc->key;
        if (k < 0)
            continue;

        if (key == k) {
            int_stream_cmd(STREAM_CMD_INTERNAL, INT_STREAM_CMD_CLOSE, k, 0, 0);
            msc->key = -1;
        } else {
            rect.left = msc->x;
            rect.top = msc->y;
            rect.right = msc->x + msc->width;
            rect.bottom = msc->y + msc->height;
            if (ind_rect_comp(&rect, x, y, x + width, y + height) != RECT_CMP_BROTHER) {
                int_stream_cmd(STREAM_CMD_INTERNAL, INT_STREAM_CMD_CLOSE, k, 0, 0);
                LOG_STRM_WARN("key = %d MOSAIC_MSG_ERROR_RECT rect[%d] x = %d, y = %d, width = %d, height = %d\n", msc->key, i, msc->x, msc->y, msc->width, msc->height);
                {
                    StrmMsgCall msgcall = msc->msgcall;
                    int callarg = msc->callarg;

                    mid_mutex_unlock(g_mutex);
                    if (msgcall)
                        msgcall(msc->key, MOSAIC_MSG_ERROR_RECT, 0, 0, callarg);
                    else
                        stream_port_message(msc->key, MOSAIC_MSG_ERROR_RECT, 0, 0);
                    mid_mutex_lock(g_mutex);
                }
                msc->key = -1;
            }
        }
    }

    k = 0;
    for (i = 0; i < MOSAIC_NUM; i ++) {
        msc = &msc_array[i];
        k = msc->key;
        if (k < 0)
            break;
    }
    if (k > 0)
        LOG_STRM_ERROUT("too many elems!\n");

    if (strncasecmp(url, "igmp://", 7) != 0 || ind_net_pton((char*)url + 7, &msc->mult_sin) <= 0)
        LOG_STRM_ERROUT("url = %s\n", url);

    msc->key = key;
    msc->x = x;
    msc->y = y;
    msc->width = width;
    msc->height = height;

    if (igmp && igmp_size > 0) {
        IND_MEMCPY(msc->igmp, igmp, igmp_size);
        msc->igmp_size = igmp_size;
    } else {
        msc->igmp_size = 0;
    }
    if (key == g_msg_key) {
        msc->callarg = g_msg_arg;
        msc->msgcall = g_msg_call;
    } else {
        msc->callarg = 0;
        msc->msgcall = NULL;
    }
    g_msg_key = -1;

    int_stream_cmd(STREAM_CMD_INTERNAL, INT_STREAM_CMD_OPEN, key, 0, 0);

    if (first) {
        if (g_msc_mute)
            g_msc_key = -1;
        else
            g_msc_key = key;
        first = 0;
    }
    result = 0;

Err:
    mid_mutex_unlock(g_mutex);

    if (first)
        mid_stream_close(0, 0);
    return result;
}

void mid_stream_mosaic_close(int key)
{
    Mosaic_t msc_array = g_mscarg->msc_array;
    int i;

    LOG_STRM_PRINTF("key = %d / %d, %d, %d / %d, %d, %d / %d, %d, %d\n", key,
        msc_array[0].key, msc_array[1].key, msc_array[2].key,
        msc_array[3].key, msc_array[4].key, msc_array[5].key,
        msc_array[6].key, msc_array[7].key, msc_array[8].key);

    mid_mutex_lock(g_mutex);

    for (i = 0; i < MOSAIC_NUM; i ++) {
        if (-1 == key || key == msc_array[i].key) {
            msc_array[i].key = -1;
            if (key >= 0) {
                int_stream_cmd(STREAM_CMD_INTERNAL, INT_STREAM_CMD_CLOSE, key, 0, 0);
                break;
            }
        }
    }

    mid_mutex_unlock(g_mutex);

    for (i = 0; i < MOSAIC_NUM; i ++) {
        if (msc_array[i].key >= 0)
            break;
    }
    if (-1 == key || i >= MOSAIC_NUM)
        mid_stream_close(0, 0);
}

void mid_stream_mosaic_save(int flag)
{
    LOG_STRM_PRINTF("save flag = %d\n", flag);
    g_save_flg = flag;
}
