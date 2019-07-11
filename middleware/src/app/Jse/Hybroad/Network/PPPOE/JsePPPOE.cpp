
#include "JsePPPOE.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "SysSetting.h"
#include "AppSetting.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef ENABLE_IPV6
static int JseIpv6NetuseraccountRead(const char* param, char* value, int len)
{
    sysSettingGetString("ipv6_netuser", value, len, 0);
    return 0;
}

static int JseIpv6NetuseraccountWrite(const char* param, char* value, int len)
{
    sysSettingSetString("ipv6_netuser", value);
    return 0;
}

static int JseIpv6NetuserpasswordRead(const char* param, char* value, int len)
{
    sysSettingGetString("ipv6_netpasswd", value, len, 0);
    return 0;
}

static int JseIpv6NetuserpasswordWrite(const char* param, char* value, int len)
{
    sysSettingSetString("ipv6_netpasswd", value);
    return 0;
}
#endif

static int JsePADIRetryTimeRead(const char* param, char* value, int len)
{
    int retryTime = 0;
    appSettingGetInt("PADIRetryTime", &retryTime, 0);
    snprintf(value, len, "%d", retryTime);
    return 0;
}

static int JsePADIRetryTimeWrite(const char* param, char* value, int len)
{
    int time = atoi(value);
    if (time < 0)
        return -1;
    appSettingSetInt("PADIRetryTime", time);
	settingManagerSave();
    return 0;
}

static int JsePADIRetryIntervalRead(const char* param, char* value, int len)
{
    int retryInterval = 0;
    appSettingGetInt("PADIRetryInterval", &retryInterval, 0);
    snprintf(value, len, "%d", retryInterval);
    return 0;
}

static int JsePADIRetryIntervalWrite(const char* param, char* value, int len)
{
    int interval = atoi(value);
    if (interval <= 0)
        return -1;
    appSettingSetInt("PADIRetryInterval", interval);
	settingManagerSave();
    return 0;
}

static int JseLCPRetryTimeRead(const char* param, char* value, int len)
{
    int retryTime = 0;
    appSettingGetInt("LCPRetryTime", &retryTime, 0);
    snprintf(value, len, "%d", retryTime);
    return 0;
}

static int JseLCPRetryTimeWrite(const char* param, char* value, int len)
{
    int time = atoi(value);
    if (time <= 0)
        return -1;
    appSettingSetInt("LCPRetryTime", time);
	settingManagerSave();
    return 0;
}

static int JseLCPRetryIntervalRead(const char* param, char* value, int len)
{
    int retryInterval = 0;
    appSettingGetInt("LCPRetryInterval", &retryInterval, 0);
    snprintf(value, len, "%d", retryInterval);
    return 0;
}

static int JseLCPRetryIntervalWrite(const char* param, char* value, int len)
{
    int interval = atoi(value);
    if (interval <= 0)
        return -1;
    appSettingSetInt("LCPRetryInterval", interval);
	settingManagerSave();
    return 0;
}

static int isSelectedPPPIPoE = 0;

static int JseIsSelectedPPPIPoERead(const char* param, char* value, int len)
{
    snprintf(value, 2, "%d", isSelectedPPPIPoE);
    return 0;
}

static int JseIsSelectedPPPIPoEWrite(const char* param, char* value, int len)
{
    if(0 == atoi(value) || 1 == atoi(value))
        isSelectedPPPIPoE = atoi(value);
    return 0;
}

/*************************************************
Description: 初始化海博PPPOE配置定义的接口,由JseNetwork.cpp调用
Input: 无
Return: 无
 *************************************************/
int JsePPPOEInit()
{
    JseCall* call;

#ifdef ENABLE_IPV6
    call = new JseFunctionCall("Ipv6Netuseraccount", JseIpv6NetuseraccountRead, JseIpv6NetuseraccountWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("Ipv6Netuserpassword", JseIpv6NetuserpasswordRead, JseIpv6NetuserpasswordWrite);
    JseRootRegist(call->name(), call);
#endif

    call = new JseFunctionCall("PADIRetryTime", JsePADIRetryTimeRead, JsePADIRetryTimeWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("PADIRetryInterval", JsePADIRetryIntervalRead, JsePADIRetryIntervalWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("LCPRetryTime", JseLCPRetryTimeRead, JseLCPRetryTimeWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("LCPRetryInterval", JseLCPRetryIntervalRead, JseLCPRetryIntervalWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("isSelectedPPPIPoE", JseIsSelectedPPPIPoERead, JseIsSelectedPPPIPoEWrite);
    JseRootRegist(call->name(), call);

    return 0;
}
