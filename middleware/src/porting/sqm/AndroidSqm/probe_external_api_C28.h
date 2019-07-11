#ifndef __probe_external_api_h__
#define __probe_external_api_h__

#include <time.h>

#define SQM_SOCKET_RX "/tmp/sqm_socket_rx"
#define SQM_SOCKET_TX "/tmp/sqm_socket_tx"
#define SQM_SIG_RX	     "/tmp/sqm_sig_rx"
#define SQM_SIG_TX	     "/tmp/sqm_sig_tx"
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

//心跳时间间隔
#define MQMC_DEFAULT_HB_TIME 60
#define MQMC_MIN_HB_TIME 1
#define MQMC_MAX_HB_TIME 1800

//最大Agent 字段长度
#define MAX_INFO_LEN          128

#define MAX_GOP_LEN           64

#define MAX_FRAME_IN_SEC     128

#define STAT_TYPE_NUM        3

//网络接入类型
#define TYPE_STATIC 1
#define TYPE_PPPOE  2

//链路类型
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
   	 PAUSE_TYPE = 5,
}CHN_STAT;

//单播链路使用的数据传输模式
typedef enum RTSP_TRANSPORT_MODE
{
    TRANSPORT_MODE_TCP = 1,
    TRANSPORT_MODE_UDP = 2,
    TRANSPORT_MODE_RTP_OVER_UDP = 3
}RTSP_TRANSPORT_MODE;


//纠错后监控数据结构
typedef struct stb_ts_pack {
    unsigned int total_len;
    int seq_num;
    struct timeval tv;
    char pack[MAX_PKT_SIZE];
} STB_TS_PACK;

#endif
