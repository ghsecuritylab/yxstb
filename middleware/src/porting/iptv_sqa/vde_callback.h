/******************************************************************
Yuxing FCC
vde_callback.h
Author:	Liuhongwei
2009/3/12 15:29:36
changed by liujunjie
2011/7/21 17:10:52
******************************************************************/
#ifndef __VDE_CALLBACK_H_
#define __VDE_CALLBACK_H_

#define VDE_GetMemory_PRINT		0
#define VDE_GetTime_PRINT		0

#include "vde.h"

//FCC
void 	yx_funcCallBackSendFCC(STB_HANDLE hClient, STB_UINT8* pRtcpData, STB_UINT32 nRtcpLen);
void 	yx_funcCallBackSendFeedBack(STB_HANDLE hClient, STB_UINT8* pRetData, STB_UINT32 nRtcpLen);
void yx_funcCallBackIGMPJoin(STB_HANDLE hClient, STB_INT32 IGMPJoinType);
int yx_funcCallBackIGMPLeave(STB_HANDLE hClient, STB_INT32 IGMPJoinType);
void * yx_funcCallBackGetMemory(STB_HANDLE hClient, STB_UINT32 len);
void yx_funcCallBackFreeMemory(STB_HANDLE hClient, STB_HANDLE memPtr);
STB_UINT32 yx_funcCallBackGetTime(void);
void yx_funcCallBackTraceOut(E_TraceLevel eLevel, const STB_INT8* pszModule, const STB_INT8* format, ...);
STB_INT32 yx_funcCallBackGetIP_FCC(STB_HANDLE hClient , GetIPType ip_type,STB_INT8*buf,STB_INT32 *ilength );
STB_INT32 yx_funcCallBackGetIP_RET(STB_HANDLE hClient , GetIPType ip_type,STB_INT8*buf,STB_INT32 *ilength );
void yx_funcCallBackTellFccStateRsp(STB_HANDLE hClient, FccStateRsp fcc_state_response);


void yx_funcCallBackFccIP(STB_HANDLE hClient,STB_UINT32 server_addr, FccIpType ip_type);
void yx_funcCallBackFccPort(STB_HANDLE hClient,STB_UINT32 port);
void yx_funcCallBackValidTime(STB_HANDLE hClient,STB_INT32 ValidTime);
void yx_funcCallBackSendNatData(STB_HANDLE hClient,STB_UINT8 *nat_data, STB_UINT32 data_len);
void yx_funcCallBackHmsRsp(STB_HANDLE hClient, HmsStateRsp hms_state_response);
//RET
void yx_funcCallBackSendFeedBack_RET(STB_HANDLE hClient, STB_UINT8* pRetData, STB_UINT32 nRtcpLen);
void yx_funcCallBackSendFeedBack_ARQ(STB_HANDLE hClient, STB_UINT8* pRetData, STB_UINT32 nRtcpLen);
void yx_funcCallBackDataBufferLevel(STB_HANDLE hClient, DataBufferLevel pDataBufferLevel);
void yx_funcCallBackDataBufferLevelNULL(STB_HANDLE hClient, DataBufferLevel pDataBufferLevel);

void yx_funcCallBackSendFCCNULL(STB_HANDLE hClient, STB_UINT8* pRtcpData, STB_UINT32 nRtcpLen);
void yx_funcCallBackSendFeedBack_RTCP(STB_HANDLE hClient, STB_UINT8* pRetData, STB_UINT32 nRtcpLen);
void yx_funcCallBackSendNatDataNULL(STB_HANDLE hClient, STB_UINT8 *nat_data, STB_UINT32 data_len);
void yx_funcCallBackIGMPJoinNULL(STB_HANDLE hClient, STB_INT32 IGMPJoinType);
void yx_funcCallBackTellFccStateRspNULL(STB_HANDLE hClient, FccStateRsp fcc_state_response);


STB_UINT16 STB_htons(STB_UINT16 hostshort);
STB_UINT16 STB_ntohs(STB_UINT16 netshort);
STB_UINT32 STB_htonl(STB_UINT32 hostshort);
STB_UINT32 STB_ntohl(STB_UINT32 netshort);
STB_UINT32 STB_time_ms(void);

#endif

