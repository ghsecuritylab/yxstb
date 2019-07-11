
#include "JseHWTime.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"
#include "JseRoot.h"

#include "mid/mid_tools.h"
#include "mid/mid_time.h"
#include "ntp/mid_ntp.h"
#include "mid/mid_tools.h"

#include "SysSetting.h"
#include "AppSetting.h"

#include <string.h>
#include <ctype.h>

#ifndef NEW_ANDROID_SETTING
extern "C" int IPTVMiddleware_SettingSetStr(const char* name, const char* value);
#endif

//C10/C20系统时间配置
static int JseSavingTimeWrite(const char *param, char *value, int len)
{
    char* pStr = 0;
    char  tBuf[16] = { 0 };

    pStr = value;
    if (!pStr[0]) {
        LogJseError("SavingTime is null\n");
        return -1;
    }
    pStr = strstr(pStr, "TimeZone");
    if (!pStr) {
        LogJseError("TimeZone is null\n");
        return -1;
    }
    pStr += strlen("TimeZone");
    pStr += strspn(pStr, " =\t\"");
    if (strncmp(pStr, "GMT", 3) && strncmp(pStr, "UTC", 3)) {
        LogJseError("TimeZone not contain GMT or UTC\n");
        return -1;
    }
    pStr += strlen("GMT");
    while(*pStr && isblank(*pStr))
        pStr++;
    if ('+' != *pStr && '-' != *pStr) {
        LogJseError("Time Format error\n");
        return -1;
    }
    strncpy(tBuf, pStr, 6);
    if (strlen(tBuf) < 6 && !strchr(tBuf, ':')) {
        LogJseError("Time Format error[%s], correct format +/-HH:MM", tBuf);
        return -1;
    }
    int tZone = mid_tool_str2timezone(tBuf);
    pStr += 6; // len: +HH:MM

    pStr = strstr(pStr, "Flag");
    if (!pStr) {
        LogJseError("Flag is null\n");
        return -1;
    }
    pStr += strlen("Flag");
    pStr += strspn(pStr, " =\t\"");
    if ('1' == *pStr && 1 == MID_UTC_SUPPORT) {
        pStr = strstr(pStr, "startTime");
        if (!pStr) {
            LogJseError("startTime is null\n");
            return -1;
        }
        pStr += strlen("startTime");
        pStr += strspn(pStr, " =\t\"");
        bzero(tBuf, sizeof(tBuf));
        strncpy(tBuf, pStr, 14);
        if (strlen(tBuf) < 14 && !strchr(tBuf, ',')) {
            LogJseError("startTime is invalid\n");
            return -1;
        }
        unsigned int sTime = mid_tool_string2time(tBuf);
        pStr = strstr(pStr, "endTime");
        if (!pStr) {
            LogJseError("endTime is null\n");
            return -1;
        }
        pStr += strlen("endTime");
        pStr += strspn(pStr, " =\t\"");
        bzero(tBuf, sizeof(tBuf));
        strncpy(tBuf, pStr, 14);
        if (strlen(tBuf) < 14 && !strchr(tBuf, ',')) {
            LogJseError("endTime is invalid\n");
            return -1;
        }
        unsigned int eTime = mid_tool_string2time(tBuf);
        unsigned int jStep = 1 * 60 * 60;
        pStr = strstr(pStr, "jumpStep");
        if (!pStr)
            LogJseWarning("jumpStep is null\n");
        pStr += strlen("jumpStep");
        pStr += strspn(pStr, " =\t\"");
        bzero(tBuf, sizeof(tBuf));
        strncpy(tBuf, pStr, 6);
        if (strlen(tBuf) < 6)
            LogJseWarning("jumpStep is invalid\n");
        else {
            int hour = 0, min = 0, sec = 0;
            if(3 == sscanf(tBuf, "%02d%02d%02d", &hour, &min, &sec))
                jStep = 3600 * hour + 60 * min + sec;
        }
		appSettingSetInt("saveflag", 1);
		appSettingSetInt("lightstart", sTime);
		appSettingSetInt("lightstop", eTime);
		appSettingSetInt("jumpstep", jStep);
        set_saving_time_sec();
    }
    sysSettingSetInt("timezone", tZone);
    set_local_time_zone();
    settingManagerSave();
    //mid_ntp_time_sync();
    return 0;
}

//C10/C20系统时间配置
static int JseNTPDomainRead(const char *param, char *value, int len)
{
    sysSettingGetString("ntp", value, len, 0);
    return 0;
}

//C10系统时间配置
static int JseNTPDomainWrite(const char *param, char *value, int len)
{
    if ((!value) || (!strlen(value)) || (!strcmp(value, "null")) || (!strcmp(value, "NULL")))
        return 0;
    char inflash_ntp[128] = { 0 };
    sysSettingGetString("ntp", inflash_ntp, 128, 0);
	if (strcmp(inflash_ntp , value)){
	    sysSettingSetString("ntp", value);
		mid_ntp_time_sync();
	}
#if defined(ANDROID)
#ifndef NEW_ANDROID_SETTING
	IPTVMiddleware_SettingSetStr("ntp", value);
#endif
#endif
    return 0;
}

//C10系统时间配置
static int JseNTPDomainBackupRead(const char *param, char *value, int len)
{
    sysSettingGetString("ntp1", value, len, 0);
    return 0;
}

//C10系统时间配置
static int JseNTPDomainBackupWrite(const char *param, char *value, int len)
{
    if ((!value) || (!strlen(value)) || (!strcmp(value, "null")) || (!strcmp(value, "NULL"))){
        return 0;
    } else {
        char inflash_ntp[128] = {0};
        sysSettingGetString("ntp1", inflash_ntp, 128, 0);
	    if(strcmp(inflash_ntp , value)){
	        sysSettingSetString("ntp1",value);
		    mid_ntp_time_sync();
	    }
	}
#if defined(ANDROID)
#ifndef NEW_ANDROID_SETTING
	IPTVMiddleware_SettingSetStr("ntp1", value);
#endif
#endif
    return 0;
}

//C20系统时间配置
static int JseUTCSupportRead(const char *param, char *value, int len)
{
    /* weather support utc time */
    if (MID_UTC_SUPPORT == 1) {
        LogJseDebug("Support UTC time.\n");
        strcpy(value, "1");
    } else {
        LogJseDebug("Do not support UTC time.\n");
        strcpy(value, "0");
    }
    return 0;
}

//C20系统时间配置
static int JseTimeIsDSTRead(const char *param, char *value, int len)
{
    unsigned int startTime = 0;
    unsigned int stopTime = 0;
    unsigned int tmpTime = 0;
    char *pstring = NULL;

    //"time":"20120419163000","timeType":"UTC"
    if (!param) {
        LogJseError("SavingTime is NULL!\n");
        goto Err;
    }

    pstring = (char *)strstr(param , "\"time\":\""); //WZW modified to fix pc-lint Error 158
    if (!pstring) {
        LogJseError("TimeZone is invalid!\n");
        goto Err;
    }

    pstring += strlen("\"time\":\"");
    tmpTime = mid_tool_string2time(pstring);

    appSettingGetInt("lightstart", (int *)&startTime, 0);
	appSettingGetInt("lightstop", (int *)&stopTime, 0);

    if ((tmpTime >= startTime) && (tmpTime <= stopTime)) {
        LogJseDebug("%d time is in saving time.\n", tmpTime);
        strcpy(value, "1");
    } else {
        LogJseDebug("%d time is not in saving time.\n", tmpTime);
        strcpy(value, "0");
    }

    return 0;
Err:
    LogJseDebug("%s\n", param);
    return -1;

}

static int JseTimeZoneRead(const char *param, char *value, int len)
{
	int timeZone = 0;

    sysSettingGetInt("timezone", &timeZone, 0);
#if defined(Cameroon_v5)
	mid_tool_timezone2str(timeZone, &value[0]);
#else
    sprintf(value, "UTC ");
    mid_tool_timezone2str(timeZone, &value[4]);
#endif
    LogJseDebug("Read timezone : %s\n", value);
    return 0;
}

static int JseTimeZoneWrite(const char *param, char *value, int len)
{
    LogJseDebug("value:%s\n", value);
    if (strstr(value, "UTC") || strstr(value, "GMT")) {
        char* pStr = strchr(value, '+');
        if (!pStr)
            pStr = strchr(value, '-');
        // Skip the blank character
        while(*pStr && isblank(*pStr))
            pStr++;
        if (*pStr)
            sysSettingSetInt("timezone", mid_tool_str2timezone(pStr));
    }
    return 0;
}

//TODO
static int JseOlsonTimeZoneRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseOlsonTimeZoneWrite(const char *param, char *value, int len)
{
    return 0;
}

/*************************************************
Description: 初始化华为系统配置里的时间定义的接口，由JseHWSTB.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWTimeInit()
{
    JseCall* call;

    //C10/C20 regist
    call = new JseFunctionCall("SavingTime", 0, JseSavingTimeWrite);
    JseRootRegist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("NTPDomain", JseNTPDomainRead, JseNTPDomainWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("NTPDomainBackup", JseNTPDomainBackupRead, JseNTPDomainBackupWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("UTCSupport", JseUTCSupportRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("timeIsDST", JseTimeIsDSTRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("timeZone", JseTimeZoneRead, JseTimeZoneWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("olsonTimeZone", JseOlsonTimeZoneRead, JseOlsonTimeZoneWrite);
    JseRootRegist(call->name(), call);
    return 0;
}
