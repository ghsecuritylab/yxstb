#ifndef _MonitorUpgrade_H_
#define _MonitorUpgrade_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void monitorUpgradeReceiveError();

void monitorUpgradeReset();
void monitorUpgradeSetLength(int length);
int monitorUpgradeGetLength();
void monitorUpgradeSetFlag(int flag);
int monitorUpgradeGetFlag();
int monitorUpgradeDownloadStart();
int monitorUpgradeGetBuffer(unsigned char** buffer, int* length);
int monitorUpgradeSubmitBuffer(unsigned char* buffer, int length);
int monitorUpgradeBurnStart();
int monitorUpgradeGetProgress();
int monitorUpgradeSetProgress(int precent);
int monitorUpgradeGetState();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _MonitorUpgrade_H_
