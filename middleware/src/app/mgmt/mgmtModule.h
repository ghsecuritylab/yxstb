#ifndef _MGMTMODULE_H_
#define _MGMTMODULE_H_
#include "hmw_mgmtlib.h"

#ifdef __cplusplus
extern "C" {
#endif
int mgmtReadCallBack(HMW_MgmtConfig_SRC eSrcType, const char * szParm, char * pBuf, int iLen);
int mgmtWriteCallback(HMW_MgmtConfig_SRC eSrcType, const char * szParm, char * pBuf, int iLen);
int mgmtNotifyCallback(HMW_MgmtMsgType eMsgType, unsigned int argc, void *argv[]);
void mgmtLogExportCallback(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...);
void mgmtLogOutNone(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...);
void mgmt_init();
void iptvSendMsgToMonitor(char *name, char *value, int len);
void* mgmtBeginThread(void *param);
int getMonitorUDiskStatus();

#ifndef ANDROID
int getMonitorState();
int setMonitorState(int);
#endif

#ifdef __cplusplus
}
#endif

#endif
