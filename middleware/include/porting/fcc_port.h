
#ifndef __FCC_PORT_H__
#define __FCC_PORT_H__

struct RTSP;

#include "ind_net.h"

/*
	FCC 嵌套进 RTSP 线程里面，FCC所用到的fd注册到RTSP中，同时RTSP会没100毫秒调用一次FCC的周期执行函数
	FCC 功能开启时，RTSP线程不再维护多播组，由FCC自行控制
 */

typedef void (*fd_recv_f)(void *fcc, int fd);
typedef int (*fd_regist_f)(struct RTSP *rtsp, void *fcc, int fd, fd_recv_f call);
typedef void (*fd_unregist_f)(struct RTSP *rtsp, int fd);
typedef int (*fd_datasize_f)(struct RTSP *rtsp);
typedef void (*fd_writedata_f)(struct RTSP *rtsp, char *buf, int len);
typedef int (*fd_writeable_f)(struct RTSP *rtsp);
/*
	数据统计，统计FEC原始丢包率
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
	int					flag;//0: 切台播放，1：时移转直播

	unsigned short		pip;//PIP标志 1 表示PIP 0 表示普通播放
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

	unsigned short clt_port;//rtcp 本地端口
	unsigned short srv_port;//rtcp 服务端端口
	cache_on_f cache_on;//开启缓冲
	cache_off_f cache_off;//关闭缓冲

	rtsp_pktfb_f rtsp_pktfb;//丢包通告
	struct ind_sin sin;
};

/*
	申请并打开fcc
	v2 为支持IPv6，参数发生了变更，为保持
	v3 增加 rtsp_datastat_f
	v4 增加 flag
 */
#define fcc_port_open	fcc_port_open_v4
void *fcc_port_open(struct FCCArg* arg);

/*
    1 加入组播
    2 加入组播并收到数据
 */
int fcc_port_mflag(void* handle);
/*
	关闭并释放fcc
 */
void fcc_port_close(void* handle);

/*
	rtsp主线程会100毫秒钟调用一次
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
