
#include "JseDHCP.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "SysSetting.h"
#include "AppSetting.h"

#include <stdio.h>
#include <stdlib.h>

static int JseIpv4DhcpUserRead(const char* param, char* value, int len)
{
    sysSettingGetString("dhcpuser", value, len, 0);
    return 0;
}

static int JseIpv4DhcpUserWrite(const char* param, char* value, int len)
{
    sysSettingSetString("dhcpuser", value);
    return 0;
}

static int JseIpv4DhcpPasswordRead(const char* param, char* value, int len)
{
    sysSettingGetString("ipoeAESpasswd", value, len, 0);
    return 0;
}

static int JseIpv4DhcpPasswordWrite(const char* param, char* value, int len)
{
    sysSettingSetString("ipoeAESpasswd", value);
    return 0;
}

static int JseDHCPRetryTimeRead(const char* param, char* value, int len)
{
    int retryTime = 0;
    appSettingGetInt("DHCPRetryTime", &retryTime, 0);
    snprintf(value, len, "%d", retryTime);
    return 0;
}

static int JseDHCPRetryTimeWrite(const char* param, char* value, int len)
{
    if (value[0] < '0' || value[0] > '9')
        return -1;
    int time = atoi(value);
    if (time < 0)
        return -1;
    appSettingSetInt("DHCPRetryTime", time);
	settingManagerSave();
    return 0;
}

static int JseDHCPRetryIntervalRead(const char* param, char* value, int len)
{
    int retryInterval = 0;
    appSettingGetInt("DHCPRetryInterval", &retryInterval, 0);
    snprintf(value, len, "%d", retryInterval);
    return 0;
}

static int JseDHCPRetryIntervalWrite(const char* param, char* value, int len)
{
    int interval = atoi(value);
    if (interval <= 0)
        return -1;
    appSettingSetInt("DHCPRetryInterval", interval);
	settingManagerSave();
    return 0;
}

static int JseIpoeNeedPasswordRead(const char* param, char* value, int len)
{
    if(value)
        sysSettingGetString("ipoeneedpswd", value, len, 0);
    return 0;
}

static int JseIpoeNeedPasswordWrite(const char* param, char* value, int len)
{
    if(value)
        sysSettingSetString("ipoeneedpswd", value);
    return 0;
}

static int JsePPPoEConnectOKWithDefaultRead(const char* param, char* value, int len)
{
    int ipoeFlag = 0;
    sysSettingGetInt("ipoe_flag", &ipoeFlag, 0);
    snprintf(value, 2, "%d", ipoeFlag);
    return 0;
}

static int JsePPPoEConnectOKWithDefaultWrite(const char* param, char* value, int len)
{
    return sysSettingSetInt("ipoe_flag", atoi(value));
}

/*************************************************
Description: 初始化海博网络DHCP配置定义的接口,由JseNetwork.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseDHCPInit()
{
    JseCall* call;

    //DHCP
    call = new JseFunctionCall("Ipv4DhcpUser", JseIpv4DhcpUserRead, JseIpv4DhcpUserWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("Ipv4DhcpPassword", JseIpv4DhcpPasswordRead, JseIpv4DhcpPasswordWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("DHCPRetryTime", JseDHCPRetryTimeRead, JseDHCPRetryTimeWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("DHCPRetryInterval", JseDHCPRetryIntervalRead, JseDHCPRetryIntervalWrite);
    JseRootRegist(call->name(), call);

    //IPOE
    call = new JseFunctionCall("IpoeUsername", JseIpv4DhcpUserRead, JseIpv4DhcpUserWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("IpoePassword", JseIpv4DhcpPasswordRead, JseIpv4DhcpPasswordWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("IpoeNeedPassword", JseIpoeNeedPasswordRead, JseIpoeNeedPasswordWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("ipoe_flag", JsePPPoEConnectOKWithDefaultRead, JsePPPoEConnectOKWithDefaultWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

