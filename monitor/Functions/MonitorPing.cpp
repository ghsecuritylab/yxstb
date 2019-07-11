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
    /*�Զ˵�ַ*/
    struct sockaddr_in send_addr;
    /*����Ӧ�û�����*/
    char send_buf[1024] = {0};
    /*�������к�*/
    int sequence = 0;
    /*������ָ��*/
    struct hostent *host;
    /*��ʶ�Ƿ��Ѿ����յ�����*/
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

    /*packet_lenΪIP��ͷ��ICMP��ͷ����֮��*/
    int packet_len =IP_HEAD_LEN + ICMP_LEN;
    /*����ʹ��ICMP��ԭʼ�׽���,�����׽���ֻ��root��������*/
    ip_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(ip_fd < 0)
    {
        printf("raw socket error.\n");
        return -1;
    }

    /*�����׽��ֽ��ջ�������50K��������ҪΪ�˼�С���ջ���������ĵĿ�����,��������pingһ���㲥��ַ��ಥ��ַ,������������Ӧ��*/
    if (setsockopt(ip_fd, SOL_SOCKET, SO_RCVBUF, &testlen, sizeof(testlen))) {
        printf("setsockopt socket error.\n");
        close(ip_fd);
        return -1;
    }

    /*ֻ����д��ַ��Ϣ������Ҫָ���˿���Ϣ����Ϊԭʼ�׽����ڴ����֮��*/
    send_addr.sin_family = AF_INET;

    /*�ж�������������ip��ַ*/
    if(inet_addr(url) == INADDR_NONE)
    {
        /*��������*/
        if((host = gethostbyname(url)) == NULL)
        {
            /*����������*/
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
        /*��ip��ַ*/
        inet_aton(url, &send_addr.sin_addr);
    }

    flag = 0;


    m = 0;
//      for(i = 0; i<PING_RETURN_NUM && m < PING_RETURN_NUM; i++)
    while( monitorGetPingFlag() == PING_RUN )
    {
        printf("while ping\n");
        //����
        {
            int len;
            struct icmphdr *icmp_p;
            memset(send_buf,0,sizeof(send_buf));
            icmp_p = (struct icmphdr *)send_buf;

            /*��дICMP��������*/
            icmp_p->type = ICMP_ECHO;
            /*��дICMP���ĵĴ���*/
            icmp_p->code = 0;
            /*��дICMP���ĵı�ʶ��*/
            (icmp_p->un).echo.id = 0;//p_id;
            /*��дICMP���ĵ���ţ�������ICMP�����*/
            (icmp_p->un).echo.sequence = sequence++;

            /*��¼����ʱ��*/
            gettimeofday((struct timeval*)(icmp_p + 1), NULL);

            /*���ĵĳ��ȵ���IP�����ȼ���ICMP���ĳ��Ⱥ����ݳ���*/
            len = sizeof(struct timeval) + packet_len ;

            /*ʹ��IP����IP��ͷУ��͵ļ��㷽��������ICMP��У���*/
            icmp_p->checksum = 0;
            icmp_p->checksum = monitorIpChecksum((u_short *)icmp_p, len);

            //printf("send_buf = %p icmp_p->checksum = %d==%d\n", send_buf, icmp_p->checksum,sequence);
            /*����IP���ݰ�*/
            if(sendto(ip_fd, send_buf, len, 0, (struct sockaddr*)&send_addr, sizeof(send_addr)) < 0)
            {
                printf("send to error.\n");
                continue; //���·��ͣ�
            }
        }

        //����
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

            /*��ȡIP��ͷ����*/
            iphead_len = ip_p->ip_hl<<2;
            /*��ȡIP���ݰ��а�����ICMP����*/
            icmp_p = (struct icmphdr *)(recv_buf + iphead_len);
            /*����ICMP���ĳ��ȣ������ڽ��ܵ��ĳ��ȼ�ȥIP��ͷ�ĳ���*/
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

            /*���ICMP������ͬ�������ʾ*/
            if(icmp_p->type == ICMP_ECHOREPLY)
            {
                if(icmp_len < 16)
                    printf("icmplen = %d.\n", icmp_len);

                flag = 1;//��ʾ�Ѿ����յ����ģ�

                gettimeofday(&now, NULL);

                time_now = &now;
                time_send = (struct timeval*)(icmp_p + 1);

                if((time_now->tv_usec -= time_send->tv_usec) < 0)
                {
                    time_now->tv_sec --;
                    time_now->tv_usec += 1000000;
                }

                time_now->tv_sec -= time_send->tv_sec;

                /*������ʱ*/
                delay = time_now->tv_sec * 1000.0 + time_now->tv_usec / 1000.0;

                /*��ӡ���յ��ı��������Ϣ*/
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
