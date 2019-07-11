#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mgmtModuleStbMonitor.h"
#include "LogModuleHuawei.h"
#include "mgmtModuleParam.h"
#include "MonitorAssertions.h"

static MgmtModuleStbMonitor* g_StbMonitorMgmtInstance = NULL;

MgmtModuleStbMonitor::MgmtModuleStbMonitor()
{
}

int MgmtModuleStbMonitor::StbMonitorParamReg(const char *param, void *rfunc, void *wfunc, int type)
{
	ParamMapFunc  StbMonitormapNode;

	if (type == INT_TYPE) {
		StbMonitormapNode.getfunc.getint = (mgmtGetIntFunc)rfunc;
		StbMonitormapNode.setfunc.setint = (mgmtSetIntFunc)wfunc;
	} else if (type == UINT_TYPE) {
		StbMonitormapNode.getfunc.getuint = (mgmtGetUintFunc)rfunc;
		StbMonitormapNode.setfunc.setuint = (mgmtSetUintFunc)wfunc;
	}else if (type == STRING_TYPE) {
		StbMonitormapNode.getfunc.getstring = (mgmtGetStringFunc)rfunc;
		StbMonitormapNode.setfunc.setstring = (mgmtSetStringFunc)wfunc;
	}

    StbMonitormapNode.paramtype = type;
    m_StbMonitorParamMap[param] = StbMonitormapNode;

    return 0;
}

int   mgmtParamRegister(const char *param, void *rfunc, void *wfunc, int type)
{
    return getStbMonitorMgmtInstance()->StbMonitorParamReg(param, rfunc, wfunc, type);
}

int mgmtReadConfig(const char *szParm, char *pBuf, int iLen)
{
    if (NULL == szParm || NULL == pBuf)
        return -1;

    StbMonitorParamMap::const_iterator it;
    it = getStbMonitorMgmtInstance( )->m_StbMonitorParamMap.find(szParm);
    if(it == getStbMonitorMgmtInstance( )->m_StbMonitorParamMap.end())
        return -1;
	else{
        const ParamMapFunc& paramNode = (it->second);
        if(paramNode.getfunc.getint != NULL && paramNode.paramtype == INT_TYPE) {
	        snprintf(pBuf, iLen, "%d", (paramNode.getfunc.getint)( ));
        }
        else if (paramNode.getfunc.getuint != NULL && paramNode.paramtype == UINT_TYPE) {
	        snprintf(pBuf, iLen, "%u", (paramNode.getfunc.getuint)( ));
	    }
	    else if (paramNode.getfunc.getstring != NULL && paramNode.paramtype == STRING_TYPE) {
	        (paramNode.getfunc.getstring)(pBuf, iLen);
	    }
		MONITOR_LOG("stbMonitor read %s = %s\n\n",szParm,pBuf);
	    return 0;
    }
}

int mgmtWriteConfig(const char *szParm, char *pBuf, int iLen)
{
    if (NULL == szParm || NULL == pBuf)
        return -1;

    StbMonitorParamMap::const_iterator it;
    it = getStbMonitorMgmtInstance( )->m_StbMonitorParamMap.find(szParm);
    if(it == getStbMonitorMgmtInstance( )->m_StbMonitorParamMap.end())
        return -1;
	else{
		const ParamMapFunc& paramNode = (it->second);
        if(paramNode.setfunc.setint != NULL && paramNode.paramtype == INT_TYPE)
	         (paramNode.setfunc.setint)(atoi(pBuf));
        else if (paramNode.setfunc.setuint != NULL && paramNode.paramtype == UINT_TYPE)
	         (paramNode.setfunc.setuint)((unsigned int)atoi(pBuf));
	    else if (paramNode.setfunc.setstring != NULL && paramNode.paramtype == STRING_TYPE)
	         (paramNode.setfunc.setstring)(pBuf);
	    MONITOR_LOG("stbMonitor write %s = %s\n\n",szParm,pBuf);

	    return 0;
    }
}

int mgmtModuleStbMonitorInit()
{
    if(!g_StbMonitorMgmtInstance)
        g_StbMonitorMgmtInstance = new MgmtModuleStbMonitor();
    return 0;
}

MgmtModuleStbMonitor* getStbMonitorMgmtInstance( )
{
    if(!g_StbMonitorMgmtInstance)
        return NULL;
    return g_StbMonitorMgmtInstance;
}

int MgmtPlayByUrlIoctl(char * buf ,int len)
{
   MONITOR_LOG("*********************just begin to play!!!!!****************\n");
   return 0;
}
int mgmtModuleStbMonitorParamRegist()
{
    mgmtParamRegister("Iptv.EDSAddress", (void *)MgmtHomepageRead , (void *)MgmtHomepageWrite, STRING_TYPE);
    mgmtParamRegister("Iptv.EDSAddressBackup",(void *)MgmtHomepageBackupRead ,(void *)MgmtHomepageBackupWrite, STRING_TYPE);

    mgmtParamRegister("NTPDomain", (void *)MgmtNTPRead, (void *)MgmtNTPWrite, STRING_TYPE);
    mgmtParamRegister("NTPDomainBackup",(void *)MgmtNTPBackupRead ,(void *)MgmtNTPBackupWrite, STRING_TYPE);

    mgmtParamRegister("NetUserID",(void *)MgmtNetUserIDRead ,(void *)MgmtNetUserIDWrite, STRING_TYPE);
    mgmtParamRegister("NetPwd",(void *)MgmtNetUserPasswordRead ,(void *)MgmtNetUserPasswordWrite, STRING_TYPE);
    mgmtParamRegister("Iptv.Account",(void *)MgmtNtvUserAccountRead ,   (void *)MgmtNtvUserAccountWrite, STRING_TYPE);
    mgmtParamRegister("Iptv.Password",(void *)MgmtNtvUserPasswordRead , (void *)MgmtNtvUserPasswordWrite, STRING_TYPE);
    mgmtParamRegister("ntvuseraccount",(void *)MgmtNtvUserAccountRead , (void *)MgmtNtvUserAccountWrite, STRING_TYPE);
    mgmtParamRegister("ntvuserpassword",(void *)MgmtNtvUserPasswordRead,(void *)MgmtNtvUserPasswordWrite, STRING_TYPE);

    mgmtParamRegister("Network.DHCPAccount",(void *)MgmtDhcpAccountRead  ,(void *)MgmtDhcpAccountWrite, STRING_TYPE);
    mgmtParamRegister("Network.DHCPPassword",(void *) MgmtDhcpPasswordRead ,(void *)MgmtDhcpPasswordWrite, STRING_TYPE);
    mgmtParamRegister("Network.PPPoEAccount",(void *) MgmtNetUserIDRead ,(void *)MgmtNetUserIDWrite, STRING_TYPE);
    mgmtParamRegister("Network.PPPoEPassword",(void *) MgmtNetUserPasswordRead ,(void *)MgmtNetUserPasswordWrite, STRING_TYPE);


    mgmtParamRegister("Network.ConnectType",(void *)MgmtConnectTypeRead  ,(void *)MgmtConnectTypeWrite, STRING_TYPE);
    mgmtParamRegister("Network.op.NetIP",(void *) MgmtIpAddressRead ,(void *)MgmtIpAddressWrite, STRING_TYPE);
    mgmtParamRegister("Network.op.NetMask",(void *) MgmtNetmaskRead ,(void *)MgmtNetmaskWrite, STRING_TYPE);
    mgmtParamRegister("Network.op.NetGateway",(void *) MgmtNetGatewayRead ,(void *)MgmtNetGatewayWrite, STRING_TYPE);
    mgmtParamRegister("Network.op.NetDns",(void *)MgmtNetDns1Read  ,(void *)MgmtNetDns1Write, STRING_TYPE);
    mgmtParamRegister("Network.op.NetDns2",(void *) MgmtNetDnsBackupRead ,(void *)MgmtNetDnsBackupWrite, STRING_TYPE);
    mgmtParamRegister("defContAcc",(void *)MgmtdefContAccRead  ,(void *)MgmtdefContAccWrite, STRING_TYPE);
    mgmtParamRegister("defContPwd",NULL ,(void *)MgmtdefContPwdWrite, STRING_TYPE);
    mgmtParamRegister("System.DirectPlay",(void *) MgmtDirectplayRead ,(void *)MgmtDirectplayWrite, STRING_TYPE);
    mgmtParamRegister("Mgmtepgurl",(void *)MgmtEpgurlRead  ,(void *)MgmtEpgurlWrite, STRING_TYPE);
    mgmtParamRegister("Network.TimeZone",(void *) MgmtTimezoneRead ,(void *)MgmtTimezoneWrite, STRING_TYPE);
    mgmtParamRegister("localTime",(void *) MgmtLocalTimeRead ,NULL, STRING_TYPE);
    //    mgmtParamRegister("System.BrowserRegion",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("System.CAMode",(void *)MgmtCAModeRead, NULL, STRING_TYPE);
    mgmtParamRegister("LogServerUrl",(void *) MgmtLogServerUrlRead ,(void *)MgmtLogServerUrlWrite, STRING_TYPE);
    mgmtParamRegister("LogUploadInterval",(void *) MgmtLogUploadIntervalRead ,(void *)MgmtLogUploadIntervalWrite, STRING_TYPE);
    mgmtParamRegister("QoSLogSwitch",(void *) MgmtQosLogSwitchRead ,(void *)MgmtQosLogSwitchWrite, STRING_TYPE);
    mgmtParamRegister("QOSLogEnables",(void *) MgmtQosLogSwitchRead ,(void *)MgmtQosLogSwitchWrite, STRING_TYPE);

    //  mgmtParamRegister("TVMSGWIP",(void *)MgmtTVMSGWIPRead  ,(void *)MgmtTVMSGWIPWrite, STRING_TYPE);
#ifdef TVMS_OPEN
    mgmtParamRegister("TVMSHeartbitInterval",(void *)MgmtTVMSHeartbitIntervalRead ,(void *)MgmtTVMSHeartbitIntervalWrite, STRING_TYPE);
    mgmtParamRegister("TVMSDelayLength",(void *)MgmtTVMSDelayLengthRead  ,(void *)MgmtTVMSDelayLengthWrite, STRING_TYPE);
    mgmtParamRegister("TVMSHeartbitUrl",(void *)MgmtTVMSHeartbitUrlRead  ,(void *)MgmtTVMSHeartbitUrlWrite, STRING_TYPE);
    mgmtParamRegister("TVMSVODHeartbitUrl",(void *)MgmtTVMSVODHeartbitUrlRead  ,(void *)MgmtTVMSVODHeartbitUrlWrite, STRING_TYPE);
#endif
    mgmtParamRegister("templateName",(void *) MgmtTemplateNameRead ,(void *)MgmtTemplateNameWrite, STRING_TYPE);
    mgmtParamRegister("areaid",(void *)MgmtareaidRead  ,(void *)MgmtareaidWrite, STRING_TYPE);
    mgmtParamRegister("ManagementDomain",(void *)MgmtTMSURLRead  ,(void *)MgmtTMSURLWrite, STRING_TYPE);
    mgmtParamRegister("BrowserTime",(void *)MgmtBrowserTimeRead  ,NULL, STRING_TYPE);

    mgmtParamRegister("System.TMSEnable",(void *) MgmtTMSEnableRead ,(void *)MgmtTMSEnableWrite, STRING_TYPE);
    mgmtParamRegister("SupportHD",(void *) MgmtSupportHDRead ,(void *)MgmtSupportHDWrite, STRING_TYPE);
    mgmtParamRegister("System.VideoZoomMode",(void *)MgmtAspectRatioRead,(void *)MgmtAspectRatioRead, STRING_TYPE);
    mgmtParamRegister("System.Standard",(void *) MgmtSDVideoStandardRead ,(void *)MgmtSDVideoStandardWrite, STRING_TYPE);
    mgmtParamRegister("System.HDStandard",(void *)MgmtHDVideoStandardRead  ,(void *)MgmtHDVideoStandardWrite, STRING_TYPE);
    mgmtParamRegister("System.SoftwareVersion",(void *)MgmtSoftwareVersionRead  ,(void *)MgmtSoftwareVersionWrite, STRING_TYPE);
    mgmtParamRegister("STBVersion",(void *)MgmtHWVersionRead  , NULL, STRING_TYPE);
    mgmtParamRegister("System.ReleaseTime",(void *)MgmtComptimeRead  ,(void *)MgmtComptimeWrite, STRING_TYPE);
    mgmtParamRegister("System.HardwareVersion",(void *)MgmtHardwareVersionRead  ,(void *)MgmtHardwareVersionWrite, STRING_TYPE);
    mgmtParamRegister("System.BrowserVersion",(void *)MgmtBrowserVersionRead  ,(void *)MgmtBrowserVersionWrite, STRING_TYPE);
    mgmtParamRegister("System.SerialNumber",(void *)MgmtSerialNumberRead  ,(void *)MgmtSerialNumberWrite, STRING_TYPE);
    mgmtParamRegister("System.ChipSeriaNumber",(void *)MgmtChipSeriaNumberRead  ,(void *)MgmtChipSeriaNumberRead, STRING_TYPE);
    mgmtParamRegister("System.MACAddress",(void *)MgmtMACAddressRead, NULL, STRING_TYPE);
    mgmtParamRegister("System.WorkModel",(void *)MgmtWorkModelRead  ,(void *)MgmtWorkModelWrite, STRING_TYPE);
    mgmtParamRegister("System.ChannelSmooth",(void *) MgmtChannelSwitchModeRead ,(void *)MgmtChannelSwitchModeWrite, STRING_TYPE);
    //    mgmtParamRegister("System.WatchDogSwitch",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("System.AntiFlickerSwitch",(void *) MgmtAntiFlickerSwitchRead ,(void *)MgmtAntiFlickerSwitchWrite, STRING_TYPE);
    mgmtParamRegister("System.CAType",(void *) MgmtCAModeRead ,NULL, STRING_TYPE);

    mgmtParamRegister("System.UpgradeServer",(void *)MgmtUpgradeUrlRead, (void *)MgmtUpgradeUrlWrite, STRING_TYPE);
    mgmtParamRegister("UpgradeServer",(void *)MgmtUpgradeUrlRead, (void *)MgmtUpgradeUrlWrite, STRING_TYPE);
    mgmtParamRegister("Device.ManagementServer.Username",(void *) MgmtTMSUsernameRead ,(void *)MgmtTMSUsernameWrite, STRING_TYPE);
    mgmtParamRegister("Device.ManagementServer.Password",(void *) MgmtTMSPasswordRead ,(void *)MgmtTMSPasswordWrite, STRING_TYPE);
    mgmtParamRegister("Device.ManagementServer.PeriodicInformEnable",(void *)MgmtTMSHeartBitRead  ,(void *)MgmtTMSHeartBitWrite, STRING_TYPE);
    mgmtParamRegister("Device.ManagementServer.PeriodicInformInterval",(void *)MgmtTMSHeartBitIntervalRead ,(void *)MgmtTMSHeartBitIntervalWrite, STRING_TYPE);
    mgmtParamRegister("Device.X_00E0FC.LogParaConfiguration.LogLevel",(void *)MgmtSysLogLevelRead  ,(void *)MgmtSysLogLevelWrite, STRING_TYPE);
    mgmtParamRegister("Device.X_00E0FC.LogParaConfiguration.LogType",(void *)MgmtSysLogTypeRead  ,(void *)MgmtSysLogTypeWrite, STRING_TYPE);
    mgmtParamRegister("Device.DeviceInfo.ProductClass",(void *) MgmtProductClassRead ,(void *)MgmtProductClassWrite, STRING_TYPE);
    mgmtParamRegister("LogFtpServer",(void *)MgmtSysLogFtpServerRead ,(void *)MgmtSysLogFtpServerWrite, STRING_TYPE);
    mgmtParamRegister("Device.X_00E0FC.LogParaConfiguration.SyslogServer",(void *) MgmtSysLogServerRead ,(void *)MgmtSysLogServerWrite, STRING_TYPE);
    mgmtParamRegister("Device.X_00E0FC.LogParaConfiguration.LogOutPutType",(void *) MgmtSysLogOutPutTypeRead ,(void *)MgmtSysLogOutPutTypeWrite, STRING_TYPE);
    mgmtParamRegister("TransportProtocol",(void *)MgmtTransportProtocolRead  ,(void *)MgmtTransportProtocolWrite, STRING_TYPE);
    mgmtParamRegister("Paralist",(void *)MgmtParaListRead  ,NULL, STRING_TYPE);
//    mgmtParamRegister("Channellist",NULL ,NULL, STRING_TYPE);

    mgmtParamRegister("System.BrowserLogSwitch",(void *)MgmtBrowserLogSwitchRead  ,(void *)MgmtBrowserLogSwitchWrite, STRING_TYPE);
    mgmtParamRegister("Network.AccessType",(void *) MgmtConnectTypeRead ,(void *)MgmtConnectTypeWrite, STRING_TYPE);
    //need to add

    mgmtParamRegister("Wifi.ConnectType",(void *) MgmtConnectTypeRead ,(void *)MgmtConnectTypeWrite, STRING_TYPE);
    mgmtParamRegister("Wifi.StaticIP",(void *) MgmtIpAddressRead ,(void *)MgmtIpAddressWrite, STRING_TYPE);
    mgmtParamRegister("Wifi.StaticGateway",(void *) MgmtNetGatewayRead ,(void *)MgmtNetGatewayWrite, STRING_TYPE);
    mgmtParamRegister("Wifi.StaticMask",(void *) MgmtNetmaskRead ,(void *)MgmtNetmaskWrite, STRING_TYPE);
    mgmtParamRegister("Wifi.StaticDNS1",(void *) MgmtNetDns1Read ,(void *)MgmtNetDns1Write, STRING_TYPE);
    mgmtParamRegister("Wifi.StaticDNS2",(void *) MgmtNetDns1Read ,(void *)MgmtNetDns1Write, STRING_TYPE);


    mgmtParamRegister("stb_epgurl",(void *) MgmtEpgUrlRead ,(void *)MgmtEpgUrlWrite, STRING_TYPE);

    // todo:have not used telnet ,instead of ssh, "initTelnet" must be changed.
    mgmtParamRegister("initTelnet",(void *) MgmtInitSSHRead ,(void *)MgmtInitSSHWrite, STRING_TYPE);

    mgmtParamRegister("stbmonitorUser",(void *) MgmtStbMonitorUserRead ,(void *)MgmtStbMonitorUserWrite, STRING_TYPE);
    mgmtParamRegister("stbmonitorPwd",(void *) MgmtStbMonitorPwdRead ,(void *)MgmtStbMonitorPwdWrite, STRING_TYPE);
    mgmtParamRegister("hw_op_CAregister", (void *)MgmtHwopCAregisterRead, (void *)MgmtHwopCAregisterWrite,STRING_TYPE); // 是否启用CAS功能 支持读取、写入。值为0时关系,非0时开启.
    mgmtParamRegister("serialPortServiceEnable", (void *)MgmtserialPortServiceEnableRead, (void*)MgmtserialPortServiceEnableWrite,STRING_TYPE); // serialPortServiceEnable	串口功能是否启用	支持读取、写入。机顶盒中默认永远是“0”，如果设置为“1”，仅当次开机有效，断电重启后恢复为“0”。取值：1：表示使能；0：表示禁用

//stbmonitor play
//	mgmtParamRegister("play",NULL, (void *)MgmtPlayByUrlIoctl, STRING_TYPE);
 //   mgmtParamRegister("pause",(void *)  ,(void *), STRING_TYPE);
 //   mgmtParamRegister("stop",(void *)  ,(void *), STRING_TYPE);
 //   mgmtParamRegister("fast_forward",(void *)  ,(void *), STRING_TYPE);
  //  mgmtParamRegister("fast_backward",(void *)  ,(void *), STRING_TYPE);

#if 0
    mgmtParamRegister("auto_standby_flag",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("auto_standby_time",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("selftesturl",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("test_mode",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("enable_fastcache",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("HDCPEnable",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("HDCPEnableDefault",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("CGMSAEnable",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("CGMSAEnableDefault",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("MacrovisionEnable",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("MacrovisionEnableDefault",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("HDCPProtectMode",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("reboot",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("restore_setting",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("set_test_mode",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("set_autotest_mode",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("set_work_mode",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("set_scriptrecord_mode",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("set_log_out_type",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("set_log_level",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("set_log_type",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("upgrade",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("set_upgradelength",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("get_upgradeprecent",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("ping",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("traceroute",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("pcap_maxfilesize",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("remotepcap",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("get_pcapfilesize",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("get_capfileuploadsize",(void *)  ,(void *), STRING_TYPE);
    mgmtParamRegister("collect_stbStatus",(void *)  ,(void *), STRING_TYPE);
#endif

#ifndef INCLUDE_HMWMGMT
    printf("sadfsafsadfdsfsd........\n");

    mgmtParamRegister("StartupCaptured",(void *)getMgmtStartupCaptured  ,(void *)setMgmtStartupCaptured, INT_TYPE);
    mgmtParamRegister("StartupUploadAddr",(void *)getMgmtStartupUploadAddr, (void *)setMgmtStartupUploadAddr, STRING_TYPE);
    mgmtParamRegister("debugInfoStatus",(void *)getMgmtDebugInfoStatus, (void *)setMgmtDebugInfoStatus, INT_TYPE);
    mgmtParamRegister("ParasListMain",(void *)getMgmtParasListMain, NULL, STRING_TYPE);
    mgmtParamRegister("ParasListPip",(void *)getMgmtParasListPip, NULL, STRING_TYPE);
    mgmtParamRegister("Stream1ParaList",(void *)getMgmtStream1ParaList, NULL, STRING_TYPE);
    mgmtParamRegister("Stream2ParaList",(void *)getMgmtStream2ParaList, NULL, STRING_TYPE);

    //mgmtParamRegister("LogServer", (void *)getMgmtLogServer, (void *)setMgmtLogServer, STRING_TYPE);
    //mgmtParamRegister("LogFtpServer", (void *)getMgmtLogFtpServer, (void *)setMgmtLogFtpServer, STRING_TYPE);


#endif
    return 0;
}
