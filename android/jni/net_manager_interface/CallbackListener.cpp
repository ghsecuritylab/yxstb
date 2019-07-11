
#include <binder/Parcel.h>
#include <utils/Log.h>

#include "CallbackListener.h"

namespace android {

	
    CallbackListener::CallbackListener() {
        LOGI("CallbackListener::CallbackListener");	
			m_fnReadCallback = NULL;
			m_fnWriteCallback = NULL;
			m_fnNotifyCallback = NULL;
			m_fnLogExpCallback = NULL;
			get_value_callback = NULL;
			set_value_callback = NULL;
			get_capability_callback = NULL;   
    }

    CallbackListener::~CallbackListener() {
        LOGI("CallbackListener::~CallbackListener");	
    }

status_t CallbackListener::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags) {
	LOGI("CallbackListener::onTransact()");
	switch (code) {
	case TRANSACTION_ReadCallback: {
		CHECK_INTERFACE(ICallbackListener, data, reply);
		LOGI("read 1\n");
		int type_read = data.readInt32();
		
		LOGI("read 2\n");
		String16 str1_read = data.readString16();
		int str1_read_len = str1_read.size();
		char *c_get_read = (char *)malloc(str1_read_len + 1);
		memset(c_get_read, 0, str1_read_len + 1);
		utf16_to_utf8(str1_read.string(), str1_read_len, c_get_read);
		
		LOGI("read 3\n");
		int len_read = data.readInt32();
		char* result_read = ReadCallback((HMW_MgmtConfig_SRC) type_read, c_get_read, len_read);
		free(c_get_read);
		
		LOGI("read ret 1\n");
		int ret = 0;
		reply->writeInt32(ret);
		
		LOGI("read ret 2\n");
		if (result_read != 	NULL) {
			String16 result1_read16(result_read);
			reply->writeString16(result1_read16);
		}
		
		else {
			String16 result2_read16("");
			reply->writeString16(result2_read16);
		}
		
		
		free(result_read);
		return NO_ERROR;
		break;
	}

	case TRANSACTION_WriteCallback: {
		CHECK_INTERFACE(ICallbackListener, data, reply);
		LOGI("write 1\n");
		int type_write = data.readInt32();
		
		LOGI("write 2\n");
		String16 str1_write = data.readString16();
		int str1_write_len = str1_write.size();
		char *c_str1_write = (char *)malloc(str1_write_len + 1);
		memset(c_str1_write, 0, str1_write_len + 1);
		utf16_to_utf8(str1_write.string(), str1_write_len, c_str1_write);
		
		LOGI("write 3\n");
		String16 str2_write = data.readString16();
		int str2_write_len = str2_write.size();
		char *c_str2_write = (char *)malloc(str2_write_len + 1);
		memset(c_str2_write, 0, str2_write_len + 1);
		utf16_to_utf8(str2_write.string(), str2_write_len, c_str2_write);
		
	    LOGI("write 4\n");
		int len_write = data.readInt32();
		int result_write = WriteCallback((HMW_MgmtConfig_SRC) type_write, c_str1_write, c_str2_write, len_write);
		free(c_str1_write);
		free(c_str2_write);
		
		LOGI("write ret 1");
		reply->writeInt32(result_write);
		return NO_ERROR;
		break;
	}

	case TRANSACTION_NotifyCallback: {
		CHECK_INTERFACE(ICallbackListener, data, reply);
		LOGI("notify 1\n");
		int type_notify = data.readInt32();
		
		LOGI("notify 2\n");
		String16 argc_notify = data.readString16();
		int argc_notify_len = argc_notify.size();
		char *c_argc_notify = (char *)malloc(argc_notify_len + 1);
		memset(c_argc_notify, 0, argc_notify_len + 1);
		utf16_to_utf8(argc_notify.string(), argc_notify_len, c_argc_notify);
		
		LOGI("notify 3\n");
		String16 argv_notify = data.readString16();
		int argv_notify_len = argv_notify.size();
		char *c_argv_notify = (char *)malloc(argv_notify_len);
		memset(c_argv_notify, 0, argv_notify_len + 1);
		utf16_to_utf8(argv_notify.string(), argv_notify_len, c_argv_notify);
		
		LOGI("notify 4\n");
		LOGI("callback %d %s %s", type_notify, c_argc_notify, c_argv_notify);
		char *cstrResult = NotifyCallback((HMW_MgmtMsgType)type_notify, (const char *)c_argc_notify, (const char *)c_argv_notify);
        String16 strResult(cstrResult);
        free(cstrResult);
        LOGI("strlen = %d\n", strResult.size()); 
        reply->writeString16(strResult);

        return NO_ERROR;
		break;
	}

        //tr069
    case TRANSACTION_GetValue: {
		CHECK_INTERFACE(ICallbackListener, data, reply);
		LOGI("TRANSACTION_GetValue start\n");
		
		String16 str1_read = data.readString16();
		char c_get_read[1024] = {0};
		utf16_to_utf8(str1_read.string(), str1_read.size(), c_get_read);
		LOGI("getvalue get ...%s \n", c_get_read);
		
		char* result_read = GetValue(c_get_read);
		LOGI("getvalue ret....%s \n", result_read);
		String16 result_read16(result_read);
		int getValueLength = result_read16.size();
		
		//send length
		reply->writeInt32(getValueLength);
		//send value
	        reply->writeString16(result_read16);
		free(result_read);
		return NO_ERROR;
		break;
	
    }
        
	case TRANSACTION_SetValue: {
			
		CHECK_INTERFACE(ICallbackListener, data, reply);
		char c_set_read1[1024] = {0};
		char c_set_read2[1024] = {0};

		String16 str1_read = data.readString16();
		utf16_to_utf8(str1_read.string(), str1_read.size(), c_set_read1);
		LOGI("SetValue.....%s\n", c_set_read1);

		String16 str2_read = data.readString16();
		utf16_to_utf8(str2_read.string(), str2_read.size(), c_set_read2);
		LOGI("SetValue.....%s\n", c_set_read2);

		int len_read = (data.readInt32());
		SetValue(c_set_read1, c_set_read2, (unsigned int)len_read);

		int ret_value = 0;
		reply->writeInt32(ret_value);
		return NO_ERROR;
		break;
	 
	}

	case TRANSACTION_GetCapability: {
	    CHECK_INTERFACE(ICallbackListener, data, reply);
	    String16 strCapability = GetCapability();
		int strLength = strCapability.size();
		reply->writeInt32(strLength);
	    reply->writeString16(strCapability);
	    return NO_ERROR;
		break;
	}

	default:
	return BBinder::onTransact(code, data, reply, flags);
	}

        return 0;
	}


	char* CallbackListener::ReadCallback(HMW_MgmtConfig_SRC eSrcType, const char * szParm, int iLen)
	{
	    
		if (m_fnReadCallback == NULL) {
            LOGI("read callback NULL!\n");
			return NULL;
		}
		
		char *pBuf = (char *)malloc(512 * 1024);
		memset(pBuf, 0, 512 * 1024);
		
		LOGI("read para = %s \n", szParm);
		if (m_fnReadCallback)
			m_fnReadCallback(eSrcType, szParm, pBuf, iLen);
		
		LOGI("read info = %s \n", pBuf);
		
	    return pBuf;
	}
		

	int CallbackListener::WriteCallback(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen)
	{
		
		if (m_fnWriteCallback == NULL) {
            LOGI("write callback NULL!\n");
			return -1;
		}

		LOGI("write para = %s \n", szParm);
		LOGI("write info = %s \n", pBuf);
		if (m_fnWriteCallback)
			m_fnWriteCallback(eSrcType, szParm, pBuf, iLen);
		
		return 0;
    }
		

	char* CallbackListener::NotifyCallback(HMW_MgmtMsgType eMsgType, const char* szParm1, const char* szParm2)
	{
	    
	    if (m_fnNotifyCallback == NULL) {
			LOGI("notify call back NULL\n");
		    return NULL;
	    }
		return m_fnNotifyCallback(eMsgType, szParm1, szParm2);
    }	
	
    int CallbackListener::LogExpCallback(const char* pszFile, int lLine, const char* pszFunc, 
			                               int lThreadId, int enLogType, int enLogLevel, 
			                               const char* pszModule, const char* format, ...)
    {
        return 0;
    }  
	
	

int CallbackListener::SetReadCallback(fnMgmtIoCallback readcallback)
{
    LOGI("SetReadCallback\n");
	m_fnReadCallback = readcallback;
	return 0;
}


int CallbackListener::SetWriteCallback(fnMgmtIoCallback writecallback)
{
    LOGI("SetWriteCallback\n");
	m_fnWriteCallback = writecallback;
	return 0;
}



int CallbackListener::SetNotifyCallback(fnMgmtNotifyCallback notifycallback)
{
	LOGI("SetNotifyCallback\n");
	m_fnNotifyCallback = notifycallback;
	return 0;
}

int CallbackListener::SetLogExpCallback(fnMgmtLogExportCallback logexpcallback)
{
	m_fnLogExpCallback = logexpcallback;
	return 0;
}

int CallbackListener::SetGetValueCallback(get_value_func getcallback)
{
	LOGI("SetGetValueCallback\n");
	get_value_callback = getcallback;
	return 0;
}

int CallbackListener::SetSetValueCallback(set_value_func setcallback)
{
	LOGI("SetSetValueCallback\n");
	set_value_callback = setcallback;
	return 0;
}

int CallbackListener::SetCapabilityCallback(get_capability_func capabilitycallback)
{
	LOGI("SetCapabilityCallback\n");
	get_capability_callback = capabilitycallback;
	return 0;
}

char* CallbackListener::GetValue(char *name)
{
	if(get_value_callback == NULL){
		return NULL;
	}
	char *tempBuf = (char *)malloc(1024 * 10);
	unsigned int length = 10 * 1024;
	memset(tempBuf, 0, 1024 * 10);
	get_value_callback(name, tempBuf, length);
	return tempBuf;
}


void CallbackListener::SetValue(char *name, char *str, unsigned int pval)
{
	if(set_value_callback == NULL){
		return ;
	}
	set_value_callback(name, str, pval);
}


String16 CallbackListener::GetCapability()
{
	LOGI("CallbackListener::GetCapability()\n");
	if(get_capability_callback == NULL){
	LOGI("get_capability_callback is null\n");
	        String16 result("");
	        return result;		
	}
	char *pBuf = (char *)malloc(512 * 1024);
	memset(pBuf, 0, 512 * 1024);
	LOGI("get_capability_callback in\n");
	get_capability_callback(pBuf);
	LOGI("get_capability_callback out \n");
	String16 capability(pBuf);
	free(pBuf);
	return capability;
}
}
