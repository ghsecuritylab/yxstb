#ifndef __probe_external_api_h__
#define __probe_external_api_h__
#include "mdi_derived_data_types.h"

#define SQM_SOCKET_RX "/tmp/sqm_socket_rx"
#define SQM_SOCKET_TX "/tmp/sqm_socket_tx"
#define MAX_PKT_SIZE 1500
#define MAX_BUFF_SIZE (512*1024)

#define PROBE_SUCCESS 0
#define PROBE_FAIL -1

#define MAX_DEV_LEN    64
#define MAX_ID_LEN     33

#define MAX_MAC_LEN   33
#define MAX_DIR_SIZE 128
#define MAX_LOSS_NUM  20
#define MAX_RESUME_NUM  10

#define MAX_SERVICE_IP_NUM     6
#define MAX_STR_LEN  256
#define TS_PACKET_NUM      7
#define TS_PACKET_SIZE     188
#define TS_SEVEN_PACKET_SIZE     1316

#define SENDTO_MQMC_PORT                     37000
#define EM_RECV_MQMC_PORT                    37001
#define MIN_PORT_VAL                          0
#define MAX_PORT_VAL                          65535

//����ʱ����
#define MQMC_DEFAULT_HB_TIME 60
#define MQMC_MIN_HB_TIME 1
#define MQMC_MAX_HB_TIME 1800

#ifdef __MDI_STB__
//STB-PCF ����ʱ����
#define MQMC_STB_DEFAULT_HB_TIME 300    //5minutes
#define MQMC_STB_MIN_HB_TIME 300         //5minutes
#define MQMC_STB_MAX_HB_TIME 86400      //24hours
#endif

//���Agent �ֶγ���
#define MAX_INFO_LEN          128

#define MAX_GOP_LEN           64

#define MAX_FRAME_IN_SEC     128

#define STAT_TYPE_NUM        3

//�����������
#define TYPE_STATIC 1
#define TYPE_PPPOE  2

//�����Ƶ����
#ifndef __MDI_STB__
#define MAX_CHANNEL_NUM   100
#define ADD_LINK   1
#define DELETE_LINK 2
#define MODIFY_LINK 3
#else
#define MAX_CHANNEL_NUM   1
#endif
//Ĭ�ϸ澯����
#define ALARM_DEFAULT_INTERVAL 60    //60��
//���澯����
#define ALARM_MAX_INTERVAL 86400    //24hours
//��С�澯����
#define ALARM_MIN_INTERVAL 1    //1��

//Ĭ��Ƶ������ʱ��
#define NOSTREAM_DEFAULT_INTERVAL 86400  //24hours
//���Ƶ������ʱ��
#define NOSTREAM_MAX_INTERVAL 604800    //7days
//��СƵ������ʱ��
#define NOSTREAM_MIN_INTERVAL 60    //1min

//�鲥�����澯�����ʱ����
#define MAX_MSERROR_INTERVAL 3600   //1hour
//�鲥�����澯����Сʱ����
#define MIN_MSERROR_INTERVAL 10   //10s

//�����յ��������pid��
#define MAX_PID_NUM 20
//��ǰ��ʶ�����С�澯ID
#define CUR_MIN_ALARM_ID 1
//��ǰ��ʶ������澯ID
#define CUR_MAX_ALARM_ID 19

#define ALARM_TOTAL 19

#define ALARM           1
#define NOT_ALARM      0

//Ĭ�ϸ澯��ֵ
#define DEFAULT_THRESHOLD_VMOS       3
#define DEFAULT_THRESHOLD_DF         50
#define DEFAULT_THRESHOLD_MLR        8
#define DEFAULT_THRESHOLD_IPLR       3
#define DEFAULT_THRESHOLD_RTPJITTER  50
#define DEFAULT_THRESHOLD_RTPLP      5
#define DEFAULT_THRESHOLD_RTPLD      5
#define DEFAULT_THRESHOLD_TCPRR      8
#define DEFAULT_THRESHOLD_MSERROR    10     //10s


enum ALARM_ID
{
    ALARM_VMOS = 1,
    ALARM_DF,
    ALARM_MLR,
    ALARM_RTPJITTER,
    ALARM_IPLR ,
    ALARM_RTPLP,
    ALARM_RTPLD,
    ALARM_TCP_RETRAN_RATE,
    ALARM_TS_SYNC_LOSS,
    ALARM_SYNC_BYTE_ERROR,
    ALARM_PAT_ERROR,
    ALARM_CC_ERROR,
    ALARM_PMT_ERROR,
    ALARM_PID_ERROR,
    ALARM_TRANSPORT_ERROR,
    ALARM_CRC_ERROR,
    ALARM_PCR_ERROR,
    ALARM_PTS_REPETIITION_ERROR,
    ALARM_MSERROR                     //Media Stream Discontinuous Error �����澯
};

//��·����
typedef enum LINK_TYPE_T{
    LINK_EM = 1,
    LINK_DM = 2,
    LINK_MULTICAST = 3,
    LINK_DETECT = 4,
    LINK_TRACE = 5,
} LINK_TYPE;

typedef enum PLAY_STATUS_T
{
	STB_RESTART = 0,
	UNICAST_CHANNEL_TYPE = 1,
	MULTICAST_CHANNEL_TYPE = 2,
	FAST_BB_BF_TYPE = 3,
	STB_IDLE = 4,
}CHN_STAT;

#ifdef __MDI_STB__
enum STB_STATUS_T
{
    STB_STATUS_START = 0,
    STB_STATUS_WARNING,
    STB_STATUS_PLAY,
    STB_STATUS_IDLE
};
#endif


//������·ʹ�õ����ݴ���ģʽ
typedef enum RTSP_TRANSPORT_MODE
{
    TRANSPORT_MODE_TCP = 1,
    TRANSPORT_MODE_UDP = 2,
    TRANSPORT_MODE_RTP_OVER_UDP = 3
}RTSP_TRANSPORT_MODE;


//����������ݽṹ
typedef struct stb_ts_pack {
    unsigned int total_len;
    int seq_num;
    struct timeval tv;
    char pack[MAX_PKT_SIZE];
} STB_TS_PACK;


/*̽��ҵ��������Ϣ*/
typedef struct stream_ip_info
{
    /*ҵ��IP��ַ*/
    MDI_UINT32    ServiceIp;

    /*ҵ����������*/
    MDI_CHAR      sServiceNetName[MAX_INFO_LEN];
}SERVICE_INFO;


typedef struct frame_info_s
{
    unsigned short frame_id;
    unsigned int frame_score;
    char frame_type;
    char  tear_persent_age;
    char  tear_reason;
    unsigned short rtp_num;
    unsigned short first_rtpsn;
    unsigned short loss_count;
    unsigned int  loss_rtp_num[MAX_LOSS_NUM];
    unsigned short fec_count;
    unsigned int  fec_resume_num[MAX_RESUME_NUM];
    unsigned short ret_count;
    unsigned int  ret_rtp_num[MAX_RESUME_NUM];
    
}FRAME_INFO_T;


typedef struct rtp_info_s
{
    unsigned int VMOS;
    unsigned int rtp_loss_num;
    unsigned int rtp_resume_num;
    unsigned int fec_resume_num;
    unsigned int ret_resume_num;
    unsigned int rtp_disoder_num;
    unsigned int rtp_repeat_num;
    unsigned int frame_num;
    unsigned int resume_frame_num;
    unsigned int resume_ratio;
    unsigned int Iframe_broken;
    unsigned int Iframe_resume;
    unsigned int Pframe_broken;
    unsigned int Pframe_resume;
    unsigned int Bframe_broken;
    unsigned int Bframe_resume;
}rtp_info_t;


typedef struct pid_info_s
{
    unsigned int pid_type;
    unsigned int pid;
}pid_info_t;


typedef struct pes_info_s
{
    unsigned int pid_count;             //�յ�����pid����
    pid_info_t pid_info [MAX_PID_NUM];   //pid��Ϣ
    unsigned int bitrate_type;
    unsigned int framerate;
    unsigned short pic_hight;
    unsigned short pic_width;
    char GOPPatten[MAX_GOP_LEN];
}pes_info_t;


typedef struct mdi_stat_s
{
    unsigned int rtp_jitter;
    unsigned int df;                              //DFֵ* 1000
    unsigned int ideal_df;                   //����DFֵ* 1000
    unsigned int mlr;                           //������
    unsigned int tcp_retran;   //TCP�ش�����
    unsigned int rtp_flag;              //rtp����־
    unsigned int rtp_num;
    unsigned int rtplr;                        //ÿ��rtp������
    unsigned int mls;                       //����������������
    //unsigned int rtp_lp;                 //ÿ������������������ֵ�Ĵ���
    unsigned int rtp_lpe;
    unsigned int rtp_lpavg;          //ÿ����ƽ����RTP����������
    unsigned int rtp_lpmax;          //ÿ��������RTP����������
    unsigned int rtp_lpmin;          //ÿ������С��RTP����������
    //unsigned int rtp_ld;                 //ÿ�붪�����������ֵ�Ĵ���
    unsigned int rtp_lde;
    unsigned int rtp_ldavg;          //ÿ����ƽ����RTP�������
    unsigned int rtp_ldmax;          //ÿ��������RTP�������
    unsigned int rtp_ldmin;          //ÿ������С��RTP�������
    unsigned int iplr;                          //IP������
    unsigned int mr;                            //ý��pcr����  Kb/s
    unsigned int linearity_mr;          //�������ʣ���ʵ��ý����������  Kb/s
    //unsigned int mr_deviation;                        //����ƫ���� �ٷֱ�
    //unsigned int vbr_flag;                 //�����Ƿ�Ϊvbr,  0:cbr;   1:vbr
}mdi_stat_t;



typedef struct qos_data_s
{
    unsigned long long timestamp;   //��ǰʱ�������ȷ��ms
    
    mdi_stat_t mdi_stat;

    rtp_info_t  rtp_info;           //RTP����ϸ��Ϣ

    unsigned short frame_num;
    
    FRAME_INFO_T frame_info[MAX_FRAME_IN_SEC];
}qos_data_t;


//�������ݵ�λ����CBBÿ�����������
typedef struct status_element_s
{
    //unsigned long long timestamp;   //��ǰʱ�������ȷ��ms

    //float gop_score;
    
    pes_info_t pes_info;

    //ͳ�Ƶ�ǰ���ڸ澯����
    //alarm_element_t sec_alarm_stat;
    qos_data_t qos_data;

    //����澯����
    unsigned int  alarm[CUR_MAX_ALARM_ID];
    
    //frame_data_t frame_data;
    
}status_element_t;


typedef struct  ChannelMdiInfo
{
    MDI_INT32 ChannelId;

    MDI_INT32 LinkId;

    MDI_INT32 LinkType;

    MDI_INT32 DeviceId;
    MDI_UINT32 ulSrcIP;
    
    MDI_UINT32 ulDestIP;
    
    MDI_UINT16 unSrcPort;
    

    MDI_UINT16 unDestPort;

    
    status_element_t  CurStat;
}CHANNEL_MDI_INFO;


typedef struct
{
    unsigned int   level;
    unsigned int   mask;             /*0 not shield, 1 shield, 2 can't shield*/
    unsigned int   status_keep;  /*0 is means had sent alarm packet*/
    char           desc[MAX_INFO_LEN];
} alarm_info_t;


typedef struct threshold_config_s
{
    //״̬��ֵ
    unsigned int threshold_vmos;
    unsigned int threshold_df;
    unsigned int threshold_mlr;
    unsigned int threshold_rtpjitter;
    unsigned int threshold_iplr;
    unsigned int threshold_rtplp;
    unsigned int threshold_rtpld;
    unsigned int threshold_tcp_retran;
    
    //�����Ƿ�澯   1 �澯 0���澯
    alarm_info_t alarm_list[ALARM_TOTAL]; 
}threshold_config_t;


/*������Ϣ*/
typedef struct  NetInfo
{
    /*������뷽ʽ:   
                            1       ��̬IP��ʽ����
                            2       pppoe    ��ʽ���� */
    MDI_UINT32    uiConnectMode;

    /*�鲥���뷽ʽ:
                             1       ��̬IP��ʽ����
                             2       pppoe    ��ʽ����*/
    MDI_UINT32    uiMutilcastMode;

    /*̽������IP����MQMC���н���*/
    MDI_UINT32    HostIP;

    /*ҵ����������*/
    MDI_UINT32    uiServiceNetCount;
    
    /*�������������Ϣ*/
    SERVICE_INFO    ServiceNetInfo[MAX_SERVICE_IP_NUM];

    /*�鲥���IP*/
    SERVICE_INFO    MutilcastNetInfo;

}NET_INFO;


typedef struct stb_info_values
{
    /*������ID*/
    MDI_CHAR     sStbId[MAX_ID_LEN];

    /*�������û�ID*/
    MDI_CHAR     sUserId[MAX_ID_LEN];

    /*������MAC��ַ*/
    MDI_CHAR     sMacAddr[MAX_MAC_LEN];

    /*������IP*/
    SERVICE_INFO    StbIpInfo;
    SERVICE_INFO    MutilcastNetInfo;//ר���������鲥����IP

    /*PPPoE�˺�*/                          
    MDI_CHAR     sPPPoEAccount[MAX_ID_LEN];
                                           
    /*EPGIP��ַ*/                          
    MDI_UINT32    EpgIp;

}MDI_STB_INFO;


#ifdef __MDI_STB__
typedef struct stb_stat_data_s
{
//    unsigned int StatType;
//    unsigned int StatDataType[STAT_TYPE_NUM];
    unsigned int linearity_mr[STAT_TYPE_NUM];
    unsigned int VMOS[STAT_TYPE_NUM];
    unsigned int df[STAT_TYPE_NUM];
    unsigned int mlr[STAT_TYPE_NUM];
    unsigned int RTPJitter[STAT_TYPE_NUM];
    unsigned int IPLR[STAT_TYPE_NUM];
    unsigned int tcp_retran[STAT_TYPE_NUM];
    unsigned int cpu_usage[STAT_TYPE_NUM];
}stb_stat_data_t;
#endif


typedef struct mdi_output_stat_value
{
    MDI_UINT32    uiOutputInterval;
}MDI_STAT_INTERVAL;


typedef struct probe_link_info
{
    char  sLinkIpUrl[MAX_STR_LEN];

    MDI_UINT32  uiLinkPort;

    MDI_UINT32  uiLinkId;

    MDI_UINT32  uiLinkType;

    MDI_UINT32  uiRealtimeFlag;
    
}MDI_LINK_INFO;


typedef struct probe_device_info
{
    unsigned int DeviceId;
    unsigned int reserved1;
}MDI_DEVICE_INFO;


typedef struct probe_link_total
{
    MDI_UINT32  link_num;
    MDI_LINK_INFO  link_info[MAX_CHANNEL_NUM];
}MDI_LINK_TOTAL;


typedef struct probe_channel_info
{
    /*ý�崫������:  1:����   2:�鲥*/
    MDI_UINT32   MediaType;
    
    /*Ƶ��ԴIP:  ������HMS IP;  �鲥���鲥IP*/
    MDI_UINT32    ChannelIp;

    /*Ƶ��Դ�˿�:  ������HMS�����˿�,  �鲥���鲥�˿�*/
    MDI_UINT32    ChannelPort;

    /*�����������˿�*/
    MDI_UINT32  StbPort;
    
    /*Ƶ��URL*/
    MDI_CHAR    ChannelUrl[MAX_STR_LEN];
}MDI_CHANNEL_INFO;


/* ̽���ʼ����Ϣ */
typedef struct probe_info_values
{
    
    /*̽���豸����*/
    MDI_CHAR     sDeviceName[MAX_DEV_LEN];
    
    /* ̽���豸ID*/
    MDI_UINT32    uiDeviceId;
    
    /* ̽���豸���� */
    MDI_UINT32    uiDeviceType;

    /* ̽������ʱ����*/
    MDI_UINT32    uiHBInterval;

    /*���ɼ��Ƶ����*/
    MDI_UINT32    uiMaxChannelNum;

    /*����ʱ����*/
    MDI_UINT32    uiNoStreamInterval;

    /*������·Э��*/
    RTSP_TRANSPORT_MODE TransportMode;
    
    /*̽������IP ��Ϣ*/
    NET_INFO      NetAddrInfo;
#ifndef __MDI_STB__
    MDI_LINK_TOTAL link_total;
#endif
}MDI_PROBE_INFO;


/*xuhu add for stb_test*/
typedef struct stb_ip_change_s
{
        /*������ID*/
    //MDI_CHAR     sStbId[MAX_ID_LEN];

    /*�������û�ID*/
    //MDI_CHAR     sUserId[MAX_ID_LEN];

    MDI_INT32 ServiceIp;
}stb_ip_change_t;


typedef enum PROBE_LOG_LEVEL
{
    PROBE_LOG_CLOSED = 0,
    PROBE_LOG_ALERT, 
    PROBE_LOG_INFO,
    PROBE_LOG_DEBUG
}PROBE_LOG_LEVEL;


typedef enum MDI_PROBE_TYPE 
{ 
    DEVICE_STB = 0, 
    DEVICE_PROBE,
    HMS_PROBE,
    RRS_PROBE,
    PASSIVE_PROBE
}MDI_PROBE_TYPE;


typedef enum MDI_RECV_PACKET_METHOD 
{ 
    RECV_FROM_PCAP, 
    RECV_FROM_STB
}MDI_RECV_PACKET_METHOD;


MDI_INT32 PROBE_initProbeCBB(MDI_PROBE_TYPE ProbeType, 
        MDI_LOGLEVELS  LogLevel, MDI_UINT32 uiRecvPort, MDI_UINT32 uiSendPort,
        MDI_UINT32 MqmcIp,
        MDI_STB_INFO * StbInfo,
        MDI_PROBE_INFO * ProbeInfo,
        MDI_INT16 *pnErrNum);

//MDI_INT32 PROBE_RegisterMdiOutputCallBackFunction(MDI_INT16(*LogHandler)
//        (qos_data_t *pChannelData, MDI_UINT32 ChannelNum), MDI_INT16 *pnErrNum);
//PROBE_RETURN_VALUE PROBE_RegisterAlarmCallBackFunction(MDI_INT16(*AlarmHandler)
//        (threshold_config_t *pLogData), MDI_INT16  *pnErrNum);

MDI_INT32 PROBE_RegisterDeviceCallBackFunction(MDI_INT16(*DeviceHandler)
        (MDI_DEVICE_INFO *pDeviceData), MDI_INT16 *pnErrNum);

MDI_INT32 PROBE_RegisterLinkCallBackFunction(MDI_INT16(*LinkHandler)
        (MDI_LINK_INFO *pLinkData, int mode), MDI_INT16 *pnErrNum);

/*************************************************************************

    Function:       PROBE_RegisterLogCallBackFunction

    @Description        PROBE_RegisterLogCallBackFunction :regist probe log 

                        callback function

    @param[in]    (*LogHandler) function pointer of handler

    @param[out]   *pnErrNum  it will point to an error number in case of

                          PROBE_FAIL

    @retval       PROBE_SUCCESS val=0   On PROBE_SUCCESS

                  PROBE_FAIL val=-1   On PROBE_FAIL

 **************************************************************************/
 
MDI_INT32 PROBE_RegisterLogCallBackFunction(MDI_INT16(*LogHandler)
            (MDI_CHAR *pLogData), MDI_INT16 *pnErrNum);

MDI_INT32 PROBE_StartMornitor( MDI_INT16 *pnErrNum);

MDI_INT32 PROBE_StopMornitor( MDI_INT16 *pnErrNum);

MDI_INT32 PROBE_SetChannelInfo(MDI_CHANNEL_INFO  *ChannelInfo,
      MDI_INT16 *pnErrNum);

MDI_INT32 PROBE_SetLogLevel(MDI_INT32 log_level, MDI_INT16 *pnErrNum);

//MDI_INT32 PROBE_SetOutputInterval(MDI_STAT_INTERVAL  *StatInterval,
//        MDI_INT16 *pnErrNum);
//MDI_INT32 PROBE_SetRecvPacketMethod(MDI_RECV_PACKET_METHOD
//        RecvPacketMethod, MDI_INT16 *pnErrNum);
//PROBE_RETURN_VALUE PROBE_SetAlarmInfo(threshold_config_t  *alarm_config,
//        MDI_INT16 *pnErrNum);

//edit by xuhu

MDI_INT32 PROBE_ReportStbStatus(stb_ip_change_t *pStbChangeInfo,MDI_INT16 *pnErrNum);

#endif
