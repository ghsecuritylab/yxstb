#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "SysSetting.h"
#include "vde.h"
#include "vde_callback.h"

#include "SqaPublic.h"

void *arq_port_open(struct RETArg* arg)
{
    struct RET *ret = NULL;

    LOG_SQA_PRINTF("###SQA### arq_port_open\n");

    if(!arg) {
        LOG_SQA_ERROR("###SQA### arg is NULL\n");
        return NULL;
    }
    ret = (struct RET*)malloc(sizeof(struct RET));
    if(!ret) {
        LOG_SQA_ERROR("###SQA### malloc!\n");
        return NULL;
    }
    memcpy(&ret->arg, arg, sizeof(struct RETArg));
    ret->port_off = 3;
    ret->ret_arq_sock = -1;
    arg = &ret->arg;

    ret->ret_ip = arg->sin.in_addr.s_addr;
    ret->ret_port = 0;
    LOG_SQA_PRINTF("###SQA### g_ret->ret_ip=%08x:%d\n", ret->ret_ip, (int)ret->ret_port);
    ret->hVOD = yx_ARQ_VDE_init(ret);
    LOG_SQA_PRINTF("###SQA### hVOD = 0x%x, ret_ip=%08x, port_off=%d, ret_port=%d, ret_arq_sock=%d, arg->server=%u, arg->bitrate=%d, arg->clt_port=%d, arg->srv_port=%d\n", ret->hVOD, ret->ret_ip, ret->port_off, (int)ret->ret_port, ret->ret_arq_sock, ret->arg.sin.in_addr, ret->arg.bitrate, ret->arg.clt_port, ret->arg.srv_port);

    return ret;
}

/* 关闭并释放arq */
void arq_port_close(void* handle)
{
    struct RETArg *arg;
    struct RET* g_ret = NULL;

    g_ret = (struct RET *)handle;
    if(g_ret == NULL) {
        LOG_SQA_ERROUT("###SQA### handle = %p, g_ret = %p\n", handle, g_ret);
    }
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

void arq_port_push(void* handle, char *buf, int len)
{
    ret_port_push(handle, buf, len);
}

void arq_port_reset(void* handle)
{
    struct RETArg *arg = NULL;
    struct RET *g_ret = NULL;
    StreamParam spArgs;
    STB_HANDLE sHandle;
    SQACallBackFuns sqacbfs;
    int type;

    LOG_SQA_WARN("###SQA### ret_port_reset\n");
    g_ret = (struct RET *)handle;
    if(g_ret == NULL) {
        LOG_SQA_ERROUT("###SQA### handle = %p, g_ret = %p\n", handle, g_ret);
    }
    memset(&sqacbfs, 0, sizeof(SQACallBackFuns));
    arg = &g_ret->arg;
    if(g_ret->hVOD) {
        sqa_reset_buf(g_ret->hVOD);
    }
    sqacbfs.hClient = (STB_HANDLE)g_ret;
    sqacbfs.fcbSendFCC = yx_funcCallBackSendFCC;
    sqacbfs.fcbSendRET = yx_funcCallBackSendFeedBack_ARQ;
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
    spArgs.cRetType = CTC;
    spArgs.port = SqaGetPort( );
    spArgs.cIPType = IPV4;
    spArgs.iSendRSRFlag = 0;
    spArgs.iFCCWithTLV = 0;
    spArgs.iFastCacheOn = 0;
    spArgs.iChanCacheTime = 100;
    spArgs.iRetDelayTime = 50;
    spArgs.iRetInterval = 300;
    sHandle = g_ret->hVOD;
    sqa_set_parameter(sHandle, &spArgs, &sqacbfs);
Err:
    return ;
}

/* rtsp主线程会100毫秒种调用一次*/
void arq_port_100ms(void *handle)
{
    struct RET *g_ret = (struct RET *)handle;

    if(g_ret) {
        yx_SQA_VDE_handleEvent(g_ret->hVOD);
    }
    return;
}
