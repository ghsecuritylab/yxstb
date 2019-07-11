#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>

#include "vde.h"
#include "vde_callback.h"
#include "SqaPublic.h"

#include "sys_basic_macro.h"
#include "LogModule.h"
#include "NetworkFunctions.h"

static unsigned short FCC_Nat_port = 0;
//channel private
void yx_funcCallBackSendFCC(STB_HANDLE hClient, STB_UINT8* pRtcpData, STB_UINT32 nRtcpLen)
{
    struct FCC *fcc = (struct FCC *)hClient;
    struct sockaddr_in addr;

    if(fcc->fcc_rtcp_sock == -1) {
        LOG_SQA_PRINTF("fcc_rtcp_sock == -1!\n");
        return;
    }

    if(!fcc->fcc_ip || fcc->fcc_rtcp_port == 0) {
        LOG_SQA_ERROR("send FCC request ip %s or port %d is 0\n", fcc->fcc_ip, (int)fcc->fcc_rtcp_port);
        return;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = fcc->fcc_ip;
    addr.sin_port = htons((u_short)fcc->fcc_rtcp_port);
    ind_net_sendto(fcc->fcc_rtcp_sock, pRtcpData, nRtcpLen, MSG_NOSIGNAL, &addr);
    LOG_SQA_PRINTF("send FCC message ip %s:%d\n", fcc->fcc_ip, (int)fcc->fcc_rtcp_port);
}

//channel private
void yx_funcCallBackSendFeedBack(STB_HANDLE hClient, STB_UINT8* pRetData, STB_UINT32 nRtcpLen)
{
    struct FCC *fcc = (struct FCC *)hClient;
    struct sockaddr_in addr;

    if(fcc->fcc_rtcp_sock == -1) {
        LOG_SQA_PRINTF("fcc_arq_sock == -1!\n");
        return;
    }
    if(fcc->port_off != 3 || !fcc->fcc_ip || fcc->fcc_rtcp_port == 0) {
        LOG_SQA_ERROR("send feed back request ip %08x or port %d is 0, fcc->port_off=%d\n", fcc->fcc_ip, (int)fcc->fcc_rtcp_port, fcc->port_off);
        return;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = fcc->fcc_ip;
    addr.sin_port = htons((u_short)fcc->fcc_rtcp_port);
    ind_net_sendto(fcc->fcc_rtcp_sock, pRetData, nRtcpLen, MSG_NOSIGNAL, &addr);
    LOG_SQA_PRINTF("sfeed ip %08x:%d\n", fcc->fcc_ip, (int)fcc->fcc_rtcp_port);
}

//channel private
extern unsigned long g_fec_port;
void yx_funcCallBackIGMPJoin(STB_HANDLE hClient, STB_INT32 IGMPJoinType)
{
    struct FCCArg *arg;
    struct FCC *g_fcc = (struct FCC *)hClient;
    arg = &g_fcc->arg;

    LOG_SQA_PRINTF("IGMPJoinType =%d\n", IGMPJoinType);

    if(IGMP_FEC_LAYER1 == IGMPJoinType || IGMP_FEC_LAYER1 == IGMPJoinType) {
        LOG_SQA_PRINTF("the IGMPJoinType is unused!!!\n");
    } else if(IGMP_ORIGINAL == IGMPJoinType || -1 == IGMPJoinType) {
        if(g_fcc->mult_recover_sock == -1 && g_fcc->mult_base_sock == -1) {
            g_fcc->mult_base_sock = stream_port_multicast_play(arg->sin.in_addr.s_addr, arg->sin.port);
            g_fcc->mult_flag = 1;
            arg->fd_regist(arg->rtsp, g_fcc, g_fcc->mult_base_sock, fcc_recv_base_mult);
            if((g_fcc->arg.type & SQA_FEC_CODE) > 0) {
                g_fcc->mult_recover_sock = stream_port_multicast_play(arg->sin.in_addr.s_addr, arg->sin.port - 1);
                arg->fd_regist(arg->rtsp, g_fcc, g_fcc->mult_recover_sock, fcc_recv_recover_mult);
            } else {
                LOG_SQA_PRINTF("FEC socket is not created!\n");
            }
            LOG_SQA_PRINTF("have join IGMP_ORIGINAL mult_base_sock = %d, mult_recover_sock = %d\n", g_fcc->mult_base_sock, g_fcc->mult_recover_sock);
        } else {
            LOG_SQA_PRINTF("WARNING!join one agian!");
        }
    }
    return;
}

//channel private
int yx_funcCallBackIGMPLeave(STB_HANDLE hClient, STB_INT32 IGMPJoinType)
{
    struct FCCArg *arg;
    struct FCC *g_fcc = (struct FCC *)hClient;
    arg = &g_fcc->arg;

    LOG_SQA_PRINTF("IGMPJoinType =%d\n", IGMPJoinType);

    if(IGMP_FEC_LAYER1 == IGMPJoinType || IGMP_FEC_LAYER1 == IGMPJoinType) {
        LOG_SQA_PRINTF("the IGMPJoinType is unused!!!\n");
    } else if(IGMP_ORIGINAL == IGMPJoinType || -1 == IGMPJoinType) {
        if(g_fcc->mult_base_sock > 0) {
            stream_port_multicast_stop(g_fcc->mult_base_sock, arg->sin.in_addr.s_addr);
            arg->fd_unregist(arg->rtsp, g_fcc->mult_base_sock);
            g_fcc->mult_base_sock = -1;
        }
        if(g_fcc->mult_recover_sock > 0) {
            stream_port_multicast_stop(g_fcc->mult_recover_sock, arg->sin.in_addr.s_addr);
            arg->fd_unregist(arg->rtsp, g_fcc->mult_recover_sock);
            g_fcc->mult_recover_sock = -1;
        }
    }
    return 0;
}

//public
void * yx_funcCallBackGetMemory(STB_HANDLE hClient, STB_UINT32 len)
{
    void * ptr = NULL;
    if(len == 0) { //WZW modified to fix pc-lint warning 568, non-negative quantity is never less than zero
        LOG_SQA_PRINTF("len == 0 !\n");
        return NULL;
    }
    ptr = malloc(len);

    if(NULL == ptr)
        LOG_SQA_PRINTF("malloc return NULL!\n");
    return ptr;
}
//public
void yx_funcCallBackFreeMemory(STB_HANDLE hClient, STB_HANDLE memPtr)
{
    if(NULL == memPtr) {
        LOG_SQA_PRINTF("Free ERR\n");
        return;
    }
    free(memPtr);
    return;
}
//public
STB_UINT32 yx_funcCallBackGetTime(void)
{
    return SqaGetMS( );
}
//public
void yx_funcCallBackTraceOut(E_TraceLevel eLevel, const STB_INT8* pszModule, const STB_INT8* format, ...)
{
    pszModule = pszModule;
    switch(eLevel) {
    case LEVEL_DEBUG:
        if(getModuleLevel("sqa") >= LOG_LEVEL_VERBOSE) {  //LOG_LEVEL_NORMAL
            char mbuf[4096 + 1];
            va_list ap;

            va_start(ap, format);
            vsnprintf(mbuf, sizeof(mbuf) - 1, format, ap);
            va_end(ap);
            LOG_SQA_PRINTF("%s\n", mbuf);
        }
        break;

    case LEVEL_INFO:
    case LEVEL_WARNING:
    case LEVEL_ERROR:
        if(getModuleLevel("sqa") >= LOG_LEVEL_ERROR) {
            char mbuf[4096 + 1];
            va_list ap;

            va_start(ap, format);
            vsnprintf(mbuf, sizeof(mbuf) - 1, format, ap);
            va_end(ap);
            LOG_SQA_PRINTF("%s\n", mbuf);
        }
        break;
    default:
        break;
    }
    return;
}

STB_INT32 yx_funcCallBackGetIP_FCC(STB_HANDLE hClient , GetIPType ip_type, STB_INT8*buf, STB_INT32 *ilength)
{
    struct FCC *g_fcc = (struct FCC*)hClient;
    struct FCCArg *fcc_arg = &g_fcc->arg;
    STB_INT32 len = 0;

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));

    if(SQA_GETIP_STBIP == ip_type) {
        char ifname[URL_LEN] = { 0 };
        network_default_ifname(ifname, URL_LEN);
        network_address_get(ifname, buf, URL_LEN);
        len = strlen(buf);
    }

    if(SQA_GETIP_MULTGROUPIP == ip_type) {
        if(0 == fcc_arg->sin.in_addr.s_addr) {
            len = 0;
            LOG_SQA_WARN("unicast\n");
        } else {
            sin.sin_addr = fcc_arg->sin.in_addr;
            ind_net_ntoa(&sin, buf);
            len = strlen(buf);
        }
    }

    if(SQA_GETIP_MULTSRCIP == ip_type) {
        sin.sin_addr.s_addr = 0;
        ind_net_ntoa(&sin, buf);
        len = strlen(buf);

    }
    *ilength = len;
    LOG_SQA_PRINTF("ip_type = %d,get ip is =%s\n", ip_type, buf);
    return 0;
}

STB_INT32 yx_funcCallBackGetIP_RET(STB_HANDLE hClient , GetIPType ip_type, STB_INT8*buf, STB_INT32 *ilength)
{
//    struct RET *g_ret = (struct RET*)hClient;
    STB_INT32 len = 0;

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));

    if(SQA_GETIP_STBIP == ip_type) {
        char ifname[URL_LEN] = { 0 };
        network_default_ifname(ifname, URL_LEN);
        network_address_get(ifname, buf, URL_LEN);
        len = strlen(buf);
    }

    else if(SQA_GETIP_MULTGROUPIP == ip_type) {
        buf = NULL;
        len = 0;
    }

    else if(SQA_GETIP_MULTSRCIP == ip_type) {
        buf = NULL;
        len = 0;

    }
    *ilength = len;
    LOG_SQA_PRINTF("ip_type = %d,get ip is =%s\n", ip_type, buf);
    return 0;
}

//vod private
void yx_funcCallBackSendFeedBack_RET(STB_HANDLE hClient, STB_UINT8* pRetData, STB_UINT32 nRtcpLen)
{
    struct RET* ret = (struct RET *)hClient;
    struct sockaddr_in addr;

    //BEGIN();

    if(-1 == ret->ret_arq_sock) {
        LOG_SQA_ERROUT("ret_arq_sock == -1!\n");
    }

    if(!ret->ret_ip || 0 == ret->ret_port) {
        LOG_SQA_ERROUT("vdf ip %08x or port %d is 0\n", ret->ret_ip, (int)ret->ret_port);
    }

    memset(&addr, 0 , sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ret->ret_ip;
    addr.sin_port = htons((u_short)ret->ret_port);

    ind_net_sendto(ret->ret_arq_sock, pRetData, nRtcpLen, MSG_NOSIGNAL, &addr);
    LOG_SQA_PRINTF("sfeed ip %08x:%d\n", ret->ret_ip, (int)ret->ret_port);

Err:
    return;
    //END_VOID();

}
#define SEQ_NUM 10
void yx_funcCallBackSendFeedBack_ARQ(STB_HANDLE hClient, STB_UINT8* pRetData, STB_UINT32 nRtcpLen)
{
    struct RET* g_ret = (struct RET *)hClient;
    struct RETArg* arg = &g_ret->arg;

    if(pRetData == NULL || nRtcpLen == 0)
        LOG_SQA_ERROUT("pRetData or nRtcplen err\n");

    arg->rtsp_pktfb(arg->rtsp, pRetData, nRtcpLen);
Err:
    return;
}

void yx_funcCallBackDataBufferLevelNULL(STB_HANDLE hClient, DataBufferLevel pDataBufferLevel)
{
    return;
}

//vod private
void yx_funcCallBackDataBufferLevel(STB_HANDLE hClient, DataBufferLevel pDataBufferLevel)
{
    struct RET *g_ret = (struct RET *)hClient;
    struct RETArg *arg = NULL;

    if(NULL == g_ret) {
        LOG_SQA_PRINTF("THE hClient is NULL!\n");
        return;
    }
    arg = &g_ret->arg;
    if(SQA_BUFFER_LEVEL_HIGH == pDataBufferLevel) {
        LOG_SQA_WARN("SQA_BUFFER_LEVEL_HIGH\n");
        arg->cache_off(arg->rtsp);
        return;
    } else if(SQA_BUFFER_LEVEL_LOW == pDataBufferLevel) {
        LOG_SQA_WARN("SQA_BUFFER_LEVEL_LOW\n");
        arg->cache_on(arg->rtsp);
        return;
    } else if(SQA_BUFFER_LEVEL_RECONNECT == pDataBufferLevel) {
        LOG_SQA_WARN("SQA_BUFFER_LEVEL_RECONNECT\n");
        return;
    } else if(SQA_BUFFER_LEVEL_OVERFLOW == pDataBufferLevel) {
        LOG_SQA_WARN("SQA_BUFFER_LEVEL_OVERFLOW\n");
        return;
    } else {
        LOG_SQA_ERROUT("ON AND OFF, BUT DataBufferLevel is %d\n", pDataBufferLevel);
    }

Err:
    return;
}

void yx_funcCallBackTellFccStateRsp(STB_HANDLE hClient, FccStateRsp fcc_state_response)
{
    struct FCC* fcc;
    struct ind_sin *sin;

    int ret = -1;

    fcc = (struct FCC*)hClient;
    sin = &fcc->arg.sin;

    fcc->port_off = fcc_state_response;
    LOG_SQA_PRINTF("the reason of fcc  respons is %d, fcc->port_off=%d\n", fcc_state_response, fcc->port_off);

	if(3 == fcc_state_response)	{
		ret = channel_array_update_fcc_server_addr(sin->in_addr.s_addr, sin->port, fcc->fcc_ip);
		if(ret)
		    LOG_SQA_PRINTF("set FCC_server error!!!\n");
	}else {
		ret = channel_array_update_fcc_server_addr(sin->in_addr.s_addr, sin->port, 0); //如果fcc server响应异常,清掉已保存在频道列表中的fcc ip. 下次重新做rrs调度,去获取新的fcc server ip.
		if(ret)
			LOG_SQA_PRINTF("set FCC_server NULL error!!!\n");
    }
}

void yx_funcCallBackFccIP(STB_HANDLE hClient, STB_UINT32 server_addr, FccIpType ip_type)
{
    struct FCC* fcc;

    int ret = -1;

    if(0 == server_addr)
        return;

    fcc = (struct FCC*)hClient;

    fcc->fcc_ip = server_addr;
    if((fcc->arg.type & SQA_RET_CODE) > 0) {
        struct ind_sin *sin = &fcc->arg.sin;
        ret = channel_array_update_fcc_server_addr(sin->in_addr.s_addr, sin->port, fcc->fcc_ip);
        if(ret)
            LOG_SQA_PRINTF("get FCC_server error!!!\n");
    }

    LOG_SQA_PRINTF("funcCallBackFccIP %08x, FccIpType is %d\n", fcc->fcc_ip, ip_type);
}

void yx_funcCallBackFccPort(STB_HANDLE hClient, STB_UINT32 port)
{
    FCC_Nat_port = port;
    LOG_SQA_PRINTF("CallBackFccPort is %d\n", port);
}

void yx_funcCallBackValidTime(STB_HANDLE hClient, STB_INT32 validTime)
{
    struct FCC *fcc;
    struct ind_sin *sin;

    if(validTime <= 0)
        return;
    fcc = (struct FCC *)hClient;
    sin = &fcc->arg.sin;

    validTime = ntohl(validTime);
    LOG_SQA_PRINTF("validTime is %d\n", validTime);
    if(validTime > 0)
        channel_array_set_fcc_validtime(sin->in_addr.s_addr, sin->port, validTime + SqaGetMS( ));
    else
        channel_array_set_fcc_validtime(sin->in_addr.s_addr, sin->port, 0);
}

void yx_funcCallBackSendNatData(STB_HANDLE hClient, STB_UINT8 *nat_data, STB_UINT32 data_len)
{
    struct FCC *fcc = (struct FCC *)hClient;
    struct sockaddr_in addr;

    if(fcc->fcc_rtp_sock == -1) {
        LOG_SQA_PRINTF("fcc_rtp_sock == -1!\n");
        return;
    }
    if(!fcc->fcc_ip || fcc->fcc_rtcp_port == 0) {
        LOG_SQA_ERROR("send FCC request ip %08x or port %d is 0\n", fcc->fcc_ip, (int)fcc->fcc_rtcp_port);
        return;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = fcc->fcc_ip;
    addr.sin_port = FCC_Nat_port;
    ind_net_sendto(fcc->fcc_rtp_sock, nat_data, data_len, MSG_NOSIGNAL, &addr);
    LOG_SQA_PRINTF("send Nat Data to  ip %08x:%d\n", fcc->fcc_ip, (int)fcc->fcc_rtcp_port);
}

void yx_funcCallBackHmsRsp(STB_HANDLE hClient, HmsStateRsp hms_state_response)
{
    LOG_SQA_PRINTF("funcCallBackHmsRsp is %d\n", hms_state_response);
    if(HMS_TIMEOUT == hms_state_response)
        LOG_SQA_PRINTF("quit\n");
    else if(HMW_SUCCESS == hms_state_response)
        LOG_SQA_PRINTF("go no\n");
}


void yx_funcCallBackSendFCCNULL(STB_HANDLE hClient, STB_UINT8* pRtcpData, STB_UINT32 nRtcpLen)
{
    LOG_SQA_PRINTF("yx_funcCallBackSendFCCNULL is %p!\n", hClient);
}

void yx_funcCallBackSendFeedBack_RTCP(STB_HANDLE hClient, STB_UINT8* pRetData, STB_UINT32 nRtcpLen)
{
    LOG_SQA_PRINTF("yx_funcCallBackSendFeedBack_RTCP is %p!\n", hClient);
}

void yx_funcCallBackSendNatDataNULL(STB_HANDLE hClient, STB_UINT8 *nat_data, STB_UINT32 data_len)
{
    LOG_SQA_PRINTF("yx_funcCallBackSendNatDataNULL is %p!\n", hClient);
}

void yx_funcCallBackIGMPJoinNULL(STB_HANDLE hClient, STB_INT32 IGMPJoinType)
{
    LOG_SQA_PRINTF("yx_funcCallBackIGMPJoinNULL is %p!\n", hClient);
}

void yx_funcCallBackTellFccStateRspNULL(STB_HANDLE hClient, FccStateRsp fcc_state_response)
{
    LOG_SQA_PRINTF("yx_funcCallBackTellFccStateRspNULL is %p!\n", hClient);
}


STB_UINT16 STB_htons(STB_UINT16 hostshort)
{
    return htons(hostshort);
}

STB_UINT16 STB_ntohs(STB_UINT16 netshort)
{
    return ntohs(netshort);
}

STB_UINT32 STB_htonl(STB_UINT32 hostshort)
{
    return htonl(hostshort);
}

STB_UINT32 STB_ntohl(STB_UINT32 netshort)
{
    return ntohl(netshort);
}

