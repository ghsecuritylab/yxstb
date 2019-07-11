
#include "JseHWPlay.h"
#include "JseHWAdditional.h"
#include "JseHWMedia.h"
#include "JseHWStream.h"
#include "Audio/JseHWAudio.h"
#include "Subtitle/JseHWSubtitle.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"
#include "SystemManager.h"
#include "UltraPlayer.h"
#include "Hippo_HString.h"
#include "BrowserEventQueue.h"

#ifdef INCLUDE_PVR
#include "PVR/JseHWPVR.h"
#endif

#ifdef INCLUDE_SQA
#include "SQA/JseHWSQA.h"
#endif

#if defined(SQM_INCLUDED)
#include "SQM/JseHWSQM.h"
#endif

#ifdef INCLUDE_LocalPlayer
#include "LocalPlayer/JseHWLocalPlayer.h"
#endif

#ifdef INCLUDE_DVBS
//#include "DVB/JseHWDVB.h"
#include "ResourceManager.h"
#include "json_public.h"
#include "json_object.h"
#endif

#ifdef INCLUDE_DOWNLOAD
#include "Download/JseHWDownload.h"
#endif

#include "codec.h"

#include "SysSetting.h"
#include "AppSetting.h"
#include "mid_stream.h"
#include "ind_mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int JseTransportProtocolRead(const char* param, char* value, int len)
{
    sysSettingGetString("TransportProtocol", value, len, 0);
    return 0;
}

static int JseTransportProtocolWrite(const char* param, char* value, int len)
{
    int protocols = atoi(value);

    if (protocols >= 0 && protocols < 4) {
        sysSettingSetInt("TransportProtocol", protocols);
        mid_stream_transport(protocols);
    }
    return 0;
}

static int JseChannelSwitchModeRead(const char* param, char* value, int len)
{
#ifdef HUAWEI_C10
	int changeVideoMode = 0;

    sysSettingGetInt("changevideomode", &changeVideoMode, 0);
    LogJseDebug("Get channel switch mode(%d)\n", changeVideoMode);
    snprintf(value, len, "%d", changeVideoMode);
#else
    int videomode = 0;
    sysSettingGetInt("changevideomode", &videomode, 0);

    if (videomode == 0)
        IND_STRCPY(value, "Normal");
    if (videomode == 1)
        IND_STRCPY(value, "last picture");
    if (videomode == 2)
        IND_STRCPY(value, "smooth switch");
#endif
    return 0;
}

static int JseChannelSwitchModeWrite(const char* param, char* value, int len)
{
#ifdef HUAWEI_C10
    LogJseDebug("Set channel switch mode(%s)(%d)\n", value, atoi(value));
    sysSettingSetInt("changevideomode", atoi(value));
#else
    if (strncasecmp(value, "Normal", 6) == 0)
	    sysSettingSetInt("changevideomode", 0);
    else if (strncasecmp(value, "smooth switch", 13) == 0)
	    sysSettingSetInt("changevideomode", 2);
    else if (strncasecmp(value, "last picture", 12) == 0)
	    sysSettingSetInt("changevideomode", 1);
    else
        return -1;
#endif
    return 0;
}

static int JsePlayBackModeRead(const char* param, char* value, int len)
{
    //app_a2_getPlayMode(value, len);

    Hippo::SystemManager &sysManager = Hippo::systemManager();
    Hippo::UltraPlayer *player = sysManager.obtainMainPlayer();
    Hippo::HString playMode;
    if (player)
        player->GetPlayBackMode(playMode);
    sysManager.releaseMainPlayer(player);

    if (len < playMode.length())
        strcpy(value, "");
    else
        strncpy(value, playMode.c_str(), playMode.length());

    LogJseDebug("getPlaybackMode : %s\n", value);
    return 0;
}

static int JseMediaServerTypeRead(const char* param, char* value, int len)
{
    int mediaServerType = 0;
    appSettingGetInt("mediaServerType", &mediaServerType, 0);
    sprintf(value, "%d", mediaServerType);
    return 0;
}

static int JseMediaServerTypeWrite(const char* param, char* value, int len)
{
    appSettingSetInt("mediaServerType", atoi(value));
    return 0;
}

static int JseGetEventRead(const char* param, char* value, int len)
{

    browserEventGet(0, value, len); //app_a2_getEvent
    LogJseDebug("browserEventGet : %s\n", value);
    return 0;
}

static int JseIsSupportDvbRead(const char* param, char* value, int len)
{
#if (defined INCLUDE_DVBS)
    char tempString[] = {"{\"DVBSupportFlag\":1, \"DVBSupportType\", [\"DVBS\"]"};
#else
    char tempString[] = {"{\"DVBSupportFlag\":0, \"DVBSupportType\", []"};
#endif
    snprintf(value, len, "%s", tempString);
    return 0;
}

static int JseDVBChannelManageFlagRead(const char* param, char* value, int len)
{
    sysSettingGetString("DVBChannelManageFlag", value, len, 0);
    return 0;
}

static int JseDVBChannelManageFlagWrite(const char* param, char* value, int len)
{
    sysSettingSetString("DVBChannelManageFlag", value);
    return 0;
}

#ifdef INCLUDE_DVBS
static int JseHeadEndStatusCheckIntervalRead(const char* param, char* value, int len)
{
    sysSettingGetString("DvbServiceCheck", value, len, 0);

    return 0;
}

static int JseWrite_headEndStatusCheckInterval(const char* param, char* value, int len)
{
    sysSettingSetString("DvbServiceCheck", value);
    return 0;
}

static int JseWriteDVBChannelDataCheck(const char* param, char* value, int len)
{
	int result = -1;
	struct json_object *jsonObj = NULL;
	int userType;

	if (value == NULL || value[0] != '{' || value[strlen(value)-1] != '}')
		return result;

	jsonObj = json_tokener_parse_string(value);
	if (jsonObj) {
		std::string jsonStr;
		struct json_object *subJsonObj = NULL;
		int channelNum = 0, appType = 0, channelResType = 0;

		subJsonObj = json_object_get_object_bykey(jsonObj, "chanKey");
		if (subJsonObj)
			channelNum = json_object_get_int(subJsonObj);
		else {
			json_object_delete(jsonObj);
			return result;
		}

		subJsonObj = json_object_get_object_bykey(jsonObj, "applicationType");
		if (subJsonObj) {
			appType = json_object_get_int(subJsonObj);
			switch(appType) {
			case 0: userType = Hippo::ResourceUser::SimplePlay; break;
			case 1: userType = Hippo::ResourceUser::PIP; break;
			case 2: userType = Hippo::ResourceUser::PVR; break;
			default: json_object_delete(jsonObj); return result;
			}
		}
		else {
			json_object_delete(jsonObj);
			return result;
		}

		subJsonObj = json_object_get_object_bykey(jsonObj, "channelResourceType");
		if (subJsonObj) {
			jsonStr = json_object_get_string(subJsonObj);
			channelResType = atoi(jsonStr.c_str());
		}

		json_object_delete(jsonObj);

		return Hippo::resourceManager().tunerResourceIsEnough(channelNum, userType);
	}

	return result;
}
#endif

/*************************************************
Description: 初始化华为播放流控配置定义的接口，由JseHuawei.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWPlayInit()
{
    JseCall* call;

    //C10/C20 regist
    call = new JseFunctionCall("TransportProtocol", JseTransportProtocolRead, JseTransportProtocolWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("ChannelSwitch", JseChannelSwitchModeRead, JseChannelSwitchModeWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("channelSwitchMode", JseChannelSwitchModeRead, JseChannelSwitchModeWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("PlayBackMode", JsePlayBackModeRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("mediaservertype", JseMediaServerTypeRead, JseMediaServerTypeWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("getEvent", JseGetEventRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseHWAdditionalLyric();
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseHWAdditionalSubtitle();
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseHWMedia();
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseHWStream();
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("DVBSupport", JseIsSupportDvbRead, 0);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("is_support_dvb", JseIsSupportDvbRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("DVBChannelManageFlag", JseDVBChannelManageFlagRead, JseDVBChannelManageFlagWrite);
    JseRootRegist(call->name(), call);

#ifdef INCLUDE_DVBS
    //C20 regist,先放这，之后再放到JseHWDVB.cpp
    call = new JseFunctionCall("headEndStatusCheckInterval", JseHeadEndStatusCheckIntervalRead, JseWrite_headEndStatusCheckInterval);
    JseRootRegist(call->name(), call);
    //C20 regist，先放这，之后再放到JseHWDVBChannel.cpp
    call = new JseFunctionCall("dvbChannelDataCheck",  0, JseDVBChannelDataCheckWrite);
    JseRootRegist(call->name(), call);
#endif

    JseHWAudioInit();
    JseHWSubtitleInit();

#ifdef INCLUDE_PVR
    //Hippo::JseHWPVRInit();  //C20 regist
    JseHWPVRInit();  //C20 regist
#endif

#ifdef INCLUDE_SQA
    JseHWSQAInit();
#endif

#if defined(SQM_INCLUDED)
    JseHWSQMInit();
#endif

#ifdef INCLUDE_LocalPlayer
    JseHWLocalPlayerInit();
#endif

#if 0
#ifdef INCLUDE_DVBS //TODO
    JseHWDVBInit();
#endif
#endif

#ifdef INCLUDE_DOWNLOAD
    call = new JseHWDownload();
    JseRootRegist(call->name(), call);
#endif
    return 0;
}

