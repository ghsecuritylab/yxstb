
#include <binder/Parcel.h>
#include <utils/Log.h>

#include "CallbackListener.h"

#define READ_BUFFER (512 * 1024)

//tr069
//extern "C" 

void tr069_port_getValue1(char *name, char *str, unsigned int* pval);


//extern "C"
//void tr069_port_setValue1(char *name, char *str, unsigned int pval);

//namespace android {
typedef void (*getValue)(char *name, char *str, unsigned int* pval);
getValue g_getFunc;

void tr069GetValue(char *name, char *str, unsigned int* pval);

void tr069GetValue(char *name, char *str, unsigned int* pval)
{
	g_getFunc(name, str, pval);

}
void setFunc(getValue funcName)
{
    g_getFunc = funcName;
    
}

void tr069SetValue(char *name, char *str, unsigned int pval);

void tr069SetValue(char *name, char *str, unsigned int pval)
{
}
//}

namespace android {

	
CallbackListener::CallbackListener() {
    LOGI("CallbackListener::CallbackListener");	
}

CallbackListener::~CallbackListener() {
	LOGI("CallbackListener::~CallbackListener");
}

status_t CallbackListener::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags) {
	LOGI("CallbackListener::onTransact()");
	switch (code) {
	case TRANSACTION_ReadCallback: {
		CHECK_INTERFACE(ICallbackListener, data, reply);
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
		CHECK_INTERFACE(ICallbackListener, data, reply);
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
		//CHECK_INTERFACE(ICallbackListener, data, reply);
		//int type_notify = data.readInt32();
		//const char *argc_notify = data.readCString();
		//const char *argv_notify = data.readCString();
		//reply->writeInt32(NotifyCallback((HMW_MgmtConfig_SRC)type_notify, argc_notify, argv_notify));
		return NO_ERROR;
		break;
	}

        //tr069
        case TRANSACTION_GetValue: {
        CHECK_INTERFACE(ICallbackListener, data, reply);
		//int type_read = data.readInt32();
		const char *str1_read = data.readCString();
		int len_read = (data.readInt32());
		char* result_read = GetValue((char *)str1_read, (unsigned int *)&len_read);
		String16 result_read16(result_read);
        reply->writeString16(result_read16);
        reply->writeInt32(len_read);
		free(result_read);
		return NO_ERROR;
		break;
	
        }
        
        case TRANSACTION_SetValue: {
        //	CHECK_INTERFACE(ICallbackListener, data, reply);
	//	int type_write = data.readInt32();
	//	const char *str1_write = data.readCString();
	//	const char *str2_write = data.readCString();
	//	int len_write = data.readInt32();
	//	int result_write = WriteCallback((HMW_MgmtConfig_SRC) type_write, str1_write, (char*) str2_write, len_write);
	//	reply->writeInt32(result_write);
		return NO_ERROR;
		break;
	     
        
        }

        case TRANSACTION_GetCapability: {
		CHECK_INTERFACE(ICallbackListener, data, reply);
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


	//int CallbackListener::ReadCallback(HMW_MgmtConfig_SRC eSrcType, 
	//	                                  const char * szParm, 
	//	                                  char * pBuf, int iLen)
	
	char* CallbackListener::ReadCallback(HMW_MgmtConfig_SRC eSrcType, const char * szParm, int iLen)
	{
	    #if 0
		if (m_fnReadCallback == NULL) {
            LOGI("read callback NULL!\n");
			return -1;
		}
		LOGI("read para = %s \n", szParm);
		m_fnReadCallback(eSrcType, szParm, pBuf, iLen);
		LOGI("read info = %s \n", pBuf);
		#endif
	    return NULL;
	}
		
    //int CallbackListener::WriteCallback(HMW_MgmtConfig_SRC eSrcType, 
	//	                                  const char* szParm, 
	//		                              char* pBuf, int iLen)
	int CallbackListener::WriteCallback(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen)
	{
		#if 0
		if (m_fnWriteCallback == NULL) {
            LOGI("write callback NULL!\n");
			return -1;
		}

		LOGI("write para = %s \n", szParm);
		LOGI("write info = %s \n", pBuf);
	    m_fnWriteCallback(eSrcType, szParm, pBuf, iLen);
		#endif
		return 0;
    }
		
    //int CallbackListener::NotifyCallback(HMW_MgmtMsgType eMsgType, unsigned int argc, void * argv[])
	char* CallbackListener::NotifyCallback(HMW_MgmtMsgType eMsgType, const char* szParm1, const char* szParm2)
	{
	    #if 0
	    if (m_fnNotifyCallback == NULL) {
			LOGI("notify call back NULL\n");
		    return -1;
	    }
		m_fnNotifyCallback(eMsgType, argc, argv);
		#endif
		return NULL;
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
	m_fnNotifyCallback = notifycallback;
	return 0;
}



int CallbackListener::SetLogExpCallback(fnMgmtLogExportCallback logexpcallback)
{
	m_fnLogExpCallback = logexpcallback;
	return 0;
}


char* CallbackListener::GetValue(char *name, unsigned int *pval)
{
    char *tempBuf = (char *)malloc(1024 * 10);
	memset(tempBuf, 0, 1024 * 10);
	tr069GetValue(name, tempBuf, pval);
	return tempBuf;
}


void CallbackListener::SetValue(char *name, char *str, unsigned int pval)
{
}

char *CallbackListener::GetCapability()
{
	printf("GetCapability return NULL\n");
	return NULL;
}


}
