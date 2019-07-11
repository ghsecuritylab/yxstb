/*
 * filename : sqm_port.h
 * create date: 2010.12.7
 */

#ifndef __SQM_PORT_H__
#define __SQM_PORT_H__

//#include <sys/socket.h>
#include <netinet/in.h>

#if defined(SQM_VERSION_C22) || defined(SQM_VERSION_C23)
#include "probe_external_api_C22.h"
#endif

#define STB_RECV_PORT   37001
#define SQM_SEND_PORT   37000

#define MAX_SQM_URL_STR_LEN  256  // same to MAX_STR_LEN in probe_external_api.h of sqm module

#define SQM_LOG_LEVEL  3          // 1--alert   2--info   3--debug

#ifdef SQM_VERSION_C21
typedef enum CHN_STAT_TAG
{
    STB_RESTART = 0,
    UNICAST_CHANNEL_TYPE = 1,
    MULTICAST_CHANNEL_TYPE = 2,
    HTTP_UNICAST_TYPE = 3,
    HTTP_MULTICAST_TYPE = 4,
    FAST_BB_BF_TYPE = 98,
    STB_IDLE = 99,
}CHN_STAT;
#endif

typedef enum SQM_MSG_TAG
{
	MSG_INIT = 1,
	MSG_START = 2,
	MSG_SETINFO = 3,
    MSG_STOP = 4
}SQM_MSG;

#ifdef __cplusplus
extern "C" {
#endif

// prepare for sqm porting module
void sqm_port_prepare(void);

// mqm_ip_set
void sqm_port_mqmip_set( const char *ipaddr);

// parse and save channel info
// when the pointer is NULL, this function will do nothing to the acordding para;
void parseChnInfo(CHN_STAT pStat, struct sockaddr_in* serv_sin, struct sockaddr_in* data_sin,char* url);

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

#ifdef __cplusplus
}
#endif

#endif                     // __SQM_PORT_H__

