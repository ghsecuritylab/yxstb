
#include "JseSTB.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"
#include "Version/JseVersion.h"
#include "Time/JseTime.h"

#include "AppSetting.h"
#include "SysSetting.h"

#include "customer.h"

#include "NetworkFunctions.h"
#include "ind_mem.h"
#include "libzebra.h"
#include "sys_basic_macro.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern "C" char* Hybroad_getPlantformDesc();

static int JseModelRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseModelWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseUpgradeModelRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseUpgradeModelWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseNameRead(const char* param, char* value, int len)
{
	return appSettingGetString("userSTBName", value, len, 0);
}

static int JseNameWrite(const char* param, char* value, int len)
{
	return appSettingSetString("userSTBName", (const char*)value);
}

static int JseOSRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseLangueRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseLangueWrite(const char* param, char* value, int len)
{
	return 0;
}

static int JseSTBCpuRead(const char* param, char* value, int len)
{
    snprintf(value, len, "%s", Hybroad_getPlantformDesc());
    return 0;
}

static int JseSTBFlashRead(const char* param, char* value, int len)
{
    char HardwareVer[128] = {0};

    HardwareVersion(HardwareVer, 128);
    if(0 == strncmp(HardwareVer, "EC2108V3", 8) || 0 == strncmp(HardwareVer, "M8043V02", 8))
        IND_STRCPY(value, "256M");
    else
        IND_STRCPY(value, "128M");
    return 0;
}

static int JseMACAddressRead(const char* param, char* value, int len)
{
    char ifname[URL_LEN] = { 0 };
    char ifmac[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    const char* pmac = network_ifacemac_get(ifname, ifmac, URL_LEN);
    if (pmac)
        sprintf(value, "%02x:%02x:%02x:%02x:%02x:%02x", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
    return 0;
}

static int JseStbSofTypeRead(const char* param, char* value, int len)
{
    sysSettingGetString("stb_softtype", value, len, 0);
    return 0;
}

static int JseStbSofTypeWrite(const char* param, char* value, int len)
{
    sysSettingSetString("stb_softtype", value);
    return 0;
}

static int JseOSVersionRead(const char* param, char* value, int len)
{
#if(SUPPORTE_HD)
    char *pOSversion = getenv("OS_VERSION");
    if (pOSversion) {
        if (!strstr(pOSversion, "linux") && !strstr(pOSversion, "Linux"))
            snprintf(value, len, "linux %s", pOSversion);
        else
            IND_STRCPY(value, pOSversion);
    } else {
        LogJseError("get OS_VERSION error, use linux 2.16.31\n");
        IND_STRCPY(value, "linux 2.16.31");
    }
#else
    IND_STRCPY(value, "linux 2.6.14_hisilicon");
#endif
    return 0;
}

static int JseSTBSerialNumber17Read(const char* param, char* value, int len)
{
#if defined(hi3716m)
    if (!yhw_env_getSerialNumByLength(value, 24)) { //get 24 serial number ok.
        LogJseDebug("get 24 serial number: %s\n", value);
    } else if (!yhw_env_getSerialNumByLength(value, 17)) {//get 17 serial number ok.
        LogJseDebug("get 17 serial number: %s\n", value);
    } else { //get serial number error.
        LogJseError("get serial number error!\n");
    }

    return 0;
#else
    char *tSerialNum = NULL;

    if(0 == yhw_board_getSerialNum(&tSerialNum))
        sprintf(value, "%s", tSerialNum);
    return 0;
#endif
}

static int JseGotoMenuTimeoutRead(const char* param, char* value, int len)
{
	int tGotoMenuTimeout = 0;
    sysSettingGetInt("gotomenu", &tGotoMenuTimeout, 0);
    snprintf(value, len, "%d", tGotoMenuTimeout);
    return 0;
}

static int JseGotoMenuTimeoutWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("gotomenu", atoi(value));
    return 0;
}

static int JseAutoStandbyModeRead(const char* param, char* value, int len)
{
	int tAutoStandbtMode = 0;
	sysSettingGetInt("AutoStandbyMode", &tAutoStandbtMode, 0);
    LogJseDebug("Get auto standby mode %d\n", tAutoStandbtMode);
    snprintf(value, len, "%d", tAutoStandbtMode);
    return 0;
}

static int JseAutoStandbyModeWrite(const char* param, char* value, int len)
{
    LogJseDebug("Set auto standby mode %d\n", atoi(value));
    sysSettingSetInt("AutoStandbyMode", atoi(value));
    return 0;
}

static int JseAutoStandbyTimeRead(const char* param, char* value, int len)
{
    int tAutoStandbtTime = 0;
    sysSettingGetInt("AutoStandbyTime", &tAutoStandbtTime, 0);
    snprintf(value, len, "%d", tAutoStandbtTime);
    return 0;
}

static int JseAutoStandbyTimeWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("AutoStandbyTime", atoi(value));
    return 0;
}

static int JseInitVolumeRead(const char* param, char* value, int len)
{
    int volume = 0;

    appSettingGetInt("volume", &volume, 0);
    //volume = volume * 5;    lh  2010 -4-15 ????????????????H????
    sprintf(value, "%d", volume);
    return 0;
}

/*************************************************
Description: 初始化海博配系统置定义的([hybroad.stb.*****]字段)接口，由JseHybroad.cpp调用
Input: 无
Return: 无
 *************************************************/
JseSTB::JseSTB()
	: JseGroupCall("stb")
{
    JseCall *call;

    call = new JseVersion();
    regist(call->name(), call);

    call = new JseFunctionCall("model", JseModelRead, JseModelWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("upgradeModel", JseUpgradeModelRead, JseUpgradeModelWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("name", JseNameRead, JseNameWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("os", JseOSRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("langue", JseLangueRead, JseLangueWrite);
    regist(call->name(), call);
}

JseSTB::~JseSTB()
{
}

/*************************************************
Description: 初始化海博配系统置定义的(非[hybroad.stb.*****]字段)接口，由JseHybroad.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseSTBInit()
{
    JseCall* call;

    call = new JseFunctionCall("STBCpu", JseSTBCpuRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("STBFlash", JseSTBFlashRead, 0);
    JseRootRegist(call->name(), call);

    // 按C30规范： <<IPTV 国内 STB与EPG子系统接口规范>>
    // mac字段被定义为盒子标识mac
    // 这个挪到Huawei分类里去。
    // call = new JseFunctionCall("mac", JseMACAddressRead, 0);
    // JseRootRegist(call->name(), call);

    call = new JseFunctionCall("MacAddr", JseMACAddressRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_stbsftype", JseStbSofTypeRead, JseStbSofTypeWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("OSVersion", JseOSVersionRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("STBSerialNumber17", JseSTBSerialNumber17Read, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("GotoMenuTimeout", JseGotoMenuTimeoutRead, JseGotoMenuTimeoutWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("AutoStandbyMode", JseAutoStandbyModeRead, JseAutoStandbyModeWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("AutoStandbyTime", JseAutoStandbyTimeRead, JseAutoStandbyTimeWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("initVolume", JseInitVolumeRead, 0);
    JseRootRegist(call->name(), call);

    JseTimeInit();
    JseVersionInit();
    return 0;
}
