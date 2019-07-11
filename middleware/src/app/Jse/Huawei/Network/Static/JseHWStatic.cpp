
#include "JseHWStatic.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "SysSetting.h"
#include "mid_stream.h"
#include "sys_basic_macro.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static int JseSTBIPRead(const char* param, char* value, int len)
{
    sysSettingGetString("ip", value, len, 0);
    return 0;
}

static int JseSTBIPWrite(const char* param, char* value, int len)
{
    sysSettingSetString("ip", value);
    return 0;
}

static int JseNetmaskRead(const char* param, char* value, int len)
{
    sysSettingGetString("netmask", value, len, 0);
    return 0;
}

static int JseNetmaskWrite(const char* param, char* value, int len)
{
    sysSettingSetString("netmask", value);
    return 0;
}

static int JseGatewayRead(const char* param, char* value, int len)
{
    sysSettingGetString("gateway", value, len, 0);
    return 0;
}

static int JseGatewayWrite(const char* param, char* value, int len)
{
    sysSettingSetString("gateway", value);
    return 0;
}

static int JseDNSRead(const char* param, char* value, int len)
{
    sysSettingGetString("dns", value, len, 0);
    return 0;
}

static int JseDNSWrite(const char* param, char* value, int len)
{
    sysSettingSetString("dns", value);
    return 0;
}

static int JseDNS2Read(const char* param, char* value, int len)
{
    sysSettingGetString("dns1", value, len, 0);
    return 0;
}

static int JseDNS2Write(const char* param, char* value, int len)
{
    sysSettingSetString("dns1", value);
    return 0;
}


/*************************************************
Description: 初始化华为网络静态配置定义的接口，由JseNetWork.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWStaticInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("stbIP", JseSTBIPRead, JseSTBIPWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("netmask", JseNetmaskRead, JseNetmaskWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("gateway", JseGatewayRead, JseGatewayWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dns", JseDNSRead, JseDNSWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dns2", JseDNS2Read, JseDNS2Write);
    JseRootRegist(call->name(), call);
    return 0;
}

