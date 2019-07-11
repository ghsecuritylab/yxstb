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
	unsigned short int command_length;  //�������������ݰ��ĳ� �ȣ��������ֶα��� )��ȡֵ��Χ0x0004-0xFFFF ��
	unsigned short int command_id;				//��ʾ PDU ��Ϣ���� 0x0001 ���ţ�0x0002 ��ͣ��0x0003 ������ˣ�0x0004  �˳���0x0005 ������־����
  SQM_MSG_C26 Var;		//��Ϣ��
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

/* Eagle add. 2011年01月19日 */
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
//����C58�°�SQM�淶,����ṹ�屣��sqm.ini ��Ϣ
 typedef struct C58_sqm_ini
{
	char StbId[33];				//������ID
	char UserId[33];			//�������û�ID
	char MacAddress[20];		//����������MAC��ַ
	char StbIP[20];				//���������� IP��ַ��
	char StbNetName[33];		//�������������� ʾ��"eth0"
	char StbMulticastNetName[33];	//�������鲥�����������ơ�ʾ��"ppp0"��
	char PppoeAccount[33];			//PPPoE�ʻ�����û�У����ÿ�
	char EpgIP[20];					// EPG��IP��ַ��ʾ��"111.79.105.1"
	char StbSoftwareVersion[33];		//����������汾�š�����ʵ��ȡֵ��д
	char StbHardwareType[33];		//������Ӳ�����͡�����ȡֵ:3560;3560E
	char MqmcIP[20];			//MQMC��IP��ַ��
	int 	MqmcSendPort;		//MQMC�����˿ڡ�
	int 	MqmcRecvPort;		//������̽��ļ����˿ڡ�
	int  SqmLogLevel;		//sqm��־���� 0���ر�; 1��Alert����;2��Info����3��Debug����
	char StbNatIP[20];		//������NAT IP��ַ
	int   StbNatPort;			//������NAT IP��ַ
	char UpdateUrl[256];		//�������������������������������ļ���URL��
	/*�����ֶ��������ֶ���sqmloadersqmloader ������д��д�룬stb������� */

	char SqmVersion[33];		//������̽��汾��
	char MqmcIPReg[20];		//������̽��汾�� Ԥ��
	int MqmcPortReg;			//ע��MQMC�Ķ˿ڡ�Ԥ��
	char SqmUpdateServerIP[20];	//������̽������������ IP��ַ
	int  SqmUpdateServerPort;	//������̽�������������˿ڡ�
	int  SqmloaderLogLevel;	//sqmloader��־���� ͬSqmLogLevel�Ƚϣ�ȡ���������־����
	int SqmproLogLevel;	    //sqmpro��־���� ͬSqmLogLevel�Ƚϣ�ȡ���������־����
} YX_sqm_ini;

int sqm_info_copy(char * writePath);

#ifdef __cplusplus
}
#endif

#endif                     // __SQM_PORT_H__

