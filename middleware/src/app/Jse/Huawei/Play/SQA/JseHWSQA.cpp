
#include "JseHWSQA.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "Buf/JseHWSQABuf.h"
#include "Ret/JseHWSQARet.h"
#include "Tools/JseHWSQATools.h"

#include "SysSetting.h"

#include <string.h>
#include <stdlib.h>

extern int RRS_EPG_FLAG;
extern char FCC_IP[64 + 4];

/*************************************************
*function : EPG Jse call to set fcc server
**************************************************/
static int JseFCCServerWrite(const char *param, char *value, int len)
{
    if (!value || strlen(value) > 68) {
        LogJseError("error fcc server!\n");
        return -1;
    }

    memset(FCC_IP, 0, 64 + 4);
    strcpy(FCC_IP, value);
    RRS_EPG_FLAG = 1;
    LogJseDebug("SAVE FCCServer:%s,FCC_IP:%s, RRS_EPG_FLAG:%d\n", value, FCC_IP, RRS_EPG_FLAG);
    return 0;
}

static int JseFCCPortWrite(const char *param, char *value, int len)
{
    sysSettingSetInt("fcc_port", atoi(value));
    settingManagerSave();
    return 0;
}

static int JseDisorderTimeWrite(const char *param, char *value, int len)
{
    sysSettingSetInt("DisorderTime", atoi(value));
    return 0;
}

static int JseRSRTimeOutWrite(const char *param, char *value, int len)
{
    sysSettingSetInt("RSRTimeOut", atoi(value));
    return 0;
}

static int JseSCNTimeOutWrite(const char *param, char *value, int len)
{
    sysSettingSetInt("SCNTimeOut", atoi(value));
    return 0;
}

static int JseChanCacheTimeWrite(const char *param, char *value, int len)
{
    sysSettingSetInt("ChanCacheTime", atoi(value));
    return 0;
}

//TODO
static int JseSQAParamWrite(const char *param, char *value, int len)
{
    LogJseError("JseSQAParamWrite is not supported now!\n");
    return 0;
}

//TODO
static int JseServiceQualityParaWrite(const char *param, char *value, int len)
{
    LogJseError("JseServiceQualityParaWrite is not supported now!\n");
    return 0;
}

JseHWSQA::JseHWSQA()
    : JseGroupCall("SQA")
{
    JseCall* call;

    call = new JseHWSQABuf();
    regist(call->name(), call);

    call = new JseHWSQARet();
    regist(call->name(), call);

    call = new JseHWSQATools();
    regist(call->name(), call);
}

JseHWSQA::~JseHWSQA()
{
}

/*************************************************
Description: 初始化华为播放流控中SQA模块配置定义的接口，由JseHWPlays.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWSQAInit()
{
    JseCall* call;

    //C10/C20 regist
    call = new JseFunctionCall("DisorderTime", 0, JseDisorderTimeWrite);
    JseRootRegist(call->name(), call);
#ifdef HUAWEI_C10
    //C10 regist
    call = new JseFunctionCall("FCCServer", 0, JseFCCServerWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("SQARtcpPort", 0, JseFCCPortWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseHWSQA();
    JseRootRegist(call->name(), call);
#else
    //C20 regist
    call = new JseFunctionCall("FCCPort", 0, JseFCCPortWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("RSRTimeOut", 0, JseRSRTimeOutWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("SCNTimeOut", 0, JseSCNTimeOutWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("ChanCacheTime", 0, JseChanCacheTimeWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("SQAParam", 0, JseSQAParamWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("ServiceQualityPara", 0, JseServiceQualityParaWrite);
    JseRootRegist(call->name(), call);

    JseHWSQABufInit();
    JseHWSQARetInit();
#endif
    return 0;
}

