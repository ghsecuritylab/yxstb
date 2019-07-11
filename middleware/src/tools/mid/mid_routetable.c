
#include "mid_routetable.h"

#include "app/Assertions.h"

#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <linux/if.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <netpacket/packet.h>
#include <linux/if_ether.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <fcntl.h>

#include "osex_net.h"

int dhcp_httpclient_connect(unsigned int remoteip, unsigned short remoteport,
                             unsigned int localip)
{
    int sockfd;
    int val;
    struct sockaddr_in local;
    struct sockaddr_in remote;
    int result;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd<0)
    {
        PRINTF("dhcp_httpclient_connect, socket failed.\n");
        return -1;
    }

    val = fcntl(sockfd, F_GETFL, 0);
    if (val == -1) 
        PRINTF("fcntl err\n");
    if (fcntl(sockfd, F_SETFL, val | O_NONBLOCK) == -1)
        PRINTF("fcntl err\n");

    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = localip;
    if (bind(sockfd, (const struct sockaddr *)&local, (socklen_t)sizeof(local))<0)
    {
        PRINTF("dhcp_httpclient_connect, %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = remoteip;
    remote.sin_port = remoteport;
    while (1)
    {
        result = connect(sockfd, (const struct sockaddr *)&remote, (socklen_t)sizeof(remote));
        if (result==0)
        {
            break;
        }
        if (result<0)
        {
            if (errno==EAGAIN || errno==EINTR)
            {
                continue;
            }
            else if (errno==EINPROGRESS || errno==EALREADY || errno==EISCONN)
            {
                break;
            }
            else
            {
                PRINTF("dhcp_httpclient_connect: %s\n", strerror(errno));
                close(sockfd);
                return -1;
            }
        }
    }
    return sockfd;
}

int dhcp_httpclient_send(int sockfd, const char *lpbuf, int len)
{
    fd_set fdset;
    struct timeval tv;
    int sentlen=0;
    int result=0;

    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);
    tv.tv_sec = INT_MAX;
    tv.tv_usec = 0;

    while (sentlen < len)
    {
        if (select(sockfd+1, NULL, &fdset, NULL, &tv)>0)
        {
            result = send(sockfd, lpbuf+sentlen, len-sentlen, 0);
            if (result<0)
            {
                if (errno==EAGAIN || errno==EWOULDBLOCK || errno==EINTR || errno==ENOBUFS || errno==ENOMEM)
                {
                    continue;
                }
                else
                {
                    PRINTF("dhcp_httpclient_send: %s\n", strerror(errno));
                    return -1;
                }
            }
            sentlen += result;
        }
    }
    return sentlen;
}

int dhcp_httpclient_recv(int sockfd, char **lpbuf)
{
    char recvbuf[1024];
    int result=0;
    char *pOutPutBuf=NULL;
    int OutPutLen=0;
    fd_set fdset;
    struct timeval tv;

    int blengthgot=0;
    int length=0;
    char *pContentLen=NULL;
    char *pContentLenEnd=NULL;
    char *pHeaderEnd=NULL;

    while (1)
    {
        FD_ZERO(&fdset);
        FD_SET(sockfd, &fdset);
        tv.tv_sec = INT_MAX;
        tv.tv_usec = 0;

        if (select(sockfd+1, &fdset, NULL, NULL, &tv)>0)
        {
            result = recv(sockfd, recvbuf, sizeof(recvbuf), 0);
            if (result == 0)
            {
                break;
            }
            if (result < 0)
            {
                if (errno==EINTR || errno==EAGAIN || errno==EWOULDBLOCK)
                {
                    continue;
                }
                else
                {
                    PRINTF("dhcp_httpclient_recv: %s\n", strerror(errno));
                    break;
                }
            }

            OutPutLen += result;
            if (pOutPutBuf==NULL)
            {
                pOutPutBuf = (char*)malloc(OutPutLen+1);
                if (!pOutPutBuf)
                    return -1;
            }
            else
            {
                pOutPutBuf = (char*)realloc(pOutPutBuf, OutPutLen+1);
                if (!pOutPutBuf)
                    return -1;
            }
            memcpy(pOutPutBuf+OutPutLen-result, recvbuf, result);

            if (blengthgot==0)
            {
                pContentLen = strstr(pOutPutBuf, "Content-Length: ");
                if (pContentLen!=NULL)
                {
                    pContentLen += strlen("Content-Length: ");
                    pContentLenEnd = pContentLen;
                    while(pContentLenEnd<pOutPutBuf+OutPutLen)
                    {
                        if (pContentLenEnd[0]==0x0d && pContentLenEnd[1]==0x0a)
                        {
                            break;
                        }
                        pContentLenEnd++;
                    }
                    if (pContentLenEnd < (pOutPutBuf+ OutPutLen -1) && pContentLenEnd[0]==0x0d && pContentLenEnd[1]==0x0a)
                    {
                        length = atoi(pContentLen);
                        blengthgot = 1;
                    }
                }
            }

            if (blengthgot==1)
            {
                pHeaderEnd = pOutPutBuf;
                while (pHeaderEnd < pOutPutBuf+OutPutLen-4)
                {
                    if (pHeaderEnd[0]==0x0d && pHeaderEnd[1]==0x0a && pHeaderEnd[2]==0x0d && pHeaderEnd[3]==0x0a)
                    {
                        break;
                    }
                    pHeaderEnd++;
                }
                if (pHeaderEnd < pOutPutBuf+OutPutLen-4 && (pHeaderEnd[0]==0x0d && pHeaderEnd[1]==0x0a && pHeaderEnd[2]==0x0d && pHeaderEnd[3]==0x0a) && ((OutPutLen - (pHeaderEnd - pOutPutBuf)) >= length))
                {
                    break;
                }
            }
        }
    }
    if (pOutPutBuf!=NULL)
    {
        pOutPutBuf[OutPutLen]='\0';
        *lpbuf = pOutPutBuf;
    }
    return OutPutLen;
}

int dhcp_httpclient_close(int sockfd)
{
    int val;

    val = fcntl(sockfd, F_GETFL, 0);
    if (val == -1) 
        PRINTF("fcntl err\n");
    if (fcntl(sockfd, F_SETFL, val & ~O_NONBLOCK) == -1)
        PRINTF("fcntl err\n");

    if (close(sockfd)<0)
    {
        PRINTF("close fail: %s\n", strerror(errno));
    }
    return 0;
}

int dhcp_route_get(unsigned int serverip, unsigned short serverport, unsigned int localip,
                   const unsigned char *lpMAC, const char *lpModelId,
                   const char *lpModelName, const char *lpSwVersion, const char *lpRouteVer,
                   const char *lpChipType, const char *lpCPUArch, char** ppRouteTable)
{
    int sockfd;
    struct in_addr address;
    char dot_decimal_address[16]="0.0.0.0";

    char method[100];
    char mOUI[100];
    char modelid[100];
    char modelname[100];
    char tvmac[100];
    char swver[100];
    char routever[100];

    char post[100];
    char uri[1000];
    char httpver[20];
    char header_accept[20];
    char header_agent[200];
    char header_host[30];
    char header_contentlen[30];
    char header_connection[30];
    char header_cache[30];

    int length=0;
    char *lpbuf=NULL;
    char *pContentlen=NULL;
    char *pContent=NULL;

    sprintf(method, "/etvbss/servlet/RouteServlet?Method=Service.GetRouteTable");
    sprintf(mOUI, "&ManufactureOUI=%02X%02X%02X", lpMAC[0], lpMAC[1], lpMAC[2]);
    sprintf(modelid, "&ModelID=%s", lpModelId);
    sprintf(modelname, "&ModelName=%s", lpModelName);
    sprintf(tvmac, "&tvMAC=%02X%02X%02X%02X%02X%02X", lpMAC[0], lpMAC[1], lpMAC[2], lpMAC[3], lpMAC[4], lpMAC[5]);
    sprintf(swver, "&SoftwareVersion=%s", lpSwVersion);
    sprintf(routever, "&RouteVersion=%s", lpRouteVer);

    sprintf(post, "POST");
    sprintf(uri, " %s%s%s%s%s%s%s", method, mOUI, modelid, modelname, tvmac, swver, routever);
    sprintf(httpver, " HTTP/1.1\r\n");
    sprintf(header_accept, "Accept: */*\r\n");
    sprintf(header_agent, "User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)\r\n");
    address.s_addr = serverip;
    inet_ntop(AF_INET, (const void*)&address, dot_decimal_address, 16);
    sprintf(header_host, "Host: %s:%d\r\n", dot_decimal_address, serverport);
    sprintf(header_contentlen, "Content-Length: %d\r\n", 0);
    sprintf(header_connection, "Connection: Keep Alive\r\n");
    sprintf(header_cache, "Cache-Control: no-cache\r\n\r\n");

    length = strlen(post)+strlen(uri)+strlen(httpver)+strlen(header_accept)+strlen(header_agent)+strlen(header_host)+
             strlen(header_contentlen)+strlen(header_connection)+strlen(header_cache);
    lpbuf = (char*)malloc(length+1);
    if (!lpbuf)
    {
        PRINTF("dhcp_route_get, malloc fail.\n");
        return -1;
    }
    sprintf(lpbuf, "%s%s%s%s%s%s%s%s%s", post, uri, httpver, header_accept, header_agent, header_host, header_contentlen,
            header_connection, header_cache);

    sockfd = dhcp_httpclient_connect(serverip, htons(serverport), localip);
    if (sockfd<0)
    {
        free(lpbuf);
        PRINTF("dhcp_route_get, dhcp_httpclient_connect fail.\n");
        return -1;
    }
    if (dhcp_httpclient_send(sockfd, lpbuf, length)<length)
    {
        dhcp_httpclient_close(sockfd);
        free(lpbuf);
        PRINTF("dhcp_route_get, dhcp_httpclient_send fail.\n");
        return -1;
    }
    free(lpbuf);
    lpbuf=NULL;
    length=0;

    length = dhcp_httpclient_recv(sockfd, &lpbuf);
    if (length<0)
    {
        dhcp_httpclient_close(sockfd);
        if (lpbuf)
            free(lpbuf);
        PRINTF("dhcp_route_get, dhcp_httpclient_recv fail.\n");
        return -1;
    }
    dhcp_httpclient_close(sockfd);

    if (memcmp(lpbuf, httpver+1, 8)!=0)
    {
        PRINTF("dhcp_route_get, http version error.\n");
        free(lpbuf);
        return -1;
    }
    if (memcmp(lpbuf+9, "200 OK", 3)!=0)
    {
        PRINTF("dhcp_route_get, http response error.\n");
        free(lpbuf);
        return -1;
    }

    pContentlen = strstr(lpbuf, "Content-Length: ");
    if (!pContentlen)
    {
        PRINTF("dhcp_route_get, get content length fail.\n");
        free(lpbuf);
        return -1;
    }
    length = atoi(pContentlen+16);

    pContent = pContentlen;
    while ((*pContent)!=0)
    {
        if (pContent[0]==0x0d && pContent[1]==0x0a && pContent[2]==0x0d && pContent[3]==0x0a)
            break;
        pContent += 1;
    }

    if (*pContent == '\0')
    {
        pContent = NULL;
    }

    if (length<=0 || pContent==NULL)
    {
        PRINTF("dhcp_route_get, find head ending fail.\n");
        free(lpbuf);
        return -1;
    }

    pContent += 4;
    *ppRouteTable = (char*)malloc(length+1);
    if ((*ppRouteTable)==NULL)
    {
        PRINTF("dhcp_route_get, malloc fail.\n");
        free(lpbuf);
        return -1;
    }
    memcpy((*ppRouteTable), pContent, length);
    (*ppRouteTable)[length]='\0';
    free(lpbuf);
    return length;
}

int get_dot_dec_num_len(char *p)
{
    int len;
    int count;
    char c;

    if (!p)
        return 0;
    len = 0;
    count = 0;
    while ((*p)!=0)
    {
        c = *p;
        if (isdigit(c))
        {
            p++;
            len++;
        }
        else if (c == '.')
        {
            if (count<3)
            {
                count++;
                len++;
                p++;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    return len;
}

int dhcp_route_parse(char *pRouteTable, int length, PDHCP_ROUTE_TABLE *ppTable)
{
    char *pHead=NULL;
    char *pTail=NULL;
    char *pLimit=NULL;
    PDHCP_ROUTE_ENTRY pEntry=NULL;
    PDHCP_ROUTE_ENTRY pEntryLast=NULL;
    char *pDestination=NULL;
    char *pNetmask=NULL;
    char *pPlane=NULL;
    char *pbyte=NULL;
    char *pbyte2=NULL;
    int addrlen=0;
    struct in_addr address;
    char dot_decimal_address_1[16]="";
    char dot_decimal_address_2[16]="";
    char dot_decimal_netmask[16]="";
    char szPlane[16];
    char ctail;

    if (!pRouteTable)
    {
        return -2;
    }

    if (!ppTable)
    {
        return -2;
    }

    if (strstr(pRouteTable, "<NullContent></NullContent>"))
    {
        *ppTable = NULL;
        return 0;
    }

    pHead = strstr(pRouteTable, "<RouteTableInfo>");
    pTail = strstr(pRouteTable, "</RouteTableInfo>");
    if (!pHead || !pTail || pTail < pHead)
    {
        return -1;
    }

    *ppTable = (PDHCP_ROUTE_TABLE)malloc(sizeof(DHCP_ROUTE_TABLE));
    if (!*ppTable)
    {
        return 1;
    }
    memset(*ppTable, 0, sizeof(DHCP_ROUTE_TABLE));

    pLimit = pTail;
    pRouteTable += strlen("<RouteTableInfo>");

    pHead = strstr(pRouteTable, "<TerminalInfo>");
    pTail = strstr(pRouteTable, "</TerminalInfo>");
    if (pHead && pTail && pLimit && pHead < pTail && pTail < pLimit)
    {
        pRouteTable = pHead + strlen("<TerminalInfo>");
        pHead = strstr(pRouteTable, "<Type>");
        pTail = strstr(pRouteTable, "</Type>");
        if (pHead && pTail && pLimit && pHead < pTail && pTail < pLimit)
        {
            pRouteTable = pHead + strlen("<Type>");
            memcpy((*ppTable)->terminaltype, pRouteTable, pTail-pRouteTable);
            pRouteTable = pTail + strlen("</Type>");
        }
        pHead = strstr(pRouteTable, "<Softver>");
        pTail = strstr(pRouteTable, "</Softver>");
        if (pHead && pTail && pLimit && pHead < pTail && pTail < pLimit)
        {
            pRouteTable = pHead + strlen("<Softver>");
            memcpy((*ppTable)->softver, pRouteTable, pTail-pRouteTable);
            pRouteTable = pTail + strlen("</Softver>");
        }
        pRouteTable = strstr(pRouteTable, "</TerminalInfo>") + strlen("</TerminalInfo>");

        pHead = strstr(pRouteTable, "<DNSSlect>");
        pTail = strstr(pRouteTable, "</DNSSlect>");
        if (pHead && pTail && pLimit && pHead < pTail && pTail < pLimit)
        {
            pRouteTable = pHead + strlen("<DNSSlect>");
            memcpy((*ppTable)->dnsselect, pRouteTable, pTail-pRouteTable);
            pRouteTable = pTail + strlen("</DNSSlect>");
        }

        pHead = strstr(pRouteTable, "<VLANID>");
        pTail = strstr(pRouteTable, "</VLANID>");
        if (pHead && pTail && pLimit && pHead < pTail && pTail < pLimit)
        {
            pRouteTable = pHead + strlen("<VLANID>");
            *pTail = '\0';
            (*ppTable)->vlanid = atoi(pRouteTable);
            *pTail = '<';
            pRouteTable = pTail + strlen("</VLANID>");
        }
        else
        {
            (*ppTable)->vlanid = 85;
        }

        pHead = strstr(pRouteTable, "<RouteVer>");
        pTail = strstr(pRouteTable, "</RouteVer>");
        if (pHead && pTail && pLimit && pHead < pTail && pTail < pLimit)
        {
            pRouteTable = pHead + strlen("<RouteVer>");
            memcpy((*ppTable)->routever, pRouteTable, pTail-pRouteTable);
            pRouteTable = pTail + strlen("</RouteVer>");
        }

        while (pRouteTable < pLimit  && strstr(pRouteTable, "<Routes>"))
        {
            pHead = strstr(pRouteTable, "<Routes>");
            pTail = strstr(pRouteTable, "</Routes>");
            if (pHead && pTail && pHead < pTail && pTail < pLimit)
            {
                pDestination = NULL;
                pNetmask = NULL;
                dot_decimal_netmask[0] = '\0';
                dot_decimal_address_1[0] = '\0';
                dot_decimal_address_2[0] = '\0';
                pPlane = NULL;
                szPlane[0] = '\0';

                pRouteTable = pHead + strlen("<Routes>");

                pHead = strstr(pRouteTable, "<Destination>");
                pTail = strstr(pRouteTable, "</Destination>");
                if (pHead && pTail && pHead < pTail && pTail < pLimit)
                {
                    pRouteTable = pHead + strlen("<Destination>");
                    pDestination = (char*)malloc(pTail-pRouteTable+1);
                    if (pDestination==NULL)
                    {
                        return 1;
                    }
                    memset(pDestination, 0, pTail-pRouteTable+1);
                    memcpy(pDestination, pRouteTable, pTail-pRouteTable);
                    pRouteTable = pTail + strlen("</Destination>");
                }

                if (strstr(pRouteTable, "<Netmask>") < strstr(pRouteTable, "<Plane>"))
                {
                    pHead = strstr(pRouteTable, "<Netmask>");
                    pTail = strstr(pRouteTable, "</Netmask>");
                    if (pHead && pTail && pHead < pTail && pTail < pLimit)
                    {
                        pRouteTable = pHead + strlen("<Netmask>");
                        pNetmask = (char*)malloc(pTail-pRouteTable+1);
                        if (pNetmask==NULL)
                        {
                            if (pDestination!=NULL)
                                free(pDestination);
                            return 1;
                        }
                        memset(pNetmask, 0, pTail-pRouteTable+1);
                        memcpy(pNetmask, pRouteTable, pTail-pRouteTable);
                        pRouteTable = pTail + strlen("</Netmask>");
                    }
                }

                pHead = strstr(pRouteTable, "<Plane>");
                pTail = strstr(pRouteTable, "</Plane");
                if (pHead && pTail && pHead < pTail && pTail < pLimit)
                {
                    pRouteTable = pHead + strlen("<Plane>");
                    pPlane = (char*)malloc(pTail-pRouteTable+1);
                    if (pPlane==NULL)
                    {
                        if (pDestination!=NULL)
                            free(pDestination);
                        if (pNetmask!=NULL)
                            free(pNetmask);
                        return 1;
                    }
                    memset(pPlane, 0, pTail-pRouteTable+1);
                    memcpy(pPlane, pRouteTable, pTail-pRouteTable);
                    pRouteTable = pTail + strlen("</Plane>");
                }
                pRouteTable = strstr(pRouteTable, "</Routes>") + strlen("</Routes>");

                if (pDestination)
                {
                    if (pNetmask!=NULL)
                    {
                        address.s_addr = inet_addr(pNetmask);
                        inet_ntop(AF_INET, (const void*)&address, dot_decimal_netmask, 16);
                    }

                    if (pPlane!=NULL)
                    {
                        strncpy(szPlane, pPlane, 16);
                    }

                    pbyte = pDestination;
                    while (*pbyte && pbyte < (pDestination+strlen(pDestination)))
                    {
                        pbyte2 = pbyte;
                        addrlen = get_dot_dec_num_len(pbyte);
                        if (addrlen>0)
                        {
                            pbyte += addrlen;

                            if (*pbyte=='\0')
                            {
                                address.s_addr = inet_addr(pbyte2);
                                inet_ntop(AF_INET, (const void*)&address, dot_decimal_address_1, 16);

                                pEntry = (PDHCP_ROUTE_ENTRY)malloc(sizeof(DHCP_ROUTE_ENTRY));
                                if (pEntry)
                                {
                                    memset(pEntry, 0, sizeof(DHCP_ROUTE_ENTRY));
                                    strncpy(pEntry->destip, dot_decimal_address_1, 15);
                                    strncpy(pEntry->netmask, dot_decimal_netmask, 15);
                                    strncpy(pEntry->plane, szPlane, 15);

                                    if ((*ppTable)->pentry==NULL)
                                    {
                                        (*ppTable)->pentry = pEntry;
                                        pEntryLast = pEntry;
                                    }
                                    else
                                    {
                                        pEntryLast->pnext = pEntry;
                                        pEntryLast = pEntry;
                                    }
                                }
                                else
                                {
									if (pDestination)
                                    	free(pDestination);
                                    if (pNetmask)
                                        free(pNetmask);
                                    if (pPlane)
                                        free(pPlane);
                                    return 1;
                                }

                                continue;
                            }

                            if (*pbyte==',')
                            {
                                *pbyte = '\0';
                                address.s_addr = inet_addr(pbyte2);
                                inet_ntop(AF_INET, (const void*)&address, dot_decimal_address_1, 16);

                                pEntry = (PDHCP_ROUTE_ENTRY)malloc(sizeof(DHCP_ROUTE_ENTRY));
                                if (pEntry)
                                {
                                    memset(pEntry, 0, sizeof(DHCP_ROUTE_ENTRY));
                                    strncpy(pEntry->destip, dot_decimal_address_1, 15);
                                    strncpy(pEntry->netmask, dot_decimal_netmask, 15);
                                    strncpy(pEntry->plane, szPlane, 15);

                                    if ((*ppTable)->pentry==NULL)
                                    {
                                        (*ppTable)->pentry = pEntry;
                                        pEntryLast = pEntry;
                                    }
                                    else
                                    {
                                        pEntryLast->pnext = pEntry;
                                        pEntryLast = pEntry;
                                    }
                                }
                                else
                                {
                                    if (pDestination)
                                    	free(pDestination);
                                    if (pNetmask)
                                        free(pNetmask);
                                    if (pPlane)
                                        free(pPlane);
                                    return 1;
                                }

                                *pbyte = ',';
                                pbyte++;
                                continue;
                            }

                            if (*pbyte=='-')
                            {
                                *pbyte = '\0';
                                address.s_addr = inet_addr(pbyte2);
                                inet_ntop(AF_INET, (const void*)&address, dot_decimal_address_1, 16);

                                *pbyte = '-';
                                pbyte++;

                                pbyte2 = pbyte;
                                addrlen = get_dot_dec_num_len(pbyte);
                                if (addrlen>0)
                                {
                                    pbyte += addrlen;
                                    ctail = *pbyte;
                                    *pbyte = '\0';

                                    address.s_addr = inet_addr(pbyte2);
                                    inet_ntop(AF_INET, (const void*)&address, dot_decimal_address_2, 16);

                                    *pbyte = ctail;
                                    pbyte++;

                                    while (ntohl(inet_addr(dot_decimal_address_1)) <= ntohl(inet_addr(dot_decimal_address_2)))
                                    {
                                        pEntry = (PDHCP_ROUTE_ENTRY)malloc(sizeof(DHCP_ROUTE_ENTRY));
                                        if (pEntry)
                                        {
                                            memset(pEntry, 0, sizeof(DHCP_ROUTE_ENTRY));
                                            strncpy(pEntry->destip, dot_decimal_address_1, 15);
                                            strncpy(pEntry->netmask, dot_decimal_netmask, 15);
                                            strncpy(pEntry->plane, szPlane, 15);

                                            if ((*ppTable)->pentry==NULL)
                                            {
                                                (*ppTable)->pentry = pEntry;
                                                pEntryLast = pEntry;
                                            }
                                            else
                                            {
                                                pEntryLast->pnext = pEntry;
                                                pEntryLast = pEntry;
                                            }
                                            address.s_addr = htonl(ntohl(inet_addr(dot_decimal_address_1))+1);
                                            inet_ntop(AF_INET, (const void*)&address, dot_decimal_address_1, 16);
                                        }
                                        else
                                        {
                                            if (pDestination)
                                    			free(pDestination);
                                            if (pNetmask)
                                                free(pNetmask);
                                            if (pPlane)
                                                free(pPlane);
                                            return 1;
                                        }
                                    }
                                }
                                else
                                {
                                    pEntry = (PDHCP_ROUTE_ENTRY)malloc(sizeof(DHCP_ROUTE_ENTRY));
                                    if (pEntry)
                                    {
                                        memset(pEntry, 0, sizeof(DHCP_ROUTE_ENTRY));
                                        strncpy(pEntry->destip, dot_decimal_address_1, 15);
                                        strncpy(pEntry->netmask, dot_decimal_netmask, 15);
                                        strncpy(pEntry->plane, szPlane, 15);

                                        if ((*ppTable)->pentry==NULL)
                                        {
                                            (*ppTable)->pentry = pEntry;
                                            pEntryLast = pEntry;
                                        }
                                        else
                                        {
                                            pEntryLast->pnext = pEntry;
                                            pEntryLast = pEntry;
                                        }
                                    }
                                    else
                                    {
                                        if (pDestination)
                                    		free(pDestination);
                                        if (pNetmask)
                                            free(pNetmask);
                                        if (pPlane)
                                            free(pPlane);
                                        return 1;
                                    }
                                }
                                continue;
                            }
                        }
                    }
                    if (pDestination){
                        free(pDestination);
						pDestination = NULL;
                    }	
                    if (pNetmask!=NULL) {
                        free(pNetmask);
						pNetmask = NULL;
                    }	
                    if (pPlane){
                        free(pPlane);
						pPlane = NULL;
                    }	
                }
            }
        }
    }
    if (pDestination)
        free(pDestination);
    if (pNetmask)
        free(pNetmask);
    if (pPlane)
        free(pPlane);
    return 2;
}

int dhcp_route_destroy(PDHCP_ROUTE_TABLE pRouteTable)
{
    PDHCP_ROUTE_ENTRY pEntry=NULL;
    PDHCP_ROUTE_ENTRY pEntryNext=NULL;

    if (!pRouteTable)
    {
        return 0;
    }
    pEntry = pRouteTable->pentry;
    while (pEntry)
    {
        pEntryNext = pEntry->pnext;
        free(pEntry);
        pEntry = pEntryNext;
    }
    free(pRouteTable);
    return 0;
}

int dhcp_route_show(PDHCP_ROUTE_TABLE pRouteTable)
{
    PDHCP_ROUTE_ENTRY pEntry=NULL;

    if (!pRouteTable)
        return -1;
    PRINTF("RouteTable:\n");
    PRINTF("TerminalType=%s\n", pRouteTable->terminaltype);
    PRINTF("Softver=%s\n", pRouteTable->softver);
    PRINTF("Dnsselect=%s\n", pRouteTable->dnsselect);
    PRINTF("Vlanid=%u\n", pRouteTable->vlanid);
    PRINTF("RouteVer=%s\n", pRouteTable->routever);

    pEntry = pRouteTable->pentry;
    while (pEntry)
    {
        PRINTF("Destination=%s\tNetmask=%s\tPlane=%s\n", pEntry->destip, pEntry->netmask, pEntry->plane);
        pEntry = pEntry->pnext;
    }
    return 0;
}

int dhcp_route_addtable(PDHCP_ROUTE_TABLE pRouteTable, const char *ip_a, const char *ip_b)
{
    PDHCP_ROUTE_ENTRY pEntry=NULL;
    char * pMask=NULL;

    if (!pRouteTable)
    {
        PRINTF("\n");
        return -1;
    }
    pEntry = pRouteTable->pentry;
    while (pEntry!=NULL)
    {
        PRINTF("ip=%s, mask=%s, gw=%s\n", pEntry->destip, pEntry->netmask, pEntry->plane);
        if (pEntry->netmask[0] != '\0')
        {
            pMask = pEntry->netmask;
        }
        else
        {
            pMask = "255.255.255.255";
        }
        if (strncmp(pEntry->plane, "A", 1) == 0)
        {
            if (osex_iproute_add(pEntry->destip, pMask, ip_a, 255))
            {
                PRINTF("add gateway route fail: %s, ip=%s,mask=%s,gw=%s\n", strerror(errno),
                    pEntry->destip, pMask, ip_a);
            }
        }
        else if (strncmp(pEntry->plane, "B", 1) == 0)
        {
            if (osex_iproute_add(pEntry->destip, pMask, ip_b, 255))
            {
                PRINTF("add gateway route fail: %s, ip=%s,mask=%s,gw=%s\n", strerror(errno),
                    pEntry->destip, pMask, ip_b);
            }
        }
        pEntry = pEntry->pnext;
    }
    return 0;
}

int dhcp_route_loadtable(const char *pPath, char **ppTable)
{
    FILE *fp=NULL;
    char *pContent=NULL;
    int length=0;


    if (!pPath || !ppTable)
    {
        PRINTF("dhcp_route_loadtable, null parameter.\n");
        return -1;
    }
    fp = fopen(pPath, "r");
    if (fp==NULL)
    {
        PRINTF("\n");
        return length;
    }
    if (!fseek(fp, 0, SEEK_END))
    {
        length = ftell(fp);
    }
    PRINTF("length=%d\n", length);
    if (length <= 0)
    {
        PRINTF("\n");
        fclose(fp);
        return 0;
    }
    rewind(fp);
    pContent = (char*)malloc(length+1);
    if (!pContent)
    {
        fclose(fp);
        PRINTF("dhcp_route_loadtable, memory limit.\n");
        return -1;
    }
    int readLen = fread(pContent, 1, length, fp);
    if ((readLen < length) && (!feof(fp))) {
        fclose(fp);
        free(pContent);
        PRINTF("fread err\n");
        return -1;        
    }
        
    pContent[length] = '\0';
    fclose(fp);
    *ppTable = pContent;
    return length;
}

int dhcp_route_savetable(const char *pPath, const char *pTable, int nLength)
{
    FILE *fp=NULL;

    if (!pPath || !pTable)
    {
        return -1;
    }
    fp = fopen(pPath, "w");
    if (fp==NULL)
    {
        return -1;
    }
    fwrite(pTable, 1, nLength, fp);
    fclose(fp);
    return 0;
}

int dhcp_line_check(unsigned int serverip, unsigned short serverport, unsigned int localip,
                   const unsigned char *lpMAC, const char *lpModelId,
                   const char *lpModelName, const char *lpSwVersion,
                   const char *lpChipType, const char *lpCPUArch, char** ppResult)
{
    int sockfd;
    struct in_addr address;
    char dot_decimal_address[16]="0.0.0.0";

    char method[200];
    char mOUI[100];
    char modelid[100];
    char modelname[100];
    char tvmac[100];
    char swver[100];
    char post[100];
    char uri[1000];
    char httpver[100];
    char header_accept[100];
    char header_agent[200];
    char header_host[100];
    char header_contentlen[30];
    char header_connection[30];
    char header_cache[30];

    int length=0;
    char *lpbuf=NULL;
    char *pContentlen=NULL;
    char *pContent=NULL;

    sprintf(method, "/etvbss/servlet/AutoCheckServlet?Method=Service.AutoSelftCheck");
    sprintf(mOUI, "&ManufactureOUI=%02X%02X%02X", lpMAC[0], lpMAC[1], lpMAC[2]);
    sprintf(modelid, "&ModelID=%s", lpModelId);
    sprintf(modelname, "&ModelName=%s", lpModelName);
    sprintf(tvmac, "&tvMAC=%02X%02X%02X%02X%02X%02X", lpMAC[0], lpMAC[1], lpMAC[2], lpMAC[3], lpMAC[4], lpMAC[5]);
    sprintf(swver, "&SoftwareVersion=%s", lpSwVersion);

    sprintf(post, "POST");
    sprintf(uri, " %s%s%s%s%s%s", method, mOUI, modelid, modelname, tvmac, swver);
    sprintf(httpver, " HTTP/1.1\r\n");
    sprintf(header_accept, "Accept: */*\r\n");
    sprintf(header_agent, "User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)\r\n");
    address.s_addr = serverip;
    inet_ntop(AF_INET, (const void*)&address, dot_decimal_address, 16);
    sprintf(header_host, "Host: %s:%d\r\n", dot_decimal_address, serverport);
    sprintf(header_contentlen, "Content-Length: %d\r\n", 0);
    sprintf(header_connection, "Connection: Keep Alive\r\n");
    sprintf(header_cache, "Cache-Control: no-cache\r\n\r\n");

    length = strlen(post)+strlen(uri)+strlen(httpver)+strlen(header_accept)+strlen(header_agent)+strlen(header_host)+
             strlen(header_contentlen)+strlen(header_connection)+strlen(header_cache);
    lpbuf = (char*)malloc(length+1);
    if (!lpbuf)
    {
        PRINTF("dhcp_line_check, malloc fail.\n");
        return -1;
    }
    sprintf(lpbuf, "%s%s%s%s%s%s%s%s%s", post, uri, httpver, header_accept, header_agent, header_host, header_contentlen,
            header_connection, header_cache);
    sockfd = dhcp_httpclient_connect(serverip, htons(serverport), localip);
    if (sockfd<0)
    {
        free(lpbuf);
        PRINTF("dhcp_line_check, dhcp_httpclient_connect fail.\n");
        return -1;
    }
    if (dhcp_httpclient_send(sockfd, lpbuf, length)<length)
    {
        dhcp_httpclient_close(sockfd);
        free(lpbuf);
        PRINTF("dhcp_line_check, dhcp_httpclient_send fail.\n");
        return -1;
    }
    free(lpbuf);
    lpbuf=NULL;
    length=0;

    length = dhcp_httpclient_recv(sockfd, &lpbuf);
    if (length<0)
    {
        dhcp_httpclient_close(sockfd);
        if (lpbuf)
        {
            free(lpbuf);
        }
        PRINTF("dhcp_line_check, dhcp_httpclient_recv fail.\n");
        return -1;
    }
    dhcp_httpclient_close(sockfd);

    if (memcmp(lpbuf, httpver+1, 8)!=0)
    {
        PRINTF("dhcp_line_check, http version error.\n");
        free(lpbuf);
        return -1;
    }
    if (memcmp(lpbuf+9, "200 OK", 3)!=0)
    {
        PRINTF("dhcp_line_check, http response error.\n");
        free(lpbuf);
        return -1;
    }

    pContentlen = strstr(lpbuf, "Content-Length: ");
    if (!pContentlen)
    {
        PRINTF("dhcp_line_check, get content length fail.\n");
        free(lpbuf);
        return -1;
    }
    length = atoi(pContentlen+16);

    pContent = pContentlen;
    while ((*pContent)!=0)
    {
        if (pContent[0]==0x0d && pContent[1]==0x0a && pContent[2]==0x0d && pContent[3]==0x0a)
            break;
        pContent += 1;
    }

    if (*pContent == '\0')
    {
        pContent = NULL;
    }
    
    if (length<=0 || pContent==NULL)
    {
        PRINTF("dhcp_line_check, find head ending fail.\n");
        free(lpbuf);
        return -1;
    }
    pContent += 4;

    *ppResult = (char*)malloc(length+1);
    if ((*ppResult)==NULL)
    {
        PRINTF("dhcp_line_check, malloc fail.\n");
        free(lpbuf);
        return -1;
    }
    memcpy((*ppResult), pContent, length);
    (*ppResult)[length]='\0';
    free(lpbuf);
    return length;
}

