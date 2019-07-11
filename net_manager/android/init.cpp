#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include <utils/RefBase.h>
#include "IHybroadService.h"

extern "C" {

android::sp<android::ICallbackListener> sCallbackListener;
fnMgmtIoCallback 		g_MgmtReadCallback;
fnMgmtIoCallback 		g_MgmtWriteCallback;
fnMgmtNotifyCallback 	g_MgmtNotifyCallback;
fnMgmtLogExportCallback g_MgmtLogExpCallback;

void HybroadServiceInit();

void HybroadServiceInit()
{
    LOGI("HybroadService cliet %d start!",getpid());
	
	android::sp<android::IServiceManager> sm = android::defaultServiceManager();
	android::sp<android::IBinder> b;
	android::sp<android::IHybroadService> sHybroadService;
  	b = sm->getService(android::String16("android.app.IHybroadService"));
	if (b == 0) {
	    LOGI("get service error!\n");
		return ;
    }
    sHybroadService= android::interface_cast<android::IHybroadService>(b);//get Bp
    //sCallbackListener = new android::CallbackListener();
    android::ProcessState::self()->startThreadPool();

    //sHybroadService->setListener(sCallbackListener);

    android::IPCThreadState::self()->flushCommands();
    
	//
	
}


/*****************************************************************************
 Prototype    : hmw_mgmtRegReadCallback
 Description  : ×¢²á¶Á²ÎÊý»Øµ÷½Ó¿Ú
 Input Param  : fnMgmtIoCallback pstCallback
                ;
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/25
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
int hmw_mgmtRegReadCallback(fnMgmtIoCallback pfnCallback)
{
    LOGI("set read callback \n");
    g_MgmtReadCallback = pfnCallback;
	return 0;
}


/*****************************************************************************
 Prototype    : hmw_mgmtRegWriteCallback
 Description  : ×¢²áÐ´²ÎÊý»Øµ÷½Ó¿Ú
 Input Param  : fnMgmtIoCallback pstCallback
                ;
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/5/25
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
int hmw_mgmtRegWriteCallback(fnMgmtIoCallback pfnCallback)
{
    LOGI("set write callback \n");
    g_MgmtWriteCallback = pfnCallback;
    return 0;

}



/*****************************************************************************
 Prototype    : fnMgmtNotifyCallback
 Description  : MgmtÄ£¿éÍ¨ÖªÓ¦ÓÃÄ£¿éµÄ»Øµ÷º¯Êý
 Input Param  : HMW_MgmtMsgType eMsgType
                unsigned int argc
                void *argv[]
                ;
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/7/2
    Author       : z00109453
    Modification : Created function

*****************************************************************************/
int hmw_mgmtRegNotifyCallback(fnMgmtNotifyCallback pfnCallback)
{
    LOGI("set notify callback \n");
    g_MgmtNotifyCallback = pfnCallback;
	return 0;

}



/*****************************************************************************
 Prototype    : hmw_mgmtRegLogCallback
 Description  :  ×¢²áÐ´ÈÕÖ¾»Øµ÷½Ó¿Ú
 Output       : None
 Return Value : None
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2013/1/11
    Author       : zkf74589
    Modification : Created function

*****************************************************************************/
int hmw_mgmtRegLogCallback(fnMgmtLogExportCallback  pfnCallback)
{
    LOGI("set log callback \n");
    g_MgmtLogExpCallback = pfnCallback;
	return 0;
	
}

/*****************************************************************************
 Prototype    : hmw_mgmtToolStop
 Description  :  ¹Ø±Õ¹ÜÀí¹¤¾ßstbMonitor_Server¶Ë
 Input Param  : None
 
 Output       : None
 Return Value : int
 Calls        : None
 Called By    : None
 
  History        :
  1.Date         : 2012/08/08
    Author       : skf74590
    Modification : Created function

*****************************************************************************/
int hmw_mgmtToolsStop()
{	
	return 0;
}

extern "C"
{
int hmw_mgmtCliInit()
{
    return 0;
}
int hmw_mgmtCpeInit(char *dataModel)
{
    return 0;
}

int hmw_mgmtValueChange(const char * szParm, char * pBuf, int iLen)
{
    return 0;
}

int hmw_mgmtCliRegCmd(unsigned int argc, void *argv[])
{
    return 0;
}

int  hmw_mgmtAlarmInit()
{
    return 0;
}

struct cli_def {};
void cli_print(struct cli_def *cli, const char *format, ...)
{
}

}
}
