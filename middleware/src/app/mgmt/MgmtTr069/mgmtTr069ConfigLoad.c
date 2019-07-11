#include <string.h>
#include "mgmtTr069ConfigLoad.h"
#include "hmw_mgmtlib.h"
#include "mid/mid_timer.h"
#include "MessageTypes.h"
#include "app_epg_para.h"
#include "mgmtTr069ConfigLoad.h"
#include "NativeHandler.h"
#include "Assertions.h"
#include "config.h"
#include "MessageValueSystem.h"
#include "mid_fpanel.h"
#include <sys/types.h>
#include <sys/stat.h>

#define MGMT_MAX_DELAY_SECONDS		(15 * 24 * 3600)
enum{
	FILETYPE_FIREWARE = 0,
	FILETYPE_CONFIG,
	FILETYPE_MAX
};

enum{
    MGMT_TYPE_DOWNLOAD = 0,
    MGMT_TYPE_UPLOAD = 1
};

#if 0

int mgmtMonitorChannellistToRootFile()
{
    FILE *fp;
    char buf[2048] = {0};
    int len = 0, channel_count = 0, i = 0;
    SystemManager &sysManager = systemManager();
    ProgramChannelC20 *pChannel = NULL;

    fp = fopen("/root/yx_GetChannelList", "w");
    if (NULL == fp){
        printf("fopen /root/yx_GetChannelList error.\n");
        return -1;
    }
    channel_count = sysManager.channelList().getProgramCount();

    for(i = 0; i < channel_count; i++) {
        pChannel = (ProgramChannelC20 *)sysManager.channelList().getProgramByIndex(i);
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
        len += sprintf(buf + len, "ChannelURL=\"%s\",", tChannelUrl);
        free(tChannelUrl);

        len += sprintf(buf + len, "TimeShift=\"%d\",", pChannel->GetTimeShift());
        len += sprintf(buf + len, "TimeShiftLength=\"%d\",", pChannel->GetTimeShiftLength());
        len += sprintf(buf + len, "TimeShiftURL=\"%s\",", pChannel->GetTimeShiftURL().c_str());
        len += sprintf(buf + len, "ChannelType=\"%d\",", pChannel->GetChanType());
        len += sprintf(buf + len, "IsHDChannel=\"%d\",", pChannel->GetIsHD_Chan());
        len += sprintf(buf + len, "PreviewEnable=\"%d\",", pChannel->GetPrevEnable());
        len += sprintf(buf + len, "Previewnum=\"%d\",", pChannel->GetPrevNum());
        len += sprintf(buf + len, "ChanKey=\"%d\",", pChannel->GetUserChanID());
        len += sprintf(buf + len, "PLTVEnable=\"%d\",", pChannel->GetTimeShift());
        len += sprintf(buf + len, "ChannelLogURL=\"%s\",", pChannel->GetLogoURL().c_str());
        len += sprintf(buf + len, "PositionX=\"%d\",", pChannel->GetLogoXPos());
        len += sprintf(buf + len, "PositionY=\"%d\",", pChannel->GetLogoYPos());
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
        len += sprintf(buf + len, "FCCEnable=\"%ld\",", pChannel->GetFCC_Enable());
        len += sprintf(buf + len, "FECEnable=\"%ld\"]\r\n", pChannel->GetFEC_Enable());
        fwrite(buf, len, 1, fp);
    }
    return 0;
}

};

 struct channelInfo
{
    char szChanID[8];
    char szChanName[64];
    int  iChanKey;
    char szChanURL[256];
    int  bPLTVEnable;
    int  iPauseLength;
    char  szChanSDP[256];
    char  szPLTVURL[256];
    int  iChanType;
    int  bHDChannel;
    int  bPreviewEnable;
    int  iChanStatus;
    int  iChanStatus1;
    int  iChanStatus2;
    char szChanLogoURL[256];
    int  iLogoPosX;
    int  iLogoPosY;
    int  iLogoBeginTime;
    int  iLogoInterval;
    int  iLogoLasting;
    int  iActionType;
};

int InitChannelInfoForTest( struct channelInfo *pChanInfo)
{
   strcpy(pChanInfo->szChanID,"1");
   strcpy(pChanInfo->szChanName,"");
   pChanInfo->iChanKey = 680;
   strcpy(pChanInfo->szChanURL,"http://110.1.1.132:33200");
   pChanInfo->bPLTVEnable = 1;
   pChanInfo->iPauseLength = 32;
   strcpy(pChanInfo->szChanSDP,"sdp");
   strcpy(pChanInfo->szPLTVURL,"www.baudu.com");
   pChanInfo->iChanType = 2;
   pChanInfo->bHDChannel = 3;
   pChanInfo->bPreviewEnable= 1;
   strcpy(pChanInfo->szChanLogoURL,"http://110.1.1.183");
   pChanInfo->iLogoPosX = 60;
   pChanInfo->iLogoPosY = 80;
   pChanInfo->iLogoBeginTime = 3;
   pChanInfo->iLogoInterval = 12;
   pChanInfo->iLogoLasting = 8;
   pChanInfo->iActionType = 4;
   return 0;
}

int mgmtGetChannelInfo(char *buf,int len)
{
    FILE *fp = NULL;
    struct stat file_stat;
	int readLen = 0;

    if(GetChannelList_stor() < 0)
        return -1;

    fp = fopen("/root/yx_GetChannelList", "r");
    if(NULL == fp) {
        printf("/root/yx_GetChannelList open fail\n");
        unlink("/root/yx_GetChannelList");
        return -1;
    }

    if(fstat(fileno(fp), &file_stat) < 0) {
        printf("/root/yx_GetChannelList stat fail");
        fclose(fp);
        unlink("/root/yx_GetChannelList");
        return -1;
    }

    readLen = fread(buf, 1, file_stat.st_size, fp);
    if((readLen < file_stat.st_size) && (!feof(fp))) {
        fclose(fp);
        return -1;
    }
    fclose(fp);

    unlink("/root/yx_GetChannelList");
    return 0;
}

int mgmtGetChannellist(char *buf ,int len)
{
    struct channelInfo *pChanInfo = NULL;
    pChanInfo = (struct channelInfo *)malloc(sizeof(struct channelInfo ));
    InitChannelInfoForTest(pChanInfo);
    snprintf(buf, 1024 - 1,

                         "ChannelID=\"%s\",ChannelName=\"%s\","

                         "UserChannelID=\"%d\",ChannelURL=\"%s\","

                         "TimeShift=\"%d\",TimeShiftLength=\"%d\","

                         "ChannelSDP=\"%s\",TimeShiftURL=\"%s\","

                         "ChannelType=\"%d\",IsHDChannel=\"%d\","

                         "PreviewEnable=\"%d\",ChannelPurchased=\"%d\","

                         "ChannelLocked=\"%d\",ChannelLogURL=\"%s\","

                         "PositionX=\"%d\",PositionY=\"%d\","

                         "BeginTime=\"%d\",Interval=\"%d\","

                         "Lasting=\"%d\",ActionType=\"%d\"\n",

                         pChanInfo->szChanID,

                         pChanInfo->szChanName,

                         pChanInfo->iChanKey,

                         pChanInfo->szChanURL,

                         pChanInfo->bPLTVEnable,

                         pChanInfo->iPauseLength,

                         pChanInfo->szChanSDP,

                         pChanInfo->szPLTVURL,

                         pChanInfo->iChanType,

                         pChanInfo->bHDChannel,

                         pChanInfo->bPreviewEnable,

                         pChanInfo->iChanStatus1,

                         pChanInfo->iChanStatus2,

                         pChanInfo->szChanLogoURL,

                         pChanInfo->iLogoPosX,

                         pChanInfo->iLogoPosY,

                         pChanInfo->iLogoBeginTime,

                         pChanInfo->iLogoInterval,

                         pChanInfo->iLogoLasting,

                         pChanInfo->iActionType);

}

#endif //先留着，待开发完成逐步去掉

static const char FileTypeArray[FILETYPE_MAX][32] =
{
    "1 Firmware Upgrade Image",
    "3 Vendor Configuration File"
};

static Mgmt_DownloadInfo download;
static Mgmt_UpLoadInfo upload;

void mgmt_load_ontime(int arg)
{
    if (arg == MGMT_TYPE_DOWNLOAD) {
        if (strcmp(download.szFileType, FileTypeArray[0]) == 0) {
            tr069_set_UpgradeUrl(download.szURL);
            sendMessageToNativeHandler(MessageType_Tr069, TR069_UPGRADE_REQUEST, 0, 0);
        } else  if (strcmp(download.szFileType, FileTypeArray[1]) == 0) {

        } else {

        }
    } else if (arg == MGMT_TYPE_UPLOAD) {
        if (strcmp(upload.szFileType, FileTypeArray[0]) == 0) {

        } else  if (strcmp(upload.szFileType, FileTypeArray[1]) == 0) {

        } else {

        }
    }
}

static unsigned int mgmt_port_get_UpTime(void)
{
    long long clk = mid_clock( );
    return (unsigned int)(clk / 1000);
}

int  mgmtCpeConfigDownload(Mgmt_DownloadInfo *info)
{
    int i = 0;
    int filetypeflag = -1;

    memcpy(&download, info, sizeof(Mgmt_DownloadInfo));

    for (i = 0; i < FILETYPE_MAX; i ++) {
        if (strcmp(download.szFileType, FileTypeArray[i]) == 0) {
            filetypeflag = i;
            break;
	}
    }

    if (filetypeflag == -1 || download.uiDelaySeconds > MGMT_MAX_DELAY_SECONDS)
        ERR_OUT("filetype = %d, delaySeconds = %d\n", filetypeflag, download.uiDelaySeconds);

    if (download.uiDelaySeconds <= 0)
        download.uiDelaySeconds = 0;
    else
        download.uiDelaySeconds= (int)mgmt_port_get_UpTime( ) + download.uiDelaySeconds;

    mid_timer_create(download.uiDelaySeconds, 1, mgmt_load_ontime, MGMT_TYPE_DOWNLOAD);

    return 0;
Err:
    return -1;
}

int  mgmtCpeConfigUpload(Mgmt_UpLoadInfo *info)
{
    int i = 0;
    int filetypeflag = -1;

    memcpy(&upload, info, sizeof(Mgmt_UpLoadInfo));

    for (i = 0; i <= FILETYPE_MAX; i ++) {
        if (strcmp(upload.szFileType, FileTypeArray[i]) == 0) {
            filetypeflag = i;
            break;
	}
    }

    if (filetypeflag == -1 || upload.uiDelaySeconds > MGMT_MAX_DELAY_SECONDS)
        ERR_OUT("filetype = %d, delaySeconds = %d\n", filetypeflag, upload.uiDelaySeconds);

    if (upload.uiDelaySeconds <= 0)
        upload.uiDelaySeconds = 0;
    else
        upload.uiDelaySeconds = (int)mgmt_port_get_UpTime( ) + upload.uiDelaySeconds;

    mid_timer_create(upload.uiDelaySeconds, 1, mgmt_load_ontime, MGMT_TYPE_UPLOAD);

    return 0;
Err:
    return -1;
}


