
#include "JseHWLog.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
//#include "JseAssertions.h"

#include "LogModuleHuawei.h"
#include "Tr069.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef ANDROID
#include "mgmt/mgmtModule.h"
#endif


static int JseLogServerUrlRead(const char *param, char *value, int len)
{
    TR069_STATISTIC_GET_CONFIGURATION((char*)"LogServerUrl", value, len);
    return 0;
}

static int JseLogServerUrlWrite(const char *param, char *value, int len)
{
    TR069_STATISTIC_SET_CONFIGURATION((char*)"LogServerUrl", value, len);
    return 0;
}

static int JseLogUploadIntervalRead(const char *param, char *value, int len)
{
    TR069_STATISTIC_GET_CONFIGURATION("LogUploadInterval",value, len);
    return 0;
}

static int JseLogUploadIntervalWrite(const char *param, char *value, int len)
{
    TR069_STATISTIC_SET_CONFIGURATION("LogUploadInterval",value, len);
    return 0;
}

static int JseLogRecordIntervalRead(const char *param, char *value, int len)
{
    TR069_STATISTIC_GET_CONFIGURATION("LogRecordInterval",value, len);
    return 0;
}

static int JseLogRecordIntervalWrite(const char *param, char *value, int len)
{
    TR069_STATISTIC_SET_CONFIGURATION("LogRecordInterval",value, len);
    return 0;
}

static int JseLogTypeRead(const char *param, char *value, int len)
{
    snprintf(value, len, "%d", huaweiGetLogType());
    return 0;
}

static int JseLogTypeWrite(const char *param, char *value, int len)
{
    huaweiSetLogType(atoi(value));
    //stb_syslog_set_logtype(atoi(value), 1);
#ifdef ANDROID
    iptvSendMsgToMonitor("SetLogType", value, len);
#endif
    huaweiLog();
    return 0;
}

static int JseLogLevelRead(const char *param, char *value, int len)
{
    snprintf(value, len, "%d", huaweiGetLogLevel());
    return 0;
}

static int JseLogLevelWrite(const char *param, char *value, int len)
{
    huaweiSetLogLevel(atoi(value));
    //stb_syslog_set_loglevel(atoi(value), 1);
#ifdef ANDROID
    iptvSendMsgToMonitor("SetLogLevel", value, len);
#endif

    huaweiLog();
    return 0;
}


static int JseLogOutputTypeRead(const char *param, char *value, int len)
{
    snprintf(value, len, "%d", huaweiGetLogOutPutType());
    return 0;
}


static int JseLogOutputTypeWrite(const char *param, char *value, int len)
{
    huaweiSetLogOutPutType(atoi(value));
    //stb_syslog_set_logoutputtype(atoi(value), 1);
#ifdef ANDROID
    iptvSendMsgToMonitor("SetLogOutType", value, len);
#endif
    huaweiLog();
    return 0;
}

static int JseLogFtpServerRead(const char *param, char *value, int len)
{
    huaweiGetLogFTPServer(value, 255);
    return 0;
}

static int JseLogFtpServerWrite(const char *param, char *value, int len)
{
    huaweiSetLogFTPServer(value);
    //stb_syslog_set_logftpservere(value);
#ifdef ANDROID
    iptvSendMsgToMonitor("SetLogFtpServer", value, len);
#endif
    huaweiLog();
    return 0;
}

static int JseLogServerRead(const char *param, char *value, int len)
{
    huaweiGetLogUDPServer(value, 255);
    return 0;
}

static int JseLogServerWrite(const char *param, char *value, int len)
{
    huaweiSetLogUDPServer(value);
    //stb_syslog_set_logserver(value, 1);
#ifdef ANDROID
    iptvSendMsgToMonitor("SetLogServer", value, len);
#endif
    huaweiLog();
    return 0;
}

/*************************************************
Description: 初始化华为维护日志的接口，由JseHWMaintenance.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWLogInit()
{
    JseCall* call;

    //C20,C10 regist  JseRead_LogServerUrl  JseWrite_LogServerUrl
    call = new JseFunctionCall("LogServerUrl", JseLogServerUrlRead, JseLogServerUrlWrite);
    JseRootRegist(call->name(), call);

    //C20,C10 regist  JseRead_LogUploadInterval  JseWrite_LogUploadInterval
    call = new JseFunctionCall("LogUploadInterval", JseLogUploadIntervalRead, JseLogUploadIntervalWrite);
    JseRootRegist(call->name(), call);

    //C10 regist  JseRead_LogRecordInterval  JseWrite_LogRecordInterval
    call = new JseFunctionCall("LogRecordInterval", JseLogRecordIntervalRead, JseLogRecordIntervalWrite);
    JseRootRegist(call->name(), call);


    //C20 regist  JseRead_LogType  JseWrite_LogType
    call = new JseFunctionCall("LogType", JseLogTypeRead, JseLogTypeWrite);
    JseRootRegist(call->name(), call);

    //C20 regist  JseRead_LogLevel  JseWrite_LogLevel
    call = new JseFunctionCall("LogLevel", JseLogLevelRead, JseLogLevelWrite);
    JseRootRegist(call->name(), call);

    //C20 regist  文档中是LogoutPutType
    call = new JseFunctionCall("LogOutPutType", JseLogOutputTypeRead, JseLogOutputTypeWrite);
    JseRootRegist(call->name(), call);

    //C20 regist  JseRead_LogFtpServer  JseWrite_LogFtpServer
    call = new JseFunctionCall("LogFtpServer", JseLogFtpServerRead, JseLogFtpServerWrite);
    JseRootRegist(call->name(), call);

    //C20 regist  JseRead_LogServer  JseWrite_LogServer
    call = new JseFunctionCall("LogServer", JseLogServerRead, JseLogServerWrite);
    JseRootRegist(call->name(), call);

    return 0;
}

