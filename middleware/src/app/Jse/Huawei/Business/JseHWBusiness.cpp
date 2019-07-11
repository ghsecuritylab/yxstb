
#include "JseHWBusiness.h"
#include "Account/JseHWAccount.h"
#include "Ad/JseHWAd.h"
#include "Channel/JseHWChannel.h"
#include "Logo/JseHWLogo.h"
#include "Resource/JseHWResource.h"
#include "Session/JseHWSession.h"

#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "config.h"
#include "app_sys.h"
#include "ind_mem.h"
#include "sys_basic_macro.h"
#include "webpageConfig.h"
#include "mid/mid_tools.h"
#include "mid/mid_mutex.h"

#include "KeyTableParser.h"
#include "sys_key_deal.h"
#include "MessageValueSystem.h"
#include "MessageTypes.h"
#include "NativeHandler.h"
#include "KeyDispatcher.h"
#include "ipanel_event.h"

#include "UltraPlayerWebPage.h"
#include "UltraPlayerBGMusic.h"
#ifdef HUAWEI_C10
#include "CTCServiceEntry.h"
#endif

#ifdef HUAWEI_C20

#include "app_c20_init.h"

#include "BrowserEventQueue.h"
#include "BootImagesShow.h"
#endif

#if defined(CYBERCLOUD)
#include "cloud_api.h"
#endif

#include "codec.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "customer.h"
#include "Setting.h"
#include "IPTVMiddleware.h"
#include "Business.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char g_app_playendURL[LARGE_URL_MAX_LEN + 4];
static mid_mutex_t g_app_mutex = NULL;

static int JseEPGDomainRead(const char* param, char* value, int len)
{
    std::string url = Hippo::Customer().AuthInfo().Get(Hippo::Auth::eAuth_epgDomain);
    snprintf(value, len, "%s", url.c_str());
    return 0;
}

static int JseEPGDomainWrite(const char* param, char* value, int len)
{
    // 不合法的Url直接扔了。
    if (!Hippo::Customer().AuthInfo().CheckUrl(value))
        return -1;
    Hippo::Customer().AuthInfo().Set(Hippo::Auth::eAuth_epgDomain, value);
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)
    appSettingSetString("epg", value);
    settingManagerSave();
#endif
#if defined(SQM_VERSION_CERTUS)
    writeCertusConfig();
    startCertusQos();
#endif
    return 0;
}

static int JseEPGDomainBackupRead(const char* param, char* value, int len)
{
    std::string url = Hippo::Customer().AuthInfo().Get(Hippo::Auth::eAuth_epgDomain);
    snprintf(value, len, "%s", url.c_str());
    return 0;
}

static int JseEPGDomainBackupWrite(const char* param, char* value, int len)
{
    // 不合法的Url直接扔了。
    if (!Hippo::Customer().AuthInfo().CheckUrl(value))
        return -1;
    Hippo::Customer().AuthInfo().Set(Hippo::Auth::eAuth_epgDomainBackup, value);
    return 0;
}

/******************************************************************
EPG调度地址(string 1024)，写入STB Flash，由EDS写入STB中，
/在STB未关机的情况下，根据该地址指定的url访问EPG。读出时STB需要组合完整的认证参数
******************************************************************/
static int JseEPGUrlRead(const char* param, char* value, int len)
{
    std::string url = Hippo::Customer().AuthInfo().Get(Hippo::Auth::eAuth_epgHomeAddr);
    snprintf(value, len, "%s", url.c_str());
#ifdef HUAWEI_C20
    if (url.empty()) {
        snprintf(value, len, LOCAL_WEBPAGE_PATH_ERROR"?errorcode=4408");
    }
#endif
    return 0;
}

static int JseEPGUrlWrite(const char* param, char* value, int len)
{
    // 不合法的Url直接扔了。
    if (!Hippo::Customer().AuthInfo().CheckUrl(value))
        return -1;

    Hippo::Customer().AuthInfo().Set(Hippo::Auth::eAuth_epgHomeAddr, value);
#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    appSettingSetString("CurrentEPGUrl", value);
    appSettingSetString("iptv_epg", value);
#else
    IPTVMiddleware_SettingSetStr("CurrentEPGUrl", value);   // 这个需要写属性，因为Settings需要做到未接入EPG时显示为空。
    IPTVMiddleware_SettingSetStr("iptv_epg", value);        // 这个需要写数据库，供第三方APK使用。
#endif
#endif

    return 0;
}

static int JseStartLocalCfgWrite(const char* param, char* value, int len)
{
    sendMessageToKeyDispatcher(MessageType_KeyDown, EIS_IRKEY_VK_F10, 0, 0);
    return 0;
}

static int JseGetAuthenticationURLRead(const char* param, char* value, int len)
{
    if(APPMODE_IPTV != sys_appmode_get()) {
         sys_appmode_set(APPMODE_IPTV);
    }
    strcpy(value, Hippo::Customer().AuthInfo().AvailableEpgAuthUrl().c_str());
    return 0;
}

static int JseGlobalKeyTableWrite(const char* param, char* value, int len)
{
    return Hippo::KeyTableParser(value);
}

static int JseKeyCtrlExWrite(const char* param, char* value, int len)
{
    business().setKeyCtrl(atoi(value));
    return 0;
}

static int JseHWKeyCtrlRead(const char* param, char* value, int len)
{
    sprintf(value, "%d", business().getKeyCtrl());
    return 0;
}

static int JseHWKeyCtrlWrite(const char* param, char* value, int len)
{
    business().setKeyCtrl(atoi(value));
    return 0;
}

static int JseWebchannelStateRead(const char* param, char* value, int len)
{
    snprintf(value, len, "%d", webchannelFlagGet());
    return 0;
}

static int JseWebchannelStateWrite(const char* param, char* value, int len)
{
    webchannelFlagSet(atoi(value));
    return 0;
}

static int JseResignonWrite(const char* param, char* value, int len)
{
    if(1 ==  atoi(value))
        sendMessageToNativeHandler(MessageType_System, RESTART_AUTH, 0, 0);
    return 0;
}

#if defined(CYBERCLOUD)
static int JseCybercloudStartWrite(const char *func, const char *param, char *value, int len)
{
    CStb_CyberCloudTaskStart(value);
    return 0;
}
#endif

/*******************************************************************
 STB bootup mode, write to flash.Parameter 0 is enter IPTV EPG main page;
 parameter 1 is enter IPTV channel; parameter 2 is enter DVB-S channel.
 *******************************************************************/
static int JseDirectPlayRead(const char* param, char* value, int len)
{
    int lastChannelPlay = 0;
    sysSettingGetInt("lastChannelPlay", &lastChannelPlay, 0);
    sprintf(value, "%d", lastChannelPlay);
    return 0;
}
static int JseDirectPlayWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("lastChannelPlay", atoi(value));
    return 0;
}

#ifdef HUAWEI_C10
static int JseServiceEntryWrite(const char* param, char* value, int len)
{
	return Hippo::parseServiceEntry(0, param, value, len);
}

#ifdef PLAY_BGMUSIC
static int JsePlayBGMusicRead(const char* param, char* value, int len)
{
    if(value){
        sprintf(value, "%d", Hippo::UltraPlayerBGMusic::isEnable());
    }
    return 0;
}

static int JsePlayBGMusicWrite(const char* param, char* value, int len)
{
    if(value){
        return Hippo::UltraPlayerBGMusic::enable(atoi(value));
    }
    return -1;
}

static int JseDefaultBGMusicUrlRead(const char* param, char* value, int len)
{
    if(value){
        return Hippo::UltraPlayerBGMusic::getUrl(value);
    }
    return -1;
}

static int JseDefaultBGMusicUrlWrite(const char* param, char* value, int len)
{
    if(value){
        return Hippo::UltraPlayerBGMusic::setUrl(value);
    }
    return -1;
}
#endif //PLAY_BGMUSIC
#endif //HUAWEI_C10

/*************************************************
Description:设置播放结束URL
Input:       无
output:     无
Return:     0:成功；-1:失败
 *************************************************/
static int JseEndUrlRead(const char* param, char* value, int len)
{
    if(mid_tool_checkURL(g_app_playendURL, NULL, NULL))
        return -1;
    IND_STRCPY(value, g_app_playendURL);
    return 0;
}

/*************************************************
Description: 设置播放结束URL
Input:        URL:指向URL地址
Return:      无
 *************************************************/
static int JseEndUrlWrite(const char* param, char* value, int len)
{
    g_app_mutex = mid_mutex_create();
    if(mid_mutex_lock(g_app_mutex))
        LogSafeOperWarn("\n");
    if(value)
        IND_STRCPY(g_app_playendURL, value);
    else
        g_app_playendURL[0] = 0;
    mid_mutex_unlock(g_app_mutex);
    return 0;
}

//TODO
static int JseResServerUrlRead(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseResServerUrlWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JsePPVWrite(const char* param, char* value, int len)
{
    return 0;
}

static int JseLastChannelNoRead(const char* param, char* value, int len)
{
    int lastChannelKey = 0;
    appSettingGetInt("lastChannelID", &lastChannelKey, 0);
    sprintf(value, "%d", lastChannelKey);
    return 0;
}

static int JseLastChannelNoWrite(const char* param, char* value, int len)
{
    appSettingSetInt("lastChannelID", atoi(value));
    return 0;
}

static int JseSTBLocalSetPageURLRead(const char* param, char* value, int len)
{
#ifdef HUAWEI_C10
    IND_STRCPY(value, LOCAL_WEBPAGE_PATH_PREFIX"/settings/config.html");
#elif defined(HUAWEI_C20)
    IND_STRCPY(value, LOCAL_WEBPAGE_PATH_PREFIX"/settings/land.html");
#endif
    return 0;
}

static int JseEPGReadyWrite(const char* param, char* value, int len)
{
    business().setEPGReadyFlag(1);
#if defined(HUAWEI_C20)
    app_epgReadyCallback_C20();
    app_netConnectCallback_C20();

    browserEventSend("{\"type\":\"EVENT_AUTHENTICATION_READY\"}", NULL);
    //Hide boot logo picture
    BootImagesShowBootLogo(0);
#endif
    return 0;
}


static int JseLastChanKeyRead(const char* param, char* value, int len)
{
    int lastChannelKey = 0;
    appSettingGetInt("lastChannelID", &lastChannelKey, 0);
#ifdef INCLUDE_DVBS
    int channelType = 0;
    if(!strncmp(param, "DVB", 3)) {
        sysSettingGetInt("LastChannelType", &channelType, 0);
        if(channelType == 2)
            sysSettingGetInt("LastAudioNumber", &lastChannelKey, 0);
        else
            sysSettingGetInt("LastVideoNumber", &lastChannelKey, 0);
        sprintf(value, "{\"chanKey\":%d,\"chanDomain\":\"DVB\"}", lastChannelKey);
    } else
#endif
    sprintf(value, "{\"chanKey\":%d,\"chanDomain\":\"IPTV\"}", lastChannelKey);
    return 0;
}

#ifdef HUAWEI_C20
static int JseServiceModeWrite(const char* param, char* value, int len)
{
#ifdef INCLUDE_TR069
    if(!strcmp(business().getServiceMode(), "")) {
        tr069_statistic_set_userstatus_time();
        tr069StaticticStandy();
    }
#endif
    business().setServiceMode(value);
#if defined (HUAWEI_C20)
    if(strcmp(value, "Hybrid") == 0) {
        app_enterHybirdMode();
    } else if(strcmp(value, "DVB") == 0) {
        app_enterDvbMode();
    } else {
        LogJseDebug("I don't know this mode (%s)\n",  value);
    }
#endif
    return 0;
}
#endif

/*****************************************************************************
 主认证地址(String 1024)，此参数保存在STB Flash，STB返回时拼装上认证参数，
  拼装的参数格式参考IPTV V100R005C03版本STB与MEM接口文档(Webkit) 1.1接入认证接口，
  ？号后的参数为需要封装的参数
 *****************************************************************************/
static int JseMainHomepageUrlRead(const char* param, char* value, int len)
{
    if(APPMODE_IPTV != sys_appmode_get()) {
        sys_appmode_set(APPMODE_IPTV);
    }
    char temp[URL_LEN];
    sysSettingGetString("eds", temp, URL_LEN, 0);
    business().changeUrlFormatFromDomainToIp(temp, URL_LEN); // only for v5
    std::string url = Hippo::Customer().AuthInfo().ConcatenateUrl(temp);
    snprintf(value, len, "%s", url.c_str());
    LogJseDebug("value=[%s]\n", url.c_str());
    return strlen(value);
}

/***************************************************************************
 备认证地址(String 1024)，此参数保存在STB Flash，STB返回时拼装上认证参数，
 拼装的参数格式参考IPTV V100R005C03版本STB与MEM接口文档(Webkit) 1.1接入认证接口，
 ？号后的参数为需要封装的参数
 ***************************************************************************/
static int JseSecondaryHomepageUrlRead(const char* param, char* value, int len)
{
    char temp[URL_LEN] = {0};
    sysSettingGetString("eds1", temp, URL_LEN, 0);
    business().changeUrlFormatFromDomainToIp(temp, URL_LEN); // only for v5
    std::string url = Hippo::Customer().AuthInfo().ConcatenateUrl(temp);
    snprintf(value, len, "%s", url.c_str());
    LogJseDebug("value=[%s]\n", value);
    return strlen(value);
}

/*************************************************
Description: 初始化华为系统配置定义的接口，由JseHuawei.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWBusinessInit()
{
    JseCall* call;

    //C10/C20 regist
    call = new JseFunctionCall("EPGUrl", JseEPGUrlRead, JseEPGUrlWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("epgurl", JseEPGUrlRead, JseEPGUrlWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("StartLocalCfg", 0, JseStartLocalCfgWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("GetAuthenticationURL", JseGetAuthenticationURLRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("LastChannelNo", JseLastChannelNoRead, JseLastChannelNoWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("lastchannelid", JseLastChannelNoRead, JseLastChannelNoWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("lastChanKey", JseLastChanKeyRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("DirectPlay", JseDirectPlayRead, JseDirectPlayWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("directplay", JseDirectPlayRead, JseDirectPlayWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("GlobalKeyTable", 0, JseGlobalKeyTableWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("key_ctrl_ex", 0, JseKeyCtrlExWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("hw_key_ctrl", JseHWKeyCtrlRead, JseHWKeyCtrlWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("WebchannelState", JseWebchannelStateRead, JseWebchannelStateWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("resignon", 0, JseResignonWrite);
    JseRootRegist(call->name(), call);

#if defined(CYBERCLOUD)
    //C10 regist
    call = new JseFunctionCall("handle_cybercloud_start", 0, JseCybercloudStartWrite);
    JseRootRegist(call->name(), call);
#endif

#ifdef HUAWEI_C10
    //C10 regist
    call = new JseFunctionCall("ServiceEntry", 0, JseServiceEntryWrite);
    JseRootRegist(call->name(), call);

#ifdef PLAY_BGMUSIC
    call = new JseFunctionCall("playbgmusic", JsePlayBGMusicRead, JsePlayBGMusicWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("default_bgmusic_url", JseDefaultBGMusicUrlRead,JseDefaultBGMusicUrlWrite);
    JseRootRegist(call->name(), call);
#endif //PLAY_BGMUSIC
#endif //HUAWEI_C10

    //C10 regist
    call = new JseFunctionCall("EPGDomain", JseEPGDomainRead, JseEPGDomainWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("EPGDomainBackup", JseEPGDomainBackupRead, JseEPGDomainBackupWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("endURL", JseEndUrlRead, JseEndUrlWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("ResServerUrl", JseResServerUrlRead, JseResServerUrlWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("PPV", 0, JsePPVWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("stbLocalSetPageURL", JseSTBLocalSetPageURLRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("EPGReady", 0, JseEPGReadyWrite);
    JseRootRegist(call->name(), call);

#ifdef HUAWEI_C20
    call = new JseFunctionCall("serviceMode", 0, JseServiceModeWrite);
    JseRootRegist(call->name(), call);
#endif //HUAWEI_C20

    //C20 regist
    call = new JseFunctionCall("Main_HomepageUrl", JseMainHomepageUrlRead, 0);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("Secondary_HomepageUrl", JseSecondaryHomepageUrlRead, 0);
    JseRootRegist(call->name(), call);

    JseHWAccountInit();
    JseHWAdInit();
    JseHWChannelInit();
    JseHWLogoInit();
    JseHWResourceInit();
    JseHWSessionInit();

    return 0;
}

