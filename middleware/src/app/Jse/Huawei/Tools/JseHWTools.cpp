
#include "JseHWTools.h"
#include "JseAssertions.h"
#include "JseFunctionCall.h"
#include "JseRoot.h"

#include "AES/JseHWAES.h"
#include "ArrayTimer/JseHWArrayTimer.h"
#include "Encrypt/JseHWEncrypt.h"
#include "Record/JseHWRecord.h"

#include "SysSetting.h"
#include "SettingModuleNetwork.h"

#ifdef TVMS_OPEN
#include "tvms_setting.h"
#endif

#include "codec.h"

#include "app_reminderlist.h"
#include "app_epg_para.h"
#include "cryptoFunc.h"
#include "charConvert.h"
#include "mid_fpanel.h"
#include "sys_basic_macro.h"
#include "NetworkFunctions.h"

#include "ipanel_event.h"
#include "sys_key_deal.h"

#include "config.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "NativeHandler.h"

#include <stdio.h>
#include <string.h>

static int JseRefreshReminderListWrite(const char* param, char* value, int len)
{
    httpReminderRequest(0);
    return 0;
}

static int JseHWOpSaveparamWrite(const char* param, char* value, int len)
{
    char ifname[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    LogJseDebug("rewrite param %d, %d\n", network_connecttype_get(ifname), NativeHandlerGetState());

#if defined(PPPOE_REWRITE)
    //just for epg pppoe rewrite
    if (NativeHandlerGetState() == 2) {//Boot 0 Config 1 Running 2
        if (network_connecttype_get(ifname) == NETTYPE_PPPOE) {
            LogJseDebug("PPPoE rewrite, open local url\n");
            sendMessageToNativeHandler(MessageType_System, MV_System_OpenCheckPPPoEAccountPage, 0, 0);
            return 0;
        }
    }
#endif

    settingManagerSave();
#ifdef TVMS_OPEN
    tvms_config_save();
#endif
    return 0;
}

static int JseHWOInformationUrlWrite(const char* param, char* value, int len)
{
    LogJseDebug("This function is not supported.\n");
    return 0;
}

static int JseHWOpSTBWrite(const char* param, char* value, int len)
{
    //hw_op_stb
    if (!strcmp(value, "homePage")) {
        NativeHandlerInputMessage(MessageType_System, EIS_IRKEY_URL_MENU, 0, NULL);
    } else if (!strcmp(value, "powerOff")) {
        NativeHandlerInputMessage(MessageType_System, EIS_IRKEY_PAGE_STANDBY, 0, NULL);
    } else if (!strncmp(value, "fullPowerOff", strlen("fullPowerOff"))) {
        settingManagerSave();
        mid_fpanel_poweroffdeep();
    } else if (!strcmp(value, "setPage")) {
        NativeHandlerInputMessage(MessageType_System, EIS_IRKEY_PAGE_CONFIG, 0, NULL);
    } else if (!strcmp(value, "switchSubtitles")) {
        int tIndex = 0, tNum = 0;

        codec_subtitle_num(&tNum);
        if (tNum > 1) {
            codec_subtitle_get(&tIndex);

            if (tIndex < tNum - 1)
                tIndex ++;
            else
                tIndex = 0;

            codec_subtitle_set(tIndex);
        }
    } else if (!strcmp(value, "restart_initialization_and_authentication")) {
        LogJseDebug("network resume, restart_initialization_and_authentication.....\n");
#if defined(HUAWEI_C20)
        sendMessageToNativeHandler(MessageType_System, NET_CONNECT, 0, 0);
#else
        sendMessageToNativeHandler(MessageType_System, MV_System_OpenBootPage, 0, 0);
        sys_appmode_set(APPMODE_IPTV);
#endif
    } else
        LogJseDebug("I don't know this parms(%s:%s)\n", param, value);
    return 0;
}

static int JseHWOpRestartWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("tr069_upgrades", 0);
    mid_fpanel_reboot();
    return 0;
}

static char g163md5String[1024] = { 0 };
static char gMd5String[1024] = { 0 };

static int JseMd5ExtentedRead(const char* param, char* value, int len)
{
    char* p = gMd5String;
    md5Encypt(&p, 1, value, len, 1);
    data2Hex(value, 16, value, len);
    return 0;
}

static int JseMd5ExtentedWrite(const char* param, char* value, int len)
{
    snprintf(gMd5String, sizeof(gMd5String), "%s", value);
    return 0;
}

static int Jse163md5ExtentedRead(const char* param, char* value, int len)
{
    char*p = g163md5String;
    md5Encypt(&p, 1, value, len, 0);
    data2Hex(value, 16, value, len);
    lower2Upper(value, strlen(value));
    return 0;
}

static int Jse163md5ExtentedWrite(const char* param, char* value, int len)
{
    snprintf(g163md5String, sizeof(g163md5String), "%s", value);
    return 0;
}

/*************************************************
Description: 初始化并注册华为定义的接口 <Tools.***>
接口相关说明见 《IPTV 海外版本STB与EPG接口文档 STB公共能力(Webkit) V1.1》
Input: 无
Return: 无
 *************************************************/
JseHWTools::JseHWTools()
	: JseGroupCall("Tools")
{
    JseCall* call;

    call = new JseHWAES();
    regist(call->name(), call);

    call = new JseHWArrayTimer();
    regist(call->name(), call);

    call = new JseHWEncrypt();
    regist(call->name(), call);

    call = new JseHWRecord();
    regist(call->name(), call);
}

JseHWTools::~JseHWTools()
{
}

/*************************************************
Description: 初始化华为Tools模块配置定义的接口，由JseHuawei.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWToolsInit()
{
    JseCall* call;

    //C10,C20 regist
    call = new JseFunctionCall("hw_op_restart", 0, JseHWOpRestartWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("hw_op_saveparam", 0, JseHWOpSaveparamWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("hw_op_informationurl", 0, JseHWOInformationUrlWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("md5Extented", JseMd5ExtentedRead, JseMd5ExtentedWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("iptvmd5Extented", JseMd5ExtentedRead, JseMd5ExtentedWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("163md5Extented", Jse163md5ExtentedRead, Jse163md5ExtentedWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("RefreshReminderList", 0, JseRefreshReminderListWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("hw_op_stb", 0, JseHWOpSTBWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseHWTools();
    JseRootRegist(call->name(), call);
    return 0;
}
