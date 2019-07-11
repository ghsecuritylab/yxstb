
#include "JseNetwork.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "Static/JseStatic.h"
#include "DHCP/JseDHCP.h"
#include "PPPOE/JsePPPOE.h"

#ifdef INCLUDE_WIFI
#include "Wifi/JseWifi.h"
#endif

#include "SysSetting.h"
#include "AppSetting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern "C" int yos_systemcall_runSystemCMD(char *buf, int *ret);

static int JseIpv4EnableRead(const char* param, char* value, int len)
{
    int ipv4_enable = -1;
    sysSettingGetInt("ipv4_enable", &ipv4_enable, 0);
    sprintf(value, "%d", ipv4_enable);
    return 0;
}

static int JseIpv4EnableWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("ipv4_enable", atoi(value));
    return 0;
}

static int JseIpv6EnableRead(const char* param, char* value, int len)
{
    int ipv6_enable = -1;
    sysSettingGetInt("ipv6_enable", &ipv6_enable, 0);
    sprintf(value, "%d", ipv6_enable);
    return 0;
}

static int JseIpv6EnableWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("ipv6_enable", atoi(value));
    return 0;
}

static int JseNetworkTypeRead(const char* param, char* value, int len)
{
	int netType = 0;
    sysSettingGetInt("nettype", &netType, 0);
    sprintf(value, "%d", netType);
    return 0;
}

static int JseNetworkTypeWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("nettype", atoi(value));
    return 0;
}

static int JseDoubleStackRead(const char* param, char* value, int len)
{
    sysSettingGetString("enableDoubleStack", value, len, 0);
    return 0;
}

static int JseDoubleStackWrite(const char* param, char* value, int len)
{
    sysSettingSetString("enableDoubleStack", value);
    return 0;
}

static int JseLinkProbeTimeRead(const char* param, char* value, int len)
{
    int probeTime = 0;
    appSettingGetInt("LinkProbeTime", &probeTime, 0);
    snprintf(value, len, "%d", probeTime);
    return 0;
}

static int JseLinkProbeTimeWrite(const char* param, char* value, int len)
{
    int time = atoi(value);
    if (time <= 0)
        return -1;
    appSettingSetInt("LinkProbeTime", time);
	settingManagerSave();
    return 0;
}

static int JseNetworkCardSetRead(const char* param, char* value, int len)
{
    if(0 == access("/root/networkCardMode_10H", F_OK)) // file exist return 0, unexist return -1
        snprintf(value, len, "10H");
    else if(0 == access("/root/networkCardMode_10F", F_OK))
        snprintf(value, len, "10F");
    else if(0 == access("/root/networkCardMode_100H", F_OK))
        snprintf(value, len, "100H");
    else if(0 == access("/root/networkCardMode_100F", F_OK))
        snprintf(value, len, "100F");
    else
        snprintf(value, len, "auto"); // when exist networkCardMode_auto file, or unexist any of the above file.
    LogJseDebug("Here is read networkCardSet.[%s].\n", value);

    return 0;
}

static int JseNetworkCardSetWrite(const char* param, char* value, int len)
{
    if((0 != strcmp("10H", value)) && (0 != strcmp("10F", value))  && (0 != strcmp("100H", value))
        && (0 != strcmp("100F", value))  && (0 != strcmp("auto", value))) {
        LogJseError("networkCardSet Mode error!\n");
        return 0;
    }
#if defined(hi3560e)
    system("rm -rf /root/networkCardMode_*");
#else
    yos_systemcall_runSystemCMD("rm -rf /root/networkCardMode_*", NULL);
#endif
    char fileName[sizeof("/root/networkCardMode_") + 4] = "/root/networkCardMode_"; // the 100F is the max value, is 4.
    strncpy(fileName + sizeof("/root/networkCardMode_") - 1, value, 4);
    if(-1 == creat(fileName, S_IRWXU | S_IRWXG | S_IRWXO))
        LogJseError("Create file error! fileName[%s]\n", fileName);
    return 0;
}

static int JseVlanIdARead(const char* param, char* value, int len)
{
    int vlanid_a = 0;
    appSettingGetInt("vlanid_a", &vlanid_a, 0);
    sprintf(value, "%d", vlanid_a);
    return 0;
}

static int JseVlanIdAWrite(const char* param, char* value, int len)
{
    appSettingSetInt("vlanid_a", atoi(value));
    return 0;
}

static int JseVlanIdBRead(const char* param, char* value, int len)
{
    int vlanid_b = 0;
    appSettingGetInt("vlanid_b", &vlanid_b, 0);
    sprintf(value, "%d", vlanid_b);
    return 0;
}

static int JseVlanIdBWrite(const char* param, char* value, int len)
{
    appSettingSetInt("vlanid_b", atoi(value));
    return 0;
}

#ifdef ENABLE_IPV6
static int JseIpv6ConnectTypeRead(const char* param, char* value, int len)
{
    sysSettingGetString("ipv6_connectType", value, len, 0);
    return 0;
}

static int JseIpv6ConnectTypeWrite(const char* param, char* value, int len)
{

    sysSettingSetString("ipv6_connectType", value);
    return 0;
}
#endif

/*************************************************
Description: 初始化海博网络配置定义的接口([hybroad.stb.network.***],由JseSTB.cpp调用
Input: 无
Return: 无
**************************************************/
JseNetwork::JseNetwork()
	: JseGroupCall("network")
{
    JseCall *call;

#ifdef INCLUDE_WIFI
    call = new JseWifi();
    regist(call->name(), call);
#endif
}

JseNetwork::~JseNetwork()
{
}

/*************************************************
Description: 初始化海博网络配置定义的接口(非[hybroad.stb.network.***],由JseSTB.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseNetworkInit()
{
    JseCall* call;

    //C10/C20 regist
    call = new JseFunctionCall("ipv4_enable", JseIpv4EnableRead, JseIpv4EnableWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("ipv6_enable", JseIpv6EnableRead, JseIpv6EnableWrite);
    JseRootRegist(call->name(), call);

    //The following are all C10/C20 regist
    call = new JseFunctionCall("NetworkType", JseNetworkTypeRead, JseNetworkTypeWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("doublestack", JseDoubleStackRead, JseDoubleStackWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("LinkProbeTime", JseLinkProbeTimeRead, JseLinkProbeTimeWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("networkCardSet", JseNetworkCardSetRead, JseNetworkCardSetWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("vlanid_a", JseVlanIdARead, JseVlanIdAWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("vlanid_b", JseVlanIdBRead, JseVlanIdBWrite);
    JseRootRegist(call->name(), call);

#ifdef ENABLE_IPV6
    call = new JseFunctionCall("Ipv6connecttype", JseIpv6ConnectTypeRead, JseIpv6ConnectTypeWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("Ipv6ConnectType", JseIpv6ConnectTypeRead, JseIpv6ConnectTypeWrite);
    JseRootRegist(call->name(), call);
#endif

    JseStaticInit();
    JseDHCPInit();
    JsePPPOEInit();
    return 0;
}

