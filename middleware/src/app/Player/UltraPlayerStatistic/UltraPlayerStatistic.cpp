#include "UltraPlayerStatistic.h"

#include "UltraPlayerVod.h"
#include "UltraPlayerMultiple.h"
#include "Assertions.h"
#include "Tr069X_CTC_IPTV_Monitor.h"
#include "app_tr069_alarm.h"
#include "mid/mid_time.h"

static int stringtime(char* buf)
{
    time_t sec;
    struct tm t;

    if(buf == NULL)
        ERR_OUT("buf is null\n");

    sec = mid_time();
    gmtime_r(&sec, &t);

    sprintf(buf, "%02d%02d%02d%02d%02d%02d",
            (t.tm_year + 1900) % 100,
            t.tm_mon + 1,
            t.tm_mday,
            t.tm_hour,
            t.tm_min,
            t.tm_sec);
    buf[14] = '\0';

    return 12;
Err:
    return 0;
}

namespace Hippo {
char UltraPlayerStatistic::s_abendInfo[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_FAID_SIZE];
int UltraPlayerStatistic::s_StreamTimeOutNum;
int UltraPlayerStatistic::s_StreamTimeOutTotalDuration;
int UltraPlayerStatistic::s_StreamTimeOutClock;
int UltraPlayerStatistic::s_StreamTimeOutMaxDuration;
int UltraPlayerStatistic::s_StreamTimeOutDuration[STATISTIC_INFO_FAID_NUM];
int UltraPlayerStatistic::s_bufferIncreaseNum;
int UltraPlayerStatistic::s_bufferDecreaseNum;
int UltraPlayerStatistic::s_playErrorNum;
char UltraPlayerStatistic::s_playErrorInfo[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_FAID_SIZE];
char UltraPlayerStatistic::s_streamTimeOutEvent[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_FAID_SIZE];
int UltraPlayerStatistic::s_switchChannelNum;
int UltraPlayerStatistic::s_switchChannelTotalTime;
int UltraPlayerStatistic::s_channelSwitchWorstTime;
int UltraPlayerStatistic::s_channelSwitchBestTime;

pthread_mutex_t* UltraPlayerStatistic::s_mutex = NULL;

pthread_mutex_t* UltraPlayerStatistic::getMutex()
{
    return s_mutex;
}


/*************for statistic **********************/

void
UltraPlayerStatistic::streamPostFail(int mult, char *url, int err_no)
{
    int tIndex = 0, len = 0;
    char *buf = NULL, *p = NULL;

    if(url) {
        pthread_mutex_lock(s_mutex);
        if(mult) {
            tIndex = UltraPlayerMultiple::s_statistic.getFailedNum() % STATISTIC_INFO_FAID_NUM;
            buf = UltraPlayerMultiple::s_statistic.getFailedInfo(tIndex);

            len = stringtime(buf);
            sprintf(buf + len, ",%s,%d", url, err_no);
            LogUserOperDebug("igmpinfo =%s,err_no=%d,buf =%s\n", url, err_no, buf);
            {
                int n = UltraPlayerMultiple::s_statistic.getFailedNum();
                UltraPlayerMultiple::s_statistic.setFailedNum(n + 1);
            }

#if defined(TR069_MONITOR) && defined(INCLUDE_TR069)
        monitor_cycle_statistics(FLAG_MULTIFAILNUM);
#endif
        } else {
            tIndex = UltraPlayerVod::s_statistic.getFailedNum() % STATISTIC_INFO_FAID_NUM;
            buf = UltraPlayerVod::s_statistic.getFailedInfo(tIndex);

            len = stringtime(buf);
            len += sprintf(buf + len, ",");
            p = strchr(url, '?');
            if(p || strlen(url) < 80) {
                len += sprintf(buf + len, "%s", url);
            } else {
                memcpy(buf + len, url, 80);
                buf[len + 80] = 0;
                len += 80;
            }
            sprintf(buf + len, ",%d", err_no);

            UltraPlayerVod::s_statistic.setFailedNum(UltraPlayerVod::s_statistic.getFailedNum() + 1);
#if defined(TR069_MONITOR) && defined(INCLUDE_TR069)
        monitor_cycle_statistics(FLAG_VODFAILNUM);
#endif
        }
    strcpy(s_playErrorInfo[s_playErrorNum % STATISTIC_INFO_FAID_NUM], buf);
    s_playErrorNum++;
#ifdef Sichuan
    app_report_media_access_alarm();
    //tr069_log_statistic_info(LOG_MSG_ERROR, url); // todo zhanglsx how to get this func
#endif
    pthread_mutex_unlock(s_mutex);
    }
    return;
}


void
UltraPlayerStatistic::streamPostOk(int mult, int rrt)
{
    pthread_mutex_lock(s_mutex);
    if (mult) {
        UltraPlayerMultiple::s_statistic.setRequestNum(UltraPlayerMultiple::s_statistic.getRequestNum() + 1);
        UltraPlayerMultiple::s_statistic.setRequestReactTime(rrt);
    } else {
        UltraPlayerVod::s_statistic.setRequestNum(UltraPlayerVod::s_statistic.getRequestNum() + 1);
        UltraPlayerVod::s_statistic.setRequestReactTime(rrt);
    }
    pthread_mutex_unlock(s_mutex);
    return;
}

void
UltraPlayerStatistic::streamAbendFail(char *url)
{
    int tIndex = 0, len = 0;
    char *buf = NULL, *p = NULL;

    if(url) {
        pthread_mutex_lock(s_mutex);
#if defined(HUAWEI_C10)
        if(strncmp(url, "igmp://", 7) == 0  || strncmp(url, "IGMP://", 7) == 0) {
            UltraPlayerMultiple::s_statistic.setUnderflowNum(UltraPlayerMultiple::s_statistic.getUnderflowNum() + 1);
            tIndex = UltraPlayerMultiple::s_statistic.getUnderflowNum() % STATISTIC_INFO_FAID_NUM;
            buf = UltraPlayerMultiple::s_statistic.getAbendInfo(tIndex);
        } else {
            UltraPlayerVod::s_statistic.setUnderflowNum(UltraPlayerVod::s_statistic.getUnderflowNum() + 1);
            tIndex = UltraPlayerVod::s_statistic.getUnderflowNum() % STATISTIC_INFO_FAID_NUM;
            buf = UltraPlayerVod::s_statistic.getAbendInfo(tIndex);
        }
#else
        if(strncmp(url, "igmp://", 7) == 0  || strncmp(url, "IGMP://", 7) == 0)
            UltraPlayerMultiple::s_statistic.setUnderflowNum(UltraPlayerMultiple::s_statistic.getUnderflowNum + 1);
        else
            UltraPlayerVod::s_statistic.setUnderflowNum(UltraPlayerVod::s_statistic.getUnderflowNum());

        tIndex = s_StreamTimeOutNum % STATISTIC_INFO_FAID_NUM;
        buf = s_abendInfo[tIndex];
#endif

        len = stringtime(buf);
        len += sprintf(buf + len, ",");
        p = strchr(url, '?');
        if(p || strlen(url) < 80) {
            len += sprintf(buf + len, "%s", url);
        } else {
            memcpy(buf + len, url, 80);
            buf[len + 80] = 0;
            len += 80;
        }
        sprintf(buf + len, ",1018");

        s_StreamTimeOutClock = mid_clock();
        pthread_mutex_unlock(s_mutex);
    }
    return;
}



#ifdef HUAWEI_C10
void
UltraPlayerStatistic::streamPostFlow(int mult, int width, int flow)
{
    pthread_mutex_lock(s_mutex);
    if (flow == -1) {
        if (width <= 720) {
            if (mult)
                UltraPlayerMultiple::s_statistic.setUnderflowNum(UltraPlayerMultiple::s_statistic.getUnderflowNum() + 1);
            else
                UltraPlayerVod::s_statistic.setUnderflowNum(UltraPlayerVod::s_statistic.getUnderflowNum() + 1);
        } else {
            if (mult)
                UltraPlayerMultiple::s_statistic.setHDUnderflowNum(UltraPlayerMultiple::s_statistic.getHDUnderflowNum() + 1);
            else
                UltraPlayerVod::s_statistic.setHDUnderflowNum(UltraPlayerVod::s_statistic.getHDUnderflowNum() + 1);
        }
        s_bufferIncreaseNum++;
#if defined(TR069_MONITOR) && defined(INCLUDE_TR069)
        monitor_cycle_statistics(FLAG_BUFDECNUM);
#endif
    } else {
        if (width <= 720) {
            if (mult)
                UltraPlayerMultiple::s_statistic.setOverflowNum(UltraPlayerMultiple::s_statistic.getOverflowNum() + 1);
            else
                UltraPlayerVod::s_statistic.setOverflowNum(UltraPlayerVod::s_statistic.getOverflowNum() + 1);
        } else {
            if (mult)
                UltraPlayerMultiple::s_statistic.setHDOverflowNum(UltraPlayerMultiple::s_statistic.getHDOverflowNum() + 1);
            else
                UltraPlayerVod::s_statistic.setHDOverflowNum(UltraPlayerVod::s_statistic.getHDOverflowNum() + 1);
        s_bufferDecreaseNum++;
#if defined(TR069_MONITOR) && defined(INCLUDE_TR069)
        monitor_cycle_statistics(FLAG_BUFINCNUM);
#endif
        }//width
    }//flow
#ifdef Sichuan
    app_report_cushion_alarm( );
#endif
    pthread_mutex_unlock(s_mutex);

    return;
}
#endif // end HUAWEI_C10

void
UltraPlayerStatistic::streamAbendend(void)
{
    pthread_mutex_lock(s_mutex);
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)
    if(!strlen(UltraPlayerVod::s_statistic.getAbendInfo(0)) && !strlen(UltraPlayerMultiple::s_statistic.getAbendInfo(0))
#else
    if(!strlen(s_abendInfo[0]))
#endif
        goto End;
    s_StreamTimeOutTotalDuration += app_abend_duration();
    s_StreamTimeOutClock = 0;
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)
    UltraPlayerVod::s_statistic.setUnderflowNum(UltraPlayerVod::s_statistic.getUnderflowNum() + 1);
    UltraPlayerMultiple::s_statistic.setUnderflowNum(UltraPlayerMultiple::s_statistic.getUnderflowNum() + 1);
#else
    s_StreamTimeOutNum++;
#endif
End:
    ;
    pthread_mutex_unlock(s_mutex);
}

int
UltraPlayerStatistic::app_abend_duration(void)
{
    int duration;
    pthread_mutex_lock(s_mutex);

    if(s_StreamTimeOutClock == 0)
        return 0;
    duration = (int)(mid_clock() - s_StreamTimeOutClock);
    if(duration <= 0)
        return 0;
    if(s_StreamTimeOutMaxDuration < duration)
        s_StreamTimeOutMaxDuration = duration;
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)
    s_StreamTimeOutDuration[UltraPlayerVod::s_statistic.getUnderflowNum() % STATISTIC_INFO_FAID_NUM] = duration;
    s_StreamTimeOutDuration[UltraPlayerMultiple::s_statistic.getUnderflowNum() % STATISTIC_INFO_FAID_NUM] = duration;
#else
    s_StreamTimeOutDuration[s_StreamTimeOutNum % STATISTIC_INFO_FAID_NUM] = duration;
#endif
    pthread_mutex_unlock(s_mutex);

    return duration;
}

void
UltraPlayerStatistic::streamStreamgap(void)
{
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
    return;
#else
    if(pthread_mutex_lock(s_mutex))
        return;
    s_StreamTimeOutNum++;
    pthread_mutex_unlock(s_mutex);
    return;
#endif //defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
}

void
UltraPlayerStatistic::streamChannelzap(int ms)  //
{
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
    return;
#else
    if(ms <= 0 || ms > 5000)
        return;
    if(pthread_mutex_lock(s_mutex))
        return;
    s_switchChannelTotalTime += ms;
    s_switchChannelNum++;
    if(ms > s_channelSwitchWorstTime)
        s_channelSwitchWorstTime = ms;
    if(ms < s_channelSwitchBestTime || s_channelSwitchBestTime == 0)
        s_channelSwitchBestTime = ms;

    pthread_mutex_unlock(s_mutex);
    return;
#endif //defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
}

void
UltraPlayerStatistic::mutexInit()
{
    if(s_mutex == NULL) {
        s_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(s_mutex, NULL);
    }
}

} // namespace Hippo

extern "C"
{
void stream_port_post_flow(int mult, int width, int flow)
{
    Hippo::UltraPlayer::s_statistic.streamPostFlow(mult, width, flow);
}
void stream_port_post_fail(int mult, char *url, int err_no)
{
     Hippo::UltraPlayer::s_statistic.streamPostFail(mult, url,err_no);
}
void stream_port_post_ok(int mult, int rrt)
{
    Hippo::UltraPlayer::s_statistic.streamPostOk(mult, rrt);
}

void stream_port_post_streamgap(void)
{
    Hippo::UltraPlayer::s_statistic.streamStreamgap();
}

void stream_port_post_channelzap(int ms)
{
    Hippo::UltraPlayer::s_statistic.streamChannelzap(ms);
}

void stream_port_post_abend_fail(char *url)
{
    Hippo::UltraPlayer::s_statistic.streamAbendFail(url);
}

void stream_port_post_abend_end(void)
{
    Hippo::UltraPlayer::s_statistic.streamAbendend();
}
}