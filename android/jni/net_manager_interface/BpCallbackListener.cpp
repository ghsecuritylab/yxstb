#include <binder/Parcel.h>
#include "BpCallbackListener.h"
#include <utils/Log.h>

namespace android {
	
	BpCallbackListener::BpCallbackListener(const sp<IBinder>& impl)
	    : BpInterface<ICallbackListener>(impl) {
	    LOGI("BpCallbackListener::BpCallbackListener");
	}
    

	char* BpCallbackListener::ReadCallback(HMW_MgmtConfig_SRC eSrcType, const char * szParm, int iLen) {
		LOGI("BpCallbackListener::ReadCallback start");
		Parcel data, reply;

		data.writeInterfaceToken(ICallbackListener::getInterfaceDescriptor());
		data.writeInt32(eSrcType);
		String16 szParm16(szParm);
		data.writeString16(szParm16);
		data.writeInt32(iLen);
		remote()->transact(TRANSACTION_ReadCallback, data, &reply);

		int ret = reply.readInt32();
		LOGI("BpCallbackListener::ReadCallback ret=%d", ret);
		char *dst = (char *)malloc(sizeof(char) * 512 * 1024);
		String16 result = reply.readString16();
		int length = result.size();
		utf16_to_utf8(result.string(), length, dst);
		LOGI("BpCallbackListener::ReadCallback dst=%s", dst);
		return dst;
	}

	int BpCallbackListener::WriteCallback(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen) {
		LOGI("BpCallbackListener::WriteCallback start");
		Parcel data, reply;

		data.writeInterfaceToken(ICallbackListener::getInterfaceDescriptor());
		data.writeInt32(eSrcType);
		String16 szParm16(szParm);
		data.writeString16(szParm16);
		String16 pBuf16(pBuf);
		data.writeString16(pBuf16);
		data.writeInt32(iLen);
		remote()->transact(TRANSACTION_WriteCallback, data, &reply);

		reply.readInt32();
		int result = reply.readInt32();
		LOGI("BpCallbackListener::WriteCallback end result=%d", result);
		return result;
	}

	char* BpCallbackListener::NotifyCallback(HMW_MgmtMsgType eMsgType, const char* szParm1, const char* szParm2) {
		LOGI("BpCallbackListener::NotifyCallback start");
		Parcel data, reply;

		data.writeInterfaceToken(ICallbackListener::getInterfaceDescriptor());
		LOGI("BpCallbackListener::NotifyCallback eMsgType=%d", eMsgType);
		data.writeInt32(eMsgType);
		String16 szParm116(szParm1);
		data.writeString16(szParm116);
		String16 szParm216(szParm2);
		data.writeString16(szParm216);
		remote()->transact(TRANSACTION_NotifyCallback, data, &reply);

		String16 strResult = reply.readString16();
		
        char *cstrResult = (char *)malloc(strResult.size() + 1);
        memset(cstrResult, 0, strResult.size() + 1);
        utf16_to_utf8(strResult.string(), strResult.size(), cstrResult);
        LOGI("BpCallbackListener::NotifyCallback end result=%s", cstrResult);
		return cstrResult;
	}

	int BpCallbackListener::LogExpCallback(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...) {
		return 0;
	}


	int BpCallbackListener::SetReadCallback (fnMgmtIoCallback readcallback)
	{
	    return 0;
	}
	int BpCallbackListener::SetWriteCallback(fnMgmtIoCallback writecallback)
	{
	    return 0;
	}
	int BpCallbackListener::SetNotifyCallback(fnMgmtNotifyCallback notifycallback)
	{
	    return 0;
	}
	int BpCallbackListener::SetLogExpCallback(fnMgmtLogExportCallback logexpcallback)
	{
	    return 0;
	}

	int BpCallbackListener::SetGetValueCallback(get_value_func getcallback)
	{
		return 0;
	}
	int BpCallbackListener::SetSetValueCallback(set_value_func setcallback)
	{
		return 0;
	}
	int BpCallbackListener::SetCapabilityCallback(get_capability_func capabilitycallback)
	{
		return 0;
	}

	char* BpCallbackListener::GetValue(char *name)
	{
	    LOGI("BpCallbackListener::GetValue start");
		Parcel data, reply;

		data.writeInterfaceToken(ICallbackListener::getInterfaceDescriptor());
		//data.writeInt32(eSrcType);
		data.writeCString(name);
		
		remote()->transact(TRANSACTION_GetValue, data, &reply);

		int ret = reply.readInt32();
		LOGI("BpCallbackListener::GetValue ret=%d", ret);
		char *dst = (char *)malloc(sizeof(char) * 512 * 1024);
		String16 result = reply.readString16();
		int length = result.size();
		utf16_to_utf8(result.string(), length, dst);
		LOGI("BpCallbackListener::GetValue dst=%s", dst);
		return dst;

	}


	void BpCallbackListener::SetValue(char *name, char *str, unsigned int pval)
	{
		LOGI("BpCallbackListener::SetValue start");
		Parcel data, reply;
		if (str == NULL)
		    LOGI("setvalue str error\n");
        LOGI("bp %s %s\n", name, str);
		data.writeInterfaceToken(ICallbackListener::getInterfaceDescriptor());
		//data.writeInt32(eSrcType);
		if (name == NULL)
		    data.writeCString("");
		else
		    data.writeCString(name);
		if (str == NULL)
		    data.writeCString("");
		else
		    data.writeCString(str);
			
		data.writeInt32(pval);
		
		remote()->transact(TRANSACTION_SetValue, data, &reply);
		int ret = reply.readInt32();
		
		LOGI("BpCallbackListener::SetValue ret=%d", ret);
		
	}

	String16 BpCallbackListener::GetCapability()
	{
		LOGI("BpCallbackListener::GetCapability start\n");
		Parcel data, reply;
	    LOGI("aaaaaaaa\n");
		data.writeInterfaceToken(ICallbackListener::getInterfaceDescriptor());
		LOGI("bbbbbb\n");
		remote()->transact(TRANSACTION_GetCapability, data, &reply);
		LOGI("cccccc\n");
		String16 result = reply.readString16();
		LOGI("BpCallbackListener::GetCapability end\n");
		return result;	    
	}
}

