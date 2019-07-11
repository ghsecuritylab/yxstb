/***************************************************************************
 *     Copyright (c) 2005-2009, Yuxing Software Corporation
 *     All Rights Reserved
 *     Confidential Property of Yuxing Softwate Corporation
 *
 * $ys_Workfile: YX_codec_porting.c $
 * $ys_Revision: 01 $
 * $ys_Date: 02/27/09 19:10 $
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
#include "YX_codec_porting.h"

#include "app/Assertions.h"
#include "independs/ind_ts.h"

#include "libzebra.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/*********************************************************************
 * Codec_create(int codec_flag, void* construct_callback)
 *
 * Descript:
 *   初始化codec结构体。
 *
 * Input Parameters:
 *
 *
 * Return Value:
 *   0 : Successd , other  : failed.
 *
 ****************************************************************/
int YX_codec_create(Codec_interface* codec, void* construct_callback)
{
	memset(codec,0,sizeof(Codec_interface));

	if(construct_callback)
		((YX_codec_callback)construct_callback)((void*)codec);

	return 0;
}

int YX_SDK_codec_construct_master(Codec_interface* codec)
{
	if(!codec)
		return -1;

	codec->decoder_getbuffer = (YX_codec_getbuffer)YX_SDK_codec_getbuffer;
	codec->decoder_putbuffer = (YX_codec_putbuffer)YX_SDK_codec_putbuffer;
	codec->decoder_start = (YX_codec_start)YX_SDK_codec_start;
	codec->decoder_pts = (YX_codec_pts)YX_SDK_codec_pts;
	codec->decoder_stop = (YX_codec_stop)YX_SDK_codec_stop;
	codec->decoder_rect = (YX_codec_rect)YX_SDK_codec_rect;
	codec->decoder_stopMode = (YX_codec_stopMode)YX_SDK_codec_stopMode;

	return 0;
}

int YX_SDK_codec_construct_pip(Codec_interface* codec)
{
	if(!codec)
		return -1;

	codec->decoder_getbuffer = (YX_codec_getbuffer)YX_SDK_codec_pip_getbuffer;
	codec->decoder_putbuffer = (YX_codec_putbuffer)YX_SDK_codec_pip_putbuffer;
	codec->decoder_start = (YX_codec_start)YX_SDK_codec_pip_start;
	codec->decoder_pts = (YX_codec_pts)YX_SDK_codec_pip_pts;
	codec->decoder_stop = (YX_codec_stop)YX_SDK_codec_pip_stop;
	codec->decoder_rect = (YX_codec_rect)YX_SDK_codec_pip_rect;
	codec->decoder_stopMode = (YX_codec_stopMode)YX_SDK_codec_pip_stopMode;

	/* 初始化pip decoder,解决第一个pip频道就是高清频道出现不能正常播放的问题 */
    printf("SDK: ymm_pip_init\n");

#ifdef INCLUDE_PIP
	ymm_pip_init();
	ymm_pip_set_show(0);
#endif
	//YX_SDK_codec_pip_rect(sysPip_get_positionx(),sysPip_get_positiony(), sysPip_get_width(), sysPip_get_height());

	return 0;
}

#ifdef LOCAL_DECODE_TEST

//static int g_checkHD = 0;

int YX_SDK_codec_getbuffer(char** buf, int* len, unsigned int *buf_phyaddr)
{
	//return ymm_decoder_getBuffer(buf,len);
    return ymm_decoder_getBufferExt(buf, len, buf_phyaddr);
}

/* 得到pip decoder的buffer */
int YX_SDK_codec_pip_getbuffer(char** buf, int* len)
{
	return ymm_pip_get_playbuffer((unsigned char**)buf,len);
}

/* 阻塞等待decoder使用完这部分数据 */
int YX_SDK_codec_putbuffer(char* buf,int len)
{
		return ymm_decoder_pushBuffer(buf,len);
}


int YX_SDK_codec_pip_putbuffer(char* buf,int len)
{
	return ymm_pip_push_playbuffer((unsigned char*)buf,len);
}


int YX_SDK_codec_start(YX_MPEG_INFO* mpeg_info)
{
	ymm_decoder_setMpegInfo(mpeg_info);
    printf("SDK: ymm_decoder_startBuffer\n");
    ymm_decoder_startBuffer();
	return 0;
}

int YX_SDK_codec_pip_start(YX_MPEG_INFO* mpeg_info)
{
	ymm_pip_set_mpeg_info(mpeg_info);
    printf("SDK: ymm_pip_start_playbuffer\n");
	ymm_pip_start_playbuffer();
	return 0;
}


/*使用空函数,当前没有使用*/
int YX_SDK_codec_pts(u_int *ppts)
{
	static int source_width = 0;

	if (ppts) {
		int video_pid = 0;

		ymm_decoder_getVideoPid(&video_pid);

		if(video_pid) {
			YX_VIDEO_DECODER_STATUS status;
			status.pts = 0;
			ymm_decoder_getVideoStatus(&status);
			if (!source_width && status.pts) {
				source_width = status.source_width;
				PRINTF("decoder width = %d, height = %d\n", status.source_width, status.source_height);
			}
			*ppts = status.pts;
		} else {
			YX_AUDIO_DECODER_STATUS status;
			status.pts = 0;
			ymm_decoder_getAudioStatus(&status);
			*ppts = status.pts;
		}
	} else {
		source_width = 0;
	}

	return 0;
}

/* pip 只有图像没有声音,返回video pts */
int YX_SDK_codec_pip_pts(u_int *ppts)
{
	static int source_width = 0;

	if (ppts) {
			YX_VIDEO_DECODER_STATUS status;
			status.pts = 0;
			ymm_pip_getVideoStatus(&status);
			if (!source_width && status.pts) {
				source_width = status.source_width;
				PRINTF("pip width = %d, height = %d\n", status.source_width, status.source_height);
			}
			*ppts = status.pts;
	} else {
		source_width = 0;
	}

	return 0;
}

int YX_SDK_codec_stop()
{
    printf("SDK: ymm_decoder_stopBuffer\n");
	ymm_decoder_stopBuffer();
	return 0;
}

int YX_SDK_codec_pip_stop()
{
    printf("SDK: ymm_pip_stop_playbuffer\n");
	ymm_pip_stop_playbuffer();
	return 0;
}

int YX_SDK_codec_rect(int x, int y, int width, int height)
{
	int s_width = 0, s_height = 0;

	ygp_layer_getScreenSize(&s_width, &s_height);

	if (x < 0 || x >= s_width || y < 0 || y >= s_height
	    || width < 0 || width > s_width	|| height < 0 || height > s_height
		|| x + width > s_width || y + height > s_height) {
		return -1;
	}
	return yhw_vout_setVideoDisplayRect(x, y, width, height);
}

int YX_SDK_codec_rect_get(int *x, int *y, int *width, int *height)
{
    return yhw_vout_getVideoDisplayRect(x, y, width, height);
}


int YX_SDK_codec_pip_rect(int x, int y, int width, int height)
{
#ifdef INCLUDE_PIP
	int s_width, s_height;

	ygp_layer_getScreenSize(&s_width, &s_height);

	if (x < 0 || x >= s_width || width < 0 || width > s_width
			|| y < 0 || y >= s_height || height < 0 || height > s_height
			|| x + width > s_width || y + height > s_height)
		return -1;

	printf("[%s]\n,%d:%d %d:%d %d:%d\n",__FUNCTION__,x,y,width,s_width, height, s_height);

	if((x == 0) && (y == 0) && (width == 0) && (height == 0))
		return ymm_pip_set_displaysize(0,0,s_width, s_height);
	else
		return ymm_pip_set_displaysize(x,y,width,height);
#endif
	return 0;
}

int YX_SDK_codec_stopMode(int mode)
{
  return ymm_decoder_setStopMode(mode);
}

int YX_SDK_codec_pip_stopMode(int mode)
{
  return ymm_pip_setStopMode(mode);
}


#endif
