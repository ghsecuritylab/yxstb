
/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#ifndef _RTSP_APP_H_
#define _RTSP_APP_H_

#include <sys/select.h>

#include "../stream.h"

#include "fcc_port.h"
#include "stream_port.h"

#include "rtsp_stat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TIMESHIFT_BOUNDARY_SEC          5
#define HEARTBIT_PERIOD_SEC             30
#define NAT_HEARTBIT_PERIOD_SEC         50

#define INTERVAL_CLK_SESSION_TIMEOUT    (15 * 100)
#define INTERVAL_CLK_IDLE               (60*60 * 100)
#define INTERVAL_CLK_SAVE               (3 * 100)
#define INTERVAL_CLK_POST               (3 * 100)
#define INTERVAL_CLK_STATISTIC          (10 * 100)
#define INTERVAL_CLK_SHIFTCHK           (1 * 100)
#define INTERVAL_CLK_LOST               (1 * 100)

#define TCP_BUFFER_LENGTH              (1024 * 64)
#define CTRL_BUFFER_LENGTH              (1024 * 4)

#define INTERVAL_CLK_MESSAGE            500

#define TIMEOUT_CLK_COMMAND             400
#define TIMEOUT_CLK_TEARDOWN            200
#define TIMEOUT_CLK_BURST               (10 * 100)
#define TIMEOUT_CLK_ARQ                 (1 * 100)

#define TIMEOUT_CLK_BURST_INT           (1 * 100)

#define INTERVAL_CLK_SEC                100

#define INTERVAL_CLK_RECV_TIMEOUT       500

#define CMD_QUEUE_LENGTH                50
#define SESSION_LENGTH                  32
#define USERAGENT_LENGTH                64

#define CACHE_CLK_LEVEL                 1000
#define CACHE_CLK_RANGE                 200

#define RTSP_ERROR_NONE                 0

#define RTSP_ERROR_CODE                 1004
#define RTSP_ERROR_SETUP                1006

#define RTSP_ERROR_URL                  1013
#define RTSP_ERROR_CONNECT              1014
#define RTSP_ERROR_SOCKET               1017
#define RTSP_ERROR_TIMEOUT              1018
#define RTSP_ERROR_OPEN                 1025
#define RTSP_ERROR_DISCRIPT             1026
#define RTSP_ERROR_ANALYZE              1027
#define RTSP_ERROR_REDIRECT             1028

#define LINE_LEN_128                    128
#define LINE_NUM_2                      2
#define RRS_NUM_4                       4
#define RRS_ARRAY_SIZE_64               64

#define IGMP_INFO_LEN                   (IND_ADDR_LEN + 16)

#define RTSP_DEFAULT_PORT               554

#define REGIST_FD_NUM                   8

#define RTCP_SEQ_NUM                    17
#define ARQ_SEQ_NUM                     10
#define ARQ_SEQ_LEN                     1024

#define RTSP_MAIN_PLAY(rtsp)            (rtsp->open_play && rtsp->index != STREAM_INDEX_PIP)

typedef enum
{
    PLAY_TYPE_SET = 0,
    PLAY_TYPE_NOW,
    PLAY_TYPE_END
}PLAY_TYPE;

typedef enum {
    IPTV_TYPE_UNICAST = 0,
    IPTV_TYPE_MULTICAST,
    IPTV_TYPE_IGMP

}IPTV_TYPE;

typedef enum
{
    RTSP_OP_NONE = 0,
    RTSP_OP_RESOLVE,
    RTSP_OP_CONNECT,
    RTSP_OP_DESCRIBE,
    RTSP_OP_HEARTBIT,
    RTSP_OP_SETUP,
    RTSP_OP_PLAY_INIT,//5
    RTSP_OP_PLAY,
    RTSP_OP_PAUSE,

    //过渡操作
    RTSP_OP_RESUME,
    RTSP_OP_SEEK,
    RTSP_OP_FAST,//10

    RTSP_OP_STOP,//时移转直播

    RTSP_OP_TEARDOWN,

    RTSP_OP_ADVERTISE,
    RTSP_OP_TIMESHIFT,

    RTSP_OP_CACHE_ON,//15
    RTSP_OP_CACHE_OFF,
    RTSP_OP_GET_RET,
    RTSP_OP_GET_RANGE,

    RTSP_OP_NAT,
    RTSP_OP_KEEPLIVE,
    RTSP_OP_BURST_UP,
    RTSP_OP_BURST_DOWN,
    RTSP_OP_ARQ,
    RTSP_OP_RTCPAPP,
	RTSP_OP_MAX
} RTSP_OP;

typedef enum
{
    RTSP_OP_SERVER_NONE = 0,
    RTSP_OP_SERVER_HUAWEI,
    RTSP_OP_SERVER_ZTE,
    RTSP_OP_SERVER_LIVE555,
    RTSP_OP_SERVER_ELECARD,
    RTSP_OP_SERVER_SIMPLE,
} RTSP_OP_SERVER;

typedef enum
{
    RTSP_STATE_TEARDOWN = 0,
    RTSP_STATE_PLAY,
    RTSP_STATE_PAUSE
} RTSP_STATE;

typedef enum
{
    CACHE_STATE_UNSPPORT = 0,
    CACHE_STATE_OFF_INIT,
    CACHE_STATE_OFF,
    CACHE_STATE_ON,
} CACHE_STATE;

typedef enum
{
    RTSP_CMD_OPTION = 0,
    RTSP_CMD_DESCRIBE,
    RTSP_CMD_SETUP,
    RTSP_CMD_PLAY,
    RTSP_CMD_PAUSE,
    RTSP_CMD_TEARDOWN
} RTSP_CMD;

typedef enum
{
    PARAM_X_TIMESHIFT_RANGE = 1,
    PARAM_X_FAST_CACHE_ON,
    PARAM_X_FAST_CACHE_OFF,
    PARAM_X_HEARTBIT_TIME,
} RTSP_PARAM;

typedef enum
{
    OPEN_PLAY_CLOSE = 0,//播放关闭
    OPEN_PLAY_RUNNING,  //正在播放
    OPEN_PLAY_END,      //播放到尾
    OPEN_PLAY_PPVWAIT,  //PPV等待播放
    OPEN_PLAY_TIMESHIFT //本地时移
} OPEN_PLAY;

enum {
    RTSP_CMD_NONE = 0,
    RTSP_CMD_HEARTBIT,

    RTSP_CMD_RTSP_HEARTBIT,
    RTSP_CMD_RTSP_BURST_UP,
    RTSP_CMD_RTSP_BURST_DOWN,
    RTSP_CMD_RTSP_NATHEARTBIT,
    RTSP_CMD_RTSP_ARQ,
    RTSP_CMD_RTSP_RTCPAPP,
};

enum {
    SEEK_END_NONE = 0,
    SEEK_END_NORMAL,
    SEEK_END_DUMMY //seekEnd立即上报结束消息
};

struct RTSP;

typedef int (*RTSP_OP_f)(struct RTSP* rtsp);

struct RTSPOps {
    int         out;//超时时间
    RTSP_OP_f   op_f;
    RTSP_OP_f   ok_f;
    RTSP_OP_f   err_f;
};

struct RegistFd {
    void*       handle;
    int         fd;
    fd_recv_f   recv;
};

struct RTSP
{
    uint32_t        clk;

    STRM_STATE      state;
    int             scale;

    int             skipfast_clks;/* 3560E在播放转快进后出现半屏现象，解决策略是丢弃快进开始的数据 */
    uint32_t        skipfast_clk;

    uint32_t        ply_magic;

    int             open_play;
    int             open_shift;

    uint32_t        shift_id;

    RecordMix_t     rec_mix;

    int             rec_index;
    int             rec_wait;
    uint32_t        rec_wait_id;

    uint32_t        rec_time;

#ifdef INCLUDE_PVR
    int                 rcall_handle;
    RecordCall_open     rcall_open;
    RecordCall_push     rcall_push;
    RecordCall_close    rcall_close;

    PvrElem_t       pvr;
    STRM_STATE      pvr_state;
    int             pvr_scale;
    struct PVRInfo  pvr_info;
    int             pvr_end;
    StrmBuffer*     pvr_sb;

    int             pvr_smooth;//直播下，暂停转播放平滑切换
    int             pvr_smooth_clk;
    int             pvr_smooth_pause;    //暂停时 从直播到暂停继续给解码器送数据
                                        //播放时 时移长度小于2秒，解码器处于暂停状态
#endif

    int         index;
    ind_tlink_t tlink;

    uint32_t    post_clk;//响应开始时间

    int         fcc_flag;
    int         fcc_type;
    void*       fcc_handle;

    int         ret_flag;
    uint32_t    ret_clk_cache_on;
    uint32_t    ret_clk_cache_off;
    uint32_t    ret_cache_iptv;
    void*       ret_handle;

    int         arq_flag;
    int         arq_clk;

    uint32_t    arq_ctc_len;
    char        arq_ctc_seqbuf[ARQ_SEQ_LEN];

    void*       arq_ctc_handle;

    int         iptv_pause;
    int         rtcp_flag;
    uint32_t    rtcp_seq;
    uint32_t    rtcp_clk;
    uint32_t    rtcp_ssrc;
    uint32_t    rtcp_lostseq;
    uint32_t    rtcp_lostnum;
    uint32_t    rtcp_lostblp;
    void*   	rtcp_handle;

    int         burst_flag;
    uint32_t    burst_clk;

    int     iframe_flag;

    int     standard;
    int     transport;

    int     cmdsn;
    int     redirect;

    StreamCmd       cmd_array[CMD_QUEUE_LENGTH];
    StreamCmd*      cmd_pool;
    StreamCmd*      cmd_queue;
    int             cmd_close;

    int             op_server;
    struct RTSPOps  ops[RTSP_OP_MAX];
    RTSP_OP         op;
    uint32_t        op_clk;
    int             op_cseq;
    int             ig_cseq;//ig : ignore

    //seek,fast参数
    uint32_t    op_off;
    PLAY_TYPE   op_type;
    int         op_scale;

    int         retry_flg;
    int         retry_time;
    RTSP_OP     retry_op;
    int         retry_op_off;
    int         retry_op_scale;

    int         clear_flg;

    int         voddelay_level;
    uint32_t    voddelay_clk;

    mid_msgq_t  msgq;
    int         msgfd;
    int         maxfd;
    fd_set      rset;
    fd_set      readset;

    int         loops;

    int         heartbit_period;
    int         heartbit_standard;
    APP_TYPE    apptype;
    int         iptvtype;//0 单播，1 组播（含单播地址），3 纯组播
    int         transtype;//TCP或UDP
    int         trickmode;

    RTSP_STATE  rtsp_state;//RTSP命令状态
    int         rtsp_code;

    //SQM 华为质量监控
    int         postvalid;

    int         bitrate;

    struct StrmRRS  strmRRS;

    char        url[STREAM_URL_SIZE];
    char        channel_url[STREAM_URL_SIZE];
    char        tmshift_url[STREAM_URL_SIZE];
    int         sock;
    uint32_t    port_tcp;
    uint32_t    port_udp;
    char        Session[SESSION_LENGTH];
    int         CSeq;
    int         trackID;
    char        UserAgent[USERAGENT_LENGTH];
    char        ContentBase[STREAM_URL_SIZE];
    int         err_no;
    char        fmt_line[LINE_NUM_2][LINE_LEN_128];

    uint32_t    recv_clk;
    uint32_t    recv_times;
    int         recv_safe;

    int                 adv_num;
    struct Advertise    adv_array[ADVERTISE_NUM];
    int                 adv_insert;
    int                 adv_inserted;

    struct ind_sin      serv_sin;

    struct ind_sin      peername;
    struct ind_sin      sockname;
    struct ind_sin      playname;

    uint32_t            data_port;
    int                 data_sock;
    int                 rtcp_sock;
    uint32_t            data_clk;

    MultiPlayCall       igmp_play_call;
    MultiStopCall       igmp_stop_call;
    char*               igmp[IGMP_INFO_SIZE];
    int                 igmp_size;

    int                 mult_save;
    int                 mult_sock;
    struct ind_sin      mult_sin;
    int                 mult_flag;//1 开始，2 收到过流，3 接收超时
    char                igmp_info[IGMP_INFO_LEN];

    mid_mutex_t         mutex;
    uint32_t            clock;

    //组播超时转单播
    int     multicast_unicast;//组播转单播

    int     multicast_forbid;//禁止RTSP携带组播信息时转组播

    //时宜第二次调度
    uint32_t    timeshift_second;//时移二次调度
    uint32_t    timeshift_status;//是否要切换到时移状态
    uint32_t    timeshift_support;//服务端是否支持时移

    //时间计算
    uint32_t    timeshift_len;//上层指定的时移长度
    uint32_t    timeshift_real;//实际服务器能支持的timeshift长度

    int     fcc_timeout;//FCC组播无流
    int     fcc_times;//FCC写数据次数
    int     fcc_counter;

    int     ctc_begin;
    int     ctc_end;

    uint32_t    ppv_begin;
    uint32_t    ppv_end;

    int         time_buffer;//本地时移时，如果解码器数据过小，加快播放速率

    int         time_begin;
    uint32_t    time_length;
    int         time_fast;
    uint32_t    time_start;
    uint32_t    time_current;

    CACHE_STATE cache;
    int         cache_clk_level;
    int         cache_clk_range;

    int         size_play;
    int         size_cache;

    int         servtime_sync;
    int         servtime_diff;//server clien sub MDN服务器与STB时间的差值
    int         signal_flg;//有无信号
    int         psi_view_play;
    int         psi_view_record;
    int         music_flg;
    int         seek_end;//seek end flag

    uint32_t    clk_full;
    uint32_t    clk_empty;

    RTSPStat    stat;

    StreamPlay*     strm_play;
    StreamRecord*   strm_record;

    StreamMsgQ*     strm_msgq;
    StrmBufOrder*   strm_bufOrder;

    //rtsp_recv.c专用
    char    ts_buf[188];
    int     ts_len;

    char    send_buf[CTRL_BUFFER_LENGTH + 4];
    char    ctrl_buf[CTRL_BUFFER_LENGTH + 4];
    int     ctrl_len;

    int         tcp_match;
    int         tcp_flag;//0 未知，1 不完整数据，2 带RTP头数据，3 数据，4 消息
    ts_buf_t    tcp_buf;
    int         tcp_len;

    int         rtp_flag;

    uint32_t    rtp_seq;
    StrmBuffer* rtp_sb;

    int             regist_magic;
    int             regist_num;
    struct RegistFd regist_fd[REGIST_FD_NUM];

    //NAT穿越的相关参数
    uint32_t    nat_ssrc;
    int         nat_flag;
    int         nat_heartbitperiod;
    char        nat_buf[NAT_BUFFER_LENGTH];

    CallBack_RtspInfo call_rtspinfo;
};

typedef void (*RTSPINTER)(void);
typedef void (*STREAMCALL)(struct RTSP* rtsp);

extern int g_rtsp_sqm_flag;

void rtsp_post_msg(struct RTSP* rtsp, STRM_MSG msgno, int arg);
void rtsp_post_state(struct RTSP* rtsp, STRM_STATE state, int scale);

int rtsp_msg_now(struct RTSP* rtsp, StreamMsg* msg);

StreamCmd* rtsp_cmd_pump(struct RTSP* rtsp);
void rtsp_cmd_free(struct RTSP* rtsp, StreamCmd* msg);
void rtsp_cmd_empty(struct RTSP* rtsp);
void rtsp_msg_back(void *handle, STRM_MSG msgno, int arg);
void rtsp_cmd_queue(struct RTSP* rtsp, StreamCmd* msg);
void rtsp_cmd_queuecmd(struct RTSP* rtsp, int msgno);

void rtsp_clt_cmdback(struct RTSP* rtsp);

void rtsp_clt_state(struct RTSP* rtsp, STRM_STATE state, int scale);
void rtsp_clt_fdset(struct RTSP* rtsp);
void rtsp_clt_reset(struct RTSP* rtsp);
void rtsp_clt_save(void* arg);
int rtsp_clt_parse_iptv(struct RTSP* rtsp, char* channel_url, char* tmshift_url);
int rtsp_clt_parse_url(struct RTSP* rtsp, char* url);
void rtsp_clt_make_url(struct RTSP* rtsp);

void rtsp_clt_fcc_reset(struct RTSP* rtsp);

void rtsp_clt_ret_open(struct RTSP* rtsp);
void rtsp_clt_ret_close(struct RTSP* rtsp);

void rtsp_clt_arq_open(struct RTSP* rtsp);
void rtsp_clt_arq_close(struct RTSP* rtsp);

void rtsp_clt_reset_play(struct RTSP* rtsp, int caReset);
int rtsp_clt_iptv_open(struct RTSP* rtsp);
int rtsp_clt_iptv_close(struct RTSP* rtsp);

void rtsp_clt_resume(struct RTSP* rtsp);

void rtsp_clt_ext_100ms(struct RTSP *rtsp);

void rtsp_clt_period_100ms(void* arg);

void rtsp_clt_heartbit(void* arg);
void rtsp_clt_natheartbit(void* arg);
void rtsp_clt_rtcpapp(void* arg);
int rtsp_clt_trickmode(struct RTSP* rtsp);
void rtsp_clt_shiftcheck(void* arg);
void rtsp_clt_rangeout(void* arg);

int rtsp_clt_datasocket(struct RTSP* rtsp);
int rtsp_clt_rtcpsocket(struct RTSP* rtsp);

int rtsp_clt_send(struct RTSP* rtsp, int len);
void rtsp_clt_close_play(struct RTSP* rtsp);
void rtsp_clt_close_vod(struct RTSP* rtsp);

//post data
void rtsp_clt_post_valid(struct RTSP* rtsp, int valid);

void rtsp_clt_advertise_select(struct RTSP* rtsp, int inserted, int sec);
struct Advertise* rtsp_clt_advertise_elem(struct RTSP* rtsp, int insert);

int rtsp_clt_tstv_calc(struct RTSP* rtsp);

void rtsp_clt_time_1000ms(struct RTSP* rtsp);
void rtsp_clt_time_sync(struct RTSP* rtsp);
uint32_t rtsp_clt_time_server(struct RTSP* rtsp);

void rtsp_clt_nbiosocket(int sock);
void rtsp_clt_time_set_total(struct RTSP* rtsp, uint32_t length);
void rtsp_clt_time_set_current(struct RTSP* rtsp, uint32_t current);
uint32_t rtsp_clt_time_make(char* string);
int rtsp_clt_time_range(struct RTSP* rtsp, char* buf);
int rtsp_clt_time_local(uint32_t sec, char* buf);
char *rtsp_clt_time_fmt(struct RTSP* rtsp, int idx, uint32_t sec);
char *rtsp_clt_addr_fmt(struct RTSP* rtsp, struct ind_sin* sin);
void rtsp_clt_time_print(uint32_t sec, char* buf);

void rtsp_clt_rtcpfb(struct RTSP* rtsp);
void rtsp_clt_rtcp_open(struct RTSP* rtsp);
void rtsp_clt_rtcp_close(struct RTSP* rtsp);

void rtsp_op_init(struct RTSP* rtsp);
void rtsp_op_init_zte(struct RTSP* rtsp);
void rtsp_op_init_huawei(struct RTSP* rtsp);
void rtsp_op_init_live555(struct RTSP* rtsp);
void rtsp_op_none(struct RTSP* rtsp);
void rtsp_op_succeed(struct RTSP* rtsp);
void rtsp_op_failed(struct RTSP* rtsp);

int rtsp_op_open_err(struct RTSP* rtsp);
int rtsp_op_location(struct RTSP* rtsp);

void rtsp_op_resolve(struct RTSP* rtsp, int idx);

int rtsp_op_header(struct RTSP* rtsp, char* method);
int rtsp_op_teardown(struct RTSP* rtsp);
int rtsp_op_teardown_ok(struct RTSP* rtsp);
int rtsp_op_rtcpfb(struct RTSP* rtsp);

void rtsp_op_timeout_reg(struct RTSP* rtsp, RTSP_OP op);
void rtsp_op_immediate_reg(struct RTSP* rtsp, RTSP_OP op);

//rtsp_op_huawei
int rtsp_op_param_set(struct RTSP* rtsp, RTSP_PARAM param);

int rtsp_recv_rtp(struct RTSP* rtsp);
int rtsp_recv_tcp(struct RTSP* rtsp);
int rtsp_recv_nat(struct RTSP* rtsp);
void rtsp_recv_unicast(struct RTSP* rtsp);
void rtsp_recv_multicast(struct RTSP* rtsp);

uint32_t rtsp_alloc_port(void);
void rtsp_clt_set_rtspsock(int sock);

#ifdef __cplusplus
}
#endif

#endif    /* _RTSP_APP_H_ */


