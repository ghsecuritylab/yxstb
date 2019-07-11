
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

//����״̬�ı�
void stream_port_state(int pIndex, STRM_STATE state, int rate, unsigned int magic);
//������Ϣ
void stream_port_message(int pIndex, STRM_MSG msg, int arg, unsigned int magic);


/* �õ���ǰrtsp״̬ */
int stream_port_get_state(void);
/* �õ���ǰ�Ĳ����ٶ� */
void stream_port_set_rate(int rate,int pIndex);
void stream_port_set_state(int state,int pIndex);

int stream_port_get_rate(void);
void stream_port_post_datavalid(int pIndex, int valid);
int stream_port_get_parameter(int pIndex,int *mode,int *state,int *rate);
int app_stream_get_master(void);

void stream_port_multicast_report(u_int addr);
void stream_multicast_leave(void);
//���ݴ���ͨ���Է�ip�Ͷ˿�
//ÿ������ͨ���л�����ʼ���������鲥תʱ�ƣ������stream_port_post_datasock֪ͨ
/*
	serv_sin Ϊ����Դ��Ϣ��Ҳ���Ƿ����ip�Ͷ˿�
	data_sin Ϊ����Ŀ����Ϣ��Ҳ���Ǳ��ؽ���ip�Ͷ˿�
	�����鲥
	serv_sin ֵΪNULL data_sin ���ipΪ�鲥��ַ���˿�Ϊ�鲥�˿ڣ����������ֽ���
	tcp����
	-1 �ر����ݽ���
	0 ��ʼudp���ݽ��գ������鲥
	1 ��ʼTCP���ݽ���
	> 1 ����״̬
 */
void stream_port_post_datasock(int pIndex, int tcp, struct sockaddr_in* serv_sin, struct sockaddr_in* data_sin, char *url);
/*
	ÿ���������ܵ�һ�����ݣ��������stream_port_post_datapush���Ա������������һ������
	scale Ϊ��ʱ�Ĳ������ʣ�Ŀǰֻ������scale=1Ҳ�����������ŵ����ݡ����scale != 1,ֱ��return;
 */
void stream_port_post_datapush(int pIndex, char *buf, int len, int seq);

//ͳ��

//����
void stream_port_post_streamgap(void);

//������������ ��λΪ 10����
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
	mult 0:VOD 1:�鲥
	rrt ��Ӧʱ��
 */
void stream_port_post_ok(int mult, int rrt);
void stream_port_post_fail(int mult, char *url, int err_no);

//url igmp��ͷ�����鲥
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
