
#ifndef _FCC_RET_H_INCLUDE_H_
#define _FCC_RET_H_INCLUDE_H_

#include "fcc_port.h"
#include "stream_port.h"

typedef void*               HANDLE;
typedef char                STB_INT8;
typedef unsigned char       STB_UINT8;
typedef short               STB_INT16;
typedef unsigned short      STB_UINT16;
typedef int                 STB_INT32;
typedef unsigned int        STB_UINT32;
typedef void*               STB_HANDLE;

#define SQA_MEM_SIZE_3M		3*1024*1024
#define SQA_MEM_SIZE_4M		4*1024*1024
#define SQA_MEM_SIZE_8M		8*1024*1024
#define SQA_MEM_SIZE_10M		10*1024*1024
#define SQA_MEM_SIZE_12M		12*1024*1024
#define SQA_MEM_SIZE_15M		15*1024*1024

#define RTP_BUFFER_SIZE		1380 //64 + 1316

struct FCC {
    STB_HANDLE hFcc;
    unsigned int fcc_ip;
    unsigned int port_off;
    unsigned short fcc_rtcp_port;
    int fcc_rtcp_sock;
    int fcc_rtp_sock;
    int fcc_arq_sock;
    int fcc_arq_date_socket;
    int ffcssrc_id;
    int fcc_port_base;

    int mult_flag; //1 加入多播组，2收到组播数据
    int mult_base_sock;	//接收源数据
    int mult_recover_sock; //接受补偿数据
    char rtp_buf[RTP_BUFFER_SIZE + 1];
    struct FCCArg arg;
};

struct RET {
    STB_HANDLE hVOD;
    unsigned int ret_ip;
    unsigned int port_off;
    unsigned short ret_port;
    //int ret_port_base;
    int ret_arq_sock;
    //int ret_arq_date_socket;
    struct RETArg	arg;
};

void fcc_recv_base_mult(void* handle, int fd);
void fcc_recv_recover_mult(void* handle, int fd);

void rcc_recv_rtcp(void* handle, int fd);
void fcc_rtp_recv_udp(void* handle, int fd);
void arq_rtp_recv_udp(void* handle, int fd);
void arq_rtp_recv_date_udp(void* handle, int fd);
void fcc_rtp_recv_udp_direct(void* handle, int fd);
void arq_rtp_recv_udp_direct(void* handle, int fd);
void arq_rtp_recv_date_udp_direct(void* handle, int fd);

void ret_arq_rtp_recv_udp_direct(void* handle, int fd);
void ret_arq_rtp_recv_date_udp_direct(void* handle, int fd);


#endif
