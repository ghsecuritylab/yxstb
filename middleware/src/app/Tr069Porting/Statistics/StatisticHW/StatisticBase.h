#ifndef StatisticBase_h
#define StatisticBase_h

#include "../StatisticHW/LianChuangStatistic.h"
#include <pthread.h>

#ifdef __cplusplus

#define LOG_SERVER_URL_SIZE             256
#define STATISTIC_INFO_FAID_NUM         10 
#define STATISTIC_INFO_MULT_SIZE        64
#define STATISTIC_INFO_FAID_SIZE        256

#define STATISTIC_FILE_SIZE             (32 * 1024*2)
/****************************************************************
* modify log：2013.12
* StatisticBase的目的：1，一个统计的对象， 其他厂商有操作再来派生； 2，独立统计的数据及其读写函数在这里
* 特点：是一个抽象类，只有数据的基本读写， 没有实现统计操作的方法，方法本来也应该由厂商的需求来决定
* 问题：其实统计的大多数据也由厂商的需求来决定, 但是现在只有一种统计需求，以后有别的需求再重新归类， 现在统一放这里更加直观明了。
******************************************************************/
class StatisticBase {
// fun
public:
    StatisticBase();
    ~StatisticBase();

    virtual void sysCfgInit(void) = 0;
    virtual void statisticCfgReset(void) = 0;
    virtual void statisticCfgSave(void) = 0;

    virtual void statisticStart(void) = 0;
    virtual void statisticPeriod(void) = 0;
    virtual void statisticPost(void) = 0;
    virtual void statisticFile(void) = 0;
    
    virtual void statisticBitrateRn(int mult, int width, int bitrate) = 0;
    virtual void statisticBitratePercentRn(int mult, int width, int bitrate) = 0;
    virtual void statisticPacketLostsRn(int mult, int width, int totals, int losts) = 0;


    static char* getPackBuf();
    static  char* packBufInit();
    static void mutexInit();
// variable
public:
    // tools
    static pthread_mutex_t* s_mutex;
    static char* s_packBuf;  //用于上传前打包
    static char s_filebuf[LOG_SERVER_URL_SIZE];// 打包文件名
    static int s_startFlag;
    static unsigned int s_startClock;
    // huawei_v5
    static int s_bootUptime;
    static char s_powerOnTime[32];
    static int s_isSetPowerOnTime;
    
    static struct StatisticCfg {
        unsigned int PacketsLostInterval;
        unsigned int FramLostInterval;
        unsigned int BitRateInterval;

        int dirty;

        // use for non HW_C10
        int PacketsLostR1From;
        int PacketsLostR1Till;
        int PacketsLostR2From;
        int PacketsLostR2Till;
        int PacketsLostR3From;
        int PacketsLostR3Till;
        int PacketsLostR4From;
        int PacketsLostR4Till;
        int PacketsLostR5From;
        int PacketsLostR5Till;

        int BitRateR1From;
        int BitRateR1Till;
        int BitRateR2From;
        int BitRateR2Till;
        int BitRateR3From;
        int BitRateR3Till;
        int BitRateR4From;
        int BitRateR4Till;
        int BitRateR5From;
        int BitRateR5Till;

        // use for HW_C10
        int PacketsLostR1;
        int PacketsLostR2;
        int PacketsLostR3;
        int PacketsLostR4;
        int PacketsLostR5;

        int HD_PacketsLostR1;
        int HD_PacketsLostR2;
        int HD_PacketsLostR3;
        int HD_PacketsLostR4;
        int HD_PacketsLostR5;

        int BitRateR1;
        int BitRateR2;
        int BitRateR3;
        int BitRateR4;
        int BitRateR5;

        int HD_BitRateR1;
        int HD_BitRateR2;
        int HD_BitRateR3;
        int HD_BitRateR4;
        int HD_BitRateR5;

        int FrameLostR1;
        int FrameLostR2;
        int FrameLostR3;
        int FrameLostR4;
        int FrameLostR5;

    }s_statisticCfg;


    static struct StatisticData {
        int checksum;

        int RebootFlag;

        unsigned Startpoint;
        unsigned Endpoint;
        int AuthNumbers;
        int AuthFailNumbers;
        char AuthFailInfo[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_FAID_SIZE];

        int HTTPReqNumbers;
        int HTTPRRT;
        int HTTPFailNumbers;
        char HTTPFailInfo[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_FAID_SIZE];

        int MultiPacketsLostR1;
        int MultiPacketsLostR2;
        int MultiPacketsLostR3;
        int MultiPacketsLostR4;
        int MultiPacketsLostR5;

        int FECMultiPacketsLostR1Nmb;
        int FECMultiPacketsLostR2Nmb;
        int FECMultiPacketsLostR3Nmb;
        int FECMultiPacketsLostR4Nmb;
        int FECMultiPacketsLostR5Nmb;

        int VODPacketsLostR1;
        int VODPacketsLostR2;
        int VODPacketsLostR3;
        int VODPacketsLostR4;
        int VODPacketsLostR5;

        int ARQVODPacketsLostR1Nmb;
        int ARQVODPacketsLostR2Nmb;
        int ARQVODPacketsLostR3Nmb;
        int ARQVODPacketsLostR4Nmb;
        int ARQVODPacketsLostR5Nmb;

        int MultiBitRateR1Nmb;
        int MultiBitRateR2Nmb;
        int MultiBitRateR3Nmb;
        int MultiBitRateR4Nmb;
        int MultiBitRateR5Nmb;

        int VODBitRateR1Nmb;
        int VODBitRateR2Nmb;
        int VODBitRateR3Nmb;
        int VODBitRateR4Nmb;
        int VODBitRateR5Nmb;

        int HD_MultiPacketsLostR1Nmb;
        int HD_MultiPacketsLostR2Nmb;
        int HD_MultiPacketsLostR3Nmb;
        int HD_MultiPacketsLostR4Nmb;
        int HD_MultiPacketsLostR5Nmb;

        int HD_FECMultiPacketsLostR1Nmb;
        int HD_FECMultiPacketsLostR2Nmb;
        int HD_FECMultiPacketsLostR3Nmb;
        int HD_FECMultiPacketsLostR4Nmb;
        int HD_FECMultiPacketsLostR5Nmb;

        int HD_VODPacketsLostR1Nmb;
        int HD_VODPacketsLostR2Nmb;
        int HD_VODPacketsLostR3Nmb;
        int HD_VODPacketsLostR4Nmb;
        int HD_VODPacketsLostR5Nmb;

        int HD_ARQVODPacketsLostR1Nmb;
        int HD_ARQVODPacketsLostR2Nmb;
        int HD_ARQVODPacketsLostR3Nmb;
        int HD_ARQVODPacketsLostR4Nmb;
        int HD_ARQVODPacketsLostR5Nmb;

        int HD_MultiBitRateR1Nmb;
        int HD_MultiBitRateR2Nmb;
        int HD_MultiBitRateR3Nmb;
        int HD_MultiBitRateR4Nmb;
        int HD_MultiBitRateR5Nmb;

        int HD_VODBitRateR1Nmb;
        int HD_VODBitRateR2Nmb;
        int HD_VODBitRateR3Nmb;
        int HD_VODBitRateR4Nmb;
        int HD_VODBitRateR5Nmb;

        int FrameLostR1;
        int FrameLostR2;
        int FrameLostR3;
        int FrameLostR4;
        int FrameLostR5;

        char PPPoEID[32];
        char PPPoEPassword[64];
        char UserID[32];
        char UserIDPassword[16];
        char AuthURL[256];
        char ipaddress[256];

        int ScreenMosaicEvent;
        int ScreenMosaicLastPeriod;

        int PacketsLost;
        int JitterMax;
        int FrameRate;
        int FrameLoss;

        int BootUptime;
        char PowerOnTime[32]; // no use?
    }s_statisticData;
#ifdef TR069_LIANCHUANG
    static LianChuangStatistic s_lianChuangStatistic;
#endif
};

int setSavePoint(); // use for set startPoint before save to flash
unsigned int statisticLocakTime(void);
unsigned int statisticGetPacketsLostInterval();
unsigned int statisticGetBitRateInterval();

// 读写接口申明
extern "C" {
void statisticSetIsPowerOnTime(int set);
int statisticIsSetPowerOnTime(void);
void statisticSetPowerOnTime(void);
void statisticSetCfgDirty(int n); //for StatisticLog

int tr069_statistic_get_HD_BitRateR1(void);
int tr069_statistic_get_HD_BitRateR2(void);
int tr069_statistic_get_HD_BitRateR3(void);
int tr069_statistic_get_HD_BitRateR4(void);
int tr069_statistic_get_HD_BitRateR5(void);
void  tr069_statistic_set_HD_BitRateR1(int value);
void  tr069_statistic_set_HD_BitRateR2(int value);
void  tr069_statistic_set_HD_BitRateR3(int value);
void  tr069_statistic_set_HD_BitRateR4(int value);
void  tr069_statistic_set_HD_BitRateR5(int value);


void tr069_statistic_get_PlayErrorInfo(char *value, int size);
int tr069_statistic_get_MultiPacketsLostR1Nmb(void);
int tr069_statistic_get_MultiPacketsLostR2Nmb(void);
int tr069_statistic_get_MultiPacketsLostR3Nmb(void);
int tr069_statistic_get_MultiPacketsLostR4Nmb(void);
int tr069_statistic_get_MultiPacketsLostR5Nmb(void);
int tr069_statistic_get_FECMultiPacketsLostR1Nmb(void);
int tr069_statistic_get_FECMultiPacketsLostR2Nmb(void);
int tr069_statistic_get_FECMultiPacketsLostR3Nmb(void);
int tr069_statistic_get_FECMultiPacketsLostR4Nmb(void);
int tr069_statistic_get_FECMultiPacketsLostR5Nmb(void);
int tr069_statistic_get_VODPacketsLostR1Nmb(void);
int tr069_statistic_get_VODPacketsLostR2Nmb(void);
int tr069_statistic_get_VODPacketsLostR3Nmb(void);
int tr069_statistic_get_VODPacketsLostR4Nmb(void);
int tr069_statistic_get_VODPacketsLostR5Nmb(void);
int tr069_statistic_get_ARQVODPacketsLostR1Nmb(void);
int tr069_statistic_get_ARQVODPacketsLostR2Nmb(void);
int tr069_statistic_get_ARQVODPacketsLostR3Nmb(void);
int tr069_statistic_get_ARQVODPacketsLostR4Nmb(void);
int tr069_statistic_get_ARQVODPacketsLostR5Nmb(void);
int tr069_statistic_get_MultiBitRateR1Nmb(void);
int tr069_statistic_get_MultiBitRateR2Nmb(void);
int tr069_statistic_get_MultiBitRateR3Nmb(void);
int tr069_statistic_get_MultiBitRateR4Nmb(void);
int tr069_statistic_get_MultiBitRateR5Nmb(void);
int tr069_statistic_get_VODBitRateR1Nmb(void);
int tr069_statistic_get_VODBitRateR2Nmb(void);
int tr069_statistic_get_VODBitRateR3Nmb(void);
int tr069_statistic_get_VODBitRateR4Nmb(void);
int tr069_statistic_get_VODBitRateR5Nmb(void);
int tr069_statistic_get_HD_MultiPacketsLostR1Nmb(void);
int tr069_statistic_get_HD_MultiPacketsLostR2Nmb(void);
int tr069_statistic_get_HD_MultiPacketsLostR3Nmb(void);
int tr069_statistic_get_HD_MultiPacketsLostR4Nmb(void);
int tr069_statistic_get_HD_MultiPacketsLostR5Nmb(void);
int tr069_statistic_get_HD_FECMultiPacketsLostR1Nmb(void);
int tr069_statistic_get_HD_FECMultiPacketsLostR2Nmb(void);
int tr069_statistic_get_HD_FECMultiPacketsLostR3Nmb(void);
int tr069_statistic_get_HD_FECMultiPacketsLostR4Nmb(void);
int tr069_statistic_get_HD_FECMultiPacketsLostR5Nmb(void);
int tr069_statistic_get_HD_VODPacketsLostR1Nmb(void);
int tr069_statistic_get_HD_VODPacketsLostR2Nmb(void);
int tr069_statistic_get_HD_VODPacketsLostR3Nmb(void);
int tr069_statistic_get_HD_VODPacketsLostR4Nmb(void);
int tr069_statistic_get_HD_VODPacketsLostR5Nmb(void);
int tr069_statistic_get_HD_ARQVODPacketsLostR1Nmb(void);
int tr069_statistic_get_HD_ARQVODPacketsLostR2Nmb(void);
int tr069_statistic_get_HD_ARQVODPacketsLostR3Nmb(void);
int tr069_statistic_get_HD_ARQVODPacketsLostR4Nmb(void);
int tr069_statistic_get_HD_ARQVODPacketsLostR5Nmb(void);
int tr069_statistic_get_HD_VODBitRateR1Nmb(void);
int tr069_statistic_get_HD_VODBitRateR2Nmb(void);
int tr069_statistic_get_HD_VODBitRateR3Nmb(void);
int tr069_statistic_get_HD_VODBitRateR4Nmb(void);
int tr069_statistic_get_HD_VODBitRateR5Nmb(void);
int tr069_statistic_get_HD_MultiBitRateR1Nmb(void);
int tr069_statistic_get_HD_MultiBitRateR2Nmb(void);
int tr069_statistic_get_HD_MultiBitRateR3Nmb(void);
int tr069_statistic_get_HD_MultiBitRateR4Nmb(void);
int tr069_statistic_get_HD_MultiBitRateR5Nmb(void);

int tr069_statistic_get_FramesLostR1Nmb(void);
int tr069_statistic_get_FramesLostR2Nmb(void);
int tr069_statistic_get_FramesLostR3Nmb(void);
int tr069_statistic_get_FramesLostR4Nmb(void);
int tr069_statistic_get_FramesLostR5Nmb(void);

int tr069_statistic_get_PacketsLostR1(void);
int tr069_statistic_get_PacketsLostR2(void);
int tr069_statistic_get_PacketsLostR3(void);
int tr069_statistic_get_PacketsLostR4(void);
int tr069_statistic_get_PacketsLostR5(void);
void tr069_statistic_set_PacketsLostR1(int value);
void tr069_statistic_set_PacketsLostR2(int value);
void tr069_statistic_set_PacketsLostR3(int value);
void tr069_statistic_set_PacketsLostR4(int value);
void tr069_statistic_set_PacketsLostR5(int value);

int tr069_statistic_get_HD_PacketsLostR1(void);
int tr069_statistic_get_HD_PacketsLostR2(void);
int tr069_statistic_get_HD_PacketsLostR3(void);
int tr069_statistic_get_HD_PacketsLostR4(void);
int tr069_statistic_get_HD_PacketsLostR5(void);
void tr069_statistic_set_HD_PacketsLostR1(int value);
void tr069_statistic_set_HD_PacketsLostR2(int value);
void tr069_statistic_set_HD_PacketsLostR3(int value);
void tr069_statistic_set_HD_PacketsLostR4(int value);
void tr069_statistic_set_HD_PacketsLostR5(int value);

int tr069_statistic_get_BitRateRangeR1(void);
int tr069_statistic_get_BitRateRangeR2(void);
int tr069_statistic_get_BitRateRangeR3(void);
int tr069_statistic_get_BitRateRangeR4(void);
int tr069_statistic_get_BitRateRangeR5(void);
void  tr069_statistic_set_BitRateRangeR1(int value);
void  tr069_statistic_set_BitRateRangeR2(int value);
void  tr069_statistic_set_BitRateRangeR3(int value);
void  tr069_statistic_set_BitRateRangeR4(int value);
void  tr069_statistic_set_BitRateRangeR5(int value);

int tr069_statistic_get_FramesLostR1(void);
int tr069_statistic_get_FramesLostR2(void);
int tr069_statistic_get_FramesLostR3(void);
int tr069_statistic_get_FramesLostR4(void);
int tr069_statistic_get_FramesLostR5(void);
void tr069_statistic_set_FramesLostR1(int value);
void tr069_statistic_set_FramesLostR2(int value);
void tr069_statistic_set_FramesLostR3(int value);
void tr069_statistic_set_FramesLostR4(int value);
void tr069_statistic_set_FramesLostR5(int value);

int tr069_statistic_get_LogUploadInterval(void);
int tr069_statistic_get_LogRecordInterval(void);
int tr069_statistic_get_MonitoringInterval(void);
int tr069_statistic_get_PacketsLostR1From(void);
int tr069_statistic_get_PacketsLostR1Till(void);
int tr069_statistic_get_PacketsLostR2From(void);
int tr069_statistic_get_PacketsLostR2Till(void);
int tr069_statistic_get_PacketsLostR3From(void);
int tr069_statistic_get_PacketsLostR3Till(void);
int tr069_statistic_get_PacketsLostR4From(void);
int tr069_statistic_get_PacketsLostR4Till(void);
int tr069_statistic_get_PacketsLostR5From(void);
int tr069_statistic_get_PacketsLostR5Till(void);
void tr069_statistic_set_PacketsLostR1From(int value);
void tr069_statistic_set_PacketsLostR1Till(int value);
void tr069_statistic_set_PacketsLostR2From(int value);
void tr069_statistic_set_PacketsLostR2Till(int value);
void tr069_statistic_set_PacketsLostR3From(int value);
void tr069_statistic_set_PacketsLostR3Till(int value);
void tr069_statistic_set_PacketsLostR4From(int value);
void tr069_statistic_set_PacketsLostR4Till(int value);
void tr069_statistic_set_PacketsLostR5From(int value);
void tr069_statistic_set_PacketsLostR5Till(int value);

int tr069_statistic_get_BitRateR1From(void);
int tr069_statistic_get_BitRateR1Till(void);
int tr069_statistic_get_BitRateR2From(void);
int tr069_statistic_get_BitRateR2Till(void);
int tr069_statistic_get_BitRateR3From(void);
int tr069_statistic_get_BitRateR3Till(void);
int tr069_statistic_get_BitRateR4From(void);
int tr069_statistic_get_BitRateR4Till(void);
int tr069_statistic_get_BitRateR5From(void);
int tr069_statistic_get_BitRateR5Till(void);
void tr069_statistic_set_BitRateR1From(int value);
void tr069_statistic_set_BitRateR1Till(int value);
void tr069_statistic_set_BitRateR2From(int value);
void tr069_statistic_set_BitRateR2Till(int value);
void tr069_statistic_set_BitRateR3From(int value);
void tr069_statistic_set_BitRateR3Till(int value);
void tr069_statistic_set_BitRateR4From(int value);
void tr069_statistic_set_BitRateR4Till(int value);
void tr069_statistic_set_BitRateR5From(int value);
void tr069_statistic_set_BitRateR5Till(int value);

int tr069_statistic_get_MultiPacketsLostR1(void);
int tr069_statistic_get_MultiPacketsLostR2(void);
int tr069_statistic_get_MultiPacketsLostR3(void);
int tr069_statistic_get_MultiPacketsLostR4(void);
int tr069_statistic_get_MultiPacketsLostR5(void);
int tr069_statistic_get_VODPacketsLostR1(void);
int tr069_statistic_get_VODPacketsLostR2(void);
int tr069_statistic_get_VODPacketsLostR3(void);
int tr069_statistic_get_VODPacketsLostR4(void);
int tr069_statistic_get_VODPacketsLostR5(void);
int tr069_statistic_get_BitRateR1(void);
int tr069_statistic_get_BitRateR2(void);
int tr069_statistic_get_BitRateR3(void);
int tr069_statistic_get_BitRateR4(void);
int tr069_statistic_get_BitRateR5(void);


void tr069_statistic_get_Startpoint(char *value, int size); 
void tr069_statistic_get_Endpoint(char *value, int size);

int tr069_statistic_get_BootUpTime(void);
void tr069_statistic_get_PowerTime(char *value, int size);



int tr069_statistic_get_MultiReqNumbers(void);
int tr069_statistic_get_MultiFailNumbers(void);
void tr069_statistic_get_MultiFailInfo(char *value, int size);
int tr069_statistic_get_VodReqNumbers(void);
int tr069_statistic_get_VodFailNumbers(void);
void tr069_statistic_get_VodFailInfo(char *value, int size);

int tr069_statistic_get_AbendNumbers(void);
void tr069_statistic_get_AbendInfo(char *value, int size);
int tr069_statistic_get_AbendDurationTotal(void);
int tr069_statistic_get_AbendDurationMax(void);
int tr069_statistic_get_MultiRRT(void);
int tr069_statistic_get_VodRRT(void);
int tr069_statistic_get_VODAbendNumbers(void);
int tr069_statistic_get_MultiAbendNumbers(void);
int tr069_statistic_get_MultiAbendUPNumbers(void);
int tr069_statistic_get_VODUPAbendNumbers(void);
int tr069_statistic_get_HD_MultiAbendNumbers(void);
int tr069_statistic_get_HD_VODAbendNumbers(void);
int tr069_statistic_get_HD_MultiUPAbendNumbers(void);
int tr069_statistic_get_HD_VODUPAbendNumbers(void);
int tr069_statistic_get_BufferIncNmb(void);
int tr069_statistic_get_HTTPRRT(void);
int tr069_statistic_get_BufferDecNmb(void);
int tr069_statistic_get_PacketsLost(void);
int tr069_statistic_get_JitterMax(void);
int tr069_statistic_get_FrameRate(void);
int tr069_statistic_get_FrameLoss(void);
/////////////////////////////////////////////////////
int tr069_statistic_get_AuthFailNumbers(void);
void tr069_statistic_get_AuthFailInfo(char *value, int size);
int tr069_statistic_get_AuthNumbers(void);

int tr069_statistic_get_HTTPReqNumbers(void);
int tr069_statistic_set_HTTPReqNumbers(void);
int tr069_statistic_get_HTTPFailNumbers(void);
int tr069_statistic_set_HTTPFailNumbers(void);
void tr069_statistic_get_HTTPFailInfo(char *value, int size);

void tr069_statistic_set_userstatus_time(void);

//int app_tr069_port_get_StatInterval(void);
//void app_tr069_port_set_StatInterval(int value);
void tr069_statistic_set_LogUploadInterval(int value);
void tr069_statistic_set_LogRecordInterval(int value);
void tr069_statistic_set_MonitoringInterval(int value);

int tr069_statistic_setConfiguration(char *name, char *str, unsigned int x);
int tr069_statistic_getConfiguration(char *, char *, unsigned int);


} // extern "C"

#endif // __cplusplus

#endif // StatisticBase_h
