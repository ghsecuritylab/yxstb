
#include "LogModule.h"

#include "Assertions.h"
#include "LogModuleHuawei.h"
#include <sys/types.h>
#include <time.h>
#include <string.h>

#include "tr069_define.h"
#include "tr069_param.h"
#include "sys_basic_macro.h"
#include "mid/mid_timer.h"
#include "mid/mid_time.h"

#if defined(INCLUDE_TR069) || defined(INCLUDE_HMWMGMT)
static unsigned int g_startTime = 0;    //unit: second
static unsigned int g_continueTime = 0; //unit: minute

//timer thread function
static void _LogTimerStart(int arg);
static void _LogTimerEnd(int arg);

static void _LogTimerStart(int arg /* second */)
{
    huaweiLog();
    printf("duration time is %ds\n", arg);
    mid_timer_create(arg, 1, _LogTimerEnd, 0);
}

static void _LogTimerEnd(int arg)
{
    mid_timer_delete_all(_LogTimerStart);
    huaweiSetLogOutPutType(0); //close log
    huaweiLog(); //release resource
}

extern "C"
void logTR069SetLogStartTime(unsigned int startTime)
{
    g_startTime = startTime; //UTC
}

extern "C"
unsigned int logTR069GetLogStartTime()
{
    return g_startTime;
}
/*当系统时区设置进去了，tr069，中间件都是utc时间比较,无需作任何处理                              */
/*当系统时区没有设置，tr069下发的时间被认定为“utc”时间，实际服务器下发的是加了时区的，那么中间件加上一个时区的时间比较  */

static void logTR069SetLogContinueTime(  int continueTime /* min */)
{
    time_t tempTime;
    g_continueTime = continueTime;
    continueTime *= 60; //unit: s
    tzset();
	if(timezone == 0)
	    tempTime = time(0) + get_local_time_zone();
    else
	    tempTime = time(0);
    printf("logTr069: now[%ld] start[%u] continue[%d]\n", tempTime, g_startTime, g_continueTime);
    if (g_startTime > tempTime) {
        tempTime = g_startTime - tempTime;
        mid_timer_create(tempTime, 1, _LogTimerStart, continueTime);
    } else {
        tempTime = tempTime - g_startTime;
        if (tempTime < g_continueTime*60)
            _LogTimerStart(g_continueTime*60 - tempTime);
    }
}

extern "C"
unsigned int logTR069GetLogContinueTime()
{
    return g_continueTime; //unit: minute
}

extern "C"
 void logTR069SetLogContinueTimeDelay(unsigned int continueTime)
{
	mid_timer_create(2,1,logTR069SetLogContinueTime,continueTime);
	return ;
}

#endif //INCLUDE_TR069 || INCLUDE_HMWMGMT
