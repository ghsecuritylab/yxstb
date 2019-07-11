
#include "JseHWNetwork.h"
#include "JseRoot.h"
#include "JseCall.h"
#include "JseFunctionCall.h"

#include "DHCP/JseHWDHCP.h"
#include "PPPOE/JseHWPPPOE.h"
#include "Static/JseHWStatic.h"

#include "SysSetting.h"
#include "SettingEnum.h"

#include "mid_stream.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern "C" void mid_net_igmpver_set2proc(const char *itrface, int ver);


static int JseIGMPVersionRead(const char* param, char* value, int len)
{
    int version;
    sysSettingGetInt("igmpversion", &version, 0);
    snprintf(value, len, "{\"IGMPVersion\":%d}", version);
    return 0;
}

static int JseIGMPVersionWrite(const char* param, char* value, int len)
{
    char * p = strchr(value, ':');
    if (!p)
        return -1;
    p++;
    while (*p == ' ' || *p == '\t' || *p == '"' || *p == '\'')
        p++;

    int ver = atoi(p);
    if (ver != 2 && ver != 3)
        return -1;

    sysSettingSetInt("igmpversion", ver);
#ifdef ENABLE_IGMPV3
    mid_net_igmpver_set2proc("all", ver);
#endif
    return 0;
}

static int JseNATRHBintervalWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("NATRHBinterval", atoi(value));
    mid_stream_nat_heartbitperiod(atoi(value));
    return 0;
}

static int JseConnectModeRead(const char* param, char* value, int len)
{
    int netType = 0;
    sysSettingGetInt("nettype", &netType, 0);

    if (netType == NET_ETH)
        netType = 1;
    else if (netType == NET_WIRELESS)
        netType = 2;

    snprintf(value, len, "%d", netType);
    return 0;
}

static int JseConnectTypeRead(const char* param, char* value, int len)
{
	int connectType = 0;

    sysSettingGetInt("connecttype", &connectType, 0);
    snprintf(value, len, "%d", connectType);
    return 0;
}

static int JseConnectTypeWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("connecttype", atoi(value));
    return 0;
}

static int JseAccessMethodRead(const char* param, char* value, int len)
{
	int connectType = 0;

    sysSettingGetInt("connecttype", &connectType, 0);
    switch (connectType) {
    case 1: snprintf(value, len, "pppoe"); break;
    case 2: snprintf(value, len, "dhcp"); break;
    case 4: snprintf(value, len, "ipoe"); break;
    default: snprintf(value, len, "lan"); break;
    }
    return 0;
}
/*************************************************
Description: 初始化华为网络配置定义的接口，由JseHuawei.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWNetworkInit()
{
    JseCall* call;

    //C10/C20 regist
    call = new JseFunctionCall("IGMPVersion", JseIGMPVersionRead, JseIGMPVersionWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("NATRHBinterval", 0, JseNATRHBintervalWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("NatInterval", 0, JseNATRHBintervalWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("ConnectMode", JseConnectModeRead, 0);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("ConnectType", JseConnectTypeRead, JseConnectTypeWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("connecttype", JseConnectTypeRead, JseConnectTypeWrite);
    JseRootRegist(call->name(), call);

    // Jiangsu regist
    // 中兴平台认证需要，因此可能不止江苏需要。
    call = new JseFunctionCall("AccessMethod", JseAccessMethodRead, NULL);
    JseRootRegist(call->name(), call);

    JseHWDHCPInit();
    JseHWPPPOEInit();
    JseHWStaticInit();
    return 0;
}

