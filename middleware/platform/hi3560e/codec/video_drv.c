/***************************************************************************
 *     Copyright(c) 2007-2008, Yuxing Software Corporation
 *     All Rights Reserved
 *     Confidential Property of Yuxing Softwate Corporation
 * $Create_Date: 2008-4-2 19:37 $
 ***************************************************************************/

#include <stdio.h>

#include "hi_unf_sound.h"
#include "hi_unf_disp.h"
#include "hi_unf_vo.h"
#include "hi_unf_avplay.h"
#include "hi_unf_demux.h"
#include "hi_unf_subtitle.h"

#include "hi_go_gdev.h"

#include "app/Assertions.h"

static HI_HANDLE g_VoHandles[2] = { -1, -1};

int yx_drv_videoout_open(int index)
{
    HI_HANDLE				handle;
    HI_UNF_VO_INIT_PARA_S	VoPara;
    HI_S32					Ret;

    PRINTF("#%d ++++\n", index);

    if(index != 0 && index != 1)
        ERR_OUT("#%d!\n", index);

    handle = g_VoHandles[index];
    if(handle != -1)
        ERR_OUT("#%d already inited!\n", index);

    if(index == 0)
        VoPara.u32ID = 0;
    else
        VoPara.u32ID = 1;

    Ret = HI_UNF_VO_CreateLayer(&VoPara, &handle);
    if(Ret != HI_SUCCESS)
        ERR_OUT("#%d HI_UNF_VO_CreateLayer\n", index);

    g_VoHandles[index] = handle;

    return 0;
Err:
    return -1;
}

HI_HANDLE yx_drv_videoout_handle(int index)
{
    return g_VoHandles[index];
}

#define CHECK_INDEX( )							\
	HI_HANDLE	handle;							\
	if (index != 0 && index != 1)				\
		ERR_OUT("index = %d\n", index);			\
	handle = g_VoHandles[index];				\
	if (handle == -1)							\
		ERR_OUT("#%d handle is error\n", index)	\
 
int yx_drv_videoout_window_set(int index, int x, int y, int width, int height)
{
    HI_UNF_WINDOW_S		VoWindow;
    HI_S32				Ret;

    CHECK_INDEX();

    Ret = HI_UNF_VO_GetOutputWindow(handle, &VoWindow);
    if(Ret != HI_SUCCESS)
        ERR_OUT("#%d HI_UNF_VO_GetOutputWindow\n", index);

    if(VoWindow.u32X == x && VoWindow.u32Y == y && VoWindow.u32Width == width && VoWindow.u32Height == height)
        return 0;

    VoWindow.u32X = x;
    VoWindow.u32Y = y;
    VoWindow.u32Width = width;
    VoWindow.u32Height = height;
    Ret = HI_UNF_VO_SetOutputWindow(handle, &VoWindow);
    if(Ret != HI_SUCCESS)
        ERR_OUT("#%d HI_UNF_VO_SetOutputWindow\n", index);

    return 0;
Err:
    return -1;
}

int yx_drv_videoout_window_get(int index, int *x, int *y, int *width, int *height)
{
    HI_UNF_WINDOW_S		VoWindow;
    HI_S32				Ret;

    CHECK_INDEX();

    Ret = HI_UNF_VO_GetOutputWindow(handle, &VoWindow);
    if(Ret != HI_SUCCESS)
        ERR_OUT("#%d HI_UNF_VO_GetOutputWindow\n", index);

    if(x)
        *x = VoWindow.u32X;
    if(y)
        *y = VoWindow.u32Y;

    if(width)
        *width = VoWindow.u32Width;
    if(height)
        *height = VoWindow.u32Height;

    return 0;
Err:
    return -1;
}

int yx_drv_videoout_show(int index, int enable)
{
    HI_U32		Alpha;
    HI_S32		Ret;

    CHECK_INDEX();

    if(enable)
        Alpha = 0;
    else
        Alpha = 100;

    Ret = HI_UNF_VO_SetAlpha(handle, Alpha);
    if(Ret != HI_SUCCESS)
        ERR_OUT("#%d HI_UNF_VO_SetAlpha\n", index);

    return 0;
Err:
    return -1;
}

int yx_drv_videoout_set(int index, int enable)
{
    HI_BOOL		Enable;
    HI_S32		Ret;

    if(enable)
        Enable = HI_TRUE;
    else
        Enable = HI_FALSE;

    CHECK_INDEX();

    Ret = HI_UNF_VO_SetEnable(handle, Enable);
    if(Ret != HI_SUCCESS)
        ERR_OUT("#%d HI_UNF_VO_SetEnable\n", index);

    return 0;
Err:
    return -1;
}
