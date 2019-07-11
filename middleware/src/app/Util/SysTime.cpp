
#include "SysTime.h"

#include <sys/time.h>
#include <time.h>
#include <stdio.h>

#include "mid/mid_time.h"


namespace Hippo {

void 
SysTime::GetDateTime(DateTime* dt)
{
    if (dt) {
        struct timeval current;
        struct tm temp_time;
        
        if (!gettimeofday(&current, NULL)){
            localtime_r(&current.tv_sec, &temp_time);
            dt->mYear       = temp_time.tm_year + 1900;
            dt->mMonth      = temp_time.tm_mon + 1;
            dt->mDayOfWeek  = temp_time.tm_wday;
            dt->mDay        = temp_time.tm_mday;
#if defined(hi3560e)
            dt->mHour       = temp_time.tm_hour + 8;
#else
            dt->mHour       = temp_time.tm_hour;
#endif
            dt->mMinute     = temp_time.tm_min;
            dt->mSecond     = temp_time.tm_sec;
	    }
    }
}

uint32_t 
SysTime::GetMSecs()
{
    //struct timeval tv;
    //gettimeofday(&tv, NULL);
    //return (uint32_t) (tv.tv_sec * 1000 + tv.tv_usec / 1000 ); // microseconds to milliseconds
    return mid_ms();
}

} // namespace Hippo
