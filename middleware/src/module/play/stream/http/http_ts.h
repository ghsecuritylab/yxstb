
#ifndef __HTTP_TS__
#define __HTTP_TS__

#include "http_live.h"

typedef struct {
    int (*ht_recv_space)(void* handle);
    int (*ht_recv_push)(void* handle);

    int (*file_recv_begin)(void* handle);
    void (*file_recv_end)(void* handle, int end);//end: 1 recv finished, 2 push finished
    void (*slice_recv_begin)(void* handle);
    void (*slice_recv_end)(void* handle);

    void (*ht_deal_error)(void* handle, STRM_MSG msgno);
    void (*ht_deal_cmd)(void* handle, StreamCmd* strmCmd);
    void (*ht_deal_msg)(void* handle, STRM_MSG msgno, int arg);

    void (*ht_100ms)(void* handle);//100毫秒调用一次
    void (*ht_1000ms)(void* handle);//1秒调用一次

    void (*ht_destroy)(void* handle);
} HttpTsOp;

struct tagHttpTs {
    int         index;
    HTTPLoop_t  loop;
    HTTP_t      http;

    HttpOp      op;

    char        url[STREAM_URL_SIZE];
    char        uri[STREAM_URL_SIZE];
    int         uri_base;

    HttpLive*   hls;
    int         hls_state;
    int         hls_index;
    uint32_t        hls_clk;
    int         stream_index;

#ifdef ENABLE_SAVE_HTTP
    FILE*       save_fp;
    long long   save_len;
#endif

    HttpTsOp    ts_op;
    void*       ts_handle;

    int         end_flg;
    int         sync_flg;//数据发送完毕后再调用 file_recv_end 或 slice_recv_end
    int         order_flg;//一个分片取完再请求下一个分片

    //分配播放时使用
    uint32_t        time_start;
    int         byte_skip;

    StrmBuffer* sb;
    StrmBufQue* sbq;
};
typedef struct tagHttpTs    HttpTs;

struct tagHttpTsArg {
    char        url[STREAM_URL_SIZE];
    long long   key;
    int         start;
    int         length;
    int         bitrate;
};
typedef struct tagHttpTsArg     HttpTsArg;

int http_ts_slice_duration(HttpTs* ht);

void http_ts_reset(HttpTs* ht);

void* http_ts_live_create(HttpTs* ht, PlayArg *arg, HttpTsArg *httparg);
void* http_ts_play_create(HttpTs* ht, PlayArg *arg, HttpTsArg *httparg);
void* http_ts_record_create(HttpTs* ht, PlayArg *plyArg, RecordArg *recArg, HttpTsArg *httparg);

#endif
