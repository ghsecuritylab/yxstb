#include "UltraPlayerStatisticVod.h"

#include "Assertions.h"
#include "UltraPlayer.h"

#define STATISTIC_INFO_FAID_NUM         10
#define STATISTIC_INFO_FAID_SIZE        256

namespace Hippo {

int UltraPlayerStatisticVod::s_vodRequestNum;
int UltraPlayerStatisticVod::s_vodRequestReactTime;
int UltraPlayerStatisticVod::s_vodFailedNum;
char UltraPlayerStatisticVod::s_vodFailedInfo[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_FAID_SIZE];
char UltraPlayerStatisticVod::s_vodAbendInfo[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_FAID_SIZE];
int UltraPlayerStatisticVod::s_vodUnderflowNum;
int UltraPlayerStatisticVod::s_vodOverflowNum;
int UltraPlayerStatisticVod::s_HD_vodUnderflowNum;
int UltraPlayerStatisticVod::s_HD_vodOverflowNum;
int UltraPlayerStatisticVod::s_vodTotalChangeStopTime;
int UltraPlayerStatisticVod::s_vodStopNum;
int UltraPlayerStatisticVod::s_vodTotalChangePauseTime;
int UltraPlayerStatisticVod::s_vodPauseNum;
int UltraPlayerStatisticVod::s_vodTotalRequestReactTime;
int UltraPlayerStatisticVod::s_vodRequsetReactWorst;
int UltraPlayerStatisticVod::s_vodRequsetReactBest;
int UltraPlayerStatisticVod::s_vodTotalChangeResumTime;
int UltraPlayerStatisticVod::s_vodResumNum;
int UltraPlayerStatisticVod::s_vodTotalChangeFastForwardTime;
int UltraPlayerStatisticVod::s_vodFastForwardNum;
int UltraPlayerStatisticVod::s_vodTotalChangeFastRewindTime;
int UltraPlayerStatisticVod::s_vodFastRewindNum;

void
UltraPlayerStatisticVod::streamVodstop(int ms)
{
    if(pthread_mutex_lock(UltraPlayer::s_statistic.getMutex()))
        return ;
    s_vodTotalChangeStopTime += ms;
    s_vodStopNum++;
    pthread_mutex_unlock(UltraPlayer::s_statistic.getMutex());
    return;
}

void
UltraPlayerStatisticVod::streamVodpause(int ms)
{
    if(pthread_mutex_lock(UltraPlayer::s_statistic.getMutex()))
        return ;
    s_vodTotalChangePauseTime += ms;
    s_vodPauseNum++;
    pthread_mutex_unlock(UltraPlayer::s_statistic.getMutex());
    return;
}

void
UltraPlayerStatisticVod::streamVodreq(int ms)
{
    if(ms <= 0)
        return;

    if(pthread_mutex_lock(UltraPlayer::s_statistic.getMutex()))
        return ;
    s_vodTotalRequestReactTime += ms;
    s_vodRequestNum++;
    if(ms > s_vodRequsetReactWorst)
        s_vodRequsetReactWorst = ms;
    if(ms < s_vodRequsetReactBest || s_vodRequsetReactBest == 0)
        s_vodRequsetReactBest = ms;

    pthread_mutex_unlock(UltraPlayer::s_statistic.getMutex());
    return;
}

void
UltraPlayerStatisticVod::streamVodresume(int ms)
{
    if(pthread_mutex_lock(UltraPlayer::s_statistic.getMutex()))
        return ;
    s_vodTotalChangeResumTime += ms;
    s_vodResumNum++;
    pthread_mutex_unlock(UltraPlayer::s_statistic.getMutex());
    return;
}


void
UltraPlayerStatisticVod::streamVodforward(int ms)
{
    if(pthread_mutex_lock(UltraPlayer::s_statistic.getMutex()))
        return;
    s_vodTotalChangeFastForwardTime += ms;
    s_vodFastForwardNum++;
    pthread_mutex_unlock(UltraPlayer::s_statistic.getMutex());
    return;
}

void
UltraPlayerStatisticVod::streamVodbackward(int ms)
{
    if(pthread_mutex_lock(UltraPlayer::s_statistic.getMutex()))
        return;
    s_vodTotalChangeFastRewindTime += ms;
    s_vodFastRewindNum++;
    pthread_mutex_unlock(UltraPlayer::s_statistic.getMutex());
    return;
}

} // namespace Hippo



// for statistic
extern "C"{
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)

void stream_port_post_vodstop(int ms)
{
   return;
}
void stream_port_post_vodpause(int ms)
{
   return;
}
void stream_port_post_vodreq(int ms)
{
   return;
}
void
stream_port_post_vodresume(int ms)
{
   return;
}
void stream_port_post_vodforward(int ms)
{
   return;
}
void stream_port_post_vodbackward(int ms)
{
   return;
}
#else
void stream_port_post_vodstop(int ms)
{
    Hippo::UltraPlayerStatisticVod::streamVodstop(ms);
}
void stream_port_post_vodpause(int ms)
{
    Hippo::UltraPlayerStatisticVod::streamVodpause(ms);
}
void stream_port_post_vodreq(int ms)
{
    Hippo::UltraPlayerStatisticVod::streamVodreq(ms);
}
void
stream_port_post_vodresume(int ms)
{
    Hippo::UltraPlayerStatisticVod::streamVodresume(ms);
}
void stream_port_post_vodforward(int ms)
{
    Hippo::UltraPlayerStatisticVod::streamVodforward(ms);
}
void stream_port_post_vodbackward(int ms)
{
    Hippo::UltraPlayerStatisticVod::streamVodbackward(ms);
}
#endif // defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)

} //  extern "C"
