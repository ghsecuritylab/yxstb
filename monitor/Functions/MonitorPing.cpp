#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <linux/icmp.h> //Android using for struct icmphdr
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include "MonitorDef.h"
#include "MonitorPing.h"
#include "MonitorManager.h"

#define IP_HEAD_LEN 20
#define ICMP_LEN 8
#define BUFFER_SIZE 50 * 1024
#define PING_RETURN_NUM 10

static PING_STATUS ping_flag = PING_STOP;
static int magic = -1;
extern char stbMonitorIP[32];
extern moni_buf g_monitorBuf;
extern msgq_t g_Msg;
extern char stbMonitorIP[32];


void monitorPingFunc(moni_buf_t buf, char *string)
{
    sprintf(buf->buf, "200connect^");
    buf->extend = (char *)malloc(strlen(string) + strlen("ping^") + 1);
    memset(buf->extend, 0, strlen(string) + strlen("ping^") + 1);
    sprintf(buf->extend, "ping^%s", string);
    MonitorManager::GetInstance()->monitor_cmd_response(buf);
}


int monitorPingConnect(moni_buf_t buf, int len)
{
    char *p = NULL, *p1 = NULL;
    DIAG_MSG msg;

    printf("buf is %s\n", buf->buf);
    memset(&msg, 0, sizeof(msg));
    p = strstr(buf->buf, "/length:");
    if(p) {
        msg.cmd = PING_START;
        p += strlen("/length:");
        msg.length = atoi(p);
        printf("length is %d\n", msg.length);

        p1 = strchr(p, '^');
        if(!p1) {
            printf("This url(%s) is error !\n", p);
            return -1;
        }

        p1 += 1;
        printf("url is %s\n", p1);

        strcpy(msg.url, p1);
        printf("msg.url is %s\n", msg.url);

        memcpy(&(msg.buf), buf, sizeof(struct moni_buf));
        printf("msg buf is %s\n", msg.buf.buf);

        msg.func = (void *)monitorPingFunc;

        if(MonitorManager::GetInstance()->monitorDiagMsgPut(&msg) == -1) {
            printf("Error: monitorDiagMsgPut return -1\n");
            return -1;
        }

        return 0;
    }

    p = strstr(buf->buf, "/stop");
    if(p) {
        printf("receive stop request, stop ping!\n");
        monitorSetPingStop();
        return 0;
    }

    p = strstr(buf->buf, "null");
    if(p) {
        msg.cmd = PING_START;
        p += strlen("null");

        msg.length = -1;

        p1 = strchr(p, '^');
        if(!p1) {
            printf("Error: Not url, buf is %s\n", p);
            return -1;
        }

        p1 += 1;
        printf("url is %s\n", p1);

        strcpy(msg.url, p1);
        printf("msg.url is %s\n", msg.url);

        memcpy(&(msg.buf), buf, sizeof(struct moni_buf));
        printf("msg buf is %s\n", msg.buf.buf);

        msg.func = (void *)monitorPingFunc;

        if(MonitorManager::GetInstance()->monitorDiagMsgPut(&msg) == -1) {
            printf("Error: monitorDiagMsgPut return -1\n");
            return -1;
        }

        return 0;
    }

    printf("Error: string illegal\n");
    return -1;
}



PING_STATUS monitorGetPingFlag( void )
{
    return ping_flag;
}

void monitorSetPingRun( void )
{
    ping_flag = PING_RUN;
    printf( "RUN ping, ping_flag is %d\n", ping_flag );
}

void monitorSetPingStop( void )
{
    ping_flag = PING_STOP;
    printf( "STOP ping, ping_flag is %d\n", ping_flag );
}

int monitorGetPingMagic( void )
{
    return magic;
}

void monitorSetPingStopMagic( void )
{
    ping_flag = PING_STOP;
    magic *= -1;
    printf( "magic STOP ping, magic is %d, ping_flag is %d\n", magic, ping_flag );
}
int monitorStbMonitorPing (char *url,moni_buf_t buf,void* stb_func )
{
    int  ip_fd = -1;;
    char pstring[512] = {0};
    //int  pstr_len = 0;
    int testlen = BUFFER_SIZE;
    /*对端地址*/
    struct sockaddr_in send_addr;
    /*发送应用缓冲区*/
    char send_buf[1024] = {0};
    /*报文序列号*/
    int sequence = 0;
    /*主机名指针*/
    struct hostent *host;
    /*标识是否已经接收到回文*/
    int flag = 0;
//    int i = 0;
    int m = 0;
    printf("monitorStbMonitorPing()\n");
	printf("url\n");

    PTR_CALLBACK func = (PTR_CALLBACK)stb_func;

    if( url == NULL || func == NULL || strlen(url) >20)
    {
        printf("stb_monitor_ping,the para is error\n");
        return -1;
    }

    printf( "ping:url is %s\n", url );

    /*packet_len为IP包头和ICMP包头长度之和*/
    int packet_len =IP_HEAD_LEN + ICMP_LEN;
    /*创建使用ICMP的原始套接字,这种套接字只有root才能生成*/
    ip_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(ip_fd < 0)
    {
        printf("raw socket error.\n");
        return -1;
    }

    /*扩大套接字接收缓冲区到50K这样做主要为了减小接收缓冲区溢出的的可能性,若无意中ping一个广播地址或多播地址,将会引来大量应答*/
    if (setsockopt(ip_fd, SOL_SOCKET, SO_RCVBUF, &testlen, sizeof(testlen))) {
        printf("setsockopt socket error.\n");
        close(ip_fd);
        return -1;
    }

    /*只须填写地址信息，不需要指定端口信息，因为原始套接字在传输层之下*/
    send_addr.sin_family = AF_INET;

    /*判断是主机名还是ip地址*/
    if(inet_addr(url) == INADDR_NONE)
    {
        /*是主机名*/
        if((host = gethostbyname(url)) == NULL)
        {
            /*主机名错误*/
            printf("get host by name error: unknow host.\n");
            close(ip_fd);

//         for(m = 0 ;m<PING_RETURN_NUM;m++){
            while( monitorGetPingFlag() == PING_RUN )
            {
                snprintf(pstring,sizeof(pstring),"the %d ping %s :unknown host\n",m+1,url);
                func(buf,pstring);
                sleep(1);
            }
            ip_fd = -1;
            return 0;
        }
        memcpy(&send_addr.sin_addr, host->h_addr, host->h_length);
    }
    else
    {
        /*是ip地址*/
        inet_aton(url, &send_addr.sin_addr);
    }

    flag = 0;


    m = 0;
//      for(i = 0; i<PING_RETURN_NUM && m < PING_RETURN_NUM; i++)
    while( monitorGetPingFlag() == PING_RUN )
    {
        printf("while ping\n");
        //发送
        {
            int len;
            struct icmphdr *icmp_p;
            memset(send_buf,0,sizeof(send_buf));
            icmp_p = (struct icmphdr *)send_buf;

            /*填写ICMP报文类型*/
            icmp_p->type = ICMP_ECHO;
            /*填写ICMP报文的代码*/
            icmp_p->code = 0;
            /*填写ICMP报文的标识符*/
            (icmp_p->un).echo.id = 0;//p_id;
            /*填写ICMP报文的序号，并增加ICMP的序号*/
            (icmp_p->un).echo.sequence = sequence++;

            /*记录发送时间*/
            gettimeofday((struct timeval*)(icmp_p + 1), NULL);

            /*报文的长度等于IP包长度加上ICMP报文长度和数据长度*/
            len = sizeof(struct timeval) + packet_len ;

            /*使用IP计算IP包头校验和的计算方法来计算ICMP的校验和*/
            icmp_p->checksum = 0;
            icmp_p->checksum = monitorIpChecksum((u_short *)icmp_p, len);

            //printf("send_buf = %p icmp_p->checksum = %d==%d\n", send_buf, icmp_p->checksum,sequence);
            /*发送IP数据包*/
            if(sendto(ip_fd, send_buf, len, 0, (struct sockaddr*)&send_addr, sizeof(send_addr)) < 0)
            {
                printf("send to error.\n");
                continue; //重新发送？
            }
        }

        //接受
        {
            char recv_buf[1024] = {0};
            int n = 0;
            struct ip *ip_p  =  NULL;
            struct timeval *time_now, *time_send;
            struct timeval now;

            int iphead_len = 0;
            int icmp_len = 0;

            struct icmphdr *icmp_p;

            float delay;

            fd_set fds;
            int nfds;
            struct timeval wait = {1,0};

            FD_ZERO(&fds);
            FD_SET((u_int)ip_fd, &fds);

            wait.tv_sec = 2;
            wait.tv_usec = 0;

            nfds = (int)(ip_fd + 1);
            if (select(nfds, &fds, (fd_set *)0, (fd_set *)0, &wait) <= 0)
            {
//                  if(m >= PING_RETURN_NUM)
                if( monitorGetPingFlag() == PING_STOP )
                    break;
                snprintf(pstring,sizeof(pstring),"the %d ping %s :Request timed out\n",m+1,url);
                func(buf,pstring);
                sleep(1);
                m ++;
                continue;
            }

            memset(recv_buf,0,sizeof(recv_buf));
            n = recvfrom(ip_fd, recv_buf, sizeof(recv_buf), 0, NULL, NULL);
            if(n < 0)
            {
//                  if(m >= PING_RETURN_NUM)
                if( monitorGetPingFlag() == PING_STOP )
                    break;
                snprintf(pstring,sizeof(pstring),"the %d ping %s :Request timed out\n",m+1,url);
                func(buf,pstring);
                sleep(1);
                m ++;
                continue;
            }
            ip_p = (struct ip*)recv_buf;

            /*获取IP包头长度*/
            iphead_len = ip_p->ip_hl<<2;
            /*获取IP数据包中包含的ICMP报文*/
            icmp_p = (struct icmphdr *)(recv_buf + iphead_len);
            /*计算ICMP报文长度，它等于接受到的长度减去IP包头的长度*/
            icmp_len = n - iphead_len;

            if(icmp_len < 8)
            {
//                  if(m >= PING_RETURN_NUM)
                if( monitorGetPingFlag() == PING_STOP )
                    break;
                printf("error icmp len = %d.\n", icmp_len);
                snprintf(pstring,sizeof(pstring),"the %d ping %s :Request timed out\n",m+1,url);
                func(buf,pstring);
                sleep(1);
                m ++;
                continue;
            }

            /*如果ICMP类型相同则输出显示*/
            if(icmp_p->type == ICMP_ECHOREPLY)
            {
                if(icmp_len < 16)
                    printf("icmplen = %d.\n", icmp_len);

                flag = 1;//表示已经接收到回文；

                gettimeofday(&now, NULL);

                time_now = &now;
                time_send = (struct timeval*)(icmp_p + 1);

                if((time_now->tv_usec -= time_send->tv_usec) < 0)
                {
                    time_now->tv_sec --;
                    time_now->tv_usec += 1000000;
                }

                time_now->tv_sec -= time_send->tv_sec;

                /*计算延时*/
                delay = time_now->tv_sec * 1000.0 + time_now->tv_usec / 1000.0;

                /*打印接收到的报文相关信息*/
                snprintf(pstring,sizeof(pstring),"the %d ping %d(%d) bytes from %s: icmp_seq=%d ttl=%d time=%.3fms\n\t\r",
                         m+1,icmp_len, n, inet_ntoa(send_addr.sin_addr), (icmp_p->un).echo.sequence,
                         ip_p->ip_ttl, delay);
//                 if(m >= PING_RETURN_NUM)
                if( monitorGetPingFlag() == PING_STOP )
                    break;
                func(buf,pstring);
                m ++;
                sleep(1);
            } else {
                snprintf(pstring,sizeof(pstring),"the %d ping %s :Destination Host Unreachable\n",m+1,url);
                func(buf,pstring);
                sleep(1);
                m ++;
            }

        }
    }
    close(ip_fd);
    ip_fd = -1;
    return 0;
}


unsigned short monitorIpChecksum(unsigned short *pcheck, int check_len)
{
    int nleft = check_len;
    int sum = 0;
    unsigned short *p = pcheck;
    unsigned short result = 0;

    while(nleft > 1)
    {
        sum = sum + *p ++;
        nleft -= 2;
    }

    if(nleft == 1)
    {
        *(unsigned char *)(&result) = *(unsigned char *)p;
        sum += result;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    result = (u_short)(~sum);

    return result;
}
