
#ifndef ANDROID_HYBROADSERVICE_H
#define ANDROID_HYBROADSERVICE_H

#include "IHybroadService.h"

#include <vector>
extern "C"{
	#include <pthread.h>
};


namespace android {

class nm_client_node{
	public:
		nm_client_node();
		virtual ~nm_client_node();
		sp<ICallbackListener> client_listener;
		void *nm_client_param;
		pthread_mutex_t node_mux;	
};

typedef std::vector<nm_client_node*> nm_client_vector_t;

class HybroadService : public BnHybroadService{
public:
	HybroadService();
	virtual ~HybroadService();

	static void instantiate();
	status_t arpping(const char* str);
	status_t ping(const char* str);
	char * nm_get_value(char *name);
	void nm_set_value(char *name, char *str, unsigned int str_len);	
	void sendToMonitor(char *name, char *value, unsigned int len);
	
	int setListener(const sp<ICallbackListener> &listener);
	virtual status_t onTransact(uint32_t code,const Parcel &data,Parcel *reply,uint32_t flags = 0);

	int Read(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen);
	int Write(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen);
	int Notify(HMW_MgmtMsgType eMsgType, unsigned int argc, void * argv[]);
	int LogExp(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...);
        
	int GetValue(char *name, char *str, int str_len);
	int SetValue(char *name, char *str, int pval);
	int module_is_registered(const char *module_name);
	
	static void* nm_client_process(void *param);

	static int connect_flag;
	pthread_mutex_t nm_client_mux;
private:
	nm_client_vector_t client_vector;
	pthread_mutex_t client_vector_mux;
	pthread_t nm_client_pid;
	sp<ICallbackListener> mListener;
	sp<ICallbackListener> listener_handle_get_byparam(const char *param_name);
	sp<ICallbackListener> listener_handle_get_bymodulename(const char *module_name);
	
};
}
#endif
