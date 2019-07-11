#include "Tr069Time.h"

#include "Tr069GroupCall.h"
#include "Tr069FunctionCall.h"
#include "SysSetting.h"
#include "TR069Assertions.h"
#include "ntp/mid_ntp.h"
#include "mid/mid_time.h"
#include "mid/mid_tools.h"

#include "ind_tmr.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/*------------------------------------------------------------------------------
	第一个 NTP 时间服务器。可以为域名或 IP 地址。当发生改变时，需立即上报终端管理
	系统。
 ------------------------------------------------------------------------------*/
static int tr069_NTPServer1_Read(char* value, unsigned int length)
{
    sysSettingGetString("ntp", value, length, 0);
    return 0;
}

static int tr069_NTPServer1_Write(char* value, unsigned int length)
{
    if (!value || !strlen(value))
        return -1;

    LogTr069Debug("Set address [%s], and then start sync\n",value);
    sysSettingSetString("ntp", value);
    mid_ntp_time_sync( );

    return 0;
}

/*------------------------------------------------------------------------------
	第二个 NTP 时间服务器。可以为域名或 IP 地址。当发生改变时，需立即上报终端管理
	系统。
 ------------------------------------------------------------------------------*/
static int tr069_NTPServer2_Read(char* value, unsigned int length)
{
    sysSettingGetString("ntp1", value, length, 0);
    return 0;
}

static int tr069_NTPServer2_Write(char* value, unsigned int length)
{
    if (!value || !strlen(value))
        return -1;

    sysSettingSetString("ntp1", value);
    mid_ntp_time_sync( );

    return 0;
}

/*------------------------------------------------------------------------------
	机顶盒本地时区中的当前日期和时间
 ------------------------------------------------------------------------------*/
static int tr069_CurrentLocalTime_Read(char* value, unsigned int length)
{
    unsigned int sec;
    int TimeZone;
    struct ind_time t;

    sec = mid_time();
    TimeZone = 0;
    sysSettingGetInt("timezone", &TimeZone, 0);
    sec += TimeZone * 3600;

    memset(&t, 0, sizeof(t));
    ind_time_local (sec, &t);
    snprintf(value, length, "%04d-%02d-%02dT%02d:%02d:%02d", t.year, t.mon, t.day, t.hour, t.min, t.sec);

	return 0;
}

static int tr069_PersistentData_Write(char* value, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	本地时间与 UTC 的偏差，形式如下："+hh:mm" "－hh:mm"。当发生改变时，需立即上报
	终端管理系统。
 ------------------------------------------------------------------------------*/
static int tr069_LocalTimeZone_Read(char* value, unsigned int length)
{
    int TimeZone = 0;
    sysSettingGetInt("timezone", &TimeZone, 0);
    mid_tool_timezone2str(TimeZone,value);

#if 0
    if(TimeZone < 0) {
        TimeZone = -TimeZone;
        sprintf(value, "-%02d:00", TimeZone);
    } else {
        sprintf(value, "+%02d:00", TimeZone);
    }
#endif

    return 0;
}

//TMS     +08:00/GMT +8:00
static int tr069_LocalTimeZone_Write(char* value, unsigned int length)
{
    int zone;
    char *pos1 = NULL;
    char *pos2 = NULL;
    char *pos3 = NULL;
    char tmp[8] = {0};

    pos1 = strchr(value, '+');
    pos2 = strchr(value, '-');
    pos3 = strchr(value, ':');
    if(pos1 == NULL&& pos2 == NULL)
         return -1;
    if(pos1 != NULL){
        if(pos3 != NULL)
            strncpy(tmp,pos1 + 1,(pos3 - pos1));
        else
            strncpy(tmp,pos1 + 1,8);
        zone = atoi(tmp);
    }
    if(pos2 != NULL){
        if(pos3 != NULL)
            strncpy(tmp,pos2 + 1,(pos3 - pos2));
        else
            strncpy(tmp,pos2 + 1,8);
        zone = - atoi(tmp);
	}
    sysSettingSetInt("timezone", zone);
    return 0;
}

Tr069Time::Tr069Time()
	: Tr069GroupCall("Time")
{

    Tr069Call* ntpsrv0 = new Tr069FunctionCall("NTPServer",      tr069_NTPServer1_Read,          tr069_NTPServer1_Write);
    Tr069Call* ntpsrv1 = new Tr069FunctionCall("NTPServer1",     tr069_NTPServer1_Read,          tr069_NTPServer1_Write);
    Tr069Call* ntpsrv2  = new Tr069FunctionCall("NTPServer2",    tr069_NTPServer2_Read,          tr069_NTPServer2_Write);
    Tr069Call* local = new Tr069FunctionCall("CurrentLocalTime", tr069_CurrentLocalTime_Read,    tr069_PersistentData_Write);
    Tr069Call* zone = new Tr069FunctionCall("LocalTimeZone",     tr069_LocalTimeZone_Read,       tr069_LocalTimeZone_Write);

    regist(ntpsrv0->name(), ntpsrv0);
    regist(ntpsrv1->name(), ntpsrv1);
    regist(ntpsrv2->name(), ntpsrv2);
    regist(local->name(), local);
    regist(zone->name(), zone);
}

Tr069Time::~Tr069Time()
{
}
