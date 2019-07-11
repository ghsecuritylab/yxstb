#include "StatisticCTCPlay.h"

#include "../StatisticRoot.h"
#include "../StatisticHW/StatisticBase.h"
#include "UltraPlayer.h"
#include "UltraPlayerMultiple.h"
#include "UltraPlayerVod.h"
#include "Assertions.h"
#include "NetworkFunctions.h"
#include "takin.h"
#include "AppSetting.h"
#include "SysSetting.h"
#include "VersionSetting.h"
#include "SettingEnum.h"
#include "Jse/Huawei/Business/Session/JseHWSession.h"
#include "Jse/Hybroad/Business/JseBusiness.h"
#include "../StatisticLog/StatisticLog.h"
#include<arpa/inet.h>

#include "mid_sys.h"
#include "mid/mid_tools.h"
#include "sys_basic_macro.h"
#include "customer.h"

#include "app_jse.h"
#include "stream/rtsp/rtsp_app.h"; // for getRtsp();
#include "stream/rtsp/rtsp_stat.h" // for struct Rtsp

StatisticCTCPlay::StatisticCTCPlay()
{
}

StatisticCTCPlay::~StatisticCTCPlay()
{
}

/*************************************************
Description:
Return:
 *************************************************/
void
StatisticCTCPlay::statisticFile(void)
{
    int len = 0, i = 0, idx = 0, num = 0;
    char buf[32] = {0};
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

#ifdef Sichuan
    len = strlen(tBuf);
    network_tokenmac_get(tBuf + len, URL_LEN, 0);
    strcat(tBuf, "_");
#endif
    len = strlen(tBuf);
    appSettingGetString("ntvuser", tBuf + len, USER_LEN, 0);
#ifdef Sichuan
    char *p = tBuf + len;
    while (*p++) {
        if (*p == '@')
            *p = '_';
    }
#endif
    LogUserOperDebug("\n ntvuser[%s]-\n", tBuf);
    len = strlen(tBuf);

    u_int addr = inet_addr(network_address_get(ifname, ifaddr, URL_LEN));

    memset(buf, 0, 32);
    mid_tool_time2string(s_statisticData.Startpoint, buf, 0);
    buf[12] = '\0';

#if defined(HUAWEI_C10)
    len += snprintf(tBuf + len, LOG_SERVER_URL_SIZE - len, "_%03d-%03d-%03d-%03d_%s", addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff, (addr >> 24) & 0xff, buf);

    if( (unsigned int)(s_statisticData.Endpoint - s_statisticData.Startpoint) < 9999 && (unsigned int)(s_statisticData.Endpoint - s_statisticData.Startpoint) > 0 )
        snprintf(tBuf + len, LOG_SERVER_URL_SIZE - len, "_%06d.csv", tr069StatisticGetLogUploadInterval());
    else
        snprintf(tBuf + len, LOG_SERVER_URL_SIZE - len, "_%06d.csv", tr069StatisticGetLogRecordInterval());
#else
    len += snprintf(tBuf + len, LOG_SERVER_URL_SIZE - len, "_%03d-%03d-%03d-%03d_%s_%s", addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff, (addr >> 24) & 0xff, QOSVER, buf + 2);

    if((unsigned int)(s_statisticData.Endpoint - s_statisticData.Startpoint) < 9999 && (unsigned int)(s_statisticData.Endpoint - s_statisticData.Startpoint) > 0)
        snprintf(tBuf + len, LOG_SERVER_URL_SIZE - len, "_%06d.csv", s_statisticData.Endpoint - s_statisticData.Startpoint);
    else
        snprintf(tBuf + len, LOG_SERVER_URL_SIZE - len, "_%06d.csv", tr069StatisticGetLogRecordInterval());
#endif
    LogUserOperDebug("\n-tr069StatisticSetLogFileName-[%s]----\n", tBuf);
    tr069StatisticSetLogFileName(tBuf);
        //LogUserOperDebug("tBuf = %s\n", tBuf);
    len = 0;

    memset(buf, 0, 32);
    mid_tool_time2string((int)s_statisticData.Startpoint, buf, 0);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "Startpoint,%s\n", buf);

    mid_tool_time2string((int)s_statisticData.Endpoint, buf, 0);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "Endpoint,%s\n", buf);

    char stbid[40];
    char hardtype[12] = {0};
    memset(stbid, 0, 40);
    mid_sys_serial(stbid);
    HardwareVersion(hardtype, 12);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "STBID,%s\n", stbid);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "OUI,%s\n", "990060");

    //app_cfg_SoftwareVersion_get(tempVersion);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "SoftwareVersion,%d\n", upgrade_version_read(SOFTWARE_VERSION));
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HardwareVersion,%s\n", hardtype);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "AuthNumbers,%d\n", s_statisticData.AuthNumbers);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "AuthFailNumbers,%d\n", s_statisticData.AuthFailNumbers);
    if(s_statisticData.AuthFailNumbers <= 0) {
        ;   //len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "AuthFailInfo,0\n");
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

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiReqNumbers,%d\n", Hippo::UltraPlayerMultiple::s_statistic.getRequestNum());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiRRT,%d\n", Hippo::UltraPlayerMultiple::s_statistic.getRequestReactTime());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiFailNumbers,%d\n", Hippo::UltraPlayerMultiple::s_statistic.getFailedNum());
    if(Hippo::UltraPlayerMultiple::s_statistic.getFailedNum() <= 0) {
        ;   //len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiFailInfo,0\n");
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

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodReqNumbers,%d\n", Hippo::UltraPlayerVod::s_statistic.getRequestNum());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodRRT,%d\n", Hippo::UltraPlayerVod::s_statistic.getRequestReactTime());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodFailNumbers,%d\n", Hippo::UltraPlayerVod::s_statistic.getFailedNum());
    if(Hippo::UltraPlayerVod::s_statistic.getFailedNum() <= 0) {
        ;   //len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodFailInfo,0\n");
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

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HTTPReqNumbers,%d\n", s_statisticData.HTTPReqNumbers);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HTTPRRT,%d\n", s_statisticData.HTTPRRT);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HTTPFailNumbers,%d\n", s_statisticData.HTTPFailNumbers);
    if(s_statisticData.HTTPFailNumbers <= 0) {
        ;   //len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HTTPFailInfo,0\n");
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

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MutiAbendNumbers,%d\n", Hippo::UltraPlayerMultiple::s_statistic.getUnderflowNum());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODAbendNumbers,%d\n", Hippo::UltraPlayerVod::s_statistic.getUnderflowNum());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiAbendUPNumbers,%d\n", Hippo::UltraPlayerMultiple::s_statistic.getOverflowNum());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODUPAbendNumbers,%d\n", Hippo::UltraPlayerVod::s_statistic.getOverflowNum());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_MutiAbendNumbers,%d\n", Hippo::UltraPlayerMultiple::s_statistic.getHDUnderflowNum());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_VODAbendNumbers,%d\n", Hippo::UltraPlayerVod::s_statistic.getHDUnderflowNum());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_MultiUPAbendNumbers,%d\n", Hippo::UltraPlayerMultiple::s_statistic.getOverflowNum());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_VODUPAbendNumbers,%d\n", Hippo::UltraPlayerVod::s_statistic.getOverflowNum());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "PlayErrorNumbers,%d\n", Hippo::UltraPlayer::s_statistic.getplayErrorNumbers());

    if(Hippo::UltraPlayer::s_statistic.getplayErrorNumbers() <= 0) {
        ;   //len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "PlayErrorInfo,0\n");
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

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR1Nmb,%d\n", s_statisticData.MultiPacketsLostR1);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR2Nmb,%d\n", s_statisticData.MultiPacketsLostR2);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR3Nmb,%d\n", s_statisticData.MultiPacketsLostR3);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR4Nmb,%d\n", s_statisticData.MultiPacketsLostR4);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR5Nmb,%d\n", s_statisticData.MultiPacketsLostR5);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FECMultiPacketsLostR1Nmb,%d\n", s_statisticData.FECMultiPacketsLostR1Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FECMultiPacketsLostR2Nmb,%d\n", s_statisticData.FECMultiPacketsLostR2Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FECMultiPacketsLostR3Nmb,%d\n", s_statisticData.FECMultiPacketsLostR3Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FECMultiPacketsLostR4Nmb,%d\n", s_statisticData.FECMultiPacketsLostR4Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FECMultiPacketsLostR5Nmb,%d\n", s_statisticData.FECMultiPacketsLostR5Nmb);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR1Nmb,%d\n", s_statisticData.VODPacketsLostR1);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR2Nmb,%d\n", s_statisticData.VODPacketsLostR2);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR3Nmb,%d\n", s_statisticData.VODPacketsLostR3);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR4Nmb,%d\n", s_statisticData.VODPacketsLostR4);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR5Nmb,%d\n", s_statisticData.VODPacketsLostR5);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ARQVODPacketsLostR1Nmb,%d\n", s_statisticData.ARQVODPacketsLostR1Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ARQVODPacketsLostR2Nmb,%d\n", s_statisticData.ARQVODPacketsLostR2Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ARQVODPacketsLostR3Nmb,%d\n", s_statisticData.ARQVODPacketsLostR3Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ARQVODPacketsLostR4Nmb,%d\n", s_statisticData.ARQVODPacketsLostR4Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ARQVODPacketsLostR5Nmb,%d\n", s_statisticData.ARQVODPacketsLostR5Nmb);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR1Nmb,%d\n", s_statisticData.MultiBitRateR1Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR2Nmb,%d\n", s_statisticData.MultiBitRateR2Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR3Nmb,%d\n", s_statisticData.MultiBitRateR3Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR4Nmb,%d\n", s_statisticData.MultiBitRateR4Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR5Nmb,%d\n", s_statisticData.MultiBitRateR5Nmb);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR1Nmb,%d\n", s_statisticData.VODBitRateR1Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR2Nmb,%d\n", s_statisticData.VODBitRateR2Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR3Nmb,%d\n", s_statisticData.VODBitRateR3Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR4Nmb,%d\n", s_statisticData.VODBitRateR4Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR5Nmb,%d\n", s_statisticData.VODBitRateR5Nmb);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_MultiPacketsLostR1Nmb,%d\n", s_statisticData.HD_MultiPacketsLostR1Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_MultiPacketsLostR2Nmb,%d\n", s_statisticData.HD_MultiPacketsLostR2Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_MultiPacketsLostR3Nmb,%d\n", s_statisticData.HD_MultiPacketsLostR3Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_MultiPacketsLostR4Nmb,%d\n", s_statisticData.HD_MultiPacketsLostR4Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_MultiPacketsLostR5Nmb,%d\n", s_statisticData.HD_MultiPacketsLostR5Nmb);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_FECMultiPacketsLostR1Nmb,%d\n", s_statisticData.HD_FECMultiPacketsLostR1Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_FECMultiPacketsLostR2Nmb,%d\n", s_statisticData.HD_FECMultiPacketsLostR2Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_FECMultiPacketsLostR3Nmb,%d\n", s_statisticData.HD_FECMultiPacketsLostR3Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_FECMultiPacketsLostR4Nmb,%d\n", s_statisticData.HD_FECMultiPacketsLostR4Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_FECMultiPacketsLostR5Nmb,%d\n", s_statisticData.HD_FECMultiPacketsLostR5Nmb);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_VODPacketsLostR1Nmb,%d\n", s_statisticData.HD_VODPacketsLostR1Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_VODPacketsLostR2Nmb,%d\n", s_statisticData.HD_VODPacketsLostR2Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_VODPacketsLostR3Nmb,%d\n", s_statisticData.HD_VODPacketsLostR3Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_VODPacketsLostR4Nmb,%d\n", s_statisticData.HD_VODPacketsLostR4Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_VODPacketsLostR5Nmb,%d\n", s_statisticData.HD_VODPacketsLostR5Nmb);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_ARQVODPacketsLostR1Nmb,%d\n", s_statisticData.HD_ARQVODPacketsLostR1Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_ARQVODPacketsLostR2Nmb,%d\n", s_statisticData.HD_ARQVODPacketsLostR2Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_ARQVODPacketsLostR3Nmb,%d\n", s_statisticData.HD_ARQVODPacketsLostR3Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_ARQVODPacketsLostR4Nmb,%d\n", s_statisticData.HD_ARQVODPacketsLostR4Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_ARQVODPacketsLostR5Nmb,%d\n", s_statisticData.HD_ARQVODPacketsLostR5Nmb);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_MultiBitRateR1Nmb,%d\n", s_statisticData.HD_MultiBitRateR1Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_MultiBitRateR2Nmb,%d\n", s_statisticData.HD_MultiBitRateR2Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_MultiBitRateR3Nmb,%d\n", s_statisticData.HD_MultiBitRateR3Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_MultiBitRateR4Nmb,%d\n", s_statisticData.HD_MultiBitRateR4Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_MultiBitRateR5Nmb,%d\n", s_statisticData.HD_MultiBitRateR5Nmb);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_VODBitRateR1Nmb,%d\n", s_statisticData.HD_VODBitRateR1Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_VODBitRateR2Nmb,%d\n", s_statisticData.HD_VODBitRateR2Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_VODBitRateR3Nmb,%d\n", s_statisticData.HD_VODBitRateR3Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_VODBitRateR4Nmb,%d\n", s_statisticData.HD_VODBitRateR4Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HD_VODBitRateR5Nmb,%d\n", s_statisticData.HD_VODBitRateR5Nmb);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "BufferIncNmb,%d\n", Hippo::UltraPlayer::s_statistic.getBufferIncNmb());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "BufferDecNmb,%d\n", Hippo::UltraPlayer::s_statistic.getBufferDecNmb());

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR1Nmb,%d\n", s_statisticData.FrameLostR1);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR2Nmb,%d\n", s_statisticData.FrameLostR2);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR3Nmb,%d\n", s_statisticData.FrameLostR3);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR4Nmb,%d\n", s_statisticData.FrameLostR4);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR5Nmb,%d\n", s_statisticData.FrameLostR5);

    memset(s_statisticData.PPPoEID, 0, sizeof(s_statisticData.PPPoEID));
    memset(s_statisticData.PPPoEPassword, 0, sizeof(s_statisticData.PPPoEPassword));
    memset(s_statisticData.UserID, 0, sizeof(s_statisticData.UserID));
    memset(s_statisticData.UserIDPassword, 0, sizeof(s_statisticData.UserIDPassword));
    memset(s_statisticData.AuthURL, 0, sizeof(s_statisticData.AuthURL));
    memset(s_statisticData.ipaddress, 0, sizeof(s_statisticData.ipaddress));
    sysSettingGetString("netuser", s_statisticData.PPPoEID, sizeof(s_statisticData.PPPoEID), 0);
    //sys_passwd_get(s_statisticData.PPPoEPassword); // can not find this func
    appSettingGetString("ntvuser", s_statisticData.UserID, sizeof(s_statisticData.UserID), 0);
    //IND_MEMCPY(s_statisticData.UserIDPassword, sysNtvpasswdGet(), 16); // can not find this func
    sysSettingGetString("eds", s_statisticData.AuthURL, sizeof(s_statisticData.AuthURL), 0);
    network_address_get(ifname, s_statisticData.ipaddress, sizeof(s_statisticData.ipaddress));
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "PPPoEID,%s\n", s_statisticData.PPPoEID);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "PPPoEPassword,%s\n", s_statisticData.PPPoEPassword);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "UserID,%s\n", s_statisticData.UserID);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "UserIDPassword,%s\n", s_statisticData.UserIDPassword);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "AuthURL,%s\n", s_statisticData.AuthURL);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "local_ipaddress,%s\n", s_statisticData.ipaddress);

    s_packBuf[STATISTIC_FILE_SIZE] = 0;
    LogUserOperDebug("\n************CTC ftp log file*************\n%s\n",s_packBuf);
    pthread_mutex_unlock(StatisticBase::s_mutex);
    pthread_mutex_unlock(Hippo::UltraPlayer::s_statistic.getMutex());
}

