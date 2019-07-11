#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <errno.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <netdb.h>

#include "sys_msg.h"
#include "app/Message/MessageTypes.h"
#include "mid/mid_task.h"
#include "mid/mid_http.h"
#include "openssl/evp.h"

#include "ind_mem.h"
#include "configCustomer.h"
#include "config/pathConfig.h"
#include "app/app_heartbit.h"
#include "app/Assertions.h"

#define BUFFER_SIZE 1024
#define URL_SIZE    512
#define DOWNLOADSIZE (8*1024)

/*将临时保存的应用程序logo图片保存成正式的logo*/

#define APP_LOGO_PATH 			SYSTEM_LOGO_DIR"app_logo.gif"
#define APP_LOGO_SIZE_MAX  (128*1024)	/*应用程序logo的最大值*/

typedef struct {
    int fd;
    unsigned char buffer[BUFFER_SIZE], *buf_ptr, *buf_end;
    int line_count;
    int http_code;
    unsigned int contentlen;
    char location[URL_SIZE];
} HTTPContext;

static int ctc_http_open(HTTPContext *s, const char *uri, int flags);
static int ctc_http_connect(HTTPContext *s, const char *path, const char *uid, const char *pw);
static int ctc_http_write(HTTPContext *s, unsigned char *buf, int size);
static int ctc_http_read(HTTPContext *s, unsigned char *buf, int httplen, int size, unsigned int recv_len);
static int ctc_http_close(HTTPContext *s);
static void ctc_http_free(void);

static HTTPContext ctc_http_session;
static char *ctc_http_buf = NULL;

extern char* global_cookies;

static void init_http_session(void)
{
    ctc_http_session.fd = 0;
    memset(ctc_http_session.buffer, 0, BUFFER_SIZE);
    ctc_http_session.buf_ptr = NULL;
    ctc_http_session.buf_end = NULL;
    ctc_http_session.line_count = 0;
    ctc_http_session.http_code = 0;
    memset(ctc_http_session.location, 0, URL_SIZE);
}

static void upgrade_percent(int per)
{
    static int lastper = 0;

    if(per == lastper)
        return;
    lastper = per;
    sendMessageToKeyDispatcher(MessageType_Unknow, 0x9280 + per, 0, 0);
    mid_task_delay(100);
}

/*将临时保存的应用程序logo图片保存成正式的logo*/
/*
int app_logo_file2file()
{

    FILE *fp = NULL;
    FILE *src_fp = NULL;
    char *file_buf = NULL;
    struct stat stat_buf;
    unsigned int file_size;
    int ret;
    int len;

    if(stat(BOOTLOGOPATH, &stat_buf))
        return -1;
    file_size = stat_buf.st_size;

    if(file_size > APP_LOGO_SIZE_MAX)
        return -1;



    fp = fopen(APP_LOGO_PATH, "w+");

    if(NULL == fp)
        return -1;

    src_fp = fopen(BOOTLOGOPATH, "r");

    if(NULL == src_fp) {
        fclose(fp);
        return -1;
    }



    file_buf = IND_MALLOC(APP_LOGO_SIZE_MAX);
    if(NULL == file_buf) {
        fclose(fp);
        fclose(src_fp);

        return -1;
    }
    len = fread(file_buf, 1, file_size, src_fp);
    if(len != file_size) {
        fclose(fp);
        fclose(src_fp);

        IND_FREE(file_buf);

        return -1;
    }

    ret = fwrite(file_buf, 1, len, fp);

    if(ret == len) {
        fflush(fp);
        fclose(fp);
        fclose(src_fp);
        IND_FREE(file_buf);
        return 0;
    }
    fclose(fp);
    fclose(src_fp);
    IND_FREE(file_buf);

    return -1;

}
*/

//代表是否升级大版本，0 : app -1:logo
int file_http_get(char* uri, mid_http_f callback, int arg, char *buf_data, int buf_len, int app)
{
    int ret;
    unsigned int ctc_http_len = 0;
    FILE *downloadFile = NULL;

    init_http_session();
    ctc_http_free(); //It will not make a re_free error.
    PRINTF("http_get: %s\n", uri);

    if(0 == app || -1 == app)
        upgrade_percent(0);
    ret = ctc_http_open(&ctc_http_session, uri, 0);
    if(ret == -1) {
        if(NULL != callback)
            callback(ret, NULL, 0, arg);
        return ret;
    } else if(ret == -303) {
        if(NULL != callback)
            callback(ret, NULL, 0, arg);
        PRINTF("%s\n", ctc_http_session.location);
        return ret;
    } else if(ret == -403) {
        if(NULL != callback)
            callback(ret, NULL, 0, arg);
        PRINTF("FORBIDDEN");
        return ret;
    } else if(ret == -404) {
        if(NULL != callback)
            callback(ret, NULL, 0, arg);
        PRINTF("NOTFOUND");
        return ret;
    }

    ctc_http_buf = (char *)IND_MALLOC(DOWNLOADSIZE + 4);
    memset(ctc_http_buf, 0, DOWNLOADSIZE + 4);
    if(ctc_http_buf == NULL) {
        ctc_http_close(&ctc_http_session);
        fprintf(stderr, "can't alloc memory to such a big request!\n");
        if(NULL != callback)
            callback(HTTP_ERROR_MALLOC, NULL, 0, arg);
        return HTTP_ERROR_MALLOC;
    }


    fprintf(stderr, "ctc_http_session.contentlen = %d, ctc_http_buf =%p\n", ctc_http_session.contentlen, ctc_http_buf);

    if(-2 == app) {
        //downloadFile = fopen(BOOTLOGOPATH, "wb");
    } else if(-3 == app) {
        //downloadFile = fopen(AUTHLOGOPATH, "wb");
    } else if(1 == app) {
        //downloadFile = fopen(UPDVBWEBSPATH, "wb");
#ifdef PAY_SHELL
    } else if( 2 == app ) {
        char *psname = NULL;
        char pspath[128];
        psname = strrchr(uri, '/');
        memset(pspath, 0, sizeof(pspath));

        if (psname == NULL) {
            PRINTF("uri is error, uri is %s\n", uri);
            strcpy(pspath, DEFAULT_MODULE_PAY_DATAPATH"/libbank.so");
        } else {
            sprintf(pspath, "%s%s", DEFAULT_MODULE_PAY_DATAPATH, psname);
        }

        PRINTF("pspath is %s\n", psname);
        downloadFile = fopen(pspath, "wb");
#endif
    } else {
        //downloadFile = fopen(UPFILEPATH, "wb");
    }

    if(downloadFile == NULL) {
        fprintf(stderr, "open download file error\n");
    }

    while(1) {
        int rlen;

        //rlen = ctc_http_read(&ctc_http_session, ctc_http_buf, DOWNLOADSIZE);
        rlen = ctc_http_read(&ctc_http_session, (unsigned char *)ctc_http_buf, ctc_http_session.contentlen, DOWNLOADSIZE, ctc_http_len);
        if(rlen > 0) {
            if(downloadFile != NULL)
                fwrite(ctc_http_buf, 1, rlen, downloadFile);
            ctc_http_len += rlen;
            if(0 == app)
                upgrade_percent((ctc_http_len / 1000) / (ctc_http_session.contentlen / 100000));
            else if(-1 == app)
                upgrade_percent((ctc_http_len / 10) / (ctc_http_session.contentlen / 1000));
            //PRINTF("***********%d,%d,%d\n", rlen,ctc_http_len,ctc_http_session.contentlen);
            if(ctc_http_len == ctc_http_session.contentlen) {
                if(0 == app || -1 == app)
                    upgrade_percent(100);
                ctc_http_close(&ctc_http_session);
                if(NULL != downloadFile)
                    fclose(downloadFile);

                IND_FREE(ctc_http_buf);
                ctc_http_buf = NULL;

                if(NULL != callback)
                    callback(HTTP_OK_READDATA, NULL, ctc_http_len, arg);
                return HTTP_OK_READDATA;
            }
            //PRINTF( "Read Data,size=%d, total_size=%d,percent=%d\n", ctc_http_len, ctc_http_session.contentlen,(ctc_http_len*100/ctc_http_session.contentlen) );
        } else if(rlen == 0 && ctc_http_len == ctc_http_session.contentlen) {
            if(0 == app || -1 == app)
                upgrade_percent(100);
            ctc_http_close(&ctc_http_session);
            if(NULL != downloadFile)
                fclose(downloadFile);

            IND_FREE(ctc_http_buf);
            ctc_http_buf = NULL;

            if(NULL != callback)
                callback(HTTP_OK_READDATA, NULL, ctc_http_len, arg);
            return HTTP_OK_READDATA;
        } else {
            PRINTF("rlen = %d\n", rlen);
            ctc_http_close(&ctc_http_session);
            IND_FREE(ctc_http_buf);
            ctc_http_buf = NULL;
            if(NULL != callback)
                callback(HTTP_ERROR_SOCKET, NULL, 0, arg);
            return HTTP_ERROR_SOCKET;
        }
    }

    ctc_http_close(&ctc_http_session);
    IND_FREE(ctc_http_buf);
    ctc_http_buf = NULL;
    if(NULL != callback)
        callback(HTTP_ERROR_TIMEOUT, NULL, 0, arg);
    return HTTP_ERROR_TIMEOUT;
}


static void ctc_http_free(void)
{
    if(ctc_http_buf) {
        IND_FREE(ctc_http_buf);
        ctc_http_buf = NULL;
    }
    return;
}

static int connectWithTimeout(int fd, struct sockaddr* daddr, int socklen, struct timeval* tv)
{
    fd_set fdset;
    int ret = 1;
    socklen_t tmp = 0;

    if(fcntl(fd, F_GETFL, &tmp) < 0) {
        PRINTF("Get socket opt failed!\n");
        return -1;
    }
    if(fcntl(fd, F_SETFL, tmp | O_NONBLOCK)) {
        PRINTF("set socket O_NONBLOCK Failed!\n");
        goto connectErr;
    }
    if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&ret, sizeof(ret)) < 0) {
        PRINTF("setsockopt failed!");
        goto connectErr;
    }

    ret = -1;
    ret = connect(fd, daddr, sizeof(struct sockaddr));

    switch(errno) {
    case 0:
        return 0;
    case ENOTSOCK:
    case EISCONN:
    case EAFNOSUPPORT:
    case ECONNREFUSED:
    case EADDRINUSE:
    case EACCES:
    case ENETUNREACH:
        goto connectErr;
    }
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);

    ret = select(fd + 1, NULL, &fdset, NULL, tv);
    if(ret > 0  && FD_ISSET(fd, &fdset)) {
        ret = -1;
        tmp = sizeof(ret);
        if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &ret, &tmp) == 0) {
            if(ret == 0) {
                fcntl(fd, F_SETFL, tmp);	//restore the options
                //TDY_PERROR( "getsockopt " );
                return ret;
            } else {
                PRINTF("get socket addr=%d\n", ret);
            }
        }
    }

    PRINTF("connectWithTimeout failed! ");

connectErr:
    perror("");
    if(fcntl(fd, F_SETFL, tmp) == -1)
        perror("fcntl err\n");

    return -1;
}

//HELPER
static int ctc_http_open(HTTPContext *s, const char *uri, int flags)
{
    struct sockaddr_in dest_addr;
    const char *p, *path, *proxy_path;
    char hostname[1024], *q, *r;
    char uid[64], pw[64];
    int port, fd = -1, use_proxy;

    proxy_path = NULL;//getenv("http_proxy");
    use_proxy = 0;//(proxy_path != NULL) && !getenv("no_proxy");

    /* fill the dest addr */
    //redo:
    if(use_proxy) {
        p = proxy_path;
    } else {
        if(strncmp(uri, "http://", 7)) {
            PRINTF("NOT HTTP, uri=%s\n", uri);
            goto fail;
        }
        p = uri + 7;
    }

    q = (char *)strchr(p, '@');  //WZW modified to fix pc-lint Error 158
    if(q) {
        r = uid;
        while(p < q && *p != ':')
            *r++ = *p++;
        *r = 0;
        p++;
        r = pw;
        while(p < q)
            *r++ = *p++;
        *r = 0;
        p++;
    } else uid[0] = 0, pw[0] = 0;

    q = hostname;
    while(*p != ':' && *p != '/' && *p != '\0') {
        if((q - hostname) < sizeof(hostname) - 1)
            *q++ = *p;
        p++;
    }
    *q = '\0';
    port = 80;
    if(*p == ':') {
        p++;
        port = strtoul(p, (char **)&p, 10);
    }
    if(port <= 0)
        goto fail;
    if(use_proxy) {
        path = uri;
    } else {
        if(*p == '\0')
            path = "/";
        else
            path = p;
    }
    memset(&dest_addr, 0, sizeof(dest_addr));  /*add by whz */
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
#if 0
    if((dest_addr.sin_addr.s_addr = inet_addr(hostname)) == -1) {
        dest_addr.sin_addr.s_addr = hostGetByName(hostname);
    }
#else
    //dest_addr.sin_addr.s_addr = inet_addr(hostname);
    struct hostent *host;
    if((host = gethostbyname(hostname)) == NULL) {
        PRINTF("Get host by name failed!\n");
        goto fail;
    }
    dest_addr.sin_addr   = *((struct in_addr *)host->h_addr);
    bzero(&(dest_addr.sin_zero), 8);
#endif

    fd = socket(PF_INET, SOCK_STREAM, 0);
    if(fd < 0)
        goto fail;


    {
        struct linger l = {1, 0};
        if(setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
            PRINTF("set Socket SO_LINGER flag Failed!\n");
            close(fd);
            fd = -2;
            goto fail;
        }
    }
#ifdef NO_SELECT_CONNECT
    if(connect(fd, (struct sockaddr *)&dest_addr,
               sizeof(dest_addr)) < 0) {
        PRINTF("connect err\n");
        goto fail;
    }
#else
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if(connectWithTimeout(fd, (struct sockaddr*) &dest_addr, sizeof(dest_addr), &tv) < 0) {
        PRINTF("connect ERROR\n");
        goto fail;
    }
#endif
    s->fd = fd;
    if(ctc_http_connect(s, path, uid, pw) < 0)
        goto fail;
    if(((s->http_code == 300) || (s->http_code == 302) || (s->http_code == 303)) && s->location[0] != '\0') {
        /* url moved, get next */
        uri = s->location;
        //goto redo;
        /*In initial code, it will try to connect to the re_location address.
          But now, we must return*/
        if(fd >= 0)
            close(fd);
        return -303;//the other 3xx code mean re_location too.
    } else if(s->http_code == 403 || s->http_code == 404) {
        if(fd >= 0)
            close(fd);
        return 0 - (s->http_code);
    }//should NOT do "else" switch

    return 0;
fail:
    if(fd >= 0)
        close(fd);
    return -1;
}

static int ctc_http_getc(HTTPContext *s)
{
    int len;
    struct timeval tv = { 3, 50000 };
    fd_set recvdc;
    FD_ZERO(&recvdc);
    FD_SET(s->fd, &recvdc);

    if(s->buf_ptr >= s->buf_end) {
        //redo:
        if(select((s->fd) + 1, &recvdc, NULL, NULL, &tv) < 0) {
            PRINTF("select error when receive header\n");
            FD_CLR(s->fd, &recvdc);
            return -1;
        }
        if(FD_ISSET(s->fd, &recvdc) > 0) {
            len = recv(s->fd, s->buffer, BUFFER_SIZE, 0);
            if(len < 0) {
                /*if (errno == EAGAIN || errno == EINTR)
                  goto redo;*/
                FD_CLR(s->fd, &recvdc);
                return -1;
            } else if(len == 0) {
                FD_CLR(s->fd, &recvdc);
                return -1;
            } else {
                s->buf_ptr = s->buffer;
                s->buf_end = s->buffer + len;
            }
        } else { //time out
            PRINTF("receive header TIME OUT\n");
            FD_CLR(s->fd, &recvdc);
            return -1;
        }
    }
    return *s->buf_ptr++;
}

static int ctc_process_line(HTTPContext *s, char *line, int line_count)
{
    char *tag, *p;

    /* end of header */
    if(line[0] == '\0')
        return 0;

    p = line;
    if(line_count == 0) {
        while(!isspace(*p) && *p != '\0')
            p++;
        while(isspace(*p))
            p++;
        s->http_code = strtol(p, NULL, 10);
    } else {
        while(*p != '\0' && *p != ':')
            p++;
        if(*p != ':')
            return 1;

        *p = '\0';
        tag = line;
        p++;
        while(isspace(*p))
            p++;
        if(!strncmp(tag, "Location", 8)) {
            strncpy(s->location, p, sizeof(s->location));
        }
        if(!strncmp(tag, "Content-Length", 14)) {
            s->contentlen = strtol(p, NULL, 10);
        }
        if(!strncmp(tag, "Transfer-Encoding", 17)) {
            char temp[32];
            memset(temp, '\0', 32);
            strncpy(temp, p, 31);
            PRINTF("unsupport encodeing:%s\n", temp);
            return -1;
        }
    }
    return 1;
}

static int ctc_http_connect(HTTPContext *s, const char *path, const char *uid, const char *pw)
{
    int post, err, ch;
    char line[1024], *q;
    char user_pw[128];
    char bufcoded[64 * 3];

    if(uid[0]) {
        snprintf(user_pw, 128, "%s:%s", uid, pw);
        strncpy(bufcoded, "Authorization: Basic ", 64 * 3);
        EVP_EncodeBlock(bufcoded + strlen(bufcoded), user_pw, strlen(user_pw));
        strncat(bufcoded, "\r\n", 2);
    } else
        bufcoded[0] = 0;


    /* send http header */
    post = 0;//h->flags & URL_WRONLY;

    //	snprintf(s->buffer, sizeof(s->buffer),
    snprintf((char*)s->buffer, BUFFER_SIZE, "%s %s HTTP/1.0\r\n%sUser-Agent: HTUser %s\r\nAccept: */*\r\n\r\n",
             post ? "POST" : "GET",
             path,
             bufcoded,
             "1.0");

    if(ctc_http_write(s, s->buffer, strlen((char*)s->buffer)) < 0)
        return -1;

    /* init input buffer */
    s->buf_ptr = s->buffer;
    s->buf_end = s->buffer;
    s->line_count = 0;
    s->location[0] = '\0';
    if(post)
        return 0;

    /* wait for header */
    q = line;
    for(;;) {
        ch = ctc_http_getc(s);
        if(ch < 0)
            return -1;
        if(ch == '\n') {
            /* process line */
            if(q > line && q[-1] == '\r')
                q--;
            *q = '\0';
#ifdef DEBUG
            PRINTF("header='%s'\n", line);
#endif
            err = ctc_process_line(s, line, s->line_count);
            if(err < 0)
                return err;
            if(err == 0)
                return 0;
            s->line_count++;
            q = line;
        } else {
            if((q - line) < sizeof(line) - 1)
                *q++ = ch;
        }
    }
}

static int ctc_http_read(HTTPContext *s, unsigned char *buf, int httplen, int size, unsigned int recv_len)
{
    int size1, len;
    //struct timeval tv = { 3, 50000 };
    struct timeval tv = { 30, 50000 };
    //struct timeval tv = { 1, 0 };
    fd_set recvdc;

    size1 = size;
    while(size > 0) {
        /* read bytes from input buffer first */
        len = s->buf_end - s->buf_ptr;
        if(len > 0) {
            if(len > size)
                len = size;
            memcpy(buf, s->buf_ptr, len);
            s->buf_ptr += len;
        } else {
            FD_ZERO(&recvdc);
            FD_SET(s->fd, &recvdc);
            tv.tv_sec = 30;
#if defined(hi3716m)
            tv.tv_sec = 15;
#endif
            if(select((s->fd) + 1, &recvdc, NULL, NULL, &tv) < 0) {
                ERR_PRN("when receive select error\n");
                FD_CLR(s->fd, &recvdc);
                return -1;
            }
            if(FD_ISSET(s->fd, &recvdc)) {
                len = recv(s->fd, buf, size, 0);
                if(len + recv_len == httplen)
                    return len;
                if(len < 0) {
                    ERR_PRN("receive from upgrade server error!\n");
                    FD_CLR(s->fd, &recvdc);
                    return -1;
                    /*if (errno != EINTR && errno != EAGAIN)
                      return -1;
                      else
                      continue;*/
                } else if(len == 0) {
                    break;
                }
                size -= len;
                break;
            } else { //time out
                ERR_PRN("receive TIME OUT\n");
                FD_CLR(s->fd, &recvdc);
                return -1;
            }
        }
        size -= len;
        buf += len;
    }
    return size1 - size;
}

/* used only when posting data */
static int ctc_http_write(HTTPContext *s, unsigned char *buf, int size)
{
    int ret, size1;

    size1 = size;
    while(size > 0) {
        ret = send(s->fd, buf, size, MSG_NOSIGNAL);
        if(ret < 0 /*&& errno != EINTR && errno != EAGAIN*/)
            return -1;
        size -= ret;
        buf += ret;
    }
    return size1 - size;
}

static int ctc_http_close(HTTPContext *s)
{
    close(s->fd);
    return 0;
}


static char six2pr[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

/*--- function ctc_base64_encode from HTUU_encode -----------------------------------------------
 *
 *   Encode a single line of binary data to a standard format that
 *   uses only printing ASCII characters (but takes up 33% more bytes).
 *
 *    Entry    bufin    points to a buffer of bytes.  If nbytes is not
 *                      a multiple of three, then the byte just beyond
 *                      the last byte in the buffer must be 0.
 *             nbytes   is the number of bytes in that buffer.
 *                      This cannot be more than 48.
 *             bufcoded points to an output buffer.  Be sure that this
 *                      can hold at least 5 + 4 * (nbytes/3) characters.
 *
 *    Exit     bufcoded contains the coded line.  The first 4*nbytes/3 bytes
 *                      contain printing ASCII characters representing
 *                      those binary bytes. This may include one or
 *                      two '=' characters used as padding at the end.
 *                      The last byte is a zero byte.
 *             Returns the number of ASCII characters in "bufcoded".
 */
 #if 0
int ctc_base64_encode(unsigned char * bufin, unsigned int nbytes, char * bufcoded)
{
    /* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) six2pr[c]

    register char *outptr = bufcoded;
    unsigned int i;

    for(i = 0; i < nbytes; i += 3) {
        *(outptr++) = ENC(*bufin >> 2);            /* c1 */
        *(outptr++) = ENC(((*bufin << 4) & 060) | ((bufin[1] >> 4) & 017)); /*c2*/
        *(outptr++) = ENC(((bufin[1] << 2) & 074) | ((bufin[2] >> 6) & 03));/*c3*/
        *(outptr++) = ENC(bufin[2] & 077);         /* c4 */

        bufin += 3;
    }

    /* If nbytes was not a multiple of 3, then we have encoded too
     * many characters.  Adjust appropriately.
     */
    if(i == nbytes + 1) {
        /* There were only 2 bytes in that last group */
        outptr[-1] = '=';
    } else if(i == nbytes + 2) {
        /* There was only 1 byte in that last group */
        outptr[-1] = '=';
        outptr[-2] = '=';
    }
    *outptr = '\0';
    return(outptr - bufcoded);
}

#endif

#define CHANNEL_LOGO_DOWNLOADSIZE (512*1024)
static HTTPContext ChanLogo_http_session;
static char *ChanLogo_http_buf = NULL;

static void init_ChanLogo_http_session(void)
{
    ChanLogo_http_session.fd = 0;
    memset(ChanLogo_http_session.buffer, 0, BUFFER_SIZE);
    ChanLogo_http_session.buf_ptr = NULL;
    ChanLogo_http_session.buf_end = NULL;
    ChanLogo_http_session.line_count = 0;
    ChanLogo_http_session.http_code = 0;
    memset(ChanLogo_http_session.location, 0, URL_SIZE);
}

static void ChanLogo_http_free(void)
{
    if(ChanLogo_http_buf) {
        IND_FREE(ChanLogo_http_buf);
        ChanLogo_http_buf = NULL;
    }
    return;
}

char* ChanLogo_http_get(char const* url, int* length)
{
    int ret;
    unsigned int ctc_http_len = 0;

    if(url == NULL || length == NULL) {
        PRINTF("url or length is NULL\n");
        return NULL;
    }

    init_ChanLogo_http_session();
    ChanLogo_http_free(); //It will not make a re_free error.
    PRINTF("http_get: %s\n", url);

    ret = ctc_http_open(&ChanLogo_http_session, url, 0);

    PRINTF("ret is %d\n", ret);
    if(ret == -1) {
        return NULL;
    } else if(ret == -303) {
        return NULL;
    } else if(ret == -403) {
        return NULL;
    } else if(ret == -404) {
        return NULL;
    }

    ChanLogo_http_buf = (char *)IND_MALLOC(CHANNEL_LOGO_DOWNLOADSIZE + 4);
    if(ChanLogo_http_buf == NULL) {
        ctc_http_close(&ChanLogo_http_session);
        fprintf(stderr, "can't alloc memory to such a big request!\n");

        return NULL;
    }
    memset(ChanLogo_http_buf, 0, CHANNEL_LOGO_DOWNLOADSIZE + 4);

    fprintf(stderr, "ctc_http_session.contentlen = %d, ctc_http_buf =%p\n", ChanLogo_http_session.contentlen, ChanLogo_http_buf);
    *length = ChanLogo_http_session.contentlen;

    while(1) {
        int rlen;

        rlen = ctc_http_read(&ChanLogo_http_session, (unsigned char *)ChanLogo_http_buf + ctc_http_len, ChanLogo_http_session.contentlen, DOWNLOADSIZE, ctc_http_len);
        if(rlen > 0) {

            ctc_http_len += rlen;

            if(ctc_http_len == ChanLogo_http_session.contentlen) {

                ctc_http_close(&ChanLogo_http_session);

                return ChanLogo_http_buf;
            }
            //PRINTF( "Read Data,size=%d, total_size=%d,percent=%d\n", ctc_http_len, ctc_http_session.contentlen,(ctc_http_len*100/ctc_http_session.contentlen) );
        } else if(rlen == 0 && ctc_http_len == ChanLogo_http_session.contentlen) {

            ctc_http_close(&ChanLogo_http_session);

            return ChanLogo_http_buf;
        } else {
            PRINTF("rlen = %d\n", rlen);
            ctc_http_close(&ChanLogo_http_session);
            IND_FREE(ChanLogo_http_buf);
            return NULL;
        }
    }

    ctc_http_close(&ChanLogo_http_session);
    IND_FREE(ChanLogo_http_buf);
    ChanLogo_http_buf = NULL;

    return NULL;
}

int ctc_http_send_GETmessage(const char *url, const char *User_Agent)
{
    HTTPContext http_session;
    struct sockaddr_in dest_addr;
    struct hostent *host;
    const char *p;
    char hostname[1024], *q, *r;
    int port, fd = -1;
    char addr[64] = {0};
    char *paddr = NULL;
    int len;

    http_session.fd = 0;
    memset(http_session.buffer, 0, BUFFER_SIZE);
    http_session.buf_ptr = NULL;
    http_session.buf_end = NULL;
    http_session.line_count = 0;
    http_session.http_code = 0;
    memset(http_session.location, 0, URL_SIZE);

    PRINTF("url is %s\n", url);

    if(strncmp(url, "http://", 7)) {
        PRINTF("NOT HTTP, uri=%s\n", url);
        goto fail;
    }
    p = url + 7;


    q = hostname;
    while(*p != ':' && *p != '/' && *p != '\0') {
        if((q - hostname) < sizeof(hostname) - 1)
            *q++ = *p;
        p++;
    }
    *q = '\0';
    port = 80;
    if(*p == ':') {
        p++;
        port = strtoul(p, (char **)&p, 10);
    }

    if(port <= 0) {
        PRINTF("port error is %d\n", port);
        goto fail;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));  /*add by whz */
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    unsigned int ipaddr = inet_addr(hostname);
    if (ipaddr == INADDR_ANY || ipaddr == INADDR_NONE) {
        if((host = gethostbyname(hostname)) == NULL) {
            PRINTF("Get host by name failed!\n");
            goto fail;
        }

        dest_addr.sin_addr = *((struct in_addr *)host->h_addr);
    } else {
        dest_addr.sin_addr.s_addr = ipaddr;
    }

    bzero(&(dest_addr.sin_zero), 8);

    fd = socket(PF_INET, SOCK_STREAM, 0);
    if(fd < 0) {
        PRINTF("socket fd error!\n");
        goto fail;
    }

    struct linger l = {1, 0};
    if(setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
        PRINTF("set Socket SO_LINGER flag Failed!\n");
        close(fd);
        fd = -2;
        goto fail;
    }

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if(connectWithTimeout(fd, (struct sockaddr*) &dest_addr, sizeof(dest_addr), &tv) < 0) {
        PRINTF("connect ERROR\n");
        goto fail;
    }

    http_session.fd = fd;

    paddr = (char *)(&dest_addr.sin_addr.s_addr);
	sprintf(addr, "%d.%d.%d.%d", paddr[0], paddr[1], paddr[2], paddr[3]);

    len = snprintf((char *)http_session.buffer, BUFFER_SIZE, "GET %s HTTP/1.1\r\nHost: %s:%d\r\n", url, addr, port);
    if (len <= 0) {
        PRINTF("Error: snprintf!\n");
        goto fail;
    }

    if (User_Agent) {
        len += snprintf((char *)http_session.buffer + len, BUFFER_SIZE - len, "User-Agent: %s\r\n", User_Agent);
        if (len <= 0) {
            PRINTF("Error: snprintf User_Agent!\n");
            goto fail;
        }
    }

    if(global_cookies) {
		len += snprintf((char *)http_session.buffer + len, BUFFER_SIZE - len, "%s\r\n", global_cookies);
        if (len <= 0) {
            PRINTF("Error: snprintf Cookie!\n");
            goto fail;
        }
	}

    len += snprintf((char *)http_session.buffer + len, BUFFER_SIZE - len, "\r\n");

    PRINTF("SEND:\n%s\n", http_session.buffer);

    if(ctc_http_write(&http_session, http_session.buffer, len) < 0) {
        PRINTF("ctc_http_write ERROR\n");
        goto fail;
    }

    ctc_http_close(&http_session);

    return 0;

fail:
    if(fd >= 0)
        close(fd);
    return -1;

}



