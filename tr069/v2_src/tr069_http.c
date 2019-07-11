/*******************************************************************************
    公司：
            Yuxing software
    纪录：
            2008-1-26 21:12:26 create by Liu Jianhua
    模块：
            tr069
    简述：
            HTTP 协议
 *******************************************************************************/

#include "tr069_global.h"
#include "tr069_header.h"
#include "tr069_md5.h"

#define TR069_DEBUG_TRACE

int g_tr069_httpTimeout = 5;
int g_tr069_holdCookie = 0;

//------------------------------------------------------------------------------
static void http_md5(char *inbuf, uint32_t inlen, char *outbuf)
{
    MD5_CTX ctx;
    u_char md5[16];

    tr069_MD5Init(&ctx);
    tr069_MD5Update(&ctx, (u_char *)inbuf, (uint32_t)inlen);
    tr069_MD5Final(md5, &ctx);

    sprintf(outbuf, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
                md5[0], md5[1], md5[2], md5[3], md5[4], md5[5], md5[6], md5[7], 
                md5[8], md5[9],md5[10],md5[11],md5[12],md5[13],md5[14],md5[15]);
    outbuf[32] = 0;
}

static int http_base(struct Http * http, const char *header)
{
    int len;
    char buf[TR069_NAME_FULL_SIZE_128 + 4];
    struct Digest *digest = &(http->digest);

    len = snprintf(buf, TR069_NAME_FULL_SIZE_128, "%s:%s", digest->username, digest->password);
    if (len < 0)
        TR069ErrorOut("snprintf\n");
    base64_encode((u_char *)buf, (uint32_t)len, (u_char *)(digest->response), DIGEST_RESPONSE_LEN);

    http->authen = AUTHENTICATION_BASIC;

    return 0;
Err:
    return -1;
}

static int http_digest_parse(struct Digest *digest, const char *header)
{
    uint32_t l;
    char name[DIGEST_NAME_LEN];
    char value[DIGEST_PARAM_LEN];

    if (header[0] == '\0')
        TR069ErrorOut("header is empty\n");
    while (header[0] != '\0') {
        int quote;

        quote = 0;
        //分析变量名称
        for (l = 0; l < DIGEST_PARAM_LEN && isalnum(header[l]); l ++) ;
        if (l >= DIGEST_NAME_LEN)
            TR069ErrorOut("name len = %d / %d\n", l, DIGEST_NAME_LEN);
        memcpy(name, header, l);
        name[l] = 0;
        header += l;
        if (header[0] != '=')
            TR069ErrorOut("op = 0x%02x\n", header[0]);
        header ++;
        if (header[0] == '"') {
            quote = 1;
            header ++;
        }

        //分析变量值
        if (quote)
            for (l = 0; l < DIGEST_PARAM_LEN && header[l] != '"'; l ++) ;
        else
            for (l = 0; l < DIGEST_PARAM_LEN && isalnum(header[l]); l ++) ;
        if (l >= DIGEST_PARAM_LEN)
            TR069ErrorOut("value len = %d / %d\n", l, DIGEST_PARAM_LEN);
        memcpy(value, header, l);
        value[l] = 0;
        header += l;
        if (quote)
            header ++;

        l = strlen(name);
        if (l == 2 && strcmp(name, "nc") == 0)
            sscanf(value, "%08d", &(digest->nc));
        else if (l == 3 && strcmp(name, "qop") == 0)
             strcpy(digest->qop, value);
        else if (l == 3 && strcmp(name, "uri") == 0)
             strcpy(digest->uri, value);
        else if (l == 5 && strcmp(name, "realm") == 0)
             strcpy(digest->realm, value);
        else if (l == 5 && strcmp(name, "nonce") == 0)
             strcpy(digest->nonce, value);
        else if (l == 6 && strcmp(name, "domain") == 0)
             strcpy(digest->uri, value);
        else if (l == 6 && strcmp(name, "opaque") == 0)
             strcpy(digest->opaque, value);
        else if (l == 6 && strcmp(name, "cnonce") == 0)
            strcpy(digest->cnonce, value);
        else if (l == 8 && strcmp(name, "response") == 0)
             strcpy(digest->response, value);
        else if (l == 8 && strcmp(name, "username") == 0)
             strcpy(digest->username, value);
        else if (l == 9 && memcmp(name, "algorithm", 9) == 0)
             strcpy(digest->algorithm, value);
        else
            TR069ErrorOut("%s\n", name);

        if (header[0] != ',')
            break;
        header ++;

        //除去空格符
        while (header[0] == ' ' || header[0] == '\t' || header[0] == '\r' || header[0] == '\n')
            header ++;
    }
    return 0;
Err:
    return -1;
}

static int http_digest_author(struct Digest *digest, char *response)
{
    int len, l;
    char buf[1024];

    len = sprintf(buf, "%s:%s:%s", digest->username, digest->realm, digest->password);
    if (strcmp(digest->algorithm, "md5-sess") == 0) {
        http_md5(buf, (uint32_t)len, buf);
        len = 32;
        len += sprintf(buf + len, ":%s:%s", digest->nonce, digest->cnonce); 
    }

    http_md5(buf, (uint32_t)len, buf);
    len = 32;
    if (digest->qop[4] == ',' && digest->qop[4] == ',')
        digest->qop[4] = 0;

    len += sprintf(buf + len, ":%s:", digest->nonce);
    if (digest->qop[0] == 'a')
        len += sprintf(buf + len, "%08x:%s:%s:", digest->nc, digest->cnonce, digest->qop);

    l = sprintf(buf + len, "%s:%s", digest->method, digest->uri);
    if (strcmp(digest->qop, "auth-int") == 0)
        l += sprintf(buf + len + l, ":%s", digest->md5body);
    http_md5(buf + len, (uint32_t)l, buf + len);
    len += 32;

    http_md5(buf, (uint32_t)len, buf);
    strcpy(response, buf);

    return 0;
}

int tr069_http_digest(struct Http *http, const char *header)
{
    char buf[16];
    struct Digest *digest = &(http->digest);

    http_digest_parse(digest, header);
    if (digest->realm[0] == 0 || digest->nonce[0] == 0)
        TR069ErrorOut("realm = %s, nonce = %s\n", digest->realm, digest->nonce);

    sprintf(buf, "%08x", tr069_sec( ));
    http_md5(buf, 8, digest->cnonce);

    digest->nc ++;
    http_digest_author(digest, digest->response);

    http->authen = AUTHENTICATION_DIGEST;

    return 0;
Err:
    return -1;
}

static int http_author(struct Http *http, const char *header)
{
    struct Digest *digest = &(http->digest);
    struct Digest client;
    char response[DIGEST_RESPONSE_LEN];

    memset(&client, 0, sizeof(struct Digest));
    http_digest_parse(&client, header);

    if (client.nc == 0 || client.cnonce[0] == 0 || client.response[0] == 0)
        TR069ErrorOut("nc = %u, cnonce = %s, response = %s\n", client.nc, client.cnonce, client.response);
    if (strcmp(client.qop, "auth"))
        TR069ErrorOut("qop = %s\n", client.qop);
    if (strcmp(client.uri, digest->uri))
        TR069ErrorOut("uri = %s / %s\n", client.uri, digest->uri);
    if (strcmp(client.realm, digest->realm))
        TR069ErrorOut("realm = %s / %s\n", client.realm, digest->realm);
    if (strcmp(client.nonce, digest->nonce))
        TR069ErrorOut("nonce = %s / %s\n", client.nonce, digest->nonce);
    if (strcmp(client.opaque, digest->opaque))
        TR069ErrorOut("opaque = %s / %s\n", client.opaque, digest->opaque);
    if (strcmp(client.username, digest->username))
        TR069ErrorOut("username = %s / %s\n", client.username, digest->username);

    strcpy(client.method, digest->method);
    strcpy(client.password, digest->password);
    http_digest_author(&client, response);

    if (strcmp(client.response, response))
        TR069ErrorOut("response = %s / %s\n", client.response, response);

    http->authen = AUTHENTICATION_DIGEST;

    return 0;
Err:
    http->authen = AUTHENTICATION_NONE;
    return -1;
}

//TR069SessionID=58E558A869F2D9A0931204672012231418;Version=1;Discard

static int http_cookie(struct Http *http, const char *buf)
{
    int i;
    uint32_t l;
    struct Cookie *cookie;

    while (isprint(buf[0]))
    {
        while (isspace(buf[0]))
            buf ++;

        if (http->cookie_num >= COOKIE_ITEM_NUM)
            TR069ErrorOut("cookie_num = %d\n", http->cookie_num);
        cookie = &(http->cookie_array[http->cookie_num]);

        //parse name
        for (l = 0; l < COOKIE_NAME_LEN && isalnum(buf[l]); l ++) ;
        if (l == 0 || l >= COOKIE_NAME_LEN)
            TR069ErrorOut("l = %d\n", l);
        memcpy(cookie->name, buf, l);
        cookie->name[l] = 0;
        buf += l;

        if (buf[0] == '=') {
            buf ++;

            //parse value
            for (l = 0; l < COOKIE_VALUE_LEN && isprint(buf[l]) && buf[l] != ';'; l ++) ;
            if (l == 0 || l >= COOKIE_VALUE_LEN)
                TR069ErrorOut("name = %s, l = %d\n", cookie->name, l);
            memcpy(cookie->value, buf, l);
            cookie->value[l] = 0;
            buf += l;
        } else {
            cookie->value[0] = 0;
        }

        if (buf[0] == ';')
            buf ++;

        if (stricmp(cookie->name, "discard") == 0 || 
            stricmp(cookie->name, "version") == 0 || 
            stricmp(cookie->name, "domain") == 0 || 
            stricmp(cookie->name, "expires") == 0 || 
            stricmp(cookie->name, "secure") == 0 || 
            stricmp(cookie->name, "path") == 0)
            continue;

        for (i = 0; i < http->cookie_num; i ++) {
            if (strcmp(cookie->name, http->cookie_array[i].name) == 0)
                break;
        }
        if (i < http->cookie_num)
            strcpy(http->cookie_array[i].value, cookie->value);
        else
            http->cookie_num ++;
    }

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
static uint32_t gethost(const char *buf)
{
    uint32_t addr = inet_addr(buf);

    if (addr == INADDR_NONE) {
        struct hostent *host = gethostbyname(buf);
        if (host)
            addr = *((uint32_t *)(host->h_addr_list[0]));
    }
    if (addr == INADDR_ANY)
        addr = INADDR_NONE;

    return addr;
}

#ifndef WIN32
static int tcp_connect(int sockfd, struct sockaddr_in* saptr, struct timeval *tv)
{
    int on, error;
    fd_set wset;
    struct sockaddr_in *addr;

    on = 1;
    ioctl(sockfd, FIONBIO, (int)&on);

    error = tr069_connect(sockfd, saptr);
    if (error == 0)
        goto done;            // connect completed immediately

    FD_ZERO(&wset);
    FD_SET(sockfd, &wset);

    addr = (struct sockaddr_in *)saptr;

    {
        fd_set rset = wset;

        if ((select(sockfd + 1, &rset, &wset, NULL, tv)) <= 0)
            TR069ErrorOut("select 0x%x, %hu errno = %d! %s\n", addr->sin_addr.s_addr, addr->sin_port, errno, strerror(errno));

        if (FD_ISSET(sockfd, &rset))
            TR069ErrorOut("select errno = %d! %s\n", errno, strerror(errno));
    }

    if (FD_ISSET(sockfd, &wset)) {
        socklen_t len;

        error = -1;
        len = sizeof(error);
        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (caddr_t)&error, &len);
        if (error == 0)
            goto done;
        TR069Error("%d\n", error);
    } else {
        TR069Error("\n");
    }

Err:
    error = -1;
done:
    on = 0;
    ioctl(sockfd, FIONBIO, (int)&on);
    return error;
}
#endif

static int tcp_send(SOCKET sock, char *buf, int nbytes)
{
    int ret, len = 0;

    buf[nbytes] = 0;
#ifndef ANDROID_LOGCAT_OUTPUT
    TR069Debug("\r\n----------------------------------------------------------------\r\nSEND:\r\n");
    TR069Debug("%s\n", buf);
#endif

    while (len < nbytes) {
        ret = send(sock, buf + len, nbytes - len, MSG_NOSIGNAL);
        if (ret == -1)
            TR069ErrorOut("send\n");
        len += ret;
    }
    buf[len] = 0;

    return 0;
Err:
    return -1;
}

static int tcp_wait(SOCKET sock, int sec, int usec)
{
    struct timeval tv;
    fd_set set;
    int ret, nfds;

    tv.tv_sec = sec;
    tv.tv_usec = usec;

    FD_ZERO(&set);
    FD_SET(sock, &set);
    nfds = (int)(sock + 1);
    //TR069Printf("WAIT 0\n");
    ret = select(nfds, &set, NULL, NULL, &tv);
    //TR069Printf("WAIT 1\n");
    if (ret <= 0)
        TR069ErrorOut("select\n");

    return 0;
Err:
    return -1;
}

static int tcp_recv(SOCKET sock, char *buf, int nbytes, const char *mark)
{
    int ret, len = 0;
    uint clk0, clk;

    clk0 = tr069_10ms( );
    while (len < nbytes) {
        if (tcp_wait(sock, g_tr069_httpTimeout, 0))
            TR069ErrorOut("tcp_wait\n");
        ret = recv(sock, buf + len, (int)(nbytes - len), 0);
        if (ret <= 0)
            TR069ErrorOut("recv ret = %d\n", ret);
        len += ret;
        clk = tr069_10ms( );
        if (clk - clk0 >= 100) {
            clk0 = clk;
            TR069Printf("recv %%%d\n", len * 100 / nbytes);
        }
        if (mark) {
            buf[len] = 0;
            if (strstr(buf, mark))
                break;
        }
    }
    buf[len] = 0;
#ifndef ANDROID_LOGCAT_OUTPUT
    TR069Debug("\r\n----------------------------------------------------------------\r\nRECV:\r\n");
    {
        char *p = strstr(buf, "SetParameterValues");
        if (p)
            p = strstr(p, "Password");
        if (p)//避免打印出密码
            TR069Debug("SetParameterValues with Password\n");
        else
            TR069Debug("%s\n", buf);
    }
#endif

    return (int)len;
Err:
    return -1;
}

//------------------------------------------------------------------------------
int tr069_http_connect(struct Http *http)
{
    uint32_t addr;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in    sa;

    if (http->sock != INVALID_SOCKET)
        TR069ErrorOut("http->sock = %d\n", http->sock);

    addr = gethost(http->host);
    if (addr == INADDR_NONE)
        TR069ErrorOut("INADDR_NONE %s\n", http->host);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
        TR069ErrorOut("INVALID_SOCKET\n");
    sa.sin_family = AF_INET;
    sa.sin_port = htons(http->port);
    sa.sin_addr.s_addr = addr;

    TR069Printf("socket = %d\n", sock);

#ifdef WIN32
    if (tr069_connect(sock, &sa))
        TR069ErrorOut("connect!\n");
#else
    {
        struct timeval tv;
        tv.tv_sec = g_tr069_httpTimeout;
        tv.tv_usec = 0;
        if (tcp_connect(sock, &sa, &tv))
            TR069ErrorOut("connect!\n");
    }
#endif

    http->sock = sock;
    if (!g_tr069_holdCookie)
        http->cookie_num = 0;

    return 0;
Err:
    if (sock != INVALID_SOCKET) {
        TR069Printf("closesocket = %d\n", sock);
        closesocket(sock);
    }
    return -1;
}

//------------------------------------------------------------------------------
int tr069_http_disconnect(struct Http *http)
{
    if (http->sock != INVALID_SOCKET) {
        TR069Printf("closesocket = %d\n", http->sock);
        closesocket(http->sock);
        http->sock = INVALID_SOCKET;
    }
    return 0;
}

//------------------------------------------------------------------------------
static int http_clt_send(struct Http *http, char *content, int length)
{
    int i, len;
    char *buf;
    struct Digest *digest = &(http->digest);
#if 0
    if (content) {
        FILE *fp = fopen("1.txt", "rb");
        if (!fp)
            TR069ErrorOut("fopen\n");
        length = fread(content, 1, 4096, fp);
        TR069Printf("length = %d\n", length);
        fclose(fp);
    }
#endif
    buf = http->buf;
    if (http->port == 80)
        len = sprintf(buf, "%s %s HTTP/1.1\r\nHost: %s\r\n", digest->method, digest->uri, http->host);
    else
        len = sprintf(buf, "%s %s HTTP/1.1\r\nHost: %s:%hu\r\n", digest->method, digest->uri, http->host, http->port);
    for (i = 0; i < http->cookie_num; i ++)
        len += sprintf(buf + len, "Cookie: %s=%s\r\n", http->cookie_array[i].name, http->cookie_array[i].value);

    len += sprintf(buf + len, "User-Agent: Yuxing_TR069_CPE\r\n");

    len += sprintf(buf + len, "%s", http->header);

    len += sprintf(buf + len, "Content-Length: %d\r\n", length);
    switch(http->authen) {
    case  AUTHENTICATION_NONE:
        break;
    case  AUTHENTICATION_BASIC:
        len += sprintf(buf + len, "Authorization: Basic %s\r\n", digest->response);
        break;
    case AUTHENTICATION_DIGEST:
        len += sprintf(buf + len, "Authorization: Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\"",
                        digest->username, digest->realm, digest->nonce, digest->uri);
        if (digest->qop[0] == 'a')
            len += sprintf(buf + len, ", qop=\"%s\", nc=%08x, cnonce=\"%s\"", digest->qop, digest->nc, digest->cnonce);
        len += sprintf(buf + len, ", response=\"%s\", opaque=\"%s\"", digest->response, digest->opaque);
        len += sprintf(buf + len, "\r\n");
        break;
    default:
        TR069ErrorOut("http->authen = %d\n", http->authen);
    }

    len += sprintf(buf + len, "\r\n");

    if (tcp_send(http->sock, buf, len))
        TR069ErrorOut("tcp_send buf\n");

    if (content && length > 0 && tcp_send(http->sock, content, length))
        TR069ErrorOut("tcp_send content\n");

    return 0;
Err:
    return -1;
}

#if 1
#define END_HEAD_STRING     "\n\r\n"
#define END_LINE_STRING     "\n"
#define END_OF_CHUNK        "\r\n"
#define END_OF_CHUNK_LEN    strlen(END_OF_CHUNK)
#else
#define END_HEAD_STRING     "\r\n\r\n"
#define END_LINE_STRING     "\r\n"
#endif

static int http_clt_recv_chunk(int sock, char *buf, int len, char *content, int size)
{
    int l, length, contlen;
    char *p;

    contlen = 0;

    while (1) {
        buf[len] = 0;

        p = strstr(buf, END_OF_CHUNK);
        if (!p) {
            l = tcp_recv(sock, buf + len, HTTP_HEAD_LEN - len, END_OF_CHUNK);
            if (l <= 0)
                TR069ErrorOut("tcp_recv0\n");
            len += l;
            buf[len] = 0;
            p = strstr(buf, END_OF_CHUNK);
            if (!p)
                TR069ErrorOut("END_LINE_STRING\n");
        }

        if (sscanf(buf, "%x", &length) != 1)
            TR069ErrorOut("\n");
        if (length <= 0) {
            if (content)
                content[contlen] = 0;
            return contlen;
        }

        length += END_OF_CHUNK_LEN;
        if (content && contlen + length > size)
            TR069ErrorOut("length(%d/0x%x) > size(%d/0x%x)\n", contlen + length, contlen + length, size, size);

        p += END_OF_CHUNK_LEN;
        len -= p - buf;

        if (len < length) {
            if (len > 0) {
                if (content)
                    memcpy(content + contlen, p, len);
                contlen += len;
                length -= len;
            }
            len = 0;

            if (length > 0) {
                if (content) {
                    if((tcp_recv(sock, content + contlen, length, NULL)) <= 0)
                        TR069ErrorOut("tcp_recv1\n");
                    contlen += length;
                } else {
                    contlen += length;
                    while (length > 0) {
                        l = length;
                        if (l > HTTP_HEAD_LEN)
                            l = HTTP_HEAD_LEN;
                        if((tcp_recv(sock, buf, l, NULL)) <= 0)
                            TR069ErrorOut("tcp_recv2\n");
                        length -= l;
                    }
                }
            }

        } else {
            if (content)
                memcpy(content + contlen, p, length);
            contlen += length;
            len -= length;
            p += length;
            length = 0;

            if (len > 0)
                memmove(buf, p, len);
        }
        contlen -= END_OF_CHUNK_LEN;
    }

Err:
    return -1;
}

static int http_clt_recv(struct Http *http, char *content, int *pLength)
{
    int code, length, len, size;
    char *p, *buf, *cont;
    int chunk;//这里的所有局部变量都不含下横线，所以不要用cont_len，保持一贯性

    chunk = 0;
    size = *pLength;
    *pLength = 0;
    buf = http->buf;
    len = tcp_recv(http->sock, buf, HTTP_HEAD_LEN, END_HEAD_STRING);
    if (len <= 0)
        TR069ErrorOut("tcp_recv ret = %d\n", len);
    buf[len] = 0;
    cont = strstr(buf, END_HEAD_STRING);
    if (!cont)
        TR069ErrorOut("END_HEAD_STRING\n");//找不到头部结束

    if (strnicmp(buf, "HTTP/1.1", 8))
        TR069ErrorOut("strnicmp HTTP/1.1\n");
    if (sscanf(buf + 9, "%d", &code) != 1)
        TR069ErrorOut("sscanf code\n");

    p = strstr(buf, END_LINE_STRING);
    if (!p)
        TR069ErrorOut("END_LINE_STRING\n");////找不到行结束
    length = 0;
    while (p < cont) {
        if (!p)
            TR069ErrorOut("p is NULL\n");
        p += strlen(END_LINE_STRING);

        if (strnicmp(p, "Transfer-Encoding: ", 19) == 0) {
            chunk = 1;
            TR069Printf("chunk mode\n");
        } else if (strnicmp(p, "Content-Length: ", 16) == 0) {
            p += 16;
            if (sscanf(p, "%d", &length) != 1)
                TR069ErrorOut("sscanf length\n");
        } else if (strnicmp(p, "WWW-Authenticate: ", 18) == 0) {
            int ret = -1;
            p += 18;
            if (strnicmp(p, "Basic ", 6) == 0)
                ret = http_base(http, p + 6);
            else if (strnicmp(p, "Digest ", 7) == 0)
                ret = tr069_http_digest(http, p + 7);
            if (ret)
                TR069ErrorOut("http_digest\n");
        } else if (strnicmp(p, "Set-Cookie: ", 12) == 0) {
            p += 12;
            if (http_cookie(http, p))
                TR069ErrorOut("Set-Cookie\n");
        } else if (strnicmp(p, "Set-Cookie2: ", 13) == 0) {
            p += 13;
            if (http_cookie(http, p))
                TR069ErrorOut("Set-Cookie2\n");
        } else if (strnicmp(p, "Connection: close", 17) == 0) {
            p += 17;
            http->discnnct = 1;
        }
        p = strstr(p, END_LINE_STRING);
        if (!p)
            TR069ErrorOut("END_LINE_STRING\n");////找不到行结束
    }

    cont += strlen(END_HEAD_STRING);
    len -= cont - buf;
    if (len > 0)
        memmove(buf, cont, len);

    if (code != 200) {
        if (chunk) {
            length = http_clt_recv_chunk(http->sock, buf, len, NULL, 0);
            if (length < 0)
                TR069ErrorOut("http_clt_recv_chunk\n");
        } else {
            length -= len;
            while (length > 0) {
                len = tcp_recv(http->sock, buf, HTTP_HEAD_LEN, NULL);
                if (len <= 0)
                    TR069ErrorOut("tcp_recv ret =%d\n", len);
                length -= len;
            }
        }
        return code;
    }

    if (chunk) {
        length = http_clt_recv_chunk(http->sock, buf, len, content, size);
        if (length < 0)
            TR069ErrorOut("http_clt_recv_chunk\n");
    } else {
        if (length > 0) {
            if (length > size)
                TR069ErrorOut("length(%d/0x%x) > size(%d/0x%x)\n", length, length, size, size);
    
            if (len > 0) {
                if (len > length)
                    TR069ErrorOut("len = %d / %d\n", len, length);
                memcpy(content, buf, len);
                length -= len;
            }
            if (length > 0 && tcp_recv(http->sock, content + len, length, NULL) <= 0)
                TR069ErrorOut("tcp_recv\n");
            length += len;
        }
    }
    *pLength = length;

    return code;
Err:
    return -1;
}

int tr069_http_transfer(struct Http *http, char *content, int *pLength, int size)
{
    int code, loops;
    int len, length;
    static int isAuth = 0;
    struct Digest *digest = &(http->digest);

    loops = 0;
    length = *pLength;
    *pLength = 0;

    http->code = 0;
Repeat:
    if (http_clt_send(http, content, length))
        TR069ErrorOut("http_clt_send\n");
    len = size;
    code = http_clt_recv(http, content, &len);
    if (code == -1)
        TR069ErrorOut("http_clt_recv\n");
    http->code = code;

    switch(code) {
    case 200:
        digest->nc++;
        http_digest_author(digest, digest->response);
        *pLength = len;
        if (!isAuth) {
            tr069_global_setOpaque(digest->opaque);
            isAuth = 1;
        }
        break;
    case 204://No Content
        *pLength = 0;
         isAuth = 0;
        break;
    case 401:
        TR069Printf("loops = %d\n", loops);
        if (loops > 0)
            TR069ErrorOut("loops = %d\n", loops);
        loops ++;
        if (http->discnnct == 1) {
            tr069_http_disconnect(http);
            if (tr069_http_connect(http))
                TR069ErrorOut("http_connect failed\n");
        }
        goto Repeat;
    default:
        TR069ErrorOut("code = %d\n", code);
    }
    return 0;
Err:
    *pLength = 0;
    return -1;
}


static int http_srv_recv(struct Http *http, char* content, int* pLength)
{
    int l, len, size, length;
    char *p, *buf, *cont;

    size = *pLength;
    *pLength = 0;
    buf = http->buf;
    len = tcp_recv(http->sock, buf, HTTP_HEAD_LEN, END_HEAD_STRING);
    if (len <= 0)
        TR069ErrorOut("len = %d\n", len);
    buf[len] = 0;
    cont = strstr(buf, END_HEAD_STRING);
    if (!cont)
        TR069ErrorOut("END_HEAD_STRING\n");//找不到头部结束

    p = strchr(buf, ' ');
    if (!p)
        TR069ErrorOut("blank not fond\n");
    l = p - buf;
    if (l < 0 || l >= DIGEST_METHOD_LEN)
        TR069ErrorOut("len = %d\n", len);
    memcpy(http->digest.method, buf, (uint32_t)l);
    http->digest.method[l] = 0;

    p = strstr(buf, END_LINE_STRING);
    if (!p)
        TR069ErrorOut("END_LINE_STRING\n");////找不到行结束
    length = 0;
    while (p < cont) {
        if (!p)
            TR069ErrorOut("p is NULL\n");
        p += strlen(END_LINE_STRING);

        if (strnicmp(p, "Content-Length: ", 16) == 0) {
            p += 16;
            if (sscanf(p, "%d", &length) != 1)
                TR069ErrorOut("sscanf length\n");
        } else if (strnicmp(p, "Authorization: Digest ", 22) == 0) {
            p += 22;
            http_author(http, p);
        } else if (strnicmp(p, "Connection: close", 17) == 0) {
            p += 17;
            http->discnnct = 1;
        }
        p = strstr(p, END_LINE_STRING);
    }

    if (length > 0) {
        if (length >= size)
            TR069ErrorOut("length(%d/0x%x) > size(%d/0x%x)\n", length, length, size, size);

        cont += strlen(END_HEAD_STRING);
        len -= cont - buf;
        if (len > 0) {
            if (len > length)
                TR069ErrorOut("len = %d / %d\n", len, length);
            memcpy(content, cont, len);
        }
        if (length > len && tcp_recv(http->sock, content + len, length - len, NULL) <= 0)
            TR069ErrorOut("tcp_recv\n");

        *pLength = length;
    }

    return 0;
Err:
    return -1;
}

//------------------------------------------------------------------------------
static int http_srv_send(struct Http *http)
{
    int len;
    char *buf;
    struct Digest *digest = &(http->digest);

    buf = http->buf;

    switch(http->authen) {
    case AUTHENTICATION_NONE:
        {
            char buf16[16];
            sprintf(http->digest.realm, "TR069 Authenticate");
            sprintf(buf16, "%x", tr069_sec( ));
            http_md5(buf16, strlen(buf16), http->digest.nonce);
            sprintf(buf16, "%d", tr069_sec( ));
            http_md5(buf16, strlen(buf16), http->digest.opaque);
        }
        len = sprintf(buf, "HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Digest realm=\"%s\", qop=\"auth\", nonce=\"%s\", opaque=\"%s\"\r\nContent-Type: text/html\r\nContent-Length: 0\r\n", digest->realm, digest->nonce, digest->opaque);
        break;
    case AUTHENTICATION_DIGEST:
        len = sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 0\r\n");
        break;
    default:
        TR069ErrorOut("authen = %d\n", http->authen);
    }
    len += sprintf(buf + len, "\r\n");

    if (tcp_send(http->sock, buf, len))
        TR069ErrorOut("tcp_send\n");

    return 0;
Err:
    return -1;
}

int tr069_http_accept(struct Http *http, SOCKET srvsock, char* buffer, int* pLength)
{
    socklen_t len, size;
    struct sockaddr_in sa;

    if (http->sock != INVALID_SOCKET)
        TR069ErrorOut("http->sock = %d\n", http->sock);

    len = sizeof(sa);
    http->sock = accept(srvsock, (struct sockaddr *)&sa, &len);
    if (http->sock == INVALID_SOCKET) {
#ifdef WIN32
        TR069Printf("WSAGetLastError = %d\n", WSAGetLastError());
#endif
        TR069ErrorOut("accept\n");
    }
    TR069Printf("accept = %d\n", http->sock);

    http->discnnct = 0;

    size = *pLength;
    if (http_srv_recv(http, buffer, pLength))
        TR069ErrorOut("http_srv_recv 1\n");

    if (http->digest.username[0] == 0)
        http->authen = AUTHENTICATION_DIGEST;
    if (http_srv_send(http))
        TR069ErrorOut("http_srv_send 1\n");

    TR069Printf("username = %s discnnct = %d, authen = %d\n", http->digest.username, http->discnnct, http->authen);
    if (http->discnnct == 0 && http->authen == AUTHENTICATION_NONE) {
        *pLength = size;
        if (http_srv_recv(http, buffer, pLength))
            TR069ErrorOut("http_srv_recv 2\n");
        if (http_srv_send(http))
            TR069ErrorOut("http_srv_send 2\n");
    }

    TR069Printf("closesocket = %d\n", http->sock);
    closesocket(http->sock);
    http->sock = INVALID_SOCKET;

    if (http->authen == AUTHENTICATION_DIGEST) {
        http->authen = AUTHENTICATION_NONE;
        return 1;
    }
    return 0;
Err:
    if (http->sock != INVALID_SOCKET) {
        TR069Printf("closesocket = %d\n", http->sock);
        closesocket(http->sock);
        http->sock = INVALID_SOCKET;

        http->authen = AUTHENTICATION_NONE;
    }
    return -1;
}

int tr069_http_post(char *url, char *username, char *password, char *content, int length)
{
    int ret;
    struct Digest *digest;
    struct Http http;

    memset(&http, 0, sizeof(struct Http));
    digest = &http.digest;

    if (!url || !username || !password)
        TR069ErrorOut("url = %p, username = %p, password = %p\n", url, username, password);
    strcpy(http.digest.method, "POST");
    if (tr069_checkurl(url, digest->uri, HTTP_URI_LEN, http.host, HTTP_HOST_LEN, &http.port))
        TR069ErrorOut("url = %s\n", url);
    strcpy(digest->username, username);
    strcpy(digest->password, password);
    TR069Printf("POST %s\n", url);

    if (tr069_http_connect(&http))
        TR069ErrorOut("http_connect failed\n");
    ret = tr069_http_transfer(&http, content, &length, 0); 
    tr069_http_disconnect(&http);
    if (ret)
        TR069ErrorOut("http_transfer failed\n");

    return 0;

Err:
    return -1;
}

void tr069_http_init(struct Http *http)
{
    if (!g_tr069_holdCookie)
        memset(http, 0, sizeof(struct Http));
    http->sock = INVALID_SOCKET;
}
