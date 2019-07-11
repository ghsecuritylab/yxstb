
#include "JseHWSession.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "app_heartbit.h"
#include "app_epg_para.h"
#include "cryptoFunc.h"
#include "charConvert.h"
#include "app_sys.h"
#include "mid/mid_timer.h"
#include "mid/mid_time.h"
#include "sys_basic_macro.h"
#include "ind_mem.h"
#include "IPTVMiddleware.h"
#include "SysSetting.h"
#include "mid/mid_mutex.h"
#include "Session.h"
#include "Account.h"

#include "AppSetting.h"
#include "Tr069.h"

#include "app_jse.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "customer.h"
#include "IPTVMiddleware.h"

static char SessionID[32] = {0};
static char g_app_usertoken[256] = {0};
static char	g_app_EPGGroupNMB[EPG_SERVICEENTRY_MAX_NUM] = {0};
static mid_mutex_t g_app_mutex = NULL;
static int AuthFailNumbers = 0;
static char AuthFailInfo[10][256] = {{0}, {0}};
extern int g_jse_auth_status; //define in JseBusiness.cpp

static int stringtime(char* buf);

static int JseSessionIDRead(const char *param, char *value, int len)
{
    if(SessionID[0] != '\0')
        strcpy(value, SessionID);
    else
        value[0] = '\0';
    return 0;
}

static int JseSessionIDWrite(const char *param, char *value, int len)
{
    strcpy(SessionID, value);
    session().setCookie(value);

#if defined(VIETTEL_HD) || defined(Cameroon_v5)
    KeyTableUrlGet();
    mid_timer_create(1, 1, httpHeartBit, 0);
#endif
    if(strncmp(session().getPlatformCode(), "0101", 4)) {
        LogJseDebug("begin httpHeartBit\n");
        /*huawei c20 start heartbit when the hybird mode is set*/
#if defined(HUAWEI_C10)
        mid_timer_create(3, 1, httpHeartBit, 0);
#endif
    }
    return 0;
}

/*************************************************
Description:获取用户令牌
Input:     无
Return:   指向用户令牌地址
 *************************************************/
static int JseUserTokenRead(const char *param, char *value, int len)
{
    strcpy(value, g_app_usertoken);
    return 0;
}

/*************************************************
Description:设置用户令牌
Input:       value:指向用户令牌地址
Return:   无
 *************************************************/
static int JseUserTokenWrite(const char *param, char *value, int len)
{
    strcpy(g_app_usertoken, value);
#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    sysSettingSetString("user_token", value);
#else
    IPTVMiddleware_SettingSetStr("user_token", value);
#endif
#endif
    return 0;
}

static int JseUserStatusRead(const char *param, char *value, int len)
{
    snprintf(value, len, "%d", session().getUserStatus());
    return 0;
}

static int JsUserStatuseWrite(const char *param, char *value, int len)
{
    int status = atoi(value);

#ifdef HUAWEI_C10
    if(g_jse_auth_status == 0) {
        if(status == 0) {
            TR069_REPORT_AUTHORIZE_ALARM(0);
            jseAuthFailInfoSet(Hippo::Customer().AuthInfo().AvailableEpgAuthUrl().c_str(), 0);
        }
    }
#else
    if(status == 0) {
        TR069_REPORT_AUTHORIZE_ALARM(0);
        jseAuthFailInfoSet(Hippo::Customer().AuthInfo().AvailableEpgAuthUrl().c_str(), 0);
    } else {
#ifdef STBTYPE_QTEL
        tr069_statistic_set_userstatus_time();
#endif
    }
#endif
    g_jse_auth_status = 1;
    session().setUserStatus(status);

#if defined(ANDROID)
    if(status == 1) {
        appSettingSetInt("epg_auth_flag", 1);
        settingManagerSave();
    #ifdef EC6106V8_TEST
        IPTVMiddlewareAgent().authSucessedToAndroid();
    #else
        IPTVMiddleware_PostEvent(200/*EVENT_PAGE*/, 202/*PAGE_AUTH_FINISHED*/, 0, 0);
    #endif
    } else {
    #ifdef EC6106V8_TEST
        IPTVMiddlewareAgent().authFailedToAndroid();
    #else
        IPTVMiddleware_PostEvent(200/*EVENT_PAGE*/, 201/*PAGE_AUTH_FILED*/, 0, 0);
    #endif
    }
#else
    if(status == 1) {
        appSettingSetInt("epg_auth_flag", 1);
        settingManagerSave();
    }
#endif
    return 0;
}

static int JseEPGGroupNMBRead(const char *param, char *value, int len)
{
    sprintf(value, "%s", g_app_EPGGroupNMB);
    return 0;
}

static int JseEPGGroupNMBWrite(const char *param, char *value, int len)
{
    if(NULL == value)
        ERR_OUT("The EPGGroupNMB is NULL!\n");
    if(strcmp(g_app_EPGGroupNMB, value))
        strcpy(g_app_EPGGroupNMB, value);
    return 0;
Err:
    return -1;
}

static int JseUserGroupNMBRead(const char *param, char *value, int len)
{
    sprintf(value, "%s", session().getUserGroupNMB());
    return 0;
}

/****************************************************************
 用户对应的用户分组信息标譿String 128)，此参数值保存至STB Flash
 ***************************************************************/
static int JseUserGroupNMBWrite(const char *param, char *value, int len)
{
    session().setUserGroupNMB(value);
    return 0;
}

static int JseareaidRead(const char *param, char *value, int len)
{
    appSettingGetString("areaid", value, len, 0);
    return 0;
}

/******************************************************************
 用户所在区域的ID(String 128)，写入STB Flash，用户每次登录认证成功后写入STB，STB立即写Flash
 *****************************************************************/
static int JseareaidWrite(const char *param, char *value, int len)
{
    appSettingSetString("areaid", value);
    return 0;
}

static int JsetemplateNameRead(const char *param, char *value, int len)
{
	appSettingGetString("templateName", value, len, 0);
    return 0;
}

/***********************************************************************
模版名称(String 64)，写入STB Flash，用户每次登录认证成功后写入STB，STB立即写Flash
***********************************************************************/
static int JsetemplateNameWrite(const char *param, char *value, int len)
{
    appSettingSetString("templateName", value);
    return 0;
}

static int JsePackageIDsRead(const char *param, char *value, int len)
{
    appSettingGetString("epg_PackageIDs", value, len, 0);
    return 0;
}

static int JsePackageIDsWrite(const char *param, char *value, int len)
{
    appSettingSetString("epg_PackageIDs", value);
    return 0;
}

static int JsePlatformCodeRead(const char *param, char *value, int len)
{
    strcpy(value, session().getPlatformCode());
    return 0;
}

static int JsePlatformCodeWrite(const char *param, char *value, int len)
{
    session().setPlatformCode(value);
    session().setPlatform(PLATFORM_HW);
    return 0;
}

static int JseidentityEncodeRead(const char *param, char *value, int len)
{
    char *s1 = NULL;

    s1 = (char *)IND_MALLOC(URL_MAX_LEN + 4);
    if(!s1)
        return 0;
    IND_MEMSET(s1, 0, URL_MAX_LEN + 4);

    sprintf(s1, "%s%s", SessionID, account().getShareKey());
    LogJseDebug("midSessionIDGet(%s), getShareKey(%s)\n", SessionID, account().getShareKey());
    if (strlen(s1)) {
#ifndef ENCRYPT_IDENTITY_VIA_LIB
        md5Encypt(&s1, 1, value, len, 0);
        data2Hex(value, 16, value, len);
        lower2Upper(value, strlen(value));
#else
        strcpy(value, termianlEntity());
#endif
    } else
        value[0] = '\0';
    IND_FREE(s1);
    return 0;
}

static int JsecnonceRead(const char *param, char *value, int len)
{
    strcpy(value, (const char *)session().getCnonce());
    return 0;
}

//TODO
static int JseEPGHeartbeatSchemeVersionRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseEPGHeartbeatSchemeVersionWrite(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseSubnetIdRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseSubnetIdWrite(const char *param, char *value, int len)
{
    return 0;
}

extern "C" int jseAuthFailCountGet()
{
    return AuthFailNumbers;
}

extern "C" void jseAuthFailInfoGet(char(*buf)[256], int size)
{
    if (!buf || size != 10 * 256)
        return;
    memcpy(buf, AuthFailInfo, size);
}

extern "C" void jseAuthFailInfoSet(const char *url, int err_no)
{
    int tIndex = 0, len = 0;
    char *buf = NULL;

    if(url) {
        tIndex = AuthFailNumbers % 10; //STATISTIC_INFO_FAID_NUM = 10
        buf = AuthFailInfo[tIndex];

        len = stringtime(buf);
        snprintf(buf + len, 256 - 1 - len, ",%s,%d", url, err_no);
        buf[256 - 1] = 0;

        AuthFailNumbers++;
    }
    return;
}

static int stringtime(char* buf)
{
    time_t sec;
    struct tm t;

    if(buf == NULL)
        ERR_OUT("buf is null\n");

    sec = mid_time();
    gmtime_r(&sec, &t);

    sprintf(buf, "%02d%02d%02d%02d%02d%02d",
            (t.tm_year + 1900) % 100,
            t.tm_mon + 1,
            t.tm_mday,
            t.tm_hour,
            t.tm_min,
            t.tm_sec);
    buf[14] = '\0';

    return 12;
Err:
    return 0;
}

/*************************************************
Description: 初始化华为业务会话定义的接口，由JseHWBusiness.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWSessionInit()
{
    JseCall* call;

    //C10/C20 regist
    call = new JseFunctionCall("SessionID", JseSessionIDRead, JseSessionIDWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("sessionid", JseSessionIDRead, JseSessionIDWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("UserToken", JseUserTokenRead, JseUserTokenWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("usertoken", JseUserTokenRead, JseUserTokenWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("userstatus", JseUserStatusRead, JsUserStatuseWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("UserStatus", JseUserStatusRead, JsUserStatuseWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("UserGroupNMB", JseUserGroupNMBRead, JseUserGroupNMBWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("areaid", JseareaidRead, JseareaidWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("templateName", JsetemplateNameRead, JsetemplateNameWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("EPGGroupNMB", JseEPGGroupNMBRead, JseEPGGroupNMBWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("PackageIDs", JsePackageIDsRead, JsePackageIDsWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("PlatformCode", JsePlatformCodeRead, JsePlatformCodeWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("identityEncode", JseidentityEncodeRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("cnonce", JsecnonceRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("EPGHeartbeatSchemeVersion", JseEPGHeartbeatSchemeVersionRead, JseEPGHeartbeatSchemeVersionWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("subnetId", JseSubnetIdRead, JseSubnetIdWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

