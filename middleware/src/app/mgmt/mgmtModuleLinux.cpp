
#include "hmw_mgmtlib.h"
#include "mgmtModule.h"
#include "mgmtModuleTr069.h"
#include "mgmtModuleStbMonitor.h"
#include "Assertions.h"
#include "SystemManager.h"
#include "mgmtMsgDealFunc.h"
#include "mgmtModuleCmdLine.h"
#include "MonitorUpgrade.h"

#include "MessageValueMaintenancePage.h"
#include "NativeHandler.h"
#include "MessageTypes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>


extern "C" void setUsbNoInsertMessage(char *msg);
extern int stbMonitorInit(char *XMLFilePath);
extern int stbMonitorConnectFlag(int flag);
extern int stbMonitorShutDown();

static int monitorState = 0;


namespace Hippo {

extern "C" int mgmtReadCallBack(HMW_MgmtConfig_SRC eSrcType, const char * szParm, char * pBuf, int iLen)
{
    int iret = 0;

    switch (eSrcType) {
    case MGMT_CONFIG_TMS:
        iret = tmsReadConfig(szParm, pBuf, iLen);
        break;
    case MGMT_CONFIG_MT:
        iret = mgmtReadConfig(szParm, pBuf, iLen);
        break;
    case MGMT_CONFIG_OTHERS:
        break;
    default:
        break;
    }
    return iret;
}

extern "C" int mgmtWriteCallback(HMW_MgmtConfig_SRC eSrcType, const char * szParm, char * pBuf, int iLen)
{
    int iret = 0;
    switch (eSrcType) {
    case MGMT_CONFIG_TMS:
        #ifdef INCLUDE_HMWMGMT
        iret = tmsWriteConfig(szParm, pBuf, iLen);
		if(!iret){
           hmw_mgmtValueChange(szParm,pBuf,iLen);
		}
        #endif
        break;
    case MGMT_CONFIG_MT:
        iret = mgmtWriteConfig(szParm, pBuf, iLen);
        break;
    case MGMT_CONFIG_OTHERS:
        break;
    default:
        break;
    }
    return iret;
}

extern "C" int mgmtNotifyCallback(HMW_MgmtMsgType eMsgType, unsigned int argc, void *argv[])
{
    int g_upgrade = -1;
    int count = 0;
    LogUserOperDebug("Mgmt send message to stb: emsgtype=0x%x, argc=%u\n", eMsgType, argc);
    switch (eMsgType) {
    case MGMT_MESSAGE_NONE:
        mgmtDealCpeMessageNone();
        break;
    case MGMT_CPE_REGISTER_ACS_OK:
        mgmtDealCpeRegisterAcsOk();
        break;
    case MGMT_CPE_ACS_REQ_REBOOT:
        mgmtDealCpeAcsReqReboot();
        break;
    case MGMT_CPE_CONFIG_DOWNLOAD:
        mgmtDealCpeConfigDownload((Mgmt_DownloadInfo *)argv[0]);
        break;
    case MGMT_CPE_CONFIG_FACTORYRESET:
        mgmtDealCpeConfigFactoryReset();
        break;
    case MGMT_CPE_CONFIG_UPLOAD:
        mgmtDealCpeConfigUpload((Mgmt_UpLoadInfo *)argv[0]);
        break;
    case MGMT_CPE_PLAY_DIAGNOSTICS:
        mgmtDealCpePlayDiagnostics((char*)argv[0], (char*)argv[1]);
        break;
    case MGMT_CPE_CALL_VALUECHANG:
        mgmtDealCpeCallValueChang();
        break;
    case MGMT_LOG_OUTPUT_CHANNEL_SET:
        mgmtDealCpeLogOutputChannelSet();
        break;
    case MGMT_CPE_GET_PLAYSTATE:
        mgmtDealCpeGetPlayState((char*)argv[0], (char*)argv[1]);
        break;
    case MGMT_MT_PLAYER_BY_CHANNO:
        mgmtDealMtPlayerbyChanno((int*)argv[0], (int*)argv[1]);
        break;
    case MGMT_MT_PLAYER_BY_URL:
        mgmtDealMtPlayerbyUrl((char*)argv[0], (int*)argv[1]);
        break;
    case MGMT_MT_PLAYER_STOP:
        mgmtDealMtPlayerStop();
        *((int*)argv[0]) = 1;
        break;
    case MGMT_MT_PLAYER_MPCTRL:
        mgmtDealMtPlayerMpctrl((char*)argv[0]);
        break;
    case MGMT_MT_TOOL_REBOOT:
        mgmtDealMtToolReboot();
        break;
    case MGMT_MT_ENTER_DEBUG:
        mgmtDealMtEnterDebug();
        break;
    case MGMT_MT_EXIT_DEBUG:
        mgmtDealMtExitDebug();
        break;
    case MGMT_MT_GET_CHANNELNUM_TOTAL:
        mgmtDealMtGetChannelNumToTal((int*)argv[0]);
        break;
    case MGMT_MT_GET_CHANNELINFO_I:
        mgmtDealMtGetChannelInfo((int*)argv[0], (char*)argv[1]);
        break;
    case MGMT_MT_GET_COLLECT_FILEPATH:
        mgmtDealMtGetCollectFilePath();
        break;
    case MGMT_MT_UPGRADE_GET_WORKSTAT:
        mgmtDealMtUpgradeGetWorkStat();
        *(int *)argv[0] = 1;
        break;
    case MGMT_MT_UPGRADE_SET_LENGTH:
        mgmtDealMtUpgradeSetLength((int)argv[0]);
        break;
    case MGMT_MT_UPGRADE_SET_FORCE:
        mgmtDealMtUpgradeSetForce((int)argv[0]);
        break;
    case MGMT_MT_UPGRADE_SET_UPHEADER:
        mgmtDealMtUpgradeSetUpgrader((char*)argv[0], (int*)argv[1]);
        *(int *)argv[1] = 1;
        break;
    case MGMT_MT_UPGRADE_GET_DOWNHANDLE:
        mgmtDealMtUpgradeGetDownHandle((FILE*)argv[0]);
        break;
    case MGMT_MT_UPGRADE_SET_CLOSEWORK:
        mgmtDealMtUpgradeSetCloseWork();
        break;
    case MGMT_MT_UPGRADE_SET_DOWNLOAD_PER:
        // Linux upgrade start
        // argv[0]:m_tUpgradeReceivePort, argv[1]:file_length .
        //return mgmtDealMtUpgradeLinux((*(int*)argv[0]), (*(long long*)argv[1]));
        mgmtDealMtUpgradeSetDownloadPer(*(int*)argv[0]);
        break;
    case MGMT_MT_UPGRADE_GET_BURN_PROCESS:
        mgmtDealMtUpgradeGetBurnProcess((int*)argv[0]);
        break;
    case MGMT_MT_UPGRADE_SET_BURN_START:
        mgmtDealMtUpgradeSetBurnStart();
        break;
    case MGMT_NT_UPGRADE_NETWORK_DISCONNECT:
        mgmtDealNtUpgradeNetworkDisconnect();
        break;
#ifndef INCLUDE_HMWMGMT
    case MGMT_MT_UDISK_OUT:
        setUsbNoInsertMessage("\u672a\u68c0\u6d4b\u5230USB\u8bbe\u5907!");
        break;

    // is not huawei'
    case MGMT_HYBROAD_TCPDUMP_DOWN:
        //MONITOR_LOG("test:mgmtModLinux,MGMT_HYBROAD_TCPDUMP_DOWN,\n");
        sendMessageToNativeHandler(MessageType_KeyDown, MV_Maintenance_stopCollectDebugInfo, 0, 0);
        break;
    case MGMT_HYBROAD_UPGRADE_EXIT:
        //MONITOR_LOG("test:mgmtModLinux,MGMT_HYBROAD_UPGRADE_EXIT,\n");
        monitorUpgradeReceiveError();
        break;

#endif
    default:
        break;
    }
    return 0;
}

extern "C"
void *monitorInitLinux(void*)
{
    pthread_detach(pthread_self());
    stbMonitorInit("/usr/local/bin/dataModel.xml");
    return 0;
}

extern "C"
void mgmt_init()
{
    int ret = 0;
    char dataModelPath[] =  "/home/hybroad/share";

    mgmtModuleTr069Init();

    mgmtModuleStbMonitorInit();
    mgmtModuleStbMonitorParamRegist();

    mgmtCmdLineManagerInit();

    shellMapExecuteFunc();
    mgmtCliCommandRegist(); //client command regist func,must follow the init func


    // linux monitor init
    LogSysOperError("monitor init pthread_create....\n");
    pthread_t monitorInit;
    pthread_create(&monitorInit, NULL, monitorInitLinux, NULL);
    setMonitorState(0); // 0:default to close monitor ,1: open
    //printf("test:new monitor,sysinit...\n");

    return;
}
} //Hippo

extern "C"
int getMonitorState()
{
    return monitorState;
}

extern "C"
int setMonitorState(int flag)
{
    if (flag != 0 && flag != 1) {
        LogSafeOperError("Input parameter error\n");
        return -1;
    }

    if (monitorState != flag) {
        monitorState = flag;
        if (monitorState) {
            stbMonitorConnectFlag(1);
        } else {
            stbMonitorConnectFlag(0);
        }
    }
    return 0;
}


