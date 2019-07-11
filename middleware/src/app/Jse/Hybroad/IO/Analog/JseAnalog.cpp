
#include "JseAnalog.h"
#include "JseFunctionCall.h"
#include "JseRoot.h"

#include "AppSetting.h"

#include <stdio.h>
#include <stdlib.h>


static int JseMacroRead(const char* param, char* value, int len)
{
    int macrovision = 0;
    appSettingGetInt("macrovision", &macrovision, 0);
    sprintf(value, "%d", macrovision);
    return 0;
}

static int JseMacroWrite(const char* param, char* value, int len)
{
    appSettingSetInt("macrovision", atoi(value));
    return 0;
}

/*************************************************
Description: 初始化海博模拟输出端口配置定义的接口，由JseIO.cpp调用
Input: 无
Return: 无
**************************************************/
int JseAnalogInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("yx_para_macro", JseMacroRead, JseMacroWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

