/*
 * filename : sqm_port.h
 * create date: 2010.12.7
 */

#ifndef __SQM_PORT_H__
#define __SQM_PORT_H__

//#include <sys/socket.h>
#include <netinet/in.h>

#include "probe_external_api_C26.h"

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
	unsigned short int command_length;  //¶¨ÒåÁËÕû¸öÊı¾İ°üµÄ³¤ ¶È£¨°üÀ¨¸Ã×Ö¶Î±¾Éí )ÆäÈ¡Öµ·¶Î§0x0004-0xFFFF ¡£
	unsigned short int command_id;				//±íÊ¾ PDU ÏûÏ¢ÀàĞÍ 0x0001 ²¥·Å£¬0x0002 ÔİÍ££¬0x0003 ¿ì½ø¿ìÍË£¬0x0004  ÍË³ö£¬0x0005 ¸üĞÂÈÕÖ¾¼¶±ğ
  SQM_MSG_C26 Var;		//ÏûÏ¢Ìå
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
}SQM_MSG;

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
void parseChnInfo(CHN_STAT stat, struct sockaddr_in* serv_sin, struct sockaddr_in* data_sin,char* url);

// write msg to fifo
int sqm_port_msg_write(SQM_MSG msg);

/* Eagle add. 2011å¹´01æœˆ19æ—¥ */
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

/***************App_sys.h*****************/
//ÒÀ¾İC58ĞÂ°æSQM¹æ·¶,¶¨Òå½á¹¹Ìå±£´æsqm.ini ĞÅÏ¢
 typedef struct C58_sqm_ini
{
	char StbId[33];				//»ú¶¥ºĞID
	char UserId[33];			//»ú¶¥ºĞÓÃ»§ID
	char MacAddress[20];		//»ú¶¥ºĞÍø¿¨MACµØÖ·
	char StbIP[20];				//»ú¶¥ºĞÍø¿¨ IPµØÖ·¡£
	char StbNetName[33];		//»ú¶¥ºĞÍø¿¨Ãû³Æ Ê¾Àı"eth0"
	char StbMulticastNetName[33];	//»ú¶¥ºĞ×é²¥½ÓÊÕÍø¿¨Ãû³Æ¡£Ê¾Àı"ppp0"¡£
	char PppoeAccount[33];			//PPPoEÕÊ»§¡£ÈôÃ»ÓĞ£¬¾ÍÖÃ¿Õ
	char EpgIP[20];					// EPGµÄIPµØÖ·¡£Ê¾Àı"111.79.105.1"
	char StbSoftwareVersion[33];		//»ú¶¥ºĞÈí¼ş°æ±¾ºÅ¡£¸ù¾İÊµ¼ÊÈ¡ÖµÌîĞ´
	char StbHardwareType[33];		//»ú¶¥ºĞÓ²¼şÀàĞÍ¡£ÈçÏÂÈ¡Öµ:3560;3560E
	char MqmcIP[20];			//MQMCµÄIPµØÖ·¡£
	int 	MqmcSendPort;		//MQMC¼àÌı¶Ë¿Ú¡£
	int 	MqmcRecvPort;		//»ú¶¥ºĞÌ½ÕëµÄ¼àÌı¶Ë¿Ú¡£
	int  SqmLogLevel;		//sqmÈÕÖ¾¼¶±ğ 0£¬¹Ø±Õ; 1£¬Alert¼¶±ğ;2£¬Info¼¶±ğ£»3£¬Debug¼¶±ğ¡£
	char StbNatIP[20];		//»ú¶¥ºĞNAT IPµØÖ·
	int   StbNatPort;			//»ú¶¥ºĞNAT IPµØÖ·
	char UpdateUrl[256];		//»ú¶¥ºĞÏòÉı¼¶·şÎñÆ÷ÇëÇóÉı¼¶ÅäÖÃÎÄ¼şµÄURL¡£
	/*ÒÔÏÂ×Ö¶ÎÓÉÒÔÏÂ×Ö¶ÎÓÉsqmloadersqmloader ¸ºÔğ¸ºÔğĞ´ÈëĞ´Èë£¬stbÇëÎğ²Ù×÷ */

	char SqmVersion[33];		//»ú¶¥ºĞÌ½Õë°æ±¾ºÅ
	char MqmcIPReg[20];		//»ú¶¥ºĞÌ½Õë°æ±¾ºÅ Ô¤Áô
	int MqmcPortReg;			//×¢²áMQMCµÄ¶Ë¿Ú¡£Ô¤Áô
	char SqmUpdateServerIP[20];	//»ú¶¥ºĞÌ½ÕëÉı¼¶·şÎñÆ÷ IPµØÖ·
	int  SqmUpdateServerPort;	//»ú¶¥ºĞÌ½ÕëÉı¼¶·şÎñÆ÷¶Ë¿Ú¡£
	int  SqmloaderLogLevel;	//sqmloaderÈÕÖ¾¼¶±ğ Í¬SqmLogLevel±È½Ï£¬È¡Á½Õß×î´óÈÕÖ¾¼¶±ğ
	int SqmproLogLevel;	    //sqmproÈÕÖ¾¼¶±ğ¡£ Í¬SqmLogLevel±È½Ï£¬È¡Á½Õß×î´óÈÕÖ¾¼¶±ğ¡£
} YX_sqm_ini;

int sqm_info_copy(char * writePath);

#ifdef __cplusplus
}
#endif

#endif                     // __SQM_PORT_H__

