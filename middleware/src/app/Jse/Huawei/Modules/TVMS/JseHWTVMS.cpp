
#include "JseHWTVMS.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "tvms.h"
#include "tvms_define.h"
#include "tvms_setting.h"
#include "SysSetting.h"

#include <stdio.h>
#include <stdlib.h>
#include "IPTVMiddleware.h"

static int gTVMSHeartFlag = 0;
static int gTVMSVODHeartFlag = 0;

static int JseTVMSGWIPRead(const char *param, char *value, int len)
{
    tvms_conf_tvmsgwip_get(value);
    return 0;
}

static int JseTVMSGWIPWrite(const char *param, char *value, int len)
{
    tvms_conf_tvmsgwip_set(value);
    return 0;
}

static int JseTVMSHeartbitIntervalRead(const char *param, char *value, int len)
{
    int tTime = 0;

    tvms_conf_tvmsheartbitinterval_get(&tTime);
    snprintf(value, len, "%d", tTime);
    return 0;
}

static int JseTVMSHeartbitIntervalWrite(const char *param, char *value, int len)
{
    tvms_conf_tvmsheartbitinterval_set(atoi(value));
#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    sysSettingSetString("tvms_heartbeat", value);
#else
    IPTVMiddleware_SettingSetStr("tvms_heartbeat", value);
#endif
#endif
    return 0;
}

static int JseTVMSDelayLengthRead(const char *param, char *value, int len)
{
    int tTime = 0;

    tvms_conf_tvmsdelaylength_get(&tTime);
    snprintf(value, len, "%d", tTime);
    return 0;
}

static int JseTVMSDelayLengthWrite(const char *param, char *value, int len)
{
    tvms_conf_tvmsdelaylength_set(atoi(value));
#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    sysSettingSetString("tvms_delay", value);
#else
    IPTVMiddleware_SettingSetStr("tvms_delay", value);
#endif
#endif
    return 0;
}

static int JseTVMSHeartbitUrlRead(const char *param, char *value, int len)
{
    tvms_conf_tvmsheartbiturl_get(value);
    return 0;
}

static int JseTVMSHeartbitUrlWrite(const char *param, char *value, int len)
{
    tvms_conf_tvmsheartbiturl_set(value);
    gTVMSHeartFlag = 1;
    if (gTVMSHeartFlag == 1 && gTVMSVODHeartFlag == 1)
        init_tvms_msg();
#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    sysSettingSetString("tvms_url", value);
#else
    IPTVMiddleware_SettingSetStr("tvms_url", value);
#endif
#endif
    return 0;
}

static int JseTVMSVODHeartbitUrlRead(const char *param, char *value, int len)
{
    tvms_conf_tvmsvodheartbiturl_get(value);
    return 0;
}

static int JseTVMSVODHeartbitUrlWrite(const char *param, char *value, int len)
{
    tvms_conf_tvmsvodheartbiturl_set(value);
    gTVMSVODHeartFlag = 1;
    if (gTVMSHeartFlag == 1 && gTVMSVODHeartFlag == 1)
        init_tvms_msg();
#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    sysSettingSetString("tvms_vod_url", value);
#else
    IPTVMiddleware_SettingSetStr("tvms_vod_url", value);
#endif
#endif
    return 0;
}


static int JseMediacodeRead(const char *param, char *value, int len)
{
    tvms_mediacode_read(value);
    return 0;
}

static int JseMediacodeWrite(const char *param, char *value, int len)
{
    tvms_mediacode_write(value);
    return 0;
}

static int JseSendVendorSpecificCommandWrite(const char *param, char *value, int len)
{
    tvms_show_status(value);
    return 0;
}

/*************************************************
Description: 初始化华为TVMS模块配置定义的接口，由JseModules.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWTVMSInit()
{
    JseCall* call;
#ifdef HUAWEI_C10
    call = new JseFunctionCall("TVMSGWIP", JseTVMSGWIPRead, JseTVMSGWIPWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("TVMSHeartbitInterval", JseTVMSHeartbitIntervalRead, JseTVMSHeartbitIntervalWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("TVMSDelayLength", JseTVMSDelayLengthRead, JseTVMSDelayLengthWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("TVMSHeartbitUrl", JseTVMSHeartbitUrlRead, JseTVMSHeartbitUrlWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("TVMSVODHeartbitUrl", JseTVMSVODHeartbitUrlRead, JseTVMSVODHeartbitUrlWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("mediacode", JseMediacodeRead, JseMediacodeWrite);
    JseRootRegist(call->name(), call);
#else
    call = new JseFunctionCall("sendVendorSpecificCommand", 0, JseSendVendorSpecificCommandWrite);
    JseRootRegist(call->name(), call);
#endif
    return 0;
}

