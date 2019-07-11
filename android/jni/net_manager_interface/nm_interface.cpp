#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include <utils/RefBase.h>
#include "IHybroadService.h"
#include "CallbackListener.h"
#include "nm_interface.h"
#include "tr069_head.h"
#include <utils/Log.h>


extern "C" {

static android::sp<android::ICallbackListener> sCallbackListener;
static android::sp<android::IHybroadService> sHybroadService = NULL;
static nm_io_callback 		g_MgmtReadCallback = NULL;
static nm_io_callback 		g_MgmtWriteCallback = NULL;
static nm_notify_callback 	g_MgmtNotifyCallback = NULL;
static nm_log_export_callback g_MgmtLogExpCallback = NULL;
static get_value_func g_get_value_callback = NULL;
static set_value_func g_set_value_callback = NULL;
static get_capability_func g_capability_callback = NULL;

static int read_callback(HMW_MgmtConfig_SRC eSrcType, const char * szParm, char * pBuf, int iLen)
{
	if(g_MgmtReadCallback)
		return g_MgmtReadCallback(szParm, pBuf, iLen);
	else {
		return -1;
	}
}

static int write_callback(HMW_MgmtConfig_SRC eSrcType, const char * szParm, char * pBuf, int iLen)
{
	if(g_MgmtWriteCallback)
		return g_MgmtWriteCallback(szParm, pBuf, iLen);
	else {
		return -1;
	}
	
}

static char* notify_callback(HMW_MgmtMsgType eMsgType, const char* szParm1, const char* szParm2)
{
	if(g_MgmtNotifyCallback)
		return g_MgmtNotifyCallback((int)eMsgType, szParm1, szParm2);
	else 
		return NULL;

}

/*
static void log_callback(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...)
{
	if(g_MgmtLogExpCallback){
		return g_MgmtLogExpCallback(pszFile, lLine, pszFunc, lThreadId, enLogType, enLogLevel, pszModule, format);
	} else {
		return -1;
	}
}

typedef  void (*proxy_notify)(char *module_name, char *param_name, int param_value); 

static void notify_func(char *module_name, char *param_name, int param_value)
{
	sHybroadService->notify(module_name, param_name, param_value);
	return;
}

proxy_notify proxy_notify_get()
{
	return notify_func;
}

*/

static int proxy_get_value(char *name, char *str, unsigned int size)
{
	char *buf = NULL;

	if(sHybroadService == NULL){
		LOGI("proxy_get_value, not ready skip:%s\n", name);
		return -1;
	}
	buf = sHybroadService->nm_get_value(name);
	if(buf) {
		strncpy(str, buf, size);
		free(buf);
		return 0;
	}
	return -1;
}

proxy_get_value_func proxy_get_value_call_get()
{
	return proxy_get_value;
}

static void proxy_set_value(char *name, char *str, unsigned int val)
{
	if(sHybroadService == NULL){
		LOGI("proxy_set_value, not ready skip:%s\n", name);
		return ;
	}
	sHybroadService->nm_set_value(name, str, val);
	return;
}

proxy_set_value_func proxy_set_value_call_get()
{
	return proxy_set_value;
}

static void sendMsgToMonitor(char *name, char *str, unsigned int val)
{
    if(sHybroadService == NULL){
		LOGI("iptvSetValueToMonitor, not ready skip:%s\n", name);
		return ;
	}
	sHybroadService->sendToMonitor(name, str, val);
	return;
}

proxy_set_value_func proxySendToMonitor()
{
    return sendMsgToMonitor;
}


static void nm_service_client_init()
{
	LOGI("HybroadService cliet %d start!",getpid());

	android::sp<android::IServiceManager> sm = android::defaultServiceManager();
	android::sp<android::IBinder> b;
	b = sm->getService(android::String16("com.hybroad.stb"));
	if (b == 0) {
		LOGI("get service error!\n");
		return ;
	}
	sHybroadService= android::interface_cast<android::IHybroadService>(b);//get Bp
	sCallbackListener = new android::CallbackListener();
	android::ProcessState::self()->startThreadPool();
	sHybroadService->setListener(sCallbackListener);
	android::IPCThreadState::self()->flushCommands();
}

int nm_read_callback_set(nm_io_callback pfnCallback)
{
	LOGI("set read callback \n");
	g_MgmtReadCallback = pfnCallback;
	return 0;
}

int nm_write_callback_set(nm_io_callback pfnCallback)
{
	LOGI("set write callback \n");
	g_MgmtWriteCallback = pfnCallback;
	return 0;
}


int nm_notify_callback_set(nm_notify_callback pfnCallback)
{
	LOGI("set notify callback \n");
	g_MgmtNotifyCallback = pfnCallback;
	return 0;

}

int nm_log_callback_set(nm_log_export_callback pfnCallback)
{
	LOGI("set log callback \n");
	g_MgmtLogExpCallback = pfnCallback;
	return 0;	
}

int nm_getvalue_callback_set(get_value_func pfnCallback)
{
	LOGI("set write callback \n");
	g_get_value_callback = pfnCallback;
	return 0;
}


int nm_setvalue_callback_set(set_value_func pfnCallback)
{
	LOGI("set notify callback \n");
	g_set_value_callback = pfnCallback;
	return 0;

}

int nm_capability_callback_set(get_capability_func pfnCallback)
{
	LOGI("set capability callback \n");
	g_capability_callback = pfnCallback;
	return 0;	
}


int nm_interface_init()
{
	LOGI("mgmtToolsInit\n");
	//stb call this func, connect to manager
	nm_service_client_init();

	if (sCallbackListener != NULL) {
		LOGI("set call back \n");
	    sCallbackListener->SetReadCallback(read_callback);
	    sCallbackListener->SetWriteCallback(write_callback);
	    sCallbackListener->SetNotifyCallback(notify_callback);
	    sCallbackListener->SetLogExpCallback((fnMgmtLogExportCallback)g_MgmtLogExpCallback);
	    sCallbackListener->SetGetValueCallback(g_get_value_callback);
	    sCallbackListener->SetSetValueCallback(g_set_value_callback);
	    sCallbackListener->SetCapabilityCallback(g_capability_callback);
	}
	return 0;
}
}
