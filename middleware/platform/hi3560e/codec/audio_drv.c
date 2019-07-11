/***************************************************************************
 *     Copyright(c) 2007-2008, Yuxing Software Corporation
 *     All Rights Reserved
 *     Confidential Property of Yuxing Softwate Corporation
 * $Create_Date: 2008-4-2 19:37 $_
 * Revision History:
 * 1. by SunnyLi  2008-4-2 19:38 create

 * $contact at lizhaohui@yu-xing.com
 ***************************************************************************/

#include <stdio.h>

#include "hi_unf_sound.h"
#include "hi_unf_disp.h"
#include "hi_unf_vo.h"
#include "hi_unf_avplay.h"
#include "hi_unf_demux.h"
#include "hi_unf_subtitle.h"

#include "app/Assertions.h"


HI_HANDLE  g_SndHandle = 0;

int yx_drv_audioout_open(void)
{
    HI_UNF_SND_INIT_PARA_S SndPara;
    HI_UNF_SND_INTERFACE_S SndIntf;
    HI_S32 Ret;

    Ret = HI_UNF_SND_Init();
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_SND_Init\n");
    }
    
    SndPara.u32ID = 0;
    Ret = HI_UNF_SND_Create(&SndPara, &g_SndHandle);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_SND_Create\n");
    }

    SndIntf.enInterface = HI_UNF_SND_INTERFACE_I2S;
    SndIntf.bEnable = HI_TRUE;
    Ret = HI_UNF_SND_SetInterface(g_SndHandle, &SndIntf);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_SND_SetInterface\n");
    }

    return 0;
Err:
    return -1;
}

int yx_drv_audioout_mute_get(void)
{
    HI_BOOL Mute;
    HI_S32 Ret;

    Ret = HI_UNF_SND_GetDigitalMute(g_SndHandle, &Mute);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_SND_GetDigitalMute\n");
    }
    return Mute;
Err:
    return -1;
}

int yx_drv_audioout_mute_set(int on_off)
{
    HI_BOOL Mute;
    HI_S32 Ret;

    if(on_off) {
        Mute = HI_TRUE;
    } else {
        Mute = HI_FALSE;
    }
    Ret = HI_UNF_SND_SetDigitalMute(g_SndHandle, Mute);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_SND_SetDigitalMute\n");
    }
    return 0;
Err:
    return -1;
}

int yx_drv_audioout_volume_set(int volume)
{
    HI_S32 Ret;

    PRINTF("volume = %d\n", volume);
    if(volume > 100 || volume < 0) {
        ERR_OUT("volume = %d\n", volume);
    }
    Ret = HI_UNF_SND_SetVolume(g_SndHandle, volume);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_SND_SetVolume\n");
    }
    return 0;
Err:
    return -1;
}

int yx_drv_audioout_volume_get(void)
{
    HI_U32    Volume;
    HI_S32    Ret;

    Ret = HI_UNF_SND_GetVolume(g_SndHandle, &Volume);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_SND_GetVolume\n");
    }
    return Volume;
Err:
    return 0;
}

int yx_drv_audioout_channel_set(int channel)
{
    HI_UNF_TRACK_MODE_E TrackMode;
    HI_S32 Ret;

    switch(channel) {
    case 0:
        TrackMode = HI_UNF_TRACK_MODE_STEREO;
        break;
    case 1:
        TrackMode = HI_UNF_TRACK_MODE_DOUBLE_LEFT;
        break;
    case 2:
        TrackMode = HI_UNF_TRACK_MODE_DOUBLE_RIGHT;
        break;
    default:
        TrackMode = HI_UNF_TRACK_MODE_STEREO;
        break;
    }
    Ret = HI_UNF_SND_SetTrackMode(g_SndHandle, TrackMode);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_SND_SetTrackMode\n");
    }
    return 0;
Err:
    return -1;
}

int yx_drv_audioout_channel_get(void)
{
    HI_UNF_TRACK_MODE_E TrackMode;
    HI_S32 Ret;

    Ret = HI_UNF_SND_GetTrackMode(g_SndHandle, &TrackMode);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_SND_GetTrackMode\n");
    }
    switch(TrackMode) {
    case HI_UNF_TRACK_MODE_STEREO:
        return 0;
    case HI_UNF_TRACK_MODE_DOUBLE_LEFT:
        return 1;
    case HI_UNF_TRACK_MODE_DOUBLE_RIGHT:
        return 2;
    default:
        return 0;
    }
Err:
    return 0;
}

int yx_drv_audioout_set(int enable)
{
    HI_UNF_SND_INTERFACE_S SndIntf;
    HI_S32 Ret;

    if(enable) {
        SndIntf.bEnable = HI_TRUE;
    } else {
        SndIntf.bEnable = HI_FALSE;
    }

    SndIntf.enInterface = HI_UNF_SND_INTERFACE_I2S;
    Ret = HI_UNF_SND_SetInterface(g_SndHandle, &SndIntf);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_SND_SetInterface\n");
    }
    return 0;
Err:
    return -1;
}

