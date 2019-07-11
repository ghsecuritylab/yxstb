/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#ifndef __HTTP_H__
#define __HTTP_H__

#include "../stream.h"

#define HTTP_M3U8_SIZE          (1024 * 1024)
#define HTTP_RECV_SIZE          (128 * 1024)

#define HTTP_TIMEOUT_SEC        15

#define HTTP_RECEIVE_SIZE       (1024 * 4)

#define HTTP_COOKIE_SIZE        1024

#define CONTENT_TYPE_SIZE        256

#define HTTP_HOST_SIZE          128

typedef enum {
    HTTP_MSG_ERROR = 0,
    HTTP_MSG_NET_TIMEOUT,
    HTTP_MSG_REFUSED_SOCKET,

    HTTP_CODE_Forbidden = 403,
    HTTP_CODE_Not_Found = 404,
    HTTP_CODE_Internal_Server_Error = 500,
} HTTP_MSG;

typedef struct __CookieArg {
    uint32_t magic;
    char cookie[HTTP_COOKIE_SIZE];
} CookieArg;

struct HTTP;
struct HTTPLoop;
typedef struct HTTP* HTTP_t;
typedef struct HTTPLoop* HTTPLoop_t;

typedef struct {
    int (*recv_begin)(void* handle);
    void (*recv_sync)(void* handle);//处理数据
    void (*recv_end)(void* handle);

    void (*deal_error)(void* handle, HTTP_MSG msgno);
} HttpOp;

typedef struct {
    void (*deal_cmd)(void* handle, StreamCmd* strmCmd);
    void (*deal_msg)(void* handle, STRM_MSG msgno, int arg);

    void (*local_100ms)(void* handle);//100毫秒调用一次
    void (*local_1000ms)(void* handle);//1秒调用一次
} HttpLoopOp;

void strm_http_set_opset(HTTP_t http, HttpOp* op, void* handle);

HTTPLoop_t strm_httploop_create(int idx, HttpLoopOp* op, void* handle, mid_msgq_t msgq);
void strm_httploop_loop(HTTPLoop_t loop);
void strm_httploop_break(HTTPLoop_t loop);
void strm_httploop_delete(HTTPLoop_t loop);

unsigned int    strm_httploop_clk(HTTPLoop_t loop);
ind_tlink_t     strm_httploop_tlink(HTTPLoop_t loop);

HTTP_t strm_http_create(HTTPLoop_t loop, int size);

void strm_http_reset(HTTP_t http);

int     strm_http_request(HTTP_t http, char *url, long long byte_start, long long byte_count);

int     strm_http_buf_get(HTTP_t http, char** pbuf, int* plen);
void    strm_http_buf_pop(HTTP_t http, int len);
int     strm_http_buf_length(HTTP_t http);
void    strm_http_buf_replace(HTTP_t http, ts_buf_t ts_buf);

void    strm_http_sbq_replace(HTTP_t http, StrmBufQue* sbq, int size);

void strm_http_msgback(HTTP_t http, int msgno, int arg);

int         strm_http_get_uri(HTTP_t http, char *uri);
int         strm_http_get_host(HTTP_t http, char *host);
int         strm_http_get_code(HTTP_t http);
int         strm_http_get_active(HTTP_t http);
uint32_t    strm_http_get_recvTimes(HTTP_t http);
long long   strm_http_get_contentLen(HTTP_t http);
void        strm_http_get_contentType(HTTP_t http, char* content_type);
//已下载字节数
long long   strm_http_get_contentBytes(HTTP_t http);
long long   strm_http_get_contentLength(HTTP_t http);

void strm_http_set_sqm(HTTP_t http, int sqm);
void strm_http_set_etag(HTTP_t http, int etag);
void strm_http_set_gzip(HTTP_t http, int gzip);
void strm_http_set_retry(HTTP_t http, int retry);

void strm_http_set_limit(HTTP_t http, int byterate);
void strm_http_set_cookie(HTTP_t http, char* cookie);
void strm_http_set_bitrate(HTTP_t http, int bitrate);

void strm_http_debug(HTTP_t http);

#endif//__HTTP_H__




