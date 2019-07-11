
#include "JseHWDigital.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"
#include "JseRoot.h"

#include "mid_sys.h"
#include "AppSetting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" int yhw_vout_getHDCPStatus(int*);
extern "C" int yhw_vout_setHdcpFailureMode(int);

static int JseHDMIConnectStatusRead(const char *param, char *value, int len)
{
    snprintf(value, len, "%d", mid_sys_HDMIConnect_status_get());
    return 0;
}

/*******************************************************************************
Functional description:
    STB HDCP switch the default value.The parameter save to flash.
Parameter:
    0 is close, 1 is open.
Note:
    The next time the STB comes into effect when using default values.
Following specifications:
    huawei v5(IPTV 海外版本STB与EPG接口文档 内容保护(Webkit) V1.1.doc)
*******************************************************************************/
static int JseHDCPEnableDefaultRead(const char* param, char* value, int len)
{
    int status = 0;

    yhw_vout_getHDCPStatus(&status);
    snprintf(value, len, "%d",status);
    return 0;
}

static int JseHDCPEnableDefaultWrite(const char* param, char* value, int len)
{
    appSettingSetInt("HDCPEnableDefault", atoi(value));
    return 0;
}

static int JseHDCPEnableWrite(const char* param, char* value, int len)
{
    mid_sys_hdcp_enableflag_set(atoi(value));
    return 0;
}

/*******************************************************************************
Functional description:
    STB HDCP protective effect.The parameter save to flash.
Parameter:
    blackScreen is video blackscreen.
    lowResolution is lower resolution output.
Note:
    On the next play HDCP protected content.
Following specifications:
    huawei v5(IPTV 海外版本STB与EPG接口文档 内容保护(Webkit) V1.1.doc)
*******************************************************************************/
static int JseHDCPProtectModeWrite(const char* param, char* value, int len)
{
    if (!strncasecmp(value, "blackScreen", 11)) // video blackscreen
        yhw_vout_setHdcpFailureMode(3); // YX_HDMI_GRAPHIC_ONLY
    else if (!strncasecmp(value, "lowResolution", 13)) // lower resolution output
        yhw_vout_setHdcpFailureMode(2); // YX_HDMI_TO_SD
    else
        LogJseError("JseHDCPProtectModeWrite value unknow\n");
    return 0;
}

/*************************************************
Description: 初始化华为数字输出端口配置定义的接口，由JseHWIO.cpp调用
Input: 无
Return: 无
**************************************************/
int JseHWDigitalInit()
{
    JseCall* call;

    //C10 regist
    call = new JseFunctionCall("HDMIConnectStatus", JseHDMIConnectStatusRead, 0);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("HDMIConnect", JseHDMIConnectStatusRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("HDCPEnableDefault", JseHDCPEnableDefaultRead, JseHDCPEnableDefaultWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("HDCPEnable", 0, JseHDCPEnableWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("HDCPProtectMode", 0, JseHDCPProtectModeWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("HDCPProtectMod", 0, JseHDCPProtectModeWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

