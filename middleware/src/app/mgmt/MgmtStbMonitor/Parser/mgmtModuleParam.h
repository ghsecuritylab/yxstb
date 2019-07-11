#ifndef MGMTMODULEPARAM_H
#define MGMTMODULEPARAM_H

#ifdef __cplusplus
extern "C" {
#endif

int MgmtHomepageRead(char  *buf, int len);
int MgmtHomepageWrite(char *buf, int len);
int MgmtHomepageBackupRead(char  *buf, int len);
int MgmtHomepageBackupWrite(char *buf, int len);
 int MgmtNTPRead(char *buf, int len);
 int MgmtNTPWrite(char *buf, int len);
 int MgmtNTPBackupRead(char *buf, int len);
 int MgmtNTPBackupWrite(char *buf, int len);
int MgmtNetUserIDRead(char *buf, int len);
int MgmtNetUserIDWrite(char *buf, int len);
int MgmtNtvUserAccountRead(char * buf, int len);
int MgmtNtvUserAccountWrite(char * buf, int len);
int MgmtNtvUserPasswordRead(char * buf, int len);
int MgmtNtvUserPasswordWrite(char * buf, int len);
int MgmtNetUserPasswordRead(char * buf, int len);
int MgmtNetUserPasswordWrite(char * buf, int len);
int MgmtDhcpAccountRead(char * buf, int len);
int MgmtDhcpAccountWrite(char * buf, int len);
int MgmtDhcpPasswordRead(char * buf, int len);
int MgmtDhcpPasswordWrite(char * buf, int len);
int MgmtConnectTypeRead(char * buf, int len);
int MgmtConnectTypeWrite(char * buf, int len);
int MgmtIpAddressRead(char * buf, int len);
int MgmtIpAddressWrite(char * buf, int len);
int MgmtNetmaskRead(char * buf, int len);
int MgmtNetmaskWrite(char * buf, int len);
int MgmtNetGatewayRead(char * buf, int len);
int MgmtNetGatewayWrite(char * buf, int len);
int MgmtNetDns1Read(char * buf, int len);
int MgmtNetDns1Write(char * buf, int len);
int MgmtNetDnsBackupRead(char * buf, int len);
int MgmtNetDnsBackupWrite(char * buf, int len);
int MgmtdefContAccRead(char * buf, int len);
int MgmtdefContAccWrite(char * buf, int len);
int MgmtdefContPwdWrite(char * buf, int len);
int MgmtDirectplayRead(char * buf, int len);
int MgmtDirectplayWrite(char * buf, int len);
int MgmtEpgurlRead(char * buf, int len);
int MgmtEpgurlWrite(char * buf, int len);
int MgmtTimezoneRead(char * buf, int len);
int MgmtTimezoneWrite(char * buf, int len);
int MgmtLocalTimeRead(char * buf, int len);
int  MgmtCAregisterRead(char * buf, int len);
int MgmtCAregisterWrite(char * buf, int len);
int MgmtCAModeRead(char * buf, int len);
int MgmtLogServerUrlRead(char * buf, int len);
int MgmtLogServerUrlWrite(char * buf, int len);
int MgmtLogRecordIntervalRead(char * buf, int len);
int MgmtLogRecordIntervalWrite(char * buf, int len);
int MgmtLogUploadIntervalRead(char * buf, int len);
int MgmtLogUploadIntervalWrite(char * buf, int len);
 int MgmtQosLogSwitchRead(char * buf, int len);
int MgmtQosLogSwitchWrite(char * buf, int len);
int MgmtTVMSGWIPRead(char * buf, int len);
int MgmtTVMSGWIPWrite(char * buf, int len);
int MgmtTVMSHeartbitIntervalRead(char * buf, int len);
int MgmtTVMSHeartbitIntervalWrite(char * buf, int len);
int  MgmtTVMSDelayLengthRead(char * buf, int len);
int  MgmtTVMSDelayLengthWrite(char * buf, int len);
int  MgmtTVMSHeartbitUrlRead(char * buf, int len);
int  MgmtTVMSHeartbitUrlWrite(char * buf, int len);
int  MgmtTVMSVODHeartbitUrlRead(char * buf, int len);
int  MgmtTVMSVODHeartbitUrlWrite(char * buf, int len);
int  MgmtTemplateNameRead(char * buf, int len);
int  MgmtTemplateNameWrite(char * buf, int len);
int  MgmtareaidRead(char * buf, int len);
int  MgmtareaidWrite(char * buf, int len);
int  MgmtTMSURLRead(char * buf, int len);
int  MgmtTMSURLWrite(char * buf, int len);
int  MgmtTMSUsernameRead(char * buf, int len);
int  MgmtTMSUsernameWrite(char * buf, int len);
int  MgmtTMSPasswordRead(char * buf, int len);
int  MgmtTMSPasswordWrite(char * buf, int len);
int  MgmtTMSEnableRead(char * buf, int len);
int  MgmtTMSEnableWrite(char * buf, int len);
int  MgmtTMSHeartBitRead(char * buf, int len);
int  MgmtTMSHeartBitWrite(char * buf, int len);
int MgmtTMSHeartBitIntervalRead(char * buf, int len);
int MgmtTMSHeartBitIntervalWrite(char * buf, int len);
int  MgmtSupportHDRead(char * buf, int len);
int  MgmtSupportHDWrite(char * buf, int len);
int  MgmtAspectRatioRead(char * buf, int len);
int  MgmtAspectRatioWrite(char * buf, int len);
int  MgmtSDVideoStandardRead(char * buf, int len);
int  MgmtSDVideoStandardWrite(char * buf, int len);
int  MgmtHDVideoStandardRead(char * buf, int len);
int  MgmtHDVideoStandardWrite(char * buf, int len);
int  MgmtProductClassRead(char * buf, int len);
int  MgmtProductClassWrite(char * buf, int len);
int  MgmtSoftwareVersionRead(char * buf, int len);
int  MgmtSoftwareVersionWrite(char * buf, int len);
int  MgmtHWVersionRead(char * buf, int len);
int  MgmtComptimeRead(char * buf, int len);
int  MgmtComptimeWrite(char * buf, int len);
int  MgmtHardwareVersionRead(char * buf, int len);
int  MgmtHardwareVersionWrite(char * buf, int len);
int  MgmtBrowserVersionRead(char * buf, int len);
int  MgmtBrowserVersionWrite(char * buf, int len);
int  MgmtBrowserTimeRead(char * buf, int len);
int  MgmtSerialNumberRead(char * buf, int len);
int  MgmtSerialNumberWrite(char * buf, int len);
int MgmtChipSeriaNumberRead(char * buf, int len);
int MgmtChipSeriaNumberWrite(char * buf, int len);
int  MgmtMACAddressRead(char * buf, int len);
int  MgmtUpgradeUrlRead(char * buf, int len);
int  MgmtUpgradeUrlWrite(char * buf, int len);
int  MgmtSysLogLevelRead(char * buf, int len);
int  MgmtSysLogLevelWrite(char * buf, int len);
int  MgmtSysLogTypeRead(char * buf, int len);
int  MgmtSysLogTypeWrite(char * buf, int len);
int  MgmtSysLogFtpServerRead(char * buf, int len);
int  MgmtSysLogFtpServerWrite(char * buf, int len);
int  MgmtSysLogServerRead(char * buf, int len);
int  MgmtSysLogServerWrite(char * buf, int len);
int  MgmtSysLogOutPutTypeRead(char * buf, int len);
int  MgmtSysLogOutPutTypeWrite(char * buf, int len);
int  MgmtWorkModelRead(char * buf, int len);
int  MgmtWorkModelWrite(char * buf, int len);
int  MgmtChannelSwitchModeRead(char * buf, int len);
int  MgmtChannelSwitchModeWrite(char * buf, int len);
int  MgmtAntiFlickerSwitchRead(char * buf, int len);
int  MgmtAntiFlickerSwitchWrite(char * buf, int len);
int  MgmtTransportProtocolRead(char * buf, int len);
int  MgmtTransportProtocolWrite(char * buf, int len);
int  MgmtSSHServiceEnableRead(char * buf, int len);
int  MgmtSSHServiceEnableWrite(char * buf, int len);
int MgmtInitSSHRead(char * buf ,int len);
int MgmtInitSSHWrite(char * buf,int len);
int  MgmtserialPortServiceEnableRead(char * buf, int len);
int  MgmtserialPortServiceEnableWrite(char * buf, int len);
int  MgmtBrowserLogSwitchRead(char * buf, int len);
int  MgmtBrowserLogSwitchWrite(char * buf, int len);
int  MgmtSTBTypeRead(char * buf, int len);
int  MgmtChipIdRead(char * buf, int len);
int MgmtPlatformCodeRead(char * buf, int len);
int MgmtPlatformCodeWrite(char * buf, int len);
int MgmtPPVlistRead(char * buf, int len);
int MgmtParaListRead(char *buf ,int length);
 int MgmtEpgUrlRead(char * buf, int len);
 int MgmtEpgUrlWrite(char * buf, int len);
int MgmtStbMonitorUserRead(char * buf ,int len);
int MgmtStbMonitorUserWrite(char * buf,int len);
int MgmtStbMonitorPwdRead(char * buf,int len);
int MgmtStbMonitorPwdWrite(char * buf,int len);
int MgmtHwopCAregisterRead(char * buf, int len);
int MgmtHwopCAregisterWrite(char * buf, int len);


// 0：开始；-1：停止；1：暂停；2：上传中。
int getMgmtDebugInfoStatus();
void setMgmtDebugInfoStatus(int value);

int getMgmtStartupCaptured(void);
void setMgmtStartupCaptured(int value);

void getMgmtStartupUploadAddr(char *value, int size);
void setMgmtStartupUploadAddr(char *value);

void getMgmtParasListMain(char *value, int size);
void getMgmtParasListPip(char *value, int size);
void getMgmtStream1ParaList(char *value, int size);
void getMgmtStream2ParaList(char *value, int size);

//void getMgmtLogServer(char *value, int size);
//void setMgmtLogServer(char *value);
//void getMgmtLogFtpServer(char *value, int size);
//void setMgmtLogFtpServer(char *value);


#ifdef __cplusplus
}
#endif

#endif
