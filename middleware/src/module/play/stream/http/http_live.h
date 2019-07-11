
#ifndef __HTTP_LIVE__
#define __HTTP_LIVE__

#define HLS_M3U8_SIZE        (1024 * 8)
#define HLS_ARRAY_SIZE        720

typedef struct tagHLiveSlice HLiveSlice;
struct tagHLiveSlice {
    uint32_t    datetime;
    int         duration;
    char        uri[1];
};

typedef struct tagHLiveElem HLiveElem;
struct tagHLiveElem {
    char    language[4];
    char    uri[1];
};

typedef struct tagHLiveMedia HLiveMedia;
struct tagHLiveMedia {
    int             slice_num;
    HLiveSlice**    slice_array;

    int             slice_complete;
    uint32_t        total_duration;

    int             x_duration;
    int             x_sequence;

    uint8_t*        key_iv;
    char*           key_uri;
};

typedef struct tagHLiveTrack HLiveTrack;
struct tagHLiveTrack {
    int             num;
    HLiveMedia      media;
    HLiveMedia      switch_media;
    HLiveElem*      array[TS_AUDIO_NUM];
};

typedef struct tagHLiveStream HLiveStream;
struct tagHLiveStream {
    HLiveStream*    prev;
    HLiveStream*    next;

    int             bandwidth;
    char            uri[1];
};

typedef struct tagHttpLive HttpLive;
struct tagHttpLive {
    HLiveMedia      media;
    HLiveStream*    stream_queue;

    HLiveTrack      audio;
    HLiveTrack      subtitle;
};

HttpLive*       http_live_create(void);

void http_live_parse_stream(HttpLive* hls, char* m3u8_buf, int m3u8_len);
void http_live_parse_slice(HLiveMedia* media, char* m3u8_buf, int m3u8_len);

void http_live_reset_slice(HLiveMedia* media);

void            http_live_delete(HttpLive* hls);

int             http_live_vodIndex(HLiveMedia* media, uint32_t* psec);
int             http_live_iptvIndex(HLiveMedia* media, uint32_t* pdatetime);
HLiveSlice*     http_live_slice(HLiveMedia* media, int sequence);
HLiveStream*    http_live_stream(HttpLive* hls, int bandwidth);

#endif//__HTTP_LIVE__
