
#include "tr069_port_alarm.h"

#ifdef Sichuan
#include "tr069_port_alarmSichuan.h"
#else
#include "tr069_port_alarmHuawei.h"
#endif

#include "tmerTask.h"
#include "extendConfig.h"

#include "tr069_api.h"
#include "tr069_port.h"
#include "tr069_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct tagAlarmRange AlarmRange;
struct tagAlarmRange {
    int left;
    int right;
};
struct AlarmConfig {
    int AlarmSwitch;
    int AlarmReportLevel;
    AlarmRange AlarmValueCPU;
    AlarmRange AlarmValueMemory;
    AlarmRange AlarmValueDisk;
    AlarmRange AlarmValuePacketsLost;
    AlarmRange BandwidthAlarmValue;
    AlarmRange FramesLostAlarmValue;
    AlarmRange TimeLapseAlarmValue;
    AlarmRange CushionAlarmValue;
};

typedef struct tagAlarm Alarm;

struct tagAlarm {
    Alarm *next;

    int type;
    int isClear;//告警将要被清除
    int instance;

    AlarmBody body;
};

/*
static const char *g_AlarmLevelName[ALARM_LEVEL_MAX][16] = {
    "",
    "Critical",//紧急
    "Major",//重要
    "Minor",//次要
    "Warning",//提示
    "indeterminate",//不确定
    "cleared"//已清除
};
*/

#ifdef INCLUDE_TR069_CU
#define ALARM_OBJECT_NAME   "Device.X_CU_STB.Alarm."
#endif//INCLUDE_TR069_CU
#ifdef INCLUDE_TR069_CTC
#ifdef Sichuan
#define ALARM_OBJECT_NAME   "Device.DeviceInfo.X_CTC_IPTV_Alarm."
#else
#define ALARM_OBJECT_NAME   "Device.X_00E0FC.Alarm."
#endif//Sichuan
#endif//INCLUDE_TR069_CTC
#ifdef INCLUDE_TR069_HUAWEI
#define ALARM_OBJECT_NAME   "Device.X_00E0FC.Alarm."
#endif//INCLUDE_TR069_HUAWEI

static Alarm *gAlarmPush = NULL;//告警产生并未上报
static Alarm *gAlarmPost = NULL;//告警上报中
static Alarm *gAlarmHold = NULL;//告警上报完毕
static Alarm *gClearPush = NULL;//清除产生并未上报
static Alarm *gClearPost = NULL;//清除上报中

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static TimerTask *gTask = NULL;
static Alarm **gAlarmArray = NULL;
static struct AlarmConfig *gAlarmConfig = NULL;

static int gAlarmID = 0;
static int gClearID = 0;

static void alarmPostTimer(int arg);

void alarmStrdup(char **pp, char *str)
{
    char *p = *pp;
    if (p)
        free(p);
    if (str)
        *pp = strdup(str);
    else
        *pp = strdup("");
}

void alarmTimerRegist(unsigned int sec, OnTimer onTimer, int arg)
{
    timerTaskRegist(gTask, sec, onTimer, arg);
}

void alarmTimerUnRegist(OnTimer onTimer, int arg)
{
    timerTaskUnRegist(gTask, onTimer, arg);
}

inline void alarmRangeScan(AlarmRange *range, char *str)
{
    if (strchr(str, ','))
        sscanf(str, "%d,%d", &range->left, &range->right);
    else
        sscanf(str, "%d", &range->left);
}

inline void alarmRangePrint(AlarmRange *range, char *str)
{
    if (range->right)
        sprintf(str, "%d,%d", range->left, range->right);
    else
        sprintf(str, "%d", range->left);
}

static void alarmFree(Alarm *alarm)
{
    if (!alarm)
        return;

    gAlarmArray[alarm->type] = NULL;

    alarmBodyFree(&alarm->body);

    free(alarm);
}

static void alarmReset(void)
{
    Alarm *alarm;

    TR069Debug("\n");

    tr069_port_alarmCPU(0, 0, 0);
    tr069_port_alarmDisk(0, 0, 0);
    tr069_port_alarmMemory(0, 0, 0);

    while (gAlarmPush) {
        alarm = gAlarmPush;
        gAlarmPush = alarm->next;
        alarmFree(alarm);
    }
    while (gAlarmPost) {
        alarm = gAlarmPost;
        gAlarmPost = alarm->next;
        alarmFree(alarm);
    }
    while (gAlarmHold) {
        alarm = gAlarmHold;
        gAlarmHold = alarm->next;
        alarmFree(alarm);
    }
    while (gClearPush) {
        alarm = gClearPush;
        gClearPush = alarm->next;
        alarmFree(alarm);
    }
    while (gClearPost) {
        alarm = gClearPost;
        gClearPost = alarm->next;
        alarmFree(alarm);
    }
}

static void alarmCheckTimer(int arg)
{
    int level;
    AlarmRange range;

    TR069Debug("AlarmSwitch = %d\n", gAlarmConfig->AlarmSwitch);

    if (gAlarmConfig->AlarmSwitch <= 0) {
        timerTaskUnRegist(gTask, alarmCheckTimer, 0);
        pthread_mutex_lock(&g_mutex);
        alarmReset( );
        pthread_mutex_unlock(&g_mutex);
        return;
    }

    level = gAlarmConfig->AlarmReportLevel;

    range = gAlarmConfig->AlarmValueCPU;
    tr069_port_alarmCPU(range.left, range.right, level);
    range = gAlarmConfig->AlarmValueDisk;
    tr069_port_alarmDisk(range.left, range.right, level);
    range = gAlarmConfig->AlarmValueMemory;
    tr069_port_alarmMemory(range.left, range.right, level);
}

static void alarmCheckTimer5(int arg)
{
    TR069Debug("\n");

    timerTaskUnRegist(gTask, alarmCheckTimer5, 0);
    alarmCheckTimer(0);
    timerTaskRegist(gTask, 60, alarmCheckTimer, 0);
}

static int alarmGet(char *name, char *str, unsigned int size)
{
    int instance;
    char *p;
    Alarm *alarm;

    p = strchr(name, '.');
    instance = atoi(name);
    if (!p || instance <= 0) {
        TR069Error("Device.X.Alarm. name = %s\n", name);
        return -1;
    }

    if (gClearPost) {
        alarm = gClearPost;
        while (alarm && instance != alarm->instance)
            alarm = alarm->next;
    } else if (gAlarmPost) {
        alarm = gAlarmPost;
        while (alarm && instance != alarm->instance)
            alarm = alarm->next;
    } else {
        alarm = NULL;
    }
    if (!alarm) {
        TR069Error("instance = %d, Post = (%p, %p)\n", instance, gClearPost, gAlarmPost);
        return -1;
    }
    name = p + 1;

    timerTaskRegist(gTask, 5, alarmPostTimer, 0);

    return alarmBodyGet(&alarm->body, name, str, size);
}

static void alarmGetConfig(char *name, char *str, unsigned int size)
{
    str[0] = 0;

    pthread_mutex_lock(&g_mutex);
    if (!strncmp(name, "Alarm.", 6)) {
        name += 6;
        alarmGet(name, str, size);
    } else if (!strcmp(name, "AlarmSwitch")) {
        snprintf(str, size, "%d", gAlarmConfig->AlarmSwitch);
    } else if (!strcmp(name, "AlarmReportLevel")) {
        snprintf(str, size, "%d", gAlarmConfig->AlarmReportLevel);
    } else if (!strcmp(name, "CPUAlarmValue")) {
        alarmRangePrint(&gAlarmConfig->AlarmValueCPU, str);
    } else if (!strcmp(name, "MemoryAlarmValue")) {
       alarmRangePrint(&gAlarmConfig->AlarmValueMemory, str);
    } else if (!strcmp(name, "DiskAlarmValue")) {
        alarmRangePrint(&gAlarmConfig->AlarmValueDisk, str);
    } else if (!strcmp(name, "PacketsLostAlarmValue")) {
        alarmRangePrint(&gAlarmConfig->AlarmValuePacketsLost, str);
    } else if (!strcmp(name, "BandwidthAlarmValue")) {
        alarmRangePrint(&gAlarmConfig->BandwidthAlarmValue, str);
    } else if (!strcmp(name, "FramesLostAlarmValue")) {
        alarmRangePrint(&gAlarmConfig->FramesLostAlarmValue, str);
    } else if (!strcmp(name, "TimeLapseAlarmValue")) {
        alarmRangePrint(&gAlarmConfig->TimeLapseAlarmValue, str);
    } else if (!strcmp(name, "CushionAlarmValue")) {
        alarmRangePrint(&gAlarmConfig->CushionAlarmValue, str);
    } else {
        TR069Error("Device.X. name = %s\n", name);
    }
    pthread_mutex_unlock(&g_mutex);
}

static int alarmGetValue(char *name, char *str, unsigned int size)
{
    //先赋上默认值
    str[0] = '\0';
    if (!gAlarmConfig)
        return -1;

    if (strncmp(name, "Device.", 7)) {
        TR069Error("name = %s\n", name);
        return -1;
    }
    name += 7;

    if (!strncmp(name, "X_00E0FC.", 9)) {
        name += 9;
        alarmGetConfig(name, str, size);
    } else if (!strncmp(name, "DeviceInfo.X_CTC_IPTV_Alarm.", 28)) {
        name += 28;
        if (!strncmp(name, "AlarmConfig.", 12)) {
            name += 12;
            alarmGetConfig(name, str, size);
        } else if (!strcmp(name, "AlarmSwitch")) {
            alarmGetConfig(name, str, size);
        } else if (!strcmp(name, "AlarmReportLevel")) {
            alarmGetConfig(name, str, size);
        } else {
            pthread_mutex_lock(&g_mutex);
            alarmGet(name, str, size);
            pthread_mutex_unlock(&g_mutex);
        }
    } else if (!strncmp(name, "AlarmConfig.", 12)) {
        name += 12;
        alarmGetConfig(name, str, size);
    } else {
        TR069Error("Device. name = %s\n", name);
        return -1;
    }

    TR069Debug("name = %s, value = %s\n", name, str);

    return 0;
}

static void alarmSetTimer(int arg)
{
    pthread_mutex_lock(&g_mutex);
    extendConfigWrite("alarm");
    pthread_mutex_unlock(&g_mutex);
}

static int alarmSetConfig(char *name, char *str, unsigned int x)
{
    int ret = 0;

    TR069Debug("name = %s, value = %s\n", name, str);

    pthread_mutex_lock(&g_mutex);
    if (0 == strcmp(name, "AlarmSwitch")) {
        gAlarmConfig->AlarmSwitch = atoi(str);
        if (gAlarmConfig->AlarmSwitch > 0) {
            timerTaskUnRegist(gTask, alarmCheckTimer, 0);
            timerTaskRegist(gTask, 5, alarmCheckTimer5, 0);
        } else {
            timerTaskUnRegist(gTask, alarmCheckTimer5, 0);
            alarmReset( );
        }
        tr069_port_setValue("Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmSwitch", str, 0);
    } else if (0 == strcmp(name, "AlarmReportLevel")) {
        gAlarmConfig->AlarmReportLevel = atoi(str);
    } else if (0 == strcmp(name, "CPUAlarmValue")) {
        alarmRangeScan(&gAlarmConfig->AlarmValueCPU, str);
    } else if (0 == strcmp(name, "MemoryAlarmValue")) {
        alarmRangeScan(&gAlarmConfig->AlarmValueMemory, str);
    } else if (0 == strcmp(name, "DiskAlarmValue")) {
        alarmRangeScan(&gAlarmConfig->AlarmValueDisk, str);
    } else if (0 == strcmp(name, "PacketsLostAlarmValue")) {
        alarmRangeScan(&gAlarmConfig->AlarmValuePacketsLost, str);
        tr069_port_setValue("Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmConfig.PacketsLostAlarmValue", str, 0);
    } else if (0 == strcmp(name, "BandwidthAlarmValue")) {
        alarmRangeScan(&gAlarmConfig->BandwidthAlarmValue, str);
    } else if (0 == strcmp(name, "FramesLostAlarmValue")) {
        alarmRangeScan(&gAlarmConfig->FramesLostAlarmValue, str);
    } else if (0 == strcmp(name, "TimeLapseAlarmValue")) {
        alarmRangeScan(&gAlarmConfig->TimeLapseAlarmValue, str);
    } else if (0 == strcmp(name, "CushionAlarmValue")) {
        alarmRangeScan(&gAlarmConfig->CushionAlarmValue, str);
    } else {
        TR069Error("Device.X. name = %s\n", name);
        ret = -1;
    }
    pthread_mutex_unlock(&g_mutex);

    if (0 == ret)
        timerTaskRegist(gTask, 1, alarmSetTimer, 0);

    return ret;
}

int alarmSetValue(char *name, char *str, unsigned int x)
{
    if (!gAlarmConfig)
        return -1;

    if (strncmp(name, "Device.", 7)) {
        TR069Error("name = %s\n", name);
        return -1;
    }
    name += 7;

    if (0 == strncmp(name, "X_00E0FC.", 9)) {
        name += 9;
        return alarmSetConfig(name, str, x);
    }
    if (!strncmp(name, "DeviceInfo.X_CTC_IPTV_Alarm.", 28)) {
        name += 28;
        if (!strncmp(name, "AlarmConfig.", 12)) {
            name += 12;
            return alarmSetConfig(name, str, x);
        }
        return alarmSetConfig(name, str, x);
    }
    TR069Error("Device. name = %s\n", name);
    return -1;
}

static void alarmPost(void)
{
    Alarm *alarm;
    char buffer[64];

    TR069Debug("gClearPush = %p, gAlarmPush = %p\n" ,gClearPush, gAlarmPush);

    if (gClearPush) {
        gClearPost = gClearPush;
        gClearPush = NULL;

        alarm = gClearPost;
        while (alarm) {
            alarm->instance = tr069_api_setValue("Event.AddObject", ALARM_OBJECT_NAME, 0);
            if (alarm->instance > 0) {
                sprintf(buffer, "%s%d.", ALARM_OBJECT_NAME, alarm->instance);
                tr069_api_setValue("Event.Parameter", buffer, gClearID);
            }
            alarm = alarm->next;
        }
        tr069_api_setValue("Event.Post", "", gClearID);
        TR069Debug("gClearID\n");

    } else if (gAlarmPush) {
        gAlarmPost = gAlarmPush;
        gAlarmPush = NULL;

        alarm = gAlarmPost;
        while (alarm) {
            alarm->instance = tr069_api_setValue("Event.AddObject", ALARM_OBJECT_NAME, 0);
            if (alarm->instance > 0) {
                sprintf(buffer, "%s%d.", ALARM_OBJECT_NAME, alarm->instance);
                tr069_api_setValue("Event.Parameter", buffer, gAlarmID);
            }
            alarm = alarm->next;
        }
        tr069_api_setValue("Event.Post", "", gAlarmID);
        TR069Debug("gAlarmID\n");
    }

    TR069Debug("gClearPost = %p, gAlarmPost = %p\n", gClearPost, gAlarmPost);
    if (gClearPost || gAlarmPost)
        timerTaskRegist(gTask, 10, alarmPostTimer, 0);
}

static void alarmPostTimer(int arg)
{
    Alarm *alarm;

    timerTaskUnRegist(gTask, alarmPostTimer, 0);

    TR069Debug("gClearPost = %p, gAlarmPost = %p\n", gClearPost, gAlarmPost);

    pthread_mutex_lock(&g_mutex);

    if (gAlarmPost) {
        tr069_api_setValue("Event.DeleteObject", ALARM_OBJECT_NAME, 0);
        while (gAlarmPost) {
            alarm = gAlarmPost;
            gAlarmPost = gAlarmPost->next;

            if (alarm->isClear) {
                alarm->next = gClearPush;
                gClearPush = alarm;

                TR069Debug("Clear type = %d\n", alarm->type);
                alarmBodyClear(&alarm->body);
            } else {
                alarm->next = gAlarmHold;
                gAlarmHold = alarm;
            }
        }
    }

    if (gClearPost) {
        tr069_api_setValue("Event.DeleteObject", ALARM_OBJECT_NAME, 0);
        while (gClearPost) {
            alarm = gClearPost;
            gClearPost = gClearPost->next;
            alarmFree(alarm);
        }
    }

    if (gClearPush || gAlarmPush)
        alarmPost( );

    pthread_mutex_unlock(&g_mutex);
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
static int alarmNew(char *args, char *location)
{
    Alarm *alarm;
    int ret, type, code, level;

    if (3 != sscanf(args, "%d.%d.%d", &type, &code, &level)) {
        TR069Error("command = %s!\n", args);
        return -1;
    }

    ret = -1;
    pthread_mutex_lock(&g_mutex);

    if(type < 0 || type >= ALARM_TYPE_MAX || level <= ALARM_LEVEL_NONE || level >= ALARM_LEVEL_MAX) {
        TR069Error("type = %d, level = %d\n", type, level);
        goto Err;
    }
    if(gAlarmConfig->AlarmSwitch == 0 || gAlarmConfig->AlarmReportLevel < level) {
        TR069Error("AlarmSwitch = %d, AlarmReportLevel = %d, level = %d\n", gAlarmConfig->AlarmSwitch, gAlarmConfig->AlarmReportLevel, level);
        goto Err;
    }

    if (gAlarmArray[type]) {
        TR069Error("type = %d, Alarming!\n", type);
        goto Err;
    }

    alarm = (Alarm *)calloc(sizeof(Alarm), 1);
    if (!alarm) {
        TR069Error("calloc type = %d\n", type);
        goto Err;
    }
    gAlarmArray[type] = alarm;

    alarm->next = gAlarmPush;
    gAlarmPush = alarm;

    alarm->type = type;

    TR069Debug("AlarmPost: type = %d\n", type);

    alarmBodyPost(&alarm->body, type, code, level, location);

    if (!gAlarmPost && !gClearPost)
        alarmPost( );

    ret = 0;
Err:
    pthread_mutex_unlock(&g_mutex);

    return ret;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
static int alarmClear(char *args)
{
    Alarm *alarm, *prev;
    int type;

    if (1 != sscanf(args, "%d", &type)) {
        TR069Error("command = %s!\n", args);
        return -1;
    }
    if(type < 0 || type >= ALARM_TYPE_MAX) {
        TR069Error("type = %d\n", type);
        return -1;
    }

    TR069Debug("AlarmClear: type = %d\n", type);

    pthread_mutex_lock(&g_mutex);

    prev = NULL;
    alarm = gAlarmPush;
    while (alarm) {
        if (type == alarm->type) {//如果在gAlarmPush队列里，直接释放
            if (prev)
                prev->next = alarm->next;
            else
                gAlarmPush = alarm->next;
            TR069Debug("free type = %d\n", type);
            alarmFree(alarm);
            goto End;
        }
        prev = alarm;
        alarm = alarm->next;
    }

    alarm = gAlarmPost;
    while (alarm) {
        if (type == alarm->type) {//如果在gAlarmPost队列里，设置清除标记
            alarm->isClear = 1;
            TR069Debug("mark type = %d\n", type);
            goto End;
        }
        alarm = alarm->next;
    }

    if (!alarm) {
        prev = NULL;
        alarm = gAlarmHold;
        while (alarm) {
            if (type == alarm->type) {//如果在gAlarmHold队列里，移动到gClearPush队列里
                if (prev)
                    prev->next = alarm->next;
                else
                    gAlarmHold = alarm->next;

                TR069Debug("Clear type = %d\n", alarm->type);
                alarmBodyClear(&alarm->body);

                alarm->next = gClearPush;
                gClearPush = alarm;
    
                if (!gAlarmPost && !gClearPost)
                    alarmPost( );
                goto End;
            }
            prev = alarm;
            alarm = alarm->next;
        }
    }
End:
    pthread_mutex_unlock(&g_mutex);
    return 0;
}

static int alarmCommand(char *name, char *str, unsigned int x)
{
    if (strncmp(name, "Alarm", 5)) {
        TR069Error("name = %s!\n", name);
        return -1;
    }
    name += 5;
    if (!strncmp(name, "Post.", 5)) {
        name += 5;
        return alarmNew(name, str);
    }
    if (!strncmp(name, "Clear.", 6)) {
        name += 6;
        return alarmClear(name);
    }

    TR069Error("command = %s!\n", name);
    return -1;
}

void tr069_port_alarmInit(void)
{
    TR069Debug("%p\n", gAlarmConfig);

    if (gAlarmConfig)
        return;

    /**          2011-4-7 lh
    **与华为SE确认alarmswith 当次开机有效，重启后清0
    **/
    gAlarmConfig = (struct AlarmConfig*)calloc(sizeof(struct AlarmConfig), 1);
    gAlarmArray = (Alarm **)calloc(sizeof(Alarm *) * ALARM_TYPE_MAX, 1);

    tr069_api_registFunction("Device.DeviceInfo.X_CTC_IPTV_Alarm.",                 alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmSwitch",      alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmReportLevel", alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmConfig.",     alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmConfig.CPUAlarmValue",            alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmConfig.MemoryAlarmValue",         alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmConfig.DiskAlarmValue",           alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmConfig.PacketsLostAlarmValue",    alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmConfig.FramesLostAlarmValue",     alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmConfig.TimeLapseAlarmValue",      alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmConfig.CushionAlarmValue",        alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CU_STB.Alarm.",                      alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CU_STB.Alarm.EnabledAlarm",          alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CU_STB.Alarm.MonitoringInterval",    alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CU_STB.Alarm.PacketsLostRateAlarm",  alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CU_STB.Alarm.PacketsLost",           alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CU_STB.Alarm.FramesLostAlarm",       alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CU_STB.Alarm.FramesLost",            alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CU_STB.Alarm.AlarmReason",           alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CTC_IPTV.AlarmSwitch",               alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CTC_IPTV.AlarmReportLevel",          alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CTC_IPTV.CPUAlarmValue",             alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CTC_IPTV.MemoryAlarmValue",          alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CTC_IPTV.DiskAlarmValue",            alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CTC_IPTV.PacketsLostAlarmValue",     alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_CTC_IPTV.BandwidthAlarmValue",       alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_00E0FC.AlarmSwitch",                 alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_00E0FC.AlarmReportLevel",            alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_00E0FC.CPUAlarmValue",               alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_00E0FC.MemoryAlarmValue",            alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_00E0FC.DiskAlarmValue",              alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_00E0FC.PacketsLostAlarmValue",       alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_00E0FC.BandwidthAlarmValue",         alarmGetValue, alarmSetValue);
    tr069_api_registFunction("Device.X_00E0FC.Alarm.",                      alarmGetValue, alarmSetValue);

    tr069_api_registOperation("Alarm", alarmCommand);

#ifdef INCLUDE_TR069_CU
    gAlarmID = tr069_api_setValue("Event.Regist", "X CU Alarm", 0);
    gClearID = tr069_api_setValue("Event.Regist", "X CU Alarm", 0);
#endif//INCLUDE_TR069_CU
#ifdef INCLUDE_TR069_CTC
#ifdef Sichuan
    gAlarmID = tr069_api_setValue("Event.Regist", "X CTC ALARM", 0);
    gClearID = tr069_api_setValue("Event.Regist", "X CTC CLEARALARM", 0);
#else
    gAlarmID = tr069_api_setValue("Event.Regist", "X CTC ALARM", 0);
    gClearID = tr069_api_setValue("Event.Regist", "X CTC ALARM", 0);
#endif//Sichuan
#endif//INCLUDE_TR069_CTC
#ifdef INCLUDE_TR069_HUAWEI
    gAlarmID = tr069_api_setValue("Event.Regist", "X00E0FCAlarm", 0);
    gClearID = tr069_api_setValue("Event.Regist", "X00E0FCAlarm", 0);
#endif//INCLUDE_TR069_HUAWEI

    gAlarmConfig->AlarmSwitch = 0;
    gAlarmConfig->AlarmReportLevel = ALARM_LEVEL_MAJOR;
#ifdef Sichuan
    alarmRangeScan(&gAlarmConfig->AlarmValueDisk, "80,80");
    alarmRangeScan(&gAlarmConfig->AlarmValueCPU, "80,80");
    alarmRangeScan(&gAlarmConfig->AlarmValueMemory, "80,80");
    alarmRangeScan(&gAlarmConfig->AlarmValuePacketsLost, "2,2");
    alarmRangeScan(&gAlarmConfig->FramesLostAlarmValue, "2,2");
    alarmRangeScan(&gAlarmConfig->TimeLapseAlarmValue, "2");
    alarmRangeScan(&gAlarmConfig->CushionAlarmValue, "0,100");
#else
    alarmRangeScan(&gAlarmConfig->AlarmValueDisk, "80");
    alarmRangeScan(&gAlarmConfig->AlarmValueCPU, "80,90");
    alarmRangeScan(&gAlarmConfig->AlarmValueMemory, "80,90");
    alarmRangeScan(&gAlarmConfig->AlarmValuePacketsLost, "80,90");
    alarmRangeScan(&gAlarmConfig->BandwidthAlarmValue, "80,90");
#endif

    extendConfigInit( );
    extendConfigInsetObject("alarm");

    extendConfigInsetInt("alarm.AlarmSwitch",               &gAlarmConfig->AlarmSwitch);
    extendConfigInsetInt("alarm.AlarmReportLevel",          &gAlarmConfig->AlarmReportLevel);

    extendConfigInsetInt("alarm.AlarmValueDiskLeft",        &gAlarmConfig->AlarmValueDisk.left);
    extendConfigInsetInt("alarm.AlarmValueDiskRight",       &gAlarmConfig->AlarmValueDisk.right);

    extendConfigInsetInt("alarm.AlarmValueCPULeft",         &gAlarmConfig->AlarmValueCPU.left);
    extendConfigInsetInt("alarm.AlarmValueCPURight",        &gAlarmConfig->AlarmValueCPU.right);

    extendConfigInsetInt("alarm.AlarmValueMemoryLeft",      &gAlarmConfig->AlarmValueMemory.left);
    extendConfigInsetInt("alarm.AlarmValueMemoryRight",     &gAlarmConfig->AlarmValueMemory.right);

    extendConfigInsetInt("alarm.AlarmValuePacketsLostLeft", &gAlarmConfig->AlarmValuePacketsLost.left);
    extendConfigInsetInt("alarm.AlarmValuePacketsLostRight",&gAlarmConfig->AlarmValuePacketsLost.right);

    extendConfigInsetInt("alarm.BandwidthAlarmValueLeft",   &gAlarmConfig->BandwidthAlarmValue.left);
    extendConfigInsetInt("alarm.BandwidthAlarmValueRight",  &gAlarmConfig->BandwidthAlarmValue.right);

    extendConfigInsetInt("alarm.FramesLostAlarmValueLeft",  &gAlarmConfig->FramesLostAlarmValue.left);
    extendConfigInsetInt("alarm.FramesLostAlarmValueRight", &gAlarmConfig->FramesLostAlarmValue.right);
    extendConfigInsetInt("alarm.TimeLapseAlarmValueLeft",   &gAlarmConfig->TimeLapseAlarmValue.left);
    extendConfigInsetInt("alarm.TimeLapseAlarmValueRight",  &gAlarmConfig->TimeLapseAlarmValue.right);
    extendConfigInsetInt("alarm.CushionAlarmValueLeft",     &gAlarmConfig->CushionAlarmValue.left);
    extendConfigInsetInt("alarm.CushionAlarmValueRight",    &gAlarmConfig->CushionAlarmValue.right);

    extendConfigRead("alarm");
    TR069Printf("AlarmSwitch = %d, AlarmReportLevel = %d\n", gAlarmConfig->AlarmSwitch, gAlarmConfig->AlarmReportLevel);

    gTask = timerTaskCreate( );
}

int alarmPermille(unsigned long long value, unsigned long long total)
{
    if (value >= total)
        return 100;

    return (int)(1000 * value / total);
}
