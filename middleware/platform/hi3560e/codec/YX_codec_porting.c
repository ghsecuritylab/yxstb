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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libzebra.h"
#include "app/Assertions.h"
#include "independs/ind_ts.h"
#include "YX_codec_porting.h"

int YX_SDK_codec_rect_get(int *x, int *y, int *width, int *height)
{
    return yx_drv_videoout_window_get(0, x, y, width, height);
}

int ymm_audio_PCMStart(int handle)
{
    return -1;
}

int ymm_audio_PCMOpenByMixType(int *handle, int MixType, unsigned int MixWeight, int MediaType)
{
    return -1;
}

int ymm_stream_playerStop(int index)
{
    return -1;
}

int ys_cfg_get_mac_addr_from_serialcode(char *serialcode, unsigned char *mac_addr)
{
    return 0;
}

int ys_cfg_set_mac_addr(void *addr)
{
    return 0;
}
