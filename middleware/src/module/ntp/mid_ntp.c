
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include <netinet/in.h>

#include "mid_ntp.h"
#include "mid/mid_sem.h"
#include "mid/mid_task.h"
#include "mid/mid_time.h"
#include "mid/mid_timer.h"
#include "mid/mid_tools.h"

#include "Assertions.h"

#include "Verimatrix.h"
#include "sys_msg.h"
#include "app/Message/MessageTypes.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "sys_basic_macro.h"
#include "app/BrowserBridge/Huawei/BrowserEventQueue.h"
#include "json/json.h"
#include "json/json_object.h"
#include "json/json_public.h"

typedef struct {
    char serv[64];
    unsigned int addr; /* -1 means do not time synchronization 0 means time synchronization failed. 1 means time synchronization ok */
    int status;
} NTPINFO;

extern int sntpc_gettime(char *server, struct timeval *ptv_timeout, struct timespec *pts);
static void a_Ntp_JseMapInit();


static time_t ntpFistSyncTime = 0;
#ifdef Chongqing
static int auto_reboot_flag = 0;
#endif
static NTPINFO g_ntpinfo = {{0}, INADDR_NONE, 0};
static mid_sem_t g_sem = NULL;
static pthread_mutex_t    g_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_pause = 0;


void mid_ntp_time_sync()
{
    // Android不会初始化NTP
    if (!g_sem)
        return;
    if(pthread_mutex_trylock(&g_mutex))
        return;
    mid_sem_give(g_sem);
    pthread_mutex_unlock(&g_mutex);
}

void mid_ntp_time_contrl(int flags)
{
    g_pause = flags;
}

unsigned int mid_ntp_firstSyncTimeGet(void)
{
    return ntpFistSyncTime;
}

int mid_ntp_firstSyncTimeSet(time_t second)
{
    ntpFistSyncTime = second;
    return 0;
}

static void mid_ntp_task(void)
{
    struct timeval tv;
    struct timespec getted_time = {0};
    int	ret = 0;
    int i = 0;
    int tTimezone = 0;
    double TimeZone = 0.0;
    int loop_sec = 2 * 60 * 60;
    int ntptypeerror = 0;
    /* If sync time from ntp server once, set 1. */
    int ntpFirtSyncFlag = 0;
    char ntpMain[USER_LEN] = "";
    char ntpBack[USER_LEN] = "";
    char ntpMain1[USER_LEN] = "";
    char ntpBack1[USER_LEN] = "";

    while(1) {
        mid_sem_take(g_sem, loop_sec, 0);
        if(g_pause)
            continue;

        pthread_mutex_lock(&g_mutex);
        LogSafeOperDebug("Begin ntp sync...\n");
        for(i = 0; i < 6; i ++) {
            unsigned int sec = mid_time();

            memset(ntpMain, 0, USER_LEN);
            memset(ntpBack, 0, USER_LEN);
            sysSettingGetString("ntp", ntpMain, USER_LEN, 0);
            sysSettingGetString("ntp1", ntpBack, USER_LEN, 0);
            LogSafeOperDebug("Main(%s), Back(%s)\n", ntpMain, ntpBack);

            if(i % 2 == 0)
                strcpy(g_ntpinfo.serv, ntpMain);
            else
                strcpy(g_ntpinfo.serv, ntpBack);

            ret = mid_tool_resolvehost(g_ntpinfo.serv, &g_ntpinfo.addr);
            if(ret == -3)
                ntptypeerror = -3;
            else if(ret == -2 || ret == -1)
                ntptypeerror = -2;
            else {
                char buf[16] = {0};
                struct timeval tTv;

                if((0 == i) || (i == 1)) {
                    tTv.tv_sec = 5;
                } else if((2 == i) || (i == 3)) {
                    tTv.tv_sec = 8;
                } else {
                    tTv.tv_sec = 15;
                }
                tTv.tv_usec = 0;

                mid_tool_addr2string(g_ntpinfo.addr, buf);
                if(sntpc_gettime(buf, &tTv, &getted_time) != 0) {
                    LogRunOperError("hsyslog: sntpc_gettime failed!\n");
                    g_ntpinfo.addr = INADDR_NONE;
                    LogRunOperError("sntpc_gettime failed from server(%s)!\n\n", g_ntpinfo.serv);
                } else {
                    break;
                }
            }

            sec = mid_time() - sec;
            if((0 == i) || (i == 1)) {
                if(sec < 5)
                    mid_task_delay((5 - sec) * 1000);
            } else if((2 == i) || (i == 3)) {
                if(sec < 8)
                    mid_task_delay((8 - sec) * 1000);
            } else {
                if(sec < 15)
                    mid_task_delay((15 - sec) * 1000);
            }
        }
        pthread_mutex_unlock(&g_mutex);
        memset(ntpMain1, 0, USER_LEN);
        memset(ntpBack1, 0, USER_LEN);
        sysSettingGetString("ntp", ntpMain1, USER_LEN, 0);
        sysSettingGetString("ntp1", ntpBack1, USER_LEN, 0);
        if(strcmp(ntpMain1,ntpMain) || strcmp(ntpBack1,ntpBack))
            mid_ntp_time_sync();

        if(i >= 6) { /* Time synchronization fails */
            g_ntpinfo.status = 0;
            if(ntptypeerror == -3) {
                sendMessageToKeyDispatcher(MessageType_Unknow, NTP_DNS_RESOLVE_ERROR, 0, 0);
            } else if(ntptypeerror == -2) {
                sendMessageToKeyDispatcher(MessageType_Unknow, NTP_DNS_SERVER_NOTFOUND, 0, 0);
            }
            sendMessageToKeyDispatcher(MessageType_Unknow, NTP_SYNC_ERROR, 0, 0);
            LogRunOperDebug("ntp sync error!\n");

            /* If synchronization failed, ten min synchronization */
            loop_sec = 10 * 60;
            g_ntpinfo.addr = INADDR_NONE;

            if(0 == ntpFirtSyncFlag) {
                mid_ntp_firstSyncTimeSet(mid_tool_string2time(DEFAULT_SYSTEM_TIME));
                ntpFirtSyncFlag = 1;
            }
            continue;
        }

        /* If successful synchronization,every two hours synchronization */
        loop_sec = 2 * 60 * 60;
        tv.tv_sec = getted_time.tv_sec;
        tv.tv_usec = getted_time.tv_nsec / 1000;

        sysSettingGetInt("timezone", &tTimezone, 0);

        if(tTimezone < 52) {
            TimeZone = tTimezone;
        } else {
            TimeZone = (double)(tTimezone - 100) / 4.0;
        }
		int saveFlag = 0;
		appSettingGetInt("saveflag", &saveFlag, 0);

        if(1 == saveFlag) {
            unsigned int delay = 2 * 60 * 60;
			int lightstart = 0, lightstop = 0;
			appSettingGetInt("lightstart", &lightstart, 0);
			appSettingGetInt("lightstop", &lightstop, 0);

            PRINTF("lightstart time = %d, lightstop time = %d.\n", lightstart, lightstop);

            if((tv.tv_sec >= lightstart) && (tv.tv_sec < lightstart + 10)) {
                char a2_event[100] = "", startTime[20] = "";
                mid_tool_time2string(lightstart, startTime, '-');
                sprintf(a2_event, "{\"type\":\"EVENT_INTO_DST\",\"startTime\":\"%s\"}", startTime);
                browserEventSend(a2_event, NULL);
            }

            if((tv.tv_sec >= lightstop) && (tv.tv_sec < lightstop + 10)) {
                char a2_event[100] = "", endTime[20] = "";
                mid_tool_time2string(lightstop, endTime, '-');
                sprintf(a2_event, "{\"type\":\"EVENT_EXIT_DST\",\"endTime\":\"%s\"}", endTime);
                browserEventSend(a2_event, NULL);
            }

            /* When current time and daylight saving time less then two hour,synchronize in less than two hours */
            if(tv.tv_sec < lightstart)
                delay = lightstart - tv.tv_sec;
            if(tv.tv_sec < lightstop)
                delay = lightstop - tv.tv_sec;
            if((delay > 0) && (delay <= 2 * 60 * 60)) {
                if(mid_timer_create(delay, 1, (mid_timer_f)mid_ntp_time_sync, 0) != 0)
                    LogRunOperError("mid_timer_create failed!\n");
            }
        }

        mid_set_timezone(mid_tool_timezone2sec(tTimezone));
        extern int yu_setTimeZone(double timeZone);
        yu_setTimeZone(TimeZone);

        settimeofday(&tv, NULL);
        tzset();
        LogRunOperDebug("Ntp sync ok %d<->%d!\n", tv.tv_sec, tv.tv_usec);
        sendMessageToKeyDispatcher(MessageType_Unknow, NTP_SYNC_OK, 0, 0);

        LogRunOperDebug("hsyslog:ntp success ,the ntp tv.time =%d\n", time(NULL));
        g_ntpinfo.status = 1;
        if(0 == ntpFirtSyncFlag) {
            mid_ntp_firstSyncTimeSet(tv.tv_sec);
            ntpFirtSyncFlag = 1;
        }
#ifdef Chongqing
        if(auto_reboot_flag == 0) {
            tv.tv_sec += tTimezone * 60 * 60;
            set_nowtime_for_reboot(tv.tv_sec);
        }
#endif
    }
}

int mid_ntp_init()
{
    struct timeval tv;
    /* registered ntp field to epg */
    if(g_sem == NULL) {
        g_sem = mid_sem_create();
        mid_task_create("mid_ntp", (mid_func_t)mid_ntp_task, 0);
    }
    LogSysOperDebug("hsysylog : begin to connect ntp server\n");

    tv.tv_sec = mid_tool_string2time(DEFAULT_SYSTEM_TIME);
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
    return 0;
}

int mid_ntp_status(void)
{
    return g_ntpinfo.status;
}

#ifdef Chongqing
int set_nowtime_for_reboot(int sec)
{
    char currenttime[64] = {0};
    char hour[10] , min[10];
    int reboot_time = -1;
    int total_sec = 0;

	sysSettingGetInt("reboot_time", &reboot_time, 0);
    if(reboot_time == -1) {
        auto_reboot_flag = 1;
        PRINTF("Don`t need auto restart\n", reboot_time);
        return  -1;
    }

    mid_tool_time2string(sec, currenttime, ':');
    PRINTF("current time = %s, Set reboot time = %d \n", currenttime, reboot_time);
    strncpy(hour, currenttime + strlen("1900:07:01:"), 2);
    strncpy(min, currenttime + strlen("1900:07:01:00:"), 2);

    total_sec = (24 - atoi(hour) - 1 + reboot_time) * 60 * 60 + (60 - atoi(min)) * 60;
    PRINTF("Will reboot STB after %02d:%02d, total_sec = %d \n", (24 - atoi(hour) - 1 + reboot_time), (60 - atoi(min)), total_sec);
    mid_timer_create(total_sec, 1, (mid_timer_f)mid_fpanel_reboot, 0);
    auto_reboot_flag = 1;
    return 0;
}
#endif

