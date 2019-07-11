#include "StatisticBase.h"

#include "Assertions.h"

#include "app_jse.h"
#include "mid/mid_time.h"
#include "mid/mid_timer.h"
#include "mid/mid_tools.h"
#include "tr069_port1.h"

#include "takin.h"

#include "SettingApi.h"

#include "../../../Player/UltraPlayer.h"
#include "../../../Player/UltraPlayerMultiple.h"
#include "../../../Player/UltraPlayerVod.h"
#include "../StatisticRoot.h"
#include "Jse/Huawei/Business/Session/JseHWSession.h"
#include "Jse/Hybroad/Business/JseBusiness.h"
#include "../StatisticLog/StatisticLog.h"

#include <stdlib.h>

extern  int jseAuthCountGet();

// 静态成员变量初始化 start
pthread_mutex_t* StatisticBase::s_mutex = NULL;

unsigned int StatisticBase::s_startClock = 0; //中间变量

char* StatisticBase::s_packBuf = 0;
char StatisticBase::s_filebuf[LOG_SERVER_URL_SIZE] = {0};
int StatisticBase::s_startFlag = 0;

char StatisticBase::s_powerOnTime[32] = {0};
int StatisticBase::s_bootUptime = 0;
int StatisticBase::s_isSetPowerOnTime = 0;

struct StatisticBase::StatisticCfg StatisticBase::s_statisticCfg;
struct StatisticBase::StatisticData StatisticBase::s_statisticData;

#ifdef TR069_LIANCHUANG
struct StatisticBase::LianChuangStatistic s_LianChuangStatistic;
#endif // TR069_LIANCHUANG
// 静态成员变量初始化 end


StatisticBase::~StatisticBase()
{
}

StatisticBase::StatisticBase()
{
}

char* StatisticBase::getPackBuf()
{
    return s_packBuf;
}

char* StatisticBase::packBufInit()
{
    s_packBuf = (char *)malloc(STATISTIC_FILE_SIZE + 4);
}

void
StatisticBase::mutexInit()
{
    if(NULL == s_mutex) {
        s_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(s_mutex, NULL);
    }
}

static int int_getStatInterval(void)
{
    unsigned int statInterval, packetsLostInterval, bitRateInterval, framLostInterval;

    packetsLostInterval = StatisticBase::s_statisticCfg.PacketsLostInterval & 0xff;
    bitRateInterval = StatisticBase::s_statisticCfg.BitRateInterval & 0xff;
    framLostInterval = StatisticBase::s_statisticCfg.FramLostInterval &0xff;
    //statInterval = packetsLostInterval << 16 + packetsLostInterval << 8 + bitRateInterval;
    statInterval = (packetsLostInterval << 16) + (framLostInterval << 8) +bitRateInterval;
    return statInterval;
}

unsigned int statisticLocakTime(void)
{
    unsigned int t = mid_time();
    if (MID_UTC_SUPPORT)
        t += get_local_time_zone();
    return t;
}

int setSavePoint()
{
    pthread_mutex_lock(StatisticBase::s_mutex);

    StatisticBase::s_statisticData.Endpoint = statisticLocakTime( );
    StatisticBase::s_statisticData.Startpoint = StatisticBase::s_statisticData.Endpoint - (mid_10ms( ) - StatisticBase::s_startClock) / 100;
    StatisticBase::s_statisticData.RebootFlag = 1;
    StatisticBase::s_statisticData.checksum = 0;
    StatisticBase::s_statisticData.checksum = -mid_tool_checksum((char *)&StatisticBase::s_statisticData, sizeof(struct StatisticBase::StatisticData));
    pthread_mutex_unlock(StatisticBase::s_mutex);
}


void statisticSetIsPowerOnTime(int set)
{
    StatisticBase::s_isSetPowerOnTime = set;
}
int statisticIsSetPowerOnTime(void)
{
    return StatisticBase::s_isSetPowerOnTime ;
}
void statisticSetPowerOnTime(void)
{
    mid_tool_time2string(((MID_UTC_SUPPORT)? mid_time() + get_local_time_zone(): mid_time()), StatisticBase::s_powerOnTime, 0);
}

void statisticSetCfgDirty(int n)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return ; //加锁失败

    StatisticBase::s_statisticCfg.dirty = n;
    pthread_mutex_unlock(StatisticBase::s_mutex);
}

// StatisticRoot.cpp 中使用
unsigned int statisticGetBitRateInterval()
{
    return  StatisticBase::s_statisticCfg.BitRateInterval;
}
// StatisticRoot.cpp 中使用
unsigned int statisticGetPacketsLostInterval()
{
    return  StatisticBase::s_statisticCfg.PacketsLostInterval;
}


// 读写接口函数
extern "C" {

void tr069_statistic_set_MonitoringInterval(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.PacketsLostInterval != value) {
        StatisticBase::s_statisticCfg.PacketsLostInterval = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

//app_jse.c 中使用，可能要删除
void tr069_statistic_set_userstatus_time(void)
{
    FILE *fp = NULL;
    char *p = NULL;
    char buf[32] = {0};

    if(StatisticBase::s_bootUptime == 0) {
        fp = fopen("/proc/uptime", "r");
        if(fp == NULL) {
            ERR_OUT("open file error!\n");
        }
        fgets(buf, sizeof(buf), fp);
        p = strstr(buf, " ");
        if(p) {
            buf[p - buf ] = 0;
            StatisticBase::s_bootUptime = atoi(buf);
            StatisticBase::s_bootUptime += 4;
        }
    }
    fclose(fp);
Err:
    return;
}


int tr069_statistic_get_HD_BitRateR1(void)
{
    return StatisticBase::s_statisticCfg.HD_BitRateR1;
}

int tr069_statistic_get_HD_BitRateR2(void)
{
    return StatisticBase::s_statisticCfg.HD_BitRateR2;
}

int tr069_statistic_get_HD_BitRateR3(void)
{
    return StatisticBase::s_statisticCfg.HD_BitRateR3;
}

int tr069_statistic_get_HD_BitRateR4(void)
{
    return StatisticBase::s_statisticCfg.HD_BitRateR4;
}

int tr069_statistic_get_HD_BitRateR5(void)
{
    return StatisticBase::s_statisticCfg.HD_BitRateR5;
}

int tr069_statistic_get_MultiRRT(void)
{
    return Hippo::UltraPlayerMultiple::s_statistic.getRequestReactTime();
}

int tr069_statistic_get_VodRRT(void)
{
    return Hippo::UltraPlayerVod::s_statistic.getRequestReactTime();
}

int tr069_statistic_get_VODAbendNumbers(void)
{
    return Hippo::UltraPlayerVod::s_statistic.getUnderflowNum();
}
int tr069_statistic_get_MultiAbendNumbers(void)
{
    return Hippo::UltraPlayerMultiple::s_statistic.getUnderflowNum();
}

int tr069_statistic_get_MultiAbendUPNumbers(void)
{
    return Hippo::UltraPlayerMultiple::s_statistic.getOverflowNum();
}

int tr069_statistic_get_VODUPAbendNumbers(void)
{
    return Hippo::UltraPlayerVod::s_statistic.getOverflowNum();
}

int tr069_statistic_get_HD_MultiAbendNumbers(void)
{
    return Hippo::UltraPlayerMultiple::s_statistic.getHDUnderflowNum();
}

int tr069_statistic_get_HD_VODAbendNumbers(void)
{
    return Hippo::UltraPlayerVod::s_statistic.getHDUnderflowNum();
}

int tr069_statistic_get_HD_MultiUPAbendNumbers(void)
{
    return Hippo::UltraPlayerMultiple::s_statistic.getHDOverflowNum();
}

int tr069_statistic_get_HD_VODUPAbendNumbers(void)
{
    return Hippo::UltraPlayerVod::s_statistic.getHDOverflowNum();
}

void tr069_statistic_get_PlayErrorInfo(char *value, int size)
{
    if(value) {
        if (pthread_mutex_lock(Hippo::UltraPlayer::s_statistic.getMutex()))
            return;
        if(Hippo::UltraPlayer::s_statistic.getplayErrorNumbers() > 0)
            strcpy(value, Hippo::UltraPlayer::s_statistic.getPlayErrorInfo( (Hippo::UltraPlayer::s_statistic.getplayErrorNumbers() - 1) % STATISTIC_INFO_FAID_NUM));
        else
            value[0] = 0;
        pthread_mutex_unlock(Hippo::UltraPlayer::s_statistic.getMutex());
    }
    return;

}

int tr069_statistic_get_MultiPacketsLostR1Nmb(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR1;
}

int tr069_statistic_get_MultiPacketsLostR2Nmb(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR2;
}

int tr069_statistic_get_MultiPacketsLostR3Nmb(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR3;
}

int tr069_statistic_get_MultiPacketsLostR4Nmb(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR4;
}

int tr069_statistic_get_MultiPacketsLostR5Nmb(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR5;
}

int tr069_statistic_get_FECMultiPacketsLostR1Nmb(void)
{
    return StatisticBase::s_statisticData.FECMultiPacketsLostR1Nmb;
}

int tr069_statistic_get_FECMultiPacketsLostR2Nmb(void)
{
    return StatisticBase::s_statisticData.FECMultiPacketsLostR2Nmb;
}

int tr069_statistic_get_FECMultiPacketsLostR3Nmb(void)
{
    return StatisticBase::s_statisticData.FECMultiPacketsLostR3Nmb;
}

int tr069_statistic_get_FECMultiPacketsLostR4Nmb(void)
{
    return StatisticBase::s_statisticData.FECMultiPacketsLostR4Nmb;
}

int tr069_statistic_get_FECMultiPacketsLostR5Nmb(void)
{
    return StatisticBase::s_statisticData.FECMultiPacketsLostR5Nmb;
}

int tr069_statistic_get_VODPacketsLostR1Nmb(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR1;
}

int tr069_statistic_get_VODPacketsLostR2Nmb(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR2;
}

int tr069_statistic_get_VODPacketsLostR3Nmb(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR3;
}

int tr069_statistic_get_VODPacketsLostR4Nmb(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR4;
}

int tr069_statistic_get_VODPacketsLostR5Nmb(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR5;
}

int tr069_statistic_get_ARQVODPacketsLostR1Nmb(void)
{
    return StatisticBase::s_statisticData.ARQVODPacketsLostR1Nmb;
}

int tr069_statistic_get_ARQVODPacketsLostR2Nmb(void)
{
    return StatisticBase::s_statisticData.ARQVODPacketsLostR2Nmb;
}

int tr069_statistic_get_ARQVODPacketsLostR3Nmb(void)
{
    return StatisticBase::s_statisticData.ARQVODPacketsLostR3Nmb;
}

int tr069_statistic_get_ARQVODPacketsLostR4Nmb(void)
{
    return StatisticBase::s_statisticData.ARQVODPacketsLostR4Nmb;
}

int tr069_statistic_get_ARQVODPacketsLostR5Nmb(void)
{
    return StatisticBase::s_statisticData.ARQVODPacketsLostR5Nmb;
}

int tr069_statistic_get_MultiBitRateR1Nmb(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR1Nmb;
}

int tr069_statistic_get_MultiBitRateR2Nmb(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR2Nmb;
}

int tr069_statistic_get_MultiBitRateR3Nmb(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR3Nmb;
}

int tr069_statistic_get_MultiBitRateR4Nmb(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR4Nmb;
}

int tr069_statistic_get_MultiBitRateR5Nmb(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR5Nmb;
}

int tr069_statistic_get_VODBitRateR1Nmb(void)
{
    return StatisticBase::s_statisticData.VODBitRateR1Nmb;
}

int tr069_statistic_get_VODBitRateR2Nmb(void)
{
    return StatisticBase::s_statisticData.VODBitRateR2Nmb;
}

int tr069_statistic_get_VODBitRateR3Nmb(void)
{
    return StatisticBase::s_statisticData.VODBitRateR3Nmb;
}

int tr069_statistic_get_VODBitRateR4Nmb(void)
{
    return StatisticBase::s_statisticData.VODBitRateR4Nmb;
}

int tr069_statistic_get_VODBitRateR5Nmb(void)
{
    return StatisticBase::s_statisticData.VODBitRateR5Nmb;
}

int tr069_statistic_get_HD_MultiPacketsLostR1Nmb(void)
{
    return StatisticBase::s_statisticData.HD_MultiPacketsLostR1Nmb;
}

int tr069_statistic_get_HD_MultiPacketsLostR2Nmb(void)
{
    return StatisticBase::s_statisticData.HD_MultiPacketsLostR2Nmb;
}

int tr069_statistic_get_HD_MultiPacketsLostR3Nmb(void)
{
    return StatisticBase::s_statisticData.HD_MultiPacketsLostR3Nmb;
}

int tr069_statistic_get_HD_MultiPacketsLostR4Nmb(void)
{
    return StatisticBase::s_statisticData.HD_MultiPacketsLostR4Nmb;
}

int tr069_statistic_get_HD_MultiPacketsLostR5Nmb(void)
{
    return StatisticBase::s_statisticData.HD_MultiPacketsLostR5Nmb;
}

int tr069_statistic_get_HD_FECMultiPacketsLostR1Nmb(void)
{
    return StatisticBase::s_statisticData.HD_FECMultiPacketsLostR1Nmb;
}

int tr069_statistic_get_HD_FECMultiPacketsLostR2Nmb(void)
{
    return StatisticBase::s_statisticData.HD_FECMultiPacketsLostR2Nmb;
}

int tr069_statistic_get_HD_FECMultiPacketsLostR3Nmb(void)
{
    return StatisticBase::s_statisticData.HD_FECMultiPacketsLostR3Nmb;
}

int tr069_statistic_get_HD_FECMultiPacketsLostR4Nmb(void)
{
    return StatisticBase::s_statisticData.HD_FECMultiPacketsLostR4Nmb;
}

int tr069_statistic_get_HD_FECMultiPacketsLostR5Nmb(void)
{
    return StatisticBase::s_statisticData.HD_FECMultiPacketsLostR5Nmb;
}

int tr069_statistic_get_HD_VODPacketsLostR1Nmb(void)
{
    return StatisticBase::s_statisticData.HD_VODPacketsLostR1Nmb;
}

int tr069_statistic_get_HD_VODPacketsLostR2Nmb(void)
{
    return StatisticBase::s_statisticData.HD_VODPacketsLostR2Nmb;
}

int tr069_statistic_get_HD_VODPacketsLostR3Nmb(void)
{
    return StatisticBase::s_statisticData.HD_VODPacketsLostR3Nmb;
}

int tr069_statistic_get_HD_VODPacketsLostR4Nmb(void)
{
    return StatisticBase::s_statisticData.HD_VODPacketsLostR4Nmb;
}

int tr069_statistic_get_HD_VODPacketsLostR5Nmb(void)
{
    return StatisticBase::s_statisticData.HD_VODPacketsLostR5Nmb;
}

int tr069_statistic_get_HD_ARQVODPacketsLostR1Nmb(void)
{
    return StatisticBase::s_statisticData.HD_ARQVODPacketsLostR1Nmb;
}

int tr069_statistic_get_HD_ARQVODPacketsLostR2Nmb(void)
{
    return StatisticBase::s_statisticData.HD_ARQVODPacketsLostR2Nmb;
}

int tr069_statistic_get_HD_ARQVODPacketsLostR3Nmb(void)
{
    return StatisticBase::s_statisticData.HD_ARQVODPacketsLostR3Nmb;
}

int tr069_statistic_get_HD_ARQVODPacketsLostR4Nmb(void)
{
    return StatisticBase::s_statisticData.HD_ARQVODPacketsLostR4Nmb;
}

int tr069_statistic_get_HD_ARQVODPacketsLostR5Nmb(void)
{
    return StatisticBase::s_statisticData.HD_ARQVODPacketsLostR5Nmb;
}

int tr069_statistic_get_HD_VODBitRateR1Nmb(void)
{
    return StatisticBase::s_statisticData.HD_VODBitRateR1Nmb;
}

int tr069_statistic_get_HD_VODBitRateR2Nmb(void)
{
    return StatisticBase::s_statisticData.HD_VODBitRateR2Nmb;
}

int tr069_statistic_get_HD_VODBitRateR3Nmb(void)
{
    return StatisticBase::s_statisticData.HD_VODBitRateR3Nmb;
}

int tr069_statistic_get_HD_VODBitRateR4Nmb(void)
{
    return StatisticBase::s_statisticData.HD_VODBitRateR4Nmb;
}

int tr069_statistic_get_HD_VODBitRateR5Nmb(void)
{
    return StatisticBase::s_statisticData.HD_VODBitRateR5Nmb;
}

int tr069_statistic_get_HD_MultiBitRateR1Nmb(void)
{
    return StatisticBase::s_statisticData.HD_MultiBitRateR1Nmb;
}

int tr069_statistic_get_HD_MultiBitRateR2Nmb(void)
{
    return StatisticBase::s_statisticData.HD_MultiBitRateR2Nmb;
}

int tr069_statistic_get_HD_MultiBitRateR3Nmb(void)
{
    return StatisticBase::s_statisticData.HD_MultiBitRateR3Nmb;
}

int tr069_statistic_get_HD_MultiBitRateR4Nmb(void)
{
    return StatisticBase::s_statisticData.HD_MultiBitRateR4Nmb;
}

int tr069_statistic_get_HD_MultiBitRateR5Nmb(void)
{
    return StatisticBase::s_statisticData.HD_MultiBitRateR5Nmb;
}

int tr069_statistic_get_BufferIncNmb(void)
{
    return Hippo::UltraPlayer::s_statistic.getBufferIncNmb();
}

int tr069_statistic_get_HTTPRRT(void)
{
    return StatisticBase::s_statisticData.HTTPRRT;
}

int tr069_statistic_get_BufferDecNmb(void)
{
    return Hippo::UltraPlayer::s_statistic.getBufferDecNmb();
}

int tr069_statistic_get_FramesLostR1Nmb(void)
{
    return StatisticBase::s_statisticData.FrameLostR1;
}

int tr069_statistic_get_FramesLostR2Nmb(void)
{
    return StatisticBase::s_statisticData.FrameLostR2;
}

int tr069_statistic_get_FramesLostR3Nmb(void)
{
    return StatisticBase::s_statisticData.FrameLostR3;
}

int tr069_statistic_get_FramesLostR4Nmb(void)
{
    return StatisticBase::s_statisticData.FrameLostR4;
}

int tr069_statistic_get_FramesLostR5Nmb(void)
{
    return StatisticBase::s_statisticData.FrameLostR5;
}


void  tr069_statistic_set_HD_BitRateR1(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.HD_BitRateR1 != value) {
        StatisticBase::s_statisticCfg.HD_BitRateR1 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_HD_BitRateR2(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.HD_BitRateR2 != value) {
        StatisticBase::s_statisticCfg.HD_BitRateR2 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_HD_BitRateR3(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.HD_BitRateR3 != value) {
        StatisticBase::s_statisticCfg.HD_BitRateR3 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_HD_BitRateR4(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.HD_BitRateR4 != value) {
        StatisticBase::s_statisticCfg.HD_BitRateR4 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_HD_BitRateR5(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.HD_BitRateR5 != value) {
        StatisticBase::s_statisticCfg.HD_BitRateR5 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}
#if 0 // not used
int app_tr069_port_get_StatInterval(void)
{
    unsigned int statInterval, packetsLostInterval, bitRateInterval, framLostInterval;

    packetsLostInterval = StatisticBase::s_statisticCfg.PacketsLostInterval & 0xff;
    bitRateInterval = StatisticBase::s_statisticCfg.BitRateInterval & 0xff;
    framLostInterval = StatisticBase::s_statisticCfg.FramLostInterval &0xff;
    //statInterval = packetsLostInterval << 16 + packetsLostInterval << 8 + bitRateInterval;
    statInterval = (packetsLostInterval << 16) + (framLostInterval << 8) + bitRateInterval;
    return statInterval;
}
#endif 
int tr069_statistic_get_PacketsLostR1(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR1;
}

int tr069_statistic_get_PacketsLostR2(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR2;
}

int tr069_statistic_get_PacketsLostR3(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR3;
}

int tr069_statistic_get_PacketsLostR4(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR4;
}

int tr069_statistic_get_PacketsLostR5(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR5;
}

int tr069_statistic_get_HD_PacketsLostR1(void)
{
    return StatisticBase::s_statisticCfg.HD_PacketsLostR1;
}

int tr069_statistic_get_HD_PacketsLostR2(void)
{
    return StatisticBase::s_statisticCfg.HD_PacketsLostR2;
}

int tr069_statistic_get_HD_PacketsLostR3(void)
{
    return StatisticBase::s_statisticCfg.HD_PacketsLostR3;
}

int tr069_statistic_get_HD_PacketsLostR4(void)
{
    return StatisticBase::s_statisticCfg.HD_PacketsLostR4;
}

int tr069_statistic_get_HD_PacketsLostR5(void)
{
    return StatisticBase::s_statisticCfg.HD_PacketsLostR5;
}

int tr069_statistic_get_BitRateRangeR1(void)
{
    return StatisticBase::s_statisticCfg.BitRateR1;
}

int tr069_statistic_get_BitRateRangeR2(void)
{
    return StatisticBase::s_statisticCfg.BitRateR2;
}

int tr069_statistic_get_BitRateRangeR3(void)
{
    return StatisticBase::s_statisticCfg.BitRateR3;
}

int tr069_statistic_get_BitRateRangeR4(void)
{
    return StatisticBase::s_statisticCfg.BitRateR4;
}

int tr069_statistic_get_BitRateRangeR5(void)
{
    return StatisticBase::s_statisticCfg.BitRateR5;
}

int tr069_statistic_get_FramesLostR1(void)
{
    return StatisticBase::s_statisticCfg.FrameLostR1;
}

int tr069_statistic_get_FramesLostR2(void)
{
    return StatisticBase::s_statisticCfg.FrameLostR2;
}

int tr069_statistic_get_FramesLostR3(void)
{
    return StatisticBase::s_statisticCfg.FrameLostR3;
}

int tr069_statistic_get_FramesLostR4(void)
{
    return StatisticBase::s_statisticCfg.FrameLostR4;
}

int tr069_statistic_get_FramesLostR5(void)
{
    return StatisticBase::s_statisticCfg.FrameLostR5;
}
#if 0 // not used
void  app_tr069_port_set_StatInterval(int value)
{
    unsigned int statInterval, packetsLostInterval, bitRateInterval;
    unsigned int framLostInterval  = 0;

    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return ;

    statInterval = value;
    packetsLostInterval = (statInterval >> 16) & 0xff;
    framLostInterval = (statInterval >>8) &0xff;
    bitRateInterval = statInterval & 0xff;

    if (packetsLostInterval != StatisticBase::s_statisticCfg.PacketsLostInterval || bitRateInterval != StatisticBase::s_statisticCfg.BitRateInterval) {
        StatisticBase::s_statisticCfg.PacketsLostInterval = packetsLostInterval;
        StatisticBase::s_statisticCfg.BitRateInterval = bitRateInterval;
        StatisticBase::s_statisticCfg.FramLostInterval = framLostInterval;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}
#endif 
void  tr069_statistic_set_PacketsLostR1(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.PacketsLostR1 != value) {
        StatisticBase::s_statisticCfg.PacketsLostR1 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void tr069_statistic_set_PacketsLostR2(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.PacketsLostR2 != value) {
        StatisticBase::s_statisticCfg.PacketsLostR2 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_PacketsLostR3(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.PacketsLostR3 != value) {
        StatisticBase::s_statisticCfg.PacketsLostR3 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void tr069_statistic_set_PacketsLostR4(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.PacketsLostR4 != value) {
        StatisticBase::s_statisticCfg.PacketsLostR4 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_PacketsLostR5(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.PacketsLostR5 != value) {
        StatisticBase::s_statisticCfg.PacketsLostR5 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void tr069_statistic_set_HD_PacketsLostR1(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.HD_PacketsLostR1 != value) {
        StatisticBase::s_statisticCfg.HD_PacketsLostR1 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void tr069_statistic_set_HD_PacketsLostR2(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.HD_PacketsLostR2 != value) {
        StatisticBase::s_statisticCfg.HD_PacketsLostR2 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_HD_PacketsLostR3(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.HD_PacketsLostR3 != value) {
        StatisticBase::s_statisticCfg.HD_PacketsLostR3 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_HD_PacketsLostR4(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.HD_PacketsLostR4 != value) {
        StatisticBase::s_statisticCfg.HD_PacketsLostR4 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_HD_PacketsLostR5(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.HD_PacketsLostR5 != value) {
        StatisticBase::s_statisticCfg.HD_PacketsLostR5 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_BitRateRangeR1(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.BitRateR1 != value) {
        StatisticBase::s_statisticCfg.BitRateR1 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_BitRateRangeR2(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.BitRateR2 != value) {
        StatisticBase::s_statisticCfg.BitRateR2 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_BitRateRangeR3(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.BitRateR3 != value) {
        StatisticBase::s_statisticCfg.BitRateR3 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_BitRateRangeR4(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.BitRateR4 != value) {
        StatisticBase::s_statisticCfg.BitRateR4 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_BitRateRangeR5(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.BitRateR5 != value) {
        StatisticBase::s_statisticCfg.BitRateR5 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_FramesLostR1(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.FrameLostR1 != value) {
        StatisticBase::s_statisticCfg.FrameLostR1 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_FramesLostR2(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.FrameLostR2 != value) {
        StatisticBase::s_statisticCfg.FrameLostR2 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_FramesLostR3(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.FrameLostR3 != value) {
        StatisticBase::s_statisticCfg.FrameLostR3 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_FramesLostR4(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.FrameLostR4) {
        StatisticBase::s_statisticCfg.FrameLostR4 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void  tr069_statistic_set_FramesLostR5(int value)
{
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.FrameLostR5 != value) {
        StatisticBase::s_statisticCfg.FrameLostR5 = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------
    丢包率、丢帧率、媒体流带宽统计周期
    单位：s 默认：10s
 ------------------------------------------------------------------------------*/
int app_tr069_port_get_MonitoringInterval(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostInterval;
}

/*------------------------------------------------------------------------------
    丢包率范围1起始
    以0.01%为单位
    例如： 1 表示0.01%
    5 表示0.05%
    默认值：0
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR1From(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR1From;
}

/*------------------------------------------------------------------------------
    丢包率范围1结束
    以0.01%为单位
    例如： 1 表示0.01%
    5 表示0.05%
    9999 表示最大
    默认值：0
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR1Till(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR1Till;
}

/*------------------------------------------------------------------------------
    丢包率范围2－起始
    以0.01%为单位
    默认值：0
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR2From(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR2From;
}

/*------------------------------------------------------------------------------
    丢包率范围2－结束
    以0.01%为单位
    默认值：10
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR2Till(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR2Till;
}

/*------------------------------------------------------------------------------
    丢包率范围3－起始
    以0.01%为单位
    默认值：10
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR3From(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR3From;
}

/*------------------------------------------------------------------------------
    丢包率范围3－结束
    以0.01%为单位
    默认值：20
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR3Till(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR3Till;
}

/*------------------------------------------------------------------------------
    丢包率范围4－起始
    以0.01%为单位
    默认值：20
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR4From(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR4From;
}

/*------------------------------------------------------------------------------
    丢包率范围4－结束
    以0.01%为单位
    默认值：50
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR4Till(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR4Till;
}

/*------------------------------------------------------------------------------
    丢包率范围5－起始
    以0.01%为单位
    默认值：50
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR5From(void)
{
    int temp = 0;

    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return -1;
    temp = StatisticBase::s_statisticCfg.PacketsLostR5From;
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return temp;
}

/*------------------------------------------------------------------------------
    丢包率范围5－结束
    以0.01%为单位
    默认值：9999
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR5Till(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR5Till;
}

/*------------------------------------------------------------------------------
    实时媒体速率范围1－起始
    以100kbps为单位
    例如：10 表示1Mbps
    13 表示1.3Mbps
    默认值：16
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR1From(void)
{
    return StatisticBase::s_statisticCfg.BitRateR1From;
}

/*------------------------------------------------------------------------------
    实时媒体速率范围1－结束
    以100kbps为单位
    例如： 10 表示1Mbps
    13 表示1.3Mbps
    9999 表示不限速
    默认值：9999
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR1Till(void)
{
    return StatisticBase::s_statisticCfg.BitRateR1Till;
}

/*------------------------------------------------------------------------------
    实时媒体速率范围2－起始
    以100kbps为单位
    默认值：14
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR2From(void)
{
    return StatisticBase::s_statisticCfg.BitRateR2From;
}

/*------------------------------------------------------------------------------
    实时媒体速率范围2－结束
    以100kbps为单位
    默认值：16
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR2Till(void)
{
    return StatisticBase::s_statisticCfg.BitRateR2Till;
}

/*------------------------------------------------------------------------------
    实时媒体速率范围3－起始
    以100kbps为单位
    默认值：12
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR3From(void)
{
    return StatisticBase::s_statisticCfg.BitRateR3From;
}

/*------------------------------------------------------------------------------
    实时媒体速率范围3－结束
    以100kbps为单位
    默认值：14
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR3Till(void)
{
    return StatisticBase::s_statisticCfg.BitRateR3Till;
}

/*------------------------------------------------------------------------------
    实时媒体速率范围4－起始
    以100kbps为单位
    默认值：8
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR4From(void)
{
    return StatisticBase::s_statisticCfg.BitRateR4From;
}

/*------------------------------------------------------------------------------
    实时媒体速率范围4－结束
    以100kbps为单位
    默认值：12
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR4Till(void)
{
    return StatisticBase::s_statisticCfg.BitRateR4Till;
}

/*------------------------------------------------------------------------------
    实时媒体速率范围5－起始
    以100kbps为单位
    默认值：0
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR5From(void)
{
    return StatisticBase::s_statisticCfg.BitRateR5From;
}

/*------------------------------------------------------------------------------
    实时媒体速率范围5－结束
    以100kbps为单位
    默认值：8
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR5Till(void)
{
    return StatisticBase::s_statisticCfg.BitRateR5Till;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_statistic_set_LogUploadInterval(int value)
{
    LogUserOperDebug("value(%d)\n", value);
    if(value < 0)
        return;

    tr069StatisticSetLogUploadInterval(value);
    StatisticBase::s_statisticCfg.dirty = 1;

#if defined(Huawei_v5)
    TR069QosLogSetInterval(value);
#else
    if(value == 0) {
        mid_timer_delete((mid_timer_f)tr069StatisticPost, 0);
        mid_timer_delete((mid_timer_f)tr069StatisticPeriodStart, 0);
    }
    if (StatisticBase::s_startFlag == 1) {
        mid_timer_delete((mid_timer_f)tr069StatisticPost, 0);
        tr069StatisticPost(0);
        mid_timer_create(value, 0, (mid_timer_f)tr069StatisticPost, 0);// zhangl test value
    }
 #endif
    return;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_statistic_set_LogRecordInterval(int value)
{
    int flag = 0;

    if(value < 0)
        return;
    if(value == 0) {
        mid_timer_delete((mid_timer_f)tr069StatisticPost, 0);
        mid_timer_delete((mid_timer_f)tr069StatisticPeriodStart, 0);
    }

    if(tr069StatisticGetLogRecordInterval() != value)
        flag = 1;
    tr069StatisticSetLogRecordInterval(value);
    StatisticBase::s_statisticCfg.dirty = 1;

    if(StatisticBase::s_startFlag == 1 && flag == 1) {
        mid_timer_delete((mid_timer_f)tr069StatisticPeriodStart, 0);
        tr069StatisticPeriodStart(0);
        mid_timer_create(value, 0, (mid_timer_f)tr069StatisticPeriodStart, 0);// zhangl test value
    }
    return;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_statistic_set_PacketsLostR1From(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.PacketsLostR1From != value) {
        StatisticBase::s_statisticCfg.PacketsLostR1From = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_statistic_set_PacketsLostR1Till(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;
    if(StatisticBase::s_statisticCfg.PacketsLostR1Till != value) {
        StatisticBase::s_statisticCfg.PacketsLostR1Till = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_statistic_set_PacketsLostR2From(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.PacketsLostR2From != value) {
        StatisticBase::s_statisticCfg.PacketsLostR2From = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
void tr069_statistic_set_PacketsLostR2Till(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.PacketsLostR2Till != value) {
        StatisticBase::s_statisticCfg.PacketsLostR2Till = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
void tr069_statistic_set_PacketsLostR3From(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.PacketsLostR3From != value) {
        StatisticBase::s_statisticCfg.PacketsLostR3From = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
void tr069_statistic_set_PacketsLostR3Till(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.PacketsLostR3Till != value) {
        StatisticBase::s_statisticCfg.PacketsLostR3Till = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
void tr069_statistic_set_PacketsLostR4From(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.PacketsLostR4From != value) {
        StatisticBase::s_statisticCfg.PacketsLostR4From = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
void tr069_statistic_set_PacketsLostR4Till(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;
    if(StatisticBase::s_statisticCfg.PacketsLostR4Till != value) {
        StatisticBase::s_statisticCfg.PacketsLostR4Till = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
void tr069_statistic_set_PacketsLostR5From(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.PacketsLostR5From != value) {
        StatisticBase::s_statisticCfg.PacketsLostR5From = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_statistic_set_PacketsLostR5Till(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.PacketsLostR5Till != value) {
        StatisticBase::s_statisticCfg.PacketsLostR5Till = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_statistic_set_BitRateR1From(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.BitRateR1From != value) {
        StatisticBase::s_statisticCfg.BitRateR1From = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_statistic_set_BitRateR1Till(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.BitRateR1Till != value) {
        StatisticBase::s_statisticCfg.BitRateR1Till = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_statistic_set_BitRateR2From(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.BitRateR2From != value) {
        StatisticBase::s_statisticCfg.BitRateR2From = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
void tr069_statistic_set_BitRateR2Till(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.BitRateR2Till != value) {
        StatisticBase::s_statisticCfg.BitRateR2Till = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_statistic_set_BitRateR3From(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.BitRateR3From != value) {
        StatisticBase::s_statisticCfg.BitRateR3From = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------
 ------------------------------------------------------------------------------*/
void tr069_statistic_set_BitRateR3Till(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.BitRateR3Till != value) {
        StatisticBase::s_statisticCfg.BitRateR3Till = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_statistic_set_BitRateR4From(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.BitRateR4From != value) {
        StatisticBase::s_statisticCfg.BitRateR4From = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_statistic_set_BitRateR4Till(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.BitRateR4Till != value) {
        StatisticBase::s_statisticCfg.BitRateR4Till = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_statistic_set_BitRateR5From(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if(StatisticBase::s_statisticCfg.BitRateR5From != value) {
        StatisticBase::s_statisticCfg.BitRateR5From = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_statistic_set_BitRateR5Till(int value)
{
    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    if (StatisticBase::s_statisticCfg.BitRateR5Till != value) {
        StatisticBase::s_statisticCfg.BitRateR5Till = value;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

/*------------------------------------------------------------------------------
    起始时间
    格式：yyyymmddhhmmss
 ------------------------------------------------------------------------------*/
void tr069_statistic_get_Startpoint(char *value, int size)
{
    unsigned int sec = statisticLocakTime( ) - (mid_10ms( ) - StatisticBase::s_startClock) / 100;
    mid_tool_time2string(sec, value, 0);
}

/*------------------------------------------------------------------------------
    结束时间
    格式：yyyymmddhhmmss
 ------------------------------------------------------------------------------*/
void tr069_statistic_get_Endpoint(char *value, int size)
{
    unsigned int sec = statisticLocakTime( );
    mid_tool_time2string(sec, value, 0);
    return;
}

/*------------------------------------------------------------------------------
    认证的总次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_AuthNumbers(void)
{
    StatisticBase::s_statisticData.AuthNumbers = jseAuthCountGet();
    return StatisticBase::s_statisticData.AuthNumbers ? StatisticBase::s_statisticData.AuthNumbers : 1;
}

int tr069_statistic_get_BootUpTime(void)
{
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)
    return StatisticBase::s_bootUptime;
#else
    return StatisticBase::s_statisticData.BootUptime;
#endif
}

void tr069_statistic_get_PowerTime(char *value, int size)
{
    if(value) {
        if(strlen(StatisticBase::s_powerOnTime) >= size)
            value[0] = 0;
        else
            strcpy(value, StatisticBase::s_powerOnTime);
    }
    return;
}

/*------------------------------------------------------------------------------
    认证失败的总次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_AuthFailNumbers(void)
{
    StatisticBase::s_statisticData.AuthFailNumbers = jseAuthFailCountGet();
    return StatisticBase::s_statisticData.AuthFailNumbers;
}

/*------------------------------------------------------------------------------
    认证失败详细信息
    每次失败，需要另起一行记录认证返回的错误码及相关信息
 ------------------------------------------------------------------------------*/
void tr069_statistic_get_AuthFailInfo(char *value, int size)
{
    if(value) {
        if(pthread_mutex_lock(StatisticBase::s_mutex))
            return;
        StatisticBase::s_statisticData.AuthFailNumbers = jseAuthFailCountGet();
        jseAuthFailInfoGet(StatisticBase::s_statisticData.AuthFailInfo, STATISTIC_INFO_FAID_NUM * STATISTIC_INFO_FAID_SIZE); //zhangl
        if(StatisticBase::s_statisticData.AuthFailNumbers > 0)
            strcpy(value, StatisticBase::s_statisticData.AuthFailInfo[(StatisticBase::s_statisticData.AuthFailNumbers - 1) % STATISTIC_INFO_FAID_NUM]);
        else
            value[0] = 0;
        pthread_mutex_unlock(StatisticBase::s_mutex);
    }
    return;
}

/*------------------------------------------------------------------------------
    加入组播组的总次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_MultiReqNumbers(void)
{
    return Hippo::UltraPlayerMultiple::s_statistic.getRequestNum() ? Hippo::UltraPlayerMultiple::s_statistic.getRequestNum() : 1;
}

/*------------------------------------------------------------------------------
    加入组播组失败（加入后没有收到组播数据）的总次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_MultiFailNumbers(void)
{
    return Hippo::UltraPlayerMultiple::s_statistic.getFailedNum();
}

/*------------------------------------------------------------------------------
    加入组播组失败（加入后没有收到组播数据）的信息，即要加入的组播频道地址
    每次失败，需要另起一行记录失败的组播频道信息
 ------------------------------------------------------------------------------*/
void tr069_statistic_get_MultiFailInfo(char *value, int size)
{
    if(value) {
        if(pthread_mutex_lock(Hippo::UltraPlayer::s_statistic.getMutex()))
            return;
        if(Hippo::UltraPlayerMultiple::s_statistic.getFailedNum() > 0)
            strcpy(value, Hippo::UltraPlayerMultiple::s_statistic.getFailedInfo((Hippo::UltraPlayerMultiple::s_statistic.getFailedNum() - 1) % STATISTIC_INFO_FAID_NUM));
        else
            value[0] = 0;
        pthread_mutex_unlock(Hippo::UltraPlayer::s_statistic.getMutex());
    }
    return;
}

/*------------------------------------------------------------------------------
    单播（含点播、回看和时移）申请的总次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_VodReqNumbers(void)
{
    return Hippo::UltraPlayerVod::s_statistic.getRequestNum();
}

/*------------------------------------------------------------------------------
    单播（含点播、回看和时移）申请失败的总次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_VodFailNumbers(void)
{
    return Hippo::UltraPlayerVod::s_statistic.getFailedNum();
}

/*------------------------------------------------------------------------------
    单播（含点播、回看和时移）申请失败详细信息
    每次失败，需要另起一行记录返回的错误码及失败的点播、回看和时移的URL信息
    格式举例：RTSP://…./xx.mpg/
 ------------------------------------------------------------------------------*/
void tr069_statistic_get_VodFailInfo(char *value, int size)
{
    if(value) {
        if(pthread_mutex_lock(Hippo::UltraPlayer::s_statistic.getMutex()))
            return;
        if(Hippo::UltraPlayerVod::s_statistic.getFailedNum() > 0)
            strcpy(value, Hippo::UltraPlayerVod::s_statistic.getFailedInfo((Hippo::UltraPlayerVod::s_statistic.getFailedNum() - 1) % STATISTIC_INFO_FAID_NUM));
        else
            value[0] = 0;
        pthread_mutex_unlock(Hippo::UltraPlayer::s_statistic.getMutex());
    }
    return;
}

/*------------------------------------------------------------------------------
    Http请求的总次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_HTTPReqNumbers(void)
{
    StatisticBase::s_statisticData.HTTPReqNumbers = BrowserStatisticHTTPReqNumbersGet();
    return StatisticBase::s_statisticData.HTTPReqNumbers;
}

int tr069_statistic_set_HTTPReqNumbers(void)
{
    StatisticBase::s_statisticData.HTTPReqNumbers++;
    return 0;
}

/*------------------------------------------------------------------------------
    Http请求失败的总次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_HTTPFailNumbers(void)
{
    StatisticBase::s_statisticData.HTTPFailNumbers = BrowserStatisticHTTPFailNumbersGet();
    return StatisticBase::s_statisticData.HTTPFailNumbers;
}

int tr069_statistic_set_HTTPFailNumbers(void)
{
    StatisticBase::s_statisticData.HTTPFailNumbers++;
    return 0;
}

/*------------------------------------------------------------------------------
    Http请求失败详细信息
    每次失败，需要另起一行记录返回的错误码及失败的URL
    说明：机顶盒只记录但前请求失败的页面URL，不记录该页面包含的所有URL
 ------------------------------------------------------------------------------*/
void tr069_statistic_get_HTTPFailInfo(char *value, int size)
{
    if(value) {
        if(pthread_mutex_lock(StatisticBase::s_mutex))
            return;
        StatisticBase::s_statisticData.HTTPFailNumbers = BrowserStatisticHTTPFailNumbersGet();
        BrowserStatisticHTTPFailInfoGet(StatisticBase::s_statisticData.HTTPFailInfo, STATISTIC_INFO_FAID_NUM * STATISTIC_INFO_FAID_SIZE);
        if(StatisticBase::s_statisticData.HTTPFailNumbers > 0)
            strcpy(value, StatisticBase::s_statisticData.HTTPFailInfo[(StatisticBase::s_statisticData.HTTPFailNumbers - 1) % STATISTIC_INFO_FAID_NUM]);
        else
            value[0] = 0;
        pthread_mutex_unlock(StatisticBase::s_mutex);
    }
    return;
}

/*------------------------------------------------------------------------------
    异常断流的次数
    异常断流定义为：在视频播放过程中（包括组播和单播），因为缓冲区被取空导致无法再播放的次数。
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_AbendNumbers(void)
{
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
#else
    return Hippo::UltraPlayer::s_statistic.getStreamGapNumbers();
#endif
}

/*------------------------------------------------------------------------------
    异常断流的详细信息
 ------------------------------------------------------------------------------*/
void tr069_statistic_get_AbendInfo(char *value, int size)
{

#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
#else
    if(value) {
        if(pthread_mutex_lock(Hippo::UltraPlayer::s_statistic.getMutex()))
            return;

        if(Hippo::UltraPlayer::s_statistic.getStreamGapNumbers() > 0)
            strcpy(value, Hippo::UltraPlayer::s_statistic.getStreamGapEvent((Hippo::UltraPlayer::s_statistic.getStreamGapNumbers() - 1) % STATISTIC_INFO_FAID_NUM));
        else
            value[0] = 0;
        pthread_mutex_unlock(Hippo::UltraPlayer::s_statistic.getMutex());
    }
    return;
#endif
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
int tr069_statistic_get_AbendDurationTotal(void)
{
    int total;

    if(pthread_mutex_lock(Hippo::UltraPlayer::s_statistic.getMutex()))
        return -1;

    total = Hippo::UltraPlayer::s_statistic.app_abend_duration();
    total += Hippo::UltraPlayer::s_statistic.getTotalAbendDuration();
    pthread_mutex_unlock(Hippo::UltraPlayer::s_statistic.getMutex());

    return total;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
int tr069_statistic_get_AbendDurationMax(void)
{
    int dmax;
    if(pthread_mutex_lock(Hippo::UltraPlayer::s_statistic.getMutex()))
        return -1;

    Hippo::UltraPlayer::s_statistic.app_abend_duration();
    dmax = Hippo::UltraPlayer::s_statistic.getAbendDurationMax();
    pthread_mutex_unlock(Hippo::UltraPlayer::s_statistic.getMutex());

    return dmax;

}

/*------------------------------------------------------------------------------
    组播丢包率范围1发生次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_MultiPacketsLostR1(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR1;
}

/*------------------------------------------------------------------------------
    组播丢包率范围2发生次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_MultiPacketsLostR2(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR2;
}

/*------------------------------------------------------------------------------
    组播丢包率范围3发生次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_MultiPacketsLostR3(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR3;
}

/*------------------------------------------------------------------------------
    组播丢包率范围4发生次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_MultiPacketsLostR4(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR4;
}

/*------------------------------------------------------------------------------
    组播丢包率范围5发生次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_MultiPacketsLostR5(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR5;
}

/*------------------------------------------------------------------------------
    单播丢包率范围1发生次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_VODPacketsLostR1(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR1;
}

/*------------------------------------------------------------------------------
    单播丢包率范围2发生次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_VODPacketsLostR2(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR2;
}

/*------------------------------------------------------------------------------
    单播丢包率范围3发生次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_VODPacketsLostR3(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR3;
}

/*------------------------------------------------------------------------------
    单播丢包率范围4发生次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_VODPacketsLostR4(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR4;
}

/*------------------------------------------------------------------------------
    单播丢包率范围5发生次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_VODPacketsLostR5(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR5;
}

/*------------------------------------------------------------------------------
    比特率在BitRateR1范围内的次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR1(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR1Nmb + StatisticBase::s_statisticData.VODBitRateR1Nmb;
}

/*------------------------------------------------------------------------------
    比特率在BitRateR2范围内的次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR2(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR2Nmb + StatisticBase::s_statisticData.VODBitRateR2Nmb;
}

/*------------------------------------------------------------------------------
    比特率在BitRateR3范围内的次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR3(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR3Nmb + StatisticBase::s_statisticData.VODBitRateR3Nmb;
}

/*------------------------------------------------------------------------------
    比特率在BitRateR4范围内的次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR4(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR4Nmb + StatisticBase::s_statisticData.VODBitRateR4Nmb;
}

/*------------------------------------------------------------------------------
    比特率在BitRateR5范围内的次数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR5(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR5Nmb + StatisticBase::s_statisticData.VODBitRateR5Nmb;
}

/*------------------------------------------------------------------------------
    在一个采样周期内的丢包数
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLost(void)
{
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
#else
    return StatisticBase::s_statisticData.PacketsLost;
#endif
}

/*------------------------------------------------------------------------------
    在一个采样周期内的最大抖动值，单位：ms
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_JitterMax(void)
{
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
#else
    return StatisticBase::s_statisticData.JitterMax;
#endif
}

/*------------------------------------------------------------------------------
    当前正在播放帧率
    如果无，则不记录；
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_FrameRate(void)
{
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
#else
    return StatisticBase::s_statisticData.FrameRate;
#endif
}

/*------------------------------------------------------------------------------
    累积丢帧数
    如果无，则不记录；
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_FrameLoss(void)
{
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
#else
    return StatisticBase::s_statisticData.FrameLoss;
#endif
}



static void int_setTimer(int x)
{
    ////LogTr069Debug("dirty = %d\n", StatisticBase::s_statisticCfg.dirty);
    tr069StatisticConfigSave( );
}

static void int_setStatInterval(int value)
{
    unsigned int statInterval, packetsLostInterval, bitRateInterval;
    unsigned int framLostInterval  = 0;
    if (pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    statInterval = value;
    packetsLostInterval = (statInterval >> 16) & 0xff;
    framLostInterval = (statInterval >>8) &0xff;
    bitRateInterval = statInterval & 0xff;

    if (packetsLostInterval != StatisticBase::s_statisticCfg.PacketsLostInterval || bitRateInterval != StatisticBase::s_statisticCfg.BitRateInterval) {
        StatisticBase::s_statisticCfg.PacketsLostInterval = packetsLostInterval;
        StatisticBase::s_statisticCfg.BitRateInterval = bitRateInterval;
        StatisticBase::s_statisticCfg.FramLostInterval = framLostInterval;
        StatisticBase::s_statisticCfg.dirty = 1;
    }
    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

#ifdef TR069_VERSION_1

#else
int tr069_statistic_setConfiguration(char *name, char *str, unsigned int x)
{
    int val, ret = 0;
    if (!strcmp(name, "LogServerUrl")) {
        tr069StatisticSetLogServerUrl(str);
        goto End;
    }

    val = atoi(str);
    if (val < 0) {
        ("name = %s, value = %s\n", name, str);
        return -1;
    }
    if (!strcmp(name, "LogUploadInterval")) {
        tr069_statistic_set_LogUploadInterval(val);
        goto End;
    }
    if (!strcmp(name, "LogRecordInterval")) {
        tr069_statistic_set_LogRecordInterval(val);
        goto End;
    }
    if (!strcmp(name, "StatInterval")) {
        int_setStatInterval(val);
        goto End;
    }

    pthread_mutex_lock(StatisticBase::s_mutex);

    if (!strcmp(name, "Logenable")) {
        tr069StatisticSetLogenable(val); //getStatisticLog()->setEnable(val);
        StatisticBase::s_statisticCfg.dirty = 1;
    } else if (!strcmp(name, "IsFileorRealTime")) {
        ;
    } else if (!strcmp(name, "MonitoringInterval")) {
        StatisticBase::s_statisticCfg.PacketsLostInterval = val;
        StatisticBase::s_statisticCfg.dirty = 1;
    } else if (!strcmp(name, "PacketsLostR1")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR1 != val) {
            StatisticBase::s_statisticCfg.PacketsLostR1 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "PacketsLostR2")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR2 != val) {
            StatisticBase::s_statisticCfg.PacketsLostR2 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "PacketsLostR3")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR3 != val) {
            StatisticBase::s_statisticCfg.PacketsLostR3 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "PacketsLostR4")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR4 != val) {
            StatisticBase::s_statisticCfg.PacketsLostR4 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "PacketsLostR5")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR5 != val) {
            StatisticBase::s_statisticCfg.PacketsLostR5 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }

    } else if (!strcmp(name, "HD_PacketsLostR1")) {
        if(StatisticBase::s_statisticCfg.HD_PacketsLostR1 != val) {
            StatisticBase::s_statisticCfg.HD_PacketsLostR1 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "HD_PacketsLostR2")) {
        if(StatisticBase::s_statisticCfg.HD_PacketsLostR2 != val) {
            StatisticBase::s_statisticCfg.HD_PacketsLostR2 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "HD_PacketsLostR3")) {
        if(StatisticBase::s_statisticCfg.HD_PacketsLostR3 != val) {
            StatisticBase::s_statisticCfg.HD_PacketsLostR3 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "HD_PacketsLostR4")) {
        if(StatisticBase::s_statisticCfg.HD_PacketsLostR4 != val) {
            StatisticBase::s_statisticCfg.HD_PacketsLostR4 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "HD_PacketsLostR5")) {
        if(StatisticBase::s_statisticCfg.HD_PacketsLostR5 != val) {
            StatisticBase::s_statisticCfg.HD_PacketsLostR5 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }

    } else if (!strcmp(name, "BitRateR1")) {
        if(StatisticBase::s_statisticCfg.BitRateR1 != val) {
            StatisticBase::s_statisticCfg.BitRateR1 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "BitRateR2")) {
        if(StatisticBase::s_statisticCfg.BitRateR2 != val) {
            StatisticBase::s_statisticCfg.BitRateR2 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "BitRateR3")) {
        if(StatisticBase::s_statisticCfg.BitRateR3 != val) {
            StatisticBase::s_statisticCfg.BitRateR3 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "BitRateR4")) {
        if(StatisticBase::s_statisticCfg.BitRateR4 != val) {
            StatisticBase::s_statisticCfg.BitRateR4 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "BitRateR5")) {
        if(StatisticBase::s_statisticCfg.BitRateR5 != val) {
            StatisticBase::s_statisticCfg.BitRateR5 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }

    } else if (!strcmp(name, "HD_BitRateR1")) {
        if(StatisticBase::s_statisticCfg.HD_BitRateR1 != val) {
            StatisticBase::s_statisticCfg.HD_BitRateR1 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "HD_BitRateR2")) {
        if(StatisticBase::s_statisticCfg.HD_BitRateR2 != val) {
            StatisticBase::s_statisticCfg.HD_BitRateR2 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "HD_BitRateR3")) {
        if(StatisticBase::s_statisticCfg.HD_BitRateR3 != val) {
            StatisticBase::s_statisticCfg.HD_BitRateR3 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "HD_BitRateR4")) {
        if(StatisticBase::s_statisticCfg.HD_BitRateR4 != val) {
            StatisticBase::s_statisticCfg.HD_BitRateR4 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "HD_BitRateR5")) {
        if(StatisticBase::s_statisticCfg.HD_BitRateR5 != val) {
            StatisticBase::s_statisticCfg.HD_BitRateR5 = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }

    } else if (!strcmp(name, "PacketsLostR1From")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR1From != val) {
            StatisticBase::s_statisticCfg.PacketsLostR1From = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "PacketsLostR1Till")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR1Till != val) {
            StatisticBase::s_statisticCfg.PacketsLostR1Till = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "PacketsLostR2From")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR2From != val) {
            StatisticBase::s_statisticCfg.PacketsLostR2From = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "PacketsLostR2Till")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR2Till != val) {
            StatisticBase::s_statisticCfg.PacketsLostR2Till = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "PacketsLostR3From")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR3From != val) {
            StatisticBase::s_statisticCfg.PacketsLostR3From = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "PacketsLostR3Till")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR3Till != val) {
            StatisticBase::s_statisticCfg.PacketsLostR3Till = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "PacketsLostR4From")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR4From != val) {
            StatisticBase::s_statisticCfg.PacketsLostR4From = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "PacketsLostR4Till")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR4Till != val) {
            StatisticBase::s_statisticCfg.PacketsLostR4Till = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "PacketsLostR5From")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR5From != val) {
            StatisticBase::s_statisticCfg.PacketsLostR5From = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "PacketsLostR5Till")) {
        if(StatisticBase::s_statisticCfg.PacketsLostR5Till != val) {
            StatisticBase::s_statisticCfg.PacketsLostR5Till = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }

    } else if (!strcmp(name, "BitRateR1From")) {
        if(StatisticBase::s_statisticCfg.BitRateR1From != val) {
            StatisticBase::s_statisticCfg.BitRateR1From = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "BitRateR1Till")) {
        if(StatisticBase::s_statisticCfg.BitRateR1Till != val) {
            StatisticBase::s_statisticCfg.BitRateR1Till = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "BitRateR2From")) {
        if(StatisticBase::s_statisticCfg.BitRateR2From != val) {
            StatisticBase::s_statisticCfg.BitRateR2From = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "BitRateR2Till")) {
        if(StatisticBase::s_statisticCfg.BitRateR2Till != val) {
            StatisticBase::s_statisticCfg.BitRateR2Till = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "BitRateR3From")) {
        if(StatisticBase::s_statisticCfg.BitRateR3From != val) {
            StatisticBase::s_statisticCfg.BitRateR3From = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "BitRateR3Till")) {
        if(StatisticBase::s_statisticCfg.BitRateR3Till != val) {
            StatisticBase::s_statisticCfg.BitRateR3Till = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "BitRateR4From")) {
        if(StatisticBase::s_statisticCfg.BitRateR4From != val) {
            StatisticBase::s_statisticCfg.BitRateR4From = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "BitRateR4Till")) {
        if(StatisticBase::s_statisticCfg.BitRateR4Till != val) {
            StatisticBase::s_statisticCfg.BitRateR4Till = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "BitRateR5From")) {
        if(StatisticBase::s_statisticCfg.BitRateR5From != val) {
            StatisticBase::s_statisticCfg.BitRateR5From = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else if (!strcmp(name, "BitRateR5Till")) {
        if(StatisticBase::s_statisticCfg.BitRateR5Till != val) {
            StatisticBase::s_statisticCfg.BitRateR5Till = val;
            StatisticBase::s_statisticCfg.dirty = 1;
        }
    } else {
        //LogTr069Error("Device.X.StatisticConfiguration. name = %s\n", name);
        ret = -1;
    }

    pthread_mutex_unlock(StatisticBase::s_mutex);

End:
    if (StatisticBase::s_statisticCfg.dirty)
        mid_timer_create(1, 1, (mid_timer_f)int_setTimer, 0);

    return ret;
}
int tr069_statistic_getConfiguration(char *name, char *str, unsigned int size)
{
    if (!strcmp(name, "Logenable")) {
        snprintf(str, size, "%d", tr069StatisticGetLogenable());
    } else if (!strcmp(name, "LogServerUrl")) {//日志文件服务器的URL信息，应该包含上传URL的鉴权信息（与机顶盒连接终端管理服务器的鉴权方式一致）
        tr069StatisticGetLogAESServerUrl(str, size);
    } else if (!strcmp(name, "AESLogServerUrl")) {
        tr069StatisticGetLogAESServerUrl(str, size);
    } else if (!strcmp(name, "LogUploadInterval")) {

/*------------------------------------------------------------------------------
    性能监测参数文件上报间隔
    单位：s 默认：3600（即1小时）
    该参数设置为0时，表示关闭性能监测参数文件上报功能
 ------------------------------------------------------------------------------*/
        snprintf(str, size, "%d", tr069StatisticGetLogUploadInterval());
    } else if (!strcmp(name, "LogRecordInterval")) {
/*------------------------------------------------------------------------------
    性能监测统计参数的记录周期时长
    单位：s 默认：3600（即1小时）
    统计起始为每次开机，到设定的统计周期时长后，启动新的统计周期。
    如不到设定的统计周期时长就关机，则结束这个周期。
    在启动新的统计周期时，应把前个周期的记录数据上传到网管平台
 ------------------------------------------------------------------------------*/
        snprintf(str, size, "%d", tr069StatisticGetLogRecordInterval());
/*------------------------------------------------------------------------------
    丢包率、丢帧率、媒体流带宽统计周期
    单位：s 默认：10s
 ------------------------------------------------------------------------------*/
    } else if (!strcmp(name, "MonitoringInterval")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostInterval);
    } else if (!strcmp(name, "IsFileorRealTime")) {
        snprintf(str, size, "%d", 0);
    } else if (!strcmp(name, "StatInterval")) {
        snprintf(str, size, "%d", int_getStatInterval( ));
    } else if (!strcmp(name, "PacketsLostR1From")) {//以0.01%为单位, 默认值：0
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR1From);
    } else if (!strcmp(name, "PacketsLostR1Till")) {//默认值：0
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR1Till);

    } else if (!strcmp(name, "PacketsLostR2From")) {//默认值：0
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR2From);
    } else if (!strcmp(name, "PacketsLostR2Till")) {//默认值：10
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR2Till);

    } else if (!strcmp(name, "PacketsLostR3From")) {//默认值：10
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR3From);
    } else if (!strcmp(name, "PacketsLostR3Till")) {//默认值：20
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR3Till);

    } else if (!strcmp(name, "PacketsLostR4From")) {//默认值：20
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR4From);
    } else if (!strcmp(name, "PacketsLostR4Till")) {//默认值：50
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR4Till);

    } else if (!strcmp(name, "PacketsLostR5From")) {//默认值：50
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR5From);
    } else if (!strcmp(name, "PacketsLostR5Till")) {//默认值：9999
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR5Till);

/*------------------------------------------------------------------------------
    实时媒体速率范围1－起始
    以100kbps为单位
    例如：10 表示1Mbps
    13 表示1.3Mbps
    默认值：16
 ------------------------------------------------------------------------------*/
    } else if (!strcmp(name, "BitRateR1From")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR1From);
    } else if (!strcmp(name, "BitRateR1Till")) {//默认值：9999
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR1Till);

    } else if (!strcmp(name, "BitRateR2From")) {//默认值：14
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR2From);
    } else if (!strcmp(name, "BitRateR2Till")) {//默认值：16
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR2Till);

    } else if (!strcmp(name, "BitRateR3From")) {//默认值：12
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR3From);
    } else if (!strcmp(name, "BitRateR3Till")) {//默认值：14
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR3Till);

    } else if (!strcmp(name, "BitRateR4From")) {//默认值：8
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR4From);
    } else if (!strcmp(name, "BitRateR4Till")) {//默认值：12
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR4Till);

    } else if (!strcmp(name, "BitRateR4From")) {//默认值：0
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR4From);
    } else if (!strcmp(name, "BitRateR5Till")) {//默认值：8
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR5Till);

    } else if (!strcmp(name, "PacketsLostR1")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR1);
    } else if (!strcmp(name, "PacketsLostR2")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR2);
    } else if (!strcmp(name, "PacketsLostR3")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR3);
    } else if (!strcmp(name, "PacketsLostR4")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR4);
    } else if (!strcmp(name, "PacketsLostR5")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR5);

    } else if (!strcmp(name, "FrameLostR1")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.FrameLostR1);
    } else if (!strcmp(name, "FrameLostR2")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.FrameLostR2);
    } else if (!strcmp(name, "FrameLostR3")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.FrameLostR3);
    } else if (!strcmp(name, "FrameLostR4")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.FrameLostR4);
    } else if (!strcmp(name, "FrameLostR5")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.FrameLostR5);

    } else if (!strcmp(name, "HD_PacketsLostR1")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.HD_PacketsLostR1);
    } else if (!strcmp(name, "HD_PacketsLostR2")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.HD_PacketsLostR2);
    } else if (!strcmp(name, "HD_PacketsLostR3")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.HD_PacketsLostR3);
    } else if (!strcmp(name, "HD_PacketsLostR4")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.HD_PacketsLostR4);
    } else if (!strcmp(name, "HD_PacketsLostR5")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.HD_PacketsLostR5);

    } else if (!strcmp(name, "HD_BitRateR1")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.HD_BitRateR1);
    } else if (!strcmp(name, "HD_BitRateR2")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.HD_BitRateR2);
    } else if (!strcmp(name, "HD_BitRateR3")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.HD_BitRateR3);
    } else if (!strcmp(name, "HD_BitRateR4")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.HD_BitRateR4);
    } else if (!strcmp(name, "HD_BitRateR5")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.HD_BitRateR5);

    } else if (!strcmp(name, "BitRateR1")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR1);
    } else if (!strcmp(name, "BitRateR2")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR2);
    } else if (!strcmp(name, "BitRateR3")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR3);
    } else if (!strcmp(name, "BitRateR4")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR4);
    } else if (!strcmp(name, "BitRateR5")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR5);
    } else {
        ;//LogTr069Error("Device.X.StatisticConfiguration. name = %s\n", name);
        return -1;
    }
    return 0;
}
#endif //TR069_VERSION_1

}// extern "C"  读写接口函数



