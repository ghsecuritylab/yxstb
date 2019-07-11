
#include "JseHWSQARet.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "SysSetting.h"

#include <stdlib.h>

static int JseRETDelayTimeWrite(const char *param, char *value, int len)
{
    sysSettingSetInt("RETDelayTime", atoi(value));
    return 0;
}

static int JseRETIntervalWrite(const char *param, char *value, int len)
{
    sysSettingSetInt("RETInterval", atoi(value));
    return 0;
}

static int JseRetSendPeriod(const char *param, char *value, int len)
{
    sysSettingSetInt("RETSendPeriod", atoi(value));
    return 0;
}

//TODO
static int JseSendRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseSendWrite(const char *param, char *value, int len)
{
    return 0;
}

JseHWSQARet::JseHWSQARet()
    : JseGroupCall("Ret")
{
    JseCall* call;

    call = new JseFunctionCall("Send", JseSendRead, JseSendWrite);
    regist(call->name(), call);
}

JseHWSQARet::~JseHWSQARet()
{
}

/*************************************************
Description: 初始化华为播放流控中SQA模块中的Ret配置定义的接口，由JseHWSQA.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWSQARetInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("RETDelayTime", 0, JseRETDelayTimeWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("RETInterval", 0, JseRETIntervalWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("RETSendPeriod", 0, JseRetSendPeriod);
    JseRootRegist(call->name(), call);
    return 0;
}

