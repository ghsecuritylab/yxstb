#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hmw_mgmtlib.h"
#include "mgmtModule.h"
#include "mgmtModuleTr069.h"
#include "mgmtModuleStbMonitor.h"
#include "MonitorAssertions.h"
#include "SystemManager.h"
#include "mgmtMsgDealFunc.h"
#include "mgmtModuleCmdLine.h"
#include "nm_interface.h"
#include "Tr069Root.h"
#include "mid_sys.h"
#include "MessageValueMaintenancePage.h"
#include "LogModuleHuawei.h"
#include "NativeHandler.h"
#include "MessageTypes.h"

proxy_get_value_func tr069GetValue = NULL;
proxy_set_value_func tr069SetValue = NULL;
proxy_set_value_func sendToMonitor = NULL;
int monitorUDiskOut = 0;


namespace Hippo {

void tr069GetValueCallback(char *name, char *str, unsigned int val);
void tr069SetValueCallback(char *name, char *str, unsigned int val);
void IptvGetCapability(char *capability);
extern "C" void setUsbNoInsertMessage(char *msg);


int nmReadCallBack( const char * szParm, char * pBuf, int iLen)
{
    int iret = 0;
    MONITOR_LOG("%s..\n", szParm);

    iret = mgmtReadConfig(szParm, pBuf, iLen);
    return iret;
}

int nmWriteCallback(const char * szParm, char * pBuf, int iLen)
{
    int iret = 0;

    // Fixme:
    // Log相关的这几个函数的调用并不安全！函数里面没加线程锁。
    if ((strcmp("SetLogServer", szParm) == 0)) {
        huaweiSetLogUDPServer(pBuf);
        huaweiLog();
    } else if ((strcmp("SetLogFtpServer", szParm) == 0)) {
        huaweiSetLogFTPServer(pBuf);
        huaweiLog();
    } else if ((strcmp("SetLogOutType", szParm) == 0)) {
        huaweiSetLogOutPutType(atoi(pBuf));
        huaweiLog();
    } else if ((strcmp("SetLogLevel", szParm) == 0)) {
        huaweiSetLogLevel(atoi(pBuf));
        huaweiLog();
    } else if ((strcmp("SetLogType", szParm) == 0)) {
        huaweiSetLogType(atoi(pBuf));
        huaweiLog();
    }

    iret = mgmtWriteConfig(szParm, pBuf, iLen);
    return iret;
}

char* nmNotifyCallback(int eMsgType, const char* szParm1, const char* szParm2)
{
    int g_upgrade = -1;
    int count = 0;
    int iParm1 = 0;
    int iParm2 = 0;
    char *szResult = NULL;


    MONITOR_LOG("nmNotifyCallback() in %d\n", eMsgType);

    if (!szParm1 || !szParm2) {
        MONITOR_LOG("nmNotifyCallback() parm1 or parm2 is NULL\n");
        return NULL;
    }

    MONITOR_LOG("manager send message to stb: emsgtype=0x%x, szParm1=%s szParm2=%s\n", eMsgType, szParm1, szParm2);

    switch (eMsgType) {
    MONITOR_LOG("%d, %s, %s \n", eMsgType, szParm1, szParm2);
    case MGMT_MT_PLAYER_BY_CHANNO:
        //MONITOR_LOG("test:MONITOR_LOG,BY_CHANNO\n");
        iParm1 = atoi(szParm1);
        mgmtDealMtPlayerbyChanno(&iParm1, &iParm2);
        szResult = (char *)malloc(10);
        memset(szResult, 0, 10);
        sprintf(szResult, "%d", iParm2);
        return szResult;

    case MGMT_MT_PLAYER_BY_URL:
        //MONITOR_LOG("test:MONITOR_LOG,BY_URL\n");
        mgmtDealMtPlayerbyUrl((char*)szParm1, &iParm2);
        szResult = (char *)malloc(10);
        memset(szResult, 0, 10);
        sprintf(szResult, "%d", iParm2);
        return szResult;

    case MGMT_MT_PLAYER_STOP:
        //LogSysOperError("play stop \n");
        mgmtDealMtPlayerStop();
        szResult = (char *)malloc(10);
        memset(szResult, 0, 10);
        sprintf(szResult, "1");
        return szResult;
        break;

    case MGMT_MT_PLAYER_MPCTRL:
        mgmtDealMtPlayerMpctrl((char*)szParm1);
        break;

    case MGMT_MT_ENTER_DEBUG:
        mgmtDealMtEnterDebug();
        break;

    case MGMT_MT_EXIT_DEBUG:
        mgmtDealMtExitDebug();
        break;

    case MGMT_MT_GET_CHANNELNUM_TOTAL:
        mgmtDealMtGetChannelNumToTal(&iParm1);
        MONITOR_LOG("channel num = %d\n", iParm1);
        szResult = (char *)malloc(10);
        memset(szResult, 0, 10);
        sprintf(szResult, "%d", iParm1);
        return szResult;
        break;

    case MGMT_MT_GET_CHANNELINFO_I:
        iParm1 = atoi(szParm1);
        szResult = (char *)malloc(512 * 1024);
        memset(szResult, 0, 512 * 1024);
        mgmtDealMtGetChannelInfo(&iParm1, szResult);
        MONITOR_LOG("channel info len = %d\n", strlen(szResult));
        return szResult;
        break;

    case MGMT_MT_UDISK_OUT:
        break;

    // is not huawei'
    case MGMT_HYBROAD_TCPDUMP_DOWN:
        //MONITOR_LOG("test:mgmtModAnd,MGMT_HYBROAD_TCPDUMP_DOWN,\n");
        sendMessageToNativeHandler(MessageType_KeyDown, MV_Maintenance_stopCollectDebugInfo, 0, 0);
        szResult = (char *)malloc(10);
        memset(szResult, 0, 10);
        sprintf(szResult, "1");
        return szResult;

        break;

    default:
        break;
    }
    return 0;

}


extern "C"
{
    void* mgmtBeginThread(void *param)
    {
        int ret = 0;
        MONITOR_LOG("mgmt_init()....\n");
    	nm_log_callback_set(mgmtLogOutNone);
        mgmtModuleTr069Init();

        mgmtModuleStbMonitorInit();
        mgmtModuleStbMonitorParamRegist();
        mgmtCmdLineManagerInit();
    	nm_getvalue_callback_set(tr069GetValueCallback);
    	nm_setvalue_callback_set(tr069SetValueCallback);
        nm_capability_callback_set(IptvGetCapability);
        sendToMonitor = proxySendToMonitor();

        ret = nm_read_callback_set((nm_io_callback)nmReadCallBack);
        if (ret)
            MONITOR_LOG("register mgmtRegReadCallback fail\n");

        ret = nm_write_callback_set((nm_io_callback)nmWriteCallback);
        if (ret)
            MONITOR_LOG("register mgmtRegWriteCallback fail\n");

        ret = nm_notify_callback_set((nm_notify_callback)nmNotifyCallback);
        if (ret)
            MONITOR_LOG("register mgmtRegNotifyCallback fail\n");

        ret = nm_interface_init();
        if (ret)
            MONITOR_LOG("hmw_mgmtToolsInit fail\n");

        iptvSendMsgToMonitor("iptv", "connect", strlen("connect"));

        shellMapExecuteFunc();
        mgmtCliCommandRegist(); //client command regist func,must follow the init func

    }


    void mgmt_init()
    {
        pthread_t mgmtThreadId = 0;
        pthread_create(&mgmtThreadId, NULL, mgmtBeginThread, NULL);
        return;
    }


    int tr069_api_init(void)
    {
	tr069SetValue = proxy_set_value_call_get();
	tr069GetValue = proxy_get_value_call_get();
	return 0;
    }

    int tr069_api_setValue(char *name, char *str, unsigned int val)
    {
        if (tr069SetValue != NULL)
            tr069SetValue(name, str, val);

        return 0;
    }
    int tr069_api_getValue(char *name, char *str, unsigned int size)
    {
        if (tr069GetValue != NULL)
            tr069GetValue(name, str, size);

        return 0;
    }


    void iptvSendMsgToMonitor(char *name, char *value, int len)
    {
        if (sendToMonitor != NULL)
            sendToMonitor(name, value, len);

    }

    int getMonitorUDiskStatus()
    {
        FILE *fstream = NULL;
        char file_buf[100 * 1024] = {0};
        char lineBuf[1024] = {0};
        char *tok = NULL;
        int udiskNum = 0;

        MONITOR_LOG("int...\n");
        if ((fstream = fopen("/proc/mounts", "r")) == NULL) {
            printf("open /proc/mounts\n");
            MONITOR_LOG("1111int...\n");
            return -1;
        }

        while(fgets(lineBuf, 1024, fstream)) {
            MONITOR_LOG("%s \n", lineBuf);
            if (((tok = strstr(lineBuf, "/mnt/sda/sda")) != NULL)
              ||((tok = strstr(lineBuf, "/mnt/sdb/sdb")) != NULL)) {
                MONITOR_LOG("%s have udisk\n", lineBuf);
                udiskNum++;
                if (udiskNum > 1) {
                    MONITOR_LOG("have 2 udisk\n");
                    setUsbNoInsertMessage("\u4e24\u4e2a U \u76d8\u65e0\u6cd5\u6536\u96c6!");
                    fclose(fstream);
                    return 2;
                }

            }
        }

        fclose(fstream);

        if (udiskNum == 1)
            return 1;
        else
            setUsbNoInsertMessage("\u672a\u68c0\u6d4b\u5230USB\u8bbe\u5907!");
        return 0;
    }
}


void tr069GetValueCallback(char *name, char *str, unsigned int val)
{
	Tr069RootRead(name, str, val);

}

void tr069SetValueCallback(char *name, char *str, unsigned int val)
{
    Tr069RootWrite(name, str, val);
}


void IptvGetCapability(char *capability)
{
    MONITOR_LOG("IptvGetCapability in\n");

    char *strPos = capability;
    if (strPos != NULL) {
        sprintf(strPos, "{module_name:\"IPTV\","
                       "params:["
                                "{param:\"Device.Config.PersistentData\"},"
                                "{param:\"Device.Config.ConfigFile\"},"
                                "{param:\"Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmSwitch\"},"
                                "{param:\"Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmConfig.PacketsLostAlarmValue\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.LogServerUrl\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.LogUploadInterval\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.LogRecordInterval\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.StatInterval\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.PacketsLostR1\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.PacketsLostR2\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.PacketsLostR3\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.PacketsLostR4\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.PacketsLostR5\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.HD_PacketsLostR1\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.HD_PacketsLostR2\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.HD_PacketsLostR3\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.HD_PacketsLostR4\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.HD_PacketsLostR5\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.BitRateR1\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.BitRateR2\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.BitRateR3\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.BitRateR4\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.BitRateR5\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.HD_BitRateR1\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.HD_BitRateR2\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.HD_BitRateR3\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.HD_BitRateR4\"},"
                                "{param:\"Device.X_CTC_IPTV.StatisticConfiguration.HD_BitRateR5\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.Startpoint\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.Endpoint\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.AuthNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.AuthFailNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.AuthFailInfo\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiReqNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiRRT\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiFailNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiFailInfo\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VodReqNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VodRRT\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VodFailNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VodFailInfo\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HTTPReqNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HTTPRRT\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HTTPFailNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HTTPFailInfo\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiAbendNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VODAbendNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiAbendUPNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VODAbendUPNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_MultiAbendNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_VODAbendNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_MultiAbendUPNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_VODAbendUPNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.PlayErrorNumbers\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.PlayErrorInfo\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiPacketsLostR1Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiPacketsLostR2Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiPacketsLostR3Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiPacketsLostR4Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiPacketsLostR5Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.FECMultiPacketsLostR1Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.FECMultiPacketsLostR2Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.FECMultiPacketsLostR3Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.FECMultiPacketsLostR4Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.FECMultiPacketsLostR5Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VODPacketsLostR1Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VODPacketsLostR2Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VODPacketsLostR3Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VODPacketsLostR4Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VODPacketsLostR5Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.ARQVODPacketsLostR1Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.ARQVODPacketsLostR2Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.ARQVODPacketsLostR3Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.ARQVODPacketsLostR4Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.ARQVODPacketsLostR5Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiBitRateR1\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiBitRateR2\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiBitRateR3\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiBitRateR4\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.MultiBitRateR5\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VODBitRateR1\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VODBitRateR2\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VODBitRateR3\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VODBitRateR4\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.VODBitRateR5\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_MultiPacketsLostR1Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_MultiPacketsLostR2Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_MultiPacketsLostR3Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_MultiPacketsLostR4Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_MultiPacketsLostR5Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_FECMultiPacketsLostR1Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_FECMultiPacketsLostR2Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_FECMultiPacketsLostR3Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_FECMultiPacketsLostR4Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_FECMultiPacketsLostR5Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_VODPacketsLostR1Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_VODPacketsLostR2Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_VODPacketsLostR3Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_VODPacketsLostR4Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_VODPacketsLostR5Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_ARQVODPacketsLostR1Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_ARQVODPacketsLostR2Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_ARQVODPacketsLostR3Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_ARQVODPacketsLostR4Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_ARQVODPacketsLostR5Nmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_MultiBitRateR1\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_MultiBitRateR2\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_MultiBitRateR3\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_MultiBitRateR4\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_MultiBitRateR5\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_VODBitRateR1\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_VODBitRateR2\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_VODBitRateR3\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_VODBitRateR4\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.HD_VODBitRateR5\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.BufferIncNmb\"},"
                                "{param:\"Device.X_CTC_IPTV.ServiceStatistics.BufferDecNmb\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.Enable\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.MsgOrFile\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.LogFtpServer\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.LogFtpUser\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.LogFtpPassword\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.Duration\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.RTSPInfo\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.HTTPInfo\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.IGMPInfo\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.PkgTotalOneSec\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.ByteTotalOneSec\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.PkgLostRate\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.AvarageRate\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.BUFFER\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.ERROR\"},"
                                "{param:\"Device.X_CTC_IPTV.LogMsg.VendorExt\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.LogServerUrl\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.LogUploadInterval\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.LogRecordInterval\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.StatInterval\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.PacketsLostR1\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.PacketsLostR2\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.PacketsLostR3\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.PacketsLostR4\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.PacketsLostR5\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.BitRateR1\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.BitRateR2\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.BitRateR3\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.BitRateR4\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.BitRateR5\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.FramesLostR1\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.FramesLostR2\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.FramesLostR3\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.FramesLostR4\"},"
                                "{param:\"Device.X_CU_STB.StatisticConfiguration.FramesLostR5\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.Startpoint\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.Endpoint\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.AuthNumbers\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.AuthFailNumbers\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.AuthFailInfo\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.MultiReqNumbers\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.MultiFailNumbers\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.MultiFailInfo\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.VodReqNumbers\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.VodFailNumbers\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.VodFailInfo\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.HTTPReqNumbers\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.HTTPFailNumbers\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.HTTPFailInfo\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.MutiAbendNumbers\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.VODAbendNumbers\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.PlayErrorNumbers\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.PlayErrorInfo\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.MultiPacketsLostR1Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.MultiPacketsLostR2Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.MultiPacketsLostR3Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.MultiPacketsLostR4Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.MultiPacketsLostR5Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.VODPacketsLostR1Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.VODPacketsLostR2Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.VODPacketsLostR3Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.VODPacketsLostR4Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.VODPacketsLostR5Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.MultiBitRateR1Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.MultiBitRateR2Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.MultiBitRateR3Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.MultiBitRateR4Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.MultiBitRateR5Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.VODBitRateR1Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.VODBitRateR2Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.VODBitRateR3Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.VODBitRateR4Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.VODBitRateR5Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.FramesLostR1Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.FramesLostR2Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.FramesLostR3Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.FramesLostR4Nmb\"},"
                                "{param:\"Device.X_CU_STB.ServiceStatistics.FramesLostR5Nmb\"},"
                                "{param:\"Device.X_00E0FC.AlarmSwitch\"},"
                                "{param:\"Device.X_00E0FC.AlarmReportLevel\"},"
                                "{param:\"Device.X_00E0FC.CPUAlarmValue\"},"
                                "{param:\"Device.X_00E0FC.MemoryAlarmValue\"},"
                                "{param:\"Device.X_00E0FC.DiskAlarmValue\"},"
                                "{param:\"Device.X_00E0FC.BandwidthAlarmValue\"},"
                                "{param:\"Device.X_00E0FC.PacketsLostAlarmValue\"},"
                                "{param:\"Device.X_00E0FC.ErrorCodeSwitch\"},"
                                "{param:\"Device.X_00E0FC.ErrorCodeInterval\"},"
                                "{param:\"Device.X_00E0FC.SQMConfiguration.SQMLisenPort\"},"
                                "{param:\"Device.X_00E0FC.SQMConfiguration.SQMServerPort\"},"
                                "{param:\"Device.X_00E0FC.PlayDiagnostics.DiagnosticsState\"},"
                                "{param:\"Device.X_00E0FC.PlayDiagnostics.PlayURL\"},"
                                "{param:\"Device.X_00E0FC.PlayDiagnostics.PlayState\"},"
                                "{param:\"Device.X_00E0FC.AutoOnOffConfiguration.IsAutoPowerOn\"},"
                                "{param:\"Device.X_00E0FC.AutoOnOffConfiguration.AutoPowerOnTime\"},"
                                "{param:\"Device.X_00E0FC.AutoOnOffConfiguration.AutoShutdownTime\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.HTTPReqNumbers\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.HTTPFailInfo\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.VodFailNumbers\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.VodFailInfo\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.MultiReqNumbers\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.VODPacketsLostR1\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.VODPacketsLostR2\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.VODPacketsLostR3\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.VODPacketsLostR4\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.VODPacketsLostR5\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR1\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR2\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR3\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR4\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR5\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.VodReqNumbers\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.HTTPFailNumbers\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.AuthFailInfo\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.AuthNumbers\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.MultiFailNumbers\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.MultiFailInfo\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.AuthFailNumbers\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.Endpoint\"},"
                                "{param:\"Device.X_00E0FC.ServiceStatistics.Startpoint\"},"
                                "{param:\"Device.X_00E0FC.StatisticConfiguration.LogServerUrl\"},"
                                "{param:\"Device.X_00E0FC.StatisticConfiguration.PacketsLostR1\"},"
                                "{param:\"Device.X_00E0FC.StatisticConfiguration.PacketsLostR2\"},"
                                "{param:\"Device.X_00E0FC.StatisticConfiguration.PacketsLostR3\"},"
                                "{param:\"Device.X_00E0FC.StatisticConfiguration.PacketsLostR4\"},"
                                "{param:\"Device.X_00E0FC.StatisticConfiguration.PacketsLostR5\"},"
                                "{param:\"Device.X_00E0FC.StatisticConfiguration.LogUploadInterval\"},"
                                "{param:\"Device.X_00E0FC.StatisticConfiguration.BitRateR1\"},"
                                "{param:\"Device.X_00E0FC.StatisticConfiguration.BitRateR2\"},"
                                "{param:\"Device.X_00E0FC.StatisticConfiguration.BitRateR3\"},"
                                "{param:\"Device.X_00E0FC.StatisticConfiguration.BitRateR4\"},"
                                "{param:\"Device.X_00E0FC.StatisticConfiguration.BitRateR5\"},"
                                "{param:\"Device.X_00E0FC.StatisticConfiguration.StatInterval\"},"
                                "{param:\"Device.X_00E0FC.StatisticConfiguration.LogRecordInterval\"},"
                                "{param:\"ParasListMain\"},"
                                "{param:\"ParasListPip\"},"
                                "{param:\"Stream1ParaList\"},"
                                "{param:\"Stream2ParaList\"},"
                                "{param:\"SetLogOutType\"},"
                                "{param:\"SetLogType\"},"
                                "{param:\"SetLogLevel\"},"
                                "{param:\"LogFtpServer\"},"
                                "{param:\"LogServer\"},"
                                "{param:\"debugInfoStatus\"}"
                                "]"
                                "}");

    }
    else
        return;
}




/*以下为空函数*/
extern "C"
{
    struct cli_def {};
    void cli_print(struct cli_def *cli, const char *format, ...)
    {
    }

    int hmw_mgmtCliRegCmd(unsigned int argc, void *argv[])
    {
        return 0;
    }

    int hmw_mgmtRegLogCallback(void*  fnCallback)
    {
        return 0;
    }

    /*do nothing ,just control libMgmt do not print debug info!!!*/
    void mgmtLogOutNone(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...)
    {

    }
}

}

