
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
Description: ��ʼ����Ϊ����������SQAģ���е�Ret���ö���Ľӿڣ���JseHWSQA.cpp����
Input: ��
Return: ��
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

