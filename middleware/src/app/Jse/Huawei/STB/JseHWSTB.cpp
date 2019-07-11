
#include "JseHWSTB.h"
#include "Version/JseHWVersion.h"
#include "Time/JseHWTime.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "mid_sys.h"
#include "ind_mem.h"
#include "sys_basic_macro.h"
#include "NetworkFunctions.h"

#include "customer.h"

#if defined(ANDROID)
#include "AppManager/JseHWAppManager.h"
#endif

#include "AppSetting.h"
#include "SysSetting.h"

#include "stbinfo/stbinfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//C10/C20系统配置
static int JseSupportHDRead(const char *param, char *value, int len)
{
#if (SUPPORTE_HD)
    snprintf(value, len, "%d", 1);
#else
    snprintf(value, len, "%d", 0);
#endif
    return 0;
}

//C10系统配置
static int JseSTBTypeRead(const char *param, char *value, int len)
{
    IND_STRCPY(value, StbInfo::STB::UpgradeModel());
    return 0;
}

//C10系统配置
static int JseSTBSerialNumberRead(const char *param, char *value, int len)
{
    char serial[64] = {0};
    mid_sys_serial(serial);
    snprintf(value, len, "%s", serial);
    return 0;
}

//C10系统配置
static int JseLangRead(const char *param, char *value, int len)
{
    int lang;
    sysSettingGetInt("lang", &lang, 0);
    snprintf(value, len, "%d", lang);
    return 0;
}

//C10系统配置
static int JseLangWrite(const char *param, char *value, int len)
{
    sysSettingSetInt("lang", atoi(value));
    return 0;
}

static int JseMuteFlagRead(const char* param, char* value, int len)
{
    int mute = 0;

    appSettingGetInt("mute", &mute, 0);
    sprintf(value, "%d", mute);
    return 0;
}

static int JseMuteFlagWrite(const char* param, char* value, int len)
{
    appSettingSetInt("mute", atoi(value));
    return 0;
}

//C10系统配置
static int JseHardWareVersionRead(const char *param, char *value, int len)
{
    HardwareVersion(value, 128);
    return 0;
}

//C10系统配置
static int JseUserFieldRead(const char *param, char *value, int len)
{
    int userField = -1;

    appSettingGetInt("UserField", &userField, 0);
    sprintf(value, "%d", userField);
    return 0;
}

static int JseUserFieldWrite(const char *param, char *value, int len)
{
    appSettingSetInt("UserField", atoi(value));
    return 0;
}

static int JseChipSeriaNumberRead(const char *param, char *value, int len)
{
    LogJseDebug("app jse JseChipSeriaNumberRead\n");
    return 0;
}

static int JseMACAddressRead(const char *param, char *value, int len)
{
    char ifname[URL_LEN] = { 0 };
    char ifmac[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    const char* pmac = network_ifacemac_get(ifname, ifmac, URL_LEN);
    if (pmac)
        snprintf(value, len, "%02x:%02x:%02x:%02x:%02x:%02x", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
    return 0;
}

static int JseTokenMACAddressRead(const char * param, char * value, int len)
{
    // Mantis 0024223, 华为测试如下说：
    //      规范要求：STB加入组播、加入单播、请求网络链接携带的MAC都为无线网卡的MAC；
    //      STB向EPG、SQM、TMS上报携带的MAC为机顶盒都为有线网卡的MAC
    network_tokenmac_get(value, len, ':');
    return 0;
}


//get system memory size(int 1) 1 is 128MB, 2 is 256MB, 3 is 512MB.
static int JseMEMConfigRead(const char *param, char *value, int len)
{
    int type = 1; //not support HD default
#if SUPPORTE_HD
    type = 2;
#endif
#if defined(LIAONING_HD)
    type = 3;
#endif
    snprintf(value, len, "%d", type);
    return 0;
}

static int JseSTBModelRead(const char *param, char *value, int len)
{
    strcpy(value, StbInfo::STB::UpgradeModel());
    return 0;
}

//TODO
static int JseWatchDogSwitchRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseWatchDogSwitchWrite(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseWorkModelRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseWorkModelWrite(const char *param, char *value, int len)
{
    return 0;
}

/*************************************************
Description: 初始化华为系统配置定义的接口，由JseHuawei.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWSTBInit()
{
    JseCall* call;

    //C10/C20 regist
    call = new JseFunctionCall("SupportHD", JseSupportHDRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("lang", JseLangRead, JseLangWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("Lang", JseLangRead, JseLangWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("muteFlag", JseMuteFlagRead, JseMuteFlagWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("HardwareVersion", JseHardWareVersionRead, 0);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("HardWareVersion", JseHardWareVersionRead, 0);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("STBType", JseSTBTypeRead, 0);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("STBID", JseSTBSerialNumberRead, 0);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("stbid", JseSTBSerialNumberRead, 0);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("UserField", JseUserFieldRead, JseUserFieldWrite);
    JseRootRegist(call->name(), call);

    // C10 regist
    call = new JseFunctionCall("mac", JseTokenMACAddressRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("ChipSeriaNumber", JseChipSeriaNumberRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("MACAddress", JseMACAddressRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("MEMConfig", JseMEMConfigRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("STB_model", JseSTBModelRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("STBSerialNumber", JseSTBSerialNumberRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("watchDogSwitch", JseWatchDogSwitchRead, JseWatchDogSwitchWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("workModel", JseWorkModelRead, JseWorkModelWrite);
    JseRootRegist(call->name(), call);

#if defined(ANDROID)
    JseHWAppManagerInit();
#endif

    JseHWVersionInit();
    JseHWTimeInit();
    return 0;
}

