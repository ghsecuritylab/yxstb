
#include "JseIO.h"
#include "JseRoot.h"
#include "JseCall.h"
#include "JseFunctionCall.h"

#include "Analog/JseAnalog.h"
#include "Digital/JseDigital.h"
#include "Margin/JseMargin.h"

#include "sys_msg.h"
#include "mid_sys.h"

#include "SysSetting.h"
#include "AppSetting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int JseReadRCModel(const char* param, char* value, int len)
{
    switch(sys_msg_ir_cus_get()){
    case 0xff00:
    case 0xfe01:
    case 0xdd22:
        strncpy(value, "101", 32);
        break;
    case 0xee11:
        strncpy(value, "102", 32);
        break;
    case 0x4db2:
        strncpy(value, "B24D", 32);
        break;
    default:
        break;
    }
    return 0;
}

static int JseSDVideoStandardRead(const char* param, char* value, int len)
{
    int ret;
	sysSettingGetInt("videoformat", &ret, 0);

#ifdef HUAWEI_C10
    sprintf(value, "%d", ret);
#elif (defined HUAWEI_C20)
    switch(ret) {  /* 0 pal, 1 ntsc    agree with the local setting page*/
    case VideoFormat_SECAM:
    case VideoFormat_480P:
    case VideoFormat_NTSC:
        sprintf(value, "NTSC");
        break;
    case VideoFormat_PAL:
        sprintf(value, "PAL");
        break;
    default:
        sprintf(value, "PAL");
        break;
    }
#endif

    return 0;
}

/******************************************************
视频制式(String 8)(针对标清,C10返回数值而不是串) ，写入STB Flash，
实时生效。PAL(1)：PAL制式。NTSC(0)：NTSC制式
*******************************************************/
static int JseSDVideoStandardWrite(const char* param, char* value, int len)
{
    int videoFormat = VideoFormat_UNKNOWN;
#ifdef HUAWEI_C10
    videoFormat = atoi(value);
#elif (defined HUAWEI_C20)
    if (!strncasecmp(value, "NTSC", 4)) {
        videoFormat = VideoFormat_NTSC;
    }
    else if(!strncasecmp(value, "PAL", 3)) {
        videoFormat = VideoFormat_PAL;
    }
    else {
        LogJseError("unkown JseSDVideoStandardWrite [%s]\n", value);
		return -1;
    }
#endif
    sysSettingSetInt("videoformat", videoFormat);
    return 0;
}

static int JseHDVideoStandardRead(const char* param, char* value, int len)
{
    int ret = 0;
	sysSettingGetInt("hd_video_format", &ret, 0);

#ifdef HUAWEI_C10
    sprintf(value, "%d", ret);
#elif (defined HUAWEI_C20)
    switch (ret) {
    case VideoFormat_1080I50HZ://  1
        sprintf(value, "1080i50Hz");
        break;
    case VideoFormat_1080I60HZ:// 2
        sprintf(value, "1080i60Hz");
        break;
    case VideoFormat_720P50HZ:
        sprintf(value, "720p50Hz");
        break;
    case VideoFormat_480P:  // 4
        sprintf(value, "480i60Hz");
        break;
    case VideoFormat_576P: // 3
        sprintf(value, "576i60Hz");
        break;
     default:
        sprintf(value, "1080i50Hz");
        break;
    }
#endif
    return 0;
}

/**************************************************
高清输出制式(String 16),写入STB Flash,实时生效.
1080i50Hz(默认亿080i60Hz/720p50Hz/576i60Hz/480i60Hz
**************************************************/
static int JseHDVideoStandardWrite(const char* param, char* value, int len)
{
    int videoFormat = 0;
#ifdef HUAWEI_C10
    videoFormat = atoi(value);
#elif (defined HUAWEI_C20)
    if (!strncasecmp(value, "1080i50Hz", 9))
        videoFormat = VideoFormat_1080I50HZ;
    else if (!strncasecmp(value, "1080i60Hz", 9))
        videoFormat = VideoFormat_1080I60HZ;
    else if (!strncasecmp(value, "720p50Hz", 8))
        videoFormat = VideoFormat_720P50HZ;
    else if (!strncasecmp(value, "576i50Hz", 8)  || !strncasecmp(value, "576i60Hz", 8))
        videoFormat = VideoFormat_576P;
    else if (!strncasecmp(value, "480i60Hz", 8))
        videoFormat = VideoFormat_480P;
    else {
        LogJseError("unkonw JseHDVideoStandardWrite [%s]\n", value);
		return -1;
    }
#endif
    sysSettingSetInt("hd_video_format", videoFormat);
    return 0;
}

static int JseAspectRatioRead(const char* param, char* value, int len)
{
    int aspectRatioMode = 0;
    appSettingGetInt("hd_aspect_mode", &aspectRatioMode, 0);

    if(aspectRatioMode)
        aspectRatioMode = 0; //0: FULL for JS
    else
        aspectRatioMode = 1; //1: LETTERBOX  for JS
    sprintf(value, "%d", aspectRatioMode);
    return 0;
}

static int JseAspectRatioWrite(const char* param, char* value, int len)
{
    int aspectRatioMode = atoi(value);
    if(aspectRatioMode == 0) //0: FULL for JS
        aspectRatioMode = 2;
	else                     //1: LETTERBOX  for JS
		aspectRatioMode = 0;
	appSettingSetInt("hd_aspect_mode", aspectRatioMode);

    //codec_set_hd_AspectRation(2, 1); //YX_ASPECT_MODE_FULL 16:9
    //codec_set_sd_AspectRation(2, 1);

    //codec_set_hd_AspectRation(0, 1); //YX_ASPECT_MODE_LETTERBOX 4:3
    //codec_set_sd_AspectRation(0, 1);
    return 0;
}

static int JseSpdifAudioModeRead(const char* param, char* value, int len)
{
    int SPDIFAudioFormat = 0;
    sysSettingGetInt("SPDIFAudioFormat", &SPDIFAudioFormat, 0);
    sprintf(value, "%d", SPDIFAudioFormat);
    return 0;
}

static int JseSpdifAudioModeWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("SPDIFAudioFormat", atoi(value));
    return 0;
}

/***********************************************************************
0 is manual mode, 1 is auto mode. Default value is auto, spdif and hdmi can't select.
***********************************************************************/
static int JseDolbyModeRead(const char* param, char* value, int len)
{
    sysSettingGetString("DolbyMode", value, len, 0);
    return 0;
}

static int JseDolbyModeWrite(const char* param, char* value, int len)
{
    sysSettingSetString("DolbyMode", value);
    return 0;
}

static int JseHDMIAudioFormatRead(const char* param, char* value, int len)
{
    int HDMIAudioFormat = 0;
    sysSettingGetInt("HDMIAudioFormat", &HDMIAudioFormat, 0);
    sprintf(value, "%d", HDMIAudioFormat);
    return 0;
}

static int JseHDMIAudioFormatWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("HDMIAudioFormat", atoi(value));
    return 0;
}

static int JseAntiFlickerSwitchRead(const char* param, char* value, int len)
{
    sysSettingGetString("Aniflicker", value, len, 0);
    return 0;
}

// AntiFlicker onoff(int 1), Write to STB Flash, with immediate effect. 0 is off, 1 is on
static int JseAntiFlickerSwitchWrite(const char* param, char* value, int len)
{
    sysSettingSetString("Aniflicker", value);
    return 0;
}

/*************************************************
Description: 初始化海博IO配置定义的接口，由JseHybroad.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseIOInit()
{
    JseCall* call;

    call = new JseFunctionCall("rc_model", JseReadRCModel, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("SDVideoStandard", JseSDVideoStandardRead, JseSDVideoStandardWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("HDVideoStandard", JseHDVideoStandardRead, JseHDVideoStandardWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("AspectRatio", JseAspectRatioRead, JseAspectRatioWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("SpdifAudioMode", JseSpdifAudioModeRead, JseSpdifAudioModeWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("DolbyMode", JseDolbyModeRead, JseDolbyModeWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_HDMIAudioFormat", JseHDMIAudioFormatRead, JseHDMIAudioFormatWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("antiFlickerSwitch", JseAntiFlickerSwitchRead, JseAntiFlickerSwitchWrite);
    JseRootRegist(call->name(), call);

    JseAnalogInit();
    JseDigitalInit();
    JseMarginInit();
    return 0;
}

