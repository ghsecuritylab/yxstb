#include <sys/stat.h>
#include <netinet/in.h>

#include "vde.h"
#include "vde_callback.h"
#include "sys_basic_macro.h"
//#include "rtsp_hdr.h"

/* 两个RTSP时,开启该宏,启动线程保护 */
//#define MULTI_THREAD_RUN

#ifdef MULTI_THREAD_RUN
#include <pthread.h>
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#else
#define pthread_mutex_lock(...)
#define pthread_mutex_unlock(...)
#endif
static unsigned int fcc_NatInterval = 0;
static unsigned int fcc_DisorderTime = 0;

#ifdef ANDROID
#else
static SQAErrInfo sqa_error_info;
#endif

#pragma message("\n\n\nmake C58 SQA\n\n")
// #if _HW_BASE_VER_ != 58
// #error 基线版本与SQA版本不符！
// #endif


int yx_VDE_get_rtpdate_pos(unsigned char *pdate, int len, int flag);
static int fcc_env_val = 0;

//int yx_VDE_get_env_log(void)
//{
//    return fcc_env_val;
//}

int yx_VDE_set_debug(int level)
{
    fcc_env_val = level;
    return 0;
}

STB_HANDLE yx_FCC_VDE_init(struct FCC* hSTB, FCC_SERVER_TYPE server_type)
{
    LOG_SQA_PRINTF("@@@@@@@@@@@yx_FCC_VDE_init@@@@@@@@@@@@\n");

    struct FCC* g_fcc = NULL;
    CallBackFuns cbfs;
    SQACallBackFuns  sqacbfs;
    SQAParam pArgs;
    StreamParam spArgs;
    STB_HANDLE hSQA;
    char type = 0;
    STB_UINT32  ret;

    g_fcc = hSTB;
    if(NULL == g_fcc) {
        LOG_SQA_PRINTF("hSTB NULL!\n");
        return NULL;
    }

    cbfs.fcbGetMemory = yx_funcCallBackGetMemory;
    cbfs.fcbFreeMemory = yx_funcCallBackFreeMemory;
    cbfs.fcbGetTime = yx_funcCallBackGetTime;
    cbfs.fcbTraceOut = yx_funcCallBackTraceOut;
    cbfs.hClient = (STB_HANDLE)hSTB;

#if(SUPPORTE_HD)
    pArgs.nMemorySize = SQA_MEM_SIZE_8M;
#else
    pArgs.nMemorySize = SQA_MEM_SIZE_3M;
#endif
    pArgs.nFastCachePeriod = 50;
    pArgs.iCacheFirst = 1;
    pArgs.iBufferLevel[0] = 70;
    pArgs.iBufferLevel[1] = 50;
    pArgs.iBufferLevel[2] = 1000;

    pArgs.iRETSendPeriod = 50;
    pArgs.iRSRTimeOut = 500;
    pArgs.iSCNTimeOut = 40000;

    hSQA = sqa_init(&cbfs, &pArgs);

    if(NULL == hSQA) {
        LOG_SQA_ERROUT("VDE_init err!!!\n");
    }
    LOG_SQA_PRINTF("g_fcc->arg.type = %x\n", g_fcc->arg.type);

    sqacbfs.hClient = (STB_HANDLE)hSTB;
    sqacbfs.fcbSendFCC = yx_funcCallBackSendFCC;
    sqacbfs.fcbSendRET = yx_funcCallBackSendFeedBack;
    sqacbfs.fcbSendNAT = yx_funcCallBackSendNatData;
    sqacbfs.fcbIGMPJoin = yx_funcCallBackIGMPJoin;
    sqacbfs.fcbDataBufferLevel = yx_funcCallBackDataBufferLevelNULL;
    sqacbfs.fcbFccStateRsp = yx_funcCallBackTellFccStateRsp;
    sqacbfs.fcbFccIP = yx_funcCallBackFccIP;
    sqacbfs.fcbFCCPort = yx_funcCallBackFccPort;
    sqacbfs.fcbValidTime = yx_funcCallBackValidTime;
    sqacbfs.fcbhmsRsp = yx_funcCallBackHmsRsp;
    sqacbfs.fcbGetIP = yx_funcCallBackGetIP_FCC;

    spArgs.iSendRSRFlag = 0;

    if((g_fcc->arg.type & SQA_RET_CODE) > 0) {
        type |= RET_CODE;
        spArgs.iSendRSRFlag = 0;
        if(FCC_SERVER_RRS == server_type) {
            spArgs.iFCCServerFlag = 0;
            spArgs.iSendRSRFlag = 1;
        }
        if(FCC_SERVER_FCC == server_type) {
            spArgs.iFCCServerFlag = 1;
        }
    }
	PRINTF("g_fcc->arg.type = %x,g_fcc->arg.flag=%d \n", g_fcc->arg.type,g_fcc->arg.flag);
	if((g_fcc->arg.type & SQA_FCC_CODE) > 0 && g_fcc->arg.flag == 0){
        type |= FCC_CODE;
        spArgs.iSendRSRFlag = 0;
        if(FCC_SERVER_RRS == server_type) {
            spArgs.iFCCServerFlag = 0;
        }
        if(FCC_SERVER_FCC == server_type) {
            spArgs.iFCCServerFlag = 1;
        }

    }

    type |= FEC_CODE;
    spArgs.cType = type;
    LOG_SQA_PRINTF("spArgs.cType = %x,spArgs.iSendRSRFlag = %d\n", spArgs.cType, spArgs.iSendRSRFlag);
    spArgs.cRetType = HW;
    spArgs.port = (STB_UINT16)(g_fcc->fcc_port_base + 1);
    //spArgs.iRetPort = (STB_UINT16)(g_fcc->fcc_port_base +3);
    spArgs.cIPType = IPV4;

    spArgs.iFCCWithTLV = 0;
    spArgs.iFastCacheOn = 0;
    spArgs.iChanCacheTime = 100;
    spArgs.iRetDelayTime = 50;
    spArgs.iRetInterval = 300;
    //sHandle = hSTB->hFcc;
    if(FCC_SERVER_RRS == server_type)
        spArgs.iFCCServerFlag = 0;
    if(FCC_SERVER_FCC == server_type)
        spArgs.iFCCServerFlag = 1;

    spArgs.iNatSupport = 1;
    if(0 == fcc_NatInterval)
        spArgs.iNatHBinterval = 50000;
    else {
        spArgs.iNatHBinterval = yx_FCC_NatInterval_get();
        spArgs.iNatHBinterval = 1000 * (spArgs.iNatHBinterval);
    }
    spArgs.iclientSSRC = 0;
    if(0 == fcc_DisorderTime)
        spArgs.iDisorderTime = 100;
    else {
        spArgs.iDisorderTime = yx_FCC_DisorderTime_get();
        spArgs.iDisorderTime = 1000 * (spArgs.iDisorderTime);
    }
    //sHandle = g_ret->hVOD;

    ret = sqa_set_parameter(hSQA, &spArgs, &sqacbfs);
    LOG_SQA_PRINTF("hSQA=%p,ret = %d\n", hSQA, ret);

    return hSQA;

Err:
    return NULL;
}

int yx_SQA_VDE_recvRtcpPacket(STB_HANDLE hSqa, void *pData, STB_UINT32 ulPacketLen)
{
    int ret = 0;

    if(NULL == pData || NULL == hSqa) {
        LOG_SQA_PRINTF("ERROR!!!\n");
        return -1;
    }

    pthread_mutex_lock(&mutex);
    ret = sqa_recv_rtcp_packet(hSqa, pData, ulPacketLen);
    pthread_mutex_unlock(&mutex);
    return ret;
}

int yx_SQA_VDE_Destroy(STB_HANDLE hSqa)
{
    if(NULL == hSqa)
        return 1;
    pthread_mutex_lock(&mutex);
    if(hSqa) {
        LOG_SQA_PRINTF("Destroy SQA hSqa : %p\n", hSqa);
        sqa_destroy(hSqa);
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

void yx_SQA_VDE_handleEvent(STB_HANDLE hSqa)
{
    pthread_mutex_lock(&mutex);
    sqa_handle_event(hSqa);
#ifdef ANDROID
#else
	sqa_get_err_info(hSqa, &sqa_error_info);
#endif
    pthread_mutex_unlock(&mutex);
    return;
}


STB_HANDLE yx_RET_VDE_init(struct RET* hSTB, FCC_SERVER_TYPE server_type)
{
    LOG_SQA_PRINTF("@@@@@@@@@@@yx_RET_VDE_init@@@@@@@@@@@@\n");

    struct RET* g_ret = NULL;

    CallBackFuns cbfs;
    SQACallBackFuns sqacbfs;
    SQAParam pArgs;
    StreamParam spArgs;
    STB_HANDLE hSQA;
    char type;
    STB_UINT32  ret;

    g_ret = hSTB;
    if(NULL == g_ret) {
        LOG_SQA_PRINTF("hSTB NULL!\n");
        return NULL;
    }


    cbfs.hClient = (STB_HANDLE)hSTB;
    cbfs.fcbGetMemory = yx_funcCallBackGetMemory;
    cbfs.fcbFreeMemory = yx_funcCallBackFreeMemory;
    cbfs.fcbGetTime = yx_funcCallBackGetTime;
    cbfs.fcbTraceOut = yx_funcCallBackTraceOut;

#if(SUPPORTE_HD)
    pArgs.nMemorySize = SQA_MEM_SIZE_15M;
#else
    pArgs.nMemorySize = SQA_MEM_SIZE_4M;
#endif
    pArgs.nFastCachePeriod = 50;
    pArgs.iCacheFirst = 1;
    pArgs.iBufferLevel[0] = 70;
    pArgs.iBufferLevel[1] = 50;
    pArgs.iBufferLevel[2] = 1000;
    pArgs.iRETSendPeriod = 50;
    pArgs.iRSRTimeOut = 500;
    pArgs.iSCNTimeOut = 12000;

    hSQA = sqa_init(&cbfs, &pArgs);
    if(NULL == hSQA) {
        LOG_SQA_ERROUT("VDE_init err!!!\n");
    }

    sqacbfs.hClient = (STB_HANDLE)hSTB;
    sqacbfs.fcbSendFCC = yx_funcCallBackSendFCC;
    sqacbfs.fcbSendRET = yx_funcCallBackSendFeedBack;
    sqacbfs.fcbSendNAT = yx_funcCallBackSendNatData;
    sqacbfs.fcbIGMPJoin = yx_funcCallBackIGMPJoin;
    sqacbfs.fcbDataBufferLevel = yx_funcCallBackDataBufferLevel;
    sqacbfs.fcbFccStateRsp = yx_funcCallBackTellFccStateRsp;
    sqacbfs.fcbFccIP = yx_funcCallBackFccIP;
    sqacbfs.fcbFCCPort = yx_funcCallBackFccPort;
    sqacbfs.fcbValidTime = yx_funcCallBackValidTime;
    sqacbfs.fcbhmsRsp = yx_funcCallBackHmsRsp;
    sqacbfs.fcbGetIP = yx_funcCallBackGetIP_RET;

    type = RET_CODE;
    spArgs.cType = type;
    LOG_SQA_PRINTF("spArgs.cType = %x\n", spArgs.cType);
    spArgs.cRetType = HW;
    spArgs.port =  g_ret->arg.clt_port;
    spArgs.cIPType = IPV4;
    spArgs.iSendRSRFlag = 0;
    spArgs.iFCCWithTLV = 0;
    spArgs.iFastCacheOn = 1;
    spArgs.iChanCacheTime = 100;
    spArgs.iRetDelayTime = 50;
    spArgs.iRetInterval = 300;

    //sHandle = hSTB->hFcc;
    if(FCC_SERVER_RRS == server_type)
        spArgs.iFCCServerFlag = 0;
    if(FCC_SERVER_FCC == server_type)
        spArgs.iFCCServerFlag = 0;
    spArgs.iNatSupport = 1;
    if(0 == fcc_NatInterval)
        spArgs.iNatHBinterval = 50000;
    else {
        spArgs.iNatHBinterval = yx_FCC_NatInterval_get();
        spArgs.iNatHBinterval = 1000 * (spArgs.iNatHBinterval);
    }
    spArgs.iclientSSRC = 0;
    if(0 == fcc_DisorderTime)
        spArgs.iDisorderTime = 100;
    else {
        spArgs.iDisorderTime = yx_FCC_DisorderTime_get();
        spArgs.iDisorderTime = 1000 * (spArgs.iDisorderTime);
    }
    //sHandle = g_ret->hVOD;
    ret = sqa_set_parameter(hSQA, &spArgs, &sqacbfs);


    LOG_SQA_PRINTF("hSQA=%p,hSTB->arg=%p,ret = %d\n", hSQA, hSTB->arg, ret);
    return hSQA;

Err:
    return NULL;
}

STB_HANDLE yx_ARQ_VDE_init(struct RET* hSTB)
{
    struct RET *g_ret = NULL;
    CallBackFuns cbfs;
    SQACallBackFuns sqacbfs;
    SQAParam pArgs;
    StreamParam spArgs;
    STB_HANDLE hSQA;
    char type;
    STB_UINT32 ret = 0;

    LOG_SQA_PRINTF("SQA RET VDE init \n");
    g_ret = hSTB;
    if(NULL == g_ret) {
        LOG_SQA_PRINTF("hSTB NULL!\n");
        return NULL;
    }

    cbfs.hClient = (STB_HANDLE)hSTB;
    cbfs.fcbGetMemory = yx_funcCallBackGetMemory;
    cbfs.fcbFreeMemory = yx_funcCallBackFreeMemory;
    cbfs.fcbGetTime = yx_funcCallBackGetTime;
    cbfs.fcbTraceOut = yx_funcCallBackTraceOut;

#if(SUPPORTE_HD)
    pArgs.nMemorySize = SQA_MEM_SIZE_15M;
#else
    pArgs.nMemorySize = SQA_MEM_SIZE_4M;
#endif
    pArgs.nFastCachePeriod = 50;
    pArgs.iCacheFirst = 1;
    pArgs.iBufferLevel[0] = 70;
    pArgs.iBufferLevel[1] = 50;
    pArgs.iBufferLevel[2] = 1000;
    pArgs.iRETSendPeriod = 50;
    pArgs.iRSRTimeOut = 500;
    pArgs.iSCNTimeOut = 12000;
    hSQA = sqa_init(&cbfs, &pArgs);
    if(NULL == hSQA) {
        LOG_SQA_ERROUT("VDE_init err!!!\n");
    }

    sqacbfs.hClient = (STB_HANDLE)hSTB;
    sqacbfs.fcbSendFCC = yx_funcCallBackSendFCC;
    sqacbfs.fcbSendRET = yx_funcCallBackSendFeedBack_ARQ;
    sqacbfs.fcbSendNAT = yx_funcCallBackSendNatData;
    sqacbfs.fcbIGMPJoin = yx_funcCallBackIGMPJoin;
    sqacbfs.fcbDataBufferLevel = yx_funcCallBackDataBufferLevel;
    sqacbfs.fcbFccStateRsp = yx_funcCallBackTellFccStateRsp;
    sqacbfs.fcbFccIP = yx_funcCallBackFccIP;
    sqacbfs.fcbFCCPort = yx_funcCallBackFccPort;
    sqacbfs.fcbValidTime = yx_funcCallBackValidTime;
    sqacbfs.fcbhmsRsp = yx_funcCallBackHmsRsp;
    sqacbfs.fcbGetIP = yx_funcCallBackGetIP_RET;

    type = RET_CODE;
    spArgs.cType = type;
    LOG_SQA_PRINTF("###SQA### spArgs.cType = %x\n", spArgs.cType);
    spArgs.cRetType = CTC;
    spArgs.port =  g_ret->arg.clt_port;
    spArgs.cIPType = IPV4;
    spArgs.iSendRSRFlag = 0;
    spArgs.iFCCWithTLV = 0;
    spArgs.iFastCacheOn = 1;
    spArgs.iChanCacheTime = 100;
    spArgs.iRetDelayTime = 50;
    spArgs.iRetInterval = 300;

    spArgs.iFCCServerFlag = 0;
    spArgs.iNatSupport = 1;
    spArgs.iNatHBinterval = 50000;
    spArgs.iclientSSRC = 0;
    spArgs.iDisorderTime = 100;
    ret = sqa_set_parameter(hSQA, &spArgs, &sqacbfs);
    LOG_SQA_PRINTF("###SQA### hSQA=%p, hSTB->arg=%p, ret = %d\n", hSQA, hSTB->arg, ret);
    return hSQA;
Err:
    return NULL;
}

STB_HANDLE yx_RTCP_VDE_init(struct RET* hSTB)
{
    struct RET *g_ret = NULL;
    CallBackFuns cbfs;
    SQACallBackFuns sqacbfs;
    SQAParam pArgs;
    StreamParam spArgs;
    STB_HANDLE hSQA;
    char type;
    STB_UINT32 ret = 0;

    LOG_SQA_PRINTF("SQA RET VDE init \n");
    g_ret = hSTB;
    if(NULL == g_ret) {
        LOG_SQA_PRINTF("hSTB NULL!\n");
        return NULL;
    }

    cbfs.hClient = (STB_HANDLE)hSTB;
    cbfs.fcbGetMemory = yx_funcCallBackGetMemory;
    cbfs.fcbFreeMemory = yx_funcCallBackFreeMemory;
    cbfs.fcbGetTime = yx_funcCallBackGetTime;
    cbfs.fcbTraceOut = yx_funcCallBackTraceOut;

#if(SUPPORTE_HD)
    pArgs.nMemorySize = SQA_MEM_SIZE_15M;
#else
    pArgs.nMemorySize = SQA_MEM_SIZE_4M;
#endif
    pArgs.nFastCachePeriod = 50;
    pArgs.iCacheFirst = 1;
    pArgs.iBufferLevel[0] = 70;
    pArgs.iBufferLevel[1] = 50;
    pArgs.iBufferLevel[2] = 1000;
    pArgs.iRETSendPeriod = 50;
    pArgs.iRSRTimeOut = 500;
    pArgs.iSCNTimeOut = 12000;
    hSQA = sqa_init(&cbfs, &pArgs);
    if(NULL == hSQA) {
        LOG_SQA_ERROUT("VDE_init err!!!\n");
    }

    sqacbfs.hClient = (STB_HANDLE)hSTB;
    sqacbfs.fcbSendFCC = yx_funcCallBackSendFCCNULL;
    sqacbfs.fcbSendRET = yx_funcCallBackSendFeedBack_RTCP;
    sqacbfs.fcbSendNAT = yx_funcCallBackSendNatDataNULL;
    sqacbfs.fcbIGMPJoin = yx_funcCallBackIGMPJoinNULL;
    sqacbfs.fcbDataBufferLevel = yx_funcCallBackDataBufferLevelNULL;
    sqacbfs.fcbFccStateRsp = yx_funcCallBackTellFccStateRspNULL;
    sqacbfs.fcbFccIP = yx_funcCallBackFccIP;
    sqacbfs.fcbFCCPort = yx_funcCallBackFccPort;
    sqacbfs.fcbValidTime = yx_funcCallBackValidTime;
    sqacbfs.fcbhmsRsp = yx_funcCallBackHmsRsp;
    sqacbfs.fcbGetIP = yx_funcCallBackGetIP_RET;

    type = RET_CODE;
    spArgs.cType = type;
    LOG_SQA_PRINTF("###SQA### spArgs.cType = %x\n", spArgs.cType);
    spArgs.cRetType = CTC;
    spArgs.port =  g_ret->arg.clt_port;
    spArgs.cIPType = IPV4;
    spArgs.iSendRSRFlag = 0;
    spArgs.iFCCWithTLV = 0;
    spArgs.iFastCacheOn = 1;
    spArgs.iChanCacheTime = 100;
    spArgs.iRetDelayTime = 50;
    spArgs.iRetInterval = 300;

    spArgs.iFCCServerFlag = 0;
    spArgs.iNatSupport = 1;
    spArgs.iNatHBinterval = 50000;
    spArgs.iclientSSRC = 0;
    spArgs.iDisorderTime = 100;
    ret = sqa_set_parameter(hSQA, &spArgs, &sqacbfs);
    LOG_SQA_PRINTF("###SQA### hSQA=%p, hSTB->arg=%p, ret = %d\n", hSQA, hSTB->arg, ret);
    return hSQA;
Err:
    return NULL;
}


void yx_FCC_NatInterval_set(int Interval_time)
{
    fcc_NatInterval = Interval_time;
}

int yx_FCC_NatInterval_get(void)
{
    return fcc_NatInterval;
}

void yx_FCC_DisorderTime_set(int DisorderTime)
{
    fcc_DisorderTime = DisorderTime;
}

int yx_FCC_DisorderTime_get(void)
{
    return fcc_DisorderTime;
}

#ifdef ANDROID
#else
int get_sqa_errInfo(int errInfoType)
{
	int errInfo = -1;
	
    switch(errInfoType) {
    case LOSTNUM:
        errInfo = sqa_error_info.uiLostNum;
		break;
    case DISORDNUM:
        errInfo = sqa_error_info.uiDisordNum;
		break;
    case PCRBITRATE:
        errInfo = sqa_error_info.uiPCRBitrate;
		break;
    case LOCALBITRATE:
        errInfo = sqa_error_info.uiLocalBitrate;
		break;
    default:
		break;
	}
	return errInfo;
}
#endif