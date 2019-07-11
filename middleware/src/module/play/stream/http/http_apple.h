
#ifndef __HTTP_APPLE__
#define __HTTP_APPLE__

#include "http.h"
#include "http_live.h"
#include "Verimatrix.h"

//#define ENABLE_SAVE_APPLE
#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_VERIMATRIX_OTT
#include "curl.h"
#include "libzebra.h"

extern void *ymm_stream_createCryptM2MHandle(int algorithm, int variant, int en_decrypt, int ts_block);
extern int ymm_stream_loadM2MKeys(void *h, char *keys,int keylen, char *iv, int ivlen);
extern int ymm_stream_cryptM2MStream(void *h, char *buf, int len,char *output);
extern int ymm_stream_destoryCryptM2MKeyHandle(void *h);
#endif

#define STREAM_BUFFER_SIZE      1316

#define HTTP_APPLE_M3U8_SIZE    (1316 * 768)

enum {
    LIVE_STATE_INDEX = 0,
    LIVE_STATE_M3U8,
    LIVE_STATE_SLICE,
    LIVE_STATE_IDLE,
};

typedef struct tagHttpApple     HttpApple;
typedef struct tagHAppleTrack   HAppleTrack;
typedef struct tagHAppleHistory HAppleHistory;

typedef struct tagAppleBufQue   AppleBufQue;
typedef struct tagHttpAppleArg  HttpAppleArg;

typedef struct tagAppleTask  AppleTask;
struct tagAppleTask {
    HTTP_t          http;
    HLiveMedia*     media;

    int             index;

    char*           recv_buffer;
    ts_buf_t        recv_ts_buf;

#ifdef USE_VERIMATRIX_OTT
    void*           crypt_handle;
    char*           crypt_buffer;
    ts_buf_t        crypt_ts_buf;
#endif

    uint32_t            ts_pid;
    struct ts_psi       ts_psi;
    ts_parse_t          ts_parse;
    ts_dr_subtitle_t    ts_subtitle;

    char            m3u8_host[HTTP_HOST_SIZE];
    char            m3u8_path[STREAM_URL_SIZE];
};

struct tagHAppleTrack {
    HttpApple*  ha;
    HLiveTrack* ltrack;

    int         index;
    int         state;
    int         isSkipSlice;
    int         isSliceBegun;

    AppleTask   task;
    AppleTask   switch_task;

    int         switch_index;
    int         switch_sequence;

    uint32_t    pid;
    char*       uri;

    char*       key_uri;

    int         serv_diff;

    uint32_t    load_clk;
    uint32_t    load_datetime;

    int         load_length;
    int         load_duration;
    int         load_bandsmooth;
};

enum {
    HAPPLE_TRACK_VIDEO = 0,
    HAPPLE_TRACK_AUDIO,
    HAPPLE_TRACK_SUBTITLE,
    HAPPLE_TRACK_NUM
};

struct tagHttpApple {
    int             index;

    int             state;
    int             scale;

    HTTPLoop_t      loop;

    HAppleTrack*    track_array[HAPPLE_TRACK_NUM];

    int             alternative_audio;//音轨分离

    char            url[STREAM_URL_SIZE];

    char            index_host[HTTP_HOST_SIZE];
    char            index_path[STREAM_URL_SIZE];

    int             servicetype;//0为Webtv点播、1为Webtv直播、2为Webtv时移、3为Webtv录播
    int             huaweimode;//URL含有servicetype字段

    HttpLive*       hls;

#ifdef ENABLE_SAVE_APPLE
    FILE*           save_fp;
#endif

    int             data_flag;
    int             error_code;

    int             load_sequence;
    int             load_bandwidth;

    HLiveStream*    stream;

    struct ts_psi           ts_psi;
    struct ts_dr_subtitle   dr_subtitle;

    int             end_flg;

    uint32_t        m3u8_clk;
    uint32_t        slice_clk;
    int             slice_duration;

    /*
        IPTV每隔半个分片周期请求一次
     */
    uint32_t        retry_clk;
    uint32_t        retry_base;
    uint32_t        retry_times;

    uint32_t        time_seek;
    uint32_t        time_start;
    uint32_t        time_length;
    uint32_t        time_current;

    int             time_begin;
    int             time_end;

    StrmBuffer*     sb;
    AppleBufQue*    abq;

    StreamMsgQ*     strm_msgq;
    StreamPlay*     strm_play;
};

struct tagHttpAppleArg {
    char    url[STREAM_URL_SIZE];
    int     shiftlen;//VOD 书签播放，IPTV 时移长度
    int     servicetype;
    int     huaweimode;
    int     begin;
    int     end;
};

void http_apple_bandwidth   (HttpApple* ha, int bandwidth);
int  http_apple_index_request(HttpApple* ha, int servicetype);

void apple_audio_error          (HAppleTrack* track);

void* apple_audio_load_key      (HLiveMedia* media);

void apple_audio_m3u8_end       (HttpApple *ha);
void apple_audio_m3u8_delay     (HttpApple *ha);
int  apple_audio_m3u8_request   (HAppleTrack* track, HTTP_t http, char* uri, int etag);
int  apple_audio_m3u8_refresh   (HttpApple* ha, int reset);

void apple_audio_slice_download (HttpApple* ha);

AppleBufQue* apple_bufque_create(int size, int num);
void         apple_bufque_reset (AppleBufQue* abq);
void         apple_bufque_delete(AppleBufQue* abq);
int          apple_bufque_space (AppleBufQue* abq);

void apple_buffer_push          (HttpApple* ha);
int  apple_buffer_delay         (HttpApple* ha);
void apple_buffer_1000ms        (HttpApple* ha);
int  apple_buffer_duration      (HttpApple* ha);
void apple_buffer_slice_end     (HAppleTrack* track);
void apple_buffer_slice_valid   (HAppleTrack* track, int valid);

void apple_buffer_switch        (HAppleTrack* track, char* buf, int len);

void apple_switch_clear_crypt   (AppleTask* task);
void apple_switch_clear         (AppleTask* task);
void apple_switch_begin         (HAppleTrack* track);
void apple_switch_end           (HAppleTrack* track);

#ifdef __cplusplus
}
#endif

#endif
