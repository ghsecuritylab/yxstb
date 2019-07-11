
#include "JseSafetyLine.h"
#include "JseFunctionCall.h"
#include "JseRoot.h"

#include "mgmtModule.h"
#include "MonitorConfig.h"

#include "TerminalControl.h"
#include "UserInformation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



static int JseYxParaSSHStateRead( const char* param, char* value, int len )
{
    sprintf(value, "%d", getSSHState());

    return 0;
}

static int JseYxParaSSHStateWrite( const char* param, char* value, int len )
{
    setSSHState(atoi(value));

    return 0;
}

#ifndef ANDROID
static int JseYxParaSSHCheckRead( const char* param, char* value, int len )
{
    if (param[0] == '\0') {
        strcpy(value, "0");
        return 0;
    }

    char temp[1024] = {0};

    snprintf(temp, 1024, "%s", param);
    char *p1 = strchr(temp, ':');

    if (!p1) {
        if (checkUserAccountInfo(NULL, temp))
            strcpy(value, "1");
        else
            strcpy(value, "0");
        return 0;
    }

    *p1 = '\0';
    p1++;
    char *pswd = NULL;
    if (strcmp(p1, "%RESET%") != 0) {
        pswd = p1;
    }

    if (checkUserAccountInfo(temp, pswd))
        strcpy(value, "1");
    else
        strcpy(value, "0");

    return 0;
}

static int JseYxParaSSHUsernameRead( const char* param, char* value, int len )
{
    getUserAccountInfo(value, len, NULL, 0);

    return 0;
}

static int JseYxParaSSHInfoWrite( const char* param, char* value, int len )
{
    char temp[4096] = {0};

    snprintf(temp, sizeof(temp), "%s", value);
    char *p = strchr(temp, ':');
    if (!p)
        return 0;

    *p = '\0';
    p++;
    saveUserAccountInfo(temp, p);

    return 0;
}

static int JseDefaultSSHPasswdWrite( const char* param, char* value, int len )
{
    char sshName[1024] = {0};
    if (!value)
        return -1;

    getUserAccountInfo(sshName, 1024, NULL, 0);
    saveUserAccountInfo(sshName, value);

    return 0;
}

static int JseYxParaSSHResetWrite( const char* param, char* value, int len )
{
    saveUserAccountInfo(DEFAULT_USER, DEFAULT_PASSWD);

    return 0;
}

#endif // ifndef ANDROID

static int JseYxParaMonitorStateRead( const char* param, char* value, int len )
{
    #ifdef ANDROID
    // android 在APK实现
    #else
    sprintf(value, "%d", getMonitorState());
    #endif
    return 0;
}

static int JseYxParaMonitorStateWrite( const char* param, char* value, int len )
{
    #ifdef ANDROID
    #else
    setMonitorState(atoi(value));
    #endif
    return 0;
}

static int JseYxParaMonitorCheckRead( const char* param, char* value, int len )
{
    if (param[0] == '\0') {
        strcpy(value, "0");
        return 0;
    }

    char temp[1024];

    snprintf(temp, 1024, "%s", param);
    char *p1 = strchr(temp, ':');

    if (!p1) {
        if (checkMonitorInfo(NULL, temp))
            strcpy(value, "1");
        else
            strcpy(value, "0");
        return 0;
    }

    *p1 = '\0';
    p1++;
    char *pswd = NULL;
    if (strcmp(p1, "%RESET%") != 0) {
        pswd = p1;
    }

    if (checkMonitorInfo(temp, pswd))
        strcpy(value, "1");
    else
        strcpy(value, "0");

    return 0;
}

static int JseYxParaMonitorUsernameRead( const char* param, char* value, int len )
{
    getMonitorInfo(value, len, NULL, 0);

    return 0;
}

static int JseYxParaMonitorPasswdRead( const char* param, char* value, int len )
{
    getMonitorInfo(NULL, 0, value, len);

    return 0;
}

static int JseYxParaMonitorWrite( const char* param, char* value, int len )
{
    char temp[4096] = {0};

    snprintf(temp, sizeof(temp), "%s", value);
    char * p = strchr(temp, ':');
    if (!p)
        return 0;

    *p = '\0';
    p++;
    saveMonitorInfo(temp, p);

    return 0;
}

static int JseDefaultSTBMonitorUserPasswdWrite( const char* param, char* value, int len )
{
    char MonitorName[1024] = {0};
    if (!value)
        return -1;

    getMonitorInfo(MonitorName, 1024, NULL, 0);
    saveMonitorInfo(MonitorName, value);

    return 0;
}

static int JseYxParaMonitorResetWrite( const char* param, char* value, int len )
{
    resetMonitorInfo();

    return 0;
}

int JseSafetyLineInit()
{
    JseCall* call;


    call = new JseFunctionCall("yx_para_ssh_state", JseYxParaSSHStateRead, JseYxParaSSHStateWrite);
    JseRootRegist(call->name(), call);

#ifndef ANDROID
    call = new JseFunctionCall("defaultSSHPasswd", 0, JseDefaultSSHPasswdWrite);
    JseRootRegist(call->name(), call);

    // 删除原来的telnet接口，替换为ssh，网页字段暂时未对应改变统一
    call = new JseFunctionCall("yx_para_telnet_check", JseYxParaSSHCheckRead, 0);
    JseRootRegist(call->name(), call);

    // 删除原来的telnet接口，替换为ssh，网页字段暂时未对应改变统一
    call = new JseFunctionCall("yx_para_telnet_username", JseYxParaSSHUsernameRead, 0);
    JseRootRegist(call->name(), call);

    // 删除原来的telnet接口，替换为ssh，网页字段暂时未对应改变统一
    call = new JseFunctionCall("yx_para_telnet_set", 0, JseYxParaSSHInfoWrite);
    JseRootRegist(call->name(), call);

    // 删除原来的telnet接口，替换为ssh，网页字段暂时未对应改变统一
    call = new JseFunctionCall("yx_para_telnet_reset", 0, JseYxParaSSHResetWrite);
    JseRootRegist(call->name(), call);
#endif // ifndef ANDROID

    call = new JseFunctionCall("yx_para_monitor_state", JseYxParaMonitorStateRead, JseYxParaMonitorStateWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_monitor_check", JseYxParaMonitorCheckRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_monitor_username", JseYxParaMonitorUsernameRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_monitor_passwd", JseYxParaMonitorPasswdRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_monitor_set", 0, JseYxParaMonitorWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("defaultSTBMonitorUserPasswd", 0, JseDefaultSTBMonitorUserPasswdWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_monitor_reset", 0, JseYxParaMonitorResetWrite);
    JseRootRegist(call->name(), call);

    return 0;
}

