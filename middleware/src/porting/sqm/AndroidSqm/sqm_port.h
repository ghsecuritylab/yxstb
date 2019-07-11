/*
 * filename : sqm_port.h
 * create date: 2010.12.7
 */

#ifndef __SQM_PORT_H__
#define __SQM_PORT_H__

//#include <sys/socket.h>
#include <netinet/in.h>

#include "probe_external_api_C28.h"
#include "sqm_types.h"

#define STB_RECV_PORT   37001
#define SQM_SEND_PORT   37000

#define MAX_SQM_URL_STR_LEN  256  // same to MAX_STR_LEN in probe_external_api.h of sqm module

#define SQM_LOG_LEVEL  3          // 1--alert   2--info   3--debug

typedef struct sqm_msg{
	int MediaType;
	int ChannelIp;
	int ChannelPort;
	int StbPort;
	char ChannelURL[MAX_SQM_URL_STR_LEN];
}SQM_MSG_C26;

typedef struct stb_sqm_msg{
	unsigned short int command_length;  //定义了整个数据包的长 度（包括该字段本身 )其取值范围0x0004-0xFFFF 。
	unsigned short int command_id;				//表示 PDU 消息类型 0x0001 播放，0x0002 暂停，0x0003 快进快退，0x0004  退出，0x0005 更新日志级别
  SQM_MSG_C26 Var;		//消息体
} STB_SQM_MSG;

typedef enum SQM_MSG_TAG
{
	MSG_INIT = 1,
	MSG_START = 2,
	MSG_SETINFO = 3,
 	MSG_STOP = 4,
    MSG_PLAY=5,
    MSG_PAUSE = 6,
    MSG_FAST=7,
    MSG_LOG_LEVEL=9,
    MSG_GET_DATA=11,
}SQM_MSG;

//C28 用来获取sqm传过来的sqm数据.


typedef struct  SQM_MSG_DATA
{
	int df;				//DF值，Sqmpro乘以1000取整，STB收到后除以1000
	int mlr;				//每秒丢包数，统计数据平均值乘以106取整，秒级数据和统计数据最大值，最小值不做处理
	int jitter;			//抖动值，Sqmpro乘以1000取整，MQMC收到后除以1000
	int vmos;		//保留指标字段，预留后续扩展，当前默认值0
	int OtherIndex2;		//保留指标字段，预留后续扩展，当前默认值0
}SQM_GET_DATA;


typedef struct StbSecondData

{

    int  result;

    SQM_GET_DATA    StatData;

}STB_SECOND_DATA;
/**********************************************************/


typedef struct  sqmStatisticStruct
{
	uint64_t timestamp;       // 16 byte
	SQM_GET_DATA	StbData[3];
	unsigned int Playduration;
	unsigned int Alarmduration;
}STB_STATISTIC_DATA ;

typedef struct SQM_GET_MSG
{
    int result;
    STB_STATISTIC_DATA sqmStbData;
}SQM_MSG_GET;
// prepare for sqm porting module
struct sqmPostData{
    int jitter;
    int mdiDF;
    int mdiMLR;
    int badDuration;
    int playDuration;
    int vmos;
};


#ifdef __cplusplus
extern "C" {
#endif

// prepare for sqm porting module
void sqm_port_prepare(void);

// mqm_ip_set
void sqm_port_mqmip_set( const char *ipaddr);
void sqm_port_mqmip_get(char *ipaddr);

int  sqm_port_sqmloader_start(void);
int sqm_port_pushmsg_open(void);

// parse and save channel info
// when the pointer is NULL, this function will do nothing to the acordding para;
void parseChnInfo(CHN_STAT pStat, struct sockaddr_in* serv_sin, struct sockaddr_in* data_sin,char* url);

// write msg to fifo
int sqm_port_msg_write(SQM_MSG msg);

/* Eagle add. 2011 */
unsigned int sqm_get_listen_port ( void );
unsigned int sqm_get_server_port ( void );
void sqm_set_listen_port ( unsigned int port );
void sqm_set_server_port ( unsigned int port );

int sqm_port_pushdata_open(void);
int sqm_port_pushdata(int pIndex, char *buf, int len, int scale);
int sqm_port_pushdata_close(void);
void sqm_port_pushdata_clear(void);

//C26 need!
int  sqm_port_sqmloader_start(void);
int sqm_port_pushmsg_open(void);

int sqm_info_copy(char * writePath);
SQM_STATUS sqm_port_getStatus(void);
void sqm_port_setStatus(SQM_STATUS status);

int sqm_port_buildmsg(SQM_MSG sqm_msg);
int sqm_port_getdata(int);
int getSqmDataMdiMLR(void);
int getSqmDataMdiDF(void);
int getSqmDataJitter(void);
int getSqmDataBadDuration(void);
int getSqmDataPlayduration(void);
int getSqmDataAvailability(void);
int getSqmDataVideoQuality(void);
#ifdef __cplusplus
}
#endif



/***************App_sys.h*****************/
//依据C58新版SQM规范,定义结构体保存sqm.ini 信息
 typedef struct C58_sqm_ini
{
	char StbId[33];				//机顶盒ID
	char UserId[33];			//机顶盒用户ID
	char MacAddress[20];		//机顶盒网卡MAC地址
	char StbIP[20];				//机顶盒网卡 IP地址。
	char StbNetName[33];		//机顶盒网卡名称 示例"eth0"
	char StbMulticastNetName[33];	//机顶盒组播接收网卡名称。示例"ppp0"。
	char PppoeAccount[33];			//PPPoE帐户。若没有，就置空
	char EpgIP[20];					// EPG的IP地址。示例"111.79.105.1"
	char StbSoftwareVersion[33];		//机顶盒软件版本号。根据实际取值填写
	char StbHardwareType[33];		//机顶盒硬件类型。如下取值:3560;3560E
	char MqmcIP[20];			//MQMC的IP地址。
	int 	MqmcSendPort;		//MQMC监听端口。
	int 	MqmcRecvPort;		//机顶盒探针的监听端口。
	int  SqmLogLevel;		//sqm日志级别 0，关闭; 1，Alert级别;2，Info级别；3，Debug级别。
	char StbNatIP[20];		//机顶盒NAT IP地址
	int   StbNatPort;			//机顶盒NAT IP地址
	char UpdateUrl[256];		//机顶盒向升级服务器请求升级配置文件的URL。
	/*以下字段由以下字段由sqmloadersqmloader 负责负责写入写入，stb请勿操作 */

	char SqmVersion[33];		//机顶盒探针版本号
	char MqmcIPReg[20];		//机顶盒探针版本号 预留
	int MqmcPortReg;			//注册MQMC的端口。预留
	char SqmUpdateServerIP[20];	//机顶盒探针升级服务器 IP地址
	int  SqmUpdateServerPort;	//机顶盒探针升级服务器端口。
	int  SqmloaderLogLevel;	//sqmloader日志级别 同SqmLogLevel比较，取两者最大日志级别
	int SqmproLogLevel;	    //sqmpro日志级别。 同SqmLogLevel比较，取两者最大日志级别。
} YX_sqm_ini;


#endif                     // __SQM_PORT_H__

