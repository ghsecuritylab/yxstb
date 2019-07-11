
#ifdef Sichuan

#include "tr069_port_alarm.h"
#include "tr069_port_alarmSichuan.h"

#include "tmerTask.h"

#include "tr069_api.h"
#include "tr069_port.h"
#include "tr069_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static int gAlarmID = 1;

static void alarmTime2str(char *str, int size, unsigned int sec)
{
    time_t t;
    struct tm tm;

    memset(&tm, 0, sizeof(tm));
    t = sec;
    localtime_r(&t, &tm);
    snprintf(str, size, "%04d-%02d-%02dT%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

int alarmBodyGet(AlarmBody *alarm, char *name, char *str, unsigned int size)
{
    if (!strcmp("AlarmID", name)) {
        snprintf(str, size, "%u", alarm->AlarmID);
    } else if (!strcmp("AlarmType", name)) {
        snprintf(str, size, "%d", alarm->AlarmType);
    } else if (!strcmp("AlarmLevel", name)) {
        snprintf(str, size, "%d", alarm->AlarmLevel);
    } else if (!strcmp("AlarmReason", name)) {
        snprintf(str, size, "%d", alarm->AlarmReason);
    } else if (!strcmp("AlarmTime", name)) {
        if (alarm->AlarmTime)
            alarmTime2str(str, size, alarm->AlarmTime);
    } else if (!strcmp("AlarmDetail", name)) {
        snprintf(str, size, "%s", alarm->AlarmDetail);
    } else {
        TR069Error("name = %s\n", name);
        return -1;
    }

    return 0;
}

void alarmBodyClear(AlarmBody *alarm)
{
    alarm->AlarmLevel = ALARM_LEVEL_CLEARED;
    alarm->AlarmTime = time(NULL);
}

void alarmBodyFree(AlarmBody *alarm)
{
    if (alarm->AlarmDetail) {
        free(alarm->AlarmDetail);
        alarm->AlarmDetail = NULL;
    }
}

static void alarmClearTimer(int type)
{
    char buffer[64];

    alarmTimerUnRegist(alarmClearTimer, type);

    sprintf(buffer, "AlarmClear.%d", type);
    tr069_api_setValue(buffer, "", 0);
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void alarmBodyPost(AlarmBody *alarm, int type, int code, int level, char *location)
{
    char buffer[64];//strlen(Device.DeviceInfo.X_CTC_IPTV_Alarm.AdditionalInformation) = 56

    alarm->AlarmID = time(NULL);
    if (alarm->AlarmID < gAlarmID)
        alarm->AlarmID = gAlarmID;
    else
        gAlarmID = alarm->AlarmID;
    gAlarmID ++;

    switch(type) {
    case ALARM_TYPE_CPU_Used:
    case ALARM_TYPE_Memory_Used:
    case ALARM_TYPE_Disk_Used:
    case ALARM_TYPE_Dropped:
    case ALARM_TYPE_Dropframe:
    case ALARM_TYPE_Timelapse:
        alarm->AlarmType = ALARM_TYPE_THRESHOULD;
        break;
    case ALARM_TYPE_Cushion:
        alarmTimerRegist(60, alarmClearTimer, type);
        alarm->AlarmType = ALARM_TYPE_THRESHOULD;
        break;
    case ALARM_TYPE_EPG_Access:
    case ALARM_TYPE_Media_Access:
    case ALARM_TYPE_File_Access:
        alarmTimerRegist(60, alarmClearTimer, type);
        alarm->AlarmType = ALARM_TYPE_NETWORK;
        break;
    default:
        break;
    }

    alarm->AlarmLevel = level;
    alarm->AlarmReason = code;
    alarm->AlarmTime = time(NULL);

    alarmStrdup(&alarm->AlarmDetail, location);
}

#endif//Sichuan
