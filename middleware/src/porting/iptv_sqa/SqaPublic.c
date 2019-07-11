#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#include "SysSetting.h"
#include "vde.h"
#include "vde_callback.h"

#include "SqaPublic.h"

void SqaHandleEvent(STB_HANDLE handle, struct RTSP* rtsp, fd_writedata_f fd_writedata, fd_writeable_f fd_writeable)
{
    STB_INT32  ret;	//WZW modified to fix pc-lint warning 650
    void *buffer;
    STB_UINT32 length;

    sqa_handle_event(handle);                                 /*  为了尽快ret ，在这里调用该函数  */

    for(;;) {
        if(1 != fd_writeable(rtsp)) /* 判断解码器缓存是否可写 */
            break;

        buffer = NULL;
        length = 0;
        ret = sqa_get_rtp_packet(handle, &buffer, &length, -1);
        if (0 != ret)
            break;
        if(buffer)
            fd_writedata(rtsp, (char*)buffer, length);/*RTP头由stream库去掉*/
        sqa_free_rtp_packet(handle, NULL);                    /*获取播放数据包后释放空间，第二个参数传入NULL */
    }
}

void sqaRecvData(int fd, STB_HANDLE handle, funFccRecPacket *fun,
    struct RTSP* rtsp, fd_writedata_f fd_writedata, fd_writeable_f fd_writeable)
{
    STB_UINT32 len = 0;
    STB_INT32  ret = 0;	//WZW modified to fix pc-lint warning 650
    void *buffer = NULL;
    STB_INT32 length = 0;

    ret = sqa_get_buf(handle, &buffer, &length);
    if(-1 == ret) {
        LOG_SQA_PRINTF("###SQA### sqa_get_buf NULL\n");
        return;
    }
    len = recv(fd, buffer, RTP_BUFFER_SIZE, 0);
    if(len <= 0) {                                          /*没有压入数据时，第二个参数要传入获取到的地址*/
        sqa_free_rtp_packet(handle, buffer);
        LOG_SQA_PRINTF("###SQA### recv len=%d\n", len);
        return;
    }

    if(0x47 == *((char *)buffer)) {
        fd_writedata(rtsp, (char *)buffer, len);
        sqa_free_rtp_packet(handle, buffer);                  /*没有压入数据时，第二个参数要传入获取到的地址*/
        return;
    }
    ret = fun(handle, buffer, len);
    if(0 != ret) {                                          /*压入异常情况释放空间*/
        sqa_free_rtp_packet(handle, buffer);
        LOG_SQA_PRINTF("###SQA### PUT ERROR = %d!\n", ret);
    }

    SqaHandleEvent(handle, rtsp, fd_writedata, fd_writeable);
}

unsigned int SqaGetMS(void)
{
	unsigned int clk;
	struct timespec tp;

	clock_gettime(CLOCK_MONOTONIC, &tp);

	clk = (unsigned int)tp.tv_sec * 1000 + (unsigned int)tp.tv_nsec / 1000000;

	return clk;
}


//RRS-FCC
static unsigned int RRS_ADDR = 0;
static unsigned short FCC_PORT = 8027;

int RRS_EPG_FLAG = 0;
char FCC_IP[64] = {0};

void mid_stream_setRRS(unsigned int addr, int port)
{
    unsigned int fccPort = (unsigned int)port;

    FCC_PORT = (unsigned short)fccPort;
    RRS_ADDR = addr;
}

unsigned short SqaGetPort(void)
{
    return FCC_PORT;
}

typedef struct tagFCCChannel FCCChannel;
struct tagFCCChannel {
    unsigned int mult;
    unsigned int port;

    unsigned int fccIP;
    unsigned int validTime;

    FCCChannel *next;
};

static FCCChannel *gChannelList = NULL;

/*************************************************
*function : get channel fcc server
*input : mult     channel url
*        fcc_ip   fcc server buffer addr
         fcc_srv_len  fcc server buffer len
**************************************************/
int channel_array_get_server(unsigned int mult, unsigned int port, unsigned int *fcc_ip, int *flag)
{
    FCCChannel *channel;
    unsigned int fccIP, validTime;

    if(1 == RRS_EPG_FLAG) {
        if(0 == strlen(FCC_IP)) {
            LOG_SQA_ERROR("FCC_IP is NULL!\n");
            fcc_ip = 0;
            return -1;
        }
        LOG_SQA_PRINTF("get FCC server =%s\n", FCC_IP);
        *fcc_ip = inet_addr(FCC_IP);
        *flag = 1;
        return 0;
    }

    channel = gChannelList;
    while (channel) {
        if (mult == channel->mult && port == channel->port)
            break;
        channel = channel->next;
    }

    if (channel) {
        fccIP = channel->fccIP;
        validTime = channel->validTime;
        //compare currenttime and FCC_Server_validTime
        if (fccIP && validTime) {
            unsigned int currentTime = SqaGetMS( );

            if(validTime > currentTime) {
                LOG_SQA_PRINTF("lefttime(%d)\n", validTime - currentTime);
            } else {
                channel->fccIP = 0;
                fccIP = 0;
                LOG_SQA_PRINTF("Current time is extend the validTime\n");
            }
        }
    } else {
        fccIP = 0;
    }
    if(!fccIP) {
        if(!RRS_ADDR) {
            LOG_SQA_ERROR("RRS_IP is NULL!\n");
            fcc_ip = 0;
            return -1;
        }
        LOG_SQA_PRINTF("get RRS server(%08x)\n", RRS_ADDR);
        *fcc_ip = RRS_ADDR;
        *flag = 1; //RRS_IP
        return 0;
    }

    LOG_SQA_PRINTF("channel_FCC_Server(%08x)\n", fccIP);
    *fcc_ip = fccIP;
    *flag = 2;	//fcc_server

    return 0;
}

void channel_array_set_fcc_validtime(unsigned int mult, unsigned int port, unsigned int validTime)
{
    FCCChannel *channel;

    channel = gChannelList;
    while (channel) {
        if (mult == channel->mult && port == channel->port) {
            break;
        }
        channel = channel->next;
    }
    if (!channel) {
        channel = (FCCChannel *)calloc(sizeof(FCCChannel), 1);
        if (!channel)
            return;
        channel->mult = mult;
        channel->port = port;
    }
    LOG_SQA_PRINTF("validtime = %d / %d\n", validTime, SqaGetMS( ));
    channel->validTime = validTime;
}

int channel_array_update_fcc_server_addr(unsigned int mult, unsigned int port, unsigned int fcc_ip)
{
    FCCChannel *channel;

    channel = gChannelList;
    while (channel) {
        if (mult == channel->mult && port == channel->port)
            break;
        channel = channel->next;
    }
    if (!channel) {
        channel = (FCCChannel *)calloc(sizeof(FCCChannel), 1);
        if (!channel)
            return -1;
        channel->mult = mult;
        channel->port = port;
    }

    LOG_SQA_PRINTF("fcc_ip = %08x\n", fcc_ip);
    channel->fccIP = fcc_ip;

    return 0;
}