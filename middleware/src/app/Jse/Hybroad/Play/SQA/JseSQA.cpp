
#include "JseSQA.h"
#include "JseRoot.h"
#include "JseCall.h"
#include "JseFunctionCall.h"

#include "sys_msg.h"
#include "mid_sys.h"

#include "SysSetting.h"
#include "AppSetting.h"

#include <stdio.h>
#include <stdlib.h>

static int JseFccSwitchRead(const char* param, char* value, int len)
{
    int fccSwitch = 0;
    appSettingGetInt("fcc_switch", &fccSwitch, 0);
    sprintf(value, "%d", fccSwitch);
    return 0;
}

static int JseFccSwitchWrite(const char* param, char* value, int len)
{
    appSettingSetInt("fcc_switch", atoi(value));
    return 0;
}

static int JseFccTimeRead(const char* param, char* value, int len)
{
    sysSettingGetString("fcc_time", value, len, 0);
    return 0;
}

static int JseFccTimeWrite(const char* param, char* value, int len)
{
    sysSettingSetString("fcc_time", value);
    return 0;
}

static int JseSqaFccRead(const char* param, char* value, int len)
{
    int flag = 0;
    sysSettingGetInt("sqa_fcc", &flag, 0);
    sprintf(value, "%d", flag);
    return 0;
}

static int JseSqaFccWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("sqa_fcc", atoi(value));
    return 0;
}

static int JseSqaRetRead(const char* param, char* value, int len)
{
    int flag = 0;
    sysSettingGetInt("sqa_ret", &flag, 0);
    sprintf(value, "%d", flag);
    return 0;
}

static int JseSqaRetWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("sqa_ret", atoi(value));
    return 0;
}

static int JseSqaFecRead(const char* param, char* value, int len)
{
    int flag = 0;
    sysSettingGetInt("sqa_fec", &flag, 0);
    sprintf(value, "%d", flag);
    return 0;
}

static int JseSqaFecWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("sqa_fec", atoi(value));
    return 0;
}

/*************************************************
Description: 初始化海博SQA配置定义的接口，由JsePlay.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseSQAInit()
{
    JseCall* call;

    //all C10/C20 regist
    call = new JseFunctionCall("yx_para_fccswitch", JseFccSwitchRead, JseFccSwitchWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_fcctime", JseFccTimeRead, JseFccTimeWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_sqa_fcc", JseSqaFccRead, JseSqaFccWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_sqa_ret", JseSqaRetRead, JseSqaRetWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_sqa_fec", JseSqaFecRead, JseSqaFecWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

