
#ifndef Sichuan

#include "tr069_port_alarm.h"
#include "tr069_port_alarmHuawei.h"

#include "tmerTask.h"

#include "tr069_api.h"
#include "tr069_port.h"
#include "tr069_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static int gAlarmSN = 1;

static void alarmTime2str(char *str, int size, unsigned int sec)
{
    time_t t;
    struct tm tm;

    memset(&tm, 0, sizeof(tm));
    t = sec;
    localtime_r(&t, &tm);
    snprintf(str, size, "%04d%02d%02d%02d%02d%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

int alarmBodyGet(AlarmBody *alarm, char *name, char *str, unsigned int size)
{
    if (!strcmp("AlarmSN", name)) {
        snprintf(str, size, "%d", alarm->AlarmSN);
    } else if (!strcmp("AlarmObjectInstance", name)) {
        snprintf(str, size, "%s", alarm->AlarmObjectInstance);
    } else if (!strcmp("AlarmLocation", name)) {
        snprintf(str, size, "%s", alarm->AlarmLocation);
    } else if (!strcmp("AlarmCode", name)) {
        snprintf(str, size, "%d", alarm->AlarmCode);
    } else if (!strcmp("AlarmRaisedTime", name)) {
        if (alarm->AlarmRaisedTime)
            alarmTime2str(str, size, alarm->AlarmRaisedTime);
    } else if (!strcmp("AlarmClearedTime", name)) {
        if (alarm->AlarmClearedTime)
            alarmTime2str(str, size, alarm->AlarmClearedTime);
    } else if (!strcmp("PerceivedSeverity", name)) {
        snprintf(str, size, "%d", alarm->PerceivedSeverity);
    } else if (!strcmp("AdditionalInformation", name)) {
        snprintf(str, size, "%s", alarm->AdditionalInformation);
    } else {
        TR069Error("name = %s\n", name);
        return -1;
    }

    return 0;
}

void alarmBodyClear(AlarmBody *alarm)
{
    alarm->PerceivedSeverity = ALARM_LEVEL_CLEARED;
    alarm->AlarmClearedTime = time(NULL);
}

void alarmBodyFree(AlarmBody *alarm)
{
    if (alarm->AlarmObjectInstance) {
        free(alarm->AlarmObjectInstance);
        alarm->AlarmObjectInstance = NULL;
    }
    if (alarm->AlarmLocation) {
        free(alarm->AlarmLocation);
        alarm->AlarmLocation = NULL;
    }
    if (alarm->AdditionalInformation) {
        free(alarm->AdditionalInformation);
        alarm->AdditionalInformation = NULL;
    }
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void alarmBodyPost(AlarmBody *alarm, int type, int code, int level, char *location)
{
    char buffer[64];//strlen(Device.DeviceInfo.X_CTC_IPTV_Alarm.AdditionalInformation) = 56

    alarm->AlarmSN = time(NULL);
    if (alarm->AlarmSN < gAlarmSN)
        alarm->AlarmSN = gAlarmSN;
    else
        gAlarmSN = alarm->AlarmSN;
    gAlarmSN ++;

    buffer[0] = 0;
    tr069_port_getValue("Device.DeviceInfo.SerialNumber", buffer, 64);
    buffer[32] = 0;
    alarmStrdup(&alarm->AlarmObjectInstance, buffer);

    alarmStrdup(&alarm->AlarmLocation, location);
    alarm->AlarmCode = code;

    alarm->AlarmRaisedTime = time(NULL);
    alarm->AlarmClearedTime = 0;

    alarm->PerceivedSeverity = level;
    alarmStrdup(&alarm->AdditionalInformation, alarm->AlarmLocation);
}

#endif//Sichuan
