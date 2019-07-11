
#include "JseHWAnalog.h"
#include "JseFunctionCall.h"
#include "JseRoot.h"

#include "mid_sys.h"
#include "AppSetting.h"

#include <stdio.h>
#include <stdlib.h>

//TODO
static int JseAnalogOutputEnableWrite(const char *param, char *value, int len)
{
    return 0;
}

/*******************************************************************************
Functional description:
    STB macrovision switch the default value. The parameter save to flash.
Parameter:
    0 is close, 1 is open.
Note:
    The next time the STB comes into effect when using default values.
Following specifications:
    huawei v5(IPTV 海外版本STB与EPG接口文档 内容保护(Webkit) V1.1.doc)
*******************************************************************************/
static int JseMacrovisionEnableDefaultWrite(const char* param, char* value, int len)
{
    appSettingSetInt("macrovision", atoi(value));
    return 0;
}

static int JseMarcovisionRead(const char* param, char* value, int len)
{
    sprintf(value, "%d", mid_sys_Macrovision_get());
    return 0;
}

static int JseMarcovisionWrite(const char* param, char* value, int len)
{
    mid_sys_Macrovision_set(atoi(value));
    return 0;
}

/*******************************************************************************
Functional description:
    STB CGMS-A switch the default value.The parameter save to flash.
Parameter:
    0：CopyFreely，Unlimited copies may be made of the content。
    1：CopyNoMore，One generation of copies has already been made; no further copying is allowed。
    2：CopyOnce，One generation of copies may be made。
    3：CopyNever，No copies may be made of the content。
Note:
    The next time the STB comes into effect when using default values.
Following specifications:
    huawei v5(IPTV 海外版本STB与EPG接口文档 内容保护(Webkit) V1.1.doc)
*******************************************************************************/
static int JseCGMSAEnableDefaultWrite(const char* param, char* value, int len)
{
    appSettingSetInt("CGMSAEnableDefault", atoi(value));
    return 0;
}

/*************************************************
Description: 初始化华为模拟输出端口配置定义的接口，由JseHWIO.cpp调用
Input: 无
Return: 无
**************************************************/
int JseHWAnalogInit()
{
    JseCall* call;

    //C10 regist
    call = new JseFunctionCall("analogOutputEnable", 0, JseAnalogOutputEnableWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("MacrovisionEnableDefault", 0, JseMacrovisionEnableDefaultWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("MacrovisionEnable", JseMarcovisionRead, JseMarcovisionWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("IsOpenMarcovision", JseMarcovisionRead, JseMarcovisionWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("CGMSAEnableDefault", 0, JseCGMSAEnableDefaultWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

