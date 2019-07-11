/***************************************************************************
 *     Copyright (c) 2007-2008, Yuxing Software Corporation
 *     All Rights Reserved
 *     Confidential Property of Yuxing Softwate Corporation
 * $Create_Date: 2008-4-2 19:37 $
 * Revision History:
 * 1. by SunnyLi  2008-4-2 19:38 create

 * $contact at lizhaohui@yu-xing.com
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#include "hi_unf_sound.h"
#include "hi_unf_disp.h"
#include "hi_unf_vo.h"
#include "hi_unf_avplay.h"
#include "hi_unf_demux.h"
#include "hi_unf_subtitle.h"
#include "hi_video.h"
#include "hi_audio.h"

#include "libzebra.h"

#include "SettingListener.h"

#include "codec.h"
#include "independs/ind_ts.h"
#include "app/Assertions.h"
#include "config/pathConfig.h"

#ifdef ENABLE_CA_VERIMATRIX
#include "ys_vmdrm_hw.h"
#endif

#include "Assertions.h"


//#define CATCHTS
#ifndef uchar
#define uchar unsigned char
#endif

enum {
    CODEC_TYPE_TS = 0,
    CODEC_TYPE_PCM,
};

enum {
    CODEC_STATE_CLOSE = 0,
    CODEC_STATE_OPEN,
    CODEC_STATE_PLAY,
    CODEC_STATE_PAUSE,
    CODEC_STATE_TPLAY
};

struct StrmCodec {
    int	type;
    int	mini;
    int	iframe;
    int	state;
    int	space;
    int	frame_flg;
    int	reset_flg;

    struct ts_psi ts_psi;
    struct ts_ca ts_ca;
    uint ts_ca_flg;
    uint ts_pts;

    int ts_track;
    int buf_state;
    int pn_mode;
    int rect_x;
    int rect_y;
    int rect_width;
    int rect_height;

    HI_HANDLE handle;
    HI_UNF_AVPLAY_BUF_S avplay_buf;
    HI_UNF_AVPLAY_MEDIA_CHAN_E avplay_chan;

#ifdef ENABLE_CA_VERIMATRIX
    void *ca_vmHandle;
#endif

    SAMPLE_RATE_E           sample_rate;    /*Sampling rate.*/
    VIDEO_FIELD_MODE_E      field_mode;   /*Field mode, such as frame, top field, or bottom field.*/
    VIDEO_ASPECT_RATIO_E    aspect_ratio; /*Output aspect ratio.*/
    HI_U32                  width; /*Width of the original image.*/
    HI_U32                  height;/*Height of the original image.*/
};


extern HI_HANDLE g_SndHandle;

#ifdef CATCHTS
static FILE *savefp = NULL;
static int savesn = 0;
#endif
static int g_ignore = 0;
static pthread_mutex_t	g_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct StrmCodec *g_codecs[2] = {NULL, NULL};


extern HI_HANDLE yx_drv_videoout_handle(int index);
extern unsigned int mid_10ms();

static void codec_pcm_close_in(int pIndex);
static void codec_close_in(int pIndex, int clear);
static void codec_create_in(int pIndex);
static void codec_destroy_in(int pIndex);


static void codec_init_in(int pIndex, int pnmode);

int mid_audio_mute_get(void)
{
	return yx_drv_audioout_mute_get();
}

int mid_audio_mute_set(int mute)
{
	if(mute) {
		yx_drv_audioout_mute_set(1);
		yx_hi_gpio_audio_mute(1);
	} else {
		yx_drv_audioout_mute_set(0);
		yx_hi_gpio_audio_mute(0);
	}

	return 0;
}

int mid_audio_volume_set(int vol)
{
	if(vol < 0 || vol > AUDIO_VOLUME_MAX) {
		LogUserOperError("set volume (%d)\n", vol);
		return 1;
	}

	yx_drv_audioout_volume_set(vol);
	return 0;
}

int mid_audio_volume_get(void)
{
	return yx_drv_audioout_volume_get( );
}


int codec_init(int pnmode)
{
    HI_S32 Ret;

    //yhw_board_init();

    yx_drv_audioout_open();
    yx_drv_display_open(pnmode);

    HI_UNF_VO_Init();

    yx_drv_videoout_open(0);

    demux_init();

    Ret = HI_UNF_AVPLAY_Init();
    if(Ret != HI_SUCCESS) {
        LogUserOperError("HI_UNF_AVPLAY_Init failed.\n");
    }
    codec_init_in(0, pnmode);
    codec_init_in(1, pnmode);

    yx_drv_display_logo_hide(); // Hidden logo display by u-boot.This layer is mouse layer on the hisi 3560E.
    {
        int vol = 0;
        appSettingGetInt("volume", &vol, 0);
        mid_audio_volume_set(vol);
    }

    SettingListenerRegist("mute", 0, muteListenFunc);
    return 0;
}

static int muteListenFunc(const char* name, const char* value)
{
    int mute = atoi(value);
    mid_audio_mute_set(mute);
    return 0;
}

static int demux_init(void)
{
    HI_S32					Ret;
    HI_UNF_DMX_GLB_ATTR_S	DmxGlbAttr;
    HI_UNF_DMX_ATTR_S 		DmxAttr;
    HI_UNF_DMX_PORT_ATTR_S	PortAttr;

    /* open DEMUX device, enable it*/
    Ret = HI_UNF_DMX_GetGlbDefaultAttr(&DmxGlbAttr);
    if(Ret != HI_SUCCESS)
        ERR_OUT("HI_UNF_DMX_GetGlbDefaultAttr\n");

    Ret = HI_UNF_DMX_Init(&DmxGlbAttr);
    if(Ret != HI_SUCCESS)
        ERR_OUT("HI_UNF_DMX_Init\n");

    Ret = HI_UNF_DMX_Open(0);
    if(Ret != HI_SUCCESS)
        ERR_OUT("HI_UNF_DMX_Open\n");

    DmxAttr.u32PortId = 0;

    Ret = HI_UNF_DMX_SetAttr(0, &DmxAttr);
    if(Ret != HI_SUCCESS)
        ERR_OUT("HI_UNF_DMX_SetAttr\n");

    Ret = HI_UNF_DMX_GetTSPortAttr(DmxAttr.u32PortId, &PortAttr);
    if(Ret != HI_SUCCESS)
        ERR_OUT("HI_UNF_DMX_GetTSPortAttr\n");

    PortAttr.enPortMod = HI_UNF_DMX_PORT_MODE_RAM;
    PortAttr.u32TunerErrMod = 1;
    PortAttr.enPortType = HI_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188;
    PortAttr.u32SyncLostTh = 0;
    PortAttr.u32SyncLockTh = 0;
    Ret = HI_UNF_DMX_SetTSPortAttr(DmxAttr.u32PortId, &PortAttr);
    if(Ret != HI_SUCCESS)
        ERR_OUT("HI_UNF_DMX_SetTSPortAttr\n");

    Ret = HI_UNF_DMX_OpenTSPort(DmxAttr.u32PortId);
    if(Ret != HI_SUCCESS)
        ERR_OUT("HI_UNF_DMX_OpenTSPort\n");

    return 0;
Err:
    return -1;
}

static HI_S32 event_proc(HI_HANDLE Handle, HI_UNF_AVPLAY_EVENT_E msg, HI_U32 param)
{
    struct StrmCodec *codec = g_codecs[0];

    switch(msg) {
    case HI_UNF_AVPLAY_EVENT_NEW_VID_FRAME:
        if(param == 0)
            ERR_OUT("EP_EVT_NEW_VID_FRAME\n");
        {
            VO_FRAMEINFO_S *info = (VO_FRAMEINFO_S *)param;

            codec->field_mode = info->field_mode;
            codec->aspect_ratio = info->aspect_ratio;
            codec->width = info->width;
            codec->height = info->height;

            if (codec->ts_psi.video_pid)
                codec->ts_pts = (unsigned int)(info->pts >> 1);
        }
        break;
    case HI_UNF_AVPLAY_EVENT_NEW_AUD_FRAME:
        {
            AO_FRAMEINFO_S *info = (AO_FRAMEINFO_S *)param;

            codec->sample_rate = info->sample_rate;

            if (codec->ts_psi.video_pid == 0)
                codec->ts_pts = (unsigned int)(info->pts >> 1);
        }
        break;
    default:
        break;
    }

    return 0;
Err:
    return -1;
}

static HI_S32 event_proc_pip(HI_HANDLE Handle, HI_UNF_AVPLAY_EVENT_E msg, HI_U32 param)
{
    struct StrmCodec *codec = g_codecs[1];
    switch(msg) {
    case HI_UNF_AVPLAY_EVENT_NEW_VID_FRAME:
        if (param == 0)
            ERR_OUT("EP_EVT_NEW_VID_FRAME\n");
        {
            VO_FRAMEINFO_S *info = (VO_FRAMEINFO_S *)param;

            codec->field_mode = info->field_mode;
            codec->aspect_ratio = info->aspect_ratio;
            codec->width = info->width;
            codec->height = info->height;

            if (codec->ts_psi.video_pid)
                codec->ts_pts = (unsigned int)(info->pts >> 1);
        }
        break;
    default:
        break;
    }

    return 0;
Err:
    return -1;
}

static void codec_init_in(int pIndex, int pnmode)
{
    struct StrmCodec *codec;

    if(pIndex < 0 || pIndex > 1)
        ERR_OUT("pIndex = %d!\n", pIndex);
    if(g_codecs[pIndex])
        ERR_OUT("#%d codec exist!\n", pIndex);

    codec = (struct StrmCodec *)malloc(sizeof(struct StrmCodec));
    if(codec == NULL)
        ERR_OUT("#%d malloc\n", pIndex);

    memset(codec, 0, sizeof(struct StrmCodec));

    codec->pn_mode = pnmode;
    codec->handle = -1;
    codec->type = CODEC_TYPE_PCM;
    codec->state = CODEC_STATE_CLOSE;

#ifdef ENABLE_CA_VERIMATRIX
    if(pIndex == 0)
        codec->ca_vmHandle = ymm_vmdrmDecryptInitContext(1, 0);
#endif

    g_codecs[pIndex] = codec;
Err:
    return;
}

void codec_mini(int pIndex, int mini)
{
    struct StrmCodec *codec;

    PRINTF("codec index(%d) mini(%d)\n", pIndex, mini);
    if(pIndex != 0) {
        ERR_PRN("codec index(%d) error\n", pIndex);
        return;
    }

    codec = g_codecs[0];

    pthread_mutex_lock(&g_mutex);
    if(codec->type == CODEC_TYPE_TS) {
        codec_close_in(pIndex, 1);
        codec_destroy_in(pIndex);
    } else if(codec->state != CODEC_STATE_CLOSE) {
        ERR_PRN("#%d state = %d\n", pIndex, codec->state);
        codec_pcm_close_in(pIndex);
    }
    codec->type = CODEC_TYPE_PCM;
    codec->mini = mini;
    pthread_mutex_unlock(&g_mutex);
    return;
}

void codec_iframe(int pIndex, int iframe)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("codec(%d) set iframe(%d)\n", pIndex, iframe);
    pthread_mutex_lock(&g_mutex);
    codec->iframe = iframe;
    pthread_mutex_unlock(&g_mutex);
}

void codec_ignore(int pIndex, int ignore)
{
    PRINTF("codec(%d) set ignore(%d)\n", pIndex, ignore);
    g_ignore = ignore;
}

static void codec_create_in(int pIndex)
{
    HI_S32						Ret;
    HI_UNF_AVPLAY_SYNC_ATTR_S	syncAttr;
    HI_UNF_AVPLAY_ATTR_S		avAttr;
    HI_HANDLE					handle;
    HI_UNF_VO_ATTACH_PARA_S		voPara;
    struct StrmCodec *codec = g_codecs[pIndex];

    if(codec == NULL)
        ERR_OUT("codec(%d) is NULL\n", pIndex);
    if(codec->handle != -1)
        ERR_OUT("codec(%d) handle is created\n", pIndex);
    PRINTF("codec(%d) mini is %d\n", pIndex, codec->mini);

    //更新了新的库后，需要在此处设置即将要创建的播放器需不需要同步，需要同步就是主播放器
    memset(&avAttr, 0, sizeof(HI_UNF_AVPLAY_ATTR_S));
    if(pIndex == 0)
        avAttr.enPlayType = HI_UNF_AVPLAY_TYPE_MAIN;
    else
        avAttr.enPlayType = HI_UNF_AVPLAY_TYPE_MINOR;
    avAttr.stStreamAttr.enStreamType = HI_UNF_AVPLAY_STREAM_TYPE_TS;
    HI_UNF_AVPLAY_GetDefaultConfig(&avAttr, HI_UNF_AVPLAY_STREAM_TYPE_TS);

    if(codec->mini == 0 && pIndex == 0)
        avAttr.stStreamAttr.unStreamAttr.stTsAttr.u32BufferSize = 2 * 1024 * 1024;
    else
        avAttr.stStreamAttr.unStreamAttr.stTsAttr.u32BufferSize = 1024 * 1024;
    if(HI_SUCCESS != HI_UNF_AVPLAY_Create(&avAttr, &handle))
        ERR_OUT("#%d HI_UNF_AVPLAY_Create failed.\n", pIndex);

    if(pIndex == 0)
        HI_UNF_SND_Attach(g_SndHandle, handle, 100);

    voPara.eSourceType = HI_UNF_VO_SOURCE_TYPE_AVPLAY;
    if(pIndex == 0) {
        voPara.stSourcePara.u32StartX = 0;
        voPara.stSourcePara.u32StartY = 0;
        voPara.stSourcePara.u32Width = 720;
        voPara.stSourcePara.u32Heigth = 576;
        voPara.stSourcePara.u32Alpha = 255;
    } else {
        voPara.stSourcePara.u32StartX = codec->rect_x;
        voPara.stSourcePara.u32StartY = codec->rect_y;
        voPara.stSourcePara.u32Width = codec->rect_width;
        voPara.stSourcePara.u32Heigth = codec->rect_height;
        voPara.stSourcePara.u32Alpha = 0;
    }

    HI_HANDLE voHandle = yx_drv_videoout_handle(pIndex);
    if(voHandle != -1) {
        HI_UNF_VO_Attach(voHandle, handle, &voPara);
        HI_UNF_VO_SetEnable(voHandle, HI_TRUE);
    }

    Ret = HI_UNF_AVPLAY_ChnOpen(handle, HI_UNF_AVPLAY_MEDIA_CHAN_VID, NULL);
    if(Ret != HI_SUCCESS)
        ERR_PRN("HI_UNF_AVPLAY_ChnOpen HI_UNF_AVPLAY_MEDIA_CHAN_VID\n");
    if(codec->mini == 0) {
        if (1 == index)
            HI_UNF_AVPLAY_RegisterEvent(handle, HI_UNF_AVPLAY_EVENT_NEW_VID_FRAME, event_proc_pip);
        else
            HI_UNF_AVPLAY_RegisterEvent(handle, HI_UNF_AVPLAY_EVENT_NEW_VID_FRAME, event_proc);
    }

    Ret = HI_UNF_AVPLAY_ChnOpen(handle, HI_UNF_AVPLAY_MEDIA_CHAN_AUD, NULL);
    if(Ret != HI_SUCCESS)
        ERR_PRN("HI_UNF_AVPLAY_ChnOpen HI_UNF_AVPLAY_MEDIA_CHAN_AUD\n");
    if(codec->mini == 0 && pIndex == 0)
        HI_UNF_AVPLAY_RegisterEvent(handle, HI_UNF_AVPLAY_EVENT_NEW_AUD_FRAME, event_proc);

    HI_UNF_AVPLAY_GetAttr(handle, HI_UNF_AVPLAY_ATTR_ID_SYNC, &syncAttr);
    if(pIndex == 0)
        syncAttr.enSyncRef = HI_UNF_AVPLAY_SYNC_REF_AUDIO;
    else
        syncAttr.enSyncRef = HI_UNF_AVPLAY_SYNC_REF_NONE;
    syncAttr.enPreSyncMode = HI_UNF_AVPLAY_PRESYNC_MODE_NONE;
    syncAttr.enRealMode = HI_FALSE;
    syncAttr.bSmoothPlay = HI_TRUE;
    HI_UNF_AVPLAY_SetAttr(handle, HI_UNF_AVPLAY_ATTR_ID_SYNC, &syncAttr);

    codec->handle = handle;
    codec->type = CODEC_TYPE_TS;
Err:
    return;
}

static void codec_destroy_in(int pIndex)
{
    HI_HANDLE handle;
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("destroy codec(%d)\n", pIndex);
    handle = codec->handle;
    HI_UNF_AVPLAY_ChnClose(handle, HI_UNF_AVPLAY_MEDIA_CHAN_AUD);
    if(codec->mini == 0)
        HI_UNF_AVPLAY_ChnClose(handle, HI_UNF_AVPLAY_MEDIA_CHAN_VID);

    if(pIndex == 0)
        HI_UNF_SND_Detach(g_SndHandle, handle);

    HI_UNF_VO_Detach(yx_drv_videoout_handle(pIndex), handle);
    HI_UNF_AVPLAY_Destroy(handle);
    codec->handle = -1;
    return;
}

/* 由mid_stream_open直接调用，用于同步设置，入音轨 */
void codec_prepare(void)
{
}

void codec_open(int pIndex, int iptv)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("open codec(%d)\n", pIndex);

    pthread_mutex_lock(&g_mutex);

    if(codec->type == CODEC_TYPE_PCM) {
        codec_pcm_close_in(pIndex);
        codec_create_in(pIndex);
    }
    else if(codec->state != CODEC_STATE_CLOSE) {
        ERR_PRN("#%d state = %d\n", pIndex, codec->state);
        codec_close_in(pIndex, 0);
    }

    codec->ts_track = 0;
    codec->state = CODEC_STATE_OPEN;
    codec->space = 0;
    codec->reset_flg = 0;
    codec->ts_ca.system_id = 0;
    codec->ts_ca_flg = 0;
    codec->ts_pts = 0;

    if(pIndex == 1)
        yx_drv_videoout_set(1, 1);
    pthread_mutex_unlock(&g_mutex);
    return 0;
}

int codec_buf_get(int pIndex, char **pbuf, int *plen)
{
    int len;
    HI_S32 Ret;
    struct StrmCodec *codec = g_codecs[pIndex];

    if(codec->type != CODEC_TYPE_TS)
        ERR_OUT("#%d type = %d\n", pIndex, codec->type);
    if(codec->state != CODEC_STATE_PLAY && codec->state != CODEC_STATE_TPLAY)
        goto Err;
    if(pbuf == NULL || plen == NULL)
        ERR_OUT("#%d bad parameter pbuf = %p, plen = %p\n", pIndex, pbuf, plen);

    Ret = HI_UNF_AVPLAY_GetBuf(codec->handle, HI_UNF_AVPLAY_BUF_ID_TS, (8 * 1316), &(codec->avplay_buf), 0);
    if(Ret != HI_SUCCESS) {
        *plen = 0;
        return 0;
    }

    *pbuf = codec->avplay_buf.pu8Data;
    *plen = codec->avplay_buf.u32Size;
    return 0;
Err:
    return -1;
}

int codec_buf_put(int pIndex, int len)
{
    HI_S32 Ret;
    struct StrmCodec *codec = g_codecs[pIndex];

    if(codec->type != CODEC_TYPE_TS)
        ERR_OUT("#%d type = %d\n", pIndex, codec->type);
    if(g_ignore) {
        if(g_ignore == 1 && codec->state == CODEC_STATE_PLAY)
            return 0;
        if(g_ignore == 2 && codec->state == CODEC_STATE_TPLAY)
            return 0;
    }

    switch(codec->state) {
    case CODEC_STATE_PLAY:
    case CODEC_STATE_TPLAY: {
        if(codec->ts_ca_flg) {
            if(codec->ts_ca_flg == 1) {
                int l;
                uint pid;
                uchar *ubuf;

                ubuf = (uchar *)codec->avplay_buf.pu8Data;
                for(l = 0; l < len; l += 188) {
                    pid = (((unsigned int)ubuf[1] & 0x1f) << 8) + ubuf[2];
                    if(pid != (unsigned int)codec->ts_ca.pid)
                        break;
                }
                if(l > 0) {
                    //起始地址为ubuf长度为l 的数据就是ECM数据
                    PRINTF("buf = %p, len = %d\n", ubuf, l);
                    codec->ts_ca_flg = 2;
                }
#ifdef ENABLE_CA_VERIMATRIX
                if(codec->ts_ca.system_id == 0x5601)
                    ymm_vmdrmDecryptSetECM(codec->ca_vmHandle, codec->avplay_buf.pu8Data, len);
#endif
            }
#ifdef ENABLE_CA_VERIMATRIX
            if(ca->system_id == 0x5601) {
                int decryption = 0;
                ymm_vmdrmDecryptSetStream(codec->ca_vmHandle, codec->avplay_buf.pu8Data, len, &decryption);
            }
#endif
        }
        codec->avplay_buf.u32Size = len;
        Ret = HI_UNF_AVPLAY_PutBuf(codec->handle, HI_UNF_AVPLAY_BUF_ID_TS, len, 0);
        if(Ret != HI_SUCCESS)
            ERR_OUT("#%d HI_UNF_AVPLAY_PutBuf\n", pIndex);

#ifdef CATCHTS
        if(savefp != NULL)
            fwrite(codec->avplay_buf.pu8Data, 1, len, savefp);
#endif
        break;
    }
    default:
        ERR_OUT("#%d state = %d\n", pIndex, codec->state);
    }
    return 0;
Err:
    return -1;
}

int codec_reset(int pIndex, int caReset)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("reset codec(%d)\n", pIndex);
    if(codec->type != CODEC_TYPE_TS)
        ERR_OUT("#%d type = %d\n", pIndex, codec->type);

    if(codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
        return 0;
    codec->reset_flg = 1;
    codec->ts_pts = 0;
    return 0;
Err:
    return -1;
}

int codec_flush(int pIndex)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("flush codec(%d)\n", pIndex);
    if(codec->type != CODEC_TYPE_TS)
        ERR_OUT("#%d type = %d\n", pIndex, codec->type);

    if(codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
        return 0;
    codec->reset_flg = 1;
    return 0;
Err:
    return -1;
}

static int vcodec_parse(int pIndex, HI_UNF_VCODEC_ATTR_S *attr)
{
    struct ts_psi *psi;
    struct StrmCodec *codec = g_codecs[pIndex];

    psi = &codec->ts_psi;

    if(psi->video_pid != 0) {
        switch(psi->video_type) {
        case ISO_IEC_11172_VIDEO:
        case ISO_IEC_13818_2_VIDEO:
            attr->enType = HI_UNF_VCODEC_TYPE_MPEG2;
            PRINTF("#%d VDEC_MPEG2\n", pIndex);
            break;
        case ISO_IEC_MPEG4_VIDEO:
            attr->enType = HI_UNF_VCODEC_TYPE_MPEG4;
            PRINTF("#%d VDEC_MPEG4\n", pIndex);
            break;
        case ISO_IEC_H264:
            attr->enType = HI_UNF_VCODEC_TYPE_H264;
            PRINTF("#%d VDEC_H264\n", pIndex);
            break;
        default:
            ERR_OUT("#%d video_type = %d/%x\n", pIndex, psi->video_type, psi->video_type);
        }
    }
    return 0;
Err:
    return -1;
}

static int acodec_parse(int pIndex, HI_UNF_ACODEC_ATTR_S *attr, int track)
{
    struct ts_psi *psi;
    struct StrmCodec *codec = g_codecs[pIndex];

    psi = &codec->ts_psi;
    if((track < 0) || (track >= psi->audio_num))
        track = 0;

    if(psi->audio_num > 0) {
        switch(psi->audio_type[track]) {
        case ISO_IEC_11172_AUDIO:
        case ISO_IEC_13818_3_AUDIO:
            attr->enType = HI_UNF_ACODEC_TYPE_MP3;
            PRINTF("#%d ADEC_MP3\n", pIndex);
            break;
        case ISO_IEC_13818_7_AUDIO:
        case ISO_IEC_MPEG4_AUDIO:
            attr->enType = HI_UNF_ACODEC_TYPE_AAC;
            PRINTF("#%d ADEC_AAC\n", pIndex);
            break;
        case ISO_IEC_AC3_AUDIO:
            attr->enType = HI_UNF_ACODEC_TYPE_AC3;
            attr->unAdecPara.stAc3Param.enAc3workmode = HI_UNF_AC3_RAW;
            PRINTF("#%d ADEC_AC3\n", pIndex);
            break;
        default:
            ERR_OUT("#%d audio_type = %d/%x\n", pIndex, psi->audio_type[track], psi->audio_type[track]);
        }
#if defined(Jiangsu)
        attr->bSmartVolumeEnable = HI_FALSE;
#else
        attr->bSmartVolumeEnable = HI_TRUE;
#endif
        HI_UNF_AVPLAY_SetAttr(codec->handle, HI_UNF_AVPLAY_ATTR_ID_AUD_PID, &psi->audio_pid[track]);
        PRINTF("#%d ++++ audio %d/%d\n", pIndex, psi->audio_pid[track], psi->audio_type[track]);
        codec->ts_track = track;
    }
    return 0;
Err:
    return -1;
}

int codec_pts(int pIndex, u_int* ppts)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    *ppts = codec->ts_pts;
    return 0;
}

static void codec_mode(int pIndex, HI_UNF_VCODEC_MODE_E mode)
{
    HI_UNF_VCODEC_ATTR_S VdecAttr;
    struct StrmCodec *codec = g_codecs[pIndex];
    HI_HANDLE handle = codec->handle;

    PRINTF("#%d ++++ mode = %d\n", pIndex, mode);
    HI_UNF_AVPLAY_GetAttr(handle, HI_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
    if(mode == HI_UNF_VCODEC_MODE_I) {
        VdecAttr.u32ErrCover = 0;
        VdecAttr.u32RefCover = 0;
    }
    else {
        VdecAttr.u32ErrCover = 100;
        VdecAttr.u32RefCover = 100;
    }
    VdecAttr.enMode = mode;
    HI_UNF_AVPLAY_SetAttr(handle, HI_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
}

int codec_pause(int pIndex)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("#%d ++++\n", pIndex);

    if(codec->type != CODEC_TYPE_TS)
        ERR_OUT("#%d type = %d\n", pIndex, codec->type);

    if((codec->state == CODEC_STATE_CLOSE) || (codec->state == CODEC_STATE_OPEN))
        ERR_OUT("#%d state = %d\n", pIndex, codec->state);

    if(codec->state == CODEC_STATE_PAUSE)
        return 0;

    if(codec->state == CODEC_STATE_TPLAY)
        codec_mode(pIndex, HI_UNF_VCODEC_MODE_NORMAL);

    HI_UNF_AVPLAY_Pause(codec->handle, codec->avplay_chan, NULL);
    codec->state = CODEC_STATE_PAUSE;
    return 0;
Err:
    return -1;
}

int codec_psi(int pIndex, struct ts_psi *psi)
{
    HI_S32					Ret;
    HI_HANDLE				handle;
    HI_UNF_VCODEC_ATTR_S	VdecAttr;
    HI_UNF_ACODEC_ATTR_S	adecAttr;
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("#%d ++++\n", pIndex);

    handle = codec->handle;

    if(codec->type != CODEC_TYPE_TS)
        ERR_OUT("#%d type = %d\n", pIndex, codec->type);

    memcpy(&codec->ts_psi, psi, sizeof(struct ts_psi));

    if(codec->mini == 0) {
        if(psi->video_pid) {
            HI_UNF_AVPLAY_GetAttr(handle, HI_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
            Ret = vcodec_parse(pIndex, &VdecAttr);
            if(Ret != HI_SUCCESS)
                ERR_OUT("#%d vcodec_parse\n", pIndex);
            HI_UNF_AVPLAY_SetAttr(handle, HI_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
            HI_UNF_AVPLAY_SetAttr(handle, HI_UNF_AVPLAY_ATTR_ID_VID_PID, &psi->video_pid);
        }
    }
    else {
        psi->video_pid = 0;
    }

    if(pIndex == 0 && psi->audio_num > 0) {
        HI_UNF_AVPLAY_GetAttr(handle, HI_UNF_AVPLAY_ATTR_ID_ADEC, &adecAttr);
        Ret = acodec_parse(pIndex, &adecAttr, 0);
        if(Ret != HI_SUCCESS)
            ERR_OUT("#%d acodec_parse\n", pIndex);
        HI_UNF_AVPLAY_SetAttr(handle, HI_UNF_AVPLAY_ATTR_ID_ADEC, &adecAttr);
    }

    if(psi->pcr_pid)
        HI_UNF_AVPLAY_SetAttr(handle, HI_UNF_AVPLAY_ATTR_ID_PCR_PID, &psi->pcr_pid);

    if(pIndex == 1)
        codec->avplay_chan = HI_UNF_AVPLAY_MEDIA_CHAN_VID;
    else if(codec->mini == 0)
        codec->avplay_chan = HI_UNF_AVPLAY_MEDIA_CHAN_AUD | HI_UNF_AVPLAY_MEDIA_CHAN_VID;
    else
        codec->avplay_chan = HI_UNF_AVPLAY_MEDIA_CHAN_AUD;

    HI_UNF_AVPLAY_Start(handle, codec->avplay_chan, NULL);

    codec->space = 0;
    if(psi->video_pid)
        codec->frame_flg = 1;

#ifdef CATCHTS
{
    char filename[16];

    if(savefp)
        fclose(savefp);
    savesn ++;
    sprintf(filename, DEFAULT_DEBUG_DATAPATH"/scale%d_play.ts", savesn);
    savefp = fopen(filename, "wb");
    PRINTF("@@@@@@@@@@@@@@@@@@: savefp = %p\n", savefp);
}
#endif

    codec->state = CODEC_STATE_PLAY;

    if(codec->iframe) {
        PRINTF("#%d iframe mode!\n", pIndex);
        codec_tplay(pIndex);
    }
    return 0;
Err:
    return -1;
}

int codec_video_width(int pIndex)
{
    if(pIndex == 0)
        return 700;
    else
        return 350;
}

int codec_pcm_open(int pIndex, int sampleRate, int bitWidth, int channels)
{
    HI_UNF_AVPLAY_ATTR_S avAttr;
    HI_UNF_ACODEC_ATTR_S adecAttr;
    HI_HANDLE handle;

    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("#%d ++++\n", pIndex);

    pthread_mutex_lock(&g_mutex);
    if(codec->type == CODEC_TYPE_TS) {
        codec_close_in(pIndex, 1);
        codec_destroy_in(pIndex);
    }
    else if(codec->state != CODEC_STATE_CLOSE) {
        ERR_PRN("#%d state = %d\n", pIndex, codec->state);
        codec_pcm_close_in(pIndex);
    }

    codec->type = CODEC_TYPE_PCM;
    codec->handle = -1;

    //更新了新的库后，需要在此处设置即将要创建的播放器需不需要同步，需要同步就是主播放器
    memset(&avAttr, 0, sizeof(HI_UNF_AVPLAY_ATTR_S));
    if (pIndex == 0)
        avAttr.enPlayType = HI_UNF_AVPLAY_TYPE_MAIN;
    else
        avAttr.enPlayType = HI_UNF_AVPLAY_TYPE_MINOR;
    avAttr.stStreamAttr.enStreamType = HI_UNF_AVPLAY_STREAM_TYPE_ES;
    HI_UNF_AVPLAY_GetDefaultConfig(&avAttr, HI_UNF_AVPLAY_STREAM_TYPE_ES);

    if(HI_SUCCESS != HI_UNF_AVPLAY_Create(&avAttr, &handle))
        ERR_OUT("#%d HI_UNF_AVPLAY_Create failed.\n", pIndex);

    if (pIndex == 0)
        HI_UNF_SND_Attach(g_SndHandle, handle, 100);
    else
        HI_UNF_SND_Attach(g_SndHandle, handle, 50);

    codec->handle = handle;

    HI_UNF_AVPLAY_ChnOpen(handle, HI_UNF_AVPLAY_MEDIA_CHAN_AUD, NULL);

    HI_UNF_AVPLAY_GetAttr(handle, HI_UNF_AVPLAY_ATTR_ID_ADEC, &adecAttr);

    adecAttr.enType = HI_UNF_ACODEC_TYPE_PCM;
#if defined(Jiangsu)
    adecAttr.bSmartVolumeEnable = HI_FALSE;
#else
    adecAttr.bSmartVolumeEnable = HI_TRUE;
#endif
    adecAttr.unAdecPara.stPcmParam.bBigEndian = HI_FALSE;/*小端，暂不支持大端*/
    adecAttr.unAdecPara.stPcmParam.bInterlace = HI_TRUE;/*交织PCM*/
    adecAttr.unAdecPara.stPcmParam.enSampleRate = sampleRate;/* HI_UNF_SAMPLE_RATE_48K 48K采样率*/
    adecAttr.unAdecPara.stPcmParam.u32BitWidth = bitWidth;/*16 16bit位宽*/
    adecAttr.unAdecPara.stPcmParam.u32Channels = channels;/*2 双声道*/
    HI_UNF_AVPLAY_SetAttr(handle, HI_UNF_AVPLAY_ATTR_ID_ADEC, &adecAttr);

    HI_UNF_AVPLAY_Start(handle, HI_UNF_AVPLAY_MEDIA_CHAN_AUD, NULL);

    codec->state = CODEC_STATE_PLAY;

    pthread_mutex_unlock(&g_mutex);
    return 0;
Err:
    pthread_mutex_unlock(&g_mutex);
    codec_pcm_close(0);

    return -1;
}

static void codec_pcm_close_in(int pIndex)
{
    HI_HANDLE					handle;
    HI_UNF_AVPLAY_STOP_OPT_S	stopOPt;
    struct StrmCodec *codec = g_codecs[pIndex];

    handle = codec->handle;

    if(codec->state == CODEC_STATE_CLOSE)
        goto Ret;

    stopOPt.enMode = HI_UNF_AVPLAY_STOP_MODE_BLACK;
    stopOPt.s32Timeout = 0;
    HI_UNF_AVPLAY_Stop(handle, HI_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopOPt);

    HI_UNF_AVPLAY_ChnClose(handle, HI_UNF_AVPLAY_MEDIA_CHAN_AUD);
    HI_UNF_SND_Detach(g_SndHandle, handle);
    HI_UNF_AVPLAY_Destroy(handle);

    codec->handle = -1;
    codec->state = CODEC_STATE_CLOSE;

Ret:
    return;
}

int codec_pcm_close(int pIndex)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("#%d ++++ pcm_state = %d\n", pIndex, codec->state);
    pthread_mutex_lock(&g_mutex);
    if(codec->type == CODEC_TYPE_PCM)
        codec_pcm_close_in(pIndex);
    pthread_mutex_unlock(&g_mutex);
    return 0;
}

int codec_pcm_push(int pIndex, char* buf, int len)
{
    HI_HANDLE handle;
    HI_UNF_AVPLAY_BUF_S AVBuf;
    struct StrmCodec *codec = g_codecs[pIndex];

    handle = codec->handle;

    if(codec->type != CODEC_TYPE_PCM)
        ERR_OUT("#%d type = %d\n", pIndex, codec->type);

    if(codec->state != CODEC_STATE_PLAY)
        goto Err;

    if(HI_SUCCESS != HI_UNF_AVPLAY_GetBuf(handle, HI_UNF_AVPLAY_BUF_ID_ES_AUD, 0x1000, &AVBuf, 0))
        return 0;

    if(len > 0x1000)
        len = 0x1000;
    memcpy(AVBuf.pu8Data, buf, len);

    if(HI_SUCCESS != HI_UNF_AVPLAY_PutBuf(handle, HI_UNF_AVPLAY_BUF_ID_ES_AUD, len, 0))
        ERR_OUT("#%d HI_UNF_AVPLAY_PutBuf\n", pIndex);

    return len;
Err:
    return -1;
}

int codec_tplay(int pIndex)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("#%d ++++\n", pIndex);

    if(codec->type != CODEC_TYPE_TS)
        ERR_OUT("#%d type = %d\n", pIndex, codec->type);

#ifdef CATCHTS
    {
        char	filename[16];

        if(savefp)
            fclose(savefp);
        savesn ++;
        sprintf(filename, DEFAULT_DEBUG_DATAPATH"/scale%d_tplay.ts", savesn);
        savefp = fopen(filename, "wb");
        PRINTF("@@@@@@@@@@@@@@@@@@: savefp = %p\n", savefp);
    }
#endif

    if((codec->state == CODEC_STATE_CLOSE) || (codec->state == CODEC_STATE_OPEN))
        ERR_OUT("#%d state = %d\n", pIndex, codec->state);

    HI_UNF_AVPLAY_Reset(codec->handle, codec->avplay_chan, NULL);

    if(codec->state != CODEC_STATE_TPLAY)
        codec_mode(pIndex, HI_UNF_VCODEC_MODE_I);

    HI_UNF_AVPLAY_Tplay(codec->handle, NULL);

    codec->space = 0;
    codec->state = CODEC_STATE_TPLAY;

    return 0;
Err:
    return -1;
}

int codec_resume(int pIndex, int iptv)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("#%d ++++ reset_flg = %d\n", pIndex, codec->reset_flg);

    if(codec->type != CODEC_TYPE_TS)
        ERR_OUT("#%d type = %d\n", pIndex, codec->type);

    if(codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
        ERR_OUT("#%d state = %d\n", pIndex, codec->state);

    if(codec->state == CODEC_STATE_TPLAY)
        codec_mode(pIndex, HI_UNF_VCODEC_MODE_NORMAL);

    if(codec->reset_flg) {
        HI_UNF_AVPLAY_Reset(codec->handle, codec->avplay_chan, NULL);
        codec->reset_flg = 0;
    }

    HI_UNF_AVPLAY_Resume(codec->handle, codec->avplay_chan, NULL);

    codec->space = 0;
    codec->state = CODEC_STATE_PLAY;

    return 0;
Err:
    return -1;
}

static void codec_close_in(int pIndex, int clear)
{
    HI_UNF_AVPLAY_STOP_OPT_S	stopOPt;
    HI_HANDLE					handle;

    struct StrmCodec *codec = g_codecs[pIndex];

    handle = codec->handle;

    if(codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN) {
        if(codec->frame_flg == 0 || clear == 0)
            goto End;
        HI_UNF_AVPLAY_Start(handle, codec->avplay_chan, NULL);
    }

    if(codec->state == CODEC_STATE_TPLAY)
        codec_mode(pIndex, HI_UNF_VCODEC_MODE_NORMAL);

    if(clear == 0) {
        stopOPt.enMode = HI_UNF_AVPLAY_STOP_MODE_STILL;
    } else {
        stopOPt.enMode = HI_UNF_AVPLAY_STOP_MODE_BLACK;
        codec->frame_flg = 0;

        if(pIndex == 1)
            yx_drv_videoout_set(1, 0);
    }
    stopOPt.s32Timeout = 0;

    HI_UNF_AVPLAY_Stop(handle, codec->avplay_chan, &stopOPt);

    codec->sample_rate = 0;
    codec->field_mode = 0;
    codec->aspect_ratio = 0;
    codec->width = 0;
    codec->height = 0;

End:
    codec->state = CODEC_STATE_CLOSE;
    return;
}

int codec_close(int pIndex, int clear)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("#%d ++++ clear = %d\n", pIndex, clear);

    pthread_mutex_lock(&g_mutex);

    if(codec->type == CODEC_TYPE_TS) {
        codec_close_in(pIndex, clear);
        if(pIndex == 1) {
            codec_destroy_in(pIndex);
            codec->type = CODEC_TYPE_PCM;
        }
    }

#ifdef CATCHTS
    if(savefp) {
        fclose(savefp);
        savefp = NULL;
    }
#endif
    if(pIndex == 1)
        yx_drv_videoout_set(1, 0);

    pthread_mutex_unlock(&g_mutex);
    return 0;
}

int codec_emm(int pIndex, int flag)
{
    return 0;
}

//width 为零表示重置为默认值
int codec_rect(int pIndex, int x, int y, int width, int height)
{
    struct StrmCodec *codec = g_codecs[pIndex];

    PRINTF("#%d ++++\n", pIndex);
    if(codec == NULL)
        ERR_OUT("#%d codec is NULL\n", pIndex);

    codec->rect_x = x;
    codec->rect_y = y;
    codec->rect_width = width;
    codec->rect_height = height;

    if(pIndex == 0) {
        if(width == 0) {
            PRINTF("#%d window reset\n", pIndex);
            x = 0;
            y = 0;
            width = 720;
            if(codec->pn_mode)
                height = 576;
            else
                height = 480;
        }
        yx_drv_videoout_window_set(pIndex, x, y, width, height);
    }

    return 0;
Err:
    return -1;
}
/* 设置切台模式 */
/* 0				切台黑屏 */
/* 1				切台时保留最后一帧  */
/* 2				切台时不停止播放并保留最后一帧  */
/* 3				切台时黑屏但是关掉同步 */
int codec_changemode(int mode)
{
    return 0;
}

int codec_audio_track_set(int track)
{
    HI_UNF_AVPLAY_STOP_OPT_S	stopOPt;
    HI_UNF_ACODEC_ATTR_S		adecAttr;
    HI_S32						Ret;
    HI_HANDLE					handle;

    struct StrmCodec *codec = g_codecs[0];

    PRINTF("++++ track = %d\n", track);

    pthread_mutex_lock(&g_mutex);

    handle = codec->handle;

    if(codec->type != CODEC_TYPE_TS)
        ERR_OUT("type = %d\n", codec->type);

    if((codec->state == CODEC_STATE_CLOSE) || (codec->state == CODEC_STATE_OPEN))
        ERR_OUT("state = %d\n", codec->state);

    stopOPt.enMode = HI_UNF_AVPLAY_STOP_MODE_STILL;
    stopOPt.s32Timeout = 0;

    HI_UNF_AVPLAY_Stop(handle, HI_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopOPt);

    HI_UNF_AVPLAY_GetAttr(handle, HI_UNF_AVPLAY_ATTR_ID_ADEC, &adecAttr);
    Ret = acodec_parse(0, &adecAttr, track);
    if(Ret != HI_SUCCESS)
        ERR_OUT("acodec_parse\n");
    HI_UNF_AVPLAY_SetAttr(handle, HI_UNF_AVPLAY_ATTR_ID_ADEC, &adecAttr);

    HI_UNF_AVPLAY_Start(handle, codec->avplay_chan, NULL);

    pthread_mutex_unlock(&g_mutex);
    return 0;
Err:
    pthread_mutex_unlock(&g_mutex);
    return -1;
}

int codec_audio_track_get(int* ptrack)
{
    struct StrmCodec *codec = g_codecs[0];
    *ptrack = codec->ts_track;
    return 0;
}
int codec_audio_track_get_info(int track, char *info)
{
    struct StrmCodec *codec = g_codecs[0];

    if(track >= 0 && track < codec->ts_psi.audio_num) {
        memcpy(info, codec->ts_psi.audio_iso693[track].language, 3);
        info[3] = 0;
    } else {
        memset(info, 0, 4);
    }
}
int codec_audio_track_get_type(int track, int *ptype)
{
    *ptype = 0;
}

int codec_audio_track_num(int* pnum)
{
    struct StrmCodec *codec = g_codecs[0];

    if(codec->state == CODEC_STATE_CLOSE || codec->state == CODEC_STATE_OPEN)
        *pnum = 0;
    else
        *pnum = codec->ts_psi.audio_num;

    return 0;
}
int codec_audio_track_get_pid(int track, int *pid)
{
    struct StrmCodec *codec = g_codecs[0];

    if(track >= 0 && track < codec->ts_psi.audio_num) {
        *pid = codec->ts_psi.audio_pid[track];
    } else {
        *pid = 0;
    }
}
int codec_subtitle_show_set(int flag)
{
    PRINTF("not implement.\n");
    return 0;
}

int codec_subtitle_show_get(int flag)
{
    PRINTF("not implement.\n");
    return 0;
}

int codec_subtitle_set(int subtitle)
{
    PRINTF("not implement.\n");
    return -1;
}

int codec_subtitle_get(int *psubtitle)
{
    PRINTF("not implement.\n");
    return -1;
}
int codec_subtitle_pid(int subtitle, unsigned short* pid)
{
    struct ts_dr_subtitle *dr_subtitle;
    struct StrmCodec *codec = g_codecs[0];

    dr_subtitle = codec->ts_psi.dr_subtitle;
    if(dr_subtitle && subtitle >= 0 && subtitle < dr_subtitle->subtitle_num) {
        *pid = dr_subtitle->subtitle_pid[subtitle];
    } else {
        *pid = 0x1fff;
    }
}

int codec_subtitle_lang(int subtitle, char *language)
{
    struct ts_dr_subtitle *dr_subtitle;

    struct StrmCodec *codec = g_codecs[0];

    pthread_mutex_lock(&g_mutex);

    dr_subtitle = codec->ts_psi.dr_subtitle;

    if(dr_subtitle && subtitle >= 0 && subtitle < dr_subtitle->subtitle_num) {
        memcpy(language, dr_subtitle->subtitle[subtitle].language, 3);
        language[3] = 0;
    } else {
        memset(language, 0, 4);
    }

    pthread_mutex_unlock(&g_mutex);
    return 0;
}

int codec_subtitle_num(int *pnum)
{
    pthread_mutex_lock(&g_mutex);

    struct StrmCodec *codec = g_codecs[0];

    *pnum = 0;

    if(codec->state == CODEC_STATE_PLAY || codec->state == CODEC_STATE_TPLAY || codec->state == CODEC_STATE_PAUSE) {
        struct ts_dr_subtitle *dr_subtitle = codec->ts_psi.dr_subtitle;
        if(dr_subtitle)
            *pnum = dr_subtitle->subtitle_num;
    }

    pthread_mutex_unlock(&g_mutex);
    return 0;
}

int codec_ca_update(int pIndex, ts_ca_t ca, char *pmt_buf, int pmt_len)
{
    struct StrmCodec *codec = g_codecs[0];

    if(ca && ca->system_id) {
        codec->ts_ca = *ca;
        codec->ts_ca_flg = 1;

#ifdef ENABLE_CA_VERIMATRIX
        if(pIndex == 0 && ca->system_id == 0x5601)
            ymm_vmdrmDecryptSetEcmPID(codec->ca_vmHandle, ca->pid);
#endif
    } else {
        codec->ts_ca.system_id = 0;
    }

    PRINTF("#%d: system_id = %d\n", pIndex, codec->ts_ca.system_id);

    return 0;
}

int codec_ca_check(int pIndex)
{
    int ret = 0;

#ifdef ENABLE_CA_VERIMATRIX
    if(pIndex == 0) {
        struct StrmCodec *codec = g_codecs[0];

        if(codec->ts_ca.system_id == 0x5601)
            ret = ymm_vmdrmDecryptGetDecryptionFlag(codec->ca_vmHandle);
    }
#endif

    return ret;
}

int codec_ca_cat(int pIndex, char *pmt_buf, int pmt_len)
{
    return -1;
}

void codec_track(void)
{
}

int codec_teletext_set(int teletext)
{
    PRINTF("teletext = %d\n", teletext);

    return -1;
}

void codec_default_audio(int pIndex, char* language)
{
    PRINTF("index = %d, language = %s\n", pIndex, language);
}

void codec_default_subtile(int pIndex, char* language)
{
    PRINTF("index = %d, language = %s\n", pIndex, language);
}

void codec_default_language(int pIndex, char* language)
{
    PRINTF("index = %d, language = %s\n", pIndex, language);
}

int codec_teletext_lang(int teletext, char *language)
{
    return -1;
}

int codec_teletext_num(int *pnum)
{
    PRINTF("pnum = %d\n", pnum);
    return -1;
}

int codec_save(int pIndex, int enable)
{
    PRINTF("not implement.\n");
    return 0;
}


int ymm_stream_subPlayerStart(char *sub_data_url, char *sub_index_url, int pIndex)
{
    return -1;
}

int ymm_stream_subPlayerStop(int pIndex)
{
    return -1;
}

void codec_lock(void)
{
    return;
}

int mosaic_create_stream()
{
    return -1;
}

int codec_set_hd_AspectRation(int aspect,int ratio)
{
    return -1;
}

int codec_set_sd_AspectRation(int aspect,int ratio)
{
    return -1;
}

static unsigned int codec_10ms(void)
{
    unsigned int clk;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    clk = (unsigned int)tp.tv_sec * 100 + (unsigned int)tp.tv_nsec / 10000000;

    return clk;
}

static int yx_get_MemUsed(unsigned int *use_ratio)
{
    FILE *fp = NULL;
    char buf[2][128];
    unsigned long mem_total = 0, mem_free = 0;

    if (!use_ratio)
        return -1;

    fp = fopen("/proc/meminfo","r");
    if (!fp) {
        perror("fopen /proc/meminfo error:");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    fgets(buf[0], 128, fp);
    fgets(buf[1], 128, fp);

    sscanf(buf[0], "%*s%ld", &mem_total);
    sscanf(buf[1], "%*s%ld", &mem_free);

	if (mem_total)
    	*use_ratio = (unsigned int)(100 * (mem_total - mem_free) / mem_total);

    fclose(fp);
    return 0;
}

static unsigned long g_all[2] = {0, 0};
static unsigned long g_used[2] = {0, 0};
static unsigned int g_clk = 0;

static int yx_get_CpuUsed(unsigned int *use_ratio)
{
    FILE *fp;
    char buf[128];
    char cpu[5];
    unsigned int clk;
    unsigned long user, nice, sys, idle, iowait, irq, softirq;
    unsigned long all, used;

    if (!use_ratio)
        return -1;

    fp = fopen("/proc/stat", "r");
    if(!fp) {
        perror("fopen /proc/stat error:");
        return -1;
    }

    buf[0] = 0;
    cpu[0] = 0;
    fgets(buf, sizeof(buf), fp);

    fclose(fp);

    user = 0;
    nice = 0;
    sys = 0;
    idle = 0;
    iowait = 0;
    irq = 0;
    softirq = 0;
    sscanf(buf, "%s%ld%ld%ld%ld%ld%ld%ld", cpu, &user, &nice, &sys, &idle, &iowait, &irq, &softirq);
    used = user + nice + sys + iowait + irq + softirq;
    all = used + idle;

    clk = codec_10ms( );
    pthread_mutex_lock(&g_mutex);
    if (clk - g_clk >= 100) {
        *use_ratio = (100 * (used - g_used[0])) / (all - g_all[0]);
        g_clk = clk;
        g_all[1] = g_all[0];
        g_used[1] = g_used[0];
        g_all[0] = all;
        g_used[0] = used;
    } else {
        *use_ratio = (100 * (used - g_used[1])) / (all - g_all[1]);
    }
    pthread_mutex_unlock(&g_mutex);
    //printf("cpu use = %d%%\n",*use_ratio);

    return 0;
}

static void int_decoder_getVisualizationInfo(struct StrmCodec *codec, YX_VISUALIZATION_INFO *info)
{
    struct ts_psi *psi = &codec->ts_psi;

    info->VideoWidth = codec->width;
    info->VideoHeight = codec->height;

    switch(codec->field_mode) {
    case VIDEO_FIELD_ALL:
        info->PictureCode = YX_DECODER_FRAME_MODE;
        break;
    case VIDEO_FIELD_TOP:
        info->PictureCode = YX_DECODER_TOP_FIELD_MODE;
        break;
    case VIDEO_FIELD_BOTTOM:
        info->PictureCode = YX_DECODER_BOTTOM_FIELD_MODE;
        break;
    default:
        info->PictureCode = YX_DECODER_FIELD_MODE_UNKNOWN;
    }

    switch(codec->aspect_ratio) {
    case HI_UNF_ASPECT_RATIO_4TO3:
        info->VideoAspect = YX_ASPECT_RATIO_4x3;
        break;
    case HI_UNF_ASPECT_RATIO_16TO9:
        info->VideoAspect = YX_ASPECT_RATIO_16x9;
        break;
    case HI_UNF_ASPECT_RATIO_SQUARE:
        info->VideoAspect = YX_ASPECT_RATIO_1x1;
        break;
    case HI_UNF_ASPECT_RATIO_221TO1:
        info->VideoAspect = YX_ASPECT_RATIO_221x1;
        break;
    default:
        info->VideoAspect = YX_ASPECT_RATIO_UNKOWN;
        break;
    }

    info->VideoCodec = YX_VIDEO_TYPE_NONE;
    if (psi->video_pid != 0) {
        switch (psi->video_type) {
            case ISO_IEC_11172_VIDEO:
            case ISO_IEC_13818_2_VIDEO:
                info->VideoCodec = YX_VIDEO_TYPE_MPEG2;
                PRINTF("#%d VDEC_MPEG2\n", index);
                break;
            case ISO_IEC_MPEG4_VIDEO:
                info->VideoCodec = YX_VIDEO_TYPE_MPEG4PART2;
                PRINTF("#%d VDEC_MPEG4\n", index);
                break;
            case ISO_IEC_H264:
                info->VideoCodec = YX_VIDEO_TYPE_H264;
                PRINTF("#%d VDEC_H264\n", index);
                break;
            default:
                break;
        }
    }

    if (psi->audio_num > 0) {
        int i;

        info->AudioNum = psi->audio_num;
        for (i = 0; i < info->AudioNum; i++) {
            switch (psi->audio_type[i]) {
            case ISO_IEC_11172_AUDIO:
            case ISO_IEC_13818_3_AUDIO:
                info->AudioCodec[i] = YX_AUDIO_TYPE_MPEG;
                break;
            case ISO_IEC_13818_7_AUDIO:
                info->AudioCodec[i] = YX_AUDIO_TYPE_AAC;
                break;
            case ISO_IEC_MPEG4_AUDIO:
                info->AudioCodec[i] = YX_AUDIO_TYPE_AACPLUS;
                break;
            case ISO_IEC_AC3_AUDIO:
                info->AudioCodec[i] = YX_AUDIO_TYPE_AC3;
                break;
            default:
                info->AudioCodec[i] = YX_AUDIO_TYPE_NONE;
                break;
            }
        }

        if (codec->ts_track >= 0 && codec->ts_track < psi->audio_num)
            info->AudioSamplingRate[codec->ts_track] = codec->sample_rate;
    }

    {
        HI_UNF_AVPLAY_SYNC_ATTR_S syncPara;
        if (HI_SUCCESS == HI_UNF_AVPLAY_GetAttr(codec->handle, HI_UNF_AVPLAY_ATTR_ID_SYNC, &syncPara))
            info->DiffAvPlayTime = syncPara.s32TimeOffset;
    }
    {
        HI_UNF_AVPLAY_DEBUG_INFO_S debugInfo;
        if (HI_SUCCESS == HI_UNF_AVPLAY_GetDebugInfo(codec->handle, &debugInfo)) {
            info->VideoBufSize = debugInfo.stVcodecInfo.u32BufferSize;
            info->VideoUsedSize = debugInfo.stVcodecInfo.u32BufferUsed;

            info->VideoDecodeError = debugInfo.stVcodecInfo.u32ErrorNum;
            info->VideoDecodeDrop = debugInfo.u32VidSkipFrameCnt;

            info->AudioDecodeError = debugInfo.stAcodecInfo.u32ErrorFrameNum;
        }
    }
}

int ymm_decoder_getVisualizationInfo(int idx, YX_VISUALIZATION_INFO *info)
{
    int i;
    struct StrmCodec *codec;

    if (!info)
        return -1;
    memset(info, 0, sizeof(YX_VISUALIZATION_INFO));
    if (1 != idx && 0 != idx)
        return -1;

    info->CpuUsed = -1;
   info->SysMemUsed = -1;

    info->VideoWidth = -1;
    info->VideoHeight = -1;
    info->VideoAspect = -1;
    info->VideoCodec = -1;
    info->PictureCode = -1;

   info->AudioNum = -1;

    for (i = 0; i < YX_MPEG_AUDIO_NUM_MAX; i++) {
        info->AudioCodec[i] = -1;
        info->AudioBitRate[i] = -1;
        info->AudioChannels[i] = -1;
        info->AudioSamplingRate[i] = -1;
    }
    unsigned char SubtitleLanguage[YX_MPEG_SUBTITLE_NUM_MAX][4];//标清不支持字幕

    unsigned int PacketLost = -1;
    unsigned int PacketDisorder = -1;
    unsigned int NetDelayTime = -1;
    unsigned int TransportProtocol = -1;

    unsigned int ContinuityError = -1;
    unsigned int SynchronisationError = -1;
    unsigned int EcmError = -1;
    unsigned int DiffAvPlayTime = -1;

    unsigned int VideoBufSize = -1;
    unsigned int VideoUsedSize = -1;
    unsigned int AudioBufSize = -1;
    unsigned int AudioUsedSize = -1;

    unsigned int VideoDecodeError = -1;
    unsigned int VideoDecodeDrop = -1;
    unsigned int VideoDecodeUnderflow = -1;
    unsigned int VideoDecodePtsError = -1;
    unsigned int AudioDecodeError = -1;
    unsigned int AudioDecodeDrop = -1;
    unsigned int AudioDecodeUnderflow = -1;
    unsigned int AudioDecodePtsError = -1;

    yx_get_MemUsed(&info->SysMemUsed);
    yx_get_CpuUsed(&info->CpuUsed);

    pthread_mutex_lock(&g_mutex);
    codec = g_codecs[idx];
    if (CODEC_TYPE_TS == codec->type && CODEC_STATE_PLAY <= codec->state)
        int_decoder_getVisualizationInfo(codec, info);
    pthread_mutex_unlock(&g_mutex);

    return 0;
}

void ymm_decoder_test(void)
{
    unsigned int ratio = 0;

    yx_get_CpuUsed(&ratio);
    printf("CPU use = %d\n", ratio);
    yx_get_MemUsed(&ratio);
    printf("MEM use = %d\n", ratio);
}
