
#include "JseTime.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "mid/mid_time.h"
#include "mid/mid_tools.h"

#include "SysSetting.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int JseTimeZoneRead(const char* param, char* value, int len)
{
    int timeZone = 0;

    sysSettingGetInt("timezone", &timeZone, 0);
    sprintf(value, "%d", timeZone);
    return 0;
}

static int JseTimeZoneWrite(const char* param, char* value, int len)
{
    int time_zone = 0;

    sysSettingSetInt("timezone", atoi(value));
    sysSettingGetInt("timezone", &time_zone, 0);
    mid_set_timezone(mid_tool_timezone2sec(time_zone));
    return 0;
}

static int JseSysTimeRead(const char* param, char* value, int len)
{
    char tTime[20] = {0}, temp[8] = {0};
    int year = 0, mon = 0, day = 0, date = 0;
    char wday_name[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    char mon_name[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    if(value == NULL) {
        return -1;
    }
    mid_tool_time2string(mid_time(), tTime, ':');
    strncpy(temp, tTime, 4);
    temp[4] = '\0';
    year = atoi(temp);
    strncpy(temp, tTime + 5, 2);
    temp[2] = '\0';
    mon = atoi(temp) - 1;
    strncpy(temp, tTime + 8, 2);
    temp[2] = '\0';
    day = atoi(temp);
    date = (mid_time() / 3600 / 24 + 4) % 7;
    snprintf(value, len, "%s %s %02d %s %d", wday_name[date], mon_name[mon], day, tTime + 11, year);
    return 0;
}

/*************************************************
Description: 初始化海博系统时间配置定义的接口,由JseSTB.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseTimeInit()
{
    JseCall* call;

    call = new JseFunctionCall("yx_para_timezone", JseTimeZoneRead, JseTimeZoneWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("systime", JseSysTimeRead, 0);
    JseRootRegist(call->name(), call);
    return 0;
}
