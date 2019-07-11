#include <stdarg.h>/*包含va_list*/
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "cloud_api.h"

C_RESULT CStb_SocketOpen(OUT C_SocketHandle *pSocketHandle, IN  C_SocketType uType,IN  C_BOOL bIsBlocking)
{
	C_U32 ms = 0;
	C_U32 flags;
	
	ms = socket(AF_INET, (uType ==1?SOCK_STREAM:SOCK_DGRAM), 0);
	if(ms < 0){
		CLOUD_LOG_TRACE("open socket error\n");
		return CLOUD_FAILURE;
	}

	/*根据移植库的需求去设置套接字的阻塞和非阻塞*/
	flags = fcntl((C_U32)ms, F_GETFL, 0);
	if(bIsBlocking){
		//fcntl((C_U32)ms, F_SETFL, flags|O_NONBLOCK);
		if(fcntl((C_U32)ms, F_SETFL, flags&~O_NONBLOCK) < 0 ){
			CLOUD_LOG_TRACE("Open Socket opt ERROR line:%d\n", __LINE__);
		}
	}else{
		//fcntl((C_U32)ms, F_SETFL, flags&~O_NONBLOCK);  
		if(fcntl((C_U32)ms, F_SETFL, flags|O_NONBLOCK) < 0 ){
			CLOUD_LOG_TRACE("Open Socket opt ERROR line:%d\n", __LINE__);
		}
	}
	
	*pSocketHandle = (C_SocketHandle*)ms;
	return CLOUD_OK;
}

C_RESULT CStb_SocketConnect(IN  C_SocketHandle hSocketHandle, IN  C_SocketAddr const *pDstSocketAddr)
{
	struct sockaddr_in serv_addr;
	C_U32 p_connect = 0;
	C_U32 flags;
	
	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = pDstSocketAddr->uIP;
	serv_addr.sin_port = pDstSocketAddr->uPort;
	p_connect = connect((C_U32)hSocketHandle, &serv_addr, sizeof(serv_addr));

	flags = fcntl((C_U32)hSocketHandle, F_GETFL, 0);

	if(flags & O_NONBLOCK){
		CLOUD_LOG_TRACE("Cable Connect Socket is NoBlock\n");
		return CLOUD_OK;
	}else{
		if(!p_connect){
			return CLOUD_OK;
		}else{
			CLOUD_LOG_TRACE("Block type socket and connect failure\n");
			return CLOUD_FAILURE;
		}
	}
}
C_RESULT CStb_SocketSetOpt(IN C_SocketHandle hSocketHandle, IN C_SocketOptLevel uLevel, IN C_SocketOptName uOptname, IN C_U8 const *pOptval, IN C_U32 uOptlen)
{
    C_RESULT ret = CLOUD_OK;

    if ( uLevel == SocketOptLevel_IPPROTO_TCP && uOptname == SocketOptName_TCP_NODELAY ){
        unsigned long nodelay = *(C_BOOL*)pOptval;
        if (setsockopt((C_U32)hSocketHandle,IPPROTO_TCP,1, pOptval, uOptlen) != 0){
            ret = CLOUD_FAILURE;
        }
    }else if ( uLevel == SocketOptLevel_SOL_SOCKET ){
        short optName = 0;
        switch (uOptname){
        case SocketOptName_SO_RCVBUF:
            optName = SO_RCVBUF;
        	break;
        case SocketOptName_SO_BROADCAST:
            optName = SO_BROADCAST;
            break;
        case SocketOptName_SO_SNDBUF:
            optName = SO_SNDBUF;
            break;
        case SocketOptName_SO_REUSEADDR:
            optName = SO_REUSEADDR;
            break;
        }

        if (setsockopt((C_U32)hSocketHandle,SOL_SOCKET,optName, pOptval, uOptlen) != 0){
            ret = CLOUD_FAILURE;
        }
    }else if ( uLevel == SocketOptLevel_FILEIO && uOptname == SocketOptName_FILEIO_NBIO ){	
    	C_BOOL g_pOptval = *pOptval;
		C_U32 flags;
		flags = fcntl((C_U32)hSocketHandle, F_GETFL, 0);
    	if(g_pOptval){
	    	if(fcntl((C_U32)hSocketHandle, F_SETFL, flags|O_NONBLOCK)<0)
			CLOUD_LOG_TRACE("set opt ERROR line:%d\n", __LINE__);
    	}else{
	    	if(fcntl((C_U32)hSocketHandle, F_SETFL, flags&~O_NONBLOCK)<0)
				CLOUD_LOG_TRACE("set opt ERROR line:%d\n", __LINE__);
		}
    }
	
    return ret;
}
C_NetworkIP CStb_SocketGetHostByName(IN  char const *pDomainName )
{
	struct hostent *pHost= NULL;
	C_NetworkIP iphost = 0;
	pHost=gethostbyname(pDomainName);
	if(pHost){
		iphost = ((struct in_addr *)(pHost->h_addr_list[0]))->s_addr;
		return iphost;
	}else{
		return 0;
	}
}

static C_U32 sockfd_read[FD_SETSIZE];	
static C_U32 sockfd_write[FD_SETSIZE];	
static C_U32 sockfd_error[FD_SETSIZE];	
C_RESULT CStb_SocketSelect(INOUT C_SocketHandle *pReadSockets, IN C_U32 uReadSocketCount, INOUT C_SocketHandle *pWriteSockets,
    IN C_U32 uWriteSocketCount, INOUT C_SocketHandle *pExceptSockets, IN C_U32 uExceptSocketCount, IN C_U32 uTimeout)
{
	fd_set fd_read, fd_write, fd_error;
	C_U32 ix, ret = 0, jx = 0;
	struct timeval timeOut;

	FD_ZERO(&fd_read);
	FD_ZERO(&fd_write);
	FD_ZERO(&fd_error);
	timeOut.tv_sec = uTimeout/1000;
	timeOut.tv_usec = (uTimeout%1000)*1000;	
	memset(sockfd_read, 0, sizeof(C_U32)*FD_SETSIZE);
	memset(sockfd_write, 0, sizeof(C_U32)*FD_SETSIZE);
	memset(sockfd_error, 0, sizeof(C_U32)*FD_SETSIZE);

	memcpy((void*)sockfd_read, pReadSockets, sizeof(C_U32)*uReadSocketCount);
	memcpy((void*)sockfd_write, pWriteSockets, sizeof(C_U32)*uWriteSocketCount);
	memcpy((void*)sockfd_error, pExceptSockets, sizeof(C_U32)*uExceptSocketCount);
	sockfd_read[uReadSocketCount] = 0;
	sockfd_write[uWriteSocketCount] = 0;
	sockfd_error[uExceptSocketCount] = 0;
	
	for(ix = 0; ix < uReadSocketCount; ix++){
		FD_SET(*(sockfd_read+ix), &fd_read);		
	}
	for(ix = 0; ix < uWriteSocketCount; ix++){
		FD_SET(*(sockfd_write+ix), &fd_write);		
	}	
	for(ix = 0; ix < uExceptSocketCount; ix++){
		FD_SET(*(sockfd_error+ix), &fd_error);		
	}

#if 0
	/*debug output.*/
	int idx = 0;
	printf("cloud socket readfd:");
	for(idx = 0; idx < uReadSocketCount; idx++){
		printf("%d ",(C_U32)sockfd_read[idx]);
	}
	printf("\n------------------\n");

	printf("cloud socket writefd:");
	for(idx = 0; idx < uReadSocketCount; idx++){
		printf("%d ",(C_U32)sockfd_write[idx]);
	}
	printf("\n------------------\n");

	printf("cloud socket errorfd:");
	for(idx = 0; idx < uReadSocketCount; idx++){
		printf("%d ",(C_U32)sockfd_error[idx]);
	}
	printf("\n------------------\n");
#endif
	
	memset(pReadSockets, 0, sizeof(C_U32)*uReadSocketCount);
	memset(pWriteSockets, 0, sizeof(C_U32)*uWriteSocketCount);
	memset(pExceptSockets, 0, sizeof(C_U32)*uExceptSocketCount);

	ret = select(FD_SETSIZE, &fd_read, &fd_write, &fd_error, &timeOut);
	if(ret < 0){
		CLOUD_LOG_TRACE("socket select failed\n");
		perror("@@@@@@@@ , socket select \n");
		return CLOUD_FAILURE;
	}else if(ret == 0){
		//CLOUD_LOG_TRACE("socket select timeout\n");
		return CLOUD_TIMEOUT;
	}

	for(ix = 0; ix < uReadSocketCount; ix++){
		if(FD_ISSET(*(sockfd_read+ix), &fd_read) != 0){
			*((C_U32*)pReadSockets+jx) = *(sockfd_read+ix);
			jx++;
		}
	}	

#if 0
	printf("cloud socket readfd to be pReadSockets:");
	for(idx = 0; idx < jx; idx++){
		printf("%d ",(C_U32)pReadSockets[idx]);
	}
	printf("\n------------------\n");
#endif
	jx = 0;
	
	for(ix = 0; ix < uWriteSocketCount; ix++){
		if(FD_ISSET(*(sockfd_write+ix), &fd_write) != 0){
			*((C_U32*)pWriteSockets+jx) = *(sockfd_write+ix);
			jx++;
		}
	}
#if 0
	printf("cloud socket writefd to be pWriteSockets:");
	for(idx = 0; idx < jx; idx++){
		printf("%d ",(C_U32)pWriteSockets[idx]);
	}
	printf("\n------------------\n");
#endif
	jx = 0;
	
	for(ix = 0; ix < uExceptSocketCount; ix++){
		if(FD_ISSET(*(sockfd_error+ix), &fd_error) != 0){
			*((C_U32*)pExceptSockets+jx) = *(sockfd_error+ix);
			jx++;
		}
	}
#if 0
	printf("cloud socket Exceptfd to be pExceptSockets:");
	for(idx = 0; idx < jx; idx++){
		printf("%d ",(C_U32)pExceptSockets[idx]);
	}
	printf("\n------------------\n");
#endif
	jx = 0;

	
	return CLOUD_OK;
}

C_RESULT  CStb_SocketSend(IN C_SocketHandle hSocketHandle, IN C_U8 const *pBuf,
    IN C_U32 uBytesToSend, OUT C_U32 *puBytesSent)
{
	C_U32 count = 0;
	count = (C_U32)send((C_U32)hSocketHandle, (void*)pBuf, uBytesToSend,0);
	*puBytesSent = count;
	if((int)count > 0)
		return CLOUD_OK;
	CLOUD_LOG_TRACE("CStb_SocketSend errorhandle= %d\n", (C_U32)hSocketHandle);
	return CLOUD_FAILURE;
}

C_RESULT  CStb_SocketRecv(IN C_SocketHandle hSocketHandle, OUT C_U8 *pBuf, IN C_U32 uBytesToReceive, OUT C_U32 *puBytesReceived)
{
	C_U32 p_recv = 0;

RECV_AGAIN:
	p_recv = recv((C_U32)hSocketHandle, pBuf, uBytesToReceive, 0);
	*puBytesReceived = p_recv;
	if(p_recv > 0 && p_recv != 0xFFFFFFFF){ 
	//	CLOUD_LOG_TRACE("handle =%d, p_recvLen = %#x\n", (C_U32)hSocketHandle, p_recv);
		return CLOUD_OK;
	}else{
		//perror("@@@@@@@@ , soceket receive err \n");
		if(errno == EAGAIN){
			//CLOUD_LOG_TRACE("socket receive data error, readagain, handle:%d, p_recv = %#x\n", (C_U32)hSocketHandle,p_recv);
			goto RECV_AGAIN;
		}
	}
	
	CLOUD_LOG_TRACE("socket receive data error, handle:%d, p_recv = %#x\n", (C_U32)hSocketHandle,p_recv);
	*puBytesReceived = 0;
	return CLOUD_FAILURE;
}

C_RESULT  CStb_SocketSendTo(IN C_SocketHandle hSocketHandle, IN C_SocketAddr const *pSocketAddr, IN C_U8 const *pBuf,
    IN C_U32 uBytesToSend, OUT C_U32 *puBytesSent)
{
	struct sockaddr_in addr;
	C_U32 p_sendto = 0;
	
	addr.sin_family=AF_INET;
	addr.sin_port= pSocketAddr->uPort;
	addr.sin_addr.s_addr=pSocketAddr->uIP;
	p_sendto=sendto((C_U32)hSocketHandle, pBuf, uBytesToSend, 0, (struct sockaddr *)&addr, sizeof(addr));
	*puBytesSent = p_sendto;
	if(*puBytesSent)
		return CLOUD_OK;
	CLOUD_LOG_TRACE("CStb_SocketSendTo errorhandle= %d\n", (C_U32)hSocketHandle);
	return CLOUD_FAILURE;
}

C_RESULT  CStb_SocketReceiveFrom(IN C_SocketHandle hSocketHandle, OUT C_SocketAddr *pSocketAddr, OUT C_U8 *pBuf, 
	IN C_U32 uBytesToReceive, OUT C_U32 *pBytesReceived)
{
	struct sockaddr_in addr;
	int len=sizeof(addr);
	C_U32 p_revfrom = 0;
	
	p_revfrom=recvfrom((C_U32)hSocketHandle,(void*)pBuf, uBytesToReceive,0,(struct sockaddr *)&addr,&len);
	pSocketAddr->uIP = addr.sin_addr.s_addr;
	pSocketAddr->uPort = addr.sin_port;
	*pBytesReceived = p_revfrom;
	if(p_revfrom)
		return CLOUD_OK;
	CLOUD_LOG_TRACE("CStb_SocketReceiveFrom error,handle= %d\n", (C_U32)hSocketHandle);
	return CLOUD_FAILURE;
}

C_RESULT CStb_SocketClose(IN C_SocketHandle hSocketHandle)
{
	C_U32 p_close = 0;
	CLOUD_LOG_TRACE("SocketClose=%d\n", (C_U32)hSocketHandle);
	p_close = (C_U32)close((C_U32)hSocketHandle);
	return CLOUD_OK;
}

