#include "StatisticHWPlay.h"

#include "StatisticBase.h"
#include "pathConfig.h"
#include "../../../Player/UltraPlayer.h"
#include "../../../Player/UltraPlayerMultiple.h"
#include "../../../Player/UltraPlayerVod.h"
#include "../StatisticRoot.h"
#include "TR069Assertions.h"
#include "NetworkFunctions.h"
#include "takin.h"
#include "AppSetting.h"
#include "SysSetting.h"
#include "SettingApi.h"
#include "Jse/Huawei/Business/Session/JseHWSession.h"
#include "Jse/Hybroad/Business/JseBusiness.h"
#include "../StatisticLog/StatisticLog.h"
#include "../../ParameterMap/Device/X_CU_STB/Alarm/Tr069CUAlarm.h"
#ifdef INCLUDE_TR069_CTC
#include "../../ParameterMap/Device/DeviceInfo/Tr069X_CTC_IPTV_Monitor.h"
#include "../../ParameterMap/Device/X_CTC_IPTV/Tr069LogMsg.h"
#include "../../../Player/UltraPlayerStatistic/UltraPlayerStatistic.h"
#endif

#include "mid_fpanel.h"
#include "mid/mid_http.h"
#include "mid/mid_time.h"
#include "mid/mid_timer.h"
#include "mid/mid_tools.h"
#include "sys_basic_macro.h"

#include "ind_cfg.h"

#include "app_tr069_alarm.h"
#include "app_jse.h"
#include "stream/rtsp/rtsp_app.h" // for getRtsp();
#include "stream/rtsp/rtsp_stat.h" // for struct Rtsp

#include <sys/stat.h>
#include <sys/socket.h>
#include<arpa/inet.h>

#define FRAMERATE 25
#define MAX_FILE_LOG_NUM  3
#define STATISTIC_PERIODIC_MIN          30

static CfgTree_t g_cfgTree = NULL;

StatisticHWPlay::StatisticHWPlay()
{
}

StatisticHWPlay::~StatisticHWPlay()
{
}

//获取百分比
//tanf mark：只被app_statistic_file_huawei调用，据说app_statistic_file_huawei没有用了？到时可一起删除
static float getRateInfo(int failnum, int totalnum) //get_rate_info
{
    float rate = 0.0;

    if(totalnum != 0)
        rate = (totalnum - failnum) * 100.00 / totalnum ;
    LogTr069Debug("getRateInfo =%.2f\n", rate); //存在问题，应该是取小数点后一位
    return rate;
}

//tanf mark：只被app_statistic_file_huawei调用，据说app_statistic_file_huawei没有用了？
static float getPerform(int ms) //getPerform
{
    float rate = 0.0;

    rate = ms / 1000.00;
    LogTr069Debug("getRateInfo =%.2f\n", rate);
    return rate;
}


#if defined(HUAWEI_C10)
/*************************************************
Description: 统计BitRate所在区间的数目
Return:
 *************************************************/
void
StatisticHWPlay::statisticBitrateRn(int mult, int width, int bitrate) //stream_port_post_bitrate
{
#ifdef INCLUDE_TR069_CTC
    static int n = 0;
    int size = 0;
    int len = 0;
    char buf[16] = {0};

#ifdef TR069_MONITOR
    monitor_play_bitrate(bitrate);
#endif
    if (tr069LogMsgGetEnable()) {
        snprintf(buf, 16, "%d", bitrate);
        tr069LogMsgStatisticInfo(LOG_MSG_AVARAGERATE,buf);
        codec_buf_info(&size, &len);
        if (size) {
            snprintf(buf, 16, "%d", len * 100 /size);
            tr069LogMsgStatisticInfo(LOG_MSG_BUFFER,buf);
        }
    }
#endif
    return;
}

/*************************************************
Description: 统计PacketLost所在区间的数目
Return:
 *************************************************/
void
StatisticHWPlay::statisticBitratePercentRn(int mult, int width, int bitPercent) // stream_port_post_bitrate_percent
{

#ifdef TR069_LIANCHUANG
    s_lianChuangStatistic.setBitRate(bitPercent);
#endif

    pthread_mutex_lock(s_mutex);
    bitPercent /= 10;
    if (width <= 720) {
        if (mult) {
            if(bitPercent >= s_statisticCfg.BitRateR1)
                s_statisticData.MultiBitRateR1Nmb++;
            else if(bitPercent >= s_statisticCfg.BitRateR2 && bitPercent < s_statisticCfg.BitRateR1)
                s_statisticData.MultiBitRateR2Nmb++;
            else if(bitPercent >= s_statisticCfg.BitRateR3 && bitPercent < s_statisticCfg.BitRateR2)
                s_statisticData.MultiBitRateR3Nmb++;
            else if(bitPercent >= s_statisticCfg.BitRateR4 && bitPercent < s_statisticCfg.BitRateR3)
                s_statisticData.MultiBitRateR4Nmb++;
            else if(bitPercent < s_statisticCfg.BitRateR5)
                s_statisticData.MultiBitRateR5Nmb++;
        } else {
            if(bitPercent >= s_statisticCfg.BitRateR1)
                s_statisticData.VODBitRateR1Nmb++;
            else if(bitPercent >= s_statisticCfg.BitRateR2 && bitPercent < s_statisticCfg.BitRateR1)
                s_statisticData.VODBitRateR2Nmb++;
            else if(bitPercent >= s_statisticCfg.BitRateR3 && bitPercent < s_statisticCfg.BitRateR2)
                s_statisticData.VODBitRateR3Nmb++;
            else if(bitPercent >= s_statisticCfg.BitRateR4 && bitPercent < s_statisticCfg.BitRateR3)
                s_statisticData.VODBitRateR4Nmb++;
            else if(bitPercent < s_statisticCfg.BitRateR5)
                s_statisticData.VODBitRateR5Nmb++;
        }
    } else {
        if (mult) {
            if(bitPercent >= s_statisticCfg.HD_BitRateR1)
                s_statisticData.HD_MultiBitRateR1Nmb++;
            else if(bitPercent >= s_statisticCfg.HD_BitRateR2 && bitPercent < s_statisticCfg.HD_BitRateR1)
                s_statisticData.HD_MultiBitRateR2Nmb++;
            else if(bitPercent >= s_statisticCfg.HD_BitRateR3 && bitPercent < s_statisticCfg.HD_BitRateR2)
                s_statisticData.HD_MultiBitRateR3Nmb++;
            else if(bitPercent >= s_statisticCfg.HD_BitRateR4 && bitPercent < s_statisticCfg.HD_BitRateR3)
                s_statisticData.HD_MultiBitRateR4Nmb++;
            else if(bitPercent < s_statisticCfg.HD_BitRateR5)
                s_statisticData.HD_MultiBitRateR5Nmb++;
        } else {
            if(bitPercent >= s_statisticCfg.HD_BitRateR1)
                s_statisticData.HD_VODBitRateR1Nmb++;
            else if(bitPercent >= s_statisticCfg.HD_BitRateR2 && bitPercent < s_statisticCfg.HD_BitRateR1)
                s_statisticData.HD_VODBitRateR2Nmb++;
            else if(bitPercent >= s_statisticCfg.HD_BitRateR3 && bitPercent < s_statisticCfg.HD_BitRateR2)
                s_statisticData.HD_VODBitRateR3Nmb++;
            else if(bitPercent >= s_statisticCfg.HD_BitRateR4 && bitPercent < s_statisticCfg.HD_BitRateR3)
                s_statisticData.HD_VODBitRateR4Nmb++;
            else if(bitPercent < s_statisticCfg.HD_BitRateR5)
                s_statisticData.HD_VODBitRateR5Nmb++;
        }
    }
    pthread_mutex_unlock(s_mutex);
    return;
}

#else

/*************************************************
Description: 统计BitRate所在区间的数目
Return:
 *************************************************/
void
StatisticHWPlay::statisticBitrateRn(int mult, int width, int bitrate)
{
    bitrate /= 100;//kps to 100kps

#ifdef TR069_LIANCHUANG
    s_lianChuangStatistic.setBitRate(bitrate);
#endif


    pthread_mutex_lock(s_mutex);
    if(mult) {
        if(bitrate >= s_statisticCfg.BitRateR1From && bitrate <= s_statisticCfg.BitRateR1Till)
            s_statisticData.MultiBitRateR1Nmb++;
        else if(bitrate >= s_statisticCfg.BitRateR2From && bitrate <= s_statisticCfg.BitRateR2Till)
            s_statisticData.MultiBitRateR2Nmb++;
        else if(bitrate >= s_statisticCfg.BitRateR3From && bitrate <= s_statisticCfg.BitRateR3Till)
            s_statisticData.MultiBitRateR3Nmb++;
        else if(bitrate >= s_statisticCfg.BitRateR4From && bitrate <= s_statisticCfg.BitRateR4Till)
            s_statisticData.MultiBitRateR4Nmb++;
        else if(bitrate >= s_statisticCfg.BitRateR5From && bitrate <= s_statisticCfg.BitRateR5Till)
            s_statisticData.MultiBitRateR5Nmb++;
    } else {
        if(bitrate >= s_statisticCfg.BitRateR1From && bitrate <= s_statisticCfg.BitRateR1Till)
            s_statisticData.VODBitRateR1Nmb++;
        else if(bitrate >= s_statisticCfg.BitRateR2From && bitrate <= s_statisticCfg.BitRateR2Till)
            s_statisticData.VODBitRateR2Nmb++;
        else if(bitrate >= s_statisticCfg.BitRateR3From && bitrate <= s_statisticCfg.BitRateR3Till)
            s_statisticData.VODBitRateR3Nmb++;
        else if(bitrate >= s_statisticCfg.BitRateR4From && bitrate <= s_statisticCfg.BitRateR4Till)
            s_statisticData.VODBitRateR4Nmb++;
        else if(bitrate >= s_statisticCfg.BitRateR5From && bitrate <= s_statisticCfg.BitRateR5Till)
            s_statisticData.VODBitRateR5Nmb++;
    }
    pthread_mutex_unlock(s_mutex);
    return;
}


/*************************************************
Description: 统计PacketLost所在区间的数目
Return:
 *************************************************/
void
StatisticHWPlay::statisticBitratePercentRn(int mult, int width, int bitrate)
{
}
#endif

/*************************************************
Description: 统计PacketLost所在区间的数目
Return:
 *************************************************/
void
StatisticHWPlay::statisticPacketLostsRn(int mult, int width, int totals, int losts) // stream_port_post_pklosts
{
    int rate = 0, limit = 0;
    int frame_lower_limit = 0, frame_upper_limit = 0;
    int frameslost = 0;
    char buf[16] = {0};
    unsigned int alarmSwitch = 0;
    static int frame_alarm_flag = ALARM_RELIEVE;

#ifdef TR069_LIANCHUANG
    s_LianChuangStatistic.setPacketsLost(totals);
    s_LianChuangStatistic.setPacketsLost(sLianChuangStatistic.getPacketsLost() + losts);
#endif
    pthread_mutex_lock(s_mutex);

#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)||defined(Sichuan)
#else
    s_statisticData.PacketsLost += losts;
    s_statisticData.FrameLoss = s_statisticData.PacketsLost * 7;
#endif

#if defined(HUAWEI_C10)
    if(totals > 0)
        rate = losts * 1000 / totals;
    else
        rate = 999;

    int tempPacketsLostR1 = s_statisticCfg.PacketsLostR1;
    int tempPacketsLostR2 = s_statisticCfg.PacketsLostR1 + s_statisticCfg.PacketsLostR2;
    int tempPacketsLostR3 = s_statisticCfg.PacketsLostR1 + s_statisticCfg.PacketsLostR2 + s_statisticCfg.PacketsLostR3;
    int tempPacketsLostR4 = s_statisticCfg.PacketsLostR1 + s_statisticCfg.PacketsLostR2 + s_statisticCfg.PacketsLostR3 + s_statisticCfg.PacketsLostR4;
    int tempPacketsLostR5 = s_statisticCfg.PacketsLostR1 + s_statisticCfg.PacketsLostR2 + s_statisticCfg.PacketsLostR3 + s_statisticCfg.PacketsLostR4 + s_statisticCfg.PacketsLostR5;

    int tempHDPacketsLostR1 = s_statisticCfg.HD_PacketsLostR1;
    int tempHDPacketsLostR2 = s_statisticCfg.HD_PacketsLostR1 + s_statisticCfg.HD_PacketsLostR2;
    int tempHDPacketsLostR3 = s_statisticCfg.HD_PacketsLostR1 + s_statisticCfg.HD_PacketsLostR2 + s_statisticCfg.HD_PacketsLostR3;
    int tempHDPacketsLostR4 = s_statisticCfg.HD_PacketsLostR1 + s_statisticCfg.HD_PacketsLostR2 + s_statisticCfg.HD_PacketsLostR3 + s_statisticCfg.HD_PacketsLostR4;
    int tempHDPacketsLostR5 = s_statisticCfg.HD_PacketsLostR1 + s_statisticCfg.HD_PacketsLostR2 + s_statisticCfg.HD_PacketsLostR3 + s_statisticCfg.HD_PacketsLostR4 + s_statisticCfg.HD_PacketsLostR5;

    if (width <= 720) {
        switch(mult){
        case PcketsLost_Flag_MULT:
            if(rate <= tempPacketsLostR1)
                s_statisticData.MultiPacketsLostR1++;
            else if(rate > tempPacketsLostR1 && rate <= tempPacketsLostR2)
                s_statisticData.MultiPacketsLostR2++;
            else if(rate > tempPacketsLostR2 && rate <= tempPacketsLostR3)
                s_statisticData.MultiPacketsLostR3++;
            else if(rate > tempPacketsLostR3 && rate <= tempPacketsLostR4)
                s_statisticData.MultiPacketsLostR4++;
            else if(tempPacketsLostR5 >= 0xFF || rate < tempPacketsLostR5)
                s_statisticData.MultiPacketsLostR5++;
            break;
        case PcketsLost_Flag_VOD:
            if(rate <= tempPacketsLostR1)
                s_statisticData.VODPacketsLostR1++;
            else if(rate > tempPacketsLostR1 && rate <= tempPacketsLostR2)
                s_statisticData.VODPacketsLostR2++;
            else if(rate > tempPacketsLostR2 && rate <= tempPacketsLostR3)
                s_statisticData.VODPacketsLostR3++;
            else if(rate > tempPacketsLostR3 && rate <= tempPacketsLostR4)
                s_statisticData.VODPacketsLostR4++;
            else if(tempPacketsLostR5 >= 0xFF || rate < tempPacketsLostR5)
                s_statisticData.VODPacketsLostR5++;
            break;
        case PcketsLost_Flag_ARQ:
            if(rate <= tempPacketsLostR1)
               s_statisticData.ARQVODPacketsLostR1Nmb++;
            else if(rate > tempPacketsLostR1 && rate <= tempPacketsLostR2)
               s_statisticData.ARQVODPacketsLostR2Nmb++;
            else if(rate > tempPacketsLostR2 && rate <= tempPacketsLostR3)
               s_statisticData.ARQVODPacketsLostR3Nmb++;
            else if(rate > tempPacketsLostR3 && rate <= tempPacketsLostR4)
               s_statisticData.ARQVODPacketsLostR4Nmb++;
            else if(tempPacketsLostR5 >= 0xFF || rate < tempPacketsLostR5)
               s_statisticData.ARQVODPacketsLostR5Nmb++;
              break;
        case PcketsLost_Flag_FEC:
            if(rate <= tempPacketsLostR1)
               s_statisticData.FECMultiPacketsLostR1Nmb++;
            else if(rate > tempPacketsLostR1 && rate <= tempPacketsLostR2)
               s_statisticData.FECMultiPacketsLostR2Nmb++;
            else if(rate > tempPacketsLostR2 && rate <= tempPacketsLostR3)
               s_statisticData.FECMultiPacketsLostR3Nmb++;
            else if(rate > tempPacketsLostR3 && rate <= tempPacketsLostR4)
               s_statisticData.FECMultiPacketsLostR4Nmb++;
            else if(tempPacketsLostR5 >= 0xFF || rate < tempPacketsLostR5)
               s_statisticData.FECMultiPacketsLostR5Nmb++;
               break;
        default:
            break;
          }
    } else {
        switch(mult){
        case PcketsLost_Flag_MULT:
            if(rate <= tempHDPacketsLostR1)
                s_statisticData.HD_MultiPacketsLostR1Nmb++;
            else if(rate > tempHDPacketsLostR1 && rate <= tempHDPacketsLostR2)
                s_statisticData.HD_MultiPacketsLostR2Nmb++;
            else if(rate > tempHDPacketsLostR2 && rate <= tempHDPacketsLostR3)
                s_statisticData.HD_MultiPacketsLostR3Nmb++;
            else if(rate > tempHDPacketsLostR3 && rate <= tempHDPacketsLostR4)
                s_statisticData.HD_MultiPacketsLostR4Nmb++;
            else if(tempHDPacketsLostR5 >= 0xFF || rate < tempHDPacketsLostR5)
                s_statisticData.HD_MultiPacketsLostR5Nmb++;
            break;
        case PcketsLost_Flag_VOD:
            if(rate <= tempHDPacketsLostR1)
                s_statisticData.HD_VODPacketsLostR1Nmb++;
            else if(rate > tempHDPacketsLostR1 && rate <= tempHDPacketsLostR2)
                s_statisticData.HD_VODPacketsLostR2Nmb++;
            else if(rate > tempHDPacketsLostR2 && rate <= tempHDPacketsLostR3)
                s_statisticData.HD_VODPacketsLostR3Nmb++;
            else if(rate > tempHDPacketsLostR3 && rate <= tempHDPacketsLostR4)
                s_statisticData.HD_VODPacketsLostR4Nmb++;
            else if(tempHDPacketsLostR5 >= 0xFF || rate < tempHDPacketsLostR5)
                s_statisticData.HD_VODPacketsLostR5Nmb++;
            break;
        case PcketsLost_Flag_ARQ:
            if(rate <= tempHDPacketsLostR1)
                s_statisticData.HD_ARQVODPacketsLostR1Nmb ++;
            else if(rate > tempHDPacketsLostR1 && rate <= tempHDPacketsLostR2)
                s_statisticData.HD_ARQVODPacketsLostR2Nmb ++;
            else if(rate > tempHDPacketsLostR2 && rate <= tempHDPacketsLostR3)
                s_statisticData.HD_ARQVODPacketsLostR3Nmb ++;
            else if(rate > tempHDPacketsLostR3 && rate <= tempHDPacketsLostR4)
                s_statisticData.HD_ARQVODPacketsLostR4Nmb++;
            else if(tempHDPacketsLostR5 >= 0xFF || rate < tempHDPacketsLostR5)
                s_statisticData.HD_ARQVODPacketsLostR5Nmb++;
            break;
        case PcketsLost_Flag_FEC:
            if(rate <= tempHDPacketsLostR1)
                s_statisticData.HD_FECMultiPacketsLostR1Nmb++;
            else if(rate > tempHDPacketsLostR1 && rate <= tempHDPacketsLostR2)
                s_statisticData.HD_FECMultiPacketsLostR2Nmb++;
            else if(rate > tempHDPacketsLostR2 && rate <= tempHDPacketsLostR3)
                s_statisticData.HD_FECMultiPacketsLostR3Nmb++;
            else if(rate > tempHDPacketsLostR3 && rate <= tempHDPacketsLostR4)
                s_statisticData.HD_FECMultiPacketsLostR4Nmb++;
            else if(tempHDPacketsLostR5 >= 0xFF || rate < tempHDPacketsLostR5)
                s_statisticData.HD_FECMultiPacketsLostR5Nmb++;
            break;
        default:
            break;
        }
    }

    frameslost = (int)(FRAMERATE * s_statisticCfg.PacketsLostInterval * sqrt((double)(100 * (double)losts / totals)) / 10);
    if(frameslost == s_statisticCfg.FrameLostR1)
        s_statisticData.FrameLostR1++;
    else if(frameslost > s_statisticCfg.FrameLostR1 && frameslost <=  s_statisticCfg.FrameLostR2)
        s_statisticData.FrameLostR2++;
    else if(frameslost > s_statisticCfg.FrameLostR2 && frameslost <= s_statisticCfg.FrameLostR3)
        s_statisticData.FrameLostR3++;
    else if(frameslost > s_statisticCfg.FrameLostR3 && frameslost <= s_statisticCfg.FrameLostR4)
        s_statisticData.FrameLostR4++;
    else if(s_statisticCfg.FrameLostR5 >= 0xFF || frameslost <= s_statisticCfg.FrameLostR5)
        s_statisticData.FrameLostR5++;
    pthread_mutex_unlock(s_mutex);

    app_reportPacketsLost(rate);

#ifdef INCLUDE_TR069_CTC
#ifdef Sichuan
  int framesRate = 0;

    framesRate=(int)sqrt((double)(100 * losts / totals)*10);
  app_get_framelos_vaue(&frame_lower_limit, &frame_upper_limit);
  if ((framesRate <= frame_lower_limit) && (frame_alarm_flag == ALARM_RINGING) && (1 == alarmSwitch)) {
      app_clear_drop_frame_alarm();
      frame_alarm_flag = ALARM_RELIEVE;
  }
  if ((framesRate > frame_upper_limit) && (frame_alarm_flag == ALARM_RELIEVE) && (1 == alarmSwitch)) {
        app_report_drop_frame_alarm();
        frame_alarm_flag = ALARM_RINGING;
  }
#endif
    if (tr069LogMsgGetEnable()) {
    snprintf(buf, 16, "%d", totals/s_statisticCfg.PacketsLostInterval);
    tr069LogMsgStatisticInfo(LOG_MSG_PKGTOTALONESEC,buf);
    snprintf(buf, 16, "%d", totals*1316/s_statisticCfg.BitRateInterval);
    tr069LogMsgStatisticInfo(LOG_MSG_BYTETOTALONESEC,buf);
    snprintf(buf, 16, "%d", rate);
    tr069LogMsgStatisticInfo(LOG_MSG_PKGLOSTRATE,buf);
    }
#else
    tr069_cu_setFramesLost(frameslost);
#endif//INCLUDE_TR069_CTC
#else

    if(totals > 0)
        rate = losts * 10000 / totals;
    else
        rate = 9999;

    if(mult) {
        if(rate >= s_statisticCfg.PacketsLostR1From && rate <= s_statisticCfg.PacketsLostR1Till)
            s_statisticData.MultiPacketsLostR1++;
        else if(rate >= s_statisticCfg.PacketsLostR2From && rate <= s_statisticCfg.PacketsLostR2Till)
            s_statisticData.MultiPacketsLostR2++;
        else if(rate >= s_statisticCfg.PacketsLostR3From && rate <= s_statisticCfg.PacketsLostR3Till)
            s_statisticData.MultiPacketsLostR3++;
        else if(rate >= s_statisticCfg.PacketsLostR4From && rate <= s_statisticCfg.PacketsLostR4Till)
            s_statisticData.MultiPacketsLostR4++;
        else if(rate >= s_statisticCfg.PacketsLostR5From && rate <= s_statisticCfg.PacketsLostR5Till)
            s_statisticData.MultiPacketsLostR5++;
    } else {
        if(rate >= s_statisticCfg.PacketsLostR1From && rate <= s_statisticCfg.PacketsLostR1Till)
            s_statisticData.VODPacketsLostR1++;
        else if(rate >= s_statisticCfg.PacketsLostR2From && rate <= s_statisticCfg.PacketsLostR2Till)
            s_statisticData.VODPacketsLostR2++;
        else if(rate >= s_statisticCfg.PacketsLostR3From && rate <= s_statisticCfg.PacketsLostR3Till)
            s_statisticData.VODPacketsLostR3++;
        else if(rate >= s_statisticCfg.PacketsLostR4From && rate <= s_statisticCfg.PacketsLostR4Till)
            s_statisticData.VODPacketsLostR4++;
        else if(rate >= s_statisticCfg.PacketsLostR5From && rate <= s_statisticCfg.PacketsLostR5Till)
            s_statisticData.VODPacketsLostR5++;
    }

    pthread_mutex_unlock(s_mutex);

    app_reportPacketsLost(rate / 10);
#endif
}
void
StatisticHWPlay::statisticStart(void)
{
#ifdef Sichuan
  if (!tr069StatisticGetLogenable())
    return;
#endif

    memset(&s_statisticData, 0, sizeof(struct StatisticData));

    pthread_mutex_lock(StatisticBase::s_mutex);
    if(tr069StatisticGetLogUploadInterval() < STATISTIC_PERIODIC_MIN)
#ifdef HUAWEI_C20
       tr069StatisticSetLogUploadInterval(STATISTIC_PERIODIC_MIN);
#else
       tr069StatisticSetLogRecordInterval(STATISTIC_PERIODIC_MIN);
#endif
    mid_timer_delete((mid_timer_f)tr069StatisticPost, 0);
    mid_timer_create(tr069StatisticGetLogUploadInterval(), 0, (mid_timer_f)tr069StatisticPost, 0);// zhangl test getStatisticLogUploadInterval()

    if(tr069StatisticGetLogRecordInterval() < STATISTIC_PERIODIC_MIN)
        tr069StatisticSetLogRecordInterval(STATISTIC_PERIODIC_MIN);
#ifdef HUAWEI_C20
    mid_timer_create(tr069StatisticGetLogUploadInterval(), 0, (mid_timer_f)tr069StatisticPeriodStart, 0);//zhangl test getStatisticLogUploadInterval()
#else
    mid_timer_create(tr069StatisticGetLogRecordInterval(), 0, (mid_timer_f)tr069StatisticPeriodStart, 0);//zhangl test getStatisticLogRecordInterval()
#endif

    pthread_mutex_unlock(StatisticBase::s_mutex);
    return;
}

void
StatisticHWPlay::statisticPeriod(void) //
{
    pthread_mutex_lock(s_mutex);
    s_statisticData.Startpoint = 0;
    StatisticBase::s_startClock = mid_10ms( );
    LogTr069Debug("s_Startpoint(%u)\n", s_statisticData.Startpoint);
    pthread_mutex_unlock(s_mutex);
    return;
}

/*************************************************
Description: 调用打包统计数据函数并上传
Return:
modify:2013.12 ,原来是函数app_statistic_proc_post_ctc，发现无论是ctc还是cu都是用的这个函数，
       因此转移到hw通用，如果ctc、cu有特别需求可在他们的派生类重写
 *************************************************/
void
StatisticHWPlay::statisticPost()
{
    int flag = 0;
    sysSettingGetInt("logSend", &flag, 0);
    if(!flag)
        return;

    //add by  zhangmin  stanby 不上报日志
    if(mid_fpanel_standby_get() == 1) {
        LogTr069Debug("statisticPost now the stb is stanby ,i can't play the buf\n"); //
        return ;
    }

    statisticFile(); //收集统计信息   存放在 s_packBuf
    LogTr069Debug("-----s_packBuf[%s]\n", s_packBuf);
    getStatisticLog()->upload(s_packBuf); //上传文件，失败处理

    return;
}

/*************************************************
Description:
Return:
 *************************************************/
void
StatisticHWPlay::statisticFile()
{
    int len = 0, i = 0, idx = 0, num = 0;
    char buf[32] = {0};
    float rate = 0;

    char ifname[URL_LEN] = { 0 };
    char ifaddr[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);

    if(pthread_mutex_lock(Hippo::UltraPlayer::s_statistic.getMutex()))
        return;

    if(pthread_mutex_lock(StatisticBase::s_mutex))
        return;

    s_statisticData.HTTPReqNumbers = BrowserStatisticHTTPReqNumbersGet();
    s_statisticData.AuthNumbers = jseAuthCountGet();
    s_statisticData.AuthFailNumbers = jseAuthFailCountGet();
    s_statisticData.HTTPFailNumbers = BrowserStatisticHTTPFailNumbersGet();
    BrowserStatisticHTTPFailInfoGet(s_statisticData.HTTPFailInfo, STATISTIC_INFO_FAID_NUM * STATISTIC_INFO_FAID_SIZE);
    jseAuthFailInfoGet(s_statisticData.AuthFailInfo, STATISTIC_INFO_FAID_NUM * STATISTIC_INFO_FAID_SIZE);

    if(s_statisticData.RebootFlag == 0) {
        s_statisticData.Endpoint = statisticLocakTime( );
        s_statisticData.Startpoint = s_statisticData.Endpoint - tr069StatisticGetLogRecordInterval();
    }
    char tBuf[256] = {'\0'};
    appSettingGetString("ntvuser", tBuf, USER_LEN, 0);
    len = strlen(tBuf);

    {
        mid_tool_time2string(s_statisticData.Startpoint, tBuf + len + 15, 0);
        u_int addr = inet_addr(network_address_get(ifname, ifaddr, URL_LEN));
        snprintf(tBuf + len, LOG_SERVER_URL_SIZE - len, "_%03d-%03d-%03d-%03d",
                 addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff, (addr >> 24) & 0xff);
        len += 16;
        tBuf[len] = '_';
        len += 11;
        if((unsigned int)(s_statisticData.Endpoint - s_statisticData.Startpoint) < 9999 && (unsigned int)(s_statisticData.Endpoint - s_statisticData.Startpoint) > 0)
            snprintf(tBuf + len, LOG_SERVER_URL_SIZE - len, "_%04d.csv", s_statisticData.Endpoint - s_statisticData.Startpoint);
        else
#ifdef HUAWEI_C20
            snprintf(tBuf + len, LOG_SERVER_URL_SIZE - len, "_%04d.csv", tr069StatisticGetLogUploadInterval());
#else
            snprintf(tBuf + len, LOG_SERVER_URL_SIZE - len, "_%04d.csv", tr069StatisticGetLogRecordInterval());
#endif
        tr069StatisticSetLogFileName(tBuf);
        //LogTr069Debug("tBuf = %s\n", tBuf);
    }

    len = 0;
    mid_tool_time2string((int)s_statisticData.Startpoint, buf, 0);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "Startpoint,%s\n", buf);
    mid_tool_time2string((int)s_statisticData.Endpoint, buf, 0);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "Endpoint,%s\n", buf);

    //è??¤
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "AuthNumbers,%d\n", s_statisticData.AuthNumbers);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "AuthFailNumbers,%d\n", s_statisticData.AuthFailNumbers);
    if(s_statisticData.AuthFailNumbers <= 0) {
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "AuthFailInfo,\n");
    } else {
        if(s_statisticData.AuthFailNumbers <= STATISTIC_INFO_FAID_NUM) {
            idx = 0;
            num = s_statisticData.AuthFailNumbers;
        } else {
            idx = s_statisticData.AuthFailNumbers - STATISTIC_INFO_FAID_NUM;
            num = STATISTIC_INFO_FAID_NUM;
        }
        for(i = idx; i < num; i++)
            len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "AuthFailInfo,%s\n", s_statisticData.AuthFailInfo[i % STATISTIC_INFO_FAID_NUM]);
    }
    if((rate = getRateInfo(s_statisticData.AuthFailNumbers, s_statisticData.AuthNumbers)) != 0.0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "AuthSucceRate,%.2f\n", rate);
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "AuthSucceRate,-1\n");

    LogTr069Debug("BootUpTime=%d\n", StatisticBase::s_bootUptime);
    if(StatisticBase::s_bootUptime != 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "BootUpTime,%d\n", StatisticBase::s_bootUptime);
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "BootUpTime,\n");
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "PowerOnTime, %s\n", StatisticBase::s_powerOnTime);

    //×é2￥
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiReqNumbers,%d\n", Hippo::UltraPlayerMultiple::s_statistic.getRequestNum());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiFailNumbers,%d\n", Hippo::UltraPlayerMultiple::s_statistic.getFailedNum());
    if(Hippo::UltraPlayerMultiple::s_statistic.getFailedNum() <= 0) {
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiFailInfo,\n");
    } else {
        if(Hippo::UltraPlayerMultiple::s_statistic.getFailedNum() <= STATISTIC_INFO_FAID_NUM) {
            idx = 0;
            num = Hippo::UltraPlayerMultiple::s_statistic.getFailedNum();
        } else {
            idx = Hippo::UltraPlayerMultiple::s_statistic.getFailedNum() - STATISTIC_INFO_FAID_NUM;
            num = STATISTIC_INFO_FAID_NUM;
        }
        for(i = idx; i < num; i++)
            len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiFailInfo,%s\n", Hippo::UltraPlayerMultiple::s_statistic.getFailedInfo(i % STATISTIC_INFO_FAID_NUM));
    }
    if((rate = getRateInfo(Hippo::UltraPlayerMultiple::s_statistic.getFailedNum(), Hippo::UltraPlayerMultiple::s_statistic.getRequestNum())) != 0.0  || (Hippo::UltraPlayerMultiple::s_statistic.getRequestNum() != 0))
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiSucceRate,%.2f\n", rate);
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiSucceRate,-1\n");

    //VOD
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodReqNumbers,%d\n", Hippo::UltraPlayerVod::s_statistic.getRequestNum());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodFailNumbers,%d\n", Hippo::UltraPlayerVod::s_statistic.getFailedNum());
    if(Hippo::UltraPlayerVod::s_statistic.getFailedNum() <= 0) {
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodFailInfo,\n");
    } else {
        if(Hippo::UltraPlayerVod::s_statistic.getFailedNum() <= STATISTIC_INFO_FAID_NUM) {
            idx = 0;
            num = Hippo::UltraPlayerVod::s_statistic.getFailedNum();
        } else {
            idx = Hippo::UltraPlayerVod::s_statistic.getFailedNum() - STATISTIC_INFO_FAID_NUM;
            num = STATISTIC_INFO_FAID_NUM;
        }
        for(i = idx; i < num; i++)
            len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodFailInfo,%s\n", Hippo::UltraPlayerVod::s_statistic.getFailedInfo(i % STATISTIC_INFO_FAID_NUM));
    }
    if((rate = getRateInfo(Hippo::UltraPlayerVod::s_statistic.getFailedNum(), Hippo::UltraPlayerVod::s_statistic.getRequestNum())) != 0.0 || (Hippo::UltraPlayerVod::s_statistic.getRequestNum() != 0))
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodSucceRate,%.2f\n", rate);
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodSucceRate,-1\n");

    //HTTP
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HTTPReqNumbers,%d\n", s_statisticData.HTTPReqNumbers);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HTTPFailNumbers,%d\n", s_statisticData.HTTPFailNumbers);
    if(s_statisticData.HTTPFailNumbers <= 0) {
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HTTPFailInfo,\n");
    } else {
        if(s_statisticData.HTTPFailNumbers <= STATISTIC_INFO_FAID_NUM) {
            idx = 0;
            num = s_statisticData.HTTPFailNumbers;
        } else {
            idx = s_statisticData.HTTPFailNumbers - STATISTIC_INFO_FAID_NUM;
            num = STATISTIC_INFO_FAID_NUM;
        }
        for(i = idx; i < num; i++)
            len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HTTPFailInfo,%s\n", s_statisticData.HTTPFailInfo[i % STATISTIC_INFO_FAID_NUM]);
    }
    if((rate = getRateInfo(s_statisticData.HTTPFailNumbers, s_statisticData.HTTPReqNumbers)) != 0.0 || (s_statisticData.HTTPReqNumbers != 0))
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HTTPSucceRate,%.2f\n", rate);
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HTTPSucceRate,-1\n");

    if(Hippo::UltraPlayerMultiple::s_statistic.getUnderflowNum() == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MutiAbendNumbers,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MutiAbendNumbers,%d\n", Hippo::UltraPlayerMultiple::s_statistic.getUnderflowNum());
    if(Hippo::UltraPlayerVod::s_statistic.getUnderflowNum() == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODAbendNumbers,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODAbendNumbers,%d\n", Hippo::UltraPlayerVod::s_statistic.getUnderflowNum());

    //playerror
    if(Hippo::UltraPlayer::s_statistic.getplayErrorNumbers() == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "PlayErrorNumbers,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "PlayErrorNumbers,%d\n", Hippo::UltraPlayer::s_statistic.getplayErrorNumbers());
    if(Hippo::UltraPlayer::s_statistic.getplayErrorNumbers() <= 0) {
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "PlayErrorInfo,\n");
    } else {
        if(Hippo::UltraPlayer::s_statistic.getplayErrorNumbers() <= STATISTIC_INFO_FAID_NUM) {
            idx = 0;
            num = Hippo::UltraPlayer::s_statistic.getplayErrorNumbers();
        } else {
            idx = Hippo::UltraPlayer::s_statistic.getplayErrorNumbers() - STATISTIC_INFO_FAID_NUM;
            num = STATISTIC_INFO_FAID_NUM;
        }
        for(i = idx; i < num; i++)
            len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "PlayErrorInfo,%s\n", Hippo::UltraPlayer::s_statistic.getPlayErrorInfo(i % STATISTIC_INFO_FAID_NUM));
    }

    //screenMosaic
    if(s_statisticData.ScreenMosaicEvent == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ScreenMosaicEvent,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ScreenMosaicEvent,%d\n", s_statisticData.ScreenMosaicEvent);

    if(s_statisticData.ScreenMosaicLastPeriod == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ScreenMosaicLastPeriod,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ScreenMosaicLastPeriod,%d\n", s_statisticData.ScreenMosaicLastPeriod);

    //playerror
    if(Hippo::UltraPlayer::s_statistic.getStreamGapNumbers() == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "StreamGapNumbers, -1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "StreamGapNumbers,%d\n", Hippo::UltraPlayer::s_statistic.getStreamGapNumbers());
    if(Hippo::UltraPlayer::s_statistic.getStreamGapNumbers() <= 0) {
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "StreamGapEvent,\n");
    } else {
        if(Hippo::UltraPlayer::s_statistic.getStreamGapNumbers() <= STATISTIC_INFO_FAID_NUM) {
            idx = 0;
            num = Hippo::UltraPlayer::s_statistic.getStreamGapNumbers();
        } else {
            idx = Hippo::UltraPlayer::s_statistic.getStreamGapNumbers() - STATISTIC_INFO_FAID_NUM;
            num = STATISTIC_INFO_FAID_NUM;
        }
        for(i = idx; i < num; i++)
            len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "StreamGapEvent,%s\n", Hippo::UltraPlayer::s_statistic.getStreamGapEvent(i % STATISTIC_INFO_FAID_NUM));
    }


    //D??ü
    int perform = 0;
    if(Hippo::UltraPlayerVod::s_statistic.getRequestNum() != 0)
        perform = Hippo::UltraPlayerVod::s_statistic.getTotalRequestReactTime() / Hippo::UltraPlayerVod::s_statistic.getRequestNum();
    if((rate = getPerform(perform)) == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodReqPerform,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodReqPerform,%.2f\n", rate);

    if((rate = getPerform(Hippo::UltraPlayerVod::s_statistic.getReqBestPerform())) == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodReqBestPerform,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodReqBestPerform,%.2f\n", rate);

    if((rate = getPerform(Hippo::UltraPlayerVod::s_statistic.getReqWorstPerform())) == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodReqWorstPerform,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodReqWorstPerform,%.2f\n", getPerform(Hippo::UltraPlayerVod::s_statistic.getReqWorstPerform()));

    //vod stop
    perform = 0;
    if(Hippo::UltraPlayerVod::s_statistic.getStopNum() != 0)
        perform = Hippo::UltraPlayerVod::s_statistic.getTotalChangeStopTime() /  Hippo::UltraPlayerVod::s_statistic.getStopNum();
    if((rate = getPerform(perform)) == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodStopPerform,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodStopPerform,%.2f\n", getPerform(perform));

    //vod pause
    perform = 0;
    if(Hippo::UltraPlayerVod::s_statistic.getPauseNum() != 0)
        perform = Hippo::UltraPlayerVod::s_statistic.getTotalChangePauseTime() /  Hippo::UltraPlayerVod::s_statistic.getPauseNum();
    if((rate = getPerform(perform)) == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodPausePerform,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodPausePerform,%.2f\n", getPerform(perform));


    //VodResumNum
    perform = 0;
    if(Hippo::UltraPlayerVod::s_statistic.getResumNum() != 0)
        perform = Hippo::UltraPlayerVod::s_statistic.getTotalChangeResumTime() / Hippo::UltraPlayerVod::s_statistic.getResumNum();
    if((rate = getPerform(perform)) == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodResumPerform,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodResumPerform,%.2f\n", getPerform(perform));
    //VodFFNum
    perform = 0;
    if(Hippo::UltraPlayerVod::s_statistic.getFastForwardNum() != 0)
        perform = Hippo::UltraPlayerVod::s_statistic.getTotalChangeFastForwardTime() /  Hippo::UltraPlayerVod::s_statistic.getFastForwardNum();
    if((rate = getPerform(perform)) == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodFFPerform,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodFFPerform,%.2f\n", getPerform(perform));

    //VodFBNum
    perform = 0;
    if(Hippo::UltraPlayerVod::s_statistic.getFastRewindNum() != 0)
        perform = Hippo::UltraPlayerVod::s_statistic.getTotalChangeFastRewindTime() /  Hippo::UltraPlayerVod::s_statistic.getFastRewindNum();
    if((rate = getPerform(perform)) == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodFBPerform,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodFBPerform,%.2f\n", getPerform(perform));

    //Channel
    perform = 0;
    if(Hippo::UltraPlayer::s_statistic.getChannelSwitchNumbers() != 0)
        perform = Hippo::UltraPlayer::s_statistic.getTotalChannelSwitchTime() /  Hippo::UltraPlayer::s_statistic.getChannelSwitchNumbers();
    if(perform == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ChannelZappingPerform,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ChannelZappingPerform,%.2f\n", getPerform(perform));

    if(Hippo::UltraPlayer::s_statistic.getChannelSwitchBestTime() == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ChannelZappingBestPerform,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ChannelZappingBestPerform,%.2f\n", getPerform(Hippo::UltraPlayer::s_statistic.getChannelSwitchBestTime()));

    if(Hippo::UltraPlayer::s_statistic.getChannelSwitchWorstTime() == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ChannelZappingWorstPerform,-1\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ChannelZappingWorstPerform,%.2f\n", getPerform(Hippo::UltraPlayer::s_statistic.getChannelSwitchWorstTime()));


    //multipackeslostr1
    if(s_statisticData.MultiPacketsLostR1 == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR1,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR1,%d\n", s_statisticData.MultiPacketsLostR1);
    if(s_statisticData.MultiPacketsLostR2 == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR2,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR2,%d\n", s_statisticData.MultiPacketsLostR2);
    if(s_statisticData.MultiPacketsLostR3 == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR3,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR3,%d\n", s_statisticData.MultiPacketsLostR3);
    if(s_statisticData.MultiPacketsLostR4 == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR4,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR4,%d\n", s_statisticData.MultiPacketsLostR4);
    if(s_statisticData.MultiPacketsLostR5 == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR5,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR5,%d\n", s_statisticData.MultiPacketsLostR5);


    if(s_statisticData.VODPacketsLostR1 == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR1,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR1,%d\n", s_statisticData.VODPacketsLostR1);
    if(s_statisticData.VODPacketsLostR2 == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR2,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR2,%d\n", s_statisticData.VODPacketsLostR2);
    if(s_statisticData.VODPacketsLostR3 == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR3,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR3,%d\n", s_statisticData.VODPacketsLostR3);
    if(s_statisticData.VODPacketsLostR4 == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR4,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR4,%d\n", s_statisticData.VODPacketsLostR4);
    if(s_statisticData.VODPacketsLostR5 == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR5,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR5,%d\n", s_statisticData.VODPacketsLostR5);
    if(s_statisticData.MultiBitRateR1Nmb == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR1,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR1,%d\n", s_statisticData.MultiBitRateR1Nmb);
    if(s_statisticData.MultiBitRateR2Nmb == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR2,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR2,%d\n", s_statisticData.MultiBitRateR2Nmb);
    if(s_statisticData.MultiBitRateR3Nmb == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR3,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR3,%d\n", s_statisticData.MultiBitRateR3Nmb);
    if(s_statisticData.MultiBitRateR4Nmb == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR4,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR4,%d\n", s_statisticData.MultiBitRateR4Nmb);
    if(s_statisticData.MultiBitRateR5Nmb == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR5,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR5,%d\n", s_statisticData.MultiBitRateR5Nmb);

    if(s_statisticData.VODBitRateR1Nmb == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR1,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR1,%d\n", s_statisticData.VODBitRateR1Nmb);
    if(s_statisticData.VODBitRateR2Nmb == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR2,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR2,%d\n", s_statisticData.VODBitRateR2Nmb);
    if(s_statisticData.VODBitRateR3Nmb == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR3,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR3,%d\n", s_statisticData.VODBitRateR3Nmb);
    if(s_statisticData.VODBitRateR4Nmb == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR4,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR4,%d\n", s_statisticData.VODBitRateR4Nmb);
    if(s_statisticData.VODBitRateR5Nmb == 0)
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR5,\n");
    else
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR5,%d\n", s_statisticData.VODBitRateR5Nmb);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR1Nmb,\n");
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR2Nmb,\n");
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR3Nmb,\n");
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR4Nmb,\n");
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR5Nmb,\n");
    s_packBuf[STATISTIC_FILE_SIZE] = 0;

    pthread_mutex_unlock(StatisticBase::s_mutex);
    pthread_mutex_unlock(Hippo::UltraPlayer::s_statistic.getMutex());

    return;
}

/*************************************************
Description: 统计模块初始化的时候调用，读取yx_config_statistic.ini配置文件，初始化统计参数
Return:
 *************************************************/
 // tanf todo:有统计回调函数待处理
void
StatisticHWPlay::sysCfgInit(void)
{

    if(g_cfgTree)
        return ;

    g_cfgTree = ind_cfg_create();
    if(g_cfgTree == NULL) {
        LogTr069Error("tree_cfg_create\n");
        return;
    }

    ind_cfg_inset_object(g_cfgTree, "statistic");

    ind_cfg_inset_string(g_cfgTree, "statistic.LogServerUrl",    tr069StatisticGetLogServerUrlAddr(), LOG_SERVER_URL_SIZE);

    ind_cfg_inset_int(g_cfgTree, "statistic.LogUploadInterval",  getStatisticLog()->getUploadIntervalAddr());
    ind_cfg_inset_int(g_cfgTree, "statistic.LogRecordInterval",  getStatisticLog()->getRecordIntervalAddr());
    ind_cfg_inset_int(g_cfgTree, "statistic.PacketsLostInterval",  (int*)&s_statisticCfg.PacketsLostInterval);
    ind_cfg_inset_int(g_cfgTree, "statistic.FramLostInterval", (int*)&s_statisticCfg.FramLostInterval);
    ind_cfg_inset_int(g_cfgTree, "statistic.BitRateInterval",  (int*)&s_statisticCfg.BitRateInterval);
#if defined(Sichuan)
#else

    ind_cfg_inset_int(g_cfgTree, "statistic.PacketsLostR1From",  &s_statisticCfg.PacketsLostR1From);
    ind_cfg_inset_int(g_cfgTree, "statistic.PacketsLostR1Till",  &s_statisticCfg.PacketsLostR1Till);
    ind_cfg_inset_int(g_cfgTree, "statistic.PacketsLostR2From",  &s_statisticCfg.PacketsLostR2From);
    ind_cfg_inset_int(g_cfgTree, "statistic.PacketsLostR2Till",  &s_statisticCfg.PacketsLostR2Till);
    ind_cfg_inset_int(g_cfgTree, "statistic.PacketsLostR3From",  &s_statisticCfg.PacketsLostR3From);
    ind_cfg_inset_int(g_cfgTree, "statistic.PacketsLostR3Till",  &s_statisticCfg.PacketsLostR3Till);
    ind_cfg_inset_int(g_cfgTree, "statistic.PacketsLostR4From",  &s_statisticCfg.PacketsLostR4From);
    ind_cfg_inset_int(g_cfgTree, "statistic.PacketsLostR4Till",  &s_statisticCfg.PacketsLostR4Till);
    ind_cfg_inset_int(g_cfgTree, "statistic.PacketsLostR5From",  &s_statisticCfg.PacketsLostR5From);
    ind_cfg_inset_int(g_cfgTree, "statistic.PacketsLostR5Till",  &s_statisticCfg.PacketsLostR5Till);

    ind_cfg_inset_int(g_cfgTree, "statistic.BitRateR1From",  &s_statisticCfg.BitRateR1From);
    ind_cfg_inset_int(g_cfgTree, "statistic.BitRateR1Till",  &s_statisticCfg.BitRateR1Till);
    ind_cfg_inset_int(g_cfgTree, "statistic.BitRateR2From",  &s_statisticCfg.BitRateR2From);
    ind_cfg_inset_int(g_cfgTree, "statistic.BitRateR2Till",  &s_statisticCfg.BitRateR2Till);
    ind_cfg_inset_int(g_cfgTree, "statistic.BitRateR3From",  &s_statisticCfg.BitRateR3From);
    ind_cfg_inset_int(g_cfgTree, "statistic.BitRateR3Till",  &s_statisticCfg.BitRateR3Till);
    ind_cfg_inset_int(g_cfgTree, "statistic.BitRateR4From",  &s_statisticCfg.BitRateR4From);
    ind_cfg_inset_int(g_cfgTree, "statistic.BitRateR4Till",  &s_statisticCfg.BitRateR4Till);
    ind_cfg_inset_int(g_cfgTree, "statistic.BitRateR5From",  &s_statisticCfg.BitRateR5From);
    ind_cfg_inset_int(g_cfgTree, "statistic.BitRateR5Till",  &s_statisticCfg.BitRateR5Till);
#endif
    ind_cfg_inset_int(g_cfgTree, "statistic.fail_log_num", getStatisticLog()->getFailNumAddr());

    settingConfigRead(g_cfgTree, "statistic");

#ifdef INCLUDE_TR069_CTC
    mid_http_infoCall(tr069LogMsgPostHTTPInfo);
    mid_stream_callback_rtspinfo(tr069LogMsgPostRTSPInfo);
#if defined(Sichuan)
    mid_stream_callback_statflow(stream_port_post_flow);
    //mid_http_statCall(stream_port_post_http_rrt, stream_port_post_http_fail); 2014-4-1 14:56:57 by liujianhua 暂时屏蔽解决编译不通过
#if defined(INCLUDE_SQM) && defined(TR069_MONITOR)
    mid_timer_create(300, 0, monitor_sqm_data, 0);
#endif //endif INCLUDE_SQM TR069_MONITOR
#endif//Sichuan
#endif//INCLUDE_TR069_CTC
    if(getStatisticLog()->getFailNum() > MAX_FILE_LOG_NUM)
        getStatisticLog()->setFailNum(MAX_FILE_LOG_NUM);
    if(strlen(getStatisticLog()->getServerUrlAddr()) == 0)
#if defined(GUANGDONG)
      getStatisticLog()->setServerUrl("");
#else
      getStatisticLog()->setServerUrl("ftp://IP:PORT@USR&PWD");
#endif//GUANGDONG

    return;
}

/*************************************************
Description: 重置统计参数
 *************************************************/
void
StatisticHWPlay::statisticCfgReset(void)
{
    if(pthread_mutex_lock(s_mutex))
        return;

    getStatisticLog()->configReset();
    s_statisticCfg.PacketsLostInterval = 5;
    s_statisticCfg.FramLostInterval = 10;
    s_statisticCfg.BitRateInterval = 2;
#if defined(Sichuan)

    s_statisticCfg.PacketsLostR1 = 0;
    s_statisticCfg.PacketsLostR2 = 1;
    s_statisticCfg.PacketsLostR3 = 1;
    s_statisticCfg.PacketsLostR4 = 3;
    s_statisticCfg.PacketsLostR5 = 0xFF;

    s_statisticCfg.HD_PacketsLostR1 = 0;
    s_statisticCfg.HD_PacketsLostR2 = 1;
    s_statisticCfg.HD_PacketsLostR3 = 1;
    s_statisticCfg.HD_PacketsLostR4 = 3;
    s_statisticCfg.HD_PacketsLostR5 = 0xFF;

    s_statisticCfg.BitRateR1 = 100;
    s_statisticCfg.BitRateR2 = 96;
    s_statisticCfg.BitRateR3 = 92;
    s_statisticCfg.BitRateR4 = 88;
    s_statisticCfg.BitRateR5 = 88;

    s_statisticCfg.HD_BitRateR1 = 100;
    s_statisticCfg.HD_BitRateR2 = 96;
    s_statisticCfg.HD_BitRateR3 = 92;
    s_statisticCfg.HD_BitRateR4 = 88;
    s_statisticCfg.HD_BitRateR5 = 88;

    s_statisticCfg.FrameLostR1 = 0;
    s_statisticCfg.FrameLostR2 = 3;
    s_statisticCfg.FrameLostR3 = 10;
    s_statisticCfg.FrameLostR4 = 20;
    s_statisticCfg.FrameLostR5 = 0xFF;
#else

    s_statisticCfg.PacketsLostR1From = 0;
    s_statisticCfg.PacketsLostR1Till = 0;
    s_statisticCfg.PacketsLostR2From = 0;
    s_statisticCfg.PacketsLostR2Till = 10;
    s_statisticCfg.PacketsLostR3From = 10;
    s_statisticCfg.PacketsLostR3Till = 20;
    s_statisticCfg.PacketsLostR4From = 20;
    s_statisticCfg.PacketsLostR4Till = 50;
    s_statisticCfg.PacketsLostR5From = 50;
    s_statisticCfg.PacketsLostR5Till = 9999;

    s_statisticCfg.PacketsLostR1 = 0;
    s_statisticCfg.PacketsLostR2 = 1;
    s_statisticCfg.PacketsLostR3 = 1;
    s_statisticCfg.PacketsLostR4 = 2;
    s_statisticCfg.PacketsLostR5 = 0xFF;

    s_statisticCfg.BitRateR1 = 100;
    s_statisticCfg.BitRateR2 = 96;
    s_statisticCfg.BitRateR3 = 92;
    s_statisticCfg.BitRateR4 = 88;
    s_statisticCfg.BitRateR5 = 88;

    s_statisticCfg.FrameLostR1 = 0;
    s_statisticCfg.FrameLostR2 = 3;
    s_statisticCfg.FrameLostR3 = 10;
    s_statisticCfg.FrameLostR4 = 20;
    s_statisticCfg.FrameLostR5 = 0xFF;

#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)
    s_statisticCfg.BitRateR1From = 100;
    s_statisticCfg.BitRateR1Till = 9999;
    s_statisticCfg.BitRateR2From = 96;
    s_statisticCfg.BitRateR2Till = 100;
    s_statisticCfg.BitRateR3From = 92;
    s_statisticCfg.BitRateR3Till = 96;
    s_statisticCfg.BitRateR4From = 88;
    s_statisticCfg.BitRateR4Till = 92;
    s_statisticCfg.BitRateR5From = 0;
    s_statisticCfg.BitRateR5Till = 88;
#else
    s_statisticCfg.BitRateR1From = 16;
    s_statisticCfg.BitRateR1Till = 9999;
    s_statisticCfg.BitRateR2From = 14;
    s_statisticCfg.BitRateR2Till = 16;
    s_statisticCfg.BitRateR3From = 12;
    s_statisticCfg.BitRateR3Till = 14;
    s_statisticCfg.BitRateR4From = 8;
    s_statisticCfg.BitRateR4Till = 12;
    s_statisticCfg.BitRateR5From = 0;
    s_statisticCfg.BitRateR5Till = 8;
#endif
#endif
    s_statisticCfg.dirty = 1;

    pthread_mutex_unlock(s_mutex);
}

/*************************************************
Description: 把统计参数存储到文件yx_config_statistic.ini
 *************************************************/
void
StatisticHWPlay::statisticCfgSave(void)
{
    pthread_mutex_lock(s_mutex);
    if(s_statisticCfg.dirty == 1) {
        //LogTr069Debug("g_statisticConfig.LogServerUrl = %s\n", getServerUrlAddr());
        settingConfigWrite(g_cfgTree, "statistic");
        s_statisticCfg.dirty = 0;
    }
    pthread_mutex_unlock(s_mutex);
    return;
}
