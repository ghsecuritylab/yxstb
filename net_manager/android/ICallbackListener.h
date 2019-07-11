#ifndef ANDROID_ICALLBACKLISTENER_H
#define ANDROID_ICALLBACKLISTENER_H

#include <binder/Parcel.h>
#include <binder/IInterface.h>
#include "hmw_mgmtlib.h"

namespace android {

	enum {
		TRANSACTION_ReadCallback = IBinder::FIRST_CALL_TRANSACTION, 
		TRANSACTION_WriteCallback, 
		TRANSACTION_NotifyCallback,
		TRANSACTION_GetCapability,	
		TRANSACTION_GetValue, 
		TRANSACTION_SetValue
	};

	/*
		TRANSACTION_GetValue, 
		TRANSACTION_SetValue,

	*/

	class ICallbackListener: public IInterface {
	public:
		DECLARE_META_INTERFACE (CallbackListener);

		virtual char *GetCapability() = 0;
		virtual char* ReadCallback(HMW_MgmtConfig_SRC eSrcType, const char * szParm, int iLen) = 0;
		virtual int WriteCallback(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen) = 0;
		virtual char* NotifyCallback(HMW_MgmtMsgType eMsgType, const char* szParm1, const char* szParm2) = 0;
		virtual int LogExpCallback(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...) = 0;
		virtual char* GetValue(char *name) = 0;
		virtual void SetValue(char *name, char *str, unsigned int pval) = 0;

	};
  
	class BnCallbackListener : public BnInterface<ICallbackListener> {
		
	virtual status_t onTransact(
	        uint32_t code, const Parcel &data, Parcel *reply,
	        uint32_t flags = 0);
	};
}
#endif
