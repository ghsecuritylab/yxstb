
#include "JseWifi.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"
#include "SysSetting.h"

#include "NetworkFunctions.h"

#include "sys_basic_macro.h"
#include "app_epg_para.h"
#include "json/json.h"
#include "json/json_object.h"
#include "jse.h"

#include <stdio.h>
#include <string.h>

static int JsePluginRead(const char* param, char* value, int len)
{
    NetworkCard* device = networkManager().getDevice(network_wifi_devname()); //getActiveDevice();
    int status = device->linkStatus();
    if (!status)
        device->linkUp(); //up the wifi card manually if check it plugined.
    return sprintf(value, "%d", status > 0 ? 1 : 0);
}

static int JseEssidRead(const char* param, char* value, int len)
{
    return sysSettingGetString("wifi_ssid", value, len, 0);
}

static int JseEssidWrite(const char* param, char* value, int len)
{
    return sysSettingSetString("wifi_ssid", value);
}

static int JsePasswordRead(const char* param, char* value, int len)
{
    sysSettingGetString("wifi_password", value, ENCRYPT_PASSWORD_LEN, 0);
    return 0;
}

static int JsePasswordWrite(const char* param, char* value, int len)
{
    sysSettingSetString("wifi_password", value);
    return 0;
}

static int JseEncrTypeRead(const char* param, char* value, int len)
{
    return sysSettingGetString("wifi_encryType", value, len, 0);
}

static int JseEncrTypeWrite(const char* param, char* value, int len)
{
    return sysSettingSetString("wifi_encryType", value);
}

static int JseAuthModeRead(const char* param, char* value, int len)
{
    return sysSettingGetString("wifi_authType", value, len, 0);
}

static int JseAuthModeWrite(const char* param, char* value, int len)
{
    return sysSettingSetString("wifi_authType", value);
}

static int JseSurveyWrite(const char* param, char* value, int len)
{
    NetworkCard* device = networkManager().getDevice(network_wifi_devname()); //getActiveDevice();
    if (NetworkCard::NT_WIRELESS != device->getType())
        return 0;
    return static_cast<WirelessNetworkCard*>(device)->findAccessPoints();
}

static int JseGetCellsRead(const char* param, char* value, int len)
{
    NetworkCard* device = networkManager().getDevice(network_wifi_devname()); //getActiveDevice();
    if (NetworkCard::NT_WIRELESS != device->getType())
        return 0;
    int length = 0;
    std::list<WifiAccessPoint> aps;
    std::list<WifiAccessPoint>::iterator iter;
    static_cast<WirelessNetworkCard*>(device)->getAccessPoints(aps);
    aps.sort();
    length = sprintf(value, "{\"nCells\" : %d, \"Cells\":[ ", aps.size());
    for (iter = aps.begin(); iter != aps.end(); ++iter) {
        length += sprintf(value + length, "{\"essid\":\"%s\",\"quality\":%d,\"enc\":\"%s\",\"auth\":\"%s\" ,\"channel\":\"%d\"},",
            iter->getEssid(),
            iter->getQuality(),
            iter->getEncrType(),
            iter->getAuthMode(),
            iter->getChannel());
        if (length > 2048)
            break;
    }
    strcat(value + length - 1, "]}");
    LogJseDebug("//TODO DebugLog: %s\n", value);
    return 0;
}

static int JseJoinRead(const char* param, char* value, int len)
{
    struct json_object* obj = json_tokener_parse(param);
    if (!obj)
        return -1;
    std::string essid = json_object_get_string(json_object_object_get(obj, "essid"));
    std::string psswd = json_object_get_string(json_object_object_get(obj, "password"));
    json_object_put(obj);

    int ret = 0;
    WirelessNetworkCard* wifi = static_cast<WirelessNetworkCard*>(networkManager().getDevice(network_wifi_devname())); //getActiveDevice();
    if (!wifi)
        return -1;
    wifi->linkDown(); //turn card down, otherwise the status of 'iwpriv rausb0 connStatus' is wrong
    wifi->linkUp();
    ret = wifi->joinAccessPoint(essid.c_str(), psswd.c_str());
    if (0 == ret)
        strncpy(value, essid.c_str(), essid.size());
    return 0;
}

/*************************************************
Description: 初始化海博Wifi配置定义的接口([hybroad.stb.network.wifi.***]),由JseNetwork.cpp调用
Input: 无
Return: 无
 *************************************************/
JseWifi::JseWifi()
	: JseGroupCall("wifi")
{
    JseCall *call;

    call = new JseFunctionCall("plugin", JsePluginRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("essid", JseEssidRead, JseEssidWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("password", JsePasswordRead, JsePasswordWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("encrType", JseEncrTypeRead, JseEncrTypeWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("authMode", JseAuthModeRead, JseAuthModeWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("survey", 0, JseSurveyWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("getCells", JseGetCellsRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("join", JseJoinRead, 0);
    regist(call->name(), call);
}

JseWifi::~JseWifi()
{
}

