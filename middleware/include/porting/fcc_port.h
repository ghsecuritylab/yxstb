
#ifndef __FCC_PORT_H__
#define __FCC_PORT_H__

struct RTSP;

#include "ind_net.h"

/*
	FCC Ƕ�׽� RTSP �߳����棬FCC���õ���fdע�ᵽRTSP�У�ͬʱRTSP��û100�������һ��FCC������ִ�к���
	FCC ���ܿ���ʱ��RTSP�̲߳���ά���ಥ�飬��FCC���п���
 */

typedef void (*fd_recv_f)(void *fcc, int fd);
typedef int (*fd_regist_f)(struct RTSP *rtsp, void *fcc, int fd, fd_recv_f call);
typedef void (*fd_unregist_f)(struct RTSP *rtsp, int fd);
typedef int (*fd_datasize_f)(struct RTSP *rtsp);
typedef void (*fd_writedata_f)(struct RTSP *rtsp, char *buf, int len);
typedef int (*fd_writeable_f)(struct RTSP *rtsp);
/*
	����ͳ�ƣ�ͳ��FECԭʼ������
 */
typedef void (*rtsp_datastat_f)(struct RTSP* rtsp, char* buf, int len);

typedef void (*cache_on_f)(struct RTSP* rtsp);
typedef void (*cache_off_f)(struct RTSP* rtsp);
typedef void (*rtsp_pktfb_f)(struct RTSP* rtsp, uint8_t *buf, uint32_t len);

int rtsp_multicast_play(struct RTSP* rtsp, unsigned short port);
void rtsp_multicast_stop(struct RTSP* rtsp, int sock);

struct FCCArg {
	struct RTSP*		rtsp;
	int					type;
	int					flag;//0: ��̨���ţ�1��ʱ��תֱ��

	unsigned short		pip;//PIP��־ 1 ��ʾPIP 0 ��ʾ��ͨ����
	fd_regist_f			fd_regist;
	fd_unregist_f		fd_unregist;

	fd_datasize_f		fd_datasize;
	fd_writedata_f		fd_writedata;
	fd_writeable_f		fd_writeable;

	rtsp_datastat_f		rtsp_datastat;

	struct ind_sin 		sin;
};

struct RETArg {
	struct RTSP *rtsp;
	int bitrate;
	fd_regist_f fd_regist;
	fd_unregist_f fd_unregist;

	fd_datasize_f fd_datasize;
	fd_writedata_f fd_writedata;
	fd_writeable_f fd_writeable;

	unsigned short clt_port;//rtcp ���ض˿�
	unsigned short srv_port;//rtcp ����˶˿�
	cache_on_f cache_on;//��������
	cache_off_f cache_off;//�رջ���

	rtsp_pktfb_f rtsp_pktfb;//����ͨ��
	struct ind_sin sin;
};

/*
	���벢��fcc
	v2 Ϊ֧��IPv6�����������˱����Ϊ����
	v3 ���� rtsp_datastat_f
	v4 ���� flag
 */
#define fcc_port_open	fcc_port_open_v4
void *fcc_port_open(struct FCCArg* arg);

/*
    1 �����鲥
    2 �����鲥���յ�����
 */
int fcc_port_mflag(void* handle);
/*
	�رղ��ͷ�fcc
 */
void fcc_port_close(void* handle);

/*
	rtsp���̻߳�100�����ӵ���һ��
 */
void fcc_port_100ms(void* handle);

#define ret_port_open	ret_port_open_v3
void *ret_port_open(struct RETArg* arg);
void ret_port_close(void* handle);
void ret_port_reset(void* handle);
void ret_port_push(void* handle, char *buf, int len);
int ret_port_pop(void* handle);
void ret_port_cache(void* handle, int on);
void ret_port_100ms(void* handle);

void *arq_port_open(struct RETArg* arg);
void arq_port_close(void* handle);
void arq_port_reset(void* handle);
void arq_port_push(void* handle, char *buf, int len);
void arq_port_100ms(void* handle);

void *rtcp_port_open(struct RETArg* arg);
void rtcp_port_close(void* handle);
void rtcp_port_push(void* handle, char *buf, int len);
void rtcp_port_reset(void* handle);
void rtcp_port_100ms(void* handle);

#endif//__FCC_PORT_H__
