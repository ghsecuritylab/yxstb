#include "stdio.h"
#include "sys/types.h"
#include "sys/stat.h"

#include "MessageTypes.h"
#include "NativeHandler.h"
#include "config.h"
#include "MonitorUpgrade.h"
#include "NativeHandler.h"
#include "MessageValueSystem.h"
#include "TR069Assertions.h"
#include "MonitorAssertions.h"
#include "mgmtTr069ConfigLoad.h"
#include "app_epg_para.h"
#include "BootImagesShow.h"
#include "KeyDispatcher.h"
#include "mgmtTr069ConfigLoad.h"
#include "SystemManager.h"
#include "mgmtMonitorChannel.h"
#include "BrowserAgent.h"
#include "mgmtModuleParam.h"
#include "mgmtMsgDealFunc.h"
#include "mid_fpanel.h"
#include "mid/mid_timer.h"
#include "Tr069.h"
#include "config/pathConfig.h"
#include "Tr069PlayInfo.h"

#include <unistd.h>

#define CHN_LIST_FILE DEFAULT_TEMP_DATAPATH"/yx_GetChannelList"

namespace Hippo {

extern "C" {
void
mgmtDealCpeAcsReqReboot()
{
    sendMessageToNativeHandler(MessageType_Tr069, TR069_REQUEST_REBOOT, 0, 0);
    return ;
}

void
mgmtDealCpeMessageNone()
{
    LogTr069Debug("this message is none, just like in tr069, recv a NULL\n");
}

void
mgmtDealCpeRegisterAcsOk()
{
    tr069SettingSetInt("Global.Bootstrap", 1);
	LogTr069Debug("mgmt register acs ok\n");
}

void
mgmtDealCpeConfigDownload(Mgmt_DownloadInfo *info)
{
    if (info)
        mgmtCpeConfigDownload(info);
}

void
mgmtDealCpeConfigFactoryReset()
{
    tr069_port_reset();//reset fun!!!!!!!
    sendMessageToNativeHandler(MessageType_Tr069, TR069_REQUEST_REBOOT, 0, 0);
    return ;
}

void
mgmtDealCpeConfigUpload(Mgmt_UpLoadInfo *info)
{
    mgmtCpeConfigUpload(info);
}

void
mgmtDealCpePlayDiagnostics(char* state, char* url)
{
    tr069_diagnostics_set_state(state);
    tr069_set_PlayURL(url);
    tr069_play_start();
//	tr069_diagnostics_get_state((char *)argv[2], 256);
//	tr069_set_PlayState()
}

void
mgmtDealCpeCallValueChang()
{
    LogTr069Debug("this msg is obligate for now\n");
}

void
mgmtDealCpeLogOutputChannelSet()
{

}

void
mgmtDealCpeGetPlayState(char* url, char* state)
{
   printf("argv[0] = %s, argv[1] = %s\n",(char *)url,(char *)state);
   if(strcmp((char *)url,"Device.X_00E0FC.PlayDiagnostics.PlayURL") == 0)
        tr069_get_PlayURL((char *)url,256);
   else
        sprintf( (char *)state,"%d",Tr069GetCurrentPlayState());
    printf("get play state = %s\n",(char *)state);
}

void
mgmtDealMtPlayerbyChanno(int* channelno, int* flag)
{
    char tmpChannum[10] = {0};

    sprintf(tmpChannum, "%d", *channelno);
    app_stbmonitor_tms_url_set(tmpChannum);
    BootImagesShowBootLogo(0);
    sendMessageToNativeHandler(MessageType_System, MV_System_OpenTransparentPage, 0, 0);
    (*flag) = 1;//temp dealing
}

void
mgmtDealMtPlayerbyUrl(char* url, int* flag)
{
    MONITOR_LOG("playURL = %s\n",url);
    app_stbmonitor_tms_url_set(url);
    BootImagesShowBootLogo(0);
    sendMessageToNativeHandler(MessageType_System, MV_System_OpenTransparentPage, 0, 0);
    *flag = 1;//temp dealing
}

void
mgmtDealMtPlayerStop()
{
    sendMessageToKeyDispatcher(MessageType_KeyDown, EIS_IRKEY_BACK, 0, 0);
    usleep(100000);
    sendMessageToKeyDispatcher(MessageType_KeyDown, EIS_IRKEY_MENU, 0, 0);
}

void
mgmtDealMtPlayerMpctrl(const char* action)
{
    if (!action)
        return;
    MONITOR_LOG("action = %s\n", action);
    if (!strcmp(action, "KEY_PAUSE_PLAY"))
        sendMessageToEPGBrowser(MessageType_KeyDown, EIS_IRKEY_PLAY, 0, 0);
    else if (!strcmp(action, "KEY_FAST_FORWARD"))
        sendMessageToEPGBrowser(MessageType_KeyDown, EIS_IRKEY_FASTFORWARD, 0, 0);
    else if (!strcmp(action, "KEY_FAST_BACK"))
        sendMessageToEPGBrowser(MessageType_KeyDown, EIS_IRKEY_REWIND, 0, 0);
    else
        MONITOR_LOG("no action marched\n");
}

void
mgmtDealMtToolReboot()
{
    mid_timer_create(2, 1, (mid_timer_f)mid_fpanel_reboot, 0);
}

void
mgmtDealMtEnterDebug()
{

}

void
mgmtDealMtExitDebug()
{

}

void
mgmtDealMtGetChannelNumToTal(int* count)
{
    SystemManager &sysManager = systemManager();
    *count = systemManager().channelList().getProgramCount();
}

void
mgmtDealMtGetChannelInfo(int* channelno, char* info)
{
  //  Hippo::mgmtMonitorChannellistToRootFile();
    //mgmtGetChannellist(buf,1024);  //for test!!!!!
  //   mgmtGetChannelInfo(buf,1024);  //for offical!!!!
    MgmtChannelListRead(info);
}

void
mgmtDealMtGetCollectFilePath()
{

}
#if 0
struct moni_buf {
    int len;
    char buf[MONITOR_MSGLEN + 1];
    char *extend;
    char remoteip[20];
};

static int MonitorUpgradeLinux(int pSocketPort)
{
    int servernew_fd = -1;
    int monitornew_port = 0;
    int recv_len = 0;
    int temp_len = 0;
    int error = 0;
    int file_length = 0;
    socklen_t len = 0;
    int clientnew_fd = -1;
    unsigned int write_len = 0;
    fd_set readnew_fd, errornew_fd;
    struct sockaddr_in tcp_addr;
    unsigned char* bufferHead = 0;
    struct moni_buf tMonitorMsg = {0};

    monitornew_port = pSocketPort;
    servernew_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(servernew_fd < 0) {
        //LOG_ERROR("server sockect creat error!\n");
        sprintf(tMonitorMsg.buf, "303ioctl^upgrate");
        tMonitorMsg.len = strlen(tMonitorMsg.buf);
        goto ERR;
    }

    memset(&tcp_addr, 0, sizeof(tcp_addr));
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_port = htons(monitornew_port);
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(servernew_fd, (struct sockaddr*)&tcp_addr, sizeof(tcp_addr)) < 0) {
        //LOG_ERROR("server socket bind");
        sprintf(tMonitorMsg.buf, "303ioctl^upgrate");
        tMonitorMsg.len = strlen(tMonitorMsg.buf);
        goto ERR;
    }
    if(listen(servernew_fd, 1) < 0) {
        //LOG_ERROR("server socket listen");
        sprintf(tMonitorMsg.buf, "303ioctl^upgrate");
        tMonitorMsg.len = strlen(tMonitorMsg.buf);
        goto ERR;
    }
    monitorUpgradeDownloadStart();
    while(1) {
        len = sizeof(tcp_addr);
        clientnew_fd = accept(servernew_fd, (struct sockaddr*)&tcp_addr, &len);
        if(clientnew_fd < 0) {
            MONITOR_LOG("server accept");
            continue;
        }
        MONITOR_LOG("server accept success\n");

        len = 1;
        if(setsockopt(clientnew_fd, SOL_SOCKET, SO_KEEPALIVE, &len, sizeof(len)) < 0) {
            //LOG_ERROR("setsockopt SO_KEEPALIVE error!!!\n");
            goto ERR;
        }
        file_length = monitorUpgradeGetLength();
        while(1)  {
            struct timeval tm = {30, 0};
            FD_ZERO(&readnew_fd);
            FD_ZERO(&errornew_fd);
            FD_SET(clientnew_fd, &readnew_fd);
            FD_SET(clientnew_fd, &errornew_fd);
            if(select(clientnew_fd + 1, &readnew_fd, (fd_set *) NULL, &errornew_fd, &tm) <= 0) {
                //LOG_ERROR("Error on tcp select request: %s", strerror(errno));
                goto END;
            }
            if(FD_ISSET(clientnew_fd, &readnew_fd) || FD_ISSET(clientnew_fd, &errornew_fd)) {
                error = -1;
                len = sizeof(error);
                getsockopt(clientnew_fd, SOL_SOCKET, SO_ERROR, (void*)&error, &len);
                if(error != 0) {
                    //LOG_ERROR("%s,client's socket timeout!!!\n", strerror(errno));
                    close(clientnew_fd);
                    clientnew_fd = -1;
                    monitorUpgradeReceiveError();
                    break;
                }
                if (monitorUpgradeGetBuffer(&bufferHead, &recv_len) < 0)
                    break;
                temp_len = recv(clientnew_fd, bufferHead, recv_len, MSG_NOSIGNAL);
                if (temp_len < 0)
                	break;
                write_len += temp_len;
                monitorUpgradeSubmitBuffer(bufferHead, temp_len);
                if(write_len >= file_length)
                    goto END;
            }
        }
    }
END:
    MONITOR_LOG("Upgrade file lenght(%d) download lenght(%d)\n", file_length, write_len);
    if (file_length != write_len) {
        sprintf(tMonitorMsg.buf, "303ioctl^upgrate");
        tMonitorMsg.len = strlen(tMonitorMsg.buf);
        monitorUpgradeReceiveError();
    } else {
        monitorUpgradeBurnStart();
    	while(monitorUpgradeGetProgress() <= 95 && monitorUpgradeGetProgress() >= 0) {
        	sprintf(tMonitorMsg.buf, "301upgrate^%d", monitorUpgradeGetProgress());
        	tMonitorMsg.len = strlen(tMonitorMsg.buf);
        	//monitor_cmd_response(&tMonitorMsg);
        	sleep(2);
    	}

        if(monitorUpgradeGetProgress() < 0){
            sprintf(tMonitorMsg.buf, "303ioctl^upgrate");
            tMonitorMsg.len = strlen(tMonitorMsg.buf);
        }else{
    	    sprintf(tMonitorMsg.buf, "302upgrate", monitorUpgradeGetProgress());
    	    tMonitorMsg.len = strlen(tMonitorMsg.buf);
            }
	}
    //monitor_cmd_response(&tMonitorMsg);
    close(clientnew_fd);
    clientnew_fd = -1;
    close(servernew_fd);
    servernew_fd = -1;
    pthread_exit((void *)(write_len));
ERR:
    //monitor_cmd_response(&tMonitorMsg);
    //upgrade_flag = 0;
    close(servernew_fd);
    servernew_fd = -1;
    pthread_exit((void *)(-1));
    return 0;
}


int
mgmtDealMtUpgradeLinux(int port, long long fileLen)
{
    int ret = 0;
    pthread_t pupgrate;

#ifdef INCLUDE_LITTLESYSTEM
    if (monitorSystemTypeGet() == 1) {
        monitorUpgradeModeSet();
        exit(0);
    }
#endif

    monitorUpgradeSetLength(fileLen);
    ret = pthread_create(&pupgrate, NULL, (void *)MonitorUpgradeLinux, (void *)port);
    if(ret != 0) {
        printf("Create upgrade thread error !\n");
        return -2;
    }
    #if 1
    //if(strstr(buf->buf, "^/f") != NULL) //判断是否为强制升级
        monitorUpgradeSetFlag(1);
    //else
    //    monitorUpgradeSetFlag(0);
    #endif

    return 0;
}
#endif

void
mgmtDealMtUpgradeGetWorkStat()
{

}

void
mgmtDealMtUpgradeSetLength(int len)
{
    MONITOR_LOG("the upgrade bin len = %d\n", len);
}

void
mgmtDealMtUpgradeSetForce(int flag)
{
    monitorUpgradeSetFlag(flag);
    monitorUpgradeDownloadStart();

}

void
mgmtDealMtUpgradeSetUpgrader(char* info, int* flag)
{
    MONITOR_LOG("upgrade header info = %s\n", info);
}

void
mgmtDealMtUpgradeGetDownHandle(FILE* downloadHandle)
{
    FILE *fp = NULL;

    fp = fopen(DEFAULT_RAM_DATAPATH"/upgrade_temp.bin", "w");
    memcpy(downloadHandle, &fp, sizeof(FILE *));
}

void
mgmtDealMtUpgradeSetCloseWork()
{

}

void
mgmtDealMtUpgradeSetDownloadPer(int percent)
{
    monitorUpgradeSetProgress(percent);
}

void
mgmtDealMtUpgradeGetBurnProcess(int* percent)
{
    if (monitorUpgradeGetProgress() == 0)
        *percent = 1;
    else
        *percent = monitorUpgradeGetProgress();
}

void
mgmtDealMtUpgradeSetBurnStart()
{
    monitorUpgradeBurnStart();
}

void
mgmtDealNtUpgradeNetworkDisconnect()
{
}

int MgmtChannelListRead(char * buf)
{
    FILE *fp = NULL;
    struct stat file_stat;

    if(mgmtGetChannelList_stor() < 0)
        return  -1;

#ifdef ANDROID
    fp = fopen("/data/yx_GetChannelList", "r");
#else
    fp = fopen(CHN_LIST_FILE, "r");
#endif
    if(NULL == fp) {
        printf("CHN_LIST_FILE open fail");
        perror("CHN_LIST_FILE open fail");
        unlink(CHN_LIST_FILE);
        return -1;
    }
    if(fstat(fileno(fp), &file_stat) < 0) {
        perror("CHN_LIST_FILE stat fail");
        fclose(fp);
        unlink(CHN_LIST_FILE);
        return -1;
    }
    int readLen = fread(buf, 1, file_stat.st_size, fp);
    if((readLen < file_stat.st_size) && (!feof(fp))) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    unlink(CHN_LIST_FILE);
    return 0;
}

}

}
