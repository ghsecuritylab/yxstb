
#include <binder/Parcel.h>
#include "ICallbackListener.h"
#include "BpCallbackListener.h"

namespace android {

IMPLEMENT_META_INTERFACE(CallbackListener, "com.hybroad.stb.ICallbackListener");

status_t BnCallbackListener::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags) {
	LOGI("CallbackListener::onTransact()");
	switch (code) {
	case TRANSACTION_ReadCallback: {
		int type_read = data.readInt32();
		const char *str1_read = data.readCString();
		int len_read = data.readInt32();
		char* result_read = ReadCallback((HMW_MgmtConfig_SRC) type_read, str1_read, len_read);
		String16 result_read16(result_read);
		reply->writeString16(result_read16);
		return NO_ERROR;
		break;
	}

	case TRANSACTION_WriteCallback: {
		int type_write = data.readInt32();
		const char *str1_write = data.readCString();
		const char *str2_write = data.readCString();
		int len_write = data.readInt32();
		int result_write = WriteCallback((HMW_MgmtConfig_SRC) type_write, str1_write, (char*) str2_write, len_write);
		reply->writeInt32(result_write);
		return NO_ERROR;
		break;
	}

	case TRANSACTION_NotifyCallback: {
		//int type_notify = data.readInt32();
		//const char *str1_write = data.readCString();
		//const char *str2_write = data.readCString();
		//int len_write = data.readInt32();
		//reply->writeInt32(NotifyCallback((HMW_MgmtConfig_SRC)type_notify, str1_write, (char *)str2_write, len_write));
		return NO_ERROR;
		break;
	}
	case TRANSACTION_GetCapability: {
		char* result_read = GetCapability();
		String16 result_read16(result_read);
		reply->writeString16(result_read16);
		return NO_ERROR;
		break;
	}
	

	default:
		return BBinder::onTransact(code, data, reply, flags);
	}

	return 0;
}

}
