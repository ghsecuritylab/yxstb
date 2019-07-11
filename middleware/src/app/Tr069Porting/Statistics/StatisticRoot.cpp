/*******************************************************************
*ͳ��ģ�����Ľӿ�ͳһ��������. 2013.12
*********************************************************************/
#include "StatisticRoot.h"

#include "StatisticHW/StatisticBase.h"
#include "StatisticHW/StatisticHWPlay.h"
#include "StatisticCTC/StatisticCTCPlay.h"
#include "StatisticCU/StatisticCUPlay.h"
#include "../../../Player/UltraPlayer.h"
#include "TR069Assertions.h"
#include "Jse/Hybroad/Business/JseBusiness.h"
#include "StatisticLog/StatisticLog.h"

#include "tr069_api.h"
#include "stream/rtsp/rtsp_app.h"; // for timer func to cal
#include "mid_task.h"
#include "mid/mid_timer.h"

#include "SysSetting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

enum {
    STAT_MSG_START = 0,
    STAT_MSG_PERIOD,
    STAT_MSG_POST
};

struct StatMsg {
    unsigned id;
};

static StatisticBase *g_statistic;
static mid_msgq_t g_msgq = 0;

#ifdef __cplusplus
extern "C" {
#endif

void stream_port_post_bitrate(int mult,int width,int bitrate)
{
    g_statistic->statisticBitrateRn(mult, width, bitrate);
}

void stream_port_post_bitrate_percent(int mult,int width,int bitpercent)
{
    g_statistic->statisticBitratePercentRn(mult, width, bitpercent);
}

void stream_port_post_pklosts(int flag, int width, int totals, int losts)
{
    g_statistic->statisticPacketLostsRn(flag, width, totals, losts);
}

void tr069StatisticPeriodStart(int arg)
{
    struct StatMsg msg;

    LogTr069Debug("tr069StatisticPeriodStart\n");

    int flag = 0;
    sysSettingGetInt("logSend", &flag, 0);
    if(!flag)
        return;
#ifdef Sichuan
    if (!tr069StatisticGetLogenable()) {
        mid_timer_delete( (mid_timer_f)tr069StatisticPeriodStart,0 );
        return;
    }
#endif
    msg.id = STAT_MSG_PERIOD;
    mid_msgq_put(g_msgq, (char*)(&msg), 0);
    return;
}

void tr069StatisticPost(int arg)
{
    struct StatMsg msg;
    int flag = 0;
    sysSettingGetInt("logSend", &flag, 0);
    LogTr069Debug("logsend flag = %d\n",flag);
    if (!flag)
        return;
#ifdef Sichuan
    if (!tr069StatisticGetLogenable()) {
        LogTr069Debug("Logenable = %d\n", tr069StatisticGetLogenable());
        mid_timer_delete( (mid_timer_f)tr069StatisticPost,0 );
        return;
    }
#endif
    msg.id = STAT_MSG_POST;
    mid_msgq_put(g_msgq, (char*)(&msg), 0);
}

static void appStatisticTask(void*)
{
    static unsigned int usedTime = 0;

    for(;;) {
        unsigned int clk = mid_10ms( );

        static int fd;
        static fd_set rfds;
        static struct timeval tv;
        static struct StatMsg msg;
        static unsigned int clks;

        fd = mid_msgq_fd(g_msgq);

        tv.tv_sec = 3600 - usedTime; // �趨˯��ʱ��
        tv.tv_usec = 0;

        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        if(select(fd + 1, &rfds , NULL,  NULL, &tv) <= 0)
            continue;

        unsigned int clk1 = mid_10ms( );
        usedTime += (clk1 - clk ) / 100;
        if (usedTime >= 3600)
            usedTime = 0;

        if(FD_ISSET(fd, &rfds)) { // ��ô������ʱ�ϴ���
            mid_msgq_get(g_msgq, (char *)(&msg), 0, 0);
            switch(msg.id) {
            case STAT_MSG_START:
                LogTr069Debug("STAT_MSG_START\n");
                g_statistic->statisticStart();
                break;
            case STAT_MSG_PERIOD:
                LogTr069Debug("STAT_MSG_PERIOD\n");
                g_statistic->statisticPeriod();
                break;
            case STAT_MSG_POST:
                LogTr069Debug("STAT_MSG_POST\n");
                g_statistic->statisticPost();
                break;
            default:
                LogTr069Debug("msg.id = %d\n", msg.id);
                break;
            }
            continue;
        }
    }

return;
}

int tr069StatisticConfigInit(void)
{
    int tr069Type = 0;

    if(g_statistic->getPackBuf() != NULL) {
        LogTr069Error("already init!\n");
        return -1;
    }

    g_statistic->packBufInit();
    g_statistic->mutexInit(); // ��ʼ��ͳ����
    Hippo::UltraPlayer::s_statistic.mutexInit(); //��ʼ��������

    if(g_statistic->getPackBuf() == NULL) {
        LogTr069Error("os_malloc STATISTIC_FILE_SIZE = %d failed\n", STATISTIC_FILE_SIZE);
        return -1;
    }

    if (g_statistic->s_mutex == NULL) {
        LogTr069Error("create Mutex failed\n");
        return -1;
    }

    if (Hippo::UltraPlayer::s_statistic.getMutex() == NULL) {
        LogTr069Error("create Mutex failed\n");
        return -1;
    }

    // ѡ����CTC����CU��HW������ʹ�ã�
    sysSettingGetInt("tr069_type", &tr069Type, 0);
    if (2 == tr069Type)
        g_statistic = new StatisticCUPlay();
    else if (1 == tr069Type)
        g_statistic = new StatisticCTCPlay();
    else
        g_statistic = new StatisticHWPlay();

    tr069StatisticLogInit(); // ��ʼ����־ģ��

    g_statistic->statisticCfgReset();

    // ��ȡyx_config_statistic.ini�����ļ�d����ʼ��ͳ�Ʋ���
    g_statistic->sysCfgInit();

    g_msgq = mid_msgq_create(10, sizeof(struct StatMsg));

    mid_stream_statint_pklosts(statisticGetPacketsLostInterval());
    mid_stream_statint_bitrate(statisticGetBitRateInterval());

    mid_task_create("mid_statistic", appStatisticTask, 0);

    return 0;
}

/****************************************************************
* modify log��2013.12
* ���������yx_config_statistic.ini�����ļ�
* �������ļ���Ϊ�˷����޸��ϴ���������ַ���ϴ����ڵȿ��ܱ��������õĲ���
* ��jse��mgmt,tr069,monitor�Ȼ��޸���Щ������ �ṩ�˽ӿ�������ı�
******************************************************************/
void tr069StatisticConfigSave(void)
{
    g_statistic->statisticCfgSave();
    return;
}

static int isStarted = 0;
void tr069StatisticStart(void)
{
    struct StatMsg msg;

    if(isStarted)
        return ;
    int flag = 0;
    sysSettingGetInt("logSend", &flag, 0);
    LogTr069Debug("[%d]\n", flag);
    if(!flag)
        return;
    StatisticBase::s_startFlag = 1;
    msg.id = STAT_MSG_START;
    mid_msgq_put(g_msgq, (char*)(&msg), 0);
    //��������������־��ַ���ԣ���������������
    //statisticStart();
    isStarted = 1;
    return;
}

/*******************************************************************
*  modify by zhangmin 3-24 2010
*  //flag = 0 ��ʾ������ falg = 1��ʾ�����ļ�
*  ȷ���ϴ��ɹ�ʱ�򣬲���д�ļ��������ļ�д����
*  modify log��2013.12
*  �ϴ�ʧ�ܣ�����ʱ����ͳ�ƺõ��������ļ��� ֮����ط�
*********************************************************************/
void tr069StatisticStore(void)
{
    tr069StatisticConfigSave();
}

//NTP ͬ��֮������ˢ�¿�ʼʱ�� (Startpoint , s_powerOnTime, Startpoint_clock(ɾ��))
void tr069StaticticPeriodRestart()
{
    LogTr069Debug("tr069StaticticPeriodRestart\n");
    StatisticBase::s_statisticData.Startpoint = 0;
    StatisticBase::s_startClock = mid_10ms();
    if(!statisticIsSetPowerOnTime()) { // ��һ�����ÿ���ʱ��
        statisticSetPowerOnTime();
        statisticSetIsPowerOnTime(1);
    }
    return;
}

/*******************************************************************
* �ϴ�һ��
*********************************************************************/
void tr069StaticticStandy()
{
    tr069StatisticPost(0);
    return;
}

/*******************************************************************
* modify log��2013.12
* ����ͳ�Ʋ���
*********************************************************************/
void tr069StatisticConfigReset(void)
{
    g_statistic->statisticCfgReset();
}

#ifdef __cplusplus
}
#endif


