#include "StatisticCUPlay.h"

#include "../StatisticRoot.h"
#include "../StatisticHW/StatisticBase.h"
#include "UltraPlayer.h"
#include "UltraPlayerMultiple.h"
#include "UltraPlayerVod.h"
#include "Assertions.h"
#include "NetworkFunctions.h"
#include "takin.h"
#include "AppSetting.h"
#include "Jse/Huawei/Business/Session/JseHWSession.h"
#include "Jse/Hybroad/Business/JseBusiness.h"
#include "stbinfo/stbinfo.h"
#include "../StatisticLog/StatisticLog.h"
#include<arpa/inet.h>

#include "mid_sys.h"
#include "mid/mid_tools.h"
#include "sys_basic_macro.h"

#include "customer.h"

#include "app_jse.h"
#include "stream/rtsp/rtsp_app.h"; // for getRtsp();
#include "stream/rtsp/rtsp_stat.h" // for struct Rtsp

StatisticCUPlay::StatisticCUPlay()
{
}

StatisticCUPlay::~StatisticCUPlay()
{
}

/*************************************************
Description:
Return:
 *************************************************/
void
StatisticCUPlay::statisticFile(char *url)
{
    int len = 0, i, idx, num;
    char buf[32] = {0};
    u_int addr;
    char ifname[URL_LEN] = { 0 };
    char ifaddr[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);

    if (pthread_mutex_lock(s_mutex))
        return;

    if(pthread_mutex_lock(Hippo::UltraPlayer::s_statistic.getMutex()))
        return;

        s_statisticData.HTTPReqNumbers = BrowserStatisticHTTPReqNumbersGet();
        s_statisticData.AuthNumbers = jseAuthCountGet();
        s_statisticData.AuthFailNumbers = jseAuthFailCountGet();
        s_statisticData.HTTPFailNumbers = BrowserStatisticHTTPFailNumbersGet();
        BrowserStatisticHTTPFailInfoGet(s_statisticData.HTTPFailInfo, STATISTIC_INFO_FAID_NUM * STATISTIC_INFO_FAID_SIZE);
        jseAuthFailInfoGet(s_statisticData.AuthFailInfo, STATISTIC_INFO_FAID_NUM * STATISTIC_INFO_FAID_SIZE);
        if (s_statisticData.RebootFlag == 0){
            s_statisticData.Endpoint = statisticLocakTime( );
            s_statisticData.Startpoint = s_statisticData.Endpoint - tr069StatisticGetLogRecordInterval();
        }

        memset(s_filebuf, 0, LOG_SERVER_URL_SIZE);
        mid_sys_serial(s_filebuf);
        len = strlen(s_filebuf);
        s_filebuf[len] = '_';
        len += 1;

        addr = inet_addr(network_address_get(ifname, ifaddr, URL_LEN));
        len += snprintf(s_filebuf + len, LOG_SERVER_URL_SIZE - len, "%03d-%03d-%03d-%03d_",
            addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff, (addr >> 24) & 0xff);

        len += snprintf(s_filebuf + len, LOG_SERVER_URL_SIZE - len, "%s_", "1-0");
        mid_tool_time2string((int)s_statisticData.Startpoint, buf, 0);
        len += snprintf(s_filebuf + len, LOG_SERVER_URL_SIZE - len, "%s_", buf);
        memset(buf,0,32);
        mid_tool_time2string((int)s_statisticData.Endpoint, buf, 0);
        len += snprintf(s_filebuf + len, LOG_SERVER_URL_SIZE - len, "%s.csv", buf);

        len = strlen(s_filebuf);
        s_filebuf[len] = '\0';
        snprintf(url + strlen(url), LOG_SERVER_URL_SIZE, "%s", s_filebuf);
        len = 0;
        memset(buf, 0, 32);

        mid_tool_time2string((int)s_statisticData.Startpoint, buf, 0);
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "Startpoint\t%s\n", buf);
        mid_tool_time2string((int)s_statisticData.Endpoint, buf, 0);
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "Endpoint\t%s\n", buf);
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "ProductClass\t%s\n", StbInfo::STB::Model());
        memset(buf, 0, 32);
#if defined(_NEW_NETWORK)
        const char* pmac = network_ifacemac_get(ifname, buf, 32);
        if (pmac)
            sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
#else
    //mid_net_mac_addr(buf, ':');
#endif
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "SerialNumber\t%s\n", buf);

        char stbid[40];
        memset(stbid, 0, 40);
        mid_sys_serial(stbid);
        len += snprintf( s_packBuf+len, STATISTIC_FILE_SIZE-len, "STBID\t%s\n", stbid);
#ifdef NEIMENGGU_HD
       len += snprintf( s_packBuf+len, STATISTIC_FILE_SIZE-len, "ManufacturerOUI\t%s\n", "000008");
#else
        len += snprintf( s_packBuf+len, STATISTIC_FILE_SIZE-len, "ManufacturerOUI\t%s\n", "00E0FC");
#endif
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodReqNumbers\t%d\n", Hippo::UltraPlayerVod::s_statistic.getRequestNum());

        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "AuthNumbers\t%d\n", s_statisticData.AuthNumbers);
        len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "AuthFailNumbers\t%d\n", s_statisticData.AuthFailNumbers);
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
        for(i = idx; i < num; i++ )
            len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "AuthFailInfo\t%s\n", s_statisticData.AuthFailInfo[i%STATISTIC_INFO_FAID_NUM]);
    }

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiReqNumbers\t%d\n", Hippo::UltraPlayerMultiple::s_statistic.getRequestNum());
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiFailNumbers\t%d\n", Hippo::UltraPlayerMultiple::s_statistic.getFailedNum());
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
        for(i = idx; i < num; i++ )
            len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiFailInfo\t%s\n", Hippo::UltraPlayerMultiple::s_statistic.getFailedInfo(i%STATISTIC_INFO_FAID_NUM));
    }
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodFailNumbers\t%d\n", Hippo::UltraPlayerVod::s_statistic.getFailedNum());
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
        for(i = idx; i < num; i++ )
            len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VodFailInfo\t%s\n", Hippo::UltraPlayerVod::s_statistic.getFailedInfo(i%STATISTIC_INFO_FAID_NUM));
    }


    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HTTPReqNumbers\t%d\n", s_statisticData.HTTPReqNumbers);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HTTPFailNumbers\t%d\n", s_statisticData.HTTPFailNumbers);
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
        for(i = idx; i < num; i++ )
            len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "HTTPFailInfo\t%s\n", s_statisticData.HTTPFailInfo[i%STATISTIC_INFO_FAID_NUM]);
    }

    len += snprintf( s_packBuf+len, STATISTIC_FILE_SIZE-len, "MutiAbendNumbers\t%d\n", Hippo::UltraPlayerMultiple::s_statistic.getUnderflowNum() );
    len += snprintf( s_packBuf+len, STATISTIC_FILE_SIZE-len, "VODAbendNumbers\t%d\n", Hippo::UltraPlayerVod::s_statistic.getUnderflowNum() );
    len += snprintf( s_packBuf+len, STATISTIC_FILE_SIZE-len, "MultiAbendUPNumbers\t%d\n", Hippo::UltraPlayerMultiple::s_statistic.getOverflowNum() );
    len += snprintf( s_packBuf+len, STATISTIC_FILE_SIZE-len, "VODUPAbendNumbers\t%d\n", Hippo::UltraPlayerVod::s_statistic.getOverflowNum() );
    len += snprintf( s_packBuf+len, STATISTIC_FILE_SIZE-len, "PlayErrorNumbers\t%d\n", Hippo::UltraPlayer::s_statistic.getplayErrorNumbers() );

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
        for(i = idx; i < num; i++ )
            len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "PlayErrorInfo\t%s\n", Hippo::UltraPlayer::s_statistic.getPlayErrorInfo(i%STATISTIC_INFO_FAID_NUM));
    }

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR1Nmb\t%d\n", s_statisticData.MultiPacketsLostR1);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR2Nmb\t%d\n", s_statisticData.MultiPacketsLostR2);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR3Nmb\t%d\n", s_statisticData.MultiPacketsLostR3);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR4Nmb\t%d\n", s_statisticData.MultiPacketsLostR4);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiPacketsLostR5Nmb\t%d\n", s_statisticData.MultiPacketsLostR5);


    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR1Nmb\t%d\n", s_statisticData.VODPacketsLostR1);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR2Nmb\t%d\n", s_statisticData.VODPacketsLostR2);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR3Nmb\t%d\n", s_statisticData.VODPacketsLostR3);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR4Nmb\t%d\n", s_statisticData.VODPacketsLostR4);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODPacketsLostR5Nmb\t%d\n", s_statisticData.VODPacketsLostR5);


    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR1Nmb\t%d\n", s_statisticData.MultiBitRateR1Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR2Nmb\t%d\n", s_statisticData.MultiBitRateR2Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR3Nmb\t%d\n", s_statisticData.MultiBitRateR3Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR4Nmb\t%d\n", s_statisticData.MultiBitRateR4Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "MultiBitRateR5Nmb\t%d\n", s_statisticData.MultiBitRateR5Nmb);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR1Nmb\t%d\n", s_statisticData.VODBitRateR1Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR2Nmb\t%d\n", s_statisticData.VODBitRateR2Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR3Nmb\t%d\n", s_statisticData.VODBitRateR3Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR4Nmb\t%d\n", s_statisticData.VODBitRateR4Nmb);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "VODBitRateR5Nmb\t%d\n", s_statisticData.VODBitRateR5Nmb);

    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR1Nmb\t%d\n", s_statisticData.FrameLostR1);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR2Nmb\t%d\n", s_statisticData.FrameLostR2);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR3Nmb\t%d\n", s_statisticData.FrameLostR3);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR4Nmb\t%d\n", s_statisticData.FrameLostR4);
    len += snprintf(s_packBuf + len, STATISTIC_FILE_SIZE - len, "FramesLostR5Nmb\t%d\n", s_statisticData.FrameLostR5);


    s_packBuf[STATISTIC_FILE_SIZE] = 0;
    LogUserOperDebug("\n**************CU qos file***********\n%s\n",s_packBuf);
    pthread_mutex_unlock(Hippo::UltraPlayer::s_statistic.getMutex());
    pthread_mutex_unlock(s_mutex);
}

