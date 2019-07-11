/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#ifndef __STREAM_P_H__
#define __STREAM_P_H__

#include "pthread.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern int g_debug_rtsp;
#define MODULE_DEBUG_ENABLE     g_debug_rtsp

#include "mid_stream.h"

#include "dns/mid_dns.h"
#include "mid/mid_msgq.h"
#include "mid/mid_mutex.h"
#include "mid/mid_time.h"

#include "ind_ts.h"
#include "ind_tmr.h"
#include "ind_pvr.h"
#include "ind_mem.h"
#include "ind_net.h"
#include "ind_pcm.h"
#include "ind_rtp.h"
#include "ind_string.h"

#include "codec.h"

#include "strm_play.h"

#include "StreamAssertions.h"

typedef struct _RecordArg RecordArg;
typedef struct _RecordArg* RecordArg_t;

#ifdef INCLUDE_PVR

#include "strm_record.h"

#include "mid_record.h"
#include "porting/record_port.h"

#define RECORD_NAME_LEN         16

#endif

#define TS_TIMESHIFT_SIZE       10528//1316 * 8

#define STREAM_HEAD_LEN         4096
#define STREAM_DECODE_NUM       2

#define RTP_BUFFER_LENGTH       (40 + 1316)//zte rtp packet len more than 1348

#define NAT_BUFFER_LENGTH       (8)

#define IGMP_INFO_SIZE          96

enum {
    STREAM_INDEX_PLAY = 0,
    STREAM_INDEX_REC0,
    STREAM_INDEX_REC1,
    STREAM_INDEX_PIP,
    STREAM_INDEX_ADV,
    STREAM_INDEX_NUM
};

enum {
    PLAYRECORD_PLAY = 0,
    PLAYRECORD_RECORD,
    PLAYRECORD_COMPREHENSIVE,
};

enum {
    STREAM_CTRL_NONE = 0,
    STREAM_CTRL_RTSP,
    STREAM_CTRL_AUDIO,
    STREAM_CTRL_HTTP,
    STREAM_CTRL_PVR,
    STREAM_CTRL_DVBS,       //5
    STREAM_CTRL_MOSAIC,
    STREAM_CTRL_FLASH,
    STREAM_CTRL_DISK,
    STREAM_CTRL_RTP2TS,
    STREAM_CTRL_MIX_PCM,    //10
    STREAM_CTRL_MIX_MP3,
    STREAM_CTRL_HTTP_PCM,
    STREAM_CTRL_HTTP_MP3,
    STREAM_CTRL_ZEBRA,
    STREAM_CTRL_ZEBRA_PCM,
    STREAM_CTRL_HTTP_MPA,
    STREAM_CTRL_RESERVE,
    STREAM_CTRL_HTTP_LIVE,
    STREAM_CTRL_APPLE,
    STREAM_CTRL_MAX,
};

typedef enum
{
    STREAM_CMD_NONE = 0,

    STREAM_CMD_INTERNAL,//模块内部消息

    STREAM_CMD_RECT,
    STREAM_CMD_OPEN,
    STREAM_CMD_PAUSE,
    STREAM_CMD_FAST,

    STREAM_CMD_RESUME,
    STREAM_CMD_SEEK,
    STREAM_CMD_STOP,        //8
    STREAM_CMD_CLOSE,

    STREAM_CMD_WATERMARK,
    STREAM_CMD_ADVERTISE,

    STREAM_CMD_ADVERTISED,

    STREAM_CMD_TIMESHIFT_OPEN,
    STREAM_CMD_TIMESHIFT_CLOSE,

    STREAM_CMD_WAKE,
    STREAM_CMD_WAIT_RECORD, //16
    STREAM_CMD_RECORD_OPEN,
    STREAM_CMD_RECORD_CLOSE,
    STREAM_CMD_RECORD_END,

    STREAM_CMD_WAIT_MOSAIC,
    STREAM_CMD_WAIT_FLASH,

    STREAM_CMD_LSEEK,//8 按长度seek

    STREAM_CMD_RECORD_CHECK_BANDWIDTH,
    STREAM_CMD_RECORD_LIMIT_BANDWIDTH,//24
    STREAM_CMD_RECORD_ADJUST_BANDWIDTH,

    STREAM_CMD_TEST_SAVE,
    STREAM_CMD_TEST_BANDWIDTH,

    STREAM_CMD_MAX
} STREAM_CMD;

typedef struct _StreamMsg StreamMsg;
struct _StreamMsg {
    StreamMsg*    next;
    STRM_MSG    msg;
    int            arg;
};

typedef struct _StreamCmd StreamCmd;
struct _StreamCmd {
    StreamCmd*    next;
    int         cmd;
    int            arg0;
    int            arg1;
    int            arg2;
    int            arg3;
};

typedef struct {
    uint32_t magic;
    APP_TYPE apptype;

    StrmStateCall statecall;
    StrmMsgCall msgcall;
    int callarg;

    StrmPrivCall privcall;
    int privarg;
    uint32_t shiftid;
} PlayArg;

typedef struct _RecordMix* RecordMix_t;
#ifdef INCLUDE_PVR
struct _RecordArg {
    RecordArg* next;

    int index;
    uint32_t magic;
    APP_TYPE apptype;

    uint32_t begin;
    uint32_t end;
    int add;

    RecordCall_open     rcall_open;
    RecordCall_push     rcall_push;
    RecordCall_close    rcall_close;

    PvrArgument pvrarg;
};

typedef struct _RecordMix RecordMix;
struct _RecordMix {
    RecordMix_t next;
    uint32_t        id;
    uint32_t        clk;//结束时间
};

#endif


typedef struct _StreamCtrl {
    void    *handle;
    void    (*loop_play)        (void *handle, int idx, mid_msgq_t msgq, PlayArg *arg, char *argbuf);
    void    (*loop_record)      (void *handle, int idx, mid_msgq_t msgq, RecordArg *arg, char *argbuf);

    int     argsize;
    int     (*argparse_play)    (int idx, PlayArg *arg, char *argbuf, const char* url, int shiftlen, int begin, int end);
    int     (*argparse_record)  (int idx, RecordArg* arg, char *argbuf, const char* url);
    int     (*urlcmp)           (char *argbuf, const char* url, APP_TYPE apptype);
} StreamCtrl;


typedef int (*ctrl_create_f)(StreamCtrl *ctrl);

int apple_create_stream     (StreamCtrl *ctrl);
int audio_create_stream     (StreamCtrl *ctrl);
int disk_create_stream      (StreamCtrl *ctrl);
int dvbs_create_stream      (StreamCtrl *ctrl);
int flash_create_stream     (StreamCtrl *ctrl);
int http_create_stream      (StreamCtrl *ctrl);
int mosaic_create_stream    (StreamCtrl *ctrl);
int pvr_create_stream       (StreamCtrl *ctrl);
int rtsp_create_stream      (StreamCtrl *ctrl);
int zebra_create_stream     (StreamCtrl *ctrl);
int zebra_pcm_create_stream (StreamCtrl *ctrl);



int rtp2ts_create_stream(StreamCtrl *ctrl);

int mix_pcm_create_stream   (StreamCtrl *ctrl);
int mix_mp3_create_stream   (StreamCtrl *ctrl);
int http_pcm_create_stream  (StreamCtrl *ctrl);
int http_mp3_create_stream  (StreamCtrl *ctrl);

int http_mpa_create_stream  (StreamCtrl *ctrl);

#define stream_ctrl_create  stream_ctrl_create_v1
int stream_ctrl_create(int ctrl_index, int playrecord, int enablepip, ctrl_create_f ccreate);

void    stream_post_datasock(int idx, int delay);
void    stream_post_msg     (int idx, STRM_MSG msgno, int arg);
void    stream_post_state   (int idx, STRM_STATE state, int scale);
int     stream_deal_cmd     (int idx, StreamCmd *cmd);


void    stream_back_cmd     (int idx, int sn);
void    stream_back_wake    (uint32_t id);

void    stream_back_totaltime   (int idx, uint32_t total);
void    stream_back_currenttime (int idx, uint32_t current);

void    stream_back_totalbyte   (int idx, long long total);
void    stream_back_currentbyte (int idx, long long current);

int     stream_back_get_arg     (int idx, uint32_t magic, PlayArg **arg, char** argbuf);

#ifdef INCLUDE_PVR
void    record_post_msg     (int idx, uint32_t id, STRM_MSG msgno, int arg);
void    record_back_close   (uint32_t id);
int     record_back_exist   (uint32_t id);
int     record_back_wait    (int idx, uint32_t id);
int     record_back_get_arg (int idx, uint32_t magic, RecordArg **arg, char** argbuf);

void strm_async_init(void);


enum {
    ASYNC_CMD_MOUNT = 1,
    ASYNC_CMD_UNMOUNT,
    ASYNC_CMD_DELETE_PVR,
    ASYNC_CMD_DELETE_DIR,
    ASYNC_CMD_DELETE_FILE,
};
void strm_async_cmd(uint32_t id, int cmd, PvrMsgCall call);
void strm_async_delete(int cmd, char *path);
#endif

void stream_back_mosaic(uint32_t magic, int key);

uint32_t int_stream_rrs_timeout(void);

int int_stream_timeshift_jump(void);

int int_stream_cmd(STREAM_CMD cmd, int arg0, int arg1, int arg2, int arg3);
uint32_t int_stream_openmagic(void);
uint32_t int_stream_playmagic(void);
uint32_t int_stream_nextmagic(void);

int int_stream_endclose(void);

mid_mutex_t int_stream_mutex(void);

APP_TYPE int_stream_get_apptype(void);

void int_record_message(int idx, STREAM_CMD cmd, int arg0, int arg1, int arg2, int arg3);

typedef struct __StreamMsgQ    StreamMsgQ;

StreamMsgQ* strm_msgq_create(int size);
void        strm_msgq_delete(StreamMsgQ* msgq);
void        strm_msgq_reset (StreamMsgQ* msgq, int idx);
int         strm_msgq_valid (StreamMsgQ* msgq);
int         strm_msgq_pump  (StreamMsgQ* msgq, StreamMsg* msg);
void        strm_msgq_print (StreamMsgQ* msgq);
void        strm_msgq_queue (StreamMsgQ* msgq, STRM_MSG msgno, int arg);

ind_tlink_t int_stream_tlink(int idx);

StreamMsgQ* int_strm_msgq(int idx);
StreamPlay* int_strm_play(int idx);
int int_strm_playing(int idx);

#ifdef INCLUDE_PVR
StreamRecord* int_strm_record(int idx);
void int_record_delete(uint32_t id);
int int_record_type(int idx);
#endif
void int_back_hls_bufferrate(int bufferrate);
void int_back_hls_recordrate(unsigned int recordrate);
void int_back_hls_playrate(unsigned int playrate);

void int_steam_setAudioChannels(int idx, int audioChannels);
void int_steam_setAudioBitRate(int idx, int audioIndex, int audioBitRate);
void int_steam_setContinuityError(int idx, int continuityError);
void int_steam_setCurBufferSize(int curBufferSize);
void int_steam_setCurSegment(char *url);
void int_steam_setDownloadRate(int downloadRate);
void int_steam_setPacketLost(int idx, int packetLost);
void int_steam_setRemainPlaytime(int remainPlaytime);
void int_steam_setStreamInfo(int idx, int bandwith, char *url);
void int_steam_setStreamNum(int streamNum);
void int_steam_setToalBufferSize(int toalBufferSize);
void int_steam_setTransportProtocol(int idx, int transportProtocol);

void int_steam_postPlay(void);
void int_steam_postAbnormal(void);

void int_back_rtspURL(int idx, char *url);
void int_back_rtspMethod(int idx, char *method);

#define RRS_NUM_4  4

struct StrmRRS {
    char            rrs_name[STREAM_URL_SIZE];
    char*           rrs_uri;

    struct ind_sin  rrs_sins[RRS_NUM_4];
    int             rrs_idx;
    int             rrs_num;
};

typedef struct StrmRRS*    StrmRRS_t;
int strm_tool_parse_url(char* url, StrmRRS_t strmRRS);

void strm_tool_dns_resolve(char* name);
uint32_t strm_tool_dns_find(char* name);

#endif//__STREAM_P_H__
