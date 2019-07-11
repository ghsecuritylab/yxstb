
#ifndef __RTSP_PORT_H__
#define __RTSP_PORT_H__

#include "mid_stream.h"
#include "mid_task.h"
#include "independs/ind_ts.h"
#include "independs/ind_net.h"

#ifdef ENABLE_IPV6
struct ind_sin_ex {
	struct ind_sin host;
	struct ind_sin srcip1;
	struct ind_sin srcip2;
	struct ind_sin srcip3;		
};
#endif

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*stream_msg_handle)(int pIndex, STRM_MSG msg, int arg, unsigned int magic);
typedef void (*stream_state_handle)(int pIndex, STRM_STATE state, int rate, unsigned int magic);

void stream_port_msg_hdl_set(stream_msg_handle msg_hdl);
void stream_port_state_hdl_set(stream_state_handle state_hdl);

void stream_port_hdl_set(void);

void stream_port_task_create(mid_func_t entry, void *arg);

void* stream_port_malloc(int size);
void stream_port_free(void *ptr);

int stream_port_multicast_read(int pIndex, char *addr);
int stream_port_multicast_write(int pIndex, char *addr);

void stream_port_peername(int pIndex, int tcp, unsigned int addr, unsigned short port);

#ifdef ENABLE_IPV6
int stream_port_mldv1_play(void* igmp, int igmp_size, struct ind_sin *mult_sin);
void stream_port_mldv1_stop(void* igmp, int igmp_size, int sock, struct ind_sin *mult_sin);
int stream_port_mldv2_play(void* igmp, int igmp_size, struct ind_sin *mult_sin);
void stream_port_mldv2_stop(void* igmp, int igmp_size, int sock, struct ind_sin *mult_sin);
#endif

#ifdef ENABLE_IGMPV3
int stream_port_multicast_v2_play(void* igmp, int igmp_size, struct ind_sin *mult_sin);
void stream_port_multicast_v2_stop(void* igmp, int igmp_size, int sock, struct ind_sin *mult_sin);
int stream_port_multicast_v3_play(void* igmp, int igmp_size, struct ind_sin *mult_sin);
void stream_port_multicast_v3_stop(void* igmp, int igmp_size, int sock, struct ind_sin *mult_sin);
#endif
#if defined(HUAWEI_C10)
void  tr069_stream_port_multicast(int i);
#endif
int stream_port_multicast_play(u_int addr, unsigned short port);
void stream_port_multicast_stop(int sock, u_int addr);

//播放状态改变
void stream_port_state(int pIndex, STRM_STATE state, int rate, unsigned int magic);
//播放消息
void stream_port_message(int pIndex, STRM_MSG msg, int arg, unsigned int magic);


/* 得到当前rtsp状态 */
int stream_port_get_state(void);
/* 得到当前的播放速度 */
void stream_port_set_rate(int rate,int pIndex);
void stream_port_set_state(int state,int pIndex);

int stream_port_get_rate(void);
void stream_port_post_datavalid(int pIndex, int valid);
int stream_port_get_parameter(int pIndex,int *mode,int *state,int *rate);
int app_stream_get_master(void);

void stream_port_multicast_report(u_int addr);
void stream_multicast_leave(void);
//数据传输通道对方ip和端口
//每次数据通道切换（开始，结束或组播转时移）会调用stream_port_post_datasock通知
/*
	serv_sin 为数据源信息，也就是服务端ip和端口
	data_sin 为数据目的信息，也就是本地结收ip和端口
	对于组播
	serv_sin 值为NULL data_sin 里的ip为组播地址，端口为组播端口，都是网络字节序
	tcp含义
	-1 关闭数据接收
	0 开始udp数据接收，包括组播
	1 开始TCP数据接收
	> 1 播放状态
 */
void stream_port_post_datasock(int pIndex, int tcp, struct sockaddr_in* serv_sin, struct sockaddr_in* data_sin, char *url);
/*
	每次重网络受到一段数据，都会调用stream_port_post_datapush，以便侦听程序遍历一遍数据
	scale 为当时的播放速率，目前只须侦听scale=1也就是正常播放的数据。如果scale != 1,直接return;
 */
void stream_port_post_datapush(int pIndex, char *buf, int len, int seq);

//统计

//断流
void stream_port_post_streamgap(void);

//单播请求性能 单位为 10毫秒
void stream_port_post_vodreq(int ms);
void stream_port_post_vodstop(int ms);
void stream_port_post_vodpause(int ms);
void stream_port_post_vodresume(int ms);
void stream_port_post_vodforward(int ms);
void stream_port_post_vodbackward(int ms);
void stream_port_post_channelzap(int ms);

void stream_port_post_bandwidth(int mult, int width, int rate);
void stream_port_post_bitrate(int mult, int width, int rate);

void stream_port_post_pklosts(int flag, int width, int totals, int losts);
void stream_port_post_bitrate_percent(int mult, int width, int rate);
/*
	mult 0:VOD 1:组播
	rrt 响应时间
 */
void stream_port_post_ok(int mult, int rrt);
void stream_port_post_fail(int mult, char *url, int err_no);

//url igmp开头的是组播
void stream_port_post_abend_fail(char *url);
void stream_port_post_abend_end(void);

void stream_port_post_http(void);
void stream_port_post_http_fail(char *url, int err_no);


/*fow new play lib*/
int stream_port_fsize(char* filepath);
void *stream_port_fopen(char *name, char *op);
int stream_port_fread(void *file, char *buf, int len);
int stream_port_fseek(void *file, int offset);
void stream_port_fclose(void *file);

int stream_port_multicast_get_address(unsigned int *addr, unsigned short *port);

void stream_port_mld_report(struct ind_sin* mld_sin);

int stream_port_in6addr(struct ind_sin* sin) ;

#ifdef __cplusplus
}
#endif


#endif //__RTSP_PORT_H__
