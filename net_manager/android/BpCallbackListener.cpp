#include <binder/Parcel.h>
#include "BpCallbackListener.h"
#include <utils/Log.h>
#include "nm_dbg.h"

namespace android {
	
	BpCallbackListener::BpCallbackListener(const sp<IBinder>& impl)
	    : BpInterface<ICallbackListener>(impl) {
	   nm_msg_level(LOG_DEBUG, "BpCallbackListener::BpCallbackListener\n");
	}
    
	char* BpCallbackListener::ReadCallback(HMW_MgmtConfig_SRC eSrcType, const char * szParm, int iLen)
	{
		//nm_msg_level(LOG_DEBUG, "BpCallbackListener::ReadCallback start\n");
		Parcel data, reply;

		data.writeInterfaceToken(ICallbackListener::getInterfaceDescriptor());
		data.writeInt32(eSrcType);
		String16 szParm16(szParm);
		data.writeString16(szParm16);
		data.writeInt32(iLen);
		remote()->transact(TRANSACTION_ReadCallback, data, &reply);

		int ret = reply.readInt32();
		//nm_msg_level(LOG_DEBUG, "BpCallbackListener::ReadCallback ret=%d\n", ret);
		char *dst = (char *)malloc(sizeof(char) * 512 * 1024);
		String16 result = reply.readString16();
		int length = result.size();
		utf16_to_utf8(result.string(), length, dst);
		//nm_msg_level(LOG_DEBUG, "BpCallbackListener::ReadCallback dst=%s\n", dst);
		return dst;
	}

	int BpCallbackListener::WriteCallback(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen) 
	{
		//nm_msg_level(LOG_DEBUG, "BpCallbackListener::WriteCallback start\n");
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
		//nm_msg_level(LOG_DEBUG, "BpCallbackListener::WriteCallback end result=%d\n", result);
		return result;
	}

	char* BpCallbackListener::NotifyCallback(HMW_MgmtMsgType eMsgType, const char* szParm1, const char* szParm2) {
		nm_msg_level(LOG_DEBUG, "BpCallbackListener::NotifyCallback start");
		Parcel data, reply;

		data.writeInterfaceToken(ICallbackListener::getInterfaceDescriptor());
		nm_msg_level(LOG_DEBUG, "BpCallbackListener::NotifyCallback eMsgType=%d\n", eMsgType);
		data.writeInt32(eMsgType);
		String16 szParm116(szParm1);
		data.writeString16(szParm116);
		String16 szParm216(szParm2);
		data.writeString16(szParm216);
		remote()->transact(TRANSACTION_NotifyCallback, data, &reply);
		String16 strResult = reply.readString16();
		char *cstrResult = (char *)malloc(512 * 1024);
		memset(cstrResult, 0, 512 * 1024);
		utf16_to_utf8(strResult.string(), strResult.size(), cstrResult);
		//nm_msg_level(LOG_DEBUG, "BpCallbackListener::NotifyCallback end result=%s", cstrResult);
		return cstrResult;
	}

	int BpCallbackListener::LogExpCallback(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...) {
		return 0;
	}

	char* BpCallbackListener::GetValue(char *name)
	{
		//nm_msg_level(LOG_DEBUG, "BpCallbackListener::GetValue start\n");
		Parcel data, reply;

		data.writeInterfaceToken(ICallbackListener::getInterfaceDescriptor());
		//data.writeInt32(eSrcType);
		String16 szParm16(name);
		data.writeString16(szParm16);
		remote()->transact(TRANSACTION_GetValue, data, &reply);
		
		int ret = reply.readInt32();
		//nm_msg_level(LOG_DEBUG, "BpCallbackListener::GetValue ret=%d\n", ret);

		//nm_msg_level(LOG_DEBUG, "BpCallbackListener::GetValue ret=%d", ret);
		char *dst = (char *)malloc(sizeof(char) * 512 * 1024);
		String16 result = reply.readString16();
		int length = result.size();
		utf16_to_utf8(result.string(), length, dst);
		//nm_msg_level(LOG_DEBUG, "BpCallbackListener::GetValue dst len is %d\n", length);
		return dst;

	}


	void BpCallbackListener::SetValue(char *name, char *str, unsigned int pval)
	{
		//nm_msg_level(LOG_DEBUG, "BpCallbackListener::SetValue start\n");
		Parcel data, reply;

		data.writeInterfaceToken(ICallbackListener::getInterfaceDescriptor());
		String16 szParm16(name);
		data.writeString16(szParm16);
		if(str != NULL) {
			String16 pBuf16(str);
			data.writeString16(pBuf16);
		} else {
			String16 pBuf16("");
			data.writeString16(pBuf16);
		}
		data.writeInt32(pval);
		remote()->transact(TRANSACTION_SetValue, data, &reply);

		reply.readInt32();
		int result = reply.readInt32();
		//nm_msg_level(LOG_DEBUG, "BpCallbackListener::SetValue end result=%d\n", result);
	}

	char* BpCallbackListener::GetCapability()
	{
		nm_msg_level(LOG_DEBUG, "BpCallbackListener::GetCapability start:macro is %d\n", TRANSACTION_GetCapability);
		Parcel data, reply;

		data.writeInterfaceToken(ICallbackListener::getInterfaceDescriptor());
		remote()->transact(TRANSACTION_GetCapability, data, &reply);

		int ret = reply.readInt32();
		nm_msg_level(LOG_DEBUG, "BpCallbackListener::GetCapability ret=%d\n", ret);
		char *dst = (char *)malloc(sizeof(char) * 512 * 1024);
		String16 result = reply.readString16();
		int length = result.size();
		utf16_to_utf8(result.string(), length, dst);
		nm_msg_level(LOG_DEBUG, "BpCallbackListener::GetCapability ret len is %d \n", length);
		return dst;
	}
}

