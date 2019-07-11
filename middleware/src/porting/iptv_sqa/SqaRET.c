#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "SysSetting.h"
#include "vde.h"
#include "vde_callback.h"

#include "SqaPublic.h"

/****************RET********************/
static inline void ret_rtp_data_recv_vde_direct(struct RET* ret, int fd, funFccRecPacket *fun)
{
    struct RETArg *arg = &ret->arg;

    sqaRecvData(fd, (STB_HANDLE)(ret->hVOD), fun,
        arg->rtsp, arg->fd_writedata, arg->fd_writeable);
}

void ret_arq_rtp_recv_udp_direct(void *handle, int fd)
{
    struct RET *ret = (struct RET*)handle;

    ret_rtp_data_recv_vde_direct(ret, ret->ret_arq_sock, sqa_recv_unicast_rtp_packet);
}

void ret_arq_rtp_recv_date_udp_direct(void *handle, int fd)
{
    //struct RET *g_ret = (struct RET*)handle;

    //ret_rtp_data_recv_vde_direct(g_ret, g_ret->ret_arq_date_socket, sqa_recv_unicast_rtp_packet);
}

int ret_arq_rtpsocket(struct RET *g_ret)
{
    struct sockaddr_in addr;
    int sock = 0, opt = 0;
    struct RETArg *arg = &g_ret->arg;
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
        LOG_SQA_PRINTF("Can't change system network size (wanted size = %d)\n", opt);
    }
    val = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, val | O_NONBLOCK);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    //addr.sin_port = htons((u_short)(g_ret->ret_port_base+3));
    addr.sin_port = htons((u_short)(arg->clt_port));
    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr))) {
        LOG_SQA_ERROUT("###SQA### bind socket failed %d\n", errno);
    }
    if(g_ret->ret_arq_sock != -1) {
        arg->fd_unregist(arg->rtsp, g_ret->ret_arq_sock);
        close(g_ret->ret_arq_sock);
    }
    g_ret->ret_arq_sock = sock;
    LOG_SQA_PRINTF("###SQA### ret_arq:%d:%d\n", arg->clt_port, g_ret->ret_arq_sock);
    arg->fd_regist(arg->rtsp, g_ret, sock, ret_arq_rtp_recv_udp_direct);
    return 0;
Err:
    if(sock >= 0) {
        close(sock);
    }
    return -1;
}

#if 0
int ret_arq_datesocket(struct RET* g_ret)
{
    struct sockaddr_in addr;
    int sock = 0, opt = 0;
    struct RETArg *arg = &g_ret->arg;
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
    addr.sin_port = htons((u_short)(g_ret->ret_port_base + 2)); //fcc->fcc_rtp_port );
    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr))) {
        LOG_SQA_ERROUT("###SQA### bind socket failed %d\n", errno);
    }
    if(g_ret->ret_arq_date_socket != -1) {
        arg->fd_unregist(arg->rtsp, g_ret->ret_arq_date_socket);
        close(g_ret->ret_arq_date_socket);
    }
    g_ret->ret_arq_date_socket = sock;
    arg->fd_regist(arg->rtsp, g_ret, sock, ret_arq_rtp_recv_date_udp_direct);
    LOG_SQA_PRINTF("###SQA### a2:%d;%d\n", g_ret->ret_port_base + 2, g_ret->ret_arq_date_socket);
    return 0;
Err:
    if(sock >= 0) {
        close(sock);
    }
    return -1;
}
#endif

/*
   申请并打开ret, 端口统一后， 不在需要建立接收重传socket
 */
void *ret_port_open(struct RETArg* arg)
{
    LOG_SQA_PRINTF("###SQA### ret_port_open \n");

    if(!arg) {
        LOG_SQA_ERROR("###SQA### arg is NULL\n");
        return NULL;
    }
    struct RET *ret = (struct RET*)malloc(sizeof(struct RET));
    if(!ret) {
        LOG_SQA_ERROR("###SQA### malloc!\n");
        return NULL;
    }

    memcpy(&ret->arg, arg, sizeof(struct RETArg));
    ret->port_off = 3;
    ret->ret_arq_sock = -1;
    ret_arq_rtpsocket(ret);
    arg = &ret->arg;

    ret->ret_ip = arg->sin.in_addr.s_addr;

    ret->ret_port = SqaGetPort( );

    LOG_SQA_PRINTF("###SQA### g_ret->ret_ip=%08x:%d\n", ret->ret_ip, (int)ret->ret_port);
    ret->hVOD = yx_RET_VDE_init(ret, 1);

    return ret;
}

void ret_port_reset(void* handle)
{
    struct RET *g_ret = NULL;
    StreamParam spArgs;
    STB_HANDLE sHandle;
    SQACallBackFuns sqacbfs;
    int type = 0;

    LOG_SQA_PRINTF("###SQA### ret_port_reset\n");
    if(handle == NULL) {
        LOG_SQA_ERROUT("###SQA### handle = %p, g_ret = %p\n", handle, g_ret);
    }
    g_ret = (struct RET *)handle;
    if(g_ret->hVOD) {
        sqa_reset_buf(g_ret->hVOD);
    }
    sqacbfs.hClient = (STB_HANDLE)g_ret;
    sqacbfs.fcbSendFCC = yx_funcCallBackSendFCC;
    sqacbfs.fcbSendRET = yx_funcCallBackSendFeedBack;
    sqacbfs.fcbSendNAT = yx_funcCallBackSendNatData;
    sqacbfs.fcbIGMPJoin = yx_funcCallBackIGMPJoin;
    sqacbfs.fcbDataBufferLevel = yx_funcCallBackDataBufferLevel;
    sqacbfs.fcbFccStateRsp = yx_funcCallBackTellFccStateRsp;
    sqacbfs.fcbFccIP = yx_funcCallBackFccIP;
    sqacbfs.fcbFCCPort = yx_funcCallBackFccPort;
    sqacbfs.fcbValidTime = yx_funcCallBackValidTime;
    sqacbfs.fcbhmsRsp = yx_funcCallBackHmsRsp;
    sqacbfs.fcbGetIP = yx_funcCallBackGetIP_RET;
    type = RET_CODE;
    spArgs.cType = type;
    LOG_SQA_PRINTF("###SQA### pArgs.cType = %x\n", spArgs.cType);
    spArgs.cRetType = HW;

    spArgs.port = SqaGetPort( );

    spArgs.cIPType = IPV4;
    spArgs.iSendRSRFlag = 0;
    spArgs.iFCCWithTLV = 0;
    spArgs.iFastCacheOn = 1;
    spArgs.iChanCacheTime = 100;
    spArgs.iRetDelayTime = 50;
    spArgs.iRetInterval = 300;
    //sHandle = hSTB->hFcc;
    spArgs.iFCCServerFlag = 0;
    spArgs.iNatSupport = 0;
    spArgs.iRetInterval = 0;
    spArgs.iclientSSRC = 0;
    sHandle = g_ret->hVOD;
    sqa_set_parameter(sHandle, &spArgs, &sqacbfs);

Err:
    return;

}
/*
   关闭并释放ret
 */
void ret_port_close(void* handle)
{
    struct RETArg *arg;
    struct RET *g_ret = NULL;

    if(handle == NULL) {
        LOG_SQA_ERROUT("handle = %p, g_ret = %p\n", handle, g_ret);
    }
    g_ret = (struct RET *)handle;
    arg = &g_ret->arg;
    if(g_ret->hVOD) {
        yx_SQA_VDE_Destroy(g_ret->hVOD);
    }
    if(g_ret->ret_arq_sock > 0) {
        arg->fd_unregist(arg->rtsp, g_ret->ret_arq_sock);
        close(g_ret->ret_arq_sock);
        g_ret->ret_arq_sock = -1;
    }
    g_ret->hVOD = NULL;
    free(g_ret);
    g_ret = NULL;
Err:
    return;
}

void ret_port_push(void* handle, char *buf, int len)
{
    struct RET* g_ret = NULL;
    struct RETArg *arg = NULL;
    void *buffer = NULL;
    STB_HANDLE hVOD;
    int back = 0;
    STB_INT32 length = 0;
    STB_INT32  ret = 0;	//WZW modified to fix pc-lint warning 650

    g_ret = (struct RET*)handle;
    if(handle == NULL) {
        LOG_SQA_ERROUT("###SQA### handle = %p\n", handle);
    }
    arg = &g_ret->arg;
    if(NULL == buf || len <= 0) {
        LOG_SQA_ERROUT("###SQA### buf=%p, len = %d\n", buf, len);
    }
    if(0x47 == *buf) {
        arg->fd_writedata(arg->rtsp, buf, len);
        LOG_SQA_PRINTF("###SQA### TS!\n");
        return;
    }
    hVOD = g_ret->hVOD;
    ret = sqa_get_buf(hVOD, &buffer, &length);
    if(-1 == ret) {
        LOG_SQA_PRINTF("###SQA### get_rtp_buf NULL!\n");
        return;
    }
    memcpy(buffer, buf, len);
    back = sqa_recv_unicast_rtp_packet(hVOD, buffer, len);
    if(0 != back) {                                         /*压入异常情况释放空间*/
        sqa_free_rtp_packet(hVOD, buffer);
        LOG_SQA_PRINTF("###SQA### PUT ERROR = %d!\n", back);
    }

    SqaHandleEvent(hVOD, arg->rtsp, arg->fd_writedata, arg->fd_writeable);
Err:
    return;
}

int ret_port_pop(void* handle)
{
    struct RET* g_ret = NULL;
    struct RETArg *arg = NULL;
    void *buffer = NULL;
    STB_HANDLE hVOD;
    STB_UINT32 length = 0;
    STB_UINT32 ret = 0;

    //LOG_SQA_PRINTF("###SQA### ret_port_pop\n");
    g_ret = (struct RET*)handle;
    if(handle == NULL) {
        LOG_SQA_ERROUT("handle = %p\n", handle);
    }
    arg = &g_ret->arg;
    hVOD = g_ret->hVOD;
    ret = sqa_get_rtp_packet(hVOD, &buffer, &length, 0);
    if(ret == 0) {
        arg->fd_writedata(arg->rtsp, (char*)buffer, length);
        sqa_free_rtp_packet(hVOD, NULL);
    } else {
        length = 0;
    }
    return length;
Err:
    return 0;
}

void ret_port_cache(void* handle, int on)
{
    struct RET *g_ret = NULL;

    g_ret = (struct RET*)handle;
    if(g_ret) {
        if(FAST_CACHE_OFF == on) {
            on = FAST_CACHE_ON;
        } else if(FAST_CACHE_ON == on) {
            on = FAST_CACHE_OFF;
        }
        sqa_set_fastcache_state(g_ret->hVOD, on);
        LOG_SQA_PRINTF("on = %d\n", on);
    }
}

/* rtspÖ÷Ïß³Ì»á100ºÁÃëÖÓµ÷ÓÃÒ»´Î */
void ret_port_100ms(void* handle)
{
    struct RET *g_ret = (struct RET *)handle;

    if(g_ret) {
        yx_SQA_VDE_handleEvent(g_ret->hVOD);
    }
    return;
}
