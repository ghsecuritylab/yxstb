#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vde.h"
#include "vde_callback.h"

#include "SqaPublic.h"

void *rtcp_port_open(struct RETArg* arg)
{
    struct RET *ret = NULL;
    struct sockaddr_in sin;

    LOG_SQA_PRINTF("###SQA### rtcp_port_open \n");

    if (!arg) {
        LOG_SQA_ERROR("arg is NULL\n");
        return NULL;
    }
    ret = (struct RET*)malloc(sizeof(struct RET));
    if (!ret) {
        LOG_SQA_ERROR("malloc!\n");
        return NULL;
    }

    memcpy(&ret->arg, arg, sizeof(struct RETArg));

    ret->port_off = 3;
    ret->ret_arq_sock = -1;

    arg = &ret->arg;
    ret->ret_ip = arg->sin.in_addr.s_addr;

    ret->ret_port = 0;

    LOG_SQA_PRINTF("g_ret->ret_ip=%08x:%d\n", ret->ret_ip,(int)ret->ret_port);

    ret->hVOD = yx_RTCP_VDE_init(ret);
    LOG_SQA_PRINTF("hVOD = 0x%x, ret_ip=%s, port_off=%d, ret_port=%d, ret_arq_sock=%d, arg->server=%u, arg->bitrate=%d, arg->clt_port=%d, arg->srv_port=%d\n", ret->hVOD, ret->ret_ip, ret->port_off, (int)ret->ret_port, ret->ret_arq_sock, ret->arg.sin.in_addr, ret->arg.bitrate, ret->arg.clt_port, ret->arg.srv_port);
    return ret;
}

/*
    ¹Ø±Õ²¢ÊÍ·Årtcp
 */
void rtcp_port_close(void* handle)
{
    struct RETArg *arg;
    struct RET *g_ret = NULL;
    if (handle == NULL )
        LOG_SQA_ERROUT("handle = %p, g_ret = %p\n", handle, g_ret);
    g_ret = (struct RET *)handle;
    arg = &g_ret->arg;

    if(g_ret->hVOD)
        yx_SQA_VDE_Destroy(g_ret->hVOD);

    if(g_ret->ret_arq_sock > 0)
    {
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

void rtcp_port_push(void* handle, char *buf, int len)
{
	ret_port_push(handle, buf, len);
}

void rtcp_port_reset(void* handle)
{
    struct RETArg *arg = NULL;
    struct RET *g_ret = NULL;
    int type;

    LOG_SQA_WARN("###SQA### ret_port_reset\n");
    g_ret = (struct RET *)handle;
    if(g_ret == NULL) {
        LOG_SQA_ERROUT("###SQA### handle = %p, g_ret = %p\n", handle, g_ret);
    }
    arg = &g_ret->arg;
    if(g_ret->hVOD) {
        sqa_reset_buf(g_ret->hVOD);
    }
Err:
    return ;
}

/*
    rtspÖ÷Ïß³Ì»á100ºÁÃëÖÓµ÷ÓÃÒ»´Î
 */
void rtcp_port_100ms(void* handle)
{
    if(NULL == handle)
        return;

    struct RET *g_ret= (struct RET *)handle;

    if (g_ret)
        yx_SQA_VDE_handleEvent(g_ret->hVOD);

    return;
}

