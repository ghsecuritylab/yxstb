#ifndef MonitorAgent_h
#define MonitorAgent_h

#include "MonitorDef.h"
#include "mgmtDef.h"

#include <pthread.h>
#include <stdio.h>

#ifdef ANDROID
void sendMsgToMonitor(char *name, char *str, unsigned int str_len);
#else
int sendMsgToMonitorLinux(char *name, char *str, unsigned int str_len);
#endif

class MonitorAgent {
public:
	~MonitorAgent();
	static MonitorAgent* GetInstance();

	int SetReadCallback	(fnMgmtIoCallback readcallback);
	int SetWriteCallback	(fnMgmtIoCallback writecallback);
	int SetNotifyCallback(fnMgmtNotifyCallback notifycallback);
	int SetLogExpCallback(fnMgmtLogExportCallback logexpcallback);

	//读写通知接口
	int ReadAgent(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen);
	int WriteAgent(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen);
	int NotifyAgent(HMW_MgmtMsgType eMsgType, unsigned int argc, void* argv[]);
	void LogExpAgent(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...);

private:
	MonitorAgent();
	static void* SerThread(void *para);
	fnMgmtIoCallback m_fnReadCallback;
	fnMgmtIoCallback m_fnWriteCallback;
	fnMgmtNotifyCallback m_fnNotifyCallback;
	fnMgmtLogExportCallback m_fnLogExpCallback;
	pthread_mutex_t m_mutex;
	pthread_rwlock_t m_rwlock;
};
#endif//MonitorAgent_h

