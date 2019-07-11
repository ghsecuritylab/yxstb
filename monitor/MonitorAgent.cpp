#include <unistd.h>
#include <string.h>

#include "MonitorAgent.h"
#include "MonitorDef.h"
#include "MonitorTool.h"
#include "MonitorManager.h"
extern "C"{
#include <stdlib.h>
}

extern int  MonitorConnectFlag;
extern STBDebugInfoType g_debugInfoType;
void monitorSetLogOutPutType(int value);
void monitorSetLogType(int type);
void monitorSetLogLevel(int level);
void monitorSetUDPLogAddress(char *address);
void monitorSetFTPLogAddress(char *address);
void MonitorLog();

static MonitorAgent *g_monitorAgent = NULL;

MonitorAgent::MonitorAgent()
{
	int ret = 0;
	m_fnReadCallback = NULL;
	m_fnWriteCallback = NULL;
	m_fnNotifyCallback = NULL;
	m_fnLogExpCallback = NULL;


	//notify 回调函数使用时要加锁
	ret = pthread_mutex_init(&m_mutex, NULL);
	if (ret != 0)
		printf("mutex init error !\n");

	//read,write回调函数使用时要加锁
	ret = pthread_rwlock_init(&m_rwlock, NULL);
	if (ret != 0)
		printf("rwlock init error !\n");
}

MonitorAgent::~MonitorAgent()
{
	pthread_mutex_destroy(&m_mutex);
	pthread_rwlock_destroy(&m_rwlock);
}

/*
*获取唯一实例
*/
MonitorAgent * MonitorAgent::GetInstance()
{
	if (!g_monitorAgent)
        g_monitorAgent = new MonitorAgent();
	return g_monitorAgent;
}

/*
*设置读回调函数
*/
int MonitorAgent::SetReadCallback(fnMgmtIoCallback readcallback)
{
	m_fnReadCallback = readcallback;
	return 0;
}


/*
*设置写回调函数
*/
int MonitorAgent::SetWriteCallback(fnMgmtIoCallback writecallback)
{
	m_fnWriteCallback = writecallback;
	return 0;
}


/*
*设置通知回调函数
*/
int MonitorAgent::SetNotifyCallback(fnMgmtNotifyCallback notifycallback)
{
	m_fnNotifyCallback = notifycallback;
	return 0;
}


/*
*设置日志文件注册函数
*/
int MonitorAgent::SetLogExpCallback(fnMgmtLogExportCallback logexpcallback)
{
	m_fnLogExpCallback = logexpcallback;
	return 0;
}


/*
*调用底层的read注册接口读取机顶盒参数
*
*/
int MonitorAgent::ReadAgent(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen)
{
	int ret = 0;

	pthread_rwlock_rdlock(&m_rwlock);

	STB_MONITOR_LOG("read param is %s\n", szParm);
	if (m_fnReadCallback != NULL)
		ret = m_fnReadCallback(eSrcType, szParm, pBuf, iLen);

	STB_MONITOR_LOG("value is %s\n", pBuf);

	pthread_rwlock_unlock(&m_rwlock);

	return ret;
}


/*
*调用底层的write注册接口将参数写入机顶盒
*
*/
int MonitorAgent::WriteAgent(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen)
{
	int ret = 0;

	pthread_rwlock_wrlock(&m_rwlock);

	if (m_fnWriteCallback != NULL)
		ret = m_fnWriteCallback(eSrcType, szParm, pBuf, iLen);

	pthread_rwlock_unlock(&m_rwlock);

	return ret;
}


/*
*调用底层的notify注册接口将消息
*传到机顶盒并执行命令或传递(传出)信息
*/
int MonitorAgent::NotifyAgent(HMW_MgmtMsgType eMsgType, unsigned int argc, void * argv[])
{
	int ret = 0;


	pthread_mutex_lock(&m_mutex);
	printf("MonitorAgent::NotifyAgent() start\n");
	printf("%d %d===== \n", eMsgType, argc);
	if (m_fnNotifyCallback != NULL)
		ret = m_fnNotifyCallback(eMsgType, argc, argv);

	printf("MonitorAgent::NotifyAgent() end\n");
	pthread_mutex_unlock(&m_mutex);

	return ret;
}

void MonitorAgent::LogExpAgent(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...)
{
		if (m_fnLogExpCallback != NULL)
			m_fnLogExpCallback(pszFile, lLine, pszFunc, lThreadId, enLogType, enLogLevel, pszModule, format);
}

#ifdef ANDROID
void sendMsgToMonitor(char *name, char *str, unsigned int str_len)
{
    printf("sendMsgToMonitor invoke...%s, %s, %d\n", name, str, str_len);
    if (!name || !str)
        return;

    if ((strcmp(name, "DebugInfo") == 0) && (strcmp(str, "start") == 0)){
        if (DEBUG_INFO_NONE != g_debugInfoType)
            return;

        printf("in dfx\n");
        g_debugInfoType = DEBUG_INFO_DFX;
        monitorStartDebugInfo(NULL);

    } else if ((strcmp(name, "DebugInfo") == 0) && (strcmp(str, "stop") == 0)){
        if (DEBUG_INFO_DFX == g_debugInfoType) {
            printf("stop dfx\n");
            monitorStopDebugInfo(NULL);
            g_debugInfoType = DEBUG_INFO_NONE;
        }

    } else if ((strcmp(name, "StartupCaptured") == 0) && (strcmp(str, "2") == 0)){
        printf("StartupCaptured 2\n");
        MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"StartupCaptured", (char *)"2", 2);

    } else if ((strcmp(name, "monitor") == 0) && (strcmp(str, "on") == 0)){
        printf("monitor on\n");
        MonitorConnectFlag = 1;

    } else if((strcmp(name, "monitor") == 0) && (strcmp(str, "off") == 0)){
        printf("monitor off\n");
        MonitorConnectFlag = 0;

    } else if((strcmp(name, "TriggerConnect") == 0) && (strlen(str) != 0)){
        printf("TriggerConnect %s\n", str);
        MonitorManager::GetInstance()->trriggerConnect(str);

    } else if((strcmp(name, "LogOutPutType") == 0) && (strlen(str) != 0)){
        printf("iptv send LogOutPutType %s\n", str);
        monitorSetLogOutPutType(atoi(str));
        MonitorLog();

    } else if((strcmp(name, "LogType") == 0) && (strlen(str) != 0)){
        printf("iptv send LogType %s\n", str);
        monitorSetLogType(atoi(str));
        MonitorLog();

    } else if((strcmp(name, "LogLevel") == 0) && (strlen(str) != 0)){
        printf("iptv send LogLevel %s\n", str);
        monitorSetLogLevel(atoi(str));
        MonitorLog();

    } else if((strcmp(name, "SetLogServer") == 0) && (strlen(str) != 0)){
        printf("iptv send SetLogServer %s\n", str);
        monitorSetUDPLogAddress(str);
        MonitorLog();

    } else if((strcmp(name, "SetLogFtpServer") == 0) && (strlen(str) != 0)){
        printf("iptv send SetLogFtpServer %s\n", str);
        monitorSetFTPLogAddress(str);
        MonitorLog();
    }
}
#else // #ifdef ANDROID
int sendMsgToMonitorLinux(char *name, char *str, unsigned int str_len)
{
    printf("sendMsgToMonitor invoke...%s, %s, %d\n", name, str, str_len);
    if (!name || !str)
        return -1;

    if ((strcmp(name, "DebugInfo") == 0) && (strcmp(str, "start") == 0)){
        if (DEBUG_INFO_NONE != g_debugInfoType)
            return -1;

        printf("in dfx\n");
        g_debugInfoType = DEBUG_INFO_DFX;
        return monitorStartDebugInfo(NULL);

    } else if ((strcmp(name, "DebugInfo") == 0) && (strcmp(str, "stop") == 0)){
        if (DEBUG_INFO_DFX == g_debugInfoType) {
            printf("stop dfx\n");
            monitorStopDebugInfo(NULL);
            g_debugInfoType = DEBUG_INFO_NONE;
        }


    } else if ((strcmp(name, "StartupCaptured") == 0) && (strcmp(str, "2") == 0)){
        printf("StartupCaptured 2\n");
        MonitorAgent::GetInstance()->WriteAgent(MGMT_CONFIG_MT, (const char *)"StartupCaptured", (char *)"2", 2);

    } else if ((strcmp(name, "monitor") == 0) && (strcmp(str, "on") == 0)){
        printf("monitor on\n");
        MonitorConnectFlag = 1;

    } else if((strcmp(name, "monitor") == 0) && (strcmp(str, "off") == 0)){
        printf("monitor off\n");
        MonitorConnectFlag = 0;

    } else if((strcmp(name, "TriggerConnect") == 0) && (strlen(str) != 0)){
        printf("TriggerConnect %s\n", str);
        MonitorManager::GetInstance()->trriggerConnect(str);

    }
    return 0;

}
#endif //#ifdef ANDROID

