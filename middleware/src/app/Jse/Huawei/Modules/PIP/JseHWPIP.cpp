
#include "JseHWPIP.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "AppSetting.h"

#include <stdio.h>

static int JseLastPipChannelNoRead(const char* param, char* value, int len)
{
    int lastPipChannelNo = 0;
    appSettingGetInt("pipchannelid", &lastPipChannelNo, 0);
    sprintf(value, "%d", lastPipChannelNo);
    return 0;
}

/*************************************************
Description: 初始化华为PIP模块配置定义的接口，由JseModules.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWPIPInit()
{
    JseCall* call;

    call = new JseFunctionCall("lastPipChannelNo", JseLastPipChannelNoRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("lastpipID", JseLastPipChannelNoRead, 0);
    JseRootRegist(call->name(), call);
    return 0;
}

