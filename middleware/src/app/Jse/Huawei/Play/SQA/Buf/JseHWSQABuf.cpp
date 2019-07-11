
#include "JseHWSQABuf.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "SysSetting.h"

#include <stdlib.h>

static int JseBufferHLevelWrite(const char *param, char *value, int len)
{
    sysSettingSetInt("SQABufferHLevel", atoi(value));
    return 0;
}

static int JseBufferMLevelWrite(const char *param, char *value, int len)
{
    sysSettingSetInt("SQABufferMLevel",atoi(value));
    return 0;
}

static int JseBufferLLevelWrite(const char *param, char *value, int len)
{
    sysSettingSetInt("SQABufferLLevel", atoi(value));
    return 0;
}

//TODO
static int JseBurstIntervalRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseBurstIntervalWrite(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JsePriorityRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JsePriorityWrite(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseSizeRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseSizeWrite(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseLevelRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseLevelWrite(const char *param, char *value, int len)
{
    return 0;
}

JseHWSQABuf::JseHWSQABuf()
    : JseGroupCall("Buf")
{
    JseCall* call;

    call = new JseFunctionCall("BurstInterval", JseBurstIntervalRead, JseBurstIntervalWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("Priority", JsePriorityRead, JsePriorityWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("Size", JseSizeRead, JseSizeWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("Level", JseLevelRead, JseLevelWrite);
    regist(call->name(), call);
}

JseHWSQABuf::~JseHWSQABuf()
{
}

/*************************************************
Description: 初始化华为播放流控中SQA模块中Buf的配置定义的接口，由JseHWPlays.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWSQABufInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("SQABufferHLevel", 0, JseBufferHLevelWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("SQABufferMLevel", 0, JseBufferMLevelWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("SQABufferLLevel", 0, JseBufferLLevelWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

