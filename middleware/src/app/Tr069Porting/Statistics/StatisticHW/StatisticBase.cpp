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

// ��̬��Ա������ʼ�� start
pthread_mutex_t* StatisticBase::s_mutex = NULL;

unsigned int StatisticBase::s_startClock = 0; //�м����

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
// ��̬��Ա������ʼ�� end


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
        return ; //����ʧ��

    StatisticBase::s_statisticCfg.dirty = n;
    pthread_mutex_unlock(StatisticBase::s_mutex);
}

// StatisticRoot.cpp ��ʹ��
unsigned int statisticGetBitRateInterval()
{
    return  StatisticBase::s_statisticCfg.BitRateInterval;
}
// StatisticRoot.cpp ��ʹ��
unsigned int statisticGetPacketsLostInterval()
{
    return  StatisticBase::s_statisticCfg.PacketsLostInterval;
}


// ��д�ӿں���
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

//app_jse.c ��ʹ�ã�����Ҫɾ��
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
    �����ʡ���֡�ʡ�ý��������ͳ������
    ��λ��s Ĭ�ϣ�10s
 ------------------------------------------------------------------------------*/
int app_tr069_port_get_MonitoringInterval(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostInterval;
}

/*------------------------------------------------------------------------------
    �����ʷ�Χ1��ʼ
    ��0.01%Ϊ��λ
    ���磺 1 ��ʾ0.01%
    5 ��ʾ0.05%
    Ĭ��ֵ��0
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR1From(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR1From;
}

/*------------------------------------------------------------------------------
    �����ʷ�Χ1����
    ��0.01%Ϊ��λ
    ���磺 1 ��ʾ0.01%
    5 ��ʾ0.05%
    9999 ��ʾ���
    Ĭ��ֵ��0
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR1Till(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR1Till;
}

/*------------------------------------------------------------------------------
    �����ʷ�Χ2����ʼ
    ��0.01%Ϊ��λ
    Ĭ��ֵ��0
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR2From(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR2From;
}

/*------------------------------------------------------------------------------
    �����ʷ�Χ2������
    ��0.01%Ϊ��λ
    Ĭ��ֵ��10
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR2Till(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR2Till;
}

/*------------------------------------------------------------------------------
    �����ʷ�Χ3����ʼ
    ��0.01%Ϊ��λ
    Ĭ��ֵ��10
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR3From(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR3From;
}

/*------------------------------------------------------------------------------
    �����ʷ�Χ3������
    ��0.01%Ϊ��λ
    Ĭ��ֵ��20
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR3Till(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR3Till;
}

/*------------------------------------------------------------------------------
    �����ʷ�Χ4����ʼ
    ��0.01%Ϊ��λ
    Ĭ��ֵ��20
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR4From(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR4From;
}

/*------------------------------------------------------------------------------
    �����ʷ�Χ4������
    ��0.01%Ϊ��λ
    Ĭ��ֵ��50
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR4Till(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR4Till;
}

/*------------------------------------------------------------------------------
    �����ʷ�Χ5����ʼ
    ��0.01%Ϊ��λ
    Ĭ��ֵ��50
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
    �����ʷ�Χ5������
    ��0.01%Ϊ��λ
    Ĭ��ֵ��9999
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLostR5Till(void)
{
    return StatisticBase::s_statisticCfg.PacketsLostR5Till;
}

/*------------------------------------------------------------------------------
    ʵʱý�����ʷ�Χ1����ʼ
    ��100kbpsΪ��λ
    ���磺10 ��ʾ1Mbps
    13 ��ʾ1.3Mbps
    Ĭ��ֵ��16
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR1From(void)
{
    return StatisticBase::s_statisticCfg.BitRateR1From;
}

/*------------------------------------------------------------------------------
    ʵʱý�����ʷ�Χ1������
    ��100kbpsΪ��λ
    ���磺 10 ��ʾ1Mbps
    13 ��ʾ1.3Mbps
    9999 ��ʾ������
    Ĭ��ֵ��9999
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR1Till(void)
{
    return StatisticBase::s_statisticCfg.BitRateR1Till;
}

/*------------------------------------------------------------------------------
    ʵʱý�����ʷ�Χ2����ʼ
    ��100kbpsΪ��λ
    Ĭ��ֵ��14
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR2From(void)
{
    return StatisticBase::s_statisticCfg.BitRateR2From;
}

/*------------------------------------------------------------------------------
    ʵʱý�����ʷ�Χ2������
    ��100kbpsΪ��λ
    Ĭ��ֵ��16
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR2Till(void)
{
    return StatisticBase::s_statisticCfg.BitRateR2Till;
}

/*------------------------------------------------------------------------------
    ʵʱý�����ʷ�Χ3����ʼ
    ��100kbpsΪ��λ
    Ĭ��ֵ��12
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR3From(void)
{
    return StatisticBase::s_statisticCfg.BitRateR3From;
}

/*------------------------------------------------------------------------------
    ʵʱý�����ʷ�Χ3������
    ��100kbpsΪ��λ
    Ĭ��ֵ��14
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR3Till(void)
{
    return StatisticBase::s_statisticCfg.BitRateR3Till;
}

/*------------------------------------------------------------------------------
    ʵʱý�����ʷ�Χ4����ʼ
    ��100kbpsΪ��λ
    Ĭ��ֵ��8
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR4From(void)
{
    return StatisticBase::s_statisticCfg.BitRateR4From;
}

/*------------------------------------------------------------------------------
    ʵʱý�����ʷ�Χ4������
    ��100kbpsΪ��λ
    Ĭ��ֵ��12
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR4Till(void)
{
    return StatisticBase::s_statisticCfg.BitRateR4Till;
}

/*------------------------------------------------------------------------------
    ʵʱý�����ʷ�Χ5����ʼ
    ��100kbpsΪ��λ
    Ĭ��ֵ��0
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR5From(void)
{
    return StatisticBase::s_statisticCfg.BitRateR5From;
}

/*------------------------------------------------------------------------------
    ʵʱý�����ʷ�Χ5������
    ��100kbpsΪ��λ
    Ĭ��ֵ��8
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
    ��ʼʱ��
    ��ʽ��yyyymmddhhmmss
 ------------------------------------------------------------------------------*/
void tr069_statistic_get_Startpoint(char *value, int size)
{
    unsigned int sec = statisticLocakTime( ) - (mid_10ms( ) - StatisticBase::s_startClock) / 100;
    mid_tool_time2string(sec, value, 0);
}

/*------------------------------------------------------------------------------
    ����ʱ��
    ��ʽ��yyyymmddhhmmss
 ------------------------------------------------------------------------------*/
void tr069_statistic_get_Endpoint(char *value, int size)
{
    unsigned int sec = statisticLocakTime( );
    mid_tool_time2string(sec, value, 0);
    return;
}

/*------------------------------------------------------------------------------
    ��֤���ܴ���
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
    ��֤ʧ�ܵ��ܴ���
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_AuthFailNumbers(void)
{
    StatisticBase::s_statisticData.AuthFailNumbers = jseAuthFailCountGet();
    return StatisticBase::s_statisticData.AuthFailNumbers;
}

/*------------------------------------------------------------------------------
    ��֤ʧ����ϸ��Ϣ
    ÿ��ʧ�ܣ���Ҫ����һ�м�¼��֤���صĴ����뼰�����Ϣ
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
    �����鲥����ܴ���
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_MultiReqNumbers(void)
{
    return Hippo::UltraPlayerMultiple::s_statistic.getRequestNum() ? Hippo::UltraPlayerMultiple::s_statistic.getRequestNum() : 1;
}

/*------------------------------------------------------------------------------
    �����鲥��ʧ�ܣ������û���յ��鲥���ݣ����ܴ���
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_MultiFailNumbers(void)
{
    return Hippo::UltraPlayerMultiple::s_statistic.getFailedNum();
}

/*------------------------------------------------------------------------------
    �����鲥��ʧ�ܣ������û���յ��鲥���ݣ�����Ϣ����Ҫ������鲥Ƶ����ַ
    ÿ��ʧ�ܣ���Ҫ����һ�м�¼ʧ�ܵ��鲥Ƶ����Ϣ
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
    ���������㲥���ؿ���ʱ�ƣ�������ܴ���
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_VodReqNumbers(void)
{
    return Hippo::UltraPlayerVod::s_statistic.getRequestNum();
}

/*------------------------------------------------------------------------------
    ���������㲥���ؿ���ʱ�ƣ�����ʧ�ܵ��ܴ���
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_VodFailNumbers(void)
{
    return Hippo::UltraPlayerVod::s_statistic.getFailedNum();
}

/*------------------------------------------------------------------------------
    ���������㲥���ؿ���ʱ�ƣ�����ʧ����ϸ��Ϣ
    ÿ��ʧ�ܣ���Ҫ����һ�м�¼���صĴ����뼰ʧ�ܵĵ㲥���ؿ���ʱ�Ƶ�URL��Ϣ
    ��ʽ������RTSP://��./xx.mpg/
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
    Http������ܴ���
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
    Http����ʧ�ܵ��ܴ���
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
    Http����ʧ����ϸ��Ϣ
    ÿ��ʧ�ܣ���Ҫ����һ�м�¼���صĴ����뼰ʧ�ܵ�URL
    ˵����������ֻ��¼��ǰ����ʧ�ܵ�ҳ��URL������¼��ҳ�����������URL
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
    �쳣�����Ĵ���
    �쳣��������Ϊ������Ƶ���Ź����У������鲥�͵���������Ϊ��������ȡ�յ����޷��ٲ��ŵĴ�����
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_AbendNumbers(void)
{
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
#else
    return Hippo::UltraPlayer::s_statistic.getStreamGapNumbers();
#endif
}

/*------------------------------------------------------------------------------
    �쳣��������ϸ��Ϣ
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
    �鲥�����ʷ�Χ1��������
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_MultiPacketsLostR1(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR1;
}

/*------------------------------------------------------------------------------
    �鲥�����ʷ�Χ2��������
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_MultiPacketsLostR2(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR2;
}

/*------------------------------------------------------------------------------
    �鲥�����ʷ�Χ3��������
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_MultiPacketsLostR3(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR3;
}

/*------------------------------------------------------------------------------
    �鲥�����ʷ�Χ4��������
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_MultiPacketsLostR4(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR4;
}

/*------------------------------------------------------------------------------
    �鲥�����ʷ�Χ5��������
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_MultiPacketsLostR5(void)
{
    return StatisticBase::s_statisticData.MultiPacketsLostR5;
}

/*------------------------------------------------------------------------------
    ���������ʷ�Χ1��������
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_VODPacketsLostR1(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR1;
}

/*------------------------------------------------------------------------------
    ���������ʷ�Χ2��������
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_VODPacketsLostR2(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR2;
}

/*------------------------------------------------------------------------------
    ���������ʷ�Χ3��������
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_VODPacketsLostR3(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR3;
}

/*------------------------------------------------------------------------------
    ���������ʷ�Χ4��������
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_VODPacketsLostR4(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR4;
}

/*------------------------------------------------------------------------------
    ���������ʷ�Χ5��������
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_VODPacketsLostR5(void)
{
    return StatisticBase::s_statisticData.VODPacketsLostR5;
}

/*------------------------------------------------------------------------------
    ��������BitRateR1��Χ�ڵĴ���
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR1(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR1Nmb + StatisticBase::s_statisticData.VODBitRateR1Nmb;
}

/*------------------------------------------------------------------------------
    ��������BitRateR2��Χ�ڵĴ���
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR2(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR2Nmb + StatisticBase::s_statisticData.VODBitRateR2Nmb;
}

/*------------------------------------------------------------------------------
    ��������BitRateR3��Χ�ڵĴ���
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR3(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR3Nmb + StatisticBase::s_statisticData.VODBitRateR3Nmb;
}

/*------------------------------------------------------------------------------
    ��������BitRateR4��Χ�ڵĴ���
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR4(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR4Nmb + StatisticBase::s_statisticData.VODBitRateR4Nmb;
}

/*------------------------------------------------------------------------------
    ��������BitRateR5��Χ�ڵĴ���
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_BitRateR5(void)
{
    return StatisticBase::s_statisticData.MultiBitRateR5Nmb + StatisticBase::s_statisticData.VODBitRateR5Nmb;
}

/*------------------------------------------------------------------------------
    ��һ�����������ڵĶ�����
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_PacketsLost(void)
{
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
#else
    return StatisticBase::s_statisticData.PacketsLost;
#endif
}

/*------------------------------------------------------------------------------
    ��һ�����������ڵ���󶶶�ֵ����λ��ms
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_JitterMax(void)
{
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
#else
    return StatisticBase::s_statisticData.JitterMax;
#endif
}

/*------------------------------------------------------------------------------
    ��ǰ���ڲ���֡��
    ����ޣ��򲻼�¼��
 ------------------------------------------------------------------------------*/
int tr069_statistic_get_FrameRate(void)
{
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
#else
    return StatisticBase::s_statisticData.FrameRate;
#endif
}

/*------------------------------------------------------------------------------
    �ۻ���֡��
    ����ޣ��򲻼�¼��
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
    } else if (!strcmp(name, "LogServerUrl")) {//��־�ļ���������URL��Ϣ��Ӧ�ð����ϴ�URL�ļ�Ȩ��Ϣ��������������ն˹���������ļ�Ȩ��ʽһ�£�
        tr069StatisticGetLogAESServerUrl(str, size);
    } else if (!strcmp(name, "AESLogServerUrl")) {
        tr069StatisticGetLogAESServerUrl(str, size);
    } else if (!strcmp(name, "LogUploadInterval")) {

/*------------------------------------------------------------------------------
    ���ܼ������ļ��ϱ����
    ��λ��s Ĭ�ϣ�3600����1Сʱ��
    �ò�������Ϊ0ʱ����ʾ�ر����ܼ������ļ��ϱ�����
 ------------------------------------------------------------------------------*/
        snprintf(str, size, "%d", tr069StatisticGetLogUploadInterval());
    } else if (!strcmp(name, "LogRecordInterval")) {
/*------------------------------------------------------------------------------
    ���ܼ��ͳ�Ʋ����ļ�¼����ʱ��
    ��λ��s Ĭ�ϣ�3600����1Сʱ��
    ͳ����ʼΪÿ�ο��������趨��ͳ������ʱ���������µ�ͳ�����ڡ�
    �粻���趨��ͳ������ʱ���͹ػ��������������ڡ�
    �������µ�ͳ������ʱ��Ӧ��ǰ�����ڵļ�¼�����ϴ�������ƽ̨
 ------------------------------------------------------------------------------*/
        snprintf(str, size, "%d", tr069StatisticGetLogRecordInterval());
/*------------------------------------------------------------------------------
    �����ʡ���֡�ʡ�ý��������ͳ������
    ��λ��s Ĭ�ϣ�10s
 ------------------------------------------------------------------------------*/
    } else if (!strcmp(name, "MonitoringInterval")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostInterval);
    } else if (!strcmp(name, "IsFileorRealTime")) {
        snprintf(str, size, "%d", 0);
    } else if (!strcmp(name, "StatInterval")) {
        snprintf(str, size, "%d", int_getStatInterval( ));
    } else if (!strcmp(name, "PacketsLostR1From")) {//��0.01%Ϊ��λ, Ĭ��ֵ��0
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR1From);
    } else if (!strcmp(name, "PacketsLostR1Till")) {//Ĭ��ֵ��0
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR1Till);

    } else if (!strcmp(name, "PacketsLostR2From")) {//Ĭ��ֵ��0
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR2From);
    } else if (!strcmp(name, "PacketsLostR2Till")) {//Ĭ��ֵ��10
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR2Till);

    } else if (!strcmp(name, "PacketsLostR3From")) {//Ĭ��ֵ��10
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR3From);
    } else if (!strcmp(name, "PacketsLostR3Till")) {//Ĭ��ֵ��20
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR3Till);

    } else if (!strcmp(name, "PacketsLostR4From")) {//Ĭ��ֵ��20
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR4From);
    } else if (!strcmp(name, "PacketsLostR4Till")) {//Ĭ��ֵ��50
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR4Till);

    } else if (!strcmp(name, "PacketsLostR5From")) {//Ĭ��ֵ��50
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR5From);
    } else if (!strcmp(name, "PacketsLostR5Till")) {//Ĭ��ֵ��9999
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.PacketsLostR5Till);

/*------------------------------------------------------------------------------
    ʵʱý�����ʷ�Χ1����ʼ
    ��100kbpsΪ��λ
    ���磺10 ��ʾ1Mbps
    13 ��ʾ1.3Mbps
    Ĭ��ֵ��16
 ------------------------------------------------------------------------------*/
    } else if (!strcmp(name, "BitRateR1From")) {
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR1From);
    } else if (!strcmp(name, "BitRateR1Till")) {//Ĭ��ֵ��9999
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR1Till);

    } else if (!strcmp(name, "BitRateR2From")) {//Ĭ��ֵ��14
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR2From);
    } else if (!strcmp(name, "BitRateR2Till")) {//Ĭ��ֵ��16
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR2Till);

    } else if (!strcmp(name, "BitRateR3From")) {//Ĭ��ֵ��12
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR3From);
    } else if (!strcmp(name, "BitRateR3Till")) {//Ĭ��ֵ��14
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR3Till);

    } else if (!strcmp(name, "BitRateR4From")) {//Ĭ��ֵ��8
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR4From);
    } else if (!strcmp(name, "BitRateR4Till")) {//Ĭ��ֵ��12
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR4Till);

    } else if (!strcmp(name, "BitRateR4From")) {//Ĭ��ֵ��0
        snprintf(str, size, "%d", StatisticBase::s_statisticCfg.BitRateR4From);
    } else if (!strcmp(name, "BitRateR5Till")) {//Ĭ��ֵ��8
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

}// extern "C"  ��д�ӿں���



