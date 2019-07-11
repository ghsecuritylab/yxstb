  /*******************************************************************************
Copyright (C), 2009, Huawei Tech. Co., Ltd.
File name: hwvde.h
Author:    y00135850
Version:   Ver 1.0
Date:	   2009-02-23
Description:
    HW VDE(video delivery enhancement),including FCC & ARQ & FEC. The STB_INT32erface to the client.
Others:

Function List:


History:
2009-02-23   y00135850 creat
2011-07-21   liujunjie changed
*******************************************************************************/


#ifndef __HWVDE_H__
#define __HWVDE_H__

#include "SqaAssertions.h"

#include "fcc_ret.h"
//#include "libzebra.h"
#include <time.h>
#ifdef __cplusplus
extern "C"
{
#endif
/************************************************************************/
/* FCC模块宏定义                                                      */
/************************************************************************/

/************************************************************************/
/* 功能选择宏定义（功能代码按位表示*/
/************************************************************************/

#ifndef FCC         // FCC
#define FCC_CODE 0x08        //4
#endif
#ifndef FEC         // FEC
#define FEC_CODE 0x04        //4
#endif
#ifndef RET         // ARQ
#define RET_CODE 0x02
#endif
#ifndef HYBRID         // Hybrid
#define HYBRID 0x06
#endif
#ifndef NOER         // 仅缓存
#define NOER 0x00
#endif
#ifndef CTC        //电信RET
#define CTC 0x01
#endif
#ifndef HW        //HW规范RET
#define HW 0x02
#endif
#ifndef IPV4        //IPV4
#define IPV4 0x01
#endif
#ifndef IPV6        //IPV6
#define IPV6 0x02
#endif
#ifndef NORTP           //仅缓存(无RTP头)
#define NORTP 0x16
#endif


typedef enum
{
      LEVEL_DEBUG = 0,    /* 调试信息 */
      LEVEL_INFO,         /* 重要信息 */
      LEVEL_WARNING,    /* 警告信息 */
      LEVEL_ERROR,        /* 错误信息 */
      LEVEL_NONE          /* 无日志   */
}E_TraceLevel;


/***********************************
  加入组播组类型
  **********************************/
  #define IGMP_ORIGINAL 0
  #define IGMP_FEC_LAYER1 1
  #define IGMP_FEC_LAYER2 2

typedef enum
{
  SQA_BUFFER_LEVEL_HIGH,// 高缓冲警告
  SQA_BUFFER_LEVEL_LOW, // 低缓冲警告
  SQA_BUFFER_LEVEL_RECONNECT, //重连警告
  SQA_BUFFER_LEVEL_OVERFLOW   //水位溢出警告
}DataBufferLevel;

typedef enum
{
  FCC_REJECT = 0,
  FCC_NOUNICAST,
  FCC_RESPONSE_TIMEOUT,
  FCC_SUCCESS,
  FCC_DOWN
}FccStateRsp;

typedef enum
{
  HMS_TIMEOUT = 0,
  HMW_SUCCESS
}HmsStateRsp;

typedef enum SQA_CONTENT_TYPE
{
    RTP_NORMAL = 0,
    RTP_RET_RESUME = 1,
    RTP_FEC_RESUME = 2
}SQA_CONTENT_TYPE;

typedef enum
{
	SQA_GETIP_STBIP,                  //获取机顶盒IP
	SQA_GETIP_MULTGROUPIP, //获取组播组IP,在单播情况不用填写
	SQA_GETIP_MULTSRCIP       //获取组播源IP,V3版本使用，在单播和V2下不用填写
}GetIPType;


typedef enum
{
  FCC_SIGNALING = 0,             //FCC信令IP
  FCC_DATA  = 1                  //FCC数据IP
}FccIpType;



/************************************************************************/
/* 以下是机顶盒提供的回调函数定义                                       */
/************************************************************************/

/*******************************************************************************
Description:   客户端回调函数：发送FCC请求报文
Calls By:       SQA Module
Input:           hClient 机顶盒标识, rtcpdata 报文数据，rtcp_len 报文长度
Output:         none
Return:         void
*******************************************************************************/
typedef void funcCallBackSendFCC(STB_HANDLE hClient, STB_UINT8 *rtcp_data, STB_UINT32 rtcp_len);

/*******************************************************************************
Description:   客户端回调函数：发送丢包重传请求报文
Calls By:       SQA Module
Input:            hClient 机顶盒标识, rtcpdata 报文数据，rtcp_len 报文长度
Output:         none
Return:         void
*******************************************************************************/
typedef void funcCallBackSendRET(STB_HANDLE hClient, STB_UINT8 *ret_data, STB_UINT32 data_len);


/*******************************************************************************
Description:    客户端回调函数：发送IGMP报文，快速频道切换使用
Calls By:       SQA Module
Input:           fnClientHandle, IGMPJoinType(0:原始组播组,1:FECLayer1组播组,2:FECLayer2组播组)
Output:         none
Return:         int
*******************************************************************************/

typedef void funcCallBackIGMPJoin(STB_HANDLE hClient, STB_INT32 igmp_join_type);


/*******************************************************************************
Description:    客户端回调函数：分配内存，为不同操作系统移植封装
Calls By:       SQA Module
Input:           fnClientHandle, len: Bytes to allocate
Output:         none
Return:         a void pointer to the allocated space or NULL if there is insufficient memory available
*******************************************************************************/
typedef void * funcCallBackGetMemory(STB_HANDLE hClient, STB_UINT32 len);


/*******************************************************************************
Description:    客户端回调函数：释放内存，为不同操作系统移植封装
Calls By:       SQA Module
Input:           fnClientHandle,memPtr: Previously allocated memory block to be freed
Output:         none
Return:         void
*******************************************************************************/
typedef void funcCallBackFreeMemory(STB_HANDLE hClient, STB_HANDLE mem_addr);


/*******************************************************************************
Description:    客户端回调函数：获取系统精度时钟(1/1000)，为不同操作系统移植封装
Calls By:       SQA Module
Input:           fnClientHandle, timeval struct point
Output:         system time in timeval struct
Return:         无
*******************************************************************************/
typedef 	STB_UINT32 funcCallBackGetTime(void);


/*******************************************************************************
Description:    客户端回调函数：输出SQA日志信息
Calls By:       SQA Module
Input:
		      eLevel            日志级别
		      pszModule      所在模块
		      format            格式串

Output:         无
Return:         无
*******************************************************************************/
typedef void funcCallBackTraceOut(E_TraceLevel eLevel, const STB_INT8* pszModule, const STB_INT8* format, ...);

/*******************************************************************************
Description:    客户端回调函数：获取机顶盒IP地址
Calls By:       HWERModule
Input:           hClient 机顶盒标识, ip_type获取IP类型
 Output:        buf：输出IP地址，字符串形式（例如：IPV4：100.1.20.247）
ilength：实际的IP字符串长度，不用填写时长度为0
Return:          如果成功返回STB_OK;否则返回STB_ERROR.

*******************************************************************************/
typedef STB_INT32 funcCallBackGetIP(STB_HANDLE hClient , GetIPType ip_type,STB_INT8*buf,STB_INT32 *ilength );

/*******************************************************************************
Description:    客户端回调函数： SQA缓冲区警告级别变化通知 by matchbox
Calls By:       HWERModule
Input:           fnClientHandle
Output:         E_DataBufferLevel 缓冲区警告级别
Return:         void
*******************************************************************************/
typedef void funcCallBackDataBufferLevel(STB_HANDLE hClient, DataBufferLevel data_buffer_level);

/*******************************************************************************
Description:     客户端回调函数： FCC服务器IP通知
Calls By:        HWERModule
Input:           无
Output:          server_addr: FCC服务器IP地址
                 ip_type:     FCC服务器IP地址类型
Return:          void
*******************************************************************************/
typedef void funcCallBackFccIP(STB_HANDLE hClient,STB_UINT32 server_addr, FccIpType ip_type);

/*******************************************************************************
Description:     客户端回调函数： FCC服务器端口通知
Calls By:        HWERModule
Input:           无
Output:          port:        FCC服务器的发包端口
Return:          void
*******************************************************************************/
typedef void funcCallBackFccPort(STB_HANDLE hClient,STB_UINT32 port);

/*******************************************************************************
Description:     客户端回调函数： FCC路由表超时时间通知
Calls By:        HWERModule
Input:           无
Output:          ValidTime:       通知FCC路由超时时间
Return:          void
*******************************************************************************/
typedef void funcCallBackValidTime(STB_HANDLE hClient,STB_INT32 ValidTime);


/*******************************************************************************
Description:     客户端回调函数： FCC server响应状态通知
Calls By:        HWERModule
Input:           无
Output:          FccStateRsp:fcc请求结果通知
Return:          void
*******************************************************************************/
typedef void funcCallBackFccStateRsp(STB_HANDLE hClient, FccStateRsp fcc_state_response);

/******************************************************************************
Description:   客户端回调函数：发送NAT心跳和地址更新请求报文
Calls By:       SQA Module
Input:           hClient 机顶盒标识, nat_data 报文数据，data_len 报文长度
Output:         none
Return:         void
*******************************************************************************/
typedef void funcCallBackSendNatData(STB_HANDLE hClient,STB_UINT8 *nat_data, STB_UINT32 data_len);

/*******************************************************************************
Description:     客户端回调函数： FCC server响应状态通知
Calls By:        HWERModule
Input:           无
Output:          FccStateRsp:fcc请求结果通知
Return:          void
*******************************************************************************/
typedef void funcCallBackHmsRsp(STB_HANDLE hClient, HmsStateRsp hms_state_response);

/* 机顶盒注册回调函数*/
/*依据C58规范,重新定义回填函数接口 ----sqa_init  使用的回调函数*/
typedef struct _CallBackFuns
{
    STB_HANDLE hClient;                               //机顶盒客户端标识
    funcCallBackGetMemory* fcbGetMemory;              //分配内存
    funcCallBackFreeMemory * fcbFreeMemory;           //释放内存
    funcCallBackGetTime* fcbGetTime;	              //获取机顶盒时间
    funcCallBackTraceOut *fcbTraceOut;                //输出日志信息
}CallBackFuns;

/*依据C58规范,重新定义回填函数接口 ----sqa_set_parameter  使用的回调函数*/
typedef struct _SQACallBackFuns
{
    STB_HANDLE hClient;                               //机顶盒客户端标识
    funcCallBackSendFCC *fcbSendFCC;                  //发送FCC报文
    funcCallBackSendRET *fcbSendRET;                  //发送丢包重传请求
    funcCallBackSendNatData* fcbSendNAT;              //发送NAT心跳和地址更新报文（NAT方案特有接口）
    funcCallBackIGMPJoin *fcbIGMPJoin;                //发送加入组播组请求
    funcCallBackDataBufferLevel *fcbDataBufferLevel;  //通知水位高低
    funcCallBackFccStateRsp*fcbFccStateRsp;          //通知FCC server响应
    funcCallBackFccIP *fcbFccIP;                      //通知FCC的IP
    funcCallBackFccPort *fcbFCCPort;                  //通知FCC的端口号
    funcCallBackValidTime *fcbValidTime;              //通知ValidTime时间
    funcCallBackHmsRsp *fcbhmsRsp;                    //点播时通知地址报文更新消息响应状态
    funcCallBackGetIP *fcbGetIP;                      //获取机顶盒IP
}SQACallBackFuns;

typedef struct _SQAParam
{
	STB_UINT32 nMemorySize;          //指定SQA数据缓冲区大小，单位Byte
	STB_UINT32 nFastCachePeriod;   //通知STB应用启/停快速缓冲的最小时间间隔
	STB_INT32 iCacheFirst;               //是否优先缓冲SQA CBB 1:优先缓冲SQA,2:优先缓冲decode
	STB_INT32 iBufferLevel[3];         //SQA CBB配置高、中、低水位
	STB_INT32 iRETSendPeriod;       //SQA CBB配置多次重传间隔
	STB_INT32 iRSRTimeOut;           //等待FCC响应超时时间，单位毫秒
       STB_INT32 iSCNTimeOut;           //等待SCN消息超时时间，单位毫秒
}SQAParam;


typedef struct _StreamParam
{
	STB_INT8 cType;                          //SQA支持功能配置参数:
	STB_INT8 cRetType;                    //支持RET兼容的配置参数 CTC：电信标准 HW：华为标准
	STB_UINT16 port;                        //机顶盒接收FCC服务器回复报文端口
	//STB_UINT16 iRetPort;          //临时版本，填写发送重传信令的本地端口号
	STB_INT8 cIPType;                     //IPV4或者ipv6格式
	STB_INT32 iSendRSRFlag;           //不发送:0 发送: 1
	STB_INT32 iFCCWithTLV;            //FCC信令交互是否采用TLV格式 1:采用 0：不采用
	STB_INT32 iFastCacheOn;          //是否开启快速缓冲标示，1：开启 0：不开启
	STB_INT32  iChanCacheTime;     //频道缓存时长,单位ms
	STB_INT32  iRetDelayTime;     //配置第一次重传延时(应对抖动)
	STB_INT32  iRetInterval;      //多次重传时间间隔的最小值
    	STB_INT32  iFCCServerFlag;                        //通知CBB机顶盒是否知道FCC地址 0:不知道 1:知道
	STB_INT32  iNatSupport;                           //配置是否支持NAT穿越1：支持 0：不支持
	STB_INT32  iNatHBinterval;                        //Nat心跳时间间隔
	STB_UINT32 iclientSSRC;
	STB_INT32  iDisorderTime;                         //非RET和FEC条件下支持乱序时间
}StreamParam;


/*******************************************************************************
Function Name:  sqa_init
Description:       SQA模块初时化
Calls By:           STB Client
Input:          yx 回调函数集指针，
                STB系统参数例如SSRC等，
                HW功能选择（FEC/ARQ/Fast/FCC）（宏定义见上）
Output:         none
Return:
                     hSqa        SQA上下文句柄

*******************************************************************************/
STB_HANDLE  sqa_init(CallBackFuns* cbfs,SQAParam *args);

/*******************************************************************************
Function Name:       sqa_set_parameter
Description:         设置媒体相关参数信息
Calls By:            STB Client
Input:               sqa_handle        SQA上下文句柄
                        args              输入参数，指向机顶盒传入参数结构体
                        cbfs		 回调函数
Output:              无
Return:              成功返回STB_OK;
			         否则返回STB_ERROR.
*******************************************************************************/

STB_INT32 sqa_set_parameter(STB_HANDLE sqa_handle,StreamParam *args,SQACallBackFuns *cbfs);
/*******************************************************************************
Function Name: sqa_destroy
Description:    SQA模块卸载
Calls By:       STB Client
Input:
                     hSqa        SQA上下文句柄

Output:         none
Return:         void
*******************************************************************************/
void  sqa_destroy(STB_HANDLE sqa_handle);


/*******************************************************************************
Function Name: sqa_recv_rtcp_packet
Description:    hwvde回调函数：通知SQA模块处理RTCP报文
Calls By:       STB Client
Input:
                     hSqa        SQA上下文句柄
                	pRtcpPacket :  RTCP 报文
                	ulPacketLen :  RTCP 报文大小
Output:         none
Return:         0 if success or false
*******************************************************************************/
STB_INT32 sqa_recv_rtcp_packet(STB_HANDLE sqa_handle, void *buffer, STB_UINT32 packet_len);


/*******************************************************************************
Function Name: sqa_recv_multicast_rtp_packet
Description:    hwvde回调函数：通知SQA模块处理组播RTP报文
Calls By:       STB Client
Input:
                     hSqa        SQA上下文句柄
                	pRtcpPacket :  RTP 报文
                	ulPacketLen :  RTP 报文大小
Output:         none
Return:         0 if success or false
*******************************************************************************/
STB_INT32 sqa_recv_multicast_rtp_packet(STB_HANDLE sqa_handle, void * buffer, STB_UINT32 packet_len);


/*******************************************************************************
Function Name: sqa_recv_unicast_rtp_packet
Description:    hwvde回调函数：通知SQA模块处理FCC单播RTP报文
Calls By:      STB Client
Input:          hSqa: SQA组件句柄
                pRtcpPacket :  RTP 报文
                ulPacketLen :  RTP 报文大小
Output:         none
Return:         0 if success or false
*******************************************************************************/
STB_INT32 sqa_recv_unicast_rtp_packet(STB_HANDLE sqa_handle, void *buffer,STB_UINT32 packet_len);


/*************************************************************************************************************
Function Name:      sqa_handle_event
Description:        机顶盒调用该接口去激活SQA的RET功能
Calls By:           STB Client

Input:              sqa_handle          SQA上下文句柄

Output:             无

Return:             成功返回STB_OK;
                    否则返回STB_ERROR.
****************************************************************************************************************/
void sqa_handle_event(STB_HANDLE sqa_handle);


/*************************************************************************************************************
Function Name:      sqa_get_rtp_packet
Description:        机顶盒调用该接口从SQA获取播放的RTP数据包
Calls By:           STB Client

Return:             成功返回STB_OK;
			        否则返回STB_ERROR.
****************************************************************************************************************/
STB_INT32 sqa_get_rtp_packet(STB_HANDLE sqa_handle, void **buffer, STB_UINT32 *length, STB_INT32 idecoder_data_size);

/*************************************************************************************************************
Function Name:      sqa_get_rtp_packet
Description:        机顶盒调用该接口从SQA获取内存单元，用于收取RTP包和RTCP信令
Calls By:           STB Client

Input:
                    sqa_handle        SQA上下文句柄
			        buffer            获取到的内存单元地址
			        length            内存单元长度

Output:             无
Return:
			        成功返回STB_OK;
			        否则返回STB_ERROR.
****************************************************************************************************************/
STB_INT32 sqa_get_buf(STB_HANDLE sqa_handle, void **buffer,STB_INT32 *length);


/*************************************************************************************************************
Function Name:   sqa_get_rtp_packet
Description:        机顶盒调用该接口重置SQA缓冲区
Calls By:            STB Client

Input:
                     hSqa        SQA上下文句柄

Output:
                     无
Return:
			成功返回STB_OK;
			否则返回STB_ERROR.
****************************************************************************************************************/
void sqa_reset_buf(STB_HANDLE sqa_handle);
/*************************************************************************************************************
Function Name:   sqa_get_rtp_packet
Description:        机顶盒调用该接口从SQA获取播放的RTP数据包
Calls By:              STB Client

Input:
                     hSqa        SQA上下文句柄
			pData	  sqa_get_rtp_packet 调用成功时返回的SQA缓存数据包指针

Output:
                      无
Return:
			成功返回STB_OK;
			否则返回STB_ERROR.
****************************************************************************************************************/
STB_INT32 sqa_free_rtp_packet(STB_HANDLE sqa_handle, void *buffer);



typedef enum
{
    FAST_CACHE_ON,   //快速缓冲开启
    FAST_CACHE_OFF   //快速缓冲关闭
}FastCacheState;     //缓冲区警告级别

/*************************************************************************************************************
Function Name:      sqa_set_fastcache_state
Description:        机顶盒调用该接口设置当前快速缓冲状态
Calls By:           STB Client

Input:              sqa_handle          SQA上下文句柄
                    fastcache_state     快速缓冲状态
Output:             无
Return:             无
****************************************************************************************************************/
void sqa_set_fastcache_state(STB_HANDLE sqa_handle, FastCacheState fastcache_state);
/***************FCC******************/
/*************************************************************************************************************
Function Name:      sqa_get_buffer_state
Description:        机顶盒调用该接口获取当前SQA缓冲区占用率
Calls By:           STB Client

Input:              sqa_handle          SQA上下文句柄

Output:             无
Return:             成功:返回当前缓冲区占用率(0-100)
                    失败:STB_ERROR
****************************************************************************************************************/
STB_INT32 sqa_get_buffer_state(STB_HANDLE sqa_handle);

#ifdef ANDROID
#else
/***************  sqa_get_err_info  ***************/
typedef struct _SQAErrInfo
{
   STB_UINT32 uiLostNum;          //丢包数量
   STB_UINT32 uiDisordNum;      //乱序包数量
   STB_UINT32 uiPCRBitrate;      //PCR 码率
   STB_UINT32 uiLocalBitrate;    //实时码率
} SQAErrInfo;

STB_INT32   sqa_get_err_info(STB_HANDLE sqa_handle, SQAErrInfo *pBuf);

typedef enum
{
    LOSTNUM,
    DISORDNUM,
    PCRBITRATE,
    LOCALBITRATE
}ERROR_INFO_TYPE;

int get_sqa_errInfo(int errInfoType);
#endif

/***************FCC******************/

typedef enum
{
  FCC_SERVER_RRS =1,
  FCC_SERVER_FCC
}FCC_SERVER_TYPE;

typedef STB_INT32 funFccRecPacket(STB_HANDLE hSqa, void *pData,STB_UINT32 ulPacketLen);
STB_HANDLE yx_FCC_VDE_init(struct FCC* hSTB,FCC_SERVER_TYPE server_type);
void yx_SQA_VDE_handleEvent(STB_HANDLE hSqa);
int yx_SQA_VDE_recvRtcpPacket(STB_HANDLE hSqa, void *pData, STB_UINT32 ulPacketLen);
int yx_SQA_VDE_Destroy(STB_HANDLE hSqa);

void yx_FCC_NatInterval_set (int Interval_time);

int yx_FCC_NatInterval_get (void);

void yx_FCC_DisorderTime_set (int DisorderTime);

int yx_FCC_DisorderTime_get (void);

/***************RET*******************/
STB_HANDLE yx_RET_VDE_init(struct RET* hSTB,FCC_SERVER_TYPE server_type);
STB_HANDLE yx_ARQ_VDE_init(struct RET* hSTB);

STB_HANDLE yx_RTCP_VDE_init(struct RET* hSTB);

#ifdef __cplusplus
}
#endif
#endif

