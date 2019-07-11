#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#include "SysSetting.h"
#include "sys_basic_macro.h"
#include "vde.h"
#include "vde_callback.h"

#include "SqaPublic.h"

static pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;

static int fccportoffset = 0;

static unsigned int save_server_ip = 0;

static int fcc_rtcp_open(struct FCC* fcc);
static int fcc_fcc_rtpsocket(struct FCC *fcc);
static int fcc_arq_rtpsocket(struct FCC *fcc);
static int fcc_arq_datesocket(struct FCC *fcc);
static inline void rtp_data_recv_vde_direct(struct FCC *fcc, int fd, funFccRecPacket *fun);
static inline void rtp_data_recv_recover_vde_direct(struct FCC *fcc, int fd, funFccRecPacket *fun);
/****************RET********************/
static inline void ret_rtp_data_recv_vde_direct(struct RET *g_ret, int fd, funFccRecPacket *fun);

extern int fcc_server_getMax(void);
extern int fcc_server_get(int index, char *ip, unsigned short *port);

static int fcc_rtcp_open(struct FCC *fcc)
{
    struct sockaddr_in addr;
    int sock = 0, opt = 0;
    struct FCCArg *arg = &fcc->arg;
    int val = 0;
    int base_port = 0;
    int port = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) {
        LOG_SQA_ERROUT("###SQA### socket failed\n");
    }
    opt = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt)) < 0) {
        LOG_SQA_ERROUT("###SQA### setsockopt SO_REUSEADDR");
    }
    opt = 1024 * 200;
    if(setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&opt, sizeof(opt)) < 0) {
        LOG_SQA_PRINTF("###SQA### Can't change system network size (wanted size = %d)\n", opt);
    }
    val = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, val | O_NONBLOCK);
    pthread_mutex_lock(&gMutex);
    base_port = fccportoffset + PORT_BASE_VALUE + PORT_MAX_OFFSET + 12;  //PORT_BASE_VALUE_2
    fccportoffset = fccportoffset + 4;
    if(fccportoffset > 200) {
        fccportoffset = 0;
    }
    pthread_mutex_unlock(&gMutex);
    port = base_port;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port + 1);
    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr))) {
        LOG_SQA_ERROUT("###SQA### bind socket failed %d\n", errno);
    }
    fcc->fcc_port_base = port;
    if(fcc->fcc_rtcp_sock != -1) {
        arg->fd_unregist(arg->rtsp, fcc->fcc_rtcp_sock);
        close(fcc->fcc_rtcp_sock);
    }
    fcc->fcc_rtcp_sock = sock;
    arg->fd_regist(arg->rtsp, fcc, sock, rcc_recv_rtcp);
    LOG_SQA_PRINTF("###SQA### %s: fcc_port_base+1 = %d sock = %d\n", __func__, fcc->fcc_port_base + 1, sock);
    return 0;
Err:
    if(sock >= 0) {
        close(sock);
    }
    return -1;
}

static int fcc_fcc_rtpsocket(struct FCC *fcc)
{
    struct sockaddr_in addr;
    int sock = 0, opt = 0;
    int val = 0;
    struct FCCArg *arg = &fcc->arg;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) {
        LOG_SQA_ERROUT("###SQA### socket failed errno=%d\n", errno);
    }
    opt = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt)) < 0) {
        LOG_SQA_ERROUT("###SQA### setsockopt SO_REUSEADDR");
    }
    opt = 1024 * 800;
    if(setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&opt, sizeof(opt)) < 0) {
        LOG_SQA_PRINTF("###SQA### Can't change system network size (wanted size = %d)\n", opt);
    }
    val = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, val | O_NONBLOCK);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;//inet_addr(fcc->fcc_ip);//INADDR_ANY;
    addr.sin_port = htons((u_short)(fcc->fcc_port_base));//fcc->fcc_rtp_port);
    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr))) {
        LOG_SQA_ERROUT("###SQA### bind socket failed %d\n", errno);
    }
    if(fcc->fcc_rtp_sock != -1) {
        arg->fd_unregist(arg->rtsp, fcc->fcc_rtp_sock);
        close(fcc->fcc_rtp_sock);
    }
    fcc->fcc_rtp_sock = sock;
    arg->fd_regist(arg->rtsp, fcc, sock, fcc_rtp_recv_udp_direct);
    LOG_SQA_PRINTF("###SQA### %s: fcc->fcc_port_base=%d:sock=%d\n", __func__, fcc->fcc_port_base, sock);
    return 0;
Err:
    if(sock >= 0) {
        close(sock);
    }
    return -1;
}

static int fcc_arq_rtpsocket(struct FCC *fcc)
{
    struct sockaddr_in  addr;
    int sock = 0, opt = 0;
    struct FCCArg *arg = &fcc->arg;
    int val = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) {
        LOG_SQA_ERROUT("###SQA### socket failed\n");
    }
    opt = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt)) < 0) {
        LOG_SQA_ERROUT("###SQA### setsockopt SO_REUSEADDR");
    }
    opt = 1024 * 200;
    if(setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&opt, sizeof(opt)) < 0) {
        LOG_SQA_PRINTF("###SQA### Can't change system network size (wanted size = %d)\n", opt);
    }
    val = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, val | O_NONBLOCK);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;//inet_addr(fcc->fcc_ip);//INADDR_ANY;
    addr.sin_port = htons((u_short)(fcc->fcc_port_base + 3)); //fcc->fcc_rtp_port );
    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr))) {
        LOG_SQA_ERROUT("###SQA### bind socket failed %d\n", errno);
    }
    if(fcc->fcc_arq_sock != -1) {
        arg->fd_unregist(arg->rtsp, fcc->fcc_arq_sock);
        close(fcc->fcc_arq_sock);
    }
    fcc->fcc_arq_sock = sock;
    LOG_SQA_PRINTF("###SQA### %s:fcc->fcc_port_base+3=%d:fcc->fcc_arq_sock=%d\n", __func__, fcc->fcc_port_base + 3, fcc->fcc_arq_sock);
    arg->fd_regist(arg->rtsp, fcc, sock, arq_rtp_recv_udp_direct);
    return 0;
Err:
    if(sock >= 0) {
        close(sock);
    }
    return -1;
}

static int fcc_arq_datesocket(struct FCC *fcc)
{
    struct sockaddr_in addr;
    int sock = 0, opt = 0;
    struct FCCArg *arg = &fcc->arg;
    int val = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) {
        LOG_SQA_ERROUT("###SQA### socket failed\n");
    }
    opt = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt)) < 0) {
        LOG_SQA_ERROUT("###SQA### setsockopt SO_REUSEADDR");
    }
    opt = 1024 * 800;
    if(setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&opt, sizeof(opt)) < 0) {
        LOG_SQA_PRINTF("###SQA### Can't change system network size (wanted size = %d)\n", opt);
    }
    val = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, val | O_NONBLOCK);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;//inet_addr(fcc->fcc_ip);//INADDR_ANY;
    addr.sin_port = htons((u_short)(fcc->fcc_port_base + 2)); //fcc->fcc_rtp_port );
    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr))) {
        LOG_SQA_ERROUT("###SQA### bind socket failed %d\n", errno);
    }
    if(fcc->fcc_arq_date_socket != -1) {
        arg->fd_unregist(arg->rtsp, fcc->fcc_arq_date_socket);
        close(fcc->fcc_arq_date_socket);
    }
    fcc->fcc_arq_date_socket = sock;
    arg->fd_regist(arg->rtsp, fcc, sock, arq_rtp_recv_date_udp_direct);
    LOG_SQA_PRINTF("###SQA### %s:fcc->fcc_port_base+2=%d;fcc->fcc_arq_date_socket=%d\n", __func__, fcc->fcc_port_base + 2, fcc->fcc_arq_date_socket);
    return 0;
Err:
    if(sock >= 0) {
        close(sock);
    }
    return -1;
}

/* 播放数据包处理函数 ，包括单播和组播 */
static inline void rtp_data_recv_vde_direct(struct FCC *fcc, int fd, funFccRecPacket *fun)
{
    struct FCCArg *arg = &fcc->arg;

    sqaRecvData(fd, (STB_HANDLE)(fcc->hFcc), fun,
        arg->rtsp, arg->fd_writedata, arg->fd_writeable);
}

/*补偿包的处理函数*/
static inline void rtp_data_recv_recover_vde_direct(struct FCC *fcc, int fd, funFccRecPacket *fun)
{
    struct FCCArg *arg = &fcc->arg;

    sqaRecvData(fd, (STB_HANDLE)(fcc->hFcc), fun,
        arg->rtsp, arg->fd_writedata, arg->fd_writeable);
}

/*  组播流接收数据 */
void fcc_recv_base_mult(void *handle, int fd)
{
    struct FCC* fcc = (struct FCC*)handle;

    if (fcc->mult_flag < 2)
        fcc->mult_flag = 2;
    //LOG_SQA_PRINTF("###SQA### recv base mult packet!\n");
    rtp_data_recv_vde_direct(fcc, fcc->mult_base_sock, sqa_recv_multicast_rtp_packet);
}

/*  fec 打开时， 补偿流接收数据  */
void fcc_recv_recover_mult(void *handle, int fd)
{
    struct FCC* fcc = (struct FCC*)handle;

    // LOG_SQA_PRINTF("###SQA### recv recover packet!\n");
    rtp_data_recv_recover_vde_direct(fcc, fcc->mult_recover_sock, sqa_recv_multicast_rtp_packet);
}

/* 接收服务器端的回应 */
void rcc_recv_rtcp(void *handle, int fd)
{
    int len = 0;
    int sqa_ret = 0;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    void *buffer = NULL;

    struct FCC *fcc = (struct FCC*)handle;

    sqa_ret = sqa_get_buf(fcc->hFcc, &buffer, &len);
    if(-1 == sqa_ret || NULL == buffer) {
        LOG_SQA_PRINTF("###SQA### get rtp buf NULL!\n");
        return;
    }
    len = recvfrom(fcc->fcc_rtcp_sock, buffer, RTP_BUFFER_SIZE, 0, (struct sockaddr*)&addr, &addr_len);
    if(len <= 0) {
        LOG_SQA_PRINTF("###SQA### get rtp buf NULL!\n");
        return;
    }

    LOG_SQA_PRINTF("###SQA### recvfrom fcc_server =%08x\n", addr.sin_addr.s_addr);
	if((fcc->arg.type & SQA_FCC_CODE) && addr.sin_addr.s_addr != save_server_ip) {
        fcc->fcc_ip = addr.sin_addr.s_addr; //先把FCC地址保存起来，随后在写到频道列表酿
        save_server_ip = addr.sin_addr.s_addr; //增加变量 save_server_ip, 修改防止第二个RSI冲掉保存的fcc 地址.
	}
    sqa_ret = yx_SQA_VDE_recvRtcpPacket(fcc->hFcc, buffer, len);//sqa_recv_rtcp return error value! 0 == ok , -2 == double, -1 == extremely
    LOG_SQA_PRINTF("###SQA### sqa_recv_rtcp return %d\n", sqa_ret);
    sqa_free_rtp_packet(fcc->hFcc, buffer);
    return;
}

/*接收单播包*/
void fcc_rtp_recv_udp_direct(void *handle, int fd)
{
    struct FCC *fcc = (struct FCC*)handle;

    rtp_data_recv_vde_direct(fcc, fcc->fcc_rtp_sock, sqa_recv_unicast_rtp_packet);
}

/*接收重传包*/
void arq_rtp_recv_udp_direct(void *handle, int fd)
{
    struct FCC *fcc = (struct FCC*)handle;

    rtp_data_recv_vde_direct(fcc, fcc->fcc_arq_sock, sqa_recv_unicast_rtp_packet);
}

/*接收重传包*/
void arq_rtp_recv_date_udp_direct(void *handle, int fd)
{
    struct FCC *fcc = (struct FCC*)handle;

    rtp_data_recv_vde_direct(fcc, fcc->fcc_arq_date_socket, sqa_recv_unicast_rtp_packet);
}

/*申请并打开fcc,  目前fec 借用fcc 的操作接口*/
void *fcc_port_open(struct FCCArg *arg)
{
    struct FCC *fcc;
    struct ind_sin *sin;

    int flag = 0;
 
    save_server_ip = '\0';

    LOG_SQA_PRINTF("###SQA### FCC port open \n");

    if(!arg) {
        LOG_SQA_ERROR("###SQA### arg is NULL\n");
        return NULL;
    }
    fcc = (struct FCC*)calloc(sizeof(struct FCC), 1);
    if(!fcc) {
        LOG_SQA_ERROR("###SQA### malloc error!\n");
        return NULL;
    }

    memcpy(&fcc->arg, arg, sizeof(struct FCCArg));
    fcc->fcc_rtcp_sock = -1;
    fcc->fcc_rtp_sock = -1;
    fcc->fcc_arq_sock = -1;
    fcc->fcc_arq_date_socket = -1;
    fcc->mult_flag = 0;
    fcc->mult_base_sock = -1;
    fcc->mult_recover_sock = -1;
    fcc->port_off = 3;

    fcc_rtcp_open(fcc);                                   /*didn't close*/
    fcc_fcc_rtpsocket(fcc);                               /* ret use the same socket as the fcc */

    sin = &fcc->arg.sin;
    LOG_SQA_PRINTF("###SQA### ChannelURL=%08x\n", sin->in_addr.s_addr);
    channel_array_get_server(sin->in_addr.s_addr, sin->port, &fcc->fcc_ip, &flag);
    fcc->fcc_rtcp_port = SqaGetPort( );

    LOG_SQA_PRINTF("###SQA### g_fcc->fcc_ip=%08x %d\n", fcc->fcc_ip, (int)fcc->fcc_rtcp_port);
    if(2 == flag)
        fcc->hFcc = yx_FCC_VDE_init(fcc, FCC_SERVER_FCC);
    else
        fcc->hFcc = yx_FCC_VDE_init(fcc, FCC_SERVER_RRS);

    return (void *)fcc;
}

int fcc_port_mflag(void* handle)
{
    struct FCC *fcc = (struct FCC *)handle;
    return fcc->mult_flag;
}

/*关闭并释放fcc */
void fcc_port_close(void* handle)
{
    struct FCCArg *arg;
    struct FCC *g_fcc = (struct FCC *)handle;

    if(handle == NULL) {
        LOG_SQA_ERROUT("###SQA### handle = %p, g_fcc = %p\n", handle, g_fcc);
    }
    arg = &g_fcc->arg;
    if(g_fcc->hFcc) {
        yx_SQA_VDE_Destroy(g_fcc->hFcc);
    }
    if(g_fcc->mult_base_sock > 0) {
        arg->fd_unregist(arg->rtsp, g_fcc->mult_base_sock);
        stream_port_multicast_stop(g_fcc->mult_base_sock, arg->sin.in_addr.s_addr);
        g_fcc->mult_base_sock = -1;
    }
    if(g_fcc->mult_recover_sock > 0) {
        arg->fd_unregist(arg->rtsp, g_fcc->mult_recover_sock);
        stream_port_multicast_stop(g_fcc->mult_recover_sock, arg->sin.in_addr.s_addr);
        g_fcc->mult_recover_sock = -1;
    }
    if(g_fcc->fcc_rtcp_sock > 0) {
        arg->fd_unregist(arg->rtsp, g_fcc->fcc_rtcp_sock);
        close(g_fcc->fcc_rtcp_sock);
        g_fcc->fcc_rtcp_sock = -1;
    }
    if(g_fcc->fcc_rtp_sock > 0) {
        arg->fd_unregist(arg->rtsp, g_fcc->fcc_rtp_sock);
        close(g_fcc->fcc_rtp_sock);
        g_fcc->fcc_rtp_sock = -1;
    }
    if(g_fcc->fcc_arq_sock > 0) {
        arg->fd_unregist(arg->rtsp, g_fcc->fcc_arq_sock);
        close(g_fcc->fcc_arq_sock);
        g_fcc->fcc_arq_sock = -1;
    }
    if(g_fcc->fcc_arq_date_socket > 0) {
        arg->fd_unregist(arg->rtsp, g_fcc->fcc_arq_date_socket);
        close(g_fcc->fcc_arq_date_socket);
        g_fcc->fcc_arq_date_socket = -1;
    }
    g_fcc->hFcc = NULL;
    free(g_fcc);
    g_fcc = NULL;
Err:
    return;
}

/*rtsp主线程会100毫秒钟调用一次，sqa_handle_event函数驱动重传、解码、异常处理 */
void fcc_port_100ms(void* handle)
{
    struct FCC *g_fcc = NULL;

    if(handle) {
        g_fcc = (struct FCC *)handle;
        yx_SQA_VDE_handleEvent(g_fcc->hFcc);
    }
    return;
}

