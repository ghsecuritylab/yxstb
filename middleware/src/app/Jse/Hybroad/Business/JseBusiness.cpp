
#include "JseBusiness.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"
#include "Logo/JseLogo.h"

#include "SysSetting.h"

#include "NativeHandlerCustomer.h"
#include "MessageTypes.h"
#include "ipanel_event.h"
#include "Business.h"

#include <stdio.h>
#include <string.h>

//add by zhangmin for qos auth begin flag 2010-8-29
int g_jse_auth_status = 2;  //0, begin 1,end 2  other
static int g_authCount = 0;

static int JseEDSRead(const char* param, char* value, int len)
{
    sysSettingGetString("eds", value, len, 0);
    return 0;
}

static int JseEDSWrite(const char* param, char* value, int len)
{
    if (!value || (strncmp(value, "http://", 7) != 0)) {
        LogJseError("JseEDSWrite value is error\n");
        return -1;
    }
    sysSettingSetString("eds", value);
    return 0;
}

static int JseEDS1Read(const char* param, char* value, int len)
{
    sysSettingGetString("eds1", value, len, 0);
    return 0;
}

static int JseEDS1Write(const char* param, char* value, int len)
{
    if (!value || (strncmp(value, "http://", 7) != 0)) {
        LogJseError("JseEDS1Write value is error\n");
        return -1;
    }
    sysSettingSetString("eds1", value);
    return 0;
}

static int JseEdsUrlWrite(const char* param, char* value, int len)
{
    // C20的同学们是不是也考虑一下走takin.cpp里面的eds轮循调度？

    // char* pr = strstr(value, ".jsp");         /*it's not a path name. so we must delete the unused filename.*/
    // if(pr) {
    //    while(*pr-- != '/');
    //    pr[1] = 0;
    // }
    // app_edsUrl_set(value);
    // app_epgUrl_set(value);
    return 0;
}

static int JseJoinFlagRead(const char* param, char* value, int len)
{
    sprintf(value, "%d", business().getEDSJoinFlag());
    return 0;
}

#ifdef HUAWEI_C10
static int JseEncryTokenWrite(const char* param, char* value, int len)
{
    business().setEncryToken(value);
    return 0;
}

static int JseAuthenticatorCTCEncryRead(const char* param, char* value, int len)
{
    char lEncryptOut[512] = { 0 };

    g_authCount++;
    g_jse_auth_status = 0;
    /* cipher = business().getCTCAuthInfo(business().getEncryToken(), tmp, 512); */
    if(!business().getCTCAuthInfo(lEncryptOut, 512))
        return -2;
    strncpy(value, lEncryptOut, strlen(lEncryptOut));
    return 0;
}

static int JseAuthenticatorCUEncryRead(const char* param, char* value, int len)
{
    char lEncryptOut[512] = { 0 };

    g_authCount++;
    g_jse_auth_status = 0;
    if(!business().getCUAuthInfo(lEncryptOut, 512))
        return -2;
    strncpy(value, lEncryptOut, strlen(lEncryptOut));
    return 0;
}

static int JseCTCLoginWrite(const char* param, char* value, int len)
{
    NativeHandlerInputMessage(MessageType_System, EIS_IRKEY_URL_MENU, 0, NULL);
    return 0;
}
#endif

int jseAuthCountGet()
{
    return g_authCount;
}

/*************************************************
Description: 初始化海博业务配置定义的接口,由JseHybroad.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseBusinessInit()
{
    JseCall* call;

    call = new JseFunctionCall("yx_para_eds", JseEDSRead, JseEDSWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_eds1", JseEDS1Read, JseEDS1Write);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("edsurl", 0, JseEdsUrlWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("joinFlag", JseJoinFlagRead, 0);
    JseRootRegist(call->name(), call);

#ifdef HUAWEI_C10
    //C10 regist, called by Hippo_ContextHWC10.cpp
    call = new JseFunctionCall("EncryToken", 0, JseEncryTokenWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("AuthenticatorCTC_Encry", JseAuthenticatorCTCEncryRead, 0);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("AuthenticatorCU_Encry", JseAuthenticatorCUEncryRead, 0);
    JseRootRegist(call->name(), call);
    //C10 regist, called by IAuthenticationCell.cpp
    call = new JseFunctionCall("CTCLogin", 0, JseCTCLoginWrite);
    JseRootRegist(call->name(), call);
#endif

    JseLogoInit();
    return 0;
}
