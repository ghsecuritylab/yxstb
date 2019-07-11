#include<sys/types.h> 
#include<sys/socket.h> 
#include<string.h> 
#include<stdio.h> 
#include<netinet/in.h> 
#include<stdlib.h> 
#include<errno.h> 
#include<sys/wait.h> 
#include<netdb.h> 
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <stdarg.h>
#include <assert.h>

#include "MessageTypes.h"
#include "KeyDispatcher.h"
#include "ipanel_event.h"
#include "sdk/yxsdk_key.h"

#include "cloud_api.h"
#include "superusbhid_acq.h"


#define CLOUD_URL_LEN 1024

/*0, deal with key by IPTV system; 1, deal with key by cyberCloud. */
static C_BOOL bCloudKeyLoopFlag = 0;

/*URL that return to IPTV system after exit from cyber cloud. */
static C_S8 csCloudBackURL[CLOUD_URL_LEN];

void CStb_CloudKeyLoopFlagSet(C_BOOL flag)
{
	bCloudKeyLoopFlag = flag;
}

C_BOOL CStb_CloudKeyLoopIsEnable(void)
{
	return bCloudKeyLoopFlag;
}

C_BOOL CStb_SendKey2CyberCloud(int status, int keyValue)
{
	C_RC rc_data;
	C_BOOL retVal = C_TRUE;
	C_BOOL isSend2Cloud = C_TRUE;
	
	CLOUD_LOG_TRACE("status = %d keyValue = %#x\n\n", status, keyValue);

	if(status == YX_EVENT_KEYUP){
		rc_data.uButton = 0;
	}else{
		rc_data.uButton = 1;
	}
	
	CLOUD_LOG_TRACE("rc_data.uButton = %d keyValue = %#x\n\n", rc_data.uButton, keyValue);
	
	switch(keyValue){
	case EIS_IRKEY_NUM0:
		rc_data.uKeyValue = CloudKey_0;
		break;
	case EIS_IRKEY_NUM1:
		rc_data.uKeyValue = CloudKey_1;
		break;
	case EIS_IRKEY_NUM2:
		rc_data.uKeyValue = CloudKey_2;
		break;
	case EIS_IRKEY_NUM3:
		rc_data.uKeyValue = CloudKey_3;
		break;
	case EIS_IRKEY_NUM4:
		rc_data.uKeyValue = CloudKey_4;
		break;
	case EIS_IRKEY_NUM5:
		rc_data.uKeyValue = CloudKey_5;
		break;
	case EIS_IRKEY_NUM6:
		rc_data.uKeyValue = CloudKey_6;
		break;
	case EIS_IRKEY_NUM7:
		rc_data.uKeyValue = CloudKey_7;
		break;
	case EIS_IRKEY_NUM8:
		rc_data.uKeyValue = CloudKey_8;
		break;
	case EIS_IRKEY_NUM9:
		rc_data.uKeyValue = CloudKey_9;
		break;
	case EIS_IRKEY_SELECT:
		rc_data.uKeyValue = CloudKey_OK;
		break;
	case EIS_IRKEY_BACK:
		rc_data.uKeyValue = CloudKey_Back;
		break;
	case EIS_IRKEY_UP:
		rc_data.uKeyValue = CloudKey_Up;
		break;
	case EIS_IRKEY_DOWN:
		rc_data.uKeyValue = CloudKey_Down;
		break;
	case EIS_IRKEY_LEFT:
		rc_data.uKeyValue = CloudKey_Left;
		break;
	case EIS_IRKEY_RIGHT:
		rc_data.uKeyValue = CloudKey_Right;
		break;
	case EIS_IRKEY_PAGE_UP:
		rc_data.uKeyValue = CloudKey_PageUp;
		break;
	case EIS_IRKEY_PAGE_DOWN:
		rc_data.uKeyValue = CloudKey_PageDown;
		break;
	case EIS_IRKEY_CHANNEL_UP:
		rc_data.uKeyValue = CloudKey_ChannelUp;
		break;
	case EIS_IRKEY_CHANNEL_DOWN:
		rc_data.uKeyValue = CloudKey_ChannelDown;
		break;
	case EIS_IRKEY_REWIND:
		rc_data.uKeyValue = CloudKey_Backward;
		break;
	case EIS_IRKEY_FASTFORWARD:
		rc_data.uKeyValue = CloudKey_Forward;
		break;
	case EIS_IRKEY_PLAY:
		rc_data.uKeyValue = CloudKey_Play;
		break;
	case EIS_IRKEY_POS:	/* What is it? is it pause key? */
		rc_data.uKeyValue = CloudKey_Pause;
		break;
	case EIS_IRKEY_STOP:
		rc_data.uKeyValue = CloudKey_Stop;
		break;
	/*To be confirm: play/pause key value.*/

	case EIS_IRKEY_MENU:
		rc_data.uKeyValue = CloudKey_Menu;
		break;
	/*To be confirm: Exit key value.*/
	case EIS_IRKEY_VOLUME_UP:
		rc_data.uKeyValue = CloudKey_VolUp;
		break;
	case EIS_IRKEY_VOLUME_DOWN:
		rc_data.uKeyValue = CloudKey_VolDown;
		break;
	case EIS_IRKEY_VOLUME_MUTE:
		rc_data.uKeyValue = CloudKey_Mute;
		break;
	case EIS_IRKEY_POWER:
	case EIS_IRKEY_VK_F10:  /* enter local confiure page. */
	case EIS_IRKEY_RED:	
	case EIS_IRKEY_GREEN:
	case EIS_IRKEY_YELLOW:
	case EIS_IRKEY_BLUE:
	case EIS_IRKEY_INFO:
		/* Exit cyber cloud, then deal by IPTV SYSTEM.*/
		CStb_CyberCloudTaskStop();
		retVal = C_FALSE;
		isSend2Cloud = C_FALSE;
		break;
	default:
		isSend2Cloud = C_FALSE;
		break;
	}
	
	if(isSend2Cloud){
		Cloud_OnKey(KeyType_RC, sizeof(C_RC), (C_U8 *)&rc_data);
	}

	return retVal;
}

void CStb_CyberCloudTaskStop(void)
{
	CLOUD_LOG_TRACE("Cyber cloud cyberCloudTaskStop\n");

	USBHIDACQ_Stop();
	Cloud_Stop();	
	Cloud_Final();
	
	CStb_CloudKeyLoopFlagSet(0);
	CStb_PreviousGraphicsPosSet();
}

static void CStb_CloudExit_callback_func( IN C_U8 cExitFlag, IN char const * szBackURL )
{	
	CLOUD_LOG_TRACE("exitFlag=%d, backURL =%s\n", cExitFlag, (szBackURL != NULL) ? szBackURL:"NONE");

	memset(csCloudBackURL, 0, CLOUD_URL_LEN);
	if(szBackURL){
		strcpy(csCloudBackURL, szBackURL);
	}
	
	switch(cExitFlag){
		case 1:		 //机顶盒进行退出键退出的逻辑处理 	
			sendMessageToKeyDispatcher(MessageType_CyberCloud, CloudKey_Exit, (int)csCloudBackURL, 0);
			break;
		case 2:		 //机顶盒进行菜单键退出的逻辑处理 
			sendMessageToKeyDispatcher(MessageType_CyberCloud, CloudKey_Menu, (int)csCloudBackURL, 0);
			break;
		default:
			break;
	}
}

int CStb_CyberCloudTaskStart(char* cloud_url)
{
	char szVersion[CLOUD_VERSION_LEN + 1] = {0};
	char szVendor[CLOUD_VENDOR_LEN + 1] = {0};

	cybercloud_flash_init();
	
	if(CLOUD_FAILURE == Cloud_Init()){
		CLOUD_LOG_TRACE("Cyber cloud init failure\n");
		return -1;
	}
	Cloud_GetVersion(szVersion, szVendor);
	
	CLOUD_LOG_TRACE("Cyber cloud version:%s, vendor:%s\n", szVersion, szVendor);
	
	Cloud_RegisterExitCallback( CStb_CloudExit_callback_func );
	if(Cloud_Start( cloud_url) == CLOUD_FAILURE){
		CLOUD_LOG_TRACE("Cyber cloud start failure\n");
		return -1;
	}
	
	CStb_CloudKeyLoopFlagSet(1);
	
	CLOUD_LOG_TRACE("Cyber cloud USBHID version:%s\n", USBHIDACQ_Version());

	Device_Callback_Superhid();
	C_BOOL hidRet = USBHIDACQ_Start();

	CLOUD_LOG_TRACE("HID start retVal = %d\n", hidRet);
	return 0;
}


/********************** Cyber Cloud portting function **********/
void CStb_Print( IN char *szFmt, IN ... )
{
    va_list List;

    va_start( List, szFmt );
    vprintf(szFmt, List);
    va_end( List );  
}

C_RESULT CStb_CreateThread (
    OUT C_ThreadHandle *pThreadHandle,
    IN char const*     szName, 
    IN C_U8   	       uPriority,
    IN void 	       (*pThreadFunc)(void*),
    IN void *          pParam,
    IN C_U16 	       uStackSize
)
{
    int ret_code;
    pthread_t *thread;
    pthread_attr_t attr;
    struct sched_param param;
      
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    param.sched_priority = uPriority;
    pthread_attr_setschedparam(&attr, &param);

    thread = (pthread_t*)malloc(sizeof(pthread_t));
    if ( thread != NULL ){
        ret_code = pthread_create(thread, &attr, (void* (*)(void*))pThreadFunc, pParam);
        if ( ret_code != 0 ){
            free(thread);
            thread = NULL;
			CLOUD_LOG_TRACE("Cyber cloud thread create failure.\n");
        }    
    }else{
    	CLOUD_LOG_TRACE("Cyber cloud thread create handle malloc failure.\n");
	}
    
    pthread_attr_destroy(&attr);  

    *pThreadHandle = (C_ThreadHandle)thread;

    return CLOUD_OK;
}

void CStb_DeleteThread ( IN  C_ThreadHandle   hThreadHandle )
{
    pthread_t *thread;
    char *d;
    if ( hThreadHandle == NULL ){
        return;
    }

    thread = (pthread_t*)hThreadHandle;
    pthread_join(*thread, NULL); 
    free(thread);
}

C_RESULT CStb_CreateSemaphore ( 
    OUT C_SemaphoreHandle *pHandle,
    IN  C_U32              uCount 
)
{
    sem_t *sem;
    int   retCode;
    
    assert(pHandle != NULL);
    
    sem = (sem_t*)malloc(sizeof(sem_t));
    if ( sem == NULL ){
        *pHandle = NULL;
		
		CLOUD_LOG_TRACE("Cyber cloud semaphore create failure.\n");
        return CLOUD_FAILURE;
    }
     
    // Initialize event semaphore
    retCode = sem_init(sem, 0, uCount);              
    if ( retCode != 0 ) {
        *pHandle = NULL;
        free(sem);
		CLOUD_LOG_TRACE("Cyber cloud semaphore sem_init failure.\n");

		return CLOUD_FAILURE;
    }    

    *pHandle = (C_SemaphoreHandle)sem;
     
    return CLOUD_OK;
}

void CStb_SemaphoreSignal( IN C_SemaphoreHandle   hHandle )
{
    assert(hHandle != NULL);

    sem_post((sem_t*)hHandle);   	
}

C_RESULT CStb_SemaphoreWait(
    IN C_SemaphoreHandle   hHandle,
    IN C_U32 		       uTimeout
)
{
    struct timespec ts;
	
    assert(hHandle != NULL);
    
    clock_gettime(CLOCK_REALTIME, &ts);

    ts.tv_sec  += uTimeout/1000;
    ts.tv_nsec += (uTimeout%1000)*1000*1000;
    ts.tv_sec  += ts.tv_nsec/1000000000;
    ts.tv_nsec  = ts.tv_nsec%1000000000;
    
    return sem_timedwait((sem_t*)hHandle, &ts) == 0 ? CLOUD_OK : CLOUD_TIMEOUT;
}

void CStb_DeleteSemaphore( IN C_SemaphoreHandle   hHandle )
{
    assert(hHandle != NULL);
    
    sem_destroy((sem_t*)hHandle);
    free((sem_t*)hHandle);  
}

void CStb_Sleep( IN C_U32 uMicroSeconds )
{
    usleep(uMicroSeconds);
}

C_U32 CStb_GetUpTime()
{
    C_U32 currentTime;
    struct timeval current;
    
    gettimeofday(&current, NULL);
    currentTime = current.tv_sec * 1000 + current.tv_usec / 1000;

    return (1.0 * currentTime + (current.tv_usec%1000) * 0.001);
}

C_RESULT  CStb_DemuxReceiveData ( 
    IN    C_FreqParam  const  *pTs,
    IN    C_U16 		    uPID,
    IN    C_U8                uTableId,
    OUT   C_U8 	            *pBuf,
    IN    C_U32 		    uBufMaxLen,
    OUT   C_U32 		    *pBytesReceived,
    IN    C_U32             uTimeout
)
{
    return CLOUD_OK;    
}

/**
 * @brief 分配内存
 */
void * CStb_Malloc( IN C_U32 uLength )
{
     return malloc( uLength );
}

/**
 * @brief 释放内存
 */
void CStb_Free(IN void * pBuf)
{
     free( pBuf );
}

void  CStb_Notify (  
	IN C_MessageCode  uMsgCode, 
	IN C_U8           uMessage[MSG_MAX_LEN], 
	IN C_U16      uErrCode 
)
{

}

void  CStb_OnKeyOutput( 
	IN C_U32          handle, 
	IN C_KeyType    uType, 
	IN C_U8         uLen, 
	IN C_U8*         puKeyData 
)
{
	//CLOUD_LOG_TRACE("Cyber cloud onKeyOutput.\n");
	
	if(uType == KeyType_HID){
		C_HID *p = (C_HID*)puKeyData;
		USBHIDACQ_OutputReport(handle, p->pdata, p->uDataSize);
	}
}


