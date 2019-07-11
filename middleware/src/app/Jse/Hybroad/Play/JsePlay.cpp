
#include "JsePlay.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#ifdef INCLUDE_SQA
#include "SQA/JseSQA.h"
#endif

#ifdef INCLUDE_SQM
#include "SQM/JseSQM.h"
#endif

#include "SysSetting.h"
#include "AppSetting.h"

#include "SystemManager.h"

#include <stdio.h>
#include <stdlib.h>

static int pltvActive = 0;
static int JsePltvFlagRead(const char* param, char* value, int len)
{
    sprintf(value, "%d", pltvActive);
    return 0;
}

static int JsePltvFlagWrite(const char* param, char* value, int len)
{
    pltvActive = atoi(value);
    return 0;
}

static int JseTeletextRead(const char* param, char* value, int len)
{
    int teletext = 0;
    appSettingGetInt("Teletext", &teletext, 0);
    sprintf(value, "%d", teletext);
    return 0;
}

static int JseTeletextWrite(const char* param, char* value, int len)
{
    appSettingSetInt("Teletext", atoi(value));
    return 0;
}

static int JseDestoryAllPlayerWrite(const char* param, char* value, int len)
{
    Hippo::SystemManager &sysManager = Hippo::systemManager();

    sysManager.destoryAllPlayer();
    return 0;
}

/*************************************************
Description: 初始化海博Play配置定义的接口，由JseHybroad.cpp调用
Input: 无
Return: 无
 *************************************************/
int JsePlayInit()
{
    JseCall* call;

    call = new JseFunctionCall("pltvActive", JsePltvFlagRead, JsePltvFlagWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_teletext", JseTeletextRead, JseTeletextWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("destoryAllPlayer", 0, JseDestoryAllPlayerWrite);
    JseRootRegist(call->name(), call);

#ifdef INCLUDE_SQA
    JseSQAInit();
#endif

#ifdef INCLUDE_SQM
    JseSQMInit();
#endif
    return 0;
}

