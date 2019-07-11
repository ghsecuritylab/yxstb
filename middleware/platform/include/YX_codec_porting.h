/***************************************************************************
 *     Copyright (c) 2005-2009, Yuxing Software Corporation
 *     All Rights Reserved
 *     Confidential Property of Yuxing Softwate Corporation
 *
 * $ys_Workfile: YX_codec_porting.h $
 * $ys_Revision: 01 $
 * $ys_Date: 02/27/09 20:10 $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $ys_Log: YX_codec_porting.c $
 *
 * 1. create by mabaoquan  02/27/2009
 *
 ***************************************************************************/
#ifndef __YX_CODEC_PORTING_H__
#define __YX_CODEC_PORTING_H__

#include "libzebra.h"

typedef int (*YX_codec_callback)(void* arg);

typedef int (*YX_codec_show)( int show_flag);

typedef int (*YX_codec_getbuffer)( char** buf, int* len,unsigned int *buf_phyaddr);

typedef int (*YX_codec_putbuffer)( char* buf,int len);
typedef int (*YX_codec_start)(YX_MPEG_INFO* mpeg_info);
typedef int (*YX_codec_pause)(void);
typedef int (*YX_codec_pts)( unsigned int* ppts);
typedef int (*YX_codec_fast)(void);
typedef int (*YX_codec_resume)(void);
typedef int (*YX_codec_stop)(void);
typedef int (*YX_codec_flush)(void);
typedef int (*YX_codec_rect)(int x, int y, int w, int h);
typedef int (*YX_codec_setAudioTrack)(int audio_pid, int audio_type);
typedef int (*YX_codec_stopMode)(int mode);

typedef struct __CodecInterface
{
  YX_codec_show				decoder_show;
  YX_codec_getbuffer		decoder_getbuffer;
  YX_codec_putbuffer		decoder_putbuffer;
  YX_codec_start 			decoder_start;
  YX_codec_pause			decoder_pause;
  YX_codec_pts				decoder_pts;
  YX_codec_fast 			decoder_fast;
  YX_codec_resume			decoder_resume;
  YX_codec_stop				decoder_stop;
  YX_codec_flush			decoder_flush;
  YX_codec_rect				decoder_rect;
  YX_codec_stopMode			decoder_stopMode;
}Codec_interface;

#ifdef __cplusplus
extern "C" {
#endif

int YX_codec_create(Codec_interface* codec, void* construct_callback);

#define LOCAL_DECODE_TEST

int YX_SDK_codec_construct_master(Codec_interface* codec);
int YX_SDK_codec_construct_pip(Codec_interface* codec);

int YX_SDK_codec_show(int show);

int YX_SDK_codec_getbuffer(char** buf, int* len, unsigned int *buf_phyaddr);

int YX_SDK_codec_putbuffer(char* buf,int len);
int YX_SDK_codec_start(YX_MPEG_INFO* mpeg_info);
int YX_SDK_codec_pause(void);
int YX_SDK_codec_pts(u_int* ppts);
int YX_SDK_codec_fast(void);
int YX_SDK_codec_resume(void);
int YX_SDK_codec_stop(void);
int YX_SDK_codec_flush(void);
int YX_SDK_codec_rect_get(int *x, int *y, int *width, int *height);
int YX_SDK_codec_rect(int x, int y, int width, int height);
int YX_SDK_codec_stopMode(int mode);

int YX_SDK_codec_pip_show(int show);
int YX_SDK_codec_pip_getbuffer(char** buf, int *len);
int YX_SDK_codec_pip_putbuffer(char* buf,int len);
int YX_SDK_codec_pip_start(YX_MPEG_INFO* mpeg_info);
int YX_SDK_codec_pip_pause(void);
int YX_SDK_codec_pip_pts( u_int* ppts);
int YX_SDK_codec_pip_fast(void);
int YX_SDK_codec_pip_resume(void);
int YX_SDK_codec_pip_stop(void);
int YX_SDK_codec_pip_flush(void);
int YX_SDK_codec_pip_rect(int x, int y, int width, int height);
int YX_SDK_codec_pip_stopMode(int mode);

#ifdef __cplusplus
}
#endif

#endif
