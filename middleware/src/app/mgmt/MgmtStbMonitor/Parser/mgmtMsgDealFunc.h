#ifndef _MGMTMSGDEALFUNC_H
#define _MGMTMSGDEALFUNC_H

#ifdef __cplusplus
extern "C" {
#endif
void mgmtDealCpeAcsReqReboot();
void mgmtDealCpeMessageNone();
void mgmtDealCpeRegisterAcsOk();
void mgmtDealCpeConfigDownload(Mgmt_DownloadInfo*);
void mgmtDealCpeConfigFactoryReset();
void mgmtDealCpeConfigUpload(Mgmt_UpLoadInfo*);
void mgmtDealCpePlayDiagnostics(char*, char*);
void mgmtDealCpeCallValueChang();
void mgmtDealCpeLogOutputChannelSet();
void mgmtDealCpeGetPlayState(char*, char*);
void mgmtDealMtPlayerbyChanno(int*, int*);
void mgmtDealMtPlayerbyUrl(char*, int*);
void mgmtDealMtPlayerStop();
void mgmtDealMtPlayerMpctrl(char*);
void mgmtDealMtToolReboot();
void mgmtDealMtEnterDebug();
void mgmtDealMtExitDebug();
void mgmtDealMtGetChannelNumToTal(int*);
void mgmtDealMtGetChannelInfo(int*, char*);
void mgmtDealMtGetCollectFilePath();
//int mgmtDealMtUpgradeLinux(int, long long);
void mgmtDealMtUpgradeGetWorkStat();
void mgmtDealMtUpgradeSetLength(int);
void mgmtDealMtUpgradeSetForce(int);
void mgmtDealMtUpgradeSetUpgrader(char*, int*);
void mgmtDealMtUpgradeGetDownHandle(FILE*);
void mgmtDealMtUpgradeSetCloseWork();
void mgmtDealMtUpgradeSetDownloadPer(int);
void mgmtDealMtUpgradeGetBurnProcess(int*);
void mgmtDealMtUpgradeSetBurnStart();
void mgmtDealNtUpgradeNetworkDisconnect();
int MgmtChannelListRead(char*);

#ifdef __cplusplus
}
#endif

#endif
