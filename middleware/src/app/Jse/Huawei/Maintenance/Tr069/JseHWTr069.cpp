
#include "JseHWTr069.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"
#include "Tr069.h"
#include "sys_basic_macro.h"
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)
#include "tr069Itms.h"
#include "ind_mem.h"
#include "mid/mid_time.h"
#endif
#include "app_sys.h"
#include "Session.h"

#include "SysSetting.h"
#include "AppSetting.h"
#include "IPTVMiddleware.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char g_app_ManagementDomainBackup[256];

/************************************************************
* TMSUsername   终端网管用户    支持读取、写入
*************************************************************/
//TODO
static int JseTMSUsernameRead( const char* param, char* value, int len )
{
    return 0;
}

//TODO
static int JseTMSUsernameWrite( const char* param, char* value, int len )
{
#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    sysSettingSetString("tr069_reg_username", value);
#else
    IPTVMiddleware_SettingSetStr("tr069_reg_username", value);
#endif
#endif
    TR069_API_SETVALUE("Device.ManagementServer.Username", value, 0);

    return 0;
}

/************************************************************
* TMSPassword   终端网管密码    支持读取
*************************************************************/
//TODO
static int JseTMSPasswordRead( const char* param, char* value, int len )
{
    return 0;
}
//TODO
static int JseTMSPasswordWrite( const char* param, char* value, int len )
{
#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
        sysSettingSetString("tr069_reg_password", value);
#else
    IPTVMiddleware_SettingSetStr("tr069_reg_password", value);
#endif
#endif
    TR069_API_SETVALUE("Device.ManagementServer.Password", value, 0);

    return 0;
}

static int JseTMSEnableRead( const char* param, char* value, int len )
{
#ifdef VIETTEL_HD
    sprintf(value, "%d", 0);
    return 0;
#endif
    int tr069Enable = 0;
    sysSettingGetInt("tr069_enable", &tr069Enable, 0);
    snprintf(value, len,  "%d", tr069Enable);

    return 0;
}

static int JseTMSEnableWrite( const char* param, char* value, int len )
{
    sysSettingSetInt("tr069_enable", atoi(value));
    settingManagerSave();

#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    sysSettingSetString("tr069_enable", value);
#else
    IPTVMiddleware_SettingSetStr("tr069_enable", value);
#endif
#endif

    return 0;
}

static char g_app_ManagementDomain[256] = {0};

int JseManagementDomainSet(const char *buf)
{
    if (NULL == buf) {
        LogJseError("The ManagementDomain is NULL!\n");
        return -1;
    }
    if (PLATFORM_ZTE == SessionGetPlatform())
		return 0;
#if defined(Sichuan)
    return 0;
#endif
    if (strcasecmp(buf, "null") == 0 || strlen(buf) == 0) {
        g_app_ManagementDomain[0] = '\0';
    }else if (strcmp(g_app_ManagementDomain, buf)) {
        strcpy(g_app_ManagementDomain, buf);
    }
    LogJseDebug("g_app_ManagementDomain =%s\n", g_app_ManagementDomain);

    if (strlen(g_app_ManagementDomain) != 0)
        tr069_api_setValue("Device.ManagementServer.URL", g_app_ManagementDomain, 128);
    tr069_api_setValue((char*)"Task.Active", NULL, 1);

    return 0;
}

static int JseManagementDomainRead( const char* param, char* value, int len )
{
    snprintf(value, len, "%s", g_app_ManagementDomain);

    return 0;
}

static int JseManagementDomainWrite( const char* param, char* value, int len )
{
#ifdef ANDROID
    if (NULL == value || strcasecmp(value, "null") == 0 || strlen(value) == 0) {
        LogJseError("The ManagementDomain is NULL or illegal!\n");
        return -1;
    }
#ifdef NEW_ANDROID_SETTING
    sysSettingSetString("tr069_url", value);
#else
    IPTVMiddleware_SettingSetStr("tr069_url", value);
#endif
    TR069_API_SETVALUE("Device.ManagementServer.URL", value, 0);
#else
   JseManagementDomainSet(value);
#endif
    return 0;
}

static int JseManagementDomainBackupRead( const char* param, char* value, int len )
{
    snprintf(value, len, "%s", g_app_ManagementDomainBackup);

    return 0;
}

static int JseManagementDomainBackupWrite( const char* param, char* value, int len )
{
    if (NULL == value) {
        LogJseError("The ManagementDomainBackup is NULL!\n");
        return -1;
    }

    if (strcasecmp(value, "null") == 0 || strlen(value) == 0) {
        g_app_ManagementDomainBackup[0] = '\0';
    } else if (strcmp(g_app_ManagementDomainBackup, value)) {
        strcpy(g_app_ManagementDomainBackup, value);
    }

    int modifyFlag = 0;
    char modifyBuf[32];
    modifyBuf[0] = 0;
    TR069_API_GETVALUE("Device.ManagementServer.URLModifyFlag", modifyBuf, 32);
    modifyFlag = atoi(modifyBuf);
    if (modifyFlag & 0x01)
        TR069_API_SETVALUE("Device.ManagementServer.URLBackup", value, 0);

    return 0;
}

//TODO
static int JseTMSHeartBitRead( const char* param, char* value, int len )
{
    return 0;
}

//TODO
static int JseTMSHeartBitWrite( const char* param, char* value, int len )
{
#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    sysSettingSetString("tr069_heartbeat_enable", value);
#else
    IPTVMiddleware_SettingSetStr("tr069_heartbeat_enable", value);
#endif
#endif
    TR069_API_SETVALUE("Device.ManagementServer.PeriodicInformEnable", value, 0);
    return 0;
}

//TODO
static int JseTMSHeartBitIntervalRead( const char* param, char* value, int len )
{
    return 0;
}
//TODO
static int JseTMSHeartBitIntervalWrite( const char* param, char* value, int len )
{
#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    sysSettingSetString("tr069_heartbeat_cycle", value);
#else
    IPTVMiddleware_SettingSetStr("tr069_heartbeat_cycle", value);
#endif
#endif
    TR069_API_SETVALUE("Device.ManagementServer.PeriodicInformInterval", value, 0);
    return 0;
}

#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)

void test_lost_rate(void)
{
    int pkglostrate = 3;
    char lostrate_temp[256] = {0};
    time_t sec;
    struct tm t;

    sec = mid_time();
    gmtime_r(&sec, &t);

    sprintf(lostrate_temp, "%02d-%02d-%02d:%02d-%02d-%02d",
            (t.tm_year + 1900) % 100,
            t.tm_mon + 1,
            t.tm_mday,
            t.tm_hour,
            t.tm_min,
            t.tm_sec
           );
    tr069_log_post(lostrate_temp, LOG_MSG_PKGLOSTRATE, "0.0005");
    tr069_log_post(lostrate_temp, LOG_MSG_HTTPINFO, "http://124.75.29.178:7001/iptv3a/UserAuthenticationAction.do\
        http://124.75.29.172:7001/images/booting_1.jpg?loginName=18000030&ip=13.167.5.34\
        http://124.75.29.178:7001/iptv3a/ValidAuthenticatorAction.do\
        http://124.75.29.172:7001/images/booting_2.jpg?loginName=18000030&ip=13.167.5.34\
        http://124.75.33.7:8080/iptvepg/function/index.jsp\
        http://10.192.72.91:8080/iptvepg/function/index.jsp?loadbalanced=1&UserIP=13.167.5.34&UserID=18000030&UserToken=18000030***224e9942543168369327b&STBID=&LastTermno=147&emg=0&DynamicAuthIP=124.75.29.178\
        http://10.192.72.91:8080/iptvepg/function/funcportalauth.jsp\
        http://10.192.72.91:8080/iptvepg/function/frameset_judger.jsp\
        http://10.192.72.91:8080/iptvepg/function/frameset_builder.jsp\
        http://10.192.72.91:8080/iptvepg/frame16/bestv/bestv-config/pos/Position_202_20110914_SMG_Dispatch/images/20120702002429135.gif");
    tr069_log_post(lostrate_temp, LOG_MSG_RTSPINFO, "RTSPFailInfo");
    tr069_log_post(lostrate_temp, LOG_MSG_BUFFER, "1316008");
    tr069_log_post(lostrate_temp, LOG_MSG_ERROR, "0000");
    tr069_log_post(lostrate_temp, LOG_MSG_VENDOREXT, "VenderCode=666666\
        VenderName=HuaWei;HWVersion=5202.0\
        ProductSN=00000000000000001\
        ProductBatch=201010\
        ProductBatch=20070925\
        MAC=00:0D:0C:82:05:02\
        DefaultPwd=810501\
        LoaderType=1");
    tr069_log_post(lostrate_temp, LOG_MSG_AVARAGERATE, "0.002");
}

void test_lost_second_rate(void)
{
    int         pkglostrate = 3;
    char        lostrate_temp[256] = {0};
    time_t sec;
    struct tm t;
    IND_MEMSET(lostrate_temp, 0, sizeof(lostrate_temp));
    sec = mid_time();
    gmtime_r(&sec, &t);

    sprintf(lostrate_temp, "%02d-%02d-%02d:%02d-%02d-%02d",
            (t.tm_year + 1900) % 100,
            t.tm_mon + 1,
            t.tm_mday,
            t.tm_hour,
            t.tm_min,
            t.tm_sec
           );
    tr069_log_post(lostrate_temp, LOG_MSG_PKGTOTALONESEC, "0.001");
    tr069_log_post(lostrate_temp, LOG_MSG_BYTETOTALONESEC, "1");
}

static int JseTr069ActiveWrite( const char* param, char* value, int len )
{
    itms_start();
    mid_timer_create(1, 0, test_lost_rate, 0);
    mid_timer_create(1, 0, test_lost_second_rate, 0);

    return 0;
}
#endif

static int JseUrlWrite( const char* param, char* value, int len )
{
    tr069_statistic_set_HTTPReqNumbers();

    return 0;
}

static int JseLogSendFlagRead( const char* param, char* value, int len )
{
    int flag = 0;
    sysSettingGetInt("logSend", &flag, 0);
    sprintf(value, "%d", flag);

    return 0;
}

static int JseLogSendFlagWrite( const char* param, char* value, int len )
{
    sysSettingSetInt("logSend", atoi(value));

    return 0;
}

#if defined (HUAWEI_C10)
static int JseTmsHeartBeatTriggeredWrite( const char* param, char* value, int len )
{
    return 0;
}

static int JseTmsregWrite( const char* param, char* value, int len )
{
    sysSettingSetInt("tr069_enable", atoi(value));
    settingManagerSave();

    return 0;
}

#endif

static int JseLogSaveWrite( const char* param, char* value, int len )
{
    tr069StatisticConfigSave();

    return 0;
}

static int JseTMSServerRead( const char* param, char* value, int len )
{
    TR069_API_GETVALUE("Device.ManagementServer.URL", value, len);

    return 0;
}

static int JseTMSServerWrite( const char* param, char* value, int len )
{
    if(value) {
        if(strlen(value)) {
            TR069_API_SETVALUE("Param.URL", value, NULL);
        }
    }
    return 0;
}
JseHWTr069::JseHWTr069()
    : JseGroupCall("tms")
{
    JseCall* call;
#if defined (HUAWEI_C10)
    //C10
    call = new JseFunctionCall("heartBeat.triggered", 0, JseTmsHeartBeatTriggeredWrite);
    regist(call->name(), call);
#endif
}

JseHWTr069::~JseHWTr069()
{
}

int
JseHWTr069::call(const char* name, const char* param, char* value, int length, int set)
{
    std::map<std::string, JseCall*>::iterator it = m_callMap.find(name);
    if ( it != m_callMap.end()) {
        return (it->second)->call(name, param, value, length, set);
    } else {
        return JseGroupCall::call(name, param, value, length, set);
    }
}

/*************************************************
Description: 初始化华为Tr069的接口，由JseHWMaintenance.cpp调用
Input: 无
Return: 无
*************************************************/
int JseHWTr069Init()
{

    JseCall* call;
    //C10 C20
    call = new JseFunctionCall("TMSUsername", JseTMSUsernameRead, JseTMSUsernameWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("TMSPassword", JseTMSPasswordRead, JseTMSPasswordWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("TMSEnable", JseTMSEnableRead, JseTMSEnableWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("ManagementDomain", JseManagementDomainRead, JseManagementDomainWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("ManagementDomainBackup", JseManagementDomainBackupRead, JseManagementDomainBackupWrite);
    JseRootRegist(call->name(), call);

    //C20
    call = new JseFunctionCall("TMSHeartBit", JseTMSHeartBitRead, JseTMSHeartBitWrite);
    JseRootRegist(call->name(), call);

    //C20
    call = new JseFunctionCall("TMSHeartBitInterval", JseTMSHeartBitIntervalRead, JseTMSHeartBitIntervalWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("url", 0, JseUrlWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("IsPostLog", JseLogSendFlagRead, JseLogSendFlagWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("QOSLogEnables", JseLogSendFlagRead, JseLogSendFlagWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("LogSendFlag", JseLogSendFlagRead, JseLogSendFlagWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("LogSave", 0, JseLogSaveWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("TMSServer", JseTMSServerRead, JseTMSServerWrite);
    JseRootRegist(call->name(), call);

#if defined (HUAWEI_C10)
    //C10
    call = new JseFunctionCall("tmsreg", 0, JseTmsregWrite);
    JseRootRegist(call->name(), call);

#endif
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)
    //C10 C20
    call = new JseFunctionCall("tr069active", 0, JseTr069ActiveWrite);
    JseRootRegist(call->name(), call);
#endif // #if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)

    call = new JseHWTr069();
    JseRootRegist(call->name(), call);

    return 0;
}
