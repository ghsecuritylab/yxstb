
#ifndef ANDROID_BPCALLBACKLISTENER_H
#define ANDROID_BPCALLBACKLISTENER_H

#include <binder/Parcel.h>
#include "ICallbackListener.h"

namespace android {
	
	class BpCallbackListener: public BpInterface<ICallbackListener> {
		public:
		BpCallbackListener(const sp<IBinder>& impl);

				
		//×¢²á»Øµ÷º¯Êý
		virtual char* GetCapability();
		virtual char* ReadCallback(HMW_MgmtConfig_SRC eSrcType, const char * szParm, int iLen);
		virtual int WriteCallback(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen);
		virtual char* NotifyCallback(HMW_MgmtMsgType eMsgType, const char* szParm1, const char* szParm2);
		virtual int LogExpCallback(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...);
		virtual char* GetValue(char *name);
		virtual void SetValue(char *name, char *str, unsigned int pval);

	};
}
#endif
