#include "Tr069ServiceStatistics.h"

#include "Tr069FunctionCall.h"
#include "TR069Assertions.h"
#include "StatisticBase.h"


Tr069Call* g_tr069ServiceStatistics = new Tr069ServiceStatistics();

/*******************************************************************************
//CTC,HW,CU共有的
********************************************************************************/
/*------------------------------------------------------------------------------
	起始时间
	格式：yyyymmddhhmmss
 ------------------------------------------------------------------------------*/
static int getAppTr069PortStartpoint(char* str, unsigned int val)
{
    tr069_statistic_get_Startpoint(str, val);
		
		return 0;
}

/*------------------------------------------------------------------------------
	结束时间
	格式：yyyymmddhhmmss
 ------------------------------------------------------------------------------*/
static int getAppTr069PortEndpoint(char* str, unsigned int val)
{
    tr069_statistic_get_Endpoint(str, val);
	
		return 0;	
}

/*------------------------------------------------------------------------------
	认证的总次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortAuthNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_AuthNumbers());
    
    return 0; 	
}

/*------------------------------------------------------------------------------
	认证失败的总次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortAuthFailNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_AuthFailNumbers());
    
    return 0; 		
}

/*------------------------------------------------------------------------------
	认证失败详细信息
	每次失败，需要另起一行记录认证返回的错误码及相关信息
	------------------------------------------------------------------------------*/
static int getAppTr069PortAuthFailInfo(char* str, unsigned int val)
{
    tr069_statistic_get_AuthFailInfo(str, val);
		
		return 0;		
}

/*------------------------------------------------------------------------------
	加入组播组的总次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiReqNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiReqNumbers());
    
    return 0; 		
}

/*------------------------------------------------------------------------------
	加入组播组失败（加入后没有收到组播数据）的总次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiFailNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiFailNumbers());
    
    return 0; 			
}

/*------------------------------------------------------------------------------
	加入组播组失败（加入后没有收到组播数据）的信息，即要加入的组播频道地址
	每次失败，需要另起一行记录失败的组播频道信息
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiFailInfo(char* str, unsigned int val)
{
    tr069_statistic_get_MultiFailInfo(str, val);
		
		return 0;			
}

/*------------------------------------------------------------------------------
	单播（含点播、回看和时移）申请的总次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVodReqNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VodReqNumbers());
    
    return 0; 			
}

/*------------------------------------------------------------------------------
	单播（含点播、回看和时移）申请失败的总次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVodFailNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VodFailNumbers());
    
    return 0; 	
}

/*------------------------------------------------------------------------------
	单播（含点播、回看和时移）申请失败详细信息
	每次失败，需要另起一行记录返回的错误码及失败的点播、回看和时移的URL信息
	格式举例：RTSP://…./xx.mpg/
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVodFailInfo(char* str, unsigned int val)
{
    tr069_statistic_get_VodFailInfo(str, val);
	
		return 0;			
}

/*------------------------------------------------------------------------------
	Http请求的总次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHTTPReqNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HTTPReqNumbers());

    return 0; 	
}

/*------------------------------------------------------------------------------
	Http请求失败的总次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHTTPFailNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HTTPFailNumbers());
   
    return 0; 		
}

/*------------------------------------------------------------------------------
	Http请求失败详细信息
	每次失败，需要另起一行记录返回的错误码及失败的URL
	说明：机顶盒只记录但前请求失败的页面URL，不记录该页面包含的所有URL
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHTTPFailInfo(char* str, unsigned int val)
{
    tr069_statistic_get_HTTPFailInfo(str, val);
	return 0;			
}

/*------------------------------------------------------------------------------
	异常断流的次数
	异常断流定义为：在视频播放过程中（包括组播和单播），因为缓冲区被取空导致无法再播放的次数。
 ------------------------------------------------------------------------------*/
static int getAppTr069PortAbendNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_AbendNumbers());
    
    return 0; 			
}

/*------------------------------------------------------------------------------
	异常断流的详细信息
 ------------------------------------------------------------------------------*/
static int getAppTr069PortAbendInfo(char* str, unsigned int val)
{
    tr069_statistic_get_AbendInfo(str, val);
		
		return 0;			
}


/*------------------------------------------------------------------------------
	组播丢包率范围1发生次数
 ------------------------------------------------------------------------------*/
 
static int getAppTr069PortMultiPacketsLostR1(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiPacketsLostR1());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	组播丢包率范围2发生次数
 ------------------------------------------------------------------------------*/
 
static int getAppTr069PortMultiPacketsLostR2(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiPacketsLostR2());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	组播丢包率范围3发生次数
 ------------------------------------------------------------------------------*/
 
static int getAppTr069PortMultiPacketsLostR3(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiPacketsLostR3());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	组播丢包率范围4发生次数
 ------------------------------------------------------------------------------*/
 
static int getAppTr069PortMultiPacketsLostR4(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiPacketsLostR4());

    return 0;		
}

/*------------------------------------------------------------------------------
	组播丢包率范围5发生次数
 ------------------------------------------------------------------------------*/
 
static int getAppTr069PortMultiPacketsLostR5(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiPacketsLostR5());
    
    return 0;		
}


/*------------------------------------------------------------------------------
	单播丢包率范围1发生次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODPacketsLostR1(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODPacketsLostR1());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	单播丢包率范围2发生次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODPacketsLostR2(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODPacketsLostR2());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	单播丢包率范围3发生次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODPacketsLostR3(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODPacketsLostR3());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	单播丢包率范围4发生次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODPacketsLostR4(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODPacketsLostR4());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	单播丢包率范围5发生次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODPacketsLostR5(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODPacketsLostR5());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	比特率在BitRateR1范围内的次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortBitRateR1(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_BitRateR1());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	比特率在BitRateR2范围内的次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortBitRateR2(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_BitRateR2());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	比特率在BitRateR3范围内的次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortBitRateR3(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_BitRateR3());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	比特率在BitRateR4范围内的次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortBitRateR4(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_BitRateR4());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	比特率在BitRateR5范围内的次数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortBitRateR5(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_BitRateR5());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	在一个采样周期内的丢包数
 ------------------------------------------------------------------------------*/
static int getAppTr069PortPacketsLost(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_PacketsLost());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	在一个采样周期内的最大抖动值，单位：ms
 ------------------------------------------------------------------------------*/
static int getAppTr069PortJitterMax(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_JitterMax());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	当前正在播放帧率
	如果无，则不记录；
 ------------------------------------------------------------------------------*/
static int getAppTr069PortFrameRate(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_FrameRate());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	累积丢帧数
	如果无，则不记录；
 ------------------------------------------------------------------------------*/
static int getAppTr069PortFrameLoss(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_FrameLoss());
    
    return 0;			
}

/*******************************************************************************
//CTC,HW共有的
********************************************************************************/

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortAbendDurationTotal(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_AbendDurationTotal());

    return 0; 		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortAbendDurationMax(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_AbendDurationMax());
    
    return 0; 			
}


/*******************************************************************************
//CTC,CU 共有
********************************************************************************/



/*------------------------------------------------------------------------------
	异常断流的次数
	异常断流定义为：在视频播放过程中（包括组播和单播），因为缓冲区被取空导致无法再播放的次数。
 ------------------------------------------------------------------------------*/
 /*
static int getAppTr069PortAbendNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_AbendNumbers());
    
    return 0; 			
}
*/
/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODAbendNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODAbendNumbers());
    
    return 0; 	
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiAbendNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiAbendNumbers());
    
    return 0; 		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortPlayErrorInfo(char* str, unsigned int val)
{
    tr069_statistic_get_PlayErrorInfo(str, val);
	
		return 0;				
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiPacketsLostR1Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiPacketsLostR1Nmb());
    
    return 0; 	
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiPacketsLostR2Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiPacketsLostR2Nmb());
    
    return 0; 		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiPacketsLostR3Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiPacketsLostR3Nmb());
    
    return 0; 	
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiPacketsLostR4Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiPacketsLostR4Nmb());
    
    return 0;	
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiPacketsLostR5Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiPacketsLostR5Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODPacketsLostR1Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODPacketsLostR1Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODPacketsLostR2Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODPacketsLostR2Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODPacketsLostR3Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODPacketsLostR3Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODPacketsLostR4Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODPacketsLostR4Nmb());

    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODPacketsLostR5Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODPacketsLostR5Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiBitRateR1Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiBitRateR1Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiBitRateR2Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiBitRateR2Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiBitRateR3Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiBitRateR3Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiBitRateR4Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiBitRateR4Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiBitRateR5Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiBitRateR5Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODBitRateR1Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODBitRateR1Nmb());

    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODBitRateR2Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODBitRateR2Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODBitRateR3Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODBitRateR3Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODBitRateR4Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODBitRateR4Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODBitRateR5Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODBitRateR5Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortFramesLostR1Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_FramesLostR1Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortFramesLostR2Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_FramesLostR2Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortFramesLostR3Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_FramesLostR3Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortFramesLostR4Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_FramesLostR4Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortFramesLostR5Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_FramesLostR5Nmb());
    
    return 0;		
}


/*******************************************************************************
//CTC独有的接口
********************************************************************************/

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiRRT(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiRRT());
    
    return 0; 		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVodRRT(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VodRRT());
    
    return 0; 		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHTTPRRT(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HTTPRRT());
    
    return 0; 	
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortMultiAbendUPNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_MultiAbendUPNumbers());
    
    return 0; 			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortVODUPAbendNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_VODUPAbendNumbers());
    
    return 0; 		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDMultiAbendNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_MultiAbendNumbers());
    
    return 0; 			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDVODAbendNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_VODAbendNumbers());
    
    return 0; 	
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDMultiUPAbendNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_MultiUPAbendNumbers());
   
    return 0; 		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDVODUPAbendNumbers(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_VODUPAbendNumbers());
    
    return 0; 		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortFECMultiPacketsLostR1Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_FECMultiPacketsLostR1Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortFECMultiPacketsLostR2Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_FECMultiPacketsLostR2Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortFECMultiPacketsLostR3Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_FECMultiPacketsLostR3Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortFECMultiPacketsLostR4Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_FECMultiPacketsLostR4Nmb());

    return 0;				
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortFECMultiPacketsLostR5Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_FECMultiPacketsLostR5Nmb());
    
    return 0;				
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortARQVODPacketsLostR1Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_ARQVODPacketsLostR1Nmb());

    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortARQVODPacketsLostR2Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_ARQVODPacketsLostR2Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortARQVODPacketsLostR3Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_ARQVODPacketsLostR3Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortARQVODPacketsLostR4Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_ARQVODPacketsLostR4Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortARQVODPacketsLostR5Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_ARQVODPacketsLostR5Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDMultiPacketsLostR1Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_MultiPacketsLostR1Nmb());

    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDMultiPacketsLostR2Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_MultiPacketsLostR2Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDMultiPacketsLostR3Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_MultiPacketsLostR3Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDMultiPacketsLostR4Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_MultiPacketsLostR4Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDMultiPacketsLostR5Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_MultiPacketsLostR5Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDFECMultiPacketsLostR1Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_FECMultiPacketsLostR1Nmb());

    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDFECMultiPacketsLostR2Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_FECMultiPacketsLostR2Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDFECMultiPacketsLostR3Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_FECMultiPacketsLostR3Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDFECMultiPacketsLostR4Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_FECMultiPacketsLostR4Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDFECMultiPacketsLostR5Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_FECMultiPacketsLostR5Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDVODPacketsLostR1Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_VODPacketsLostR1Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDVODPacketsLostR2Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_VODPacketsLostR2Nmb());
    
    return 0;	
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDVODPacketsLostR3Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_VODPacketsLostR3Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDVODPacketsLostR4Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_VODPacketsLostR4Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDVODPacketsLostR5Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_VODPacketsLostR5Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDARQVODPacketsLostR1Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_ARQVODPacketsLostR1Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDARQVODPacketsLostR2Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_ARQVODPacketsLostR2Nmb());
    
    return 0;	
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDARQVODPacketsLostR3Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_ARQVODPacketsLostR3Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDARQVODPacketsLostR4Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_ARQVODPacketsLostR4Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDARQVODPacketsLostR5Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_ARQVODPacketsLostR5Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDVODBitRateR1Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_VODBitRateR1Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDVODBitRateR2Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_VODBitRateR2Nmb());
    
    return 0;				
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDVODBitRateR3Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_VODBitRateR3Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDVODBitRateR4Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_VODBitRateR4Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDVODBitRateR5Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_VODBitRateR5Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDMultiBitRateR1Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_MultiBitRateR1Nmb());
    
    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDMultiBitRateR2Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_MultiBitRateR2Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDMultiBitRateR3Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_MultiBitRateR3Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDMultiBitRateR4Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_MultiBitRateR4Nmb());
    
    return 0;	
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortHDMultiBitRateR5Nmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_HD_MultiBitRateR5Nmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortBufferIncNmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_BufferIncNmb());
    
    return 0;		
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortBufferDecNmb(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_BufferDecNmb());
    
    return 0;		
}


/*******************************************************************************
//HW独有的接口
********************************************************************************/

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortBootUpTime(char* str, unsigned int val)
{
    snprintf(str, val, "%d", tr069_statistic_get_BootUpTime());

    return 0;			
}

/*------------------------------------------------------------------------------
	
 ------------------------------------------------------------------------------*/
static int getAppTr069PortPowerTime(char* str, unsigned int val)
{
    tr069_statistic_get_PowerTime(str, val);
  
	  return 0;
}


Tr069ServiceStatistics::Tr069ServiceStatistics()
	: Tr069GroupCall("ServiceStatistics")
{
    //  HW,CTC,CU 共有		
    Tr069Call* Startpoint         = new Tr069FunctionCall("Startpoint", getAppTr069PortStartpoint, NULL);
    Tr069Call* Endpoint           = new Tr069FunctionCall("Endpoint", getAppTr069PortEndpoint, NULL);
    Tr069Call* AuthNumbers        = new Tr069FunctionCall("AuthNumbers", getAppTr069PortAuthNumbers, NULL);
    Tr069Call* AuthFailNumbers    = new Tr069FunctionCall("AuthFailNumbers", getAppTr069PortAuthFailNumbers, NULL);
    Tr069Call* AuthFailInfo       = new Tr069FunctionCall("AuthFailInfo", getAppTr069PortAuthFailInfo, NULL);
    Tr069Call* MultiReqNumbers    = new Tr069FunctionCall("MultiReqNumbers", getAppTr069PortMultiReqNumbers, NULL);
    Tr069Call* MultiFailNumbers   = new Tr069FunctionCall("MultiFailNumbers", getAppTr069PortMultiFailNumbers, NULL); 
    Tr069Call* MultiFailInfo      = new Tr069FunctionCall("MultiFailInfo", getAppTr069PortMultiFailInfo, NULL);
    Tr069Call* VodReqNumbers      = new Tr069FunctionCall("VodReqNumbers", getAppTr069PortVodReqNumbers, NULL);
    Tr069Call* VodFailNumbers     = new Tr069FunctionCall("VodFailNumbers", getAppTr069PortVodFailNumbers, NULL);
    Tr069Call* VodFailInfo        = new Tr069FunctionCall("VodFailInfo", getAppTr069PortVodFailInfo, NULL);
    Tr069Call* HTTPReqNumbers     = new Tr069FunctionCall("HTTPReqNumbers", getAppTr069PortHTTPReqNumbers, NULL);
    Tr069Call* HTTPFailNumbers    = new Tr069FunctionCall("HTTPFailNumbers", getAppTr069PortHTTPFailNumbers, NULL);
    Tr069Call* HTTPFailInfo       = new Tr069FunctionCall("HTTPFailInfo", getAppTr069PortHTTPFailInfo, NULL);    
    Tr069Call* AbendNumbers       = new Tr069FunctionCall("AbendNumbers", getAppTr069PortAbendNumbers, NULL);
    Tr069Call* AbendInfo          = new Tr069FunctionCall("AbendInfo", getAppTr069PortAbendInfo, NULL);
 
    Tr069Call* MultiPacketsLostR1 = new Tr069FunctionCall("MultiPacketsLostR1", getAppTr069PortMultiPacketsLostR1, NULL);
    Tr069Call* MultiPacketsLostR2 = new Tr069FunctionCall("MultiPacketsLostR2", getAppTr069PortMultiPacketsLostR2, NULL);
    Tr069Call* MultiPacketsLostR3 = new Tr069FunctionCall("MultiPacketsLostR3", getAppTr069PortMultiPacketsLostR3, NULL);
    Tr069Call* MultiPacketsLostR4 = new Tr069FunctionCall("MultiPacketsLostR4", getAppTr069PortMultiPacketsLostR4, NULL);
    Tr069Call* MultiPacketsLostR5 = new Tr069FunctionCall("MultiPacketsLostR5", getAppTr069PortMultiPacketsLostR5, NULL);

    Tr069Call* VODPacketsLostR1   = new Tr069FunctionCall("VODPacketsLostR1", getAppTr069PortVODPacketsLostR1, NULL);
    Tr069Call* VODPacketsLostR2   = new Tr069FunctionCall("VODPacketsLostR2", getAppTr069PortVODPacketsLostR2, NULL);
    Tr069Call* VODPacketsLostR3   = new Tr069FunctionCall("VODPacketsLostR3", getAppTr069PortVODPacketsLostR3, NULL);
    Tr069Call* VODPacketsLostR4   = new Tr069FunctionCall("VODPacketsLostR4", getAppTr069PortVODPacketsLostR4, NULL);
    Tr069Call* VODPacketsLostR5   = new Tr069FunctionCall("VODPacketsLostR5", getAppTr069PortVODPacketsLostR5, NULL);
    
    Tr069Call* BitRateR1          = new Tr069FunctionCall("BitRateR1", getAppTr069PortBitRateR1, NULL);
    Tr069Call* BitRateR2          = new Tr069FunctionCall("BitRateR2", getAppTr069PortBitRateR2, NULL);
    Tr069Call* BitRateR3          = new Tr069FunctionCall("BitRateR3", getAppTr069PortBitRateR3, NULL);
    Tr069Call* BitRateR4          = new Tr069FunctionCall("BitRateR4", getAppTr069PortBitRateR4, NULL);
    Tr069Call* BitRateR5          = new Tr069FunctionCall("BitRateR5", getAppTr069PortBitRateR5, NULL);
    
    Tr069Call* PacketsLost        = new Tr069FunctionCall("PacketsLost", getAppTr069PortPacketsLost, NULL);
    Tr069Call* JitterMax          = new Tr069FunctionCall("JitterMax", getAppTr069PortJitterMax, NULL);
    Tr069Call* FrameRate          = new Tr069FunctionCall("FrameRate", getAppTr069PortFrameRate, NULL);
    Tr069Call* FrameLoss          = new Tr069FunctionCall("FrameLoss", getAppTr069PortFrameLoss, NULL);


    regist(Startpoint->name(),       Startpoint);
    regist(Endpoint->name(),         Endpoint);
    regist(AuthNumbers->name(),      AuthNumbers);
    regist(AuthFailNumbers->name(),  AuthFailNumbers);
    regist(AuthFailInfo->name(),     AuthFailInfo);
    regist(MultiReqNumbers->name(),  MultiReqNumbers);
    regist(MultiFailNumbers->name(), MultiFailNumbers);
    regist(MultiFailInfo->name(),    MultiFailInfo);
    regist(VodReqNumbers->name(),    VodReqNumbers);
    regist(VodFailNumbers->name(),   VodFailNumbers);
    regist(VodFailInfo->name(),      VodFailInfo);
    regist(HTTPReqNumbers->name(),   HTTPReqNumbers);
    regist(HTTPFailNumbers->name(),  HTTPFailNumbers);
    regist(HTTPFailInfo->name(),     HTTPFailInfo);
    regist(AbendNumbers->name(),     AbendNumbers);
    regist(AbendInfo->name(),        AbendInfo);
    
    regist(MultiPacketsLostR1->name(),        MultiPacketsLostR1);
    regist(MultiPacketsLostR2->name(),        MultiPacketsLostR2);
    regist(MultiPacketsLostR3->name(),        MultiPacketsLostR3);
    regist(MultiPacketsLostR4->name(),        MultiPacketsLostR4);
    regist(MultiPacketsLostR5->name(),        MultiPacketsLostR5);
    
    regist(VODPacketsLostR1->name(),        VODPacketsLostR1);
    regist(VODPacketsLostR2->name(),        VODPacketsLostR2);
    regist(VODPacketsLostR3->name(),        VODPacketsLostR3);
    regist(VODPacketsLostR4->name(),        VODPacketsLostR4);
    regist(VODPacketsLostR5->name(),        VODPacketsLostR5);
    
    regist(BitRateR1->name(),        BitRateR1);
    regist(BitRateR2->name(),        BitRateR2);
    regist(BitRateR3->name(),        BitRateR3);
    regist(BitRateR4->name(),        BitRateR4);
    regist(BitRateR5->name(),        BitRateR5);
    
    regist(PacketsLost->name(),      PacketsLost);
    regist(JitterMax->name(),        JitterMax);
    regist(FrameRate->name(),        FrameRate);
    regist(FrameLoss->name(),        FrameLoss);

 
    //HW,CTC 共有
    
    Tr069Call* TotalAbendDuration = new Tr069FunctionCall("TotalAbendDuration", getAppTr069PortAbendDurationTotal, NULL);
    Tr069Call* MaxAbendDuration   = new Tr069FunctionCall("MaxAbendDuration", getAppTr069PortAbendDurationMax, NULL);

    regist(TotalAbendDuration->name(),        TotalAbendDuration);
    regist(MaxAbendDuration->name(), MaxAbendDuration);


    //CTC,CU 共有
    Tr069Call* PlayErrorNumbers            = new Tr069FunctionCall("PlayErrorNumbers", getAppTr069PortAbendNumbers, NULL);
    Tr069Call* VODAbendNumbers             = new Tr069FunctionCall("VODAbendNumbers", getAppTr069PortVODAbendNumbers, NULL);
    Tr069Call* MutiAbendNumbers            = new Tr069FunctionCall("MutiAbendNumbers", getAppTr069PortMultiAbendNumbers, NULL);
    Tr069Call* MultiAbendNumbers            = new Tr069FunctionCall("MultiAbendNumbers", getAppTr069PortMultiAbendNumbers, NULL);
    Tr069Call* PlayErrorInfo               = new Tr069FunctionCall("PlayErrorInfo", getAppTr069PortPlayErrorInfo, NULL);

    Tr069Call* MultiPacketsLostR1Nmb       = new Tr069FunctionCall("MultiPacketsLostR1Nmb", getAppTr069PortMultiPacketsLostR1Nmb, NULL);
    Tr069Call* MultiPacketsLostR2Nmb       = new Tr069FunctionCall("MultiPacketsLostR2Nmb", getAppTr069PortMultiPacketsLostR2Nmb, NULL);
    Tr069Call* MultiPacketsLostR3Nmb       = new Tr069FunctionCall("MultiPacketsLostR3Nmb", getAppTr069PortMultiPacketsLostR3Nmb, NULL);
    Tr069Call* MultiPacketsLostR4Nmb       = new Tr069FunctionCall("MultiPacketsLostR4Nmb", getAppTr069PortMultiPacketsLostR4Nmb, NULL);
    Tr069Call* MultiPacketsLostR5Nmb       = new Tr069FunctionCall("MultiPacketsLostR5Nmb", getAppTr069PortMultiPacketsLostR5Nmb, NULL);

    Tr069Call* VODPacketsLostR1Nmb         = new Tr069FunctionCall("VODPacketsLostR1Nmb", getAppTr069PortVODPacketsLostR1Nmb, NULL);
    Tr069Call* VODPacketsLostR2Nmb         = new Tr069FunctionCall("VODPacketsLostR2Nmb", getAppTr069PortVODPacketsLostR2Nmb, NULL);
    Tr069Call* VODPacketsLostR3Nmb         = new Tr069FunctionCall("VODPacketsLostR3Nmb", getAppTr069PortVODPacketsLostR3Nmb, NULL);
    Tr069Call* VODPacketsLostR4Nmb         = new Tr069FunctionCall("VODPacketsLostR4Nmb", getAppTr069PortVODPacketsLostR4Nmb, NULL);
    Tr069Call* VODPacketsLostR5Nmb         = new Tr069FunctionCall("VODPacketsLostR5Nmb", getAppTr069PortVODPacketsLostR5Nmb, NULL);

    Tr069Call* MultiBitRateR1Nmb           = new Tr069FunctionCall("MultiBitRateR1Nmb", getAppTr069PortMultiBitRateR1Nmb, NULL);
    Tr069Call* MultiBitRateR2Nmb           = new Tr069FunctionCall("MultiBitRateR2Nmb", getAppTr069PortMultiBitRateR2Nmb, NULL);
    Tr069Call* MultiBitRateR3Nmb           = new Tr069FunctionCall("MultiBitRateR3Nmb", getAppTr069PortMultiBitRateR3Nmb, NULL);
    Tr069Call* MultiBitRateR4Nmb           = new Tr069FunctionCall("MultiBitRateR4Nmb", getAppTr069PortMultiBitRateR4Nmb, NULL);
    Tr069Call* MultiBitRateR5Nmb           = new Tr069FunctionCall("MultiBitRateR5Nmb", getAppTr069PortMultiBitRateR5Nmb, NULL);

    Tr069Call* VODBitRateR1Nmb             = new Tr069FunctionCall("VODBitRateR1Nmb", getAppTr069PortVODBitRateR1Nmb, NULL);
    Tr069Call* VODBitRateR2Nmb             = new Tr069FunctionCall("VODBitRateR2Nmb", getAppTr069PortVODBitRateR2Nmb, NULL);
    Tr069Call* VODBitRateR3Nmb             = new Tr069FunctionCall("VODBitRateR3Nmb", getAppTr069PortVODBitRateR3Nmb, NULL);
    Tr069Call* VODBitRateR4Nmb             = new Tr069FunctionCall("VODBitRateR4Nmb", getAppTr069PortVODBitRateR4Nmb, NULL);
    Tr069Call* VODBitRateR5Nmb             = new Tr069FunctionCall("VODBitRateR5Nmb", getAppTr069PortVODBitRateR5Nmb, NULL);
    
    Tr069Call* FramesLostR1Nmb             = new Tr069FunctionCall("FramesLostR1Nmb", getAppTr069PortFramesLostR1Nmb, NULL);
    Tr069Call* FramesLostR2Nmb             = new Tr069FunctionCall("FramesLostR2Nmb", getAppTr069PortFramesLostR2Nmb, NULL);
    Tr069Call* FramesLostR3Nmb             = new Tr069FunctionCall("FramesLostR3Nmb", getAppTr069PortFramesLostR3Nmb, NULL);
    Tr069Call* FramesLostR4Nmb             = new Tr069FunctionCall("FramesLostR4Nmb", getAppTr069PortFramesLostR4Nmb, NULL);
    Tr069Call* FramesLostR5Nmb             = new Tr069FunctionCall("FramesLostR5Nmb", getAppTr069PortFramesLostR5Nmb, NULL);


    regist(PlayErrorNumbers->name(), PlayErrorNumbers);
    regist(VODAbendNumbers->name(),  VODAbendNumbers);
    regist(MutiAbendNumbers->name(), MutiAbendNumbers);
    regist(MultiAbendNumbers->name(), MultiAbendNumbers);
    regist(PlayErrorInfo->name(),    PlayErrorInfo);

    regist(MultiPacketsLostR1Nmb->name(),    MultiPacketsLostR1Nmb);
    regist(MultiPacketsLostR2Nmb->name(),    MultiPacketsLostR2Nmb);
    regist(MultiPacketsLostR3Nmb->name(),    MultiPacketsLostR3Nmb);
    regist(MultiPacketsLostR4Nmb->name(),    MultiPacketsLostR4Nmb);
    regist(MultiPacketsLostR5Nmb->name(),    MultiPacketsLostR5Nmb);
    
    regist(VODPacketsLostR1Nmb->name(),    VODPacketsLostR1Nmb);
    regist(VODPacketsLostR2Nmb->name(),    VODPacketsLostR2Nmb);
    regist(VODPacketsLostR3Nmb->name(),    VODPacketsLostR3Nmb);
    regist(VODPacketsLostR4Nmb->name(),    VODPacketsLostR4Nmb);
    regist(VODPacketsLostR5Nmb->name(),    VODPacketsLostR5Nmb);

    regist(MultiBitRateR1Nmb->name(),    MultiBitRateR1Nmb);
    regist(MultiBitRateR2Nmb->name(),    MultiBitRateR2Nmb);
    regist(MultiBitRateR3Nmb->name(),    MultiBitRateR3Nmb);
    regist(MultiBitRateR4Nmb->name(),    MultiBitRateR4Nmb);
    regist(MultiBitRateR5Nmb->name(),    MultiBitRateR5Nmb);
    
    regist(VODBitRateR1Nmb->name(),    VODBitRateR1Nmb);
    regist(VODBitRateR2Nmb->name(),    VODBitRateR2Nmb);
    regist(VODBitRateR3Nmb->name(),    VODBitRateR3Nmb);
    regist(VODBitRateR4Nmb->name(),    VODBitRateR4Nmb);
    regist(VODBitRateR5Nmb->name(),    VODBitRateR5Nmb);
 
    regist(FramesLostR1Nmb->name(),    FramesLostR1Nmb);
    regist(FramesLostR2Nmb->name(),    FramesLostR2Nmb);
    regist(FramesLostR3Nmb->name(),    FramesLostR3Nmb);
    regist(FramesLostR4Nmb->name(),    FramesLostR4Nmb);
    regist(FramesLostR5Nmb->name(),    FramesLostR5Nmb);
   

    //CTC独有
    Tr069Call* MultiRRT                    = new Tr069FunctionCall("MultiRRT", getAppTr069PortMultiRRT, NULL);
    Tr069Call* VodRRT                      = new Tr069FunctionCall("VodRRT", getAppTr069PortVodRRT, NULL);
    Tr069Call* HTTPRRT                     = new Tr069FunctionCall("HTTPRRT", getAppTr069PortHTTPRRT, NULL);
    Tr069Call* MultiAbendUPNumbers         = new Tr069FunctionCall("MultiAbendUPNumbers", getAppTr069PortMultiAbendUPNumbers, NULL);
    Tr069Call* VODUPAbendNumbers           = new Tr069FunctionCall("VODUPAbendNumbers", getAppTr069PortVODUPAbendNumbers, NULL);
    Tr069Call* HD_MultiAbendNumbers        = new Tr069FunctionCall("HD_MultiAbendNumbers", getAppTr069PortHDMultiAbendNumbers, NULL);
    Tr069Call* HD_VODAbendNumbers          = new Tr069FunctionCall("HD_VODAbendNumbers", getAppTr069PortHDVODAbendNumbers, NULL);
    Tr069Call* HD_MultiUPAbendNumbers      = new Tr069FunctionCall("HD_MultiUPAbendNumbers", getAppTr069PortHDMultiUPAbendNumbers, NULL);
    Tr069Call* HD_VODUPAbendNumbers        = new Tr069FunctionCall("HD_VODUPAbendNumbers", getAppTr069PortHDVODUPAbendNumbers, NULL);

    Tr069Call* FECMultiPacketsLostR1Nmb    = new Tr069FunctionCall("FECMultiPacketsLostR1Nmb", getAppTr069PortFECMultiPacketsLostR1Nmb, NULL);
    Tr069Call* FECMultiPacketsLostR2Nmb    = new Tr069FunctionCall("FECMultiPacketsLostR2Nmb", getAppTr069PortFECMultiPacketsLostR2Nmb, NULL);
    Tr069Call* FECMultiPacketsLostR3Nmb    = new Tr069FunctionCall("FECMultiPacketsLostR3Nmb", getAppTr069PortFECMultiPacketsLostR3Nmb, NULL);
    Tr069Call* FECMultiPacketsLostR4Nmb    = new Tr069FunctionCall("FECMultiPacketsLostR4Nmb", getAppTr069PortFECMultiPacketsLostR4Nmb, NULL);
    Tr069Call* FECMultiPacketsLostR5Nmb    = new Tr069FunctionCall("FECMultiPacketsLostR5Nmb", getAppTr069PortFECMultiPacketsLostR5Nmb, NULL);

    Tr069Call* ARQVODPacketsLostR1Nmb      = new Tr069FunctionCall("ARQVODPacketsLostR1Nmb", getAppTr069PortARQVODPacketsLostR1Nmb, NULL);
    Tr069Call* ARQVODPacketsLostR2Nmb      = new Tr069FunctionCall("ARQVODPacketsLostR2Nmb", getAppTr069PortARQVODPacketsLostR2Nmb, NULL);
    Tr069Call* ARQVODPacketsLostR3Nmb      = new Tr069FunctionCall("ARQVODPacketsLostR3Nmb", getAppTr069PortARQVODPacketsLostR3Nmb, NULL);
    Tr069Call* ARQVODPacketsLostR4Nmb      = new Tr069FunctionCall("ARQVODPacketsLostR4Nmb", getAppTr069PortARQVODPacketsLostR4Nmb, NULL);
    Tr069Call* ARQVODPacketsLostR5Nmb      = new Tr069FunctionCall("ARQVODPacketsLostR5Nmb", getAppTr069PortARQVODPacketsLostR5Nmb, NULL);

    Tr069Call* HD_MultiPacketsLostR1Nmb    = new Tr069FunctionCall("HD_MultiPacketsLostR1Nmb", getAppTr069PortHDMultiPacketsLostR1Nmb, NULL);
    Tr069Call* HD_MultiPacketsLostR2Nmb    = new Tr069FunctionCall("HD_MultiPacketsLostR2Nmb", getAppTr069PortHDMultiPacketsLostR2Nmb, NULL);
    Tr069Call* HD_MultiPacketsLostR3Nmb    = new Tr069FunctionCall("HD_MultiPacketsLostR3Nmb", getAppTr069PortHDMultiPacketsLostR3Nmb, NULL);
    Tr069Call* HD_MultiPacketsLostR4Nmb    = new Tr069FunctionCall("HD_MultiPacketsLostR4Nmb", getAppTr069PortHDMultiPacketsLostR4Nmb, NULL);
    Tr069Call* HD_MultiPacketsLostR5Nmb    = new Tr069FunctionCall("HD_MultiPacketsLostR5Nmb", getAppTr069PortHDMultiPacketsLostR5Nmb, NULL);
    
    Tr069Call* HD_FECMultiPacketsLostR1Nmb = new Tr069FunctionCall("HD_FECMultiPacketsLostR1Nmb", getAppTr069PortHDFECMultiPacketsLostR1Nmb, NULL);
    Tr069Call* HD_FECMultiPacketsLostR2Nmb = new Tr069FunctionCall("HD_FECMultiPacketsLostR2Nmb", getAppTr069PortHDFECMultiPacketsLostR2Nmb, NULL);
    Tr069Call* HD_FECMultiPacketsLostR3Nmb = new Tr069FunctionCall("HD_FECMultiPacketsLostR3Nmb", getAppTr069PortHDFECMultiPacketsLostR3Nmb, NULL);
    Tr069Call* HD_FECMultiPacketsLostR4Nmb = new Tr069FunctionCall("HD_FECMultiPacketsLostR4Nmb", getAppTr069PortHDFECMultiPacketsLostR4Nmb, NULL);
    Tr069Call* HD_FECMultiPacketsLostR5Nmb = new Tr069FunctionCall("HD_FECMultiPacketsLostR5Nmb", getAppTr069PortHDFECMultiPacketsLostR5Nmb, NULL);

    Tr069Call* HD_VODPacketsLostR1Nmb      = new Tr069FunctionCall("HD_VODPacketsLostR1Nmb", getAppTr069PortHDVODPacketsLostR1Nmb, NULL);
    Tr069Call* HD_VODPacketsLostR2Nmb      = new Tr069FunctionCall("HD_VODPacketsLostR2Nmb", getAppTr069PortHDVODPacketsLostR2Nmb, NULL);
    Tr069Call* HD_VODPacketsLostR3Nmb      = new Tr069FunctionCall("HD_VODPacketsLostR3Nmb", getAppTr069PortHDVODPacketsLostR3Nmb, NULL);
    Tr069Call* HD_VODPacketsLostR4Nmb      = new Tr069FunctionCall("HD_VODPacketsLostR4Nmb", getAppTr069PortHDVODPacketsLostR4Nmb, NULL);
    Tr069Call* HD_VODPacketsLostR5Nmb      = new Tr069FunctionCall("HD_VODPacketsLostR5Nmb", getAppTr069PortHDVODPacketsLostR5Nmb, NULL);

    Tr069Call* HD_ARQVODPacketsLostR1Nmb   = new Tr069FunctionCall("HD_ARQVODPacketsLostR1Nmb", getAppTr069PortHDARQVODPacketsLostR1Nmb, NULL);
    Tr069Call* HD_ARQVODPacketsLostR2Nmb   = new Tr069FunctionCall("HD_ARQVODPacketsLostR2Nmb", getAppTr069PortHDARQVODPacketsLostR2Nmb, NULL);
    Tr069Call* HD_ARQVODPacketsLostR3Nmb   = new Tr069FunctionCall("HD_ARQVODPacketsLostR3Nmb", getAppTr069PortHDARQVODPacketsLostR3Nmb, NULL);
    Tr069Call* HD_ARQVODPacketsLostR4Nmb   = new Tr069FunctionCall("HD_ARQVODPacketsLostR4Nmb", getAppTr069PortHDARQVODPacketsLostR4Nmb, NULL);
    Tr069Call* HD_ARQVODPacketsLostR5Nmb   = new Tr069FunctionCall("HD_ARQVODPacketsLostR5Nmb", getAppTr069PortHDARQVODPacketsLostR5Nmb, NULL);

    Tr069Call* HD_VODBitRateR1Nmb          = new Tr069FunctionCall("HD_VODBitRateR1Nmb", getAppTr069PortHDVODBitRateR1Nmb, NULL);
    Tr069Call* HD_VODBitRateR2Nmb          = new Tr069FunctionCall("HD_VODBitRateR2Nmb", getAppTr069PortHDVODBitRateR2Nmb, NULL);
    Tr069Call* HD_VODBitRateR3Nmb          = new Tr069FunctionCall("HD_VODBitRateR3Nmb", getAppTr069PortHDVODBitRateR3Nmb, NULL);
    Tr069Call* HD_VODBitRateR4Nmb          = new Tr069FunctionCall("HD_VODBitRateR4Nmb", getAppTr069PortHDVODBitRateR4Nmb, NULL);
    Tr069Call* HD_VODBitRateR5Nmb          = new Tr069FunctionCall("HD_VODBitRateR5Nmb", getAppTr069PortHDVODBitRateR5Nmb, NULL);

    Tr069Call* HD_MultiBitRateR1Nmb        = new Tr069FunctionCall("HD_MultiBitRateR1Nmb", getAppTr069PortHDMultiBitRateR1Nmb, NULL);
    Tr069Call* HD_MultiBitRateR2Nmb        = new Tr069FunctionCall("HD_MultiBitRateR2Nmb", getAppTr069PortHDMultiBitRateR2Nmb, NULL);
    Tr069Call* HD_MultiBitRateR3Nmb        = new Tr069FunctionCall("HD_MultiBitRateR3Nmb", getAppTr069PortHDMultiBitRateR3Nmb, NULL);
    Tr069Call* HD_MultiBitRateR4Nmb        = new Tr069FunctionCall("HD_MultiBitRateR4Nmb", getAppTr069PortHDMultiBitRateR4Nmb, NULL);
    Tr069Call* HD_MultiBitRateR5Nmb        = new Tr069FunctionCall("HD_MultiBitRateR5Nmb", getAppTr069PortHDMultiBitRateR5Nmb, NULL);
     
    Tr069Call* BufferIncNmb                = new Tr069FunctionCall("BufferIncNmb", getAppTr069PortBufferIncNmb, NULL);
    Tr069Call* BufferDecNmb                = new Tr069FunctionCall("BufferDecNmb", getAppTr069PortBufferDecNmb, NULL);


    regist(MultiRRT->name(),    MultiRRT);
    regist(VodRRT->name(),    VodRRT);
    regist(HTTPRRT->name(),    HTTPRRT);
    regist(MultiAbendUPNumbers->name(),    MultiAbendUPNumbers);
    regist(VODUPAbendNumbers->name(),    VODUPAbendNumbers);
    regist(HD_MultiAbendNumbers->name(),    HD_MultiAbendNumbers);
    regist(HD_VODAbendNumbers->name(),    HD_VODAbendNumbers);
    regist(HD_MultiUPAbendNumbers->name(),    HD_MultiUPAbendNumbers);
    regist(HD_VODUPAbendNumbers->name(),    HD_VODUPAbendNumbers);

    regist(FECMultiPacketsLostR1Nmb->name(),    FECMultiPacketsLostR1Nmb);
    regist(FECMultiPacketsLostR2Nmb->name(),    FECMultiPacketsLostR2Nmb);
    regist(FECMultiPacketsLostR3Nmb->name(),    FECMultiPacketsLostR3Nmb);
    regist(FECMultiPacketsLostR4Nmb->name(),    FECMultiPacketsLostR4Nmb);
    regist(FECMultiPacketsLostR5Nmb->name(),    FECMultiPacketsLostR5Nmb);

    regist(ARQVODPacketsLostR1Nmb->name(),    ARQVODPacketsLostR1Nmb);
    regist(ARQVODPacketsLostR2Nmb->name(),    ARQVODPacketsLostR2Nmb);
    regist(ARQVODPacketsLostR3Nmb->name(),    ARQVODPacketsLostR3Nmb);
    regist(ARQVODPacketsLostR4Nmb->name(),    ARQVODPacketsLostR4Nmb);
    regist(ARQVODPacketsLostR5Nmb->name(),    ARQVODPacketsLostR5Nmb);

    regist(HD_MultiPacketsLostR1Nmb->name(),    HD_MultiPacketsLostR1Nmb);
    regist(HD_MultiPacketsLostR2Nmb->name(),    HD_MultiPacketsLostR2Nmb);
    regist(HD_MultiPacketsLostR3Nmb->name(),    HD_MultiPacketsLostR3Nmb);
    regist(HD_MultiPacketsLostR4Nmb->name(),    HD_MultiPacketsLostR4Nmb);
    regist(HD_MultiPacketsLostR5Nmb->name(),    HD_MultiPacketsLostR5Nmb);

    regist(HD_FECMultiPacketsLostR1Nmb->name(),    HD_FECMultiPacketsLostR1Nmb);
    regist(HD_FECMultiPacketsLostR2Nmb->name(),    HD_FECMultiPacketsLostR2Nmb);
    regist(HD_FECMultiPacketsLostR3Nmb->name(),    HD_FECMultiPacketsLostR3Nmb);
    regist(HD_FECMultiPacketsLostR4Nmb->name(),    HD_FECMultiPacketsLostR4Nmb);
    regist(HD_FECMultiPacketsLostR5Nmb->name(),    HD_FECMultiPacketsLostR5Nmb);

    regist(HD_VODPacketsLostR1Nmb->name(),    HD_VODPacketsLostR1Nmb);
    regist(HD_VODPacketsLostR2Nmb->name(),    HD_VODPacketsLostR2Nmb);
    regist(HD_VODPacketsLostR3Nmb->name(),    HD_VODPacketsLostR3Nmb);
    regist(HD_VODPacketsLostR4Nmb->name(),    HD_VODPacketsLostR4Nmb);
    regist(HD_VODPacketsLostR5Nmb->name(),    HD_VODPacketsLostR5Nmb);

    regist(HD_ARQVODPacketsLostR1Nmb->name(),    HD_ARQVODPacketsLostR1Nmb);
    regist(HD_ARQVODPacketsLostR2Nmb->name(),    HD_ARQVODPacketsLostR2Nmb);
    regist(HD_ARQVODPacketsLostR3Nmb->name(),    HD_ARQVODPacketsLostR3Nmb);
    regist(HD_ARQVODPacketsLostR4Nmb->name(),    HD_ARQVODPacketsLostR4Nmb);
    regist(HD_ARQVODPacketsLostR5Nmb->name(),    HD_ARQVODPacketsLostR5Nmb);

    regist(HD_VODBitRateR1Nmb->name(),    HD_VODBitRateR1Nmb);
    regist(HD_VODBitRateR2Nmb->name(),    HD_VODBitRateR2Nmb);
    regist(HD_VODBitRateR3Nmb->name(),    HD_VODBitRateR3Nmb);
    regist(HD_VODBitRateR4Nmb->name(),    HD_VODBitRateR4Nmb);
    regist(HD_VODBitRateR5Nmb->name(),    HD_VODBitRateR5Nmb);

    regist(HD_MultiBitRateR1Nmb->name(),    HD_MultiBitRateR1Nmb);
    regist(HD_MultiBitRateR2Nmb->name(),    HD_MultiBitRateR2Nmb);
    regist(HD_MultiBitRateR3Nmb->name(),    HD_MultiBitRateR3Nmb);
    regist(HD_MultiBitRateR4Nmb->name(),    HD_MultiBitRateR4Nmb);
    regist(HD_MultiBitRateR5Nmb->name(),    HD_MultiBitRateR5Nmb);

    regist(BufferIncNmb->name(),    BufferIncNmb);
    regist(BufferDecNmb->name(),    BufferDecNmb);


    //HW独有
    Tr069Call* BootUpTime         = new Tr069FunctionCall("BootUpTime", getAppTr069PortBootUpTime, NULL);
    Tr069Call* PowerOnTime        = new Tr069FunctionCall("PowerOnTime", getAppTr069PortPowerTime, NULL);
    
    regist(BootUpTime->name(),     BootUpTime);
    regist(PowerOnTime->name(),    PowerOnTime);

    
}

Tr069ServiceStatistics::~Tr069ServiceStatistics()
{
}