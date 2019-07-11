#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include "cutils/properties.h"
#include "hmw_mgmtlib.h"
#include "HybroadService.h"
#include "nm_dbg.h"

using namespace android;

HybroadService *pHybroad;

int GetConnFlag()
{
    return HybroadService::connect_flag;
}


int DoRead(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen)
{
	int ret = 0;
	nm_track();
	printf("DoRead start\n");
    if (pHybroad == NULL)
	    return -1;
    ret = pHybroad->Read(eSrcType, szParm, pBuf, iLen);
	nm_track();
	printf("DoRead stop\n");
	return ret;
}

int DoWrite(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen)
{
	int ret = 0;
	nm_track();

    //ld add for dfx
    char value[8] = { 0 };
    property_get("sys.dfx.source", value, "0");
    if (strcmp(value, "1"))
        property_set("sys.dfx.source", "1");

    if (pHybroad == NULL)
	    return -1;
    ret = pHybroad->Write(eSrcType, szParm, pBuf, iLen);
	nm_track();
	return ret;
}


int DoNotify(HMW_MgmtMsgType eMsgType, unsigned int argc, void * argv[])
{
	int ret = 0;
	nm_track();
    if (pHybroad == NULL)
	    return -1;
    ret = pHybroad->Notify(eMsgType, argc, argv);
	nm_track();
	return ret;
}

int DoLogExp(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...)
{
	int ret = 0;
    if (pHybroad == NULL)
	    return -1;
    ret = pHybroad->LogExp(pszFile, lLine, pszFunc, lThreadId, enLogType, enLogLevel, pszModule, format);
	return ret;
}


//tr069
extern "C"
{

	
	int android_platform_getValue(char *name, char *str, int str_Len)
	{
		//nm_msg_level(LOG_DEBUG, "len is %d\n", str_Len);

		if (pHybroad == NULL){
			nm_msg("pHybroad is NULL\n");
			return -1;
		}
		return pHybroad->GetValue(name, str, str_Len);
		nm_track();
	}

	int android_platform_setValue(char *name, char *str, int pval)
	{
		nm_track();

        //ld add for dfx
        char value[8] = { 0 };
        property_get("sys.dfx.source", value, "0");
        if (strcmp(value, "2"))
            property_set("sys.dfx.source", "2");

		if (pHybroad == NULL){
			nm_msg("pHybroad is NULL\n");
			return -1;
		}
		return pHybroad->SetValue(name, str, pval);
		nm_track();

	}
}

