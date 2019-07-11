/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/
#include <sys/select.h>

#include "stream.h"
#include "stream_port.h"

#if SUPPORTE_HD == 1
#include "libzebra.h"
#endif

typedef struct {
    int             index; //当前播放进程: 0 PLAY ;1 录制; 2 录制; 3 pip ; 4 vod 带广告

    int             cmdsn; //
    int             backsn; //与rtsp层的cmdsn相同.
    int             seeksn; //如果有seek操作,为cmdsn + 1, seek完成后清零.

    int             apptype; //节目类型 vod 等.
    int             state; //节目播放状态   play pause 等
    int             scale; //节目播放速度
    int             isMusic;

    mid_msgq_t      msgq; //通过pipe传送消息.

    StreamMsgQ*     strm_msgq; //流库内部 消息队列
    ind_tlink_t     tlink; //定时器

    PlayArg         plyarg; //播放回调 /参数
    StreamPlay*     strm_play; //对应功能层参数

#ifdef INCLUDE_PVR
    RecordArg       recarg;
    StreamRecord*   strm_record;
#endif
    char*           argbuf; //存放play->argbuf 内容.

    StreamCtrl      ctrls[STREAM_CTRL_MAX]; //对应的各播放方式对应的控制函数
} STREAM;

typedef struct tagPLAY {
    STREAM*         stream;
    PlayArg         arg;
    char*           argbuf; //play 的流地址
    int             argsize; //对应的argbuf的size值

    uint32_t        play_magic; //对应一个特定的值.以保证上层和stram 操作同一个节目

    uint32_t        rec_id;

    int             new_flag; //控制是否 new_magic 是否增加标志位.目前用处不大,默认为0
    uint32_t        new_magic; //每次切换频道后,new_magic 加1, 作为后续的play_magic值.

    int             open; //执行open操作后,此标志位置1, (close)关闭置0
    int             sync_total; //获取节目时间,open是置1, 获取后置0.

    uint32_t        time_total; //组播表示时移时间(-1表示不支持时移), VOD表示节目实际时间
    uint32_t        time_current; //当前时间
    long long       byte_total; //两个应该是local 播放用的.可不关注 zebra调用
    long long       byte_current;

    StrmStateCall   statecall; //状态(state)回调 外部可重新设置
    StrmMsgCall     msgcall; //msg 回调处理 外部可重新设置
    int             callarg; //上面两个回调的最后一个参数,一般为0

    StrmPrivCall    privcalls[STREAM_CTRL_MAX]; //回调 (国外用)
    int             privargs[STREAM_CTRL_MAX]; //回调对应的最后一个参数
} PLAY;

#ifdef INCLUDE_PVR
typedef struct tagRECORD {
    int             index;//record index
    uint32_t        leader;
    STREAM*         stream;

    RecordArg_t     arg;
    char*           argbuf;
    int             argsize;

    uint32_t        new_magic;
} RECORD;

#endif//INCLUDE_PVR

static uint32_t     g_emm_magic = 0; //只有匈牙利使用, 保存下一个将要播放节目的magic值

static int*         g_ctrls = NULL; //设置播放内容对应的协议 eg:APP_TYPE_VOD 对应 STREAM_CTRL_RTSP.(数组)

static PLAY*        g_play = NULL; //保存当前线程对应的play 结构体(play rec0 rec1)
static mid_mutex_t  g_mutex = NULL;
static mid_mutex_t  g_int_mutex = NULL;

static PLAY*        g_playArray[STREAM_INDEX_NUM]; //保存各线程对应的play 结构体 play rec0 rec1 pip adv

static STREAM*      g_streamArray[STREAM_INDEX_NUM] = {NULL, NULL, NULL, NULL, NULL};
 //保存各线程对应的stream 结构体 play rec0 rec1 pip adv
#ifdef INCLUDE_PVR
static STREAM*      g_idle = NULL;

#define RECORD_NUM  2
static RECORD*      g_records[RECORD_NUM] = {NULL, NULL};

static RecordCall_open  g_rcall_open[RECORD_NUM]    = {NULL, NULL};
static RecordCall_push  g_rcall_push[RECORD_NUM]    = {NULL, NULL};
static RecordCall_close g_rcall_close[RECORD_NUM]   = {NULL, NULL};

static int          g_record_num = 0;
static int          g_record_mix = 1;
#endif//INCLUDE_PVR

static PLAY*        g_pip = NULL; //保存当前线程对应的play 结构体(pip)

static PLAY*        g_play_adv = NULL; //保存当前线程对应的play 结构体(adv-vod带广告)

static char         g_language[4] = {0,}; //语言, 国外使用
static char         g_language_flg = 0; //语言, 国外使用

static int          g_msg_flag = 1; //log 打印级别, telnet设置

//华为要求组播切换保持MQM开启
static int          g_mqm_delay = 0;

#ifdef INCLUDE_PVR
static uint32_t int_record_id(void);
#endif

static void stream_cmd(STREAM *stream, STREAM_CMD cmd, int arg0, int arg1, int arg2, int arg3)
{
    StreamCmd strmCmd;

    strmCmd.cmd = cmd;
    strmCmd.arg0 = arg0;
    strmCmd.arg1 = arg1;
    strmCmd.arg2 = arg2;
    strmCmd.arg3 = arg3;
    mid_msgq_putmsg(stream->msgq, (void*)(&strmCmd));
}

/*
    大窗口消息
 */
int int_stream_cmd(STREAM_CMD cmd, int arg0, int arg1, int arg2, int arg3)
{
    stream_cmd(g_play->stream, cmd, arg0, arg1, arg2, arg3);
    return 0;
}

/*
    正在播放
 */
uint32_t int_stream_playmagic(void)
{
    return g_play->play_magic;
}

/*
    最近打开
 */
uint32_t int_stream_openmagic(void)
{
    return g_play->arg.magic;
}

uint32_t int_stream_nextmagic(void)
{
    uint32_t magic;

    mid_mutex_lock(g_mutex);

    if (g_play->new_flag) {
        magic = g_play->new_magic;
    } else {
        magic = g_play->new_magic + 1;
        if (magic == 0)
            magic ++;
    }

    mid_mutex_unlock(g_mutex);

    return magic;
}

mid_mutex_t int_stream_mutex(void)
{
    return g_int_mutex;
}

ind_tlink_t int_stream_tlink(int idx)
{
    STREAM *stream;

    if (idx < 0 || idx >= STREAM_INDEX_NUM)
        LOG_STRM_ERROUT("#%d invalid index\n", idx);

    stream = g_streamArray[idx];
    if (stream == NULL)
        LOG_STRM_ERROUT("#%d stream is NULL\n", idx);
    ind_timer_delete_all(stream->tlink);
    return stream->tlink;
Err:
    return NULL;
}

StreamMsgQ* int_strm_msgq(int idx)
{
    STREAM *stream;

    if (idx < 0 || idx >= STREAM_INDEX_NUM)
        LOG_STRM_ERROUT("#%d invalid index\n", idx);

    stream = g_streamArray[idx];
    if (stream == NULL)
        LOG_STRM_ERROUT("#%d stream is NULL\n", idx);
    strm_msgq_reset(stream->strm_msgq, idx);
    return stream->strm_msgq;
Err:
    return NULL;
}

StreamPlay* int_strm_play(int idx)
{
    STREAM *stream;

    if (idx < 0 || idx >= STREAM_INDEX_NUM)
        LOG_STRM_ERROUT("#%d invalid index\n", idx);

    stream = g_streamArray[idx];
    if (stream == NULL)
        LOG_STRM_ERROUT("#%d stream is NULL\n", idx);
    return stream->strm_play;
Err:
    return NULL;
}

int int_strm_playing(int idx)
{
    if (idx == STREAM_INDEX_PIP) {
        if (g_pip->open)
            return 1;
        return 0;
    }

    if (idx < 0 || idx >= STREAM_INDEX_NUM)
        LOG_STRM_ERROUT("#%d invalid index\n", idx);

    if (g_play->open && g_play->stream == g_streamArray[idx])
        return 1;
Err:
    return 0;
}

#ifdef INCLUDE_PVR
StreamRecord* int_strm_record(int idx)
{
    STREAM *stream;

    if (idx < 0 || idx >= STREAM_INDEX_NUM)
        LOG_STRM_ERROUT("#%d invalid index\n", idx);

    stream = g_streamArray[idx];
    if (stream == NULL)
        LOG_STRM_ERROUT("#%d stream is NULL\n", idx);
    return stream->strm_record;
Err:
    return NULL;
}
#endif//INCLUDE_PVR

static void stream_task(void *handle)
{
#ifdef INCLUDE_PVR
    RecordArg *rec_arg;
#endif
    mid_msgq_t msgq;

    PlayArg *ply_arg;
    char *argbuf;
    StreamCmd strmCmd;
    struct timeval tv;
    fd_set rset;
    int idx, msgfd;
    StreamCtrl *ctrl;
    STREAM *stream = (STREAM *)handle;
    PLAY *play;

    msgq = stream->msgq;
    idx = stream->index;
    msgfd = mid_msgq_fd(msgq);
#if defined(DEBUG_BUILD) && !defined(x86win32)
    printf("stream thread [%d] id = 0x%x pid = %d\n", idx, (uint32_t)pthread_self(), (uint32_t)getpid());
#endif

    while(1) {
        FD_ZERO(&rset);
        FD_SET((uint32_t)msgfd, &rset);
        tv.tv_sec = 3600 * 24 * 356;
        tv.tv_usec = 0;
        if (select(msgfd + 1, &rset , NULL,  NULL, &tv) != 1)
            continue;

        memset(&strmCmd, 0, sizeof(strmCmd));
        mid_msgq_getmsg(msgq, (char *)(&strmCmd));

        if (stream_deal_cmd(idx, &strmCmd) == 1)
            continue;

        switch(strmCmd.cmd) {
        case STREAM_CMD_OPEN:
            if (stream_back_get_arg(idx, strmCmd.arg0, &ply_arg, &argbuf)) {
                LOG_STRM_WARN("#%d stream_back_get_arg\n", stream->index);
                break;
            }
            if (idx != STREAM_INDEX_PIP && g_mqm_delay && ply_arg->apptype != APP_TYPE_IPTV && ply_arg->apptype != APP_TYPE_TSTV)
                stream_post_datasock(0, 0);
#if !defined(x86win32)  // cannot convert pthread_t to unsigned int using win32 pthread library.
            LOG_STRM_PRINTF("#%d STREAM_CMD_OPEN tid = 0x%x, apptype = %d\n", stream->index, (uint32_t)pthread_self(), ply_arg->apptype);
#endif
            ctrl = &stream->ctrls[g_ctrls[ply_arg->apptype]];
            stream->apptype = ply_arg->apptype;

            if (idx < STREAM_INDEX_PIP) {
                mid_mutex_lock(g_mutex);

                if (g_emm_magic && g_emm_magic == ply_arg->magic)
                    codec_emm(0, 1);
                else
                    codec_emm(0, 0);
                g_emm_magic = 0;

                mid_mutex_unlock(g_mutex);

                strm_play_resize(stream->strm_play, stream->index, RTP_BUFFER_LENGTH, STREAM_BLOCK_LEVEL_PLAY);
            }

            strm_play_leader_set(stream->strm_play, stream->index);
            stream->state = STRM_STATE_OPEN;
            ctrl->loop_play(ctrl->handle, idx, msgq, ply_arg, argbuf);
            stream->state = STRM_STATE_CLOSE;
            break;

        case STREAM_CMD_CLOSE:
            LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE arg0 = %d\n", idx, strmCmd.arg0);
            if (idx != STREAM_INDEX_PIP && g_mqm_delay && strmCmd.arg0)
                stream_post_datasock(0, 0);
            break;

        #ifdef INCLUDE_PVR
        case STREAM_CMD_RECORD_OPEN:
            LOG_STRM_PRINTF("#%d STREAM_CMD_RECORD_OPEN\n", idx);

            if (record_back_get_arg(idx, strmCmd.arg0, &rec_arg, &argbuf) == 0) {
                LOG_STRM_PRINTF("#%d apptype = %d\n", stream->index, rec_arg->apptype);
                ctrl = &stream->ctrls[g_ctrls[rec_arg->apptype]];
                ctrl->loop_record(ctrl->handle, idx, msgq, rec_arg, argbuf);
            }
            break;
        #endif//INCLUDE_PVR

        case STREAM_CMD_WAIT_RECORD:
            LOG_STRM_PRINTF("#%d STREAM_CMD_WAIT_RECORD\n", idx);
            stream_back_wake(0);
            break;

#if SUPPORTE_HD == 1
        case STREAM_CMD_WAIT_MOSAIC:
            LOG_STRM_PRINTF("#%d STREAM_CMD_WAIT_MOSAIC\n", idx);
            mid_mutex_lock(g_mutex);

            if (stream->index == STREAM_INDEX_PIP)
                play = g_play;
            else
                play = g_pip;
            stream_cmd(play->stream, STREAM_CMD_OPEN, strmCmd.arg0, 0, 0, 0);

            mid_mutex_unlock(g_mutex);
            break;
        case STREAM_CMD_WAIT_FLASH:
            LOG_STRM_PRINTF("#%d STREAM_CMD_WAIT_FLASH\n", idx);
            mid_mutex_lock(g_mutex);

            if (stream->index == STREAM_INDEX_PIP)
                play = g_play;
            else
                play = g_pip;
            stream_cmd(play->stream, STREAM_CMD_OPEN, strmCmd.arg0, 0, 0, 0);

            mid_mutex_unlock(g_mutex);
            break;
#endif

        default:
            LOG_STRM_WARN("#%d not deal with message %d\n", idx, strmCmd.cmd);
            break;
        }
    }
}

static STREAM* stream_create(int idx)
{
    STREAM *stream;

    LOG_STRM_PRINTF("#%d\n", idx);

    stream = (STREAM *)IND_MALLOC(sizeof(STREAM));
    if (stream == NULL)
        LOG_STRM_ERROUT("#%d malloc\n", idx);
    IND_MEMSET(stream, 0, sizeof(STREAM));

    stream->msgq = mid_msgq_create(50, sizeof(StreamCmd));
    stream->index = idx;

    stream->tlink = ind_tlink_create(10);
    stream->strm_msgq = strm_msgq_create(10);

    if (STREAM_INDEX_PLAY == idx)
        stream->strm_play = strm_play_create(0, STREAM_BLOCK_LEVEL_PLAY);
    else if (STREAM_INDEX_PIP == idx)
        stream->strm_play = strm_play_create(1, STREAM_BLOCK_LEVEL_PIP);
    else
        stream->strm_play = g_streamArray[STREAM_INDEX_PLAY]->strm_play;

#ifdef INCLUDE_PVR
    if (idx < STREAM_INDEX_PIP)
        stream->strm_record = strm_record_create(idx, STREAM_BLOCK_LEVEL_RECORD);
#endif

    g_streamArray[idx] = stream;

    return stream;
Err:
    return NULL;
}

static PLAY* stream_create_play(int idx)
{
    PLAY *play;

    play = (PLAY*)IND_MALLOC(sizeof(PLAY));
    IND_MEMSET(play, 0, sizeof(PLAY));

    play->stream = stream_create(idx);

    stream_port_task_create(stream_task, play->stream);

    return play;
}

#ifdef INCLUDE_PVR
static RECORD* stream_create_record(int idx)
{
    RECORD *record;

    record = (RECORD*)IND_MALLOC(sizeof(RECORD));
    IND_MEMSET(record, 0, sizeof(RECORD));

    record->stream = stream_create(idx);

    stream_port_task_create(stream_task, record->stream);

    return record;
}
#endif

void mid_stream_buildtime(void)
{
    LOG_STRM_PRINTF("buildtime : "__DATE__" "__TIME__" : stream\n");
}

static int stream_ctrl_attach(PLAY* play, int ctrl_index, ctrl_create_f ccreate)
{
    StreamCtrl* ctrl;

    ctrl = &play->stream->ctrls[ctrl_index];
    if (ctrl->handle)
        LOG_STRM_ERROUT("ctrlindex = %d already create\n", ctrl_index);

#ifdef INCLUDE_PVR
    ccreate(ctrl);
#else
    ccreate(ctrl);
#endif
    if (ctrl->handle == NULL)
        LOG_STRM_ERROUT("ctrlindex = %d failed create\n", ctrl_index);
    if (ctrl->argsize > play->argsize) {
        if (play->argbuf)
            IND_FREE(play->argbuf);
        play->argbuf = IND_MALLOC(ctrl->argsize);
        if (play->stream->argbuf)
            IND_FREE(play->stream->argbuf);
        play->stream->argbuf = IND_MALLOC(ctrl->argsize);
        play->argsize = ctrl->argsize;
    }

    return 0;
Err:
    return -1;
}

int stream_ctrl_create(int ctrl_index, int playrecord, int enablepip, ctrl_create_f ccreate)
{
#ifdef INCLUDE_PVR
    int i;
#endif

    if (ctrl_index <= 0 || ctrl_index >= STREAM_CTRL_MAX)
        LOG_STRM_ERROUT("ctrl_index = %d\n", ctrl_index);

    switch(playrecord) {
    case PLAYRECORD_PLAY:
        stream_ctrl_attach(g_play, ctrl_index, ccreate);
#ifdef INCLUDE_PVR
        for (i = 0; i < g_record_num; i ++)
            g_records[i]->stream->ctrls[ctrl_index] = g_play->stream->ctrls[ctrl_index];
#endif
        break;

    case PLAYRECORD_COMPREHENSIVE:
        stream_ctrl_attach(g_play, ctrl_index, ccreate);

#ifdef INCLUDE_PVR
        for (i = 0; i < g_record_num; i ++) {
            StreamCtrl *ctrl = &g_records[i]->stream->ctrls[ctrl_index];
            if (ctrl->handle)
                LOG_STRM_ERROUT("ctrl_index = %d already create\n", ctrl_index);

            ccreate(ctrl);
            if (ctrl->handle == NULL)
                LOG_STRM_ERROUT("ctrlindex = %d failed create [%d]\n", ctrl_index, i);

            if (ctrl->argsize > g_records[i]->argsize) {
                if (g_records[i]->argbuf)
                    IND_FREE(g_records[i]->argbuf);
                g_records[i]->argbuf = IND_MALLOC(ctrl->argsize);
                if (g_records[i]->stream->argbuf)
                    IND_FREE(g_records[i]->stream->argbuf);
                g_records[i]->stream->argbuf = IND_MALLOC(ctrl->argsize);
                g_records[i]->argsize = ctrl->argsize;
            }
        }
#endif
        break;
    default: //包括 PLAYRECRD_RECORD:
        LOG_STRM_ERROUT("playrecord = %d\n", playrecord);
    }

    if (enablepip)
        stream_ctrl_attach(g_pip, ctrl_index, ccreate);

    return 0;
Err:
    return -1;
}

void mid_stream_init(int num)
{
    int i;
    int pip;

    pip = 1;
    LOG_STRM_PRINTF("pip = %d\n", pip);

    mid_stream_buildtime( );

    if (g_mutex)
        LOG_STRM_ERROUT("already inited\n");

    for (i = 0; i < STREAM_INDEX_NUM; i ++)
        g_streamArray[i] = NULL;

    g_mutex = mid_mutex_create( );
    g_int_mutex = mid_mutex_create( );

    LOG_STRM_PRINTF("num = %d\n", num);

    g_ctrls = (int*)IND_MALLOC(sizeof(int) * APP_TYPE_MAX);

    g_ctrls[APP_TYPE_VOD        ] = STREAM_CTRL_RTSP;
    g_ctrls[APP_TYPE_VODADV     ] = STREAM_CTRL_RTSP;
    g_ctrls[APP_TYPE_IPTV       ] = STREAM_CTRL_RTSP;
    g_ctrls[APP_TYPE_IPTV2      ] = STREAM_CTRL_RTSP;
    g_ctrls[APP_TYPE_HTTP       ] = STREAM_CTRL_HTTP;
    g_ctrls[APP_TYPE_PVR        ] = STREAM_CTRL_PVR;
    g_ctrls[APP_TYPE_AUDIO      ] = STREAM_CTRL_AUDIO;
    g_ctrls[APP_TYPE_TSTV       ] = STREAM_CTRL_RTSP;
    g_ctrls[APP_TYPE_DVBS       ] = STREAM_CTRL_DVBS;
    g_ctrls[APP_TYPE_MOSAIC     ] = STREAM_CTRL_MOSAIC;
    g_ctrls[APP_TYPE_FLASH      ] = STREAM_CTRL_FLASH;
    g_ctrls[APP_TYPE_DISK       ] = STREAM_CTRL_DISK;
    g_ctrls[APP_TYPE_RTP2TS     ] = STREAM_CTRL_RTP2TS;
    g_ctrls[APP_TYPE_MIX_PCM    ] = STREAM_CTRL_MIX_PCM;
    g_ctrls[APP_TYPE_MIX_MP3    ] = STREAM_CTRL_MIX_MP3;
    g_ctrls[APP_TYPE_HTTP_PCM   ] = STREAM_CTRL_HTTP_PCM;
    g_ctrls[APP_TYPE_HTTP_MP3   ] = STREAM_CTRL_HTTP_MP3;
    g_ctrls[APP_TYPE_ZEBRA      ] = STREAM_CTRL_ZEBRA;
    g_ctrls[APP_TYPE_ZEBRA_PCM  ] = STREAM_CTRL_ZEBRA_PCM;
    g_ctrls[APP_TYPE_HTTP_MPA   ] = STREAM_CTRL_HTTP_MPA;
    g_ctrls[APP_TYPE_HLS        ] = STREAM_CTRL_HTTP;
    g_ctrls[APP_TYPE_HTTP_LIVE  ] = STREAM_CTRL_HTTP_LIVE;
    g_ctrls[APP_TYPE_APPLE_VOD  ] = STREAM_CTRL_APPLE;
    g_ctrls[APP_TYPE_APPLE_IPTV ] = STREAM_CTRL_APPLE;

    g_play = stream_create_play(STREAM_INDEX_PLAY);
    g_playArray[STREAM_INDEX_PLAY] = g_play;
    g_playArray[STREAM_INDEX_REC0] = g_play;
    g_playArray[STREAM_INDEX_REC1] = g_play;

    g_pip = stream_create_play(STREAM_INDEX_PIP);
    g_playArray[STREAM_INDEX_PIP] = g_pip;

#ifdef INCLUDE_PVR
    g_records[0] = stream_create_record(STREAM_INDEX_REC0);
    g_record_num = 1;
    if (num == 3) {
        g_records[1] = stream_create_record(STREAM_INDEX_REC1);
        g_record_num = 2;
    }
#endif

    stream_ctrl_create(STREAM_CTRL_RTSP,    PLAYRECORD_COMPREHENSIVE,   pip,  rtsp_create_stream);
    stream_ctrl_create(STREAM_CTRL_HTTP,    PLAYRECORD_COMPREHENSIVE,   0,    http_create_stream);

#if SUPPORTE_HD == 1
    stream_ctrl_create(STREAM_CTRL_APPLE,   PLAYRECORD_PLAY,            0,    apple_create_stream);
#endif

    stream_ctrl_create(STREAM_CTRL_AUDIO,   PLAYRECORD_PLAY,            0,    audio_create_stream);
    stream_ctrl_create(STREAM_CTRL_MIX_PCM, PLAYRECORD_PLAY,            1,    mix_pcm_create_stream);
    stream_ctrl_create(STREAM_CTRL_HTTP_PCM,PLAYRECORD_PLAY,            1,    http_pcm_create_stream);
#ifdef INCLUDE_MP3
    stream_ctrl_create(STREAM_CTRL_MIX_MP3, PLAYRECORD_PLAY,            1,    mix_mp3_create_stream);
    stream_ctrl_create(STREAM_CTRL_HTTP_MP3,PLAYRECORD_PLAY,            1,    http_mp3_create_stream);
#endif
    stream_ctrl_create(STREAM_CTRL_HTTP_MPA,PLAYRECORD_PLAY,            0,    http_mpa_create_stream);

#if SUPPORTE_HD == 1
    stream_ctrl_create(STREAM_CTRL_MOSAIC,  PLAYRECORD_PLAY,            0,    mosaic_create_stream);
    stream_ctrl_create(STREAM_CTRL_ZEBRA,   PLAYRECORD_PLAY,            0,    zebra_create_stream);
    stream_ctrl_create(STREAM_CTRL_ZEBRA_PCM,  PLAYRECORD_PLAY,         0,    zebra_pcm_create_stream);
#endif
#ifdef INCLUDE_FLASHPLAY
    stream_ctrl_create(STREAM_CTRL_FLASH,   PLAYRECORD_PLAY,            0,    flash_create_stream);
#endif
#ifdef INCLUDE_PVR
    stream_ctrl_create(STREAM_CTRL_PVR,     PLAYRECORD_PLAY,            0,    pvr_create_stream);
#endif

#ifdef INCLUDE_RTP2TS
    stream_ctrl_create(STREAM_CTRL_RTP2TS,  PLAYRECORD_PLAY,            pip,  rtp2ts_create_stream);
#endif
#ifdef ENABLE_DISK
    stream_ctrl_create(STREAM_CTRL_DISK,    PLAYRECORD_PLAY,            0,    disk_create_stream);
#endif
#ifdef INCLUDE_DVBS
    stream_ctrl_create(STREAM_CTRL_DVBS,    PLAYRECORD_COMPREHENSIVE,   pip,  dvbs_create_stream);
#endif

    //广告播放
    g_play_adv = stream_create_play(STREAM_INDEX_ADV);

    g_playArray[STREAM_INDEX_ADV] = g_play_adv;

    stream_ctrl_attach(g_play_adv, STREAM_CTRL_RTSP, rtsp_create_stream);

#ifdef INCLUDE_PVR
    mid_record_init( );
    //异步处理PVR
    strm_async_init( );
#endif

Err:
    return;
}

void stream_post_datasock(int idx, int delay)
{
    LOG_STRM_PRINTF("delay = %d / %d\n", delay, g_mqm_delay);
    if (delay) {
        g_mqm_delay = 1;
    } else if (g_mqm_delay) {
        g_mqm_delay = 0;
        stream_port_post_datasock(0, -1, NULL, NULL, NULL);
    }
}

void stream_post_msg(int idx, STRM_MSG msgno, int arg)
{
    uint32_t clk, diff;
    STREAM *stream = g_streamArray[idx];

    switch(idx) {
    case STREAM_INDEX_PLAY:
    case STREAM_INDEX_REC0:
    case STREAM_INDEX_REC1:
        LOG_STRM_DEBUG("#%d msgno = %d, arg = %d, magic = %d, msg_flag = %d, msgcall = %p\n", idx, msgno, arg, g_play->play_magic, g_msg_flag, stream->plyarg.msgcall);
        if (0 == g_msg_flag)
            return;
        if (stream != g_play->stream) {
            LOG_STRM_WARN("#%d overdue!\n", idx);
            return;
        }

        if (STRM_MSG_PTS_VIEW == msgno) {
            mid_mutex_lock(g_mutex);

            if (g_play->open && g_play->play_magic == g_play->arg.magic) {
                g_play->open = 2;
                LOG_STRM_DEBUG("#%d play_open 2\n", idx);
            }

            mid_mutex_unlock(g_mutex);
        } else if (STRM_MSG_STREAM_MUSIC == msgno) {
            stream->isMusic = 1;
        } else if (STRM_MSG_STREAM_VIDEO == msgno) {
            stream->isMusic = 0;
        }

        clk = mid_10ms( );

        if (stream->plyarg.msgcall)
            stream->plyarg.msgcall(0, msgno, arg, g_play->play_magic, stream->plyarg.callarg);
        else
            stream_port_message(0, msgno, arg, g_play->play_magic);
        diff = mid_10ms( ) - clk;
        if (diff > 10)
            LOG_STRM_ERROR("#%d msgno = %d, diff = %d\n", idx, msgno, diff);
        break;
    case STREAM_INDEX_PIP:
        LOG_STRM_DEBUG("#%d msgno = %d, arg = %d, magic = %d, msg_flag = %d, msgcall = %p\n", idx, msgno, arg, g_pip->play_magic, g_msg_flag, stream->plyarg.msgcall);
        if (g_msg_flag == 0)
            return;
        if (stream->plyarg.msgcall)
            stream->plyarg.msgcall(1, msgno, arg, g_pip->play_magic, stream->plyarg.callarg);
        else
            stream_port_message(1, msgno, arg, g_pip->play_magic);
        break;
    default:
        break;
    }
    LOG_STRM_DEBUG("#%d msgno = %d, arg = %d\n", idx, msgno, arg);
}

static STREAM* stream_dispatch(int idx, int* psn)
{
    STREAM *stream;

    if (psn)
        *psn = 0;
    switch(idx) {
    case 0:
        mid_mutex_lock(g_mutex);

        stream = g_play->stream;
        if (psn && g_play->play_magic == g_play->arg.magic) {
            stream->cmdsn ++;
            *psn = stream->cmdsn;
        }
        mid_mutex_unlock(g_mutex);
        break;
    case 1:
        stream = g_pip->stream;
        break;
    case STREAM_INDEX_ADV:
        if (g_play_adv == NULL)
            LOG_STRM_ERROUT("#%d play is NULL\n", idx);
        stream = g_play_adv->stream;
        break;
    default:
        LOG_STRM_ERROUT("index = %d\n", idx);
    }

    return stream;
Err:
    return NULL;
}

int stream_deal_cmd(int idx, StreamCmd* strmCmd)
{
    int cmd = strmCmd->cmd;
    StreamPlay*    strm_play;

    mid_mutex_lock(g_mutex);

    if (STREAM_INDEX_PIP == idx)
        strm_play = g_pip->stream->strm_play;
    else
        strm_play = g_play->stream->strm_play;

    mid_mutex_unlock(g_mutex);

    switch(cmd) {
    case STREAM_CMD_RECT:
        LOG_STRM_PRINTF("#%d RECT %d %d %d %d\n", idx, strmCmd->arg0, strmCmd->arg1, strmCmd->arg2, strmCmd->arg3);
        strm_play_rect(strm_play, idx, strmCmd->arg0, strmCmd->arg1, strmCmd->arg2, strmCmd->arg3);
        return 1;

    default:
        break;
    }

    return 0;
}

void stream_post_state(int idx, STRM_STATE state, int scale)
{
    STREAM *stream = g_streamArray[idx];

    LOG_STRM_DEBUG("#%d state = %d scale = %d, msg_flag = %d\n", idx, state, scale, g_msg_flag);
    if (g_msg_flag == 0)
        return;

    switch(idx) {
    case STREAM_INDEX_PLAY:
    case STREAM_INDEX_REC0:
    case STREAM_INDEX_REC1:
        stream->state = state;
        stream->scale = scale;

        if (stream != g_play->stream) {
            LOG_STRM_WARN("#%d overdue!\n", idx);
            return;
        }

        if (stream->plyarg.statecall)
            stream->plyarg.statecall(0, state, scale, g_play->play_magic, stream->plyarg.callarg);
        else
            stream_port_state(0, state, scale, g_play->play_magic);
        break;
    case STREAM_INDEX_PIP:
        if (stream->plyarg.statecall)
            stream->plyarg.statecall(1, state, scale, g_pip->play_magic, stream->plyarg.callarg);
        else
            stream_port_state(1, state, scale, g_pip->play_magic);
        break;

    default:
        break;
    }
}

static void stream_close_play(int clear)
{
    LOG_STRM_DEBUG("STREAM_CMD_CLOSE\n");
    stream_cmd(g_play->stream, STREAM_CMD_CLOSE, clear, 0, 0, 0);
#ifdef INCLUDE_PVR
    if (g_idle) {
        LOG_STRM_PRINTF("#%d demix play!\n", g_idle->index);
        g_play->stream = g_idle;
        g_idle = NULL;
    }
#endif
}

static PLAY* int_stream_play(int idx)
{
    switch (idx) {
    case 0:
        return g_play;
    case 1:
        return g_pip;
    default:
        return NULL;
    }
}

unsigned int mid_stream_magic(int idx)
{
    unsigned int magic;
    PLAY* play = int_stream_play(idx);

    if (!play)
        return 0;

    LOG_STRM_PRINTF("#%d\n", idx);

    mid_mutex_lock(g_mutex);

    play->new_magic ++;
    if (play->new_magic == 0)
        play->new_magic ++;
    play->new_flag = 1;

    magic = play->new_magic;

    mid_mutex_unlock(g_mutex);

    return magic;
}

uint32_t mid_stream_open_range(int idx, const char* url, APP_TYPE apptype, int shiftlen, int begin, int end)
{
    PlayArg *arg;
    STREAM *stream;
    StreamCtrl *ctrl;

    uint32_t magic = 0;
    PLAY *play = NULL;
    int ctrl_index, open_delay = 0;

    LOG_STRM_PRINTF("#%d buildtime : "__DATE__" "__TIME__" : stream\n", idx);

    mid_mutex_lock(g_mutex);

    if (1 == idx)
        idx = STREAM_INDEX_PIP;
    play = g_playArray[idx];
    if (play == NULL)
        LOG_STRM_ERROUT("#%d play is NULL\n", idx);

    if (apptype >= APP_TYPE_MAX)
        LOG_STRM_ERROUT("#%d apptype = %d\n", idx, apptype);

    if (0 == idx)
        codec_prepare( );

    arg = &play->arg;
    IND_MEMSET(arg, 0, sizeof(PlayArg));

#ifdef INCLUDE_PVR
    if (APP_TYPE_HLS == apptype || APP_TYPE_PVR == apptype) {
        int_back_hls_bufferrate(0);
        int_back_hls_recordrate(0);
    }

    if (APP_TYPE_HLS == apptype && strm_timeshift_size( ) > 0)
        arg->shiftid = int_record_id( );

    if (APP_TYPE_DVBS == apptype || APP_TYPE_TSTV == apptype)
        arg->shiftid = int_record_id( );
#endif

    ctrl_index = g_ctrls[apptype];

    if (idx == 0) {
        stream_close_play(0);
    } else {
        LOG_STRM_DEBUG("STREAM_CMD_CLOSE\n");
        stream_cmd(play->stream, STREAM_CMD_CLOSE, 0, 0, 0, 0);
    }

    play->open = 1;
    LOG_STRM_DEBUG("#%d play_open 1\n", idx);

    arg->apptype = apptype;

    if (play->new_flag) {
        play->new_flag = 0;
    } else {
        play->new_magic ++;
        if (play->new_magic == 0)
            play->new_magic ++;
    }
    arg->magic = play->new_magic;

    arg->statecall = play->statecall;
    arg->msgcall = play->msgcall;
    arg->callarg = play->callarg;
    arg->privcall = play->privcalls[ctrl_index];
    arg->privarg = play->privargs[ctrl_index];

    stream = play->stream;

    ctrl = &stream->ctrls[ctrl_index];

    if (ctrl->loop_play == NULL || ctrl->argparse_play == NULL)
        LOG_STRM_ERROUT("#%d apptype = %d ctrls = %p, argparse_play = %p\n", idx, apptype, ctrl->loop_play, ctrl->argparse_play);

    if (ctrl->argparse_play(idx, arg, play->argbuf, url, shiftlen, begin, end))
        LOG_STRM_ERROUT("#%d plyarg_parse\n", idx);

    play->rec_id = 0;

#ifdef INCLUDE_PVR
    //复用录制
    if (g_record_mix == 1 && idx == 0) {
        int i;
        RECORD *record;

        for (i = 0; i < RECORD_NUM; i ++) {
            RecordArg *rec_arg;

            record = g_records[i];
            if (!record)
                continue;
            rec_arg = record->arg;
            if (rec_arg && NULL == rec_arg->rcall_open && (STREAM_CTRL_RTSP == ctrl_index || STREAM_CTRL_DVBS == ctrl_index)) {
                LOG_STRM_PRINTF("#%d record[%d] apptype = %d\n", idx, i, rec_arg->apptype);
                if (ctrl_index == g_ctrls[rec_arg->apptype] && ctrl->urlcmp && ctrl->urlcmp(record->argbuf, url, apptype) == 0) {
                    LOG_STRM_PRINTF("#%d mix record!\n", idx);
                    g_idle = play->stream;
                    play->stream = record->stream;
                    stream = play->stream;
                    play->rec_id = rec_arg->pvrarg.id;
                    break;
                }
            }
        }
    }
#endif

#ifdef INCLUDE_PVR
    LOG_STRM_PRINTF("#%d apptype = %d, shiftlen = %d, begin = %d, end = %d, mix = %d\n", idx, apptype, shiftlen, begin, end, g_record_mix);
#else
    LOG_STRM_PRINTF("#%d apptype = %d, shiftlen = %d, begin = %d, end = %d\n", idx, apptype, shiftlen, begin, end);
#endif

    stream->cmdsn = 1;
    stream->backsn = 0;
    stream->seeksn = 0;

    stream->state = STRM_STATE_CLOSE;
    stream->isMusic = 0;
    stream->scale = 0;

    magic = play->arg.magic;

    {
        open_delay = 1;
        //注意：下面的g_play->strm 和 g_pip->strm 不能 替换为 stream
        if (idx == STREAM_INDEX_PIP && g_play->open && g_play->arg.apptype == APP_TYPE_MOSAIC) {
            stream_close_play(1);
            stream_cmd(g_play->stream, STREAM_CMD_WAIT_MOSAIC, magic, 0, 0, 0);
        } else if (idx == STREAM_INDEX_PIP && g_play->open && play->arg.apptype == APP_TYPE_FLASH) {
            stream_close_play(1);
            stream_cmd(g_play->stream, STREAM_CMD_WAIT_FLASH, magic, 0, 0, 0);
        } else if (apptype == APP_TYPE_MOSAIC && g_pip->open >= 1) {
            LOG_STRM_DEBUG("STREAM_CMD_CLOSE\n");
            stream_cmd(g_pip->stream, STREAM_CMD_CLOSE, 0, 0, 0, 0);
            stream_cmd(g_pip->stream, STREAM_CMD_WAIT_MOSAIC, magic, 0, 0, 0);
        } else if (apptype == APP_TYPE_FLASH && g_pip->open >= 1) {
            LOG_STRM_DEBUG("STREAM_CMD_CLOSE\n");
            stream_cmd(g_pip->stream, STREAM_CMD_CLOSE, 0, 0, 0, 0);
            stream_cmd(g_pip->stream, STREAM_CMD_WAIT_FLASH, magic, 0, 0, 0);
        } else {
            open_delay = 0;
        }
    }

    play->byte_total = 0;
    play->byte_current = 0;

    play->time_total = 0;
    play->sync_total = 1;

    if (apptype == APP_TYPE_IPTV || apptype == APP_TYPE_TSTV || apptype == APP_TYPE_DVBS || apptype == APP_TYPE_APPLE_IPTV) {
        if (shiftlen > 0)
            play->time_total = shiftlen;
        play->time_current = mid_time( );
    } else {
        if (apptype == APP_TYPE_HLS && shiftlen > 0)
            play->time_total = shiftlen;
        if ((apptype == APP_TYPE_VOD || apptype == APP_TYPE_PVR || apptype == APP_TYPE_APPLE_VOD) && shiftlen > 0)
            play->time_current = shiftlen;
        else
            play->time_current = 0;
    }

    if(idx == 0) {
        if (g_language[0] == 0) {
            if (g_language_flg == 1) {
                codec_default_language(0, NULL);
                g_language_flg = 0;
            }
        } else {
            codec_default_language(0, g_language);
            g_language[0] = 0;
            g_language_flg = 1;
        }
    }

    if (open_delay == 0)
        stream_cmd(stream, STREAM_CMD_OPEN, magic, 0, 0, 0);
Err:
    if (play) {
        if (magic == 0) {
            play->open = 0;
            LOG_STRM_DEBUG("#%d play_open 0\n", idx);
            /*
            LOG_STRM_PRINTF("#%d STRM_MSG_OPEN_ERROR\n", idx);
            if (play->msgcall)
                play->msgcall(idx, STRM_MSG_OPEN_ERROR, 0, 0, play->callarg);
            else
                stream_port_message(idx, STRM_MSG_OPEN_ERROR, 0, 0);
            */
        }
        play->statecall = NULL;
        play->msgcall = NULL;
    }
    mid_mutex_unlock(g_mutex);

    return magic;
}

int mid_stream_sync(int idx, int msec)
{
    uint32_t base;
    int open_flg;

    open_flg = 0;
    base = mid_10ms( );

    if (idx != 0)
        goto Err;
    if (msec <= 0 || msec > 10000)
        LOG_STRM_ERROUT("#%d msec = %d\n", idx, msec);

    for (;;) {
        if (base + msec / 10 < mid_10ms( ))
            break;
        mid_mutex_lock(g_mutex);

        open_flg = g_play->open;

        mid_mutex_unlock(g_mutex);
        if (open_flg != 1)
            break;
        mid_task_delay(10);
    }
Err:
    LOG_STRM_PRINTF("#%d delay = %d, open = %d\n", idx, mid_10ms( ) - base, open_flg);

    return 0;
}

void mid_stream_close(int idx, int clear)
{
    PLAY *play;
    STREAM *stream;

    if (idx != 0 && idx != 1 && idx != STREAM_INDEX_ADV)
        LOG_STRM_ERROUT("idx = %d\n", idx);
    LOG_STRM_PRINTF("#%d clear = %d\n", idx, clear);

    if (idx == 1)
        idx = STREAM_INDEX_PIP;
    play = g_playArray[idx];
    if (play == NULL)
        LOG_STRM_ERROUT("#%d play is NULL\n", idx);

    mid_mutex_lock(g_mutex);

    stream = play->stream;

    if (clear)
        strm_play_clear(stream->strm_play, stream->index);

    if (idx == 0) {
        stream_close_play(clear);
    } else {
        LOG_STRM_DEBUG("STREAM_CMD_CLOSE\n");
        stream_cmd(stream, STREAM_CMD_CLOSE, clear, 0, 0, 0);
    }
    play->open = 0;

    mid_mutex_unlock(g_mutex);

Err:
    return;
}

int stream_back_get_arg(int idx, uint32_t magic, PlayArg **pp_arg, char **pp_argbuf)
{
    PLAY *play;
    STREAM *stream;

    int ret = -1;

    if (pp_arg)
        *pp_arg = NULL;
    if (pp_argbuf)
        *pp_argbuf = NULL;

    mid_mutex_lock(g_mutex);

    play = g_playArray[idx];
    if (play == NULL)
        LOG_STRM_ERROUT("#%d play is NULL\n", idx);


    stream = play->stream;
    LOG_STRM_DEBUG("#%d, play_index = %d\n", idx, stream->index);
    if (idx == stream->index) {
        if (magic == play->arg.magic) {//播放关闭时g_play->magic会被置为零
            stream->plyarg = play->arg;
            IND_MEMCPY(stream->argbuf, play->argbuf, play->argsize);
            if (pp_arg)
                *pp_arg = &stream->plyarg;
            if (pp_argbuf)
                *pp_argbuf = stream->argbuf;
            play->play_magic = magic;
            ret = 0;
        } else {
            LOG_STRM_PRINTF("#%d overdue!\n", idx);
        }
    } else {
        LOG_STRM_ERROR("#%d not play! idx = %d\n", idx, stream->index);
    }

    mid_mutex_unlock(g_mutex);

    return ret;
Err:
    return -1;
}

void stream_back_cmd(int idx, int sn)
{
    STREAM *stream;

    mid_mutex_lock(g_mutex);

    stream = g_play->stream;
    LOG_STRM_DEBUG("#%d play_index = %d\n", idx, stream->index);
    if (idx == stream->index) {
        stream->backsn = sn;
        if (stream->seeksn && stream->seeksn <= sn)
            stream->seeksn = 0;
    }
    mid_mutex_unlock(g_mutex);
}

void stream_back_wake(uint32_t id)
{
    stream_cmd(g_play->stream, STREAM_CMD_WAKE, id, 0, 0, 0);
}

void stream_back_totaltime(int idx, uint32_t total)
{
    mid_mutex_lock(g_mutex);

    if (idx == g_play->stream->index && g_play->play_magic == g_play->arg.magic) {
        g_play->time_total = total;
        LOG_STRM_DEBUG("#%d time_total = %d\n", idx, total);
    }

    mid_mutex_unlock(g_mutex);
}

void stream_back_currenttime(int idx, uint32_t current)
{
    STREAM *stream;

    mid_mutex_lock(g_mutex);

    stream = g_play->stream;
    if (idx != stream->index || g_play->play_magic != g_play->arg.magic)
        goto End;
    if (stream->seeksn && stream->seeksn > stream->backsn)
        goto End;
    g_play->time_current = current;
    LOG_STRM_DEBUG("#%d time_current = %d\n", idx, current);

End:
    mid_mutex_unlock(g_mutex);
}

void stream_back_totalbyte(int idx, long long total)
{
    mid_mutex_lock(g_mutex);

    if (idx == g_play->stream->index && g_play->play_magic == g_play->arg.magic) {
        g_play->byte_total = total;
        LOG_STRM_DEBUG("#%d byte_total = %lld\n", idx, total);
    }

    mid_mutex_unlock(g_mutex);
}

void stream_back_currentbyte(int idx, long long current)
{
    STREAM *stream;

    mid_mutex_lock(g_mutex);

    stream = g_play->stream;
    if (idx != stream->index || g_play->play_magic != g_play->arg.magic)
        goto End;
    if (stream->seeksn && stream->seeksn > stream->backsn)
        goto End;
    g_play->byte_current = current;
    LOG_STRM_DEBUG("#%d byte_current = %lld\n", idx, current);

End:
    mid_mutex_unlock(g_mutex);
}

void mid_stream_rect(int idx, int x, int y, int w, int h)
{
    STREAM *stream = stream_dispatch(idx, NULL);
    if (stream == NULL)
        LOG_STRM_ERROUT("#%d stream_dispatch\n", idx);
    LOG_STRM_PRINTF("#%d %d %d %d %d\n", idx, x, y, w, h);
    stream_cmd(stream, STREAM_CMD_RECT, x, y, w, h);
Err:
    return;
}

void mid_stream_pause(int idx)
{
    int sn = 0;
    STREAM *stream = stream_dispatch(idx, &sn);
    if (stream == NULL)
        LOG_STRM_ERROUT("#%d stream_dispatch\n", idx);
    LOG_STRM_PRINTF("#%d\n", idx);
    stream_cmd(stream, STREAM_CMD_PAUSE, 0, 0, 0, sn);
Err:
    return;
}

void mid_stream_resume(int idx)
{
    int sn = 0;
    STREAM *stream = stream_dispatch(idx, &sn);
    if (stream == NULL)
        LOG_STRM_ERROUT("#%d stream_dispatch\n", idx);
    LOG_STRM_PRINTF("#%d\n", idx);
    stream_cmd(stream, STREAM_CMD_RESUME, 0, 0, 0, sn);
Err:
    return;
}

void mid_stream_fast(int idx, int scale)
{
    int sn = 0;
    STREAM *stream = stream_dispatch(idx, &sn);
    if (stream == NULL)
        LOG_STRM_ERROUT("#%d stream_dispatch\n", idx);

    switch(scale) {
    case -64:   break;
    case -32:   break;
    case -16:   break;
    case -8:    break;
    case -4:    break;
    case -2:    break;
    case 1:     break;
    case 2:     break;
    case 4:     break;
    case 8:     break;
    case 16:    break;
    case 32:    break;
    case 64:    break;
    default:    LOG_STRM_ERROUT("#%d scale = %d\n", idx, scale);
    }

    LOG_STRM_PRINTF("#%d scale = %d isMusic = %d\n", idx, scale, stream->isMusic);
    stream_cmd(stream, STREAM_CMD_FAST, scale, 0, 0, sn);

#ifndef VIETTEL_HD
    if (idx == 0 && g_play->open > 0 && !stream->isMusic && stream->state >= STRM_STATE_PLAY
        && ((scale == -2 && stream->apptype <= APP_TYPE_IPTV2) || (scale == 2 && stream->apptype <= APP_TYPE_VODADV)))
    {
        if (stream->plyarg.statecall)
            stream->plyarg.statecall(0, STRM_STATE_FAST, scale, g_play->play_magic, stream->plyarg.callarg);
        else
            stream_port_state(0, STRM_STATE_FAST, scale, g_play->play_magic);
    }
#endif

Err:
    return;
}

void mid_stream_seek(int idx, int sec)
{
    int sn = 0;
    STREAM *stream;

    if (idx != 0)
        LOG_STRM_ERROUT("#%d unsurpot\n", idx);

    stream = stream_dispatch(idx, &sn);
    if (stream == NULL)
        LOG_STRM_ERROUT("#%d stream_dispatch\n", idx);
    LOG_STRM_PRINTF("#%d sec = %d\n", idx, sec);

    mid_mutex_lock(g_mutex);

    if (g_play->open == 0)
        goto End;
    stream->seeksn = sn;
    if (g_play->arg.apptype != APP_TYPE_HLS)
        g_play->time_current = sec;

    stream_cmd(stream, STREAM_CMD_SEEK, sec, 0, 0, sn);

End:
    mid_mutex_unlock(g_mutex);
Err:
    return;
}

void mid_stream_lseek(int idx, long long offset)
{
    int sn;
    uint32_t off0, off1;
    STREAM *stream;

    if (idx != 0)
        LOG_STRM_ERROUT("#%d unsurpot\n", idx);

    sn = 0;
    stream = stream_dispatch(idx, &sn);
    if (stream == NULL)
        LOG_STRM_ERROUT("#%d stream_dispatch\n", idx);

    off0 = (uint32_t)((unsigned long long)offset >> 32);
    off1 = (uint32_t)((unsigned long long)offset);
    LOG_STRM_PRINTF("#%d offset = %lld off0 = %08x, off1 = %08x\n", idx, offset, off0, off1);

    mid_mutex_lock(g_mutex);
    if (g_play->open == 0)
        goto End;
    stream->seeksn = sn;
    g_play->byte_current = offset;

    stream_cmd(stream, STREAM_CMD_LSEEK, (int)off0, (int)off1, 0, sn);
End:
    mid_mutex_unlock(g_mutex);
Err:
    return;
}

void mid_stream_stop(int idx)
{
    int apptype, sn = 0;
    STREAM *stream;

    if (idx != 0)
        LOG_STRM_ERROUT("#%d unsurpot\n", idx);

    stream = stream_dispatch(idx, &sn);
    if (stream == NULL)
        LOG_STRM_ERROUT("#%d stream_dispatch\n", idx);
    LOG_STRM_PRINTF("#%d\n", idx);

    mid_mutex_lock(g_mutex);
    if (g_play->open == 0)
        goto End;
    apptype = g_play->arg.apptype;
    switch (apptype) {
    case APP_TYPE_VOD:
        if (g_play->time_total > 3) {
            stream->seeksn = sn;
            g_play->time_current = g_play->time_total - 3;
        }
        break;
    case APP_TYPE_IPTV:
    case APP_TYPE_TSTV:
    case APP_TYPE_DVBS:
        stream->seeksn = sn;
        g_play->time_current = mid_time( );
        break;
    default:
        break;
    }

    stream_cmd(stream, STREAM_CMD_STOP, 0, 0, 0, sn);
End:
    mid_mutex_unlock(g_mutex);
Err:
    return;
}

void mid_stream_emm(void)
{
    uint32_t magic;

    LOG_STRM_PRINTF("\n");
    magic = int_stream_nextmagic( );

    mid_mutex_lock(g_mutex);

    g_emm_magic = magic;

    mid_mutex_unlock(g_mutex);
}

void mid_stream_set_call(int idx, StrmStateCall statecall, StrmMsgCall msgcall, int callarg)
{
    PLAY* play = int_stream_play(idx);

    if (!play)
        return;

    LOG_STRM_PRINTF("#%d statecall = %p, msgcall = %p, callarg = %d\n", idx, statecall, msgcall, callarg);

    mid_mutex_lock(g_mutex);

    play->statecall = statecall;
    play->msgcall = msgcall;
    play->callarg = callarg;

    mid_mutex_unlock(g_mutex);
}

void mid_stream_set_privcall(int idx, APP_TYPE apptype, StrmPrivCall privcall, int privarg)
{
    int ctrl_index;
    PLAY* play = int_stream_play(idx);

    if (!play)
        return;

    LOG_STRM_PRINTF("#%d\n", idx);

    if (apptype >= APP_TYPE_MAX)
        LOG_STRM_ERROUT("#%d apptype = %d\n", idx, apptype);
    ctrl_index = g_ctrls[apptype];

    mid_mutex_lock(g_mutex);

    play->privcalls[ctrl_index] = privcall;
    play->privargs[ctrl_index] = privarg;

    mid_mutex_unlock(g_mutex);

Err:
    return;
}

void mid_stream_set_language(char* language)
{
    if (language)
        LOG_STRM_PRINTF("language = %s\n", language);
    else
        LOG_STRM_PRINTF("\n");

    mid_mutex_lock(g_mutex);

    if(language) {
        IND_MEMCPY(g_language, language, 3);
        g_language[3] = 0;
    } else {
        g_language[0] = 0;
    }

    mid_mutex_unlock(g_mutex);
}

void mid_stream_set_msg(int msg_flag)
{
    LOG_STRM_PRINTF("fcc msg_flag = %d\n", msg_flag);
    g_msg_flag = msg_flag;
}

APP_TYPE mid_stream_get_apptype(int idx)
{
    APP_TYPE apptype = APP_TYPE_MAX;
    PLAY* play = int_stream_play(idx);

    if (!play)
        return APP_TYPE_MAX;

    mid_mutex_lock(g_mutex);

    if (play) {
        apptype = play->arg.apptype;
        if (APP_TYPE_HTTP == apptype && play->arg.shiftid)
            apptype = APP_TYPE_HLS;
    }

    mid_mutex_unlock(g_mutex);

    return apptype;
}

APP_TYPE int_stream_get_apptype(void)
{
    APP_TYPE apptype = APP_TYPE_MAX;
    PLAY *play = g_play;

    if (play && play->open) {
        apptype = play->arg.apptype;
        if (APP_TYPE_HTTP == apptype && play->arg.shiftid)
            apptype = APP_TYPE_HLS;
    }

    return apptype;
}

void mid_stream_sync_totaltime(int idx, int msec)
{
    int delay;
    STREAM *stream = g_play->stream;

    delay = 0;

    if (idx != 0)
        goto End;
    if (g_play->sync_total == 0 || g_play->arg.apptype != APP_TYPE_VOD)
        goto End;

    while (g_play->time_total == 0 && g_play->open && stream->backsn < stream->cmdsn && msec > 0) {
        mid_task_delay(10);
        msec -= 10;
        delay += 10;
    }
    g_play->sync_total = 0;

End:
    if (delay > 0)
        LOG_STRM_DEBUG("#%d delay = %d\n", idx, delay);
}

unsigned int mid_stream_get_totaltime(int idx)
{
    uint32_t total;

    mid_mutex_lock(g_mutex);

    if (idx == 0) {
        total = g_play->time_total;
#if SUPPORTE_HD == 1
        if (0 == total && APP_TYPE_ZEBRA == g_play->arg.apptype) {
            ymm_stream_playerGetTotalTime(0, &total);
            total /= 1000;
        }
#endif
        LOG_STRM_DEBUG("#%d total = %d\n", idx, total);
    } else {
        LOG_STRM_DEBUG("#%d unvalid time!\n", idx);
        total = 0;
    }

    mid_mutex_unlock(g_mutex);

    return total;
}

unsigned int mid_stream_get_currenttime(int idx)
{
    int current;

    mid_mutex_lock(g_mutex);

    if (idx == 0) {
        current = g_play->time_current;
        if (APP_TYPE_IPTV == g_play->arg.apptype || APP_TYPE_APPLE_IPTV == g_play->arg.apptype) {
            char buf[16];
            struct ind_time tp;

            ind_time_local(current, &tp);
            snprintf(buf, 16, "%04d%02d%02dT%02d%02d%02d", tp.year, tp.mon, tp.day, tp.hour, tp.min, tp.sec);
            buf[15] = 0;
            LOG_STRM_DEBUG("#%d current = %d / %s\n", idx, current, buf);
        } else {
            LOG_STRM_DEBUG("#%d current = %d\n", idx, current);
        }
    } else {
        LOG_STRM_DEBUG("#%d unvalid time!\n", idx);
        current = 0;
    }

    mid_mutex_unlock(g_mutex);

    return current;
}

long long mid_stream_get_totalbyte(int idx)
{
    long long total;

    mid_mutex_lock(g_mutex);

    if (idx == 0) {
        total = g_play->byte_total;
        LOG_STRM_DEBUG("#%d currentbyte = %lld\n", idx, total);
    } else {
        LOG_STRM_DEBUG("#%d invalid byte!\n", idx);
        total = 0;
    }

    mid_mutex_unlock(g_mutex);

    return total;
}

long long mid_stream_get_currentbyte(int idx)
{
    long long current;

    mid_mutex_lock(g_mutex);

    if (idx == 0) {
        current = g_play->byte_current;
        LOG_STRM_DEBUG("#%d currentbyte = %lld\n", idx, current);
    } else {
        LOG_STRM_DEBUG("#%d invalid byte!\n", idx);
        current = 0;
    }

    mid_mutex_unlock(g_mutex);

    return current;
}

int mid_stream_auido_length(char* filepath)
{
    void *fp;
    int length, filesize;

    fp = NULL;
    length = 0;

    if (filepath == NULL)
        LOG_STRM_ERROUT("filepath is NULL");
    filesize = stream_port_fsize(filepath);
    if (filesize <= 0)
        LOG_STRM_ERROUT("filesize = %d\n", filesize);

    fp = stream_port_fopen(filepath, "rb");
    if (fp == NULL)
        LOG_STRM_ERROUT("fopen %s\n", filepath);

    length = ts_audio_duration(fp, stream_port_fread, filesize);

Err:
    if (fp)
        stream_port_fclose(fp);
    return length;
}


#ifdef INCLUDE_PVR


#define CHECK_RECORD_INDEX(idx)         \
    RECORD *record = NULL;              \
    if (idx < 0 || idx >= RECORD_NUM)   \
        LOG_STRM_ERROUT("#%d\n", idx);          \
    record = g_records[idx];            \
    if (record == NULL)                 \
        LOG_STRM_ERROUT("#%d record is NULL\n", idx)

static void int_record_close0(RECORD *record, int idx, int del, uint32_t id)
{
    RecordArg *arg;

    arg = record->arg;
    if (id) {
        RecordArg *prev = NULL;

        while (arg && id != arg->pvrarg.id) {
            prev = arg;
            arg = arg->next;
        }
        if (arg) {
            if (prev)
                prev->next = arg->next;
            else
                record->arg = arg->next;
            IND_FREE(arg);
        }
    } else {
        RecordArg *next;

        record->arg = NULL;
        while (arg) {
            next = arg->next;
            IND_FREE(arg);
            arg = next;
        }
    }

    if (!record->arg && record->stream == g_play->stream) {
        if (g_idle) {
            LOG_STRM_PRINTF("#%d demix play!\n", idx);
            record->stream = g_idle;
            g_idle = NULL;
        } else {
            LOG_STRM_ERROR("#%d g_idle is NULL\n", idx);
        }
    }
}

static void int_record_close1(RECORD *record, int idx, int del, uint32_t id)
{
    stream_cmd(record->stream, STREAM_CMD_RECORD_CLOSE, del, (int)id, 0, 0);
    int_record_close0(record, idx, del, id);
}

void mid_record_mix(int mix)
{
    g_record_mix = mix;
}

void record_post_msg(int idx, uint32_t id, STRM_MSG msgno, int arg)
{
    LOG_STRM_DEBUG("#%d msgno = %d, id = %08x, msgno = %d, arg = %d\n", idx, msgno, id, msgno, arg);
    record_port_message(idx, id, msgno, arg);
    LOG_STRM_DEBUG("#%d\n", idx);
}

static uint32_t g_id = 0;
static uint32_t int_record_id(void)
{
    uint32_t id = mid_time( );
    if (id == g_id)
        id ++;
    while(ind_pvr_exist(id))
        id ++;
    g_id = id;

    return id;
}

static uint32_t int_record_open0(int idx, char* url, APP_TYPE apptype, char* info_buf, int info_len, uint32_t id, uint32_t begin, uint32_t end)
{
    uint32_t ret = 0;
    int add, record_mix, ctrl_index;
    RecordArg *arg = NULL;
    StreamCtrl *ctrl;

    CHECK_RECORD_INDEX(idx);

    LOG_STRM_PRINTF("#%d buildtime "__DATE__" "__TIME__", id = %08x\n", idx, id);
    LOG_STRM_PRINTF("#%d open = %p\n", idx, record->arg);

    if (url == NULL || strlen(url) >= STREAM_URL_SIZE)
        LOG_STRM_ERROUT("#%d url  = %p\n", idx, url);

    if (info_buf == NULL || info_len <= 0 || info_len >= PVR_INFO_SIZE)
        LOG_STRM_ERROUT("#%d info_buf = %p, info_len = %d\n", idx, info_buf, info_len);

    if (apptype >= APP_TYPE_MAX)
        LOG_STRM_ERROUT("#%d apptype = %d\n", idx, apptype);
    ctrl_index = g_ctrls[apptype];
    ctrl = &record->stream->ctrls[ctrl_index];
    if (ctrl->loop_record == NULL)
        LOG_STRM_ERROUT("#%d apptype = %d ctrls is NULL\n", idx, apptype);

    add = 0;
    if (g_rcall_open[idx] && g_rcall_push[idx] && g_rcall_close[idx]) {
        record_mix = 3;
    } else {
        record_mix = 0;

        if (id) {
            struct PVRInfo info;

            memset(&info, 0, sizeof(info));
            if (ind_pvr_get_info(id, &info) == 0) {
                if (0 == info.time_len) {
                    ind_pvr_delete(id, NULL);
                    id = 0;
                } else if (2 == info.recording)
                    add = 2;
                else
                    add = 1;
            } else {
                id = 0;
            }
        }

        if (!add && APP_TYPE_HTTP == apptype) {
            if (1 == g_record_mix && NULL == g_idle && g_play->open && APP_TYPE_HLS == g_play->arg.apptype && ctrl->urlcmp && 0 == ctrl->urlcmp(g_play->argbuf, url, apptype)) {
                record_mix = 1;
                id = g_play->arg.shiftid;
            } else {
                record_mix = -1;
            }
        }
    }
    if (!id)
        id = int_record_id( );

    if (record->arg) {
        if (0 == record_mix && NULL == record->arg->rcall_open && 1 == g_record_mix && ctrl->urlcmp && ctrl->urlcmp(record->argbuf, url, apptype) == 0) {
            if (add) {
                uint32_t base_id0, base_id1;

                base_id0 = ind_pvr_get_base(record->arg->pvrarg.id);
                base_id1 = ind_pvr_get_base(id);

                LOG_STRM_PRINTF("#%d base_id0 = %08x, base_id1 = %08x!\n", idx, base_id0, base_id1);
                if (base_id0 == base_id1)
                    record_mix = 2;
            } else {
                record_mix = 2;
            }
        }

        if (2 != record_mix)
            int_record_close1(record, idx, 0, 0);
    }

    arg = (RecordArg *)IND_CALLOC(sizeof(RecordArg), 1);
    if (!arg)
        LOG_STRM_ERROUT("#%d malloc RecordArg\n", idx);

    arg->index = idx;
    arg->apptype = apptype;

    IND_MEMCPY(arg->pvrarg.info_buf, info_buf, info_len);
    arg->pvrarg.info_len = info_len;

    if (apptype == APP_TYPE_HTTP) {
        arg->begin = 0;
        if (end > 0 && end < 8*3600)
            arg->pvrarg.time_length = end;
        arg->end = 0;
    } else {
        arg->begin = begin;
        arg->end = end;
        if (end != (uint32_t)-1 && end > begin && begin > 0)
            arg->pvrarg.time_length = end - begin;
    }
    arg->add = add;
    arg->pvrarg.id = id;

    record->new_magic ++;
    arg->magic = record->new_magic;

    if (ctrl->argparse_record(idx, arg, record->argbuf, url))
        LOG_STRM_ERROUT("#%d plyarg_parse\n", idx);

    if (1 == record_mix) {
            LOG_STRM_PRINTF("#%d mix http play!\n", idx);
            g_idle = record->stream;
            record->stream = g_play->stream;
    } else if (0 == record_mix && 1 == g_record_mix && g_idle == NULL && g_play->open && (0 == arg->add || id == g_play->rec_id)) {
        /*
            码流复用只考虑频道开始播放时启动时移情况 2012-4-12 12:17:05 by liujianhua
         */
        if (ctrl_index == g_ctrls[g_play->arg.apptype] && ctrl->urlcmp && 0 == ctrl->urlcmp(g_play->argbuf, url, apptype)) {
            LOG_STRM_PRINTF("#%d mix play!\n", idx);
            g_idle = record->stream;
            record->stream = g_play->stream;
            g_play->rec_id = id;
        }
    }

    LOG_STRM_PRINTF("#%d apptype = %d, info_len = %d, id = %08x, begin = %08x, end = %08x\n", idx, apptype, info_len, id, begin, end);

    if (3 == record_mix) {
        arg->rcall_open = g_rcall_open[idx];
        arg->rcall_push = g_rcall_push[idx];
        arg->rcall_close = g_rcall_close[idx];
    }
    arg->next = record->arg;
    record->arg = arg;

    ret = id;
Err:
    if (!ret && record && arg)
        IND_FREE(arg);

    if (idx >= 0 && idx < RECORD_NUM)
        g_rcall_open[idx] = NULL;
    return ret;
}

static void int_record_open1(int idx)
{
    RecordArg *arg;
    CHECK_RECORD_INDEX(idx);

    arg = record->arg;
    if (arg && 2 == arg->add) {
        int i = arg->index;
        uint32_t id = arg->pvrarg.id;
        LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_END\n", arg->index);
        record_post_msg(i, id, RECORD_MSG_SUCCESS_END, 0);
        int_record_close0(record, i, 0, id);
    } else {
        stream_cmd(record->stream, STREAM_CMD_RECORD_OPEN, record->new_magic, 0, 0, 0);
    }
Err:
    return;
}

uint32_t mid_record_open(int idx, char* url, APP_TYPE apptype, char* info_buf, int info_len, uint32_t id, uint32_t begin, uint32_t end)
{
    uint32_t ret = 0;

    LOG_STRM_PRINTF("#%d apptype = %d, id = %08x\n", idx, apptype, id);

    mid_mutex_lock(g_mutex);

    ret = int_record_open0(idx, url,apptype, info_buf, info_len, id, begin, end);
    if (ret == 0)
        LOG_STRM_ERROUT("#%d int_record_open0\n", idx);
    int_record_open1(idx);

Err:
    mid_mutex_unlock(g_mutex);

    return ret;
}

int mid_record_call(int idx, RecordCall_open rcall_open, RecordCall_push rcall_push, RecordCall_close rcall_close)
{
    if (idx < 0 || idx >= RECORD_NUM)
        LOG_STRM_ERROUT("index = %d\n", idx);
    if (!rcall_open || !rcall_push || !rcall_close)
        LOG_STRM_ERROUT("#%d rcall_open = %p, rcall_push = %p, rcall_close = %p\n", idx, rcall_open, rcall_push, rcall_close);

    LOG_STRM_PRINTF("#%d\n", idx);

    g_rcall_open[idx] = rcall_open;
    g_rcall_push[idx] = rcall_push;
    g_rcall_close[idx] = rcall_close;

    return 0;
Err:
    return -1;
}

uint32_t mid_record_open0(int idx, char* url, APP_TYPE apptype, char* info_buf, int info_len, uint32_t id, uint32_t begin, uint32_t end)
{
    uint32_t ret = 0;

    LOG_STRM_PRINTF("#%d apptype = %d, id = %08x\n", idx, apptype, id);

    mid_mutex_lock(g_mutex);

    ret = int_record_open0(idx, url,apptype, info_buf, info_len, id, begin, end);

    mid_mutex_unlock(g_mutex);

    return ret;
}

void mid_record_open1(int idx)
{
    mid_mutex_lock(g_mutex);

    int_record_open1(idx);

    mid_mutex_unlock(g_mutex);
}

void mid_record_close(int idx, uint32_t id)
{
    CHECK_RECORD_INDEX(idx);

    LOG_STRM_PRINTF("#%d id = 0x%08x\n", idx, id);

    mid_mutex_lock(g_mutex);

    int_record_close1(record, idx, 0, id);

    mid_mutex_unlock(g_mutex);
Err:
    return;
}

int int_record_type(int idx)
{
    CHECK_RECORD_INDEX(idx);

    if (record->arg)
        return record->arg->apptype;
Err:
    return -1;
}

void int_record_delete(uint32_t id)
{
    int idx;
    RECORD *record;
    RecordArg *arg;

    mid_mutex_lock(g_mutex);

    for (idx = 0; idx < RECORD_NUM; idx ++) {
        record = g_records[idx];

        if (record) {
            arg = record->arg;
            while (arg) {
                if (id == arg->pvrarg.id) {
                    LOG_STRM_PRINTF("id = %08x\n", id);
                    int_record_close1(record, idx, -1, id);
                    break;
                }
                arg = arg->next;
            }
        }
    }

    mid_mutex_unlock(g_mutex);
    return;
}

void int_record_message(int idx, STREAM_CMD cmd, int arg0, int arg1, int arg2, int arg3)
{
    CHECK_RECORD_INDEX(idx);

    mid_mutex_lock(g_mutex);

    if (record->arg)
        stream_cmd(record->stream, cmd, arg0, arg1, arg2, arg3);

    mid_mutex_unlock(g_mutex);
Err:
    return;
}

#ifdef INCLUDE_PVR
static RECORD* record_find_index(int idx)
{
    RECORD *record;
    int i;

    for (i = 0; i < RECORD_NUM; i ++) {
        record = g_records[i];
        if (record == NULL)
            continue;
        if (idx == record->stream->index)
            return record;
    }
    return NULL;
}

static RECORD* record_find_id(uint32_t id)
{
    int i;
    RECORD *rec;
    RecordArg *arg;

    for (i = 0; i < RECORD_NUM; i ++) {
        rec = g_records[i];
        if (rec == NULL)
            continue;
        arg = rec->arg;
        while (arg && arg->pvrarg.id != id)
            arg = arg->next;
        if (arg)
            return rec;
    }
    return NULL;
}
#endif

int record_back_get_arg(int idx, uint32_t magic, RecordArg **pp_arg, char **pp_argbuf)
{
    STREAM *stream;
    RecordArg *arg;
    int ret = -1;
    RECORD* record = record_find_index(idx);

    if (record == NULL)
        LOG_STRM_ERROUT("#%d record is NULL\n", idx);

    mid_mutex_lock(g_mutex);

    arg = record->arg;
    while (arg) {
        if (magic == arg->magic) {
            stream = record->stream;

            stream->recarg = *arg;
            IND_MEMCPY(stream->argbuf, record->argbuf, record->argsize);
            if (pp_arg)
                *pp_arg = &stream->recarg;
            if (pp_argbuf)
                *pp_argbuf = stream->argbuf;
            ret = 0;

            break;
        }
        arg = arg->next;
    }
    if (-1 == ret)
        LOG_STRM_PRINTF("#%d overdue!\n", idx);

    mid_mutex_unlock(g_mutex);

Err:
    return ret;
}

int record_back_wait(int idx, uint32_t id)
{
    RECORD *record;

    LOG_STRM_PRINTF("#%d id = 0x%x\n", idx, id);

    mid_mutex_lock(g_mutex);

    record = record_find_id(id);

    mid_mutex_unlock(g_mutex);

    if (record == NULL)
        LOG_STRM_ERROUT("#%d record is NULL\n", idx);

    stream_cmd(record->stream, STREAM_CMD_WAIT_RECORD, (int)id, 0, 0, 0);

    return 0;
Err:
    return -1;
}

int record_back_exist(uint32_t id)
{
    RECORD *record;

    mid_mutex_lock(g_mutex);

    record = record_find_id(id);

    mid_mutex_unlock(g_mutex);

    if (record)
        return 1;
    else
        return 0;
}

void record_back_close(uint32_t id)
{
    int idx;
    RECORD *record;
    RecordArg *arg;

    mid_mutex_lock(g_mutex);

    for (idx = 0; idx < RECORD_NUM; idx ++) {
        record = g_records[idx];

        if (record) {
            arg = record->arg;
            while (arg) {
                if (id == arg->pvrarg.id) {
                    LOG_STRM_PRINTF("id = %08x\n", id);
                    int_record_close0(record, idx, 0, id);
                    break;
                }
                arg = arg->next;
            }
        }
    }

    mid_mutex_unlock(g_mutex);
    return;
}

#endif//INCLUDE_PVR

