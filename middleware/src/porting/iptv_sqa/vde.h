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
/* FCCģ��궨��                                                      */
/************************************************************************/

/************************************************************************/
/* ����ѡ��궨�壨���ܴ��밴λ��ʾ*/
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
#ifndef NOER         // ������
#define NOER 0x00
#endif
#ifndef CTC        //����RET
#define CTC 0x01
#endif
#ifndef HW        //HW�淶RET
#define HW 0x02
#endif
#ifndef IPV4        //IPV4
#define IPV4 0x01
#endif
#ifndef IPV6        //IPV6
#define IPV6 0x02
#endif
#ifndef NORTP           //������(��RTPͷ)
#define NORTP 0x16
#endif


typedef enum
{
      LEVEL_DEBUG = 0,    /* ������Ϣ */
      LEVEL_INFO,         /* ��Ҫ��Ϣ */
      LEVEL_WARNING,    /* ������Ϣ */
      LEVEL_ERROR,        /* ������Ϣ */
      LEVEL_NONE          /* ����־   */
}E_TraceLevel;


/***********************************
  �����鲥������
  **********************************/
  #define IGMP_ORIGINAL 0
  #define IGMP_FEC_LAYER1 1
  #define IGMP_FEC_LAYER2 2

typedef enum
{
  SQA_BUFFER_LEVEL_HIGH,// �߻��徯��
  SQA_BUFFER_LEVEL_LOW, // �ͻ��徯��
  SQA_BUFFER_LEVEL_RECONNECT, //��������
  SQA_BUFFER_LEVEL_OVERFLOW   //ˮλ�������
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
	SQA_GETIP_STBIP,                  //��ȡ������IP
	SQA_GETIP_MULTGROUPIP, //��ȡ�鲥��IP,�ڵ������������д
	SQA_GETIP_MULTSRCIP       //��ȡ�鲥ԴIP,V3�汾ʹ�ã��ڵ�����V2�²�����д
}GetIPType;


typedef enum
{
  FCC_SIGNALING = 0,             //FCC����IP
  FCC_DATA  = 1                  //FCC����IP
}FccIpType;



/************************************************************************/
/* �����ǻ������ṩ�Ļص���������                                       */
/************************************************************************/

/*******************************************************************************
Description:   �ͻ��˻ص�����������FCC������
Calls By:       SQA Module
Input:           hClient �����б�ʶ, rtcpdata �������ݣ�rtcp_len ���ĳ���
Output:         none
Return:         void
*******************************************************************************/
typedef void funcCallBackSendFCC(STB_HANDLE hClient, STB_UINT8 *rtcp_data, STB_UINT32 rtcp_len);

/*******************************************************************************
Description:   �ͻ��˻ص����������Ͷ����ش�������
Calls By:       SQA Module
Input:            hClient �����б�ʶ, rtcpdata �������ݣ�rtcp_len ���ĳ���
Output:         none
Return:         void
*******************************************************************************/
typedef void funcCallBackSendRET(STB_HANDLE hClient, STB_UINT8 *ret_data, STB_UINT32 data_len);


/*******************************************************************************
Description:    �ͻ��˻ص�����������IGMP���ģ�����Ƶ���л�ʹ��
Calls By:       SQA Module
Input:           fnClientHandle, IGMPJoinType(0:ԭʼ�鲥��,1:FECLayer1�鲥��,2:FECLayer2�鲥��)
Output:         none
Return:         int
*******************************************************************************/

typedef void funcCallBackIGMPJoin(STB_HANDLE hClient, STB_INT32 igmp_join_type);


/*******************************************************************************
Description:    �ͻ��˻ص������������ڴ棬Ϊ��ͬ����ϵͳ��ֲ��װ
Calls By:       SQA Module
Input:           fnClientHandle, len: Bytes to allocate
Output:         none
Return:         a void pointer to the allocated space or NULL if there is insufficient memory available
*******************************************************************************/
typedef void * funcCallBackGetMemory(STB_HANDLE hClient, STB_UINT32 len);


/*******************************************************************************
Description:    �ͻ��˻ص��������ͷ��ڴ棬Ϊ��ͬ����ϵͳ��ֲ��װ
Calls By:       SQA Module
Input:           fnClientHandle,memPtr: Previously allocated memory block to be freed
Output:         none
Return:         void
*******************************************************************************/
typedef void funcCallBackFreeMemory(STB_HANDLE hClient, STB_HANDLE mem_addr);


/*******************************************************************************
Description:    �ͻ��˻ص���������ȡϵͳ����ʱ��(1/1000)��Ϊ��ͬ����ϵͳ��ֲ��װ
Calls By:       SQA Module
Input:           fnClientHandle, timeval struct point
Output:         system time in timeval struct
Return:         ��
*******************************************************************************/
typedef 	STB_UINT32 funcCallBackGetTime(void);


/*******************************************************************************
Description:    �ͻ��˻ص����������SQA��־��Ϣ
Calls By:       SQA Module
Input:
		      eLevel            ��־����
		      pszModule      ����ģ��
		      format            ��ʽ��

Output:         ��
Return:         ��
*******************************************************************************/
typedef void funcCallBackTraceOut(E_TraceLevel eLevel, const STB_INT8* pszModule, const STB_INT8* format, ...);

/*******************************************************************************
Description:    �ͻ��˻ص���������ȡ������IP��ַ
Calls By:       HWERModule
Input:           hClient �����б�ʶ, ip_type��ȡIP����
 Output:        buf�����IP��ַ���ַ�����ʽ�����磺IPV4��100.1.20.247��
ilength��ʵ�ʵ�IP�ַ������ȣ�������дʱ����Ϊ0
Return:          ����ɹ�����STB_OK;���򷵻�STB_ERROR.

*******************************************************************************/
typedef STB_INT32 funcCallBackGetIP(STB_HANDLE hClient , GetIPType ip_type,STB_INT8*buf,STB_INT32 *ilength );

/*******************************************************************************
Description:    �ͻ��˻ص������� SQA���������漶��仯֪ͨ by matchbox
Calls By:       HWERModule
Input:           fnClientHandle
Output:         E_DataBufferLevel ���������漶��
Return:         void
*******************************************************************************/
typedef void funcCallBackDataBufferLevel(STB_HANDLE hClient, DataBufferLevel data_buffer_level);

/*******************************************************************************
Description:     �ͻ��˻ص������� FCC������IP֪ͨ
Calls By:        HWERModule
Input:           ��
Output:          server_addr: FCC������IP��ַ
                 ip_type:     FCC������IP��ַ����
Return:          void
*******************************************************************************/
typedef void funcCallBackFccIP(STB_HANDLE hClient,STB_UINT32 server_addr, FccIpType ip_type);

/*******************************************************************************
Description:     �ͻ��˻ص������� FCC�������˿�֪ͨ
Calls By:        HWERModule
Input:           ��
Output:          port:        FCC�������ķ����˿�
Return:          void
*******************************************************************************/
typedef void funcCallBackFccPort(STB_HANDLE hClient,STB_UINT32 port);

/*******************************************************************************
Description:     �ͻ��˻ص������� FCC·�ɱ�ʱʱ��֪ͨ
Calls By:        HWERModule
Input:           ��
Output:          ValidTime:       ֪ͨFCC·�ɳ�ʱʱ��
Return:          void
*******************************************************************************/
typedef void funcCallBackValidTime(STB_HANDLE hClient,STB_INT32 ValidTime);


/*******************************************************************************
Description:     �ͻ��˻ص������� FCC server��Ӧ״̬֪ͨ
Calls By:        HWERModule
Input:           ��
Output:          FccStateRsp:fcc������֪ͨ
Return:          void
*******************************************************************************/
typedef void funcCallBackFccStateRsp(STB_HANDLE hClient, FccStateRsp fcc_state_response);

/******************************************************************************
Description:   �ͻ��˻ص�����������NAT�����͵�ַ����������
Calls By:       SQA Module
Input:           hClient �����б�ʶ, nat_data �������ݣ�data_len ���ĳ���
Output:         none
Return:         void
*******************************************************************************/
typedef void funcCallBackSendNatData(STB_HANDLE hClient,STB_UINT8 *nat_data, STB_UINT32 data_len);

/*******************************************************************************
Description:     �ͻ��˻ص������� FCC server��Ӧ״̬֪ͨ
Calls By:        HWERModule
Input:           ��
Output:          FccStateRsp:fcc������֪ͨ
Return:          void
*******************************************************************************/
typedef void funcCallBackHmsRsp(STB_HANDLE hClient, HmsStateRsp hms_state_response);

/* ������ע��ص�����*/
/*����C58�淶,���¶��������ӿ� ----sqa_init  ʹ�õĻص�����*/
typedef struct _CallBackFuns
{
    STB_HANDLE hClient;                               //�����пͻ��˱�ʶ
    funcCallBackGetMemory* fcbGetMemory;              //�����ڴ�
    funcCallBackFreeMemory * fcbFreeMemory;           //�ͷ��ڴ�
    funcCallBackGetTime* fcbGetTime;	              //��ȡ������ʱ��
    funcCallBackTraceOut *fcbTraceOut;                //�����־��Ϣ
}CallBackFuns;

/*����C58�淶,���¶��������ӿ� ----sqa_set_parameter  ʹ�õĻص�����*/
typedef struct _SQACallBackFuns
{
    STB_HANDLE hClient;                               //�����пͻ��˱�ʶ
    funcCallBackSendFCC *fcbSendFCC;                  //����FCC����
    funcCallBackSendRET *fcbSendRET;                  //���Ͷ����ش�����
    funcCallBackSendNatData* fcbSendNAT;              //����NAT�����͵�ַ���±��ģ�NAT�������нӿڣ�
    funcCallBackIGMPJoin *fcbIGMPJoin;                //���ͼ����鲥������
    funcCallBackDataBufferLevel *fcbDataBufferLevel;  //֪ͨˮλ�ߵ�
    funcCallBackFccStateRsp*fcbFccStateRsp;          //֪ͨFCC server��Ӧ
    funcCallBackFccIP *fcbFccIP;                      //֪ͨFCC��IP
    funcCallBackFccPort *fcbFCCPort;                  //֪ͨFCC�Ķ˿ں�
    funcCallBackValidTime *fcbValidTime;              //֪ͨValidTimeʱ��
    funcCallBackHmsRsp *fcbhmsRsp;                    //�㲥ʱ֪ͨ��ַ���ĸ�����Ϣ��Ӧ״̬
    funcCallBackGetIP *fcbGetIP;                      //��ȡ������IP
}SQACallBackFuns;

typedef struct _SQAParam
{
	STB_UINT32 nMemorySize;          //ָ��SQA���ݻ�������С����λByte
	STB_UINT32 nFastCachePeriod;   //֪ͨSTBӦ����/ͣ���ٻ������Сʱ����
	STB_INT32 iCacheFirst;               //�Ƿ����Ȼ���SQA CBB 1:���Ȼ���SQA,2:���Ȼ���decode
	STB_INT32 iBufferLevel[3];         //SQA CBB���øߡ��С���ˮλ
	STB_INT32 iRETSendPeriod;       //SQA CBB���ö���ش����
	STB_INT32 iRSRTimeOut;           //�ȴ�FCC��Ӧ��ʱʱ�䣬��λ����
       STB_INT32 iSCNTimeOut;           //�ȴ�SCN��Ϣ��ʱʱ�䣬��λ����
}SQAParam;


typedef struct _StreamParam
{
	STB_INT8 cType;                          //SQA֧�ֹ������ò���:
	STB_INT8 cRetType;                    //֧��RET���ݵ����ò��� CTC�����ű�׼ HW����Ϊ��׼
	STB_UINT16 port;                        //�����н���FCC�������ظ����Ķ˿�
	//STB_UINT16 iRetPort;          //��ʱ�汾����д�����ش�����ı��ض˿ں�
	STB_INT8 cIPType;                     //IPV4����ipv6��ʽ
	STB_INT32 iSendRSRFlag;           //������:0 ����: 1
	STB_INT32 iFCCWithTLV;            //FCC������Ƿ����TLV��ʽ 1:���� 0��������
	STB_INT32 iFastCacheOn;          //�Ƿ������ٻ����ʾ��1������ 0��������
	STB_INT32  iChanCacheTime;     //Ƶ������ʱ��,��λms
	STB_INT32  iRetDelayTime;     //���õ�һ���ش���ʱ(Ӧ�Զ���)
	STB_INT32  iRetInterval;      //����ش�ʱ��������Сֵ
    	STB_INT32  iFCCServerFlag;                        //֪ͨCBB�������Ƿ�֪��FCC��ַ 0:��֪�� 1:֪��
	STB_INT32  iNatSupport;                           //�����Ƿ�֧��NAT��Խ1��֧�� 0����֧��
	STB_INT32  iNatHBinterval;                        //Nat����ʱ����
	STB_UINT32 iclientSSRC;
	STB_INT32  iDisorderTime;                         //��RET��FEC������֧������ʱ��
}StreamParam;


/*******************************************************************************
Function Name:  sqa_init
Description:       SQAģ���ʱ��
Calls By:           STB Client
Input:          yx �ص�������ָ�룬
                STBϵͳ��������SSRC�ȣ�
                HW����ѡ��FEC/ARQ/Fast/FCC�����궨����ϣ�
Output:         none
Return:
                     hSqa        SQA�����ľ��

*******************************************************************************/
STB_HANDLE  sqa_init(CallBackFuns* cbfs,SQAParam *args);

/*******************************************************************************
Function Name:       sqa_set_parameter
Description:         ����ý����ز�����Ϣ
Calls By:            STB Client
Input:               sqa_handle        SQA�����ľ��
                        args              ���������ָ������д�������ṹ��
                        cbfs		 �ص�����
Output:              ��
Return:              �ɹ�����STB_OK;
			         ���򷵻�STB_ERROR.
*******************************************************************************/

STB_INT32 sqa_set_parameter(STB_HANDLE sqa_handle,StreamParam *args,SQACallBackFuns *cbfs);
/*******************************************************************************
Function Name: sqa_destroy
Description:    SQAģ��ж��
Calls By:       STB Client
Input:
                     hSqa        SQA�����ľ��

Output:         none
Return:         void
*******************************************************************************/
void  sqa_destroy(STB_HANDLE sqa_handle);


/*******************************************************************************
Function Name: sqa_recv_rtcp_packet
Description:    hwvde�ص�������֪ͨSQAģ�鴦��RTCP����
Calls By:       STB Client
Input:
                     hSqa        SQA�����ľ��
                	pRtcpPacket :  RTCP ����
                	ulPacketLen :  RTCP ���Ĵ�С
Output:         none
Return:         0 if success or false
*******************************************************************************/
STB_INT32 sqa_recv_rtcp_packet(STB_HANDLE sqa_handle, void *buffer, STB_UINT32 packet_len);


/*******************************************************************************
Function Name: sqa_recv_multicast_rtp_packet
Description:    hwvde�ص�������֪ͨSQAģ�鴦���鲥RTP����
Calls By:       STB Client
Input:
                     hSqa        SQA�����ľ��
                	pRtcpPacket :  RTP ����
                	ulPacketLen :  RTP ���Ĵ�С
Output:         none
Return:         0 if success or false
*******************************************************************************/
STB_INT32 sqa_recv_multicast_rtp_packet(STB_HANDLE sqa_handle, void * buffer, STB_UINT32 packet_len);


/*******************************************************************************
Function Name: sqa_recv_unicast_rtp_packet
Description:    hwvde�ص�������֪ͨSQAģ�鴦��FCC����RTP����
Calls By:      STB Client
Input:          hSqa: SQA������
                pRtcpPacket :  RTP ����
                ulPacketLen :  RTP ���Ĵ�С
Output:         none
Return:         0 if success or false
*******************************************************************************/
STB_INT32 sqa_recv_unicast_rtp_packet(STB_HANDLE sqa_handle, void *buffer,STB_UINT32 packet_len);


/*************************************************************************************************************
Function Name:      sqa_handle_event
Description:        �����е��øýӿ�ȥ����SQA��RET����
Calls By:           STB Client

Input:              sqa_handle          SQA�����ľ��

Output:             ��

Return:             �ɹ�����STB_OK;
                    ���򷵻�STB_ERROR.
****************************************************************************************************************/
void sqa_handle_event(STB_HANDLE sqa_handle);


/*************************************************************************************************************
Function Name:      sqa_get_rtp_packet
Description:        �����е��øýӿڴ�SQA��ȡ���ŵ�RTP���ݰ�
Calls By:           STB Client

Return:             �ɹ�����STB_OK;
			        ���򷵻�STB_ERROR.
****************************************************************************************************************/
STB_INT32 sqa_get_rtp_packet(STB_HANDLE sqa_handle, void **buffer, STB_UINT32 *length, STB_INT32 idecoder_data_size);

/*************************************************************************************************************
Function Name:      sqa_get_rtp_packet
Description:        �����е��øýӿڴ�SQA��ȡ�ڴ浥Ԫ��������ȡRTP����RTCP����
Calls By:           STB Client

Input:
                    sqa_handle        SQA�����ľ��
			        buffer            ��ȡ�����ڴ浥Ԫ��ַ
			        length            �ڴ浥Ԫ����

Output:             ��
Return:
			        �ɹ�����STB_OK;
			        ���򷵻�STB_ERROR.
****************************************************************************************************************/
STB_INT32 sqa_get_buf(STB_HANDLE sqa_handle, void **buffer,STB_INT32 *length);


/*************************************************************************************************************
Function Name:   sqa_get_rtp_packet
Description:        �����е��øýӿ�����SQA������
Calls By:            STB Client

Input:
                     hSqa        SQA�����ľ��

Output:
                     ��
Return:
			�ɹ�����STB_OK;
			���򷵻�STB_ERROR.
****************************************************************************************************************/
void sqa_reset_buf(STB_HANDLE sqa_handle);
/*************************************************************************************************************
Function Name:   sqa_get_rtp_packet
Description:        �����е��øýӿڴ�SQA��ȡ���ŵ�RTP���ݰ�
Calls By:              STB Client

Input:
                     hSqa        SQA�����ľ��
			pData	  sqa_get_rtp_packet ���óɹ�ʱ���ص�SQA�������ݰ�ָ��

Output:
                      ��
Return:
			�ɹ�����STB_OK;
			���򷵻�STB_ERROR.
****************************************************************************************************************/
STB_INT32 sqa_free_rtp_packet(STB_HANDLE sqa_handle, void *buffer);



typedef enum
{
    FAST_CACHE_ON,   //���ٻ��忪��
    FAST_CACHE_OFF   //���ٻ���ر�
}FastCacheState;     //���������漶��

/*************************************************************************************************************
Function Name:      sqa_set_fastcache_state
Description:        �����е��øýӿ����õ�ǰ���ٻ���״̬
Calls By:           STB Client

Input:              sqa_handle          SQA�����ľ��
                    fastcache_state     ���ٻ���״̬
Output:             ��
Return:             ��
****************************************************************************************************************/
void sqa_set_fastcache_state(STB_HANDLE sqa_handle, FastCacheState fastcache_state);
/***************FCC******************/
/*************************************************************************************************************
Function Name:      sqa_get_buffer_state
Description:        �����е��øýӿڻ�ȡ��ǰSQA������ռ����
Calls By:           STB Client

Input:              sqa_handle          SQA�����ľ��

Output:             ��
Return:             �ɹ�:���ص�ǰ������ռ����(0-100)
                    ʧ��:STB_ERROR
****************************************************************************************************************/
STB_INT32 sqa_get_buffer_state(STB_HANDLE sqa_handle);

#ifdef ANDROID
#else
/***************  sqa_get_err_info  ***************/
typedef struct _SQAErrInfo
{
   STB_UINT32 uiLostNum;          //��������
   STB_UINT32 uiDisordNum;      //���������
   STB_UINT32 uiPCRBitrate;      //PCR ����
   STB_UINT32 uiLocalBitrate;    //ʵʱ����
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

