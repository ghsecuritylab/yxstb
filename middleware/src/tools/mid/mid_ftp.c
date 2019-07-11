//==========================================================================
//
//      ftpclient.c
//
//      A simple FTP client
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Andrew Lunn.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    andrew.lunn@ascom.ch
// Contributors: andrew.lunn@ascom.ch
// Date:         2001-11-4
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include "app/Assertions.h"

#include "mid_ftp.h"
#include "ind_mem.h"

#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct mid_ftp {
	int ctrl_s;
	int data_s;
	char msgbuf[BUFSIZ];
};

static int ftp_port;
/* Build one command to send to the FTP server */
static int 
build_cmd(char *buf,
					unsigned bufsize,
					char *cmd, 
					char *arg1)
{
	int cnt;
	
	if (arg1) {
		cnt = snprintf(buf,bufsize,"%s %s\r\n",cmd,arg1);
	} else {
		cnt = snprintf(buf,bufsize,"%s\r\n",cmd);
	}
	
	if (cnt < bufsize) 
		return 1;
	
	return 0;
}


/* Read one line from the server, being careful not to overrun the
	 buffer. If we do reach the end of the buffer, discard the rest of
	 the line. */
static int 
get_line(int s, char *buf, unsigned buf_size) {
	
	int eol = 0;
	int cnt = 0;
	int ret;
	char c;
	
	while(!eol) {
		ret = recv(s,&c,1, 0);
		
		if (ret != 1) {
			ERR_PRN("read errno = %d / %s\n", errno, strerror(errno));
			return -1;
		}
		
		if (c == '\n') {
			eol = 1;
			continue;
		}

		if (cnt < buf_size) {
			buf[cnt++] = c;
		}   
	}
	if (cnt < buf_size) {
		buf[cnt++] = '\0';
	} else {
		buf[buf_size -1] = '\0';
	}
	return 0;
}  

/* Read the reply from the server and return the MSB from the return
	 code. This gives us a basic idea if the command failed/worked. The
	 reply can be spread over multiple lines. When this happens the line
	 will start with a - to indicate there is more*/
static int 
get_reply(int s, char* msgbuf, int msgbuflen) {

	int more = 0;
	int ret;
	int first_line=1;
	int code=0;
	
	do {
		
		if ((ret=get_line(s,msgbuf,msgbuflen)) < 0) {
			return(ret);
		}
		
		PRINTF("FTP: %s\n",msgbuf);
		
		if (first_line) {
			code = strtoul(msgbuf,NULL,0);
			first_line=0;
			more = (msgbuf[3] == '-');
		} else {
			if (isdigit(msgbuf[0]) && isdigit(msgbuf[1]) && isdigit(msgbuf[2]) &&
					(code == strtoul(msgbuf,NULL,0)) && 
					msgbuf[3]==' ') {
				more=0;
			} else {
				more =1;
			}
		}
	} while (more);

	return (msgbuf[0] - '0');
}

/* Send a command to the server */
static int 
send_cmd(int s,char * msgbuf) {
	
	int len;
	int slen = strlen(msgbuf);
	
	if ((len = send(s,msgbuf,slen,MSG_NOSIGNAL)) != slen) {
		if (len < 0) {
			ERR_PRN("write\n");
			return -1;
		} else {
			ERR_PRN("write truncated!\n");
			return -1;
		}
	}
	return 1;
}

/* Send a complete command to the server and receive the reply. Return the 
	 MSB of the reply code. */
static int 
command(char * cmd, 
				char * arg, 
				int s, 
				char *msgbuf, 
				int msgbuflen) {

	int err;
	
	if (!build_cmd(msgbuf,msgbuflen,cmd,arg)) {
		ERR_PRN("%s command to long\n",cmd);
		return -1;
	}

	PRINTF("FTP: Sending %s command\n",cmd);
	
	if ((err=send_cmd(s,msgbuf)) < 0) {
		return(err);
	}

	return (get_reply(s, msgbuf, msgbuflen));
}

/* Open a socket and connect it to the server. Also print out the
	 address of the server for debug purposes.*/

static int
openctrlsock(char *hostname)
{ 
	int s = -1;
	struct sockaddr_in servaddr;

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		ERR_OUT("socket s = %d\n", s);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(21);
	servaddr.sin_addr.s_addr = inet_addr(hostname);

	if (connect(s, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		ERR_OUT("connect %s failed\n", hostname);

	return s;
Err:
	if (s >= 0)
		close(s);
	return -1;
}

/* Perform a login to the server. Pass the username and password and
	 put the connection into binary mode. This assumes a passwd is
	 always needed. Is this true? */

static int 
login(int s, 
			char *username, 
			char *password, 
			char *msgbuf, 
			unsigned msgbuflen) {
	
	int ret;

	ret = command("USER",username,s,msgbuf,msgbuflen);
	if (ret != 3) {
		ERR_PRN("User not accepted\n");
		return -1;
	}
	
	ret = command("PASS",password,s,msgbuf,msgbuflen);
	if (ret < 0) {
		return -1;
	}
	if (ret != 2) {
		ERR_PRN("Login failed \n");
		return -1;
	}
	
	PRINTF("FTP: Login sucessfull\n");
	
	ret = command("TYPE","I",s,msgbuf,msgbuflen);
	if (ret < 0) {
		return -1;
	}
	if (ret != 2) {
		ERR_PRN("TYPE failed!\n");
		return -1;
	}
	return (ret);
}


/* Open a data socket. This is a client socket, i.e. its listening
waiting for the FTP server to connect to it. Once the socket has been
opened send the port command to the server so the server knows which
port we are listening on.*/
static int 
opendatasock(int ctrl_s,
					   char *msgbuf, 
					   unsigned msgbuflen) {
	struct sockaddr_in servaddr;
	char ip[32];
	char *p;
	int ret;
	int s;
	int i1,i2,i3,i4,i5,i6;
	short port;

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		ERR_PRN("socket\n");
		return -1;
	}

	ret = command("PASV","",ctrl_s,msgbuf,msgbuflen);
	if (ret < 0) {
		close(s);
		return (ret);
	}
	if (ret != 2) {
		ERR_PRN("PORT failed!\n");
		close(s);
		return (-1);
	}

	p = strchr(msgbuf, '(');//Õë¶ÔServ-U
	if (!p) {
		ERR_PRN("strchr return null!");
		close(s);
		return -1;
	}
	p ++;
	ret = sscanf(p, "%d,%d,%d,%d,%d,%d", &i1, &i2, &i3, &i4, &i5, &i6);
	if (ret != 6) {
			ERR_PRN("sscanf %d!\n", ret);
			close(s);
			return (-1);
	}

	sprintf(ip, "%d.%d.%d.%d", i1, i2, i3, i4);
	port = i5*256 + i6;

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(ip);

		if (connect(s, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
			ERR_PRN("connect\n");
			close (s);
			return -1;
		}

	return (s);
}

/* All done, say bye, bye */
static int quit(int s, 
					      char *msgbuf, 
					      unsigned msgbuflen) {
	
	int ret;
	
	ret = command("QUIT",NULL,s,msgbuf,msgbuflen);
	if (ret < 0) {
		return (ret);
	}
	if (ret != 2) {
		ERR_PRN("Quit failed!\n");
		return (-1);
	}
	
	PRINTF("Connection closed\n");
	return (0);
}

static int mid_ftp_init(mid_ftp_t ftp)
{
	if (ftp == NULL)
		ERR_OUT("ftp is NULL\n");

	ftp->ctrl_s = -1;
	ftp->data_s = -1;

	return 0;
Err:
	return -1;
}

mid_ftp_t mid_ftp_create(void)
{
	mid_ftp_t ftp;

	ftp = (mid_ftp_t)IND_MALLOC(sizeof(struct mid_ftp));
	if (ftp == NULL)
		ERR_OUT("malloc %d failed\n", sizeof(struct mid_ftp));

	mid_ftp_init(ftp);

	return ftp;
Err:
	return NULL;
}

void mid_ftp_delete(mid_ftp_t ftp)
{
	if (ftp == NULL)
		return;
	if (ftp->ctrl_s >= 0)
		mid_ftp_close(ftp);
	IND_FREE(ftp);
}

int mid_ftp_open(mid_ftp_t ftp, char *hostname, char *username, char *password)
{
	int ret;

	if (ftp == NULL)
		ERR_OUT("ftp is NULL\n");

	ftp->ctrl_s = openctrlsock(hostname);
	if (ftp->ctrl_s < 0)
		ERR_OUT("ftp->ctrl_s = %d\n", ftp->ctrl_s);

	/* Read the welcome message from the server */
	ret = get_reply(ftp->ctrl_s, ftp->msgbuf, BUFSIZ);
	if (ret != 2)
	{
	  PRINTF("get_reply ret = %d\n", ret);
      return -2;
	}

	ret = login(ftp->ctrl_s,username,password,ftp->msgbuf,BUFSIZ);
	if (ret < 0)
		ERR_OUT("login ret = %d\n", ret);

	return 0;

Err:
	if (ftp) {
		if (ftp->ctrl_s >= 0)
				close(ftp->ctrl_s);
			ftp->ctrl_s = -1;
	}
	return -1;
}

int mid_ftp_close(mid_ftp_t ftp)
{
	if (ftp == NULL)
		ERR_OUT("ftp is NULL\n");

	if (ftp->ctrl_s == -1)
		ERR_OUT("ftp->ctrl_s = %d\n", ftp->ctrl_s);

	if (ftp->data_s != -1) {
		WARN_PRN("ftp file not close!\n");
		mid_ftp_put_end(ftp);
	}

	quit(ftp->ctrl_s,ftp->msgbuf,BUFSIZ);

	close(ftp->ctrl_s);
	ftp->ctrl_s = -1;

	return 0;
Err:
	return -1;
}

int mid_ftp_put_begin(mid_ftp_t ftp, char *filename)
{
	int ret;

	if (ftp == NULL)
		ERR_OUT("ftp is NULL\n");
	if (ftp->ctrl_s == -1)
		ERR_OUT("ftp->ctrl_s = %d\n", ftp->ctrl_s);
	if (ftp->data_s != -1)
		ERR_OUT("ftp->data_s = %d\n", ftp->data_s);

	ftp->data_s = opendatasock(ftp->ctrl_s,ftp->msgbuf,BUFSIZ);
	if (ftp->data_s < 0)
		ERR_OUT("opendatasock = %d\n", ftp->data_s);

	ret = command("STOR",filename,ftp->ctrl_s,ftp->msgbuf,BUFSIZ);
	if (ret < 0)
		ERR_OUT("command = %d\n", ret);

	if (ret != 1)
		goto Err;

	return 0;
Err:
	if (ftp) {
		if (ftp->data_s >= 0)
			close(ftp->data_s);
		ftp->data_s = -1;
	}
	return -1;
}

int mid_ftp_put_data(mid_ftp_t ftp, char *buf, int size)
{
	int len;

	if (ftp == NULL)
		ERR_OUT("ftp is NULL\n");
	if (ftp->data_s == -1)
		ERR_OUT("ftp->data_s = %d\n", ftp->data_s);
	do {
		len = send(ftp->data_s, buf, size, MSG_NOSIGNAL);
		if (len < 0)
			ERR_OUT("send %d/%d\n", len, size);

		if (len == size)
			break;
		size -= len;
		buf += len;
	} while (1);

	return 0;
Err:
	return -1;
}

int mid_ftp_put_end(mid_ftp_t ftp)
{
	int ret;

	if (ftp == NULL)
		ERR_OUT("ftp is NULL\n");
	if (ftp->data_s == -1)
		ERR_OUT("ftp->data_s = %d\n", ftp->data_s);

	close(ftp->data_s);
	ftp->data_s = -1;

	ret = get_reply(ftp->ctrl_s, ftp->msgbuf, BUFSIZ);
	if (ret != 2)
		ERR_OUT("Transfer failed! %d\n", ret);

	return 0;
Err:
	return -1;
}



int mid_ftp_url_check(char* url, char *host, int host_len, char *uri, int uri_len)
{
	char *p;
	int len;

	if (url == NULL || host == NULL || uri == NULL)
		ERR_OUT("url = %p, host = %p, uri = %p\n", url, host, uri);
	if (strncasecmp(url, "ftp://", 6))
		ERR_OUT("url error! %s\n", url);

	url += 6;
	/* lh 2010-7-26 */
       p=strchr(url,':');
	 if(p==NULL)
	 /*end*/
	 {
	  p = strchr(url, '/');
	  if (p == NULL)
	  ERR_OUT("uri is NULL\n");
	 }

	len = (int)(p - url);
	if (len >= host_len)
		ERR_OUT("host_len = %d / %d\n", host_len, len);
	memcpy(host, url, len);
	host[len] = 0;
	 p = strchr(url, '/');
	  if (p == NULL || p[1] == 0)
	  ERR_OUT("uri is NULL\n");
	p++;  
	len = (int)strlen(p);
	if (len >= uri_len)
		ERR_OUT("uri_len = %d / %d\n", uri_len, len);
	strcpy(uri, p);

	return 0;
Err:
	return -1;
}

int mid_ftp_post(char *url, char *username, char *password, char *content, int length)
{
	char uri[200 + 4];
	char host[32 + 4];
	struct mid_ftp ftp;

	mid_ftp_init(&ftp);

	PRINTF("url = %s\n", url);
	if (url == NULL || username == NULL || password == NULL || content == NULL)
		ERR_OUT("url = %p, username = %p, password = %p, content = %p\n", url, username, password, content);
	if (mid_ftp_url_check(url, host, 32, uri, 200))
		ERR_OUT("url = %s\n", url);

	PRINTF("uri = %s\n", uri);
	if (mid_ftp_open(&ftp, host, username, password))
		ERR_OUT("mid_ftp_open failed\n");

	if (mid_ftp_put_begin(&ftp, uri))
		ERR_OUT("mid_ftp_put_begin failed\n");
	if (mid_ftp_put_data(&ftp, content, length))
		ERR_OUT("mid_ftp_put_data failed\n");
	mid_ftp_put_end(&ftp);
	mid_ftp_close(&ftp);

	return 0;
Err:
	if (ftp.ctrl_s != -1)
		mid_ftp_close(&ftp);
	return -1;
}

int mid_ftp_get_begin(mid_ftp_t ftp, char *filename)
{
    int ret;

	if(ftp == NULL)
		ERR_OUT("ftp is NULL\n");
	if(ftp->ctrl_s == -1)
		ERR_OUT("ftp->ctrl_s = %d\n", ftp->ctrl_s);
	if(ftp->data_s != -1)
		ERR_OUT("ftp->data_s = %d\n", ftp->data_s);

	ftp->data_s = opendatasock(ftp->ctrl_s, ftp->msgbuf, BUFSIZ);
	if (ftp->data_s < 0)
		ERR_OUT("opendatasock = %d\n", ftp->data_s);

	ret = command("RETR",filename,ftp->ctrl_s, ftp->msgbuf, BUFSIZ);
	if (ret < 0)
		ERR_OUT("command = %d\n", ret);

	if (ret != 1)
		goto Err;

	return 0;
Err:
	if(ftp) {
		if(ftp->data_s >= 0)
			close(ftp->data_s);
		ftp->data_s = -1;
	}
	return -1;
}

int mid_ftp_get_data_to(mid_ftp_t ftp, char * buf, int size, int ms)
{
    int len;
    char *bufp = buf;
    int remaining = size;
    int finished = 0;
    int total_size=0;

    if(ftp == NULL)
        ERR_OUT("ftp is NULL\n");
    if(ftp->data_s == -1)
        ERR_OUT("ftp->data_s = %d\n", ftp->data_s);

    int total_to = 0;
    do {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(ftp->data_s, &rfds);
        struct timeval  tm;
        tm.tv_sec = 0;
        tm.tv_usec = 500000;
        int timeout = tm.tv_sec * 1000 + tm.tv_usec / 1000;
        int ret = select(ftp->data_s + 1, &rfds, NULL, NULL, &tm);
        if (ret > 0) {
            total_to = 0;

            len = read(ftp->data_s,bufp,remaining);
            if (len < 0) {
                close(ftp->data_s);
                ERR_OUT("FTP BAD");
            }
            if (len == 0) {
                finished = 1;
            } else {
                total_size += len;
                remaining -= len;
                bufp += len;

                if (total_size > size) {
                    close(ftp->data_s);
                    ERR_OUT("FTP: File too big!\n");
                }
            }
        } else if (ret == 0) {
            total_to += timeout;
            if (total_to >= ms)
                ERR_OUT("FTP: select timedout!\n");
        } else {
            close(ftp->data_s);
            ERR_OUT("FTP: select fail!\n");
        }

    }while(!finished);

    return total_size;
Err:
    return -1;

}

int mid_ftp_get_data(mid_ftp_t ftp, char *buf, int size)
{
	int len;
	char *bufp = buf;
	int remaining = size;
	int finished = 0;
	int total_size=0;

	if(ftp == NULL)
		ERR_OUT("ftp is NULL\n");
	if(ftp->data_s == -1)
		ERR_OUT("ftp->data_s = %d\n", ftp->data_s);

	do {
		len = read(ftp->data_s,bufp,remaining);
		if (len < 0) {
			close(ftp->data_s);
			ERR_OUT("FTP BAD");
		}

		if (len == 0) {
			finished = 1;
		} else {
			total_size += len;
			remaining -= len;
			bufp += len;

			if (total_size > size) {
				close(ftp->data_s);
				ERR_OUT("FTP: File too big!\n");
			}
		}

	}while(!finished);
	
	return total_size;
Err:
	return -1;
}

int mid_ftp_get_end(mid_ftp_t ftp)
{
	int ret;

	if(ftp == NULL)
		ERR_OUT("ftp is NULL\n");
	if(ftp->data_s == -1)
		ERR_OUT("ftp->data_s = %d\n", ftp->data_s);

	close(ftp->data_s);
	ftp->data_s = -1;

	ret = get_reply(ftp->ctrl_s, ftp->msgbuf, BUFSIZ);
	if (ret != 2)
		ERR_OUT("Transfer failed! %d\n", ret);

	return 0;
Err:
	return -1;
}
