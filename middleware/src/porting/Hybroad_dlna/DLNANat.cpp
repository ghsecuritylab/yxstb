

#include "DLNANat.h"
#include <string>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "AppSetting.h"
#include "DlnaAssertions.h"
#include "AppSetting.h"

#include "app_heartbit.h"
#include "Assertions.h"
#include "mid/mid_tools.h"
#include "mid_sys.h"
#include "ipanel_event.h"
#include "json/json_public.h"

extern "C" {
int UpnpGetServerPort();
char *UpnpGetServerIpAddress();
}

namespace Hippo {

static DLNANat * g_dlnanat = NULL;

DLNANat* DLNANat::GetInstance(void)
{
    if (!g_dlnanat)
        g_dlnanat = new DLNANat();
    return g_dlnanat;
}

DLNANat::DLNANat()
    : m_hbPort(0)
    , m_hbInterval(0)
    , m_hbSock(-1)
    , m_inSock(-1)
    , m_outSock(-1)
    , firsttcpflag(0)
    , UrgResultlen(0)
    , serialNo(0)
    , Heartbitflag(0)
{
}

DLNANat::~DLNANat()
{
    g_dlnanat = NULL;
}


void DLNANat::Start(const char * ip, int port, int interval)
{/*{{{*/
    if (!ip) {
        DLNA_LOG_ERROR("DLNANat::Start error: ip == NULL!\n");
        return;
    }

    struct sockaddr_in  addr;
    int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        DLNA_LOG_ERROR("DLNANat::Start error: socket() failed: %s\n", strerror(errno));
        return;
    }

    int op = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&op, sizeof(op));
    if (ret != 0) {
        perror("setsockopt.SO_REUSEADDR");
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int tport = 10234;
    while (true) {
        addr.sin_port = htons(tport);
        if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            break;
        }
        perror("bind");
        if (tport > 10234 + 10) {
            DLNA_LOG_ERROR("DLNANat::Start error: bind to port (10234~10244) failed\n");
            close(sockfd);
            return;
        }
        tport ++;
    }
    DLNA_LOG("DLNANat::Start: bind to port %d\n", tport);

    m_hbIp = ip;
    m_hbPort = port;
    m_hbInterval = interval;

    m_hbSock = sockfd;

    mid_sys_serial(stbId);
    appSettingGetString("ntvuser", stbAccount, 32, 0);
    strcpy(method ,"UrgConnect");

    Message* message = this->obtainMessage(MessageType_Timer, NATTIMER_HEARTBIT, 0);
    this->sendMessage(message);
}/*}}}*/

void DLNANat::sendHeartBit(int opType)
{

    char HBbuf[512] = {0};
    struct json_object *udphb_obj = NULL;
    struct sockaddr_in address;
    udphb_obj = json_object_new_object();
    if(!udphb_obj)
    {
        printf("object_new error!!\n");
        return ;
    }
    DLNA_LOG_ERROR("sendHeartBit= %d \n",Heartbitflag);
    json_object_object_add(udphb_obj, "stbId", json_object_new_string(stbId));
    json_object_object_add(udphb_obj, "stbAccount", json_object_new_string(stbAccount));
    json_object_object_add(udphb_obj, "serialNo", json_object_new_int(serialNo++));
    json_object_object_add(udphb_obj, "opType", json_object_new_int(opType));
    json_object_object_add(udphb_obj, "method", json_object_new_string("StbHeartBeat"));

    strcpy(HBbuf,json_object_to_json_string(udphb_obj));
    DLNA_LOG_VERBOSE("HBbuf= %s \n",HBbuf);

    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(m_hbIp.c_str());
    address.sin_port = htons(m_hbPort);

    sendto(m_hbSock, HBbuf, sizeof(HBbuf), 0, (struct sockaddr *)&address, sizeof(address));
    if(opType)
    {
        Stop();
    }
    if(Heartbitflag>3)
    {
        DLNA_LOG_ERROR("Heartbit time out\n");
        Stop();
        //todo//发送重新注册事件
    }
    Heartbitflag++;
	//m_hbSock //发送心跳
}

void DLNANat::handleHeartbitResult(void)
{
    // 心跳回应
    int ret;
    char message[4096];
    //sin
    ret = recv(m_hbSock, message, sizeof(message), 0);
    //ret = recvfrom(m_hbSock, message, sizeof(message), 0, (struct sockaddr *)&sin, &sin_len);
    //if(sin.addr == m_hbIp)
    DLNA_LOG("Response from server : %s\n", message);

    if (strstr(message,"StbConnect"))
    {


        ret = handle_tcp_Register(message);
        if(ret)
        {
            DLNA_LOG("dlna_tcp_Register error %d \n",ret);
        }
        //应答参数 errorCode msg   移动到handleUrgConnectResult
        //sprintf(buf,"{\"errorCode\":\"%d\",\"msg\":\"    \"}",ret);
        //sendto(m_hbSock, buf, sizeof(buf), 0, (struct sockaddr *)&address, sizeof(address));
       return;
    }
    //tcp长连接
    //urg
    //upnp ip127.0.0.1
    Heartbitflag= 0;

}
void DLNANat::handleUrgConnectResult(int Result )
{
    char buf[1024];
    struct sockaddr_in address;

    if(Result)
    {
        DLNA_LOG_ERROR("dlna_tcp_Register error %d \n",Result);
        firsttcpflag = 0;//tcp 回应重新开始
    }
    sprintf(buf,"{\"errorCode\":\"%d\",\"msg\":\"    \"}",Result);
    //send(m_hbSock, buf, sizeof(buf), 0);

    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(m_hbIp.c_str());
    address.sin_port = htons(m_hbPort);

    sendto(m_hbSock, buf, sizeof(buf), 0, (struct sockaddr *)&address, sizeof(address));

    firsttcpflag = 2;//tcp 回应完成
}
int DLNANat::handle_tcp_Register( char* message )
{
    struct sockaddr_in servaddr;
    struct json_object *connect_obj = NULL, *parse_obj = NULL;
    int ret = 0;
    char *rand = NULL,*urgIp = NULL,*urgPort = NULL;
    char register_buff[256] = {0};

    connect_obj = json_tokener_parse(message);
    if(!connect_obj)
    {
       DLNA_LOG_ERROR("connect_obj parse error\n");
       return -1;
    }
    parse_obj = json_object_object_get(connect_obj, "rand");
    rand = (char*)json_object_get_string(parse_obj);

    parse_obj = json_object_object_get(connect_obj, "urgPort");
    urgPort = (char*)json_object_get_string(parse_obj);

    parse_obj = json_object_object_get(connect_obj, "urgIp");
    urgIp = (char*)json_object_get_string(parse_obj);
    json_object_put(connect_obj);


    m_outSock =  socket(AF_INET, SOCK_STREAM, 0);
    m_inSock = socket(AF_INET, SOCK_STREAM, 0);
    //servaddr >>>>urgIp
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(urgIp);
    servaddr.sin_port = htons(atoi(urgPort));

    ret = connect(m_outSock,(struct sockaddr *)&servaddr,sizeof(servaddr));
     if( ret )
    {
        DLNA_LOG_ERROR("connect m_outSock error %d \n",ret);
        stop_tcp();
        return -2;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(UpnpGetServerPort());
    ret = connect(m_inSock,(struct sockaddr *)&servaddr,sizeof(servaddr));
    if( ret )
    {
        DLNA_LOG_ERROR("connect m_inSock error %d \n",ret);
        stop_tcp();
        return -3;
    }
    sprintf(register_buff, "{\"stbId\":\"%s\",\"stbAccount\":\"%s\",\"method\":\"%s\"}",stbId,stbAccount,method);
    send(m_outSock, register_buff, strlen(register_buff), 0);
    return 0;

}
void DLNANat::handleMessage(Message* msg)
{
	//flag stop
    if (msg->what != MessageType_Timer) {
        DLNA_LOG_ERROR("DLNANat::handleMessage error: recv msg(what = %d, arg1 = %d, arg2 = %d).\n", msg->what, msg->arg1, msg->arg2);
        return;
    }

    if (msg->arg1 == NATTIMER_HEARTBIT) {
        DLNA_LOG_VERBOSE("DLNANat::handleMessage timer:Heartbit\n");
        sendHeartBit(0);
        Message* message = this->obtainMessage(MessageType_Timer, NATTIMER_HEARTBIT, 0);
        this->sendMessageDelayed(message, m_hbInterval * 1000);
    }

    fd_set  rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    if (m_hbSock >= 0)
        FD_SET(m_hbSock, &rfds);
    if (m_inSock >= 0 && m_outSock >= 0) {
        FD_SET(m_inSock, &rfds);
        FD_SET(m_outSock, &rfds);
    }
    int maxfd = std::max(std::max(m_hbSock, m_inSock), m_outSock);

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    int ret = select(maxfd + 1, &rfds, NULL, NULL, &tv);
    if (ret <= 0) {
        Message* message = this->obtainMessage(MessageType_Timer, NATTIMER_DATA, 0);
        this->sendMessageDelayed(message, 100);
        return;
    }
    if (m_inSock >= 0 && FD_ISSET(m_inSock, &rfds)) {
        if (m_outSock >= 0) {
            char    buffer[4096];
            int ret = recv(m_inSock, buffer, sizeof(buffer), 0);
            if( ret <= 0)
            {
                stop_tcp();
                return ;
            }
            send(m_outSock, buffer, ret, 0);
        }
    }
    if (m_outSock >= 0 && FD_ISSET(m_outSock, &rfds)) {
        if (m_inSock >= 0) {
            char    buffer[4096];
            int ret = recv(m_outSock, buffer, sizeof(buffer)-1, 0);
            if( ret <= 0)
            {
                stop_tcp();
                return ;
            }
            if(firsttcpflag != 2)
            {
                buffer[ret] = 0;
                if(firsttcpflag != 1)
                {
                    char *tempbuf = strstr(buffer,"{");
                     if(tempbuf)
                    {
                        firsttcpflag = 1;
                    }
                }
                if(1 == firsttcpflag)
                {
                    char *tempbuf1 = strstr(buffer,"}");
                    if(tempbuf1)
                    {
                        int jsonret = 1;


                        if(UrgResultlen+(strlen(buffer) - strlen(tempbuf1) - 1)<2048)
                            strncpy(UrgResult,buffer,(strlen(buffer) - strlen(tempbuf1) - 1));

                        UrgResultlen += strlen(buffer) - strlen(tempbuf1) - 1;
                        struct json_object *connect_obj = NULL, *parse_obj = NULL;
                        connect_obj = json_tokener_parse(UrgResult);
                        if(!connect_obj)
                        {
                           DLNA_LOG_VERBOSE("connect_obj parse error    UrgResult =%s \n",UrgResult);
                           return;
                        }

                        parse_obj = json_object_object_get(connect_obj, "errorCode");
                        if(parse_obj)
                            jsonret = json_object_get_int(parse_obj);
                        handleUrgConnectResult(jsonret);
                        json_object_put(connect_obj);
                        if(2 == firsttcpflag)
                            send(m_inSock, tempbuf1+1, buffer+ret-1-tempbuf1, 0);
                    }
                    else
                    {
                        if(ret<2048)
                        {
                            strcpy(UrgResult,buffer);
                            UrgResultlen += ret;
                        }
                        else
                        {
                            stop_tcp();
                            return;
                        }
                    }
                }

            }
            else
            {
                send(m_inSock, buffer, ret, 0);
            }
        }
    }
    if (m_hbSock >= 0 && FD_ISSET(m_hbSock, &rfds)) {
        handleHeartbitResult();
    }
    Message* message = this->obtainMessage(MessageType_Timer, NATTIMER_DATA, 0);
    this->sendMessageDelayed(message, 100);
}

void DLNANat::stop_tcp()
{
    close(m_outSock);
    close(m_inSock);
    m_outSock = -1;
    m_inSock = -1;
}

void DLNANat::Stop()
{
    stop_tcp();
    close(m_hbSock);
    m_hbPort = 0;
    m_hbInterval= 0;
    m_hbSock= -1;
    firsttcpflag = 0;
    UrgResultlen = 0;
    serialNo= 0;
    Heartbitflag= 0;
}

} // namespace Hippo

