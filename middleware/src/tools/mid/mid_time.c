
#include <time.h>

#include "AppSetting.h"
#include "SysSetting.h"

#include "mid_time.h"
#include "mid_tools.h"


extern int yu_setTimeZone(double timeZone);

static int g_timezone_sec = 0;
static int g_saving_time_sec = 0;

mid_msec_t mid_clock(void)
{
    mid_msec_t msec;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    msec = tp.tv_sec;
    msec = msec * 1000 + tp.tv_nsec / 1000000;
    return msec;
}

unsigned int mid_ms(void)
{
    mid_msec_t msec;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    msec = tp.tv_sec;
    msec = msec * 1000 + tp.tv_nsec / 1000000;
    return msec;
}

int set_local_time_zone(void)
{
    double real_time_zone = 0;
    int time_zone = 0;

    sysSettingGetInt("timezone", &time_zone, 0);
    if(time_zone < 52) {
        real_time_zone = time_zone;
    } else {
        real_time_zone = (time_zone - 100) / 4.0;
    }
#if defined(ANDROID)
    char    tz_str[200];
    int     hour_off, minute_off;
    hour_off = (int)real_time_zone;
    minute_off = (int)((real_time_zone - (double)hour_off) * 60);
    if (real_time_zone > 0)  {
        snprintf(tz_str, sizeof(tz_str), "GMT-%02d:%02d", hour_off, minute_off);
    } else {
        snprintf(tz_str, sizeof(tz_str), "GMT-%02d:%02d", hour_off, minute_off);
    }
    setenv("TZ", tz_str, 1);
    tzset();
#else
    // ANDROID调这个函数会占用7秒之久。。
    yu_setTimeZone(real_time_zone);
#endif
    g_timezone_sec = mid_tool_timezone2sec(time_zone);

    return 0;
}

int get_local_time_zone(void)
{
    return g_timezone_sec;
}

int set_saving_time_sec(void)
{
	appSettingGetInt("jumpstep", &g_saving_time_sec, 0);
    return 0;
}

int get_saving_time_sec(void)
{
    return g_saving_time_sec;
}

unsigned int mid_tvms_time(void)
{
    int save_flag = 0;
	appSettingGetInt("saveflag", &save_flag, 0);
    if(save_flag == 1) {
        unsigned int lightstart = 0;
        unsigned int lightstop = 0;
        unsigned int currenttime = time(NULL);

		appSettingGetInt("lightstart", &lightstart, 0);
        appSettingGetInt("lightstop", &lightstop, 0);

        if(currenttime >= lightstart && currenttime <= lightstop) {
            return (time(NULL) + g_timezone_sec + g_saving_time_sec);
        } else {
            return (time(NULL) + g_timezone_sec);
        }
    }

    return (time(NULL) + g_timezone_sec);
}

unsigned int mid_time(void)
{
    /* 0 means do not support utc time */
    if(MID_UTC_SUPPORT == 0) {
        int save_flag = 0;
		appSettingGetInt("saveflag", &save_flag, 0);
        if(save_flag == 1) {
            unsigned int lightstart = 0;
            unsigned int lightstop = 0;
            unsigned int currenttime = time(NULL);

			appSettingGetInt("lightstart", &lightstart, 0);
			appSettingGetInt("lightstop", &lightstop, 0);

            if(currenttime >= lightstart && currenttime <= lightstop) {
                return (time(NULL) + g_timezone_sec + g_saving_time_sec);
            } else {
                return (time(NULL) + g_timezone_sec);
            }
        } else {
            return (time(NULL) + g_timezone_sec);
        }
    } else {
        return (time(NULL));
    }
}

void mid_set_timezone(unsigned int sec)
{
    g_timezone_sec = sec;
}

unsigned int mid_10ms(void)
{
    unsigned int clk;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    clk = (unsigned int)tp.tv_sec * 100 + (unsigned int)tp.tv_nsec / 10000000;

    return clk;
}

unsigned int mid_get_times_sec()
{
    int save_flag = 0;
	appSettingGetInt("saveflag", &save_flag, 0);

    if(save_flag == 1) {
        unsigned int lightstart = 0;
        unsigned int lightstop = 0;
        unsigned int currenttime = time(NULL);

		appSettingGetInt("lightstart", &lightstart, 0);
		appSettingGetInt("lightstop", &lightstop, 0);

        if(currenttime >= lightstart && currenttime <= lightstop) {
            return g_timezone_sec + g_saving_time_sec;
        } else {
            return g_timezone_sec;
        }
    }

    return g_timezone_sec;
}

