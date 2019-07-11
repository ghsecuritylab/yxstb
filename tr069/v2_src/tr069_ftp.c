//==========================================================================
//
//      ftpclient.c
//
//      A simple FTP client
//      http://sources.redhat.com/ecos/ecos-license/
//
//==========================================================================

#include "tr069_ftp.h"
#include "tr069_header.h"

struct mid_ftp {
    SOCKET ctrl_s;
    SOCKET data_s;
};

/* Build one command to send to the FTP server */
static int build_cmd(char *buf, u_int bufsize, char *cmd, char *arg1)
{
    int cnt;

    if (arg1) {
        cnt = snprintf(buf, bufsize, "%s %s\r\n", cmd, arg1);
    } else {
        cnt = snprintf(buf, bufsize, "%s\r\n", cmd);
    }

    if (cnt < (int)bufsize)
        return 1;

    return 0;
}


/* Read one line from the server, being careful not to overrun the
     buffer. If we do reach the end of the buffer, discard the rest of
     the line. */
static int get_line(SOCKET s, char *buf, int buf_size)
{
    int eol = 0;
    int cnt = 0;
    int ret;
    char c;

    while (!eol) {
        ret = recv(s, &c, 1, 0);

        if (ret != 1)
            TR069ErrorOut("read\n");

        if (c == '\n') {
            eol = 1;
            continue;
        }

        if (cnt < buf_size) {
            buf[cnt++] = c;
        }
    }
    if (cnt < buf_size) {
        buf[cnt ++] = '\0';
    } else {
        buf[buf_size - 1] = '\0';
    }
    return 0;
Err:
    return -1;
} 

/* Read the reply from the server and return the MSB from the return
     code. This gives us a basic idea if the command failed/worked. The
     reply can be spread over multiple lines. When this happens the line
     will start with a - to indicate there is more*/
static int get_reply(SOCKET s, char * msgbuf)
{
    char buf[BUFSIZ];
    int more = 0;
    int ret;
    int first_line = 1;
    unsigned int code = 0;

    do {
        ret = get_line(s, buf, sizeof(buf));
        if (ret < 0)
            TR069ErrorOut("ret = %d\n", ret);

        TR069Printf("FTP: %s\n", buf);

        if (first_line) {
            code = strtoul(buf, NULL, 0);
            first_line = 0;
            more = (buf[3] == '-');
        } else {
            if (isdigit(buf[0]) && isdigit(buf[1]) && isdigit(buf[2]) &&
                    (code == strtoul(buf,NULL,0)) && buf[3]==' ')
                more=0;
            else
                more =1;
        }
    } while (more);

    strcpy(msgbuf, buf);

    return (buf[0] - '0');
Err:
    return -1;
}

/* Send a command to the server */
static int send_cmd(SOCKET s, const char *msgbuf)
{
    int len;
    int slen = (int)strlen(msgbuf);

    len = send(s, (char *)msgbuf, slen, MSG_NOSIGNAL);
    if (len != slen) {
        if (slen < 0) {
            TR069ErrorOut("write\n");
        } else {
            TR069ErrorOut("write truncated!\n");
        }
    }
    return 1;
Err:
    return -1;
}

/* Send a complete command to the server and receive the reply. Return the 
     MSB of the reply code. */
static int command(char * cmd, char * arg, SOCKET s, char *msgbuf, u_int msgbuflen)
{
    int err;

    if (!build_cmd(msgbuf, msgbuflen, cmd, arg))
        TR069ErrorOut("%s command to long\n", cmd);

    TR069Printf("FTP: Sending %s command\n", cmd);

    err = send_cmd(s, msgbuf);
    if (err < 0)
        TR069ErrorOut("err = %d\n", err);

    return (get_reply(s, msgbuf));
Err:
    return -1;
}

/* Open a socket and connect it to the server. Also print out the
     address of the server for debug purposes.*/

static SOCKET openctrlsock(char *hostname)
{ 
    SOCKET s = INVALID_SOCKET;
    struct sockaddr_in servaddr;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET)
        TR069ErrorOut("socket s = %d\n", s);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(21);
    servaddr.sin_addr.s_addr = inet_addr(hostname);

    if (tr069_connect(s, &servaddr) < 0)
        TR069ErrorOut("connect %s failed\n", hostname);

    return s;
Err:
    if (s != INVALID_SOCKET)
        closesocket(s);
    return INVALID_SOCKET;
}

/* Perform a login to the server. Pass the username and password and
     put the connection into binary mode. This assumes a passwd is
     always needed. Is this true? */

static int login(SOCKET s, 
            char *username, char *password, 
            char *msgbuf, u_int msgbuflen)
{
    
    int ret;

    ret = command("USER", username, s, msgbuf, msgbuflen);
    if (ret != 3)
        TR069ErrorOut("User not accepted: %s\n", username);

    ret = command("PASS",password,s,msgbuf,msgbuflen);
    if (ret < 0)
        TR069ErrorOut("ret = %d\n", ret);

    if (ret != 2)
        TR069ErrorOut("Login failed \n");

    TR069Printf("FTP: Login sucessfull\n");

    ret = command("TYPE", "I", s, msgbuf, msgbuflen);
    if (ret < 0)
        TR069ErrorOut("ret = %d\n", ret);

    if (ret != 2)
        TR069ErrorOut("TYPE failed!\n");

    return (ret);
Err:
    return -1;
}


/* Open a data socket. This is a client socket, i.e. its listening
waiting for the FTP server to connect to it. Once the socket has been
opened send the port command to the server so the server knows which
port we are listening on.*/
static SOCKET opendatasock(SOCKET ctrl_s, char *msgbuf, unsigned msgbuflen)
{
    struct sockaddr_in servaddr;
    char ip[32];
    char *p;
    int ret;
    SOCKET s;
    int i1,i2,i3,i4,i5,i6;
    u_short port;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET)
        TR069ErrorOut("socket\n");

    ret = command("PASV", "", ctrl_s, msgbuf, msgbuflen);
    if (ret < 0)
        TR069ErrorOut("ret = %d\n", ret);

    if (ret != 2)
        TR069ErrorOut("PORT failed!\n");

    p = strchr(msgbuf, '(');//Õë¶ÔServ-U
    if (!p)
        TR069ErrorOut("strchr (!\n");
    p ++;
    ret = sscanf(p, "%d,%d,%d,%d,%d,%d", &i1, &i2, &i3, &i4, &i5, &i6);
    if (ret != 6)
        TR069ErrorOut("sscanf %d!\n", ret);

    sprintf(ip, "%d.%d.%d.%d", i1, i2, i3, i4);
    port = (u_short)(i5*256 + i6);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(ip);

    if (tr069_connect(s, &servaddr) < 0)
        TR069ErrorOut("connect\n");

    return (s);
Err:
    if (s != INVALID_SOCKET)
        closesocket(s);
    return INVALID_SOCKET;
}

/* All done, say bye, bye */
static int quit(SOCKET s, char *msgbuf, unsigned msgbuflen)
{
    int ret;

    ret = command("QUIT",NULL,s,msgbuf,msgbuflen);
    if (ret < 0)
        TR069ErrorOut("ret = %d\n", ret);

    if (ret != 2)
        TR069ErrorOut("Quit failed!\n");

    TR069Printf("Connection closed\n");
    return (0);
Err:
    return -1;
}

static int ftp_init(struct mid_ftp *ftp)
{
    if (!ftp)
        TR069ErrorOut("ftp is NULL\n");

    ftp->ctrl_s = INVALID_SOCKET;
    ftp->data_s = INVALID_SOCKET;

    return 0;
Err:
    return -1;
}

static int ftp_open(struct mid_ftp *ftp, char *hostname, char *username, char *password)
{
    char msgbuf[256];
    int ret;

    if (!ftp)
        TR069ErrorOut("ftp is NULL\n");

    ftp->ctrl_s = openctrlsock(hostname);
    if (ftp->ctrl_s == INVALID_SOCKET)
        TR069ErrorOut("ftp->ctrl_s = %d\n", ftp->ctrl_s);

    /* Read the welcome message from the server */
    ret = get_reply(ftp->ctrl_s, msgbuf);
    if (ret != 2)
        TR069ErrorOut("get_reply ret = %d\n", ret);

    ret = login(ftp->ctrl_s,username,password,msgbuf,sizeof(msgbuf));
    if (ret < 0)
        TR069ErrorOut("login ret = %d\n", ret);

    return 0;

Err:
    if (ftp) {
        if (ftp->ctrl_s != INVALID_SOCKET)
            closesocket(ftp->ctrl_s);
        ftp->ctrl_s = INVALID_SOCKET;
    }
    return -1;
}

static int ftp_begin(struct mid_ftp *ftp, char *filename, int download)
{
    int ret;
    char msgbuf[256];

    if (!ftp)
        TR069ErrorOut("ftp is NULL\n");
    if (ftp->ctrl_s == INVALID_SOCKET)
        TR069ErrorOut("ftp->ctrl_s = %d\n", ftp->ctrl_s);
    if (ftp->data_s != INVALID_SOCKET)
        TR069ErrorOut("ftp->data_s = %d\n", ftp->data_s);

    ftp->data_s = opendatasock(ftp->ctrl_s, msgbuf, sizeof(msgbuf));
    if (ftp->data_s == INVALID_SOCKET)
        TR069ErrorOut("opendatasock = %d\n", ftp->data_s);

    if (download)
        ret = command("RETR", filename, ftp->ctrl_s, msgbuf, sizeof(msgbuf));
    else
        ret = command("STOR", filename, ftp->ctrl_s, msgbuf, sizeof(msgbuf));
    if (ret < 0)
        TR069ErrorOut("command = %d\n", ret);

    if (ret != 1)
        goto Err;

    return 0;
Err:
    if (ftp) {
        if (ftp->data_s != INVALID_SOCKET)
            closesocket(ftp->data_s);
        ftp->data_s = INVALID_SOCKET;
    }
    return -1;
}

static int ftp_end(struct mid_ftp *ftp)
{
    int ret;
    char msgbuf[256];

    if (!ftp)
        TR069ErrorOut("ftp is NULL\n");
    if (ftp->data_s == INVALID_SOCKET)
        TR069ErrorOut("ftp->data_s = %d\n", ftp->data_s);

    closesocket(ftp->data_s);
    ftp->data_s = INVALID_SOCKET;

    ret = get_reply(ftp->ctrl_s, msgbuf);
    if (ret != 2)
        TR069ErrorOut("Transfer failed! %d\n", ret);

    return 0;
Err:
    return -1;
}

static int ftp_close(struct mid_ftp *ftp)
{
    char msgbuf[256];

    if (!ftp)
        TR069ErrorOut("ftp is NULL\n");

    if (ftp->ctrl_s == INVALID_SOCKET)
        TR069ErrorOut("ftp->ctrl_s = %d\n", ftp->ctrl_s);

    if (ftp->data_s != INVALID_SOCKET) {
        TR069Error("ftp file not close!\n");
        ftp_end(ftp);
    }

    quit(ftp->ctrl_s,msgbuf,sizeof(msgbuf));

    closesocket(ftp->ctrl_s);
    ftp->ctrl_s = INVALID_SOCKET;

    return 0;
Err:
    return -1;
}

static int ftp_put_data(struct mid_ftp const *ftp, const char *buf, int size)
{
    int len, l;

    if (!ftp)
        TR069ErrorOut("ftp is NULL\n");
    if (ftp->data_s == INVALID_SOCKET)
        TR069ErrorOut("ftp->data_s = %d\n", ftp->data_s);

    len = 0;
    while (len < size) {
        l = send(ftp->data_s, (char *)(buf + len), size - len, MSG_NOSIGNAL);
        if (l <= 0)
            TR069ErrorOut("send %d/%d\n", size - len, size);

        len += l;
    }

    return 0;
Err:
    return -1;
}

static int ftp_get_data(struct mid_ftp const *ftp, char *buf, int size)
{
    int l, len;

    if (!ftp)
        TR069ErrorOut("ftp is NULL\n");
    if (ftp->data_s == INVALID_SOCKET)
        TR069ErrorOut("ftp->data_s = %d\n", ftp->data_s);
    len = 0;
    for (;;) {
        l = recv(ftp->data_s, buf + len, size - len, 0);
        if (l == 0)
            break;
        if (l < 0)
            TR069ErrorOut("recv %d\n", l);
        len += l;
        if (len >= size)
            TR069ErrorOut("FTP: File too big!\n");
    }

    return len;
Err:
    return -1;
}

static int ftp_url_check(char* url, char *host, u_int host_len, char *uri, u_int uri_len)
{
    char *p;
    u_int len;

    if (!url || !host || !uri)
        TR069ErrorOut("url = %p, host = %p, uri = %p\n", url, host, uri);
    if (strnicmp(url, "ftp://", 6))
        TR069ErrorOut("url error! %s\n", url);

    url += 6;
    p = strchr(url, '/');
    if (!p)
        TR069ErrorOut("uri is NULL\n");

    len = (u_int)(p - url);
    if (len >= host_len)
        TR069ErrorOut("host_len = %d / %d\n", host_len, len);
    memcpy(host, url, len);
    host[len] = 0;

    len = strlen(p);
    if (len >= uri_len)
        TR069ErrorOut("uri_len = %d / %d\n", uri_len, len);
    strcpy(uri, p);

    return 0;
Err:
    return -1;
}

int tr069_ftp_put(char *url, char *username, char *password, char *content, int length)
{
    char uri[200 + 4];
    char host[32 + 4];
    struct mid_ftp ftp;

    ftp_init(&ftp);

    if (!url || !username || !password || !content)
        TR069ErrorOut("url = %p, username = %p, password = %p, content = %p\n", url, username, password, content);
    if (ftp_url_check(url, host, 32, uri, 200))
        TR069ErrorOut("url = %s\n", url);

    if (ftp_open(&ftp, host, username, password))
        TR069ErrorOut("mid_ftp_open failed\n");

    if (ftp_begin(&ftp, uri, 0))
        TR069ErrorOut("mid_ftp_put_begin failed\n");
    if (ftp_put_data(&ftp, content, length))
        TR069ErrorOut("mid_ftp_put_data failed\n");
    ftp_end(&ftp);
    ftp_close(&ftp);

    return 0;
Err:
    if (ftp.ctrl_s != INVALID_SOCKET)
        ftp_close(&ftp);
    return -1;
}

int tr069_ftp_get(char *url, char *username, char *password, char *content, int length)
{
    int len;
    char uri[200 + 4];
    char host[32 + 4];
    struct mid_ftp ftp;

    ftp_init(&ftp);

    if (!url || !username || !password || !content)
        TR069ErrorOut("url = %p, username = %p, password = %p, content = %p\n", url, username, password, content);
    if (ftp_url_check(url, host, 32, uri, 200))
        TR069ErrorOut("url = %s\n", url);

    if (ftp_open(&ftp, host, username, password))
        TR069ErrorOut("mid_ftp_open failed\n");

    if (ftp_begin(&ftp, uri, 1))
        TR069ErrorOut("mid_ftp_put_begin failed\n");
    len = ftp_get_data(&ftp, content, length);
    if (len < 0)
        TR069ErrorOut("mid_ftp_put_data failed\n");
    ftp_end(&ftp);
    ftp_close(&ftp);

    return 0;
Err:
    if (ftp.ctrl_s != INVALID_SOCKET)
        ftp_close(&ftp);
    return -1;
}
