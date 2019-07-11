/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#ifndef __STRM_RECORD_H__
#define __STRM_RECORD_H__

#ifdef INCLUDE_PVR

#include "strm_play.h"

StreamRecord* strm_record_create(int idx, int buf_size);

void    strm_record_clean       (int clean);

void    strm_record_push        (StreamRecord* sr, char *buf, int len, uint32_t clk);
void    strm_record_stamp       (StreamRecord* sr);

void    strm_record_reserve     (uint32_t bitrate, uint32_t second);
uint32_t    strm_record_shifttime   (void);
int     strm_record_size        (void);
int     strm_timeshift_size     (void);
int     strm_record_space       (StreamRecord* sr);

#define strm_record_open strm_record_open_v3
void    strm_record_open        (StreamRecord* sr, void* msg_handle, MsgCallBack rec_callback, RecordArg *arg);
void    strm_record_netwkid     (StreamRecord* sr, int netwkid);
void    strm_record_end         (StreamRecord* sr);
void    strm_record_close       (StreamRecord* sr, int end);
void    strm_record_arg         (StreamRecord* sr, PvrArgument_t arg);

void    strm_record_pause       (StreamRecord* sr);
void    strm_record_resume      (StreamRecord* sr);

void    strm_record_mix_open    (StreamRecord* sr, RecordArg *arg);
void    strm_record_mix_close   (StreamRecord* sr, uint32_t id, int end);

void strm_record_break_point    (StreamRecord* sr);

void strm_record_emm(StreamRecord* sr, int *emm_pid, int *emm_num);

#endif//INCLUDE_PVR

#endif//__STRM_RECORD_H__
