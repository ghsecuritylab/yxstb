/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#ifndef __STRM_PLAY_H__
#define __STRM_PLAY_H__

#include "strm_buffer.h"

typedef struct _StreamPlay StreamPlay;
typedef struct _StreamRecord StreamRecord;

#define INTERVAL_CLK_1000MS             100
#define INTERVAL_CLK_100MS              10
#define INTERVAL_CLK_10MS               1
#define INTERVAL_CLK_DATA_TIMEOUT       (5 * 100)//依据 IPTV STB V100R002C20设计规格.pdf 5s超时
#define INTERVAL_CLK_DATA_TIMEOUT15     (15 * 100)

#if SUPPORTE_HD == 1
#define STREAM_BLOCK_LEVEL_CACHE        (1024 * 10 * 1316)
#define STREAM_BLOCK_LEVEL_PLAY         (1024 * 3 * 1316)
#else
#define STREAM_BLOCK_LEVEL_CACHE        (1024 * 3 * 1316)
#define STREAM_BLOCK_LEVEL_PLAY         (1024 * 1 * 1316)
#endif

#define STREAM_BLOCK_BURST_SIZE         (512 * 1316)

#define STREAM_BLOCK_LEVEL_RECORD       (1024 * 1316)
#define STREAM_BLOCK_LEVEL_PIP          (1024 * 1316)

#define STREAM_BLOCK_LEVEL_BUFFER       (STREAM_BLOCK_LEVEL_PLAY * 3 / 4)


double int_timeNow(void);

typedef void (*MsgCallBack)(void *handle, STRM_MSG msgno, int arg);

StreamPlay* strm_play_create(int codec, int buf_size);

int strm_play_leader_get    (StreamPlay* sp);
void strm_play_leader_set   (StreamPlay* sp, int leader);

int     strm_play_open      (StreamPlay* sp, int leader, void* msg_handle, MsgCallBack msg_callback, APP_TYPE apptype, int size);
void    strm_play_clear     (StreamPlay* sp, int leader);
void    strm_play_pause     (StreamPlay* sp, int leader);
void    strm_play_reset     (StreamPlay* sp, int leader, int caReset);
void    strm_play_resume    (StreamPlay* sp, int leader, int iptv);
void    strm_play_tplay     (StreamPlay* sp, int leader, int scale);
void    strm_play_tskip     (StreamPlay* sp, int leader);
void    strm_play_rect      (StreamPlay* sp, int leader, int x, int y, int width, int height);
void    strm_play_end       (StreamPlay* sp, int leader);
void    strm_play_set_idle     (StreamPlay* sp, int leader, int idle);
void    strm_play_close     (StreamPlay* sp, int leader, int clear);
int     strm_play_time      (StreamPlay* sp);
int     strm_play_diff      (StreamPlay* sp);
int     strm_play_buffer    (StreamPlay* sp);
int     strm_play_forbit    (StreamPlay* sp, int leader, int forbit);

void    strm_play_resize    (StreamPlay* sp, int leader, int size, int resize);
int     strm_play_space     (StreamPlay* sp);
int     strm_play_length    (StreamPlay* sp);
void    strm_play_push      (StreamPlay* sp, int leader, StrmBuffer **psb);

int     strm_play_byte_rate     (StreamPlay* sp);
int     strm_play_byte_percent  (StreamPlay* sp, int leader, uint32_t pktlst);

void    strm_play_emm       (StreamPlay* sp, int *emm_pid, int *emm_num);

int     strm_play_get_psi   (StreamPlay* sp);
int     strm_play_get_width (StreamPlay* sp);

void strm_play_set_timeout  (StreamPlay* sp);
void strm_play_set_flow     (StreamPlay* sp);

void    strm_play_mpeg2ts_set(int flag);

#endif//__STRM_PLAY_H__
