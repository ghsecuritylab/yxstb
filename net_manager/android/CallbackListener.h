
#ifndef ANDROID_CALLBACKLISTENER_H
#define ANDROID_CALLBACKLISTENER_H

#include <binder/Parcel.h>

#include "ICallbackListener.h"

namespace android {
	
	class  CallbackListener : public BnCallbackListener {
		public:
			CallbackListener();
			virtual ~CallbackListener();
			//

			status_t onTransact(
			              uint32_t code, 
			              const Parcel &data, 
			              Parcel *reply,
			              uint32_t flags = 0);

			//×¢²á»Øµ÷º¯Êý
			virtual char* ReadCallback(HMW_MgmtConfig_SRC eSrcType, const char * szParm, int iLen);
			virtual int WriteCallback(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen);
			virtual char* NotifyCallback(HMW_MgmtMsgType eMsgType, const char* szParm1, const char* szParm2);
			virtual int LogExpCallback(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...);
							   
			//reg callback func
			int SetReadCallback	(fnMgmtIoCallback readcallback);
			int SetWriteCallback(fnMgmtIoCallback writecallback);
			int SetNotifyCallback(fnMgmtNotifyCallback notifycallback);
			int SetLogExpCallback(fnMgmtLogExportCallback logexpcallback);

			virtual char* GetValue(char *name, unsigned int *pval);
			virtual void SetValue(char *name, char *str, unsigned int pval);
			virtual char *GetCapability();
			
			private:		
			fnMgmtIoCallback 		m_fnReadCallback;
			fnMgmtIoCallback 		m_fnWriteCallback;
			fnMgmtNotifyCallback 	m_fnNotifyCallback;
			fnMgmtLogExportCallback m_fnLogExpCallback;
             
	};
	
	
}
#endif
