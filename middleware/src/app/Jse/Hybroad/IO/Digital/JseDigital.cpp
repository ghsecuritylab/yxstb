
#include "JseDigital.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"
#include "JseRoot.h"

#include "mid_sys.h"
#include "AppSetting.h"

#include <stdio.h>
#include <stdlib.h>

static int JseHdcpKeyRead(const char* param, char* value, int len)
{
    int hdcpKey = 0;
    appSettingGetInt("hdcpkey", &hdcpKey, 0);
    sprintf(value, "%d", hdcpKey);
    return 0;
}

static int JseHdcpKeyWrite(const char* param, char* value, int len)
{
    appSettingSetInt("hdcpkey", atoi(value));
    return 0;
}

static int JseHdmiNgotiationRead(const char* param, char* value, int len)
{
    int hdmiNegotiation = 0;
    appSettingGetInt("hdmi_negotiation", &hdmiNegotiation, 0);
    sprintf(value, "%d", hdmiNegotiation);
    return 0;
}

static int JseHdmiNgotiationWrite(const char* param, char* value, int len)
{
    appSettingSetInt("hdmi_negotiation", atoi(value));
    return 0;
}

/*************************************************
Description: 初始化海博数字输出端口配置定义的接口，由JseIO.cpp调用
Input: 无
Return: 无
**************************************************/
int JseDigitalInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("yx_para_hdcpkey", JseHdcpKeyRead, JseHdcpKeyWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("yx_para_hdmi_negotiation", JseHdmiNgotiationRead, JseHdmiNgotiationWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

