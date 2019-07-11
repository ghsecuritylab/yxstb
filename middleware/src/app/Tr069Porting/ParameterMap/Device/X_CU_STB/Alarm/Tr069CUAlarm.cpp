#include "Tr069CUAlarm.h"

#include "Tr069FunctionCall.h"
#include "TR069Assertions.h"
#include "Tr069.h"
#include <string.h>
#include <stdlib.h>

static unsigned int gEnabledAlarm = 0;//off
static unsigned int gMonitoringInterval = 30;//s
static unsigned int gPacketsLostRateAlarm = 10;//%
static unsigned int gPacketsLost = 0;//%
static unsigned int gFramesLostAlarm = 300;
static unsigned int gFramesLost = 0;
static char gAlarmReason[32] = "0";


static void tr069_cu_alarmStart()
{
#ifdef INCLUDE_TR069_CU
    if(gEnabledAlarm) {
        strcpy(gAlarmReason, "1");
        LogTr069Debug("TR069_EXTERN_EVENT_ALARM\n");

        tr069_api_setValue((char*)"Event.Parameter", "Device.X_CU_STB.STBInfo.STBID",       TR069_EXTERN_EVENT_ALARM);
        tr069_api_setValue((char*)"Event.Parameter", "Device.X_CU_STB.Alarm.AlarmReason",   TR069_EXTERN_EVENT_ALARM);
        tr069_api_setValue((char*)"Event.Parameter", "Device.X_CU_STB.Alarm.PacketsLost",   TR069_EXTERN_EVENT_ALARM);
        tr069_api_setValue((char*)"Event.Parameter", "Device.X_CU_STB.Alarm.FramesLost",    TR069_EXTERN_EVENT_ALARM);

        tr069_api_setValue("Event.Post", "", TR069_EXTERN_EVENT_ALARM);
    }
#endif
}

static int tr069_EnabledAlarm_Read(char* value, unsigned int length)
{
    LogTr069Debug("gEnabledAlarm = %d\n", gEnabledAlarm);
    sprintf(value, "%u", gEnabledAlarm);
    return 0;
}

static int tr069_EnabledAlarm_Write(char* value, unsigned int length)
{
    unsigned int enable;

    if(gEnabledAlarm != 0 && gEnabledAlarm != 1)
        return -1;

    enable = atoi(value);
    LogTr069Debug("enable = %d\n", enable);
    if(enable != gEnabledAlarm)
        gEnabledAlarm = enable;

    return 0;
}

static int tr069_MonitoringInterval_Read(char* value, unsigned int length)
{
    LogTr069Debug("gMonitoringInterval = %d\n", gMonitoringInterval);
    sprintf(value, "%u", gMonitoringInterval);
    return 0;
}

static int tr069_MonitoringInterval_Write(char* value, unsigned int length)
{
    unsigned int interval;
    if(value < 0)
    	return -1;

    interval = atoi(value);
    LogTr069Debug("value = %d\n", interval);
    if(interval != gMonitoringInterval) {
        gMonitoringInterval = interval;
        tr069_statistic_set_MonitoringInterval(interval);
    }
    return 0;
}

static int tr069_PacketsLostRateAlarm_Read(char* value, unsigned int length)
{
    LogTr069Debug("gPacketsLostRateAlarm = %d\n", gPacketsLostRateAlarm);
    sprintf(value, "%u", gPacketsLostRateAlarm);
    return 0;
}

static int tr069_PacketsLostRateAlarm_Write(char* value, unsigned int length)
{
    unsigned int rate;
    if(value < 0)
    	 return -1;

    rate = atoi(value);
    LogTr069Debug("value = %d\n", rate);
    if(rate != gPacketsLostRateAlarm)
        gPacketsLostRateAlarm = rate;
    return 0;
}

static int tr069_PacketsLost_Read(char* value, unsigned int length)
{
    LogTr069Debug("gPacketsLost = %d\n", gPacketsLost);
    sprintf(value , "%u",gPacketsLost);
    return 0;
}

void tr069_cu_setPacketsLost(unsigned int permille)//Õâ¸ö½Ó¿ÚÓ¦¸Ã»á±»Í³¼ÆÀïÃæµ÷ÓÃ£¬±»¸ÄÖ®ºóÃ»ÈËµ÷ÓÃ£¬ÓÐµãÎÊÌâå
{
    static int alarmFlag = 0;
    unsigned int limit;

    limit = gPacketsLostRateAlarm * 10;
    LogTr069Debug("limit = %d, permille = %d, alarmFlag = %d, alarmEnable = %d\n", limit, permille, alarmFlag, gEnabledAlarm);

    if(!gEnabledAlarm)
        return;

    if (permille < limit && alarmFlag) {
        gPacketsLost = permille / 10;
        alarmFlag = 0;
    }
    if (permille >= limit && !alarmFlag) {
        gPacketsLost = permille / 10;
        tr069_cu_alarmStart( );
        alarmFlag = 1;
    }
}

int tr069_FramesLostAlarm_Read(char* value, unsigned int length)
{
    LogTr069Debug("gFramesLostAlarm = %d\n", gFramesLostAlarm);
    sprintf(value , "%u",gFramesLostAlarm);
    return 0;
}

static int tr069_FramesLostAlarm_Write(char* value, unsigned int length)
{
    unsigned int rate;
    if(value < 0)
        return -1;

    rate = atoi(value);
    LogTr069Debug("value = %d\n", rate);
    if(rate != gFramesLostAlarm)
        gFramesLostAlarm = rate;
    return 0;
}

static int tr069_FramesLost_Read(char* value, unsigned int length)
{
    LogTr069Debug("gFramesLost = %d\n", gFramesLost);
    sprintf(value , "%u",gFramesLost);
    return 0;
}

void tr069_cu_setFramesLost(unsigned int framesLost)
{
    static int alarmFlag = 0;

    LogTr069Debug("framesLost = %d / %d, alarmFlag = %d, alarmEnable = %d\n", framesLost, gFramesLostAlarm, alarmFlag, gEnabledAlarm);

    if(!gEnabledAlarm)
        return;

    if (framesLost < gFramesLostAlarm && alarmFlag) {
        gFramesLost = framesLost;
        alarmFlag = 0;
    }
    if (framesLost >= gFramesLostAlarm && !alarmFlag) {
        gFramesLost = framesLost;
        tr069_cu_alarmStart( );
        alarmFlag = 1;
    }
}


static int tr069_AlarmReason_Read(char* value, unsigned int length)
{
    strcpy(value, gAlarmReason);
    LogTr069Debug("value = [%s]\n", value);
    return 0;
}

/*************************************************
Description: ³õÊ¼»¯tr069V1¶¨ÒåµÄ½Ó¿Ú, <Device.Tr069CUAlarm.***>
Input: ÎÞ
Return: ÎÞ
 *************************************************/
Tr069CUAlarm::Tr069CUAlarm()
	: Tr069GroupCall("Alarm")
{
    Tr069Call* enalrm  = new Tr069FunctionCall("EnabledAlarm",          tr069_EnabledAlarm_Read,              tr069_EnabledAlarm_Write);
    Tr069Call* interv  = new Tr069FunctionCall("MonitoringInterval",    tr069_MonitoringInterval_Read,        tr069_MonitoringInterval_Write);
    Tr069Call* pkgrate  = new Tr069FunctionCall("PacketsLostRateAlarm", tr069_PacketsLostRateAlarm_Read,      tr069_PacketsLostRateAlarm_Write);
    Tr069Call* pkglost  = new Tr069FunctionCall("PacketsLost",          tr069_PacketsLost_Read,               NULL);
    Tr069Call* frmalrm  = new Tr069FunctionCall("FramesLostAlarm",      tr069_FramesLostAlarm_Read,           tr069_FramesLostAlarm_Write);
    Tr069Call* frm  = new Tr069FunctionCall("FramesLost",               tr069_FramesLost_Read,                NULL);
    Tr069Call* reason  = new Tr069FunctionCall("AlarmReason",           tr069_AlarmReason_Read,               NULL);

    regist(enalrm->name(), enalrm);
    regist(interv->name(), interv);
    regist(pkgrate->name(), pkgrate);
    regist(pkglost->name(), pkglost);
    regist(frmalrm->name(), frmalrm);
    regist(frm->name(), frm);
    regist(reason->name(), reason);

#ifdef INCLUDE_TR069_CU
    tr069_api_setValue((char*)"Event.Regist", (char*)"X CU Alarm", TR069_EXTERN_EVENT_ALARM);
#endif
}

Tr069CUAlarm::~Tr069CUAlarm()
{
}
