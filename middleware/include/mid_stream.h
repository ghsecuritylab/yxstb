#ifndef __MID_STREAM_H__
#define __MID_STREAM_H__


struct ind_sin;

/*
    播放过程中
        发生状态改变会通过porting函数rtsp_port_state通知应用层
        其他消息通过porting函数rtsp_port_message通知应用层
    PVR引入的接口有
        mid_stream_record_open和mid_stream_record_close
 */




//上海电信播放标准
#define RTSP_STANDARD_CTC_SHANGHAI      101/*0101*/
//广东电信播放标准
#define RTSP_STANDARD_CTC_GUANGDONG     102/*0102*/
//华为私有播放标准
#define RTSP_STANDARD_HUAWEI            200/*0200*/

//华为私有播放标准(默认模式，对接华为最早MDN服务器)
#define RTSP_STANDARD_YUXING            201
//ELECARD
#define RTSP_STANDARD_ELECARD           202

// 联通标准。目前只调了青牛软件这边。
#define RTSP_STANDARD_UNICOM            301

/*
    RTSP 错误代号扩展
    610 分段播放范围错误
 */

typedef enum
{
    STRM_STATE_CLOSE = 0,   //播放退出（mid_stream_close 触发状态改变）
    STRM_STATE_OPEN,        //播放打开状态，此状态为一临时状态（只在播放模块内部使用）
    STRM_STATE_BUFFER,      //缓冲 OTT播放特有状态
    STRM_STATE_PLAY,        //播放
    STRM_STATE_PAUSE,       //暂停
    STRM_STATE_FAST,        //快进快退
    STRM_STATE_RESERVE6,
    STRM_STATE_IPTV,        //iptv播放，单播或组播的其他状态（除CLOSE和ERROR）均为时移
    STRM_STATE_ADVERTISE,   //VOD 广告播放
    STRM_STATE_MAX
} STRM_STATE;

/*
    为兼容现有国内项目
    新增播放失败的部分消息 不单独使用主消息类型，而是作为
    STRM_MSG_NET_ERROR或STRM_MSG_OPEN_ERROR消息的参数
    否则国内项目可能因漏处理某个消息而造成问题
 */
typedef enum
{
    RECORD_CODE_ERROR = 100,    //录制错误

    RECORD_CODE_DISK_ERROR,     //硬盘不存在或错误
    RECORD_CODE_DISK_WARN,      //硬盘告警（硬盘快满）
    RECORD_CODE_DISK_FULL,      //磁盘已满
    RECORD_CODE_REFUSED_SOCKET, //拒绝连接 socket级
    RECORD_CODE_REFUSED_SERVER, //拒绝连接 server级 105
    RECORD_CODE_NOT_FOUND,      //文件未找到
    RECORD_CODE_PVR_CONFLICT,   //PVR 冲突

    RTSP_CODE_No_Content = 203,
    RTSP_CODE_Bad_Request = 400,
    RTSP_CODE_Unauthorized = 401,
    RTSP_CODE_Forbidden = 403,  //认证不通过
    RTSP_CODE_Not_Found = 404,
    RTSP_CODE_Method_Not_Allowed = 405,
    RTSP_CODE_Not_Acceptable = 406,
    RTSP_CODE_Unsupported_Media_Type = 415,
    RTSP_CODE_Parameter_Not_Understood = 451,//服务器不支持的参数
    RTSP_CODE_Session_Not_Found = 454,
    RTSP_CODE_Parameter_Is_Read_Only = 458,
    RTSP_CODE_Method_Not_Valid_In_This_State = 455,
    RTSP_CODE_Unsupported_Transport = 461,
    RTSP_CODE_Internal_Server_Error = 500,
    RTSP_CODE_Not_Implemented = 501,
    RTSP_CODE_Service_Unavailable = 503,
    RTSP_CODE_Version_Not_Supported = 505,
    RTSP_CODE_Option_Not_Supported = 551,

    //IPTV 国内 STB故障代码表.xlsx
    RTSP_CODE_Unknown_Error = 600,      //自定义未知错误
    RTSP_CODE_Server_Bandwidth = 601,   //服务器带宽不足
    RTSP_CODE_Server_Busying = 602,     //服务器忙
    RTSP_CODE_User_Bandwidth = 603,     //接入带宽不足
    RTSP_CODE_User_Unauthorized = 604,  //用户无权限
    RTSP_CODE_User_LackOfFunds = 605,   //用户费用不足
    RTSP_CODE_Unsupported_Media = 606,//STB不支持的格式
    RTSP_CODE_CA_Uninitiated = 607,//CA未初始化等

    RTSP_CODE_Slice_Range_Error = 610,  //时间段错误
    RTSP_CODE_Connect_Timeout = 611,
    RTSP_CODE_URL_FORMAT_Error = 612,
    RTSP_CODE_Connect_Error = 613,
    RTSP_CODE_Timeshift_Error = 614,
    RTSP_CODE_Socket_Error = 615,
    RTSP_CODE_Session_Timeout = 616,
    RTSP_CODE_Parse_Error = 617,
    RTSP_CODE_Play_Timeout = 618,
    RTSP_CODE_Play_Error = 619,
    RTSP_CODE_server_Error = 620,

    PVR_CODE_NOT_FOUND = 630,       //找不到对应的本地文件
    PVR_CODE_FILE_DAMAGE = 631,     //本地文件损坏
    PVR_CODE_BUFFER_FAILED = 633,   //本地文件缓冲失败

    RTSP_CODE_INVALID_RANGE = 701,  //时间段错误 如书签超出播放范围

    DVBS_CODE_TUNER_ERROR = 801,

    RTSP_CODE_Socket_Playing_Error = DVBS_CODE_TUNER_ERROR + 1,
    RTSP_CODE_Socket_Start_Error

} RTSP_CODE;

typedef enum
{
    STRM_MSG_WARN = 1,
    STRM_MSG_OPEN_ERROR,

    STRM_MSG_STREAM_VIEW,   //播放出画面
    STRM_MSG_STREAM_BEGIN,  //播放到头
    STRM_MSG_STREAM_END,    //5 播放到尾
    STRM_MSG_STREAM_MUSIC,  //码流为纯音频

    STRM_MSG_SEEK_BEGIN,    //SEEK到文件开头
    STRM_MSG_SEEK_END,      //SEEK到文件末尾
    STRM_MSG_RECV_FIRST,    //第一次收到数据
    STRM_MSG_RECV_TIMEOUT,  //10 数据接收超时 IPTV STB V100R002C20设计规格.pdf 断流
    STRM_MSG_RECV_RESUME,   //数据接收超时后恢复正常

    STRM_MSG_STREAM_VIDEO,  //码流有视频
    STRM_MSG_PPV_END,

    STRM_MSG_UNSUPPORT_OP,      //不支持操作（一般指不支持时移，播放不支持时移频道，收到上层的时移操作时，返回给上层说明不支持时移操作）
    STRM_MSG_UNSUPPORT_MEDIA,   //15 不支持格式
    STRM_MSG_DECODER_ERROR,     //解码失败
    STRM_MSG_TSTV_CONFLICT,     //TSTV 冲突
    STRM_MSG_TSTV_ERROR,        //TSTV 失败

    STRM_MSG_RESERVE19,
    STRM_MSG_RESERVE20,     //20 停止播放，暂时没用

    //播放正在下载文件消息
    STRM_MSG_BUFFER_BEGIN,  //如果数据不足8秒，播放自动暂停，上报开始缓冲消息
    STRM_MSG_BUFFER_END,    //数据缓冲达到8秒，播放自动恢复，上报结束缓冲消息

    STRM_MSG_ADVERTISE_BEGIN,
    STRM_MSG_ADVERTISE_END,

    STRM_MSG_INDEX_DAMAGE,  //25 索引文件损坏，不能trickmode

    STRM_MSG_PSI_ERROR,     //15秒内PSI信息错误大于3次
    STRM_MSG_PSI_RESUME,    //15秒内PSI信息无错误
    STRM_MSG_RECV_TIMEOUT15,

    STRM_MSG_PACKETS_LOST = 50,

    STRM_MSG_RESERVE51,
    STRM_MSG_RESERVE52,

    STRM_MSG_PTS_VIEW,      //参数为收到数据到显示第一帧的时间计算

    //60 ~ 89 为内部消息，上层不用考虑处理
    STRM_MSG_PLAY_ERROR = 60,
    STRM_MSG_FIRST_TIMEOUT,
    STRM_MSG_FREEZE,
    STRM_MSG_BUF_FULL,
    STRM_MSG_BUF_EMPTY,
    STRM_MSG_CHANGE_PSI,
    STRM_MSG_CHANGE_CRC,

    STRM_MSG_PSI_VIEW,

    STRM_MSG_OVERFLOW,
    STRM_MSG_UNDERFLOW,

    RECORD_MSG_PSI_VIEW = 90,
    RECORD_MSG_DISK_DETACHED,
    RECORD_MSG_DATA_TIMEOUT,
    RECORD_MSG_DATA_RESUME,

    RECORD_MSG_ERROR = 100,     //录制错误

    RECORD_MSG_DISK_ERROR,      //硬盘不存在或错误
    RECORD_MSG_DISK_WARN,       //硬盘告警（硬盘快满）
    RECORD_MSG_DISK_FULL,       //磁盘已满
    RECORD_MSG_REFUSED_SOCKET,  //拒绝连接 socket级
    RECORD_MSG_REFUSED_SERVER,  //拒绝连接 server级 105
    RECORD_MSG_NOT_FOUND,       //文件未找到
    RECORD_MSG_PVR_CONFLICT,    //PVR 冲突

    RECORD_MSG_SUCCESS_BEGIN = 110,//录制成功开始
    RECORD_MSG_SUCCESS_END,     //录制成功结束

    RECORD_MSG_CLOSE,

    RECORD_MSG_DATA_DAMAGE, //录制数据损坏
    RECORD_MSG_NET_TIMEOUT, //网络超时
    RECORD_MSG_NOT_ENOUGH,  //HTTP下载时，磁盘空间不够存放整个文件
    RECORD_MSG_BANDWIDTH,
    RECORD_MSG_FORBID,

    MOSAIC_MSG_ERROR = 120, //马赛克错误
    MOSAIC_MSG_ERROR_RECT,
    MOSAIC_MSG_ERROR_PID,
    MOSAIC_MSG_SUCCESS,
    MOSAIC_MSG_TIMEOUT,
    MOSAIC_MSG_RESUME,

    FLASH_MSG_ERROR = 130,  //FLASH错误
    FLASH_MSG_SUCCESS,

    //IPTV STB V100R002C20 附录5 STB与EPG约定的虚拟事件.pdf
    HLS_MSG_BUFFER_BEGIN = 200, //104061
    HLS_MSG_BUFFER_END,         //104062
    HLS_MSG_BUFFER_LIMITED,     //104065 播放过程中网络带宽不足，缓存数据降到 15 秒的警界线以内，且连续 12 秒在警界线之内
    HLS_MSG_BUFFER_DISKERROR,   //104064 初始播放机顶盒判断硬盘空间不足以缓存一个完整节目
    HLS_MSG_BUFFER_BANDWIDTH,   //104063 缓存期间的平均下载速度如果小于播放速度

    //隋大文维护
    STRM_MSG_BANDWIDTH_LOW,     //add for hls 205~212
    STRM_MSG_NOT_FOUND,
    STRM_MSG_BITRATE_CHANEGE,
    STRM_MSG_RECV_TIMEOUT_ERROR,//数据接收超时       10
    STRM_MSG_RECV_TIMEOUT_PLAY, //数据接收超时       10
    STRM_MSG_SUCCESS_OPEN,
    STRM_MSG_SUCCESS_CLOSE,
    STRM_MSG_BUF_RECV,           //add for hls end

    STRM_MSG_RECV_IGMP_TIMEOUT = STRM_MSG_BUF_RECV + 1,
    STRM_MSG_PLAY_RECV_RTSP_TIMEOUT,
    STRM_MSG_START_RECV_RTSP_TIMEOUT,
    STRM_MSG_RECV_IGMP_RESUME,
    STRM_MSG_RECV_RTSP_RESUME

} STRM_MSG;

/* IPTV 国内 STB故障代码表.xlsx
10200    STRM_MSG_RECV_TIMEOUT arg = 1  加入组播组后无流下发，包括进入组播频道时和在组播频道播放过程中
10201    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_URL_FORMAT_Error频道地址或节目URL地址为空或格式错误
10202    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_Connect_Timeout
                             或 arg = RTSP_CODE_Connect_Timeout STB与流媒体服务器无法建立连接（socket连接失败）
10203    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_Session_Timeout STB可以连接流媒体服务器，但信令交互超时，致无法进入收流播放阶段
10204    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_Parse_Error     在进入媒体流播放前的协商阶段，机顶盒无法解析流媒体服务器响应的信令
10205    STRM_MSG_RECV_TIMEOUT  arg = 0                         初始播放时，流媒体服务器连接交互正常，但并未收到媒体流
10206    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_Play_Error      机顶盒发送播放控制命令失败。显示方式为小窗口，在屏幕中间展示5秒后自动消失
10207    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_Play_Timeout    机顶盒发送播放控制命令，流媒体服务器超时无响应。显示方式为小窗口，在屏幕中间展示5秒后自动消失
10208    STRM_MSG_PACKETS_LOST                                  流媒体播放时发生严重丢包现象，连续2个采样周期丢包率不小于1% 【注】：1、只适用纠错后的RTP/UDP流；2、采样周期与 QOS 规范中的规定保持一致，为 10s；3、显示方式为小窗口，在屏幕中间展示5秒后自动消失。
10209    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_Socket_Error    播放过程中STB与流媒体服务器连接中断导致媒体流中断
10087    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_server_Error    server关闭了rtsp，但媒体流没中断。
 */

typedef enum
{
    ASYNC_MSG_MOUNT_FAILED = 101,
    ASYNC_MSG_MOUNT_SUCCESS,
    ASYNC_MSG_UNMOUNT_FINISH,
    //ASYNC_MSG_DELETE_PROGRESS,
} ASYNC_MSG;

typedef enum
{
    APP_TYPE_VOD = 0,   //VOD播放
    APP_TYPE_VODADV,
    APP_TYPE_IPTV,      //频道播放 APP_TYPE_UNICAST，APP_TYPE_MULTICAST和APP_TYPE_IGMP合并而成
    APP_TYPE_IPTV2,     //直播url 和 时移url分离。用该类型播放后，mid_stream_get_apptype获取的值依然为APP_TYPE_IPTV
                        //mid_stream_open 的第二个参数要传递一个结构体IPTVUrl指针。
    APP_TYPE_RESERVE4,
    APP_TYPE_HTTP,      //5
    APP_TYPE_PVR,       //录制文件播放
    APP_TYPE_AUDIO,
    APP_TYPE_TSTV,      // 支持本地时移频道播放
    APP_TYPE_DVBS,
    APP_TYPE_MOSAIC,    //10 码赛克
    APP_TYPE_FLASH,     //FLASH
    APP_TYPE_DISK,      //磁盘文件播放
    APP_TYPE_RTP2TS,
    APP_TYPE_RESERVE14,
    APP_TYPE_MIX_PCM,   //15
    APP_TYPE_MIX_MP3,
    APP_TYPE_HTTP_PCM,
    APP_TYPE_HTTP_MP3,
    APP_TYPE_ZEBRA,
    APP_TYPE_ZEBRA_PCM, //20
    APP_TYPE_HTTP_MPA,  //支持 mp3,mp2,aac
    APP_TYPE_HLS,       //Http live streaming
    APP_TYPE_HTTP_LIVE, //Http live streaming 隋大文维护
    APP_TYPE_APPLE_VOD,
    APP_TYPE_APPLE_IPTV,
    APP_TYPE_RESERVE26,
    APP_TYPE_MAX
} APP_TYPE;


//丢包统计，电信需求（上海，四川）
enum {
    PcketsLost_Flag_VOD = 0,
    PcketsLost_Flag_MULT,
    PcketsLost_Flag_ARQ,
    PcketsLost_Flag_FEC
};

#define ADVERTISE_NUM       16

#define STREAM_URL_SIZE     2048

#define TIMESHIFT_OFF       0
#define TIMESHIFT_AUTO     -1

struct MixPCM {
    int sampleRate;
    int bitWidth;
    int channels;
};

struct Advertise
{
    int insert;
    char url[STREAM_URL_SIZE];
    int apptype;
};

struct VODAdv
{
    char url[STREAM_URL_SIZE];
    int adv_num;
    struct Advertise adv_array[ADVERTISE_NUM];
};

struct IPTVUrl {
    char channel_url[STREAM_URL_SIZE];
    char tmshift_url[STREAM_URL_SIZE];
};


#ifdef __cplusplus
extern "C" {
#endif

//打印编译时间
void mid_stream_buildtime(void);

/*
    播放初始化
    num：播放实例，取值范围为
        1    一路播放
        2    支持PIP
 */
void mid_stream_init(int num);

void mid_stream_regist_mix(void);
void mid_stream_regist_test(void);
void mid_stream_regist_voole(void);//优朋
void mid_stream_regist_zebra(void);


/**
    设置本次播放语言
**/
#define mid_stream_set_language mid_stream_set_language_v1
void mid_stream_set_language(char* language);

/*
    设置RTSP User-Agent
*/
void mid_stream_set_useragent(char *useragent);

/*
    去掉了
    mid_stream_fcc 和 mid_stream_ret
    增加了
    mid_stream_set_fcc 用于设置播放节目属性
    mid_record_set_fcc 用于设置录制节目属性
 */
void mid_stream_set_fcc(int idx, int fcc_type);
/* 禁止上报数据 */
void mid_stream_set_sqm(int flag);
/* 禁止上报消息 */
void mid_stream_set_msg(int flag);

typedef int     (*MultiPlayCall)(void* igmp, int igmp_size, struct ind_sin *mult_sin);
typedef void    (*MultiStopCall)(void* igmp, int igmp_size, int sock, struct ind_sin *mult_sin);

#define mid_stream_set_igmp mid_stream_set_igmp_v1
void mid_stream_set_igmp(int idx, MultiPlayCall play_call, MultiStopCall stop_call, void* igmp, int igmp_size);

void mid_stream_ret_cache(int flag);//启用SQA库管理cache

void mid_stream_set_arq(int flag);
//流量控制
void mid_stream_set_burst(int flag);
/*
    低于on开启快速缓冲，-1表示关闭快速缓冲
    高于off关闭快速缓冲
    on off 的单位为毫秒，
    默认情况下
    高清 on=1500 off=2000
    标清 on=3000 off=5000
 */
void mid_stream_cache(int on, int off);

void mid_stream_cache_size(int on, int off);

/*
    VOD积累一定数据量才开始播放
 */
void mid_stream_vodlevel(int ms);

typedef enum
{
    PUSH_TYPE_REALTIME = 0,
    PUSH_TYPE_PERIOD100,
    PUSH_TYPE_PERIOD50,
    PUSH_TYPE_PERIOD20,
} PUSH_TYPE;
void mid_stream_push_type(int type);

/*
    tcp 方式下，缓冲满后不接收数据
 */
void mid_stream_recv_safe(int safe);

//I帧播放，该函数只做测试使用
void mid_stream_iframe(int flag);

typedef enum {
    RTSP_TRANSPORT_TCP = 0,
    RTSP_TRANSPORT_UDP = 1,
    RTSP_TRANSPORT_RTP_TCP = 2,
    RTSP_TRANSPORT_RTP_UDP = 3
} RTSP_STANDARD;
/*
    transport 取值意义如下
    对于 RTSP_STANDARD_YUXING(兼容旧MDN) 依然沿用 0: udp 1 tcp 均不支持RTP头
    对于 RTSP_STANDARD_HUAWEI\RTSP_STANDARD_CTC_SHANGHAI和RTSP_STANDARD_CTC_GUANGDONG遵循如下定义
        0:
            TCP, RTP/TCP, UDP, RTP/UDP
        1:
            UDP, RTP/UDP, TCP, RTP/TCP
        2:
            RTP/TCP, TCP, UDP, RTP/UDP
        3:
            RTP/UDP, UDP, TCP, RTP/TCP
 */
void mid_stream_transport(int transport);

void mid_stream_skipfast(int ms);


/*
    设置RTSP播放遵循的标准
 */
void mid_stream_standard(int standard);

/*
    使能或取消时宜二次调度
 */
void mid_stream_timeshift_second(int enable);

/*
    解码器缓冲过多会减慢音轨切换。可以通过下面设置来保证解码器缓冲维持在一定水位。
 */
#define mid_stream_timeshift_level mid_stream_timeshift_level_v1
void mid_stream_timeshift_level(int level);

/*
    暂停时间到达做断点自动转播放
 */
void mid_stream_timeshift_jump(int jump);

/*
    打开或关闭本地时移录制
    id 为频道号或频道key，具体由上层自定义
    id 取值范围为 0 ~ 999，该值可通过mid_stream_timeshift_getid 获取到
    id 为 -1 表示自动录制
    v1 返回值修改为void
 */
#define mid_stream_timeshift_open mid_stream_timeshift_open_v1
void mid_stream_timeshift_open(void);
void mid_stream_timeshift_close(void);

/*
    使能或取消组播超时转单播
    0 默认模式
    1 组播转单播
    2 组播转单播 但是遇到一频道5秒数据接收超时时，自动转为单播优先
    3 单播优先
 */
void mid_stream_multicast_unicast(int enable);
/*
 */
void mid_stream_multicast_forbid(int forbid);

/*
    设置心跳周期
    interval 单位为秒（默认是30s）
 */
void mid_stream_heartbit_period(int interval);

/*
    华为私有规范默认采用OPTIONS作为心跳
    如果standard传 1 则强制采用标准GET_PARAMETER方式心跳
 */
void mid_stream_heartbit_standard(int standard);

/*
 */
void mid_stream_heartbit_active(void);

/*
    设置播放窗口大小
 */
void mid_stream_rect(int idx, int x, int y, int width, int height);

/*
    设置rtsp的TCP端口
 */
void mid_stream_port(int idx, int port_tcp, int port_udp);

/*
    丢弃标清VOD播放开始ms收到的数据
    v1 去掉参数 int idx
 */
#define mid_stream_skip mid_stream_skip_v1
void mid_stream_skip(int ms);

/*
    设置从解析出CA，到CA key设置成功的等待超时
    v1 去掉参数 int idx
 */
#define mid_stream_cawait mid_stream_cawait_v1
void mid_stream_cawait(int ms);

/*
重置大窗口播放缓冲大小，默认值是
#if SUPPORTE_HD == 1
#define SIZE_CACHE        (1024 * 8 * 1316)
#define SIZE_PLAY         (1024 * 3 * 1316)
#else
#define SIZE_CACHE        (1024 * 3 * 1316)
#define SIZE_PLAY         (1024 * 1 * 1316)
#endif
 */
void mid_stream_set_size(int playSize, int cacheSize);

void mid_stream_set_pcr(int flag);

/*
    reset后重新播放，延迟一段时间才开始播
 */
void mid_stream_vodsync(int ms);

/*
    VOD延迟一段时间才开始播放
    v1 去掉参数 int idx
 */
#define mid_stream_voddelay mid_stream_voddelay_v1
void mid_stream_voddelay(int ms);
void mid_stream_iptvdelay(int ms);

typedef void (*StrmStateCall)(int idx, STRM_STATE state, int rate, unsigned int magic, int callarg);
typedef void (*StrmMsgCall)(int idx, STRM_MSG msg, int arg, unsigned int magic, int callarg);
void mid_stream_set_call(int idx, StrmStateCall statecall, StrmMsgCall msgcall, int callarg);

typedef void (*StrmPrivCall)(int msg, int arg, unsigned int magic, int privarg);
void mid_stream_set_privcall(int idx, APP_TYPE atype, StrmPrivCall msgcall, int callarg);


/*
 */
unsigned int mid_stream_magic(int index);

/*
    打开并开始播放
    url：播放地址
        对录制文件播放（APP_TYPE_PVR）url就是录制ID字符串形式
    atype：播放类型
        对于支持本地时移的频道播放，APP_TYPE必须为APP_TYPE_PVRCAST
    shiftlen：
        VOD情况下代表播放开始时间starttime
        IPTV情况下代表时移长度
            > 0 按上层指定时移长度播放
            = 0 （TIMESHIFT_OFF）禁止时移
            = -1 （TIMESHIFT_AUTO）自动按服务器支持的最大时移长度播放
            对于支持本地时移的频道播放，按目前华为规定，此值为1800，也就是30分钟。

        对于APP_TYPE_HTTP_MP3和APP_TYPE_HTTP_MPA，shiftlen为1表示循环播放

    返回值：
        每次播放，都有一个唯一的整型值做标记，该值等于
        stream_port_state和stream_port_message的最后一个参数magic
        上层可以这个参数来区别是那次播放
        下面三个函数的返回值意义相同
 */
unsigned int mid_stream_open(int idx, const char* url, APP_TYPE atype, int shiftlen);
//列表播放（VOD加广告）
unsigned int mid_stream_open_vodAdv(int idx, struct VODAdv* list, int starttime);

/*
    等待播放真正开始
 */
int mid_stream_sync(int idx, int msec);

/*
    打开并开始播放
    VOD情况下
        普通播放：begin 等于 -1
        分段播放：begin >= 0，应用层遇到begin < 0的分段播放情况时，begin 传 0
    IPTV情况下 begin 目前无实际意义
 */
unsigned int mid_stream_open_range(int idx, const char* url, APP_TYPE apptype, int shiftlen, int begin, int end);

/*
    停止并关闭播放
    clear等于1 立即清除画面
 */
void mid_stream_close(int idx, int clear);


/*
    停止时移播放，转直播
 */
void mid_stream_stop(int index);
/*
    暂停播放
 */
void mid_stream_pause(int index);
/*
    从暂停状态或快捷快退状态 恢复到 正常播放
 */
void mid_stream_resume(int index);
/*
    快进或快退播放
    level 取值范围为:
        -32, -16, -8, -4, -2 快退
        32, 16, 8, 4, 2 快进
 */
void mid_stream_fast(int idx, int level);
/*
    跳跃到新位置(SEEK)播放
 */
void mid_stream_seek(int idx, int second);

/*
 */
void mid_stream_lseek(int idx, long long offset);

/*
    获取播放总类型
 */
APP_TYPE mid_stream_get_apptype(int index);

/*
    获取播放总长度，对于IPTV播放总长度等于时移长度
 */
unsigned int mid_stream_get_totaltime(int index);
void mid_stream_sync_totaltime(int idx, int msec);
/*
    获取当前播放时间
 */
unsigned int mid_stream_get_currenttime(int index);

long long mid_stream_get_totalbyte(int index);
long long mid_stream_get_currentbyte(int index);

/*
    计算音乐文件的总长，目前只支持mp3
 */
int mid_stream_auido_length(char* filepath);


int mid_stream_mix_space(int idx, unsigned int magic);
int mid_stream_mix_push(int idx, unsigned int magic, char* buf, int len);

//msgcall回调时，第一个参数是key。该函数要在mid_stream_mosaic_open之前调用
void mid_stream_mosaic_setcall(int key, StrmMsgCall msgcall, int callarg);
//马赛克播放
int mid_stream_mosaic_open(int key, const char* url, int x, int y, int width, int height, char* igmp, int igmp_size);
void mid_stream_mosaic_close(int key);

void mid_stream_mosaic_save(int flag);
void mid_stream_mosaic_mute(int mute);
void mid_stream_mosaic_set(int key);
int mid_stream_mosaic_get(void);
/*
  返回hls数据缓存百分比
*/
int mid_stream_hls_buffrate(void);
unsigned int mid_stream_hls_recordrate(void);

/*
    设置APP_TYPE_HTTP_MPA播放的cookie
 */
void mid_stream_hmpa_cookie(int idx, char* cookie);

/*
    设置HTTP下载播放华为方式解密key
 */
void mid_stream_http_pvrkey(int idx, long long key);
/*
    设置HTTP下载gzip解压函数
 */
typedef int (* StrmHttpUnzip)(char* srcbuf, int srclen, char* dstbuf, int* dstlen);
void mid_stream_http_unzip(StrmHttpUnzip httpUnzip);

void mid_stream_set_tuner(int idx, int tuner);

/*
    如果epg下发过EMM数据，在当次播放前要调用该函数通知播放
 */
#define mid_stream_emm mid_stream_emm_v1
void mid_stream_emm(void);

/*
    设置丢包统计周期
    interval：单位为秒
 */
void mid_stream_statint_pklosts(int interval);
/*
    设置码率统计周期
    interval：单位为秒
 */
void mid_stream_statint_bitrate(int interval);

/*
    检查缓冲上下溢(四川局点要求)
    picwidth:
        <= 720 标清
        >  720 高清
    flow:
         1 上溢
        -1 下溢
 */
typedef void (* CallBack_StatFlow)(int multflg, int picwidth, int flow);
void mid_stream_callback_statflow(CallBack_StatFlow statflow);

typedef void (* CallBack_RtspInfo)(int multflg, char *info_buf, int info_len);
void mid_stream_callback_rtspinfo(CallBack_RtspInfo rtspinfo);

void mid_stream_nat(int mode);
void mid_stream_nat_heartbitperiod(int time);

/*
    设置RRS连接超时时间
    C58基线US 迭代计划-20120227.xlsx
    1、建议改为每次依次遍历每个RRS。
    2、请求没有响应2s认为该RRS故障。
 */
void mid_stream_rrs_timeout(int ms);

void mid_stream_get_rtspInfo(int idx, char *url, char *method);

void mid_stream_set_apple_buffersize(int size);
//测试使用
void mid_stream_set_apple_bandwidth(int bandwidth);
void mid_stream_set_apple_level(int level);

/*
    AudioBitRate    范围 0 ~ AudioChannels - 1
    AudioChannels
    CurBufferSize
    DownloadRate
    PacketLost 单位为千分之一
    Playrate
    StreamBandwith  范围 0 ~ StreamNum - 1
    StreamNum
    SegmentNum
    RemainPlaytime
    ToalBufferSize
    TransportProtocol
 */
int mid_stream_getInt(char *paramName, int arg);

/*
    CurSegment
    SegmentList     范围 0 ~ SegmentNum - 1
    StreamURL       范围 0 ~ StreamNum - 1
 */
void mid_stream_getString(char *paramName, int arg, char *buf, int size);

typedef void (*TSSyncNotifyCall)(void);
/*
    TS同步出错上报
    interval 上报周期，单位为秒
 */
void mid_stream_set_tsSyncNotify(unsigned int interval, TSSyncNotifyCall notifyCall);

unsigned int mid_stream_hls_playrate(void);

typedef void (StreamRouteCall)(unsigned int addr);
void mid_stream_setRouteCallback(StreamRouteCall func);

/*
    addr: IPTV V1R3C56 特性设计-SQA设计规格V1.6.doc 4.6.3主场景流程描述 3、UE从频道信息列表中任选一个频道，从其URL中解析出RRS的IP地址。
    port: sysSettingGetInt("fcc_port", &port, 0);
*/
void mid_stream_setRRS(unsigned int addr, int port);

/******************************测试函数**************************************/
/*
    flag：1 开始保存，0 停止保存
 */
void mid_stream_save(int flag);
/*
    bandwidth：http下载带宽限制
 */
void mid_stream_bandwidth(int bandwidth);

#ifdef __cplusplus
}
#endif

#endif /* __MID_STREAM_H__ */
