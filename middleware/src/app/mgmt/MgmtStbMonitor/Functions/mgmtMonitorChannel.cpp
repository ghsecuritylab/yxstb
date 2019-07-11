
#include <iostream>
#include <map>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mgmtMonitorChannel.h"
#include "MonitorAssertions.h"
#include "config/pathConfig.h"

#include "SystemManager.h"
#if defined( HUAWEI_C10 )
#include "C10/ProgramChannelC10.h"
#elif defined( HUAWEI_C20 )
#include "C20/ProgramChannelC20.h"
#else
#program error.
#endif


namespace Hippo
{

static int mgmtmonitorChannellistToFile(FILE *fp)
{
    char buf[2048] = {0};
    int len = 0, channel_count = 0, i = 0;
    SystemManager &sysManager = systemManager();
#if defined( HUAWEI_C10 )
    ProgramChannelC10 *pChannel = NULL;
#elif defined( HUAWEI_C20 )
    ProgramChannelC20 *pChannel = NULL;
#endif

    channel_count = sysManager.channelList().getProgramCount();

    for(i = 0; i < channel_count; i++) {
#if defined( HUAWEI_C10 )
        pChannel = (ProgramChannelC10 *)sysManager.channelList().getProgramByIndex(i);
#elif defined( HUAWEI_C20 )
        pChannel = (ProgramChannelC20 *)sysManager.channelList().getProgramByIndex(i);
#endif
        if(NULL == pChannel)
            continue;

        len = 0;
        len += sprintf(buf + len, "[ChannelID=\"%s\",", pChannel->GetChanID().c_str());
        len += sprintf(buf + len, "ChannelName=\"%s\",", pChannel->GetChanName().c_str());
        len += sprintf(buf + len, "UserChannelID=\"%d\",", pChannel->GetUserChanID());

        char *tChannelUrl = NULL;
        int tChannelUrlLen = 0;
        char *tPoint = NULL;

        tChannelUrlLen = strlen(pChannel->GetChanURL().c_str());
        tChannelUrl = (char *)malloc(tChannelUrlLen + 1);
        memset(tChannelUrl, 0, tChannelUrlLen + 1);
        strcpy(tChannelUrl, pChannel->GetChanURL().c_str());
#if defined(HUAWEI_C10)
        if(pChannel->GetCanUnicast() == 1) {
            tPoint = strchr(tChannelUrl, '|');
            if(tPoint)
                *tPoint = '\0';
        }
#endif
        len += sprintf(buf + len, "ChannelURL=\"%s\",", tChannelUrl);
        free(tChannelUrl);

        len += sprintf(buf + len, "TimeShift=\"%d\",", pChannel->GetTimeShift());
        len += sprintf(buf + len, "TimeShiftLength=\"%d\",", pChannel->GetTimeShiftLength());
#if defined(HUAWEI_C10)
        len += sprintf(buf + len, "ChannelSDP=\"%s\",", pChannel->GetChanSDP().c_str());
#endif
        len += sprintf(buf + len, "TimeShiftURL=\"%s\",", pChannel->GetTimeShiftURL().c_str());
        len += sprintf(buf + len, "ChannelType=\"%d\",", pChannel->GetChanType());
        len += sprintf(buf + len, "IsHDChannel=\"%d\",", pChannel->GetIsHD_Chan());
        len += sprintf(buf + len, "PreviewEnable=\"%d\",", pChannel->GetPrevEnable());
        len += sprintf(buf + len, "Previewnum=\"%d\",", pChannel->GetPrevNum());
        len += sprintf(buf + len, "ChanKey=\"%d\",", pChannel->GetUserChanID());
        len += sprintf(buf + len, "PLTVEnable=\"%d\",", pChannel->GetTimeShift());
#if defined(HUAWEI_C10)
        len += sprintf(buf + len, "ChannelPurchased=\"%d\",", pChannel->GetChanPurchased());
        len += sprintf(buf + len, "ChannelLocked=\"%d\",", pChannel->GetChanLocked());
#endif
        len += sprintf(buf + len, "ChannelLogURL=\"%s\",", pChannel->GetLogoURL().c_str());
        len += sprintf(buf + len, "PositionX=\"%d\",", pChannel->GetLogoXPos());
        len += sprintf(buf + len, "PositionY=\"%d\",", pChannel->GetLogoYPos());
#if defined(HUAWEI_C10)
        len += sprintf(buf + len, "BeginTime=\"%d\",", pChannel->GetBeginTime());
        len += sprintf(buf + len, "Interval=\"%d\",", pChannel->GetInterval());
        len += sprintf(buf + len, "Lasting=\"%d\",", pChannel->GetLasting());
        len += sprintf(buf + len, "ActionType=\"%d\",", pChannel->GetActionType());
        len += sprintf(buf + len, "ChannelFECPort=\"%ld\",", pChannel->GetChannelFECPort());
        len += sprintf(buf + len, "CanUnicast=\"%ld\",", pChannel->GetCanUnicast());
#elif defined( HUAWEI_C20 )
        len += sprintf(buf + len, "ChanStatus=\"%d\",", pChannel->GetChanStatus());
        len += sprintf(buf + len, "hasPip=\"%d\",", pChannel->GetHasPIP());
        len += sprintf(buf + len, "pipMulticastIP=\"%s\",", pChannel->GetPIPMulticastIP().c_str());
        len += sprintf(buf + len, "pipMulticastPort=\"%s\",", pChannel->GetMulticastPort().c_str());
        len += sprintf(buf + len, "ChanBandwith=\"%d\",", pChannel->GetChanBandwith());
        len += sprintf(buf + len, "LogoDisplay=\"%d;%d\",", pChannel->GetLogoDisplay(), pChannel->GetLogoHide());
        len += sprintf(buf + len, "PreviewLength=\"%d\",", pChannel->GetPrevLength());
        len += sprintf(buf + len, "PreviewCount=\"%d\",", pChannel->GetPrevCount());
        len += sprintf(buf + len, "CGMSAEnable=\"%d\",", pChannel->GetCGMSAEnable());
        len += sprintf(buf + len, "HDCPEnable=\"%d\",", pChannel->GetHDCP_Enable());
        len += sprintf(buf + len, "MacrovisionEnable=\"%d\",", pChannel->GetMacrovEnable());
#endif
        len += sprintf(buf + len, "FCCEnable=\"%ld\",", pChannel->GetFCC_Enable());
        len += sprintf(buf + len, "FECEnable=\"%ld\"]\r\n", pChannel->GetFEC_Enable());

        fwrite(buf, len, 1, fp);
    }
    return 0;
}


static const char* mgmtmonitorChannelURLbyId(int id)
{
    SystemManager &sysManager = systemManager();

#if defined( HUAWEI_C10 )
    ProgramChannelC10 *pChannel = NULL;
    pChannel = (ProgramChannelC10 *)sysManager.channelList().getProgramByNumberID(id);

    if(pChannel)
        return pChannel->GetChanURL().c_str();
    else
        return NULL;
#elif defined( HUAWEI_C20 )
    ProgramChannelC20 *pChannel = NULL;
    char channID[16] = {0};
    snprintf(channID, 16, "MULTIPLE%d" , id);
    pChannel = (ProgramChannelC20 *)sysManager.channelList().getProgramByStringID(channID);

    if(pChannel)
        return pChannel->GetChanURL().c_str();
    else
        return NULL;
#else
    return NULL;
#endif

}

int mgmtGetChannelList_stor()
{
    FILE *fp;
#ifdef ANDROID
    fp = fopen("/data/yx_GetChannelList", "w");
#else
    fp = fopen(DEFAULT_TEMP_DATAPATH"/yx_GetChannelList", "w");
#endif
    if(NULL == fp) {
        perror("DEFAULT_TEMP_DATAPATH/yx_GetChannelList open fail");
        return -1;
    }

    Hippo::mgmtmonitorChannellistToFile(fp);
    fflush(fp);
    fclose(fp);

    return 0;
}

//extern "C"
const char* mgmtGetChannelURL_byId(int id)
{
    return Hippo::mgmtmonitorChannelURLbyId(id);
}

} // namespace Hippo

