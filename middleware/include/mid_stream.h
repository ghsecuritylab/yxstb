#ifndef __MID_STREAM_H__
#define __MID_STREAM_H__


struct ind_sin;

/*
    ���Ź�����
        ����״̬�ı��ͨ��porting����rtsp_port_state֪ͨӦ�ò�
        ������Ϣͨ��porting����rtsp_port_message֪ͨӦ�ò�
    PVR����Ľӿ���
        mid_stream_record_open��mid_stream_record_close
 */




//�Ϻ����Ų��ű�׼
#define RTSP_STANDARD_CTC_SHANGHAI      101/*0101*/
//�㶫���Ų��ű�׼
#define RTSP_STANDARD_CTC_GUANGDONG     102/*0102*/
//��Ϊ˽�в��ű�׼
#define RTSP_STANDARD_HUAWEI            200/*0200*/

//��Ϊ˽�в��ű�׼(Ĭ��ģʽ���Խӻ�Ϊ����MDN������)
#define RTSP_STANDARD_YUXING            201
//ELECARD
#define RTSP_STANDARD_ELECARD           202

// ��ͨ��׼��Ŀǰֻ������ţ�����ߡ�
#define RTSP_STANDARD_UNICOM            301

/*
    RTSP ���������չ
    610 �ֶβ��ŷ�Χ����
 */

typedef enum
{
    STRM_STATE_CLOSE = 0,   //�����˳���mid_stream_close ����״̬�ı䣩
    STRM_STATE_OPEN,        //���Ŵ�״̬����״̬Ϊһ��ʱ״̬��ֻ�ڲ���ģ���ڲ�ʹ�ã�
    STRM_STATE_BUFFER,      //���� OTT��������״̬
    STRM_STATE_PLAY,        //����
    STRM_STATE_PAUSE,       //��ͣ
    STRM_STATE_FAST,        //�������
    STRM_STATE_RESERVE6,
    STRM_STATE_IPTV,        //iptv���ţ��������鲥������״̬����CLOSE��ERROR����Ϊʱ��
    STRM_STATE_ADVERTISE,   //VOD ��沥��
    STRM_STATE_MAX
} STRM_STATE;

/*
    Ϊ�������й�����Ŀ
    ��������ʧ�ܵĲ�����Ϣ ������ʹ������Ϣ���ͣ�������Ϊ
    STRM_MSG_NET_ERROR��STRM_MSG_OPEN_ERROR��Ϣ�Ĳ���
    ���������Ŀ������©����ĳ����Ϣ���������
 */
typedef enum
{
    RECORD_CODE_ERROR = 100,    //¼�ƴ���

    RECORD_CODE_DISK_ERROR,     //Ӳ�̲����ڻ����
    RECORD_CODE_DISK_WARN,      //Ӳ�̸澯��Ӳ�̿�����
    RECORD_CODE_DISK_FULL,      //��������
    RECORD_CODE_REFUSED_SOCKET, //�ܾ����� socket��
    RECORD_CODE_REFUSED_SERVER, //�ܾ����� server�� 105
    RECORD_CODE_NOT_FOUND,      //�ļ�δ�ҵ�
    RECORD_CODE_PVR_CONFLICT,   //PVR ��ͻ

    RTSP_CODE_No_Content = 203,
    RTSP_CODE_Bad_Request = 400,
    RTSP_CODE_Unauthorized = 401,
    RTSP_CODE_Forbidden = 403,  //��֤��ͨ��
    RTSP_CODE_Not_Found = 404,
    RTSP_CODE_Method_Not_Allowed = 405,
    RTSP_CODE_Not_Acceptable = 406,
    RTSP_CODE_Unsupported_Media_Type = 415,
    RTSP_CODE_Parameter_Not_Understood = 451,//��������֧�ֵĲ���
    RTSP_CODE_Session_Not_Found = 454,
    RTSP_CODE_Parameter_Is_Read_Only = 458,
    RTSP_CODE_Method_Not_Valid_In_This_State = 455,
    RTSP_CODE_Unsupported_Transport = 461,
    RTSP_CODE_Internal_Server_Error = 500,
    RTSP_CODE_Not_Implemented = 501,
    RTSP_CODE_Service_Unavailable = 503,
    RTSP_CODE_Version_Not_Supported = 505,
    RTSP_CODE_Option_Not_Supported = 551,

    //IPTV ���� STB���ϴ����.xlsx
    RTSP_CODE_Unknown_Error = 600,      //�Զ���δ֪����
    RTSP_CODE_Server_Bandwidth = 601,   //������������
    RTSP_CODE_Server_Busying = 602,     //������æ
    RTSP_CODE_User_Bandwidth = 603,     //���������
    RTSP_CODE_User_Unauthorized = 604,  //�û���Ȩ��
    RTSP_CODE_User_LackOfFunds = 605,   //�û����ò���
    RTSP_CODE_Unsupported_Media = 606,//STB��֧�ֵĸ�ʽ
    RTSP_CODE_CA_Uninitiated = 607,//CAδ��ʼ����

    RTSP_CODE_Slice_Range_Error = 610,  //ʱ��δ���
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

    PVR_CODE_NOT_FOUND = 630,       //�Ҳ�����Ӧ�ı����ļ�
    PVR_CODE_FILE_DAMAGE = 631,     //�����ļ���
    PVR_CODE_BUFFER_FAILED = 633,   //�����ļ�����ʧ��

    RTSP_CODE_INVALID_RANGE = 701,  //ʱ��δ��� ����ǩ�������ŷ�Χ

    DVBS_CODE_TUNER_ERROR = 801,

    RTSP_CODE_Socket_Playing_Error = DVBS_CODE_TUNER_ERROR + 1,
    RTSP_CODE_Socket_Start_Error

} RTSP_CODE;

typedef enum
{
    STRM_MSG_WARN = 1,
    STRM_MSG_OPEN_ERROR,

    STRM_MSG_STREAM_VIEW,   //���ų�����
    STRM_MSG_STREAM_BEGIN,  //���ŵ�ͷ
    STRM_MSG_STREAM_END,    //5 ���ŵ�β
    STRM_MSG_STREAM_MUSIC,  //����Ϊ����Ƶ

    STRM_MSG_SEEK_BEGIN,    //SEEK���ļ���ͷ
    STRM_MSG_SEEK_END,      //SEEK���ļ�ĩβ
    STRM_MSG_RECV_FIRST,    //��һ���յ�����
    STRM_MSG_RECV_TIMEOUT,  //10 ���ݽ��ճ�ʱ IPTV STB V100R002C20��ƹ��.pdf ����
    STRM_MSG_RECV_RESUME,   //���ݽ��ճ�ʱ��ָ�����

    STRM_MSG_STREAM_VIDEO,  //��������Ƶ
    STRM_MSG_PPV_END,

    STRM_MSG_UNSUPPORT_OP,      //��֧�ֲ�����һ��ָ��֧��ʱ�ƣ����Ų�֧��ʱ��Ƶ�����յ��ϲ��ʱ�Ʋ���ʱ�����ظ��ϲ�˵����֧��ʱ�Ʋ�����
    STRM_MSG_UNSUPPORT_MEDIA,   //15 ��֧�ָ�ʽ
    STRM_MSG_DECODER_ERROR,     //����ʧ��
    STRM_MSG_TSTV_CONFLICT,     //TSTV ��ͻ
    STRM_MSG_TSTV_ERROR,        //TSTV ʧ��

    STRM_MSG_RESERVE19,
    STRM_MSG_RESERVE20,     //20 ֹͣ���ţ���ʱû��

    //�������������ļ���Ϣ
    STRM_MSG_BUFFER_BEGIN,  //������ݲ���8�룬�����Զ���ͣ���ϱ���ʼ������Ϣ
    STRM_MSG_BUFFER_END,    //���ݻ���ﵽ8�룬�����Զ��ָ����ϱ�����������Ϣ

    STRM_MSG_ADVERTISE_BEGIN,
    STRM_MSG_ADVERTISE_END,

    STRM_MSG_INDEX_DAMAGE,  //25 �����ļ��𻵣�����trickmode

    STRM_MSG_PSI_ERROR,     //15����PSI��Ϣ�������3��
    STRM_MSG_PSI_RESUME,    //15����PSI��Ϣ�޴���
    STRM_MSG_RECV_TIMEOUT15,

    STRM_MSG_PACKETS_LOST = 50,

    STRM_MSG_RESERVE51,
    STRM_MSG_RESERVE52,

    STRM_MSG_PTS_VIEW,      //����Ϊ�յ����ݵ���ʾ��һ֡��ʱ�����

    //60 ~ 89 Ϊ�ڲ���Ϣ���ϲ㲻�ÿ��Ǵ���
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

    RECORD_MSG_ERROR = 100,     //¼�ƴ���

    RECORD_MSG_DISK_ERROR,      //Ӳ�̲����ڻ����
    RECORD_MSG_DISK_WARN,       //Ӳ�̸澯��Ӳ�̿�����
    RECORD_MSG_DISK_FULL,       //��������
    RECORD_MSG_REFUSED_SOCKET,  //�ܾ����� socket��
    RECORD_MSG_REFUSED_SERVER,  //�ܾ����� server�� 105
    RECORD_MSG_NOT_FOUND,       //�ļ�δ�ҵ�
    RECORD_MSG_PVR_CONFLICT,    //PVR ��ͻ

    RECORD_MSG_SUCCESS_BEGIN = 110,//¼�Ƴɹ���ʼ
    RECORD_MSG_SUCCESS_END,     //¼�Ƴɹ�����

    RECORD_MSG_CLOSE,

    RECORD_MSG_DATA_DAMAGE, //¼��������
    RECORD_MSG_NET_TIMEOUT, //���糬ʱ
    RECORD_MSG_NOT_ENOUGH,  //HTTP����ʱ�����̿ռ䲻����������ļ�
    RECORD_MSG_BANDWIDTH,
    RECORD_MSG_FORBID,

    MOSAIC_MSG_ERROR = 120, //�����˴���
    MOSAIC_MSG_ERROR_RECT,
    MOSAIC_MSG_ERROR_PID,
    MOSAIC_MSG_SUCCESS,
    MOSAIC_MSG_TIMEOUT,
    MOSAIC_MSG_RESUME,

    FLASH_MSG_ERROR = 130,  //FLASH����
    FLASH_MSG_SUCCESS,

    //IPTV STB V100R002C20 ��¼5 STB��EPGԼ���������¼�.pdf
    HLS_MSG_BUFFER_BEGIN = 200, //104061
    HLS_MSG_BUFFER_END,         //104062
    HLS_MSG_BUFFER_LIMITED,     //104065 ���Ź�������������㣬�������ݽ��� 15 ��ľ��������ڣ������� 12 ���ھ�����֮��
    HLS_MSG_BUFFER_DISKERROR,   //104064 ��ʼ���Ż������ж�Ӳ�̿ռ䲻���Ի���һ��������Ŀ
    HLS_MSG_BUFFER_BANDWIDTH,   //104063 �����ڼ��ƽ�������ٶ����С�ڲ����ٶ�

    //�����ά��
    STRM_MSG_BANDWIDTH_LOW,     //add for hls 205~212
    STRM_MSG_NOT_FOUND,
    STRM_MSG_BITRATE_CHANEGE,
    STRM_MSG_RECV_TIMEOUT_ERROR,//���ݽ��ճ�ʱ       10
    STRM_MSG_RECV_TIMEOUT_PLAY, //���ݽ��ճ�ʱ       10
    STRM_MSG_SUCCESS_OPEN,
    STRM_MSG_SUCCESS_CLOSE,
    STRM_MSG_BUF_RECV,           //add for hls end

    STRM_MSG_RECV_IGMP_TIMEOUT = STRM_MSG_BUF_RECV + 1,
    STRM_MSG_PLAY_RECV_RTSP_TIMEOUT,
    STRM_MSG_START_RECV_RTSP_TIMEOUT,
    STRM_MSG_RECV_IGMP_RESUME,
    STRM_MSG_RECV_RTSP_RESUME

} STRM_MSG;

/* IPTV ���� STB���ϴ����.xlsx
10200    STRM_MSG_RECV_TIMEOUT arg = 1  �����鲥��������·������������鲥Ƶ��ʱ�����鲥Ƶ�����Ź�����
10201    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_URL_FORMAT_ErrorƵ����ַ���ĿURL��ַΪ�ջ��ʽ����
10202    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_Connect_Timeout
                             �� arg = RTSP_CODE_Connect_Timeout STB����ý��������޷��������ӣ�socket����ʧ�ܣ�
10203    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_Session_Timeout STB����������ý������������������ʱ�����޷������������Ž׶�
10204    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_Parse_Error     �ڽ���ý��������ǰ��Э�̽׶Σ��������޷�������ý���������Ӧ������
10205    STRM_MSG_RECV_TIMEOUT  arg = 0                         ��ʼ����ʱ����ý����������ӽ�������������δ�յ�ý����
10206    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_Play_Error      �����з��Ͳ��ſ�������ʧ�ܡ���ʾ��ʽΪС���ڣ�����Ļ�м�չʾ5����Զ���ʧ
10207    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_Play_Timeout    �����з��Ͳ��ſ��������ý���������ʱ����Ӧ����ʾ��ʽΪС���ڣ�����Ļ�м�չʾ5����Զ���ʧ
10208    STRM_MSG_PACKETS_LOST                                  ��ý�岥��ʱ�������ض�����������2���������ڶ����ʲ�С��1% ��ע����1��ֻ���þ�����RTP/UDP����2������������ QOS �淶�еĹ涨����һ�£�Ϊ 10s��3����ʾ��ʽΪС���ڣ�����Ļ�м�չʾ5����Զ���ʧ��
10209    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_Socket_Error    ���Ź�����STB����ý������������жϵ���ý�����ж�
10087    STRM_MSG_OPEN_ERROR    arg = RTSP_CODE_server_Error    server�ر���rtsp����ý����û�жϡ�
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
    APP_TYPE_VOD = 0,   //VOD����
    APP_TYPE_VODADV,
    APP_TYPE_IPTV,      //Ƶ������ APP_TYPE_UNICAST��APP_TYPE_MULTICAST��APP_TYPE_IGMP�ϲ�����
    APP_TYPE_IPTV2,     //ֱ��url �� ʱ��url���롣�ø����Ͳ��ź�mid_stream_get_apptype��ȡ��ֵ��ȻΪAPP_TYPE_IPTV
                        //mid_stream_open �ĵڶ�������Ҫ����һ���ṹ��IPTVUrlָ�롣
    APP_TYPE_RESERVE4,
    APP_TYPE_HTTP,      //5
    APP_TYPE_PVR,       //¼���ļ�����
    APP_TYPE_AUDIO,
    APP_TYPE_TSTV,      // ֧�ֱ���ʱ��Ƶ������
    APP_TYPE_DVBS,
    APP_TYPE_MOSAIC,    //10 ������
    APP_TYPE_FLASH,     //FLASH
    APP_TYPE_DISK,      //�����ļ�����
    APP_TYPE_RTP2TS,
    APP_TYPE_RESERVE14,
    APP_TYPE_MIX_PCM,   //15
    APP_TYPE_MIX_MP3,
    APP_TYPE_HTTP_PCM,
    APP_TYPE_HTTP_MP3,
    APP_TYPE_ZEBRA,
    APP_TYPE_ZEBRA_PCM, //20
    APP_TYPE_HTTP_MPA,  //֧�� mp3,mp2,aac
    APP_TYPE_HLS,       //Http live streaming
    APP_TYPE_HTTP_LIVE, //Http live streaming �����ά��
    APP_TYPE_APPLE_VOD,
    APP_TYPE_APPLE_IPTV,
    APP_TYPE_RESERVE26,
    APP_TYPE_MAX
} APP_TYPE;


//����ͳ�ƣ����������Ϻ����Ĵ���
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

//��ӡ����ʱ��
void mid_stream_buildtime(void);

/*
    ���ų�ʼ��
    num������ʵ����ȡֵ��ΧΪ
        1    һ·����
        2    ֧��PIP
 */
void mid_stream_init(int num);

void mid_stream_regist_mix(void);
void mid_stream_regist_test(void);
void mid_stream_regist_voole(void);//����
void mid_stream_regist_zebra(void);


/**
    ���ñ��β�������
**/
#define mid_stream_set_language mid_stream_set_language_v1
void mid_stream_set_language(char* language);

/*
    ����RTSP User-Agent
*/
void mid_stream_set_useragent(char *useragent);

/*
    ȥ����
    mid_stream_fcc �� mid_stream_ret
    ������
    mid_stream_set_fcc �������ò��Ž�Ŀ����
    mid_record_set_fcc ��������¼�ƽ�Ŀ����
 */
void mid_stream_set_fcc(int idx, int fcc_type);
/* ��ֹ�ϱ����� */
void mid_stream_set_sqm(int flag);
/* ��ֹ�ϱ���Ϣ */
void mid_stream_set_msg(int flag);

typedef int     (*MultiPlayCall)(void* igmp, int igmp_size, struct ind_sin *mult_sin);
typedef void    (*MultiStopCall)(void* igmp, int igmp_size, int sock, struct ind_sin *mult_sin);

#define mid_stream_set_igmp mid_stream_set_igmp_v1
void mid_stream_set_igmp(int idx, MultiPlayCall play_call, MultiStopCall stop_call, void* igmp, int igmp_size);

void mid_stream_ret_cache(int flag);//����SQA�����cache

void mid_stream_set_arq(int flag);
//��������
void mid_stream_set_burst(int flag);
/*
    ����on�������ٻ��壬-1��ʾ�رտ��ٻ���
    ����off�رտ��ٻ���
    on off �ĵ�λΪ���룬
    Ĭ�������
    ���� on=1500 off=2000
    ���� on=3000 off=5000
 */
void mid_stream_cache(int on, int off);

void mid_stream_cache_size(int on, int off);

/*
    VOD����һ���������ſ�ʼ����
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
    tcp ��ʽ�£��������󲻽�������
 */
void mid_stream_recv_safe(int safe);

//I֡���ţ��ú���ֻ������ʹ��
void mid_stream_iframe(int flag);

typedef enum {
    RTSP_TRANSPORT_TCP = 0,
    RTSP_TRANSPORT_UDP = 1,
    RTSP_TRANSPORT_RTP_TCP = 2,
    RTSP_TRANSPORT_RTP_UDP = 3
} RTSP_STANDARD;
/*
    transport ȡֵ��������
    ���� RTSP_STANDARD_YUXING(���ݾ�MDN) ��Ȼ���� 0: udp 1 tcp ����֧��RTPͷ
    ���� RTSP_STANDARD_HUAWEI\RTSP_STANDARD_CTC_SHANGHAI��RTSP_STANDARD_CTC_GUANGDONG��ѭ���¶���
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
    ����RTSP������ѭ�ı�׼
 */
void mid_stream_standard(int standard);

/*
    ʹ�ܻ�ȡ��ʱ�˶��ε���
 */
void mid_stream_timeshift_second(int enable);

/*
    ����������������������л�������ͨ��������������֤����������ά����һ��ˮλ��
 */
#define mid_stream_timeshift_level mid_stream_timeshift_level_v1
void mid_stream_timeshift_level(int level);

/*
    ��ͣʱ�䵽�����ϵ��Զ�ת����
 */
void mid_stream_timeshift_jump(int jump);

/*
    �򿪻�رձ���ʱ��¼��
    id ΪƵ���Ż�Ƶ��key���������ϲ��Զ���
    id ȡֵ��ΧΪ 0 ~ 999����ֵ��ͨ��mid_stream_timeshift_getid ��ȡ��
    id Ϊ -1 ��ʾ�Զ�¼��
    v1 ����ֵ�޸�Ϊvoid
 */
#define mid_stream_timeshift_open mid_stream_timeshift_open_v1
void mid_stream_timeshift_open(void);
void mid_stream_timeshift_close(void);

/*
    ʹ�ܻ�ȡ���鲥��ʱת����
    0 Ĭ��ģʽ
    1 �鲥ת����
    2 �鲥ת���� ��������һƵ��5�����ݽ��ճ�ʱʱ���Զ�תΪ��������
    3 ��������
 */
void mid_stream_multicast_unicast(int enable);
/*
 */
void mid_stream_multicast_forbid(int forbid);

/*
    ������������
    interval ��λΪ�루Ĭ����30s��
 */
void mid_stream_heartbit_period(int interval);

/*
    ��Ϊ˽�й淶Ĭ�ϲ���OPTIONS��Ϊ����
    ���standard�� 1 ��ǿ�Ʋ��ñ�׼GET_PARAMETER��ʽ����
 */
void mid_stream_heartbit_standard(int standard);

/*
 */
void mid_stream_heartbit_active(void);

/*
    ���ò��Ŵ��ڴ�С
 */
void mid_stream_rect(int idx, int x, int y, int width, int height);

/*
    ����rtsp��TCP�˿�
 */
void mid_stream_port(int idx, int port_tcp, int port_udp);

/*
    ��������VOD���ſ�ʼms�յ�������
    v1 ȥ������ int idx
 */
#define mid_stream_skip mid_stream_skip_v1
void mid_stream_skip(int ms);

/*
    ���ôӽ�����CA����CA key���óɹ��ĵȴ���ʱ
    v1 ȥ������ int idx
 */
#define mid_stream_cawait mid_stream_cawait_v1
void mid_stream_cawait(int ms);

/*
���ô󴰿ڲ��Ż����С��Ĭ��ֵ��
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
    reset�����²��ţ��ӳ�һ��ʱ��ſ�ʼ��
 */
void mid_stream_vodsync(int ms);

/*
    VOD�ӳ�һ��ʱ��ſ�ʼ����
    v1 ȥ������ int idx
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
    �򿪲���ʼ����
    url�����ŵ�ַ
        ��¼���ļ����ţ�APP_TYPE_PVR��url����¼��ID�ַ�����ʽ
    atype����������
        ����֧�ֱ���ʱ�Ƶ�Ƶ�����ţ�APP_TYPE����ΪAPP_TYPE_PVRCAST
    shiftlen��
        VOD����´����ſ�ʼʱ��starttime
        IPTV����´���ʱ�Ƴ���
            > 0 ���ϲ�ָ��ʱ�Ƴ��Ȳ���
            = 0 ��TIMESHIFT_OFF����ֹʱ��
            = -1 ��TIMESHIFT_AUTO���Զ���������֧�ֵ����ʱ�Ƴ��Ȳ���
            ����֧�ֱ���ʱ�Ƶ�Ƶ�����ţ���Ŀǰ��Ϊ�涨����ֵΪ1800��Ҳ����30���ӡ�

        ����APP_TYPE_HTTP_MP3��APP_TYPE_HTTP_MPA��shiftlenΪ1��ʾѭ������

    ����ֵ��
        ÿ�β��ţ�����һ��Ψһ������ֵ����ǣ���ֵ����
        stream_port_state��stream_port_message�����һ������magic
        �ϲ��������������������Ǵβ���
        �������������ķ���ֵ������ͬ
 */
unsigned int mid_stream_open(int idx, const char* url, APP_TYPE atype, int shiftlen);
//�б��ţ�VOD�ӹ�棩
unsigned int mid_stream_open_vodAdv(int idx, struct VODAdv* list, int starttime);

/*
    �ȴ�����������ʼ
 */
int mid_stream_sync(int idx, int msec);

/*
    �򿪲���ʼ����
    VOD�����
        ��ͨ���ţ�begin ���� -1
        �ֶβ��ţ�begin >= 0��Ӧ�ò�����begin < 0�ķֶβ������ʱ��begin �� 0
    IPTV����� begin Ŀǰ��ʵ������
 */
unsigned int mid_stream_open_range(int idx, const char* url, APP_TYPE apptype, int shiftlen, int begin, int end);

/*
    ֹͣ���رղ���
    clear����1 �����������
 */
void mid_stream_close(int idx, int clear);


/*
    ֹͣʱ�Ʋ��ţ�תֱ��
 */
void mid_stream_stop(int index);
/*
    ��ͣ����
 */
void mid_stream_pause(int index);
/*
    ����ͣ״̬���ݿ���״̬ �ָ��� ��������
 */
void mid_stream_resume(int index);
/*
    �������˲���
    level ȡֵ��ΧΪ:
        -32, -16, -8, -4, -2 ����
        32, 16, 8, 4, 2 ���
 */
void mid_stream_fast(int idx, int level);
/*
    ��Ծ����λ��(SEEK)����
 */
void mid_stream_seek(int idx, int second);

/*
 */
void mid_stream_lseek(int idx, long long offset);

/*
    ��ȡ����������
 */
APP_TYPE mid_stream_get_apptype(int index);

/*
    ��ȡ�����ܳ��ȣ�����IPTV�����ܳ��ȵ���ʱ�Ƴ���
 */
unsigned int mid_stream_get_totaltime(int index);
void mid_stream_sync_totaltime(int idx, int msec);
/*
    ��ȡ��ǰ����ʱ��
 */
unsigned int mid_stream_get_currenttime(int index);

long long mid_stream_get_totalbyte(int index);
long long mid_stream_get_currentbyte(int index);

/*
    ���������ļ����ܳ���Ŀǰֻ֧��mp3
 */
int mid_stream_auido_length(char* filepath);


int mid_stream_mix_space(int idx, unsigned int magic);
int mid_stream_mix_push(int idx, unsigned int magic, char* buf, int len);

//msgcall�ص�ʱ����һ��������key���ú���Ҫ��mid_stream_mosaic_open֮ǰ����
void mid_stream_mosaic_setcall(int key, StrmMsgCall msgcall, int callarg);
//�����˲���
int mid_stream_mosaic_open(int key, const char* url, int x, int y, int width, int height, char* igmp, int igmp_size);
void mid_stream_mosaic_close(int key);

void mid_stream_mosaic_save(int flag);
void mid_stream_mosaic_mute(int mute);
void mid_stream_mosaic_set(int key);
int mid_stream_mosaic_get(void);
/*
  ����hls���ݻ���ٷֱ�
*/
int mid_stream_hls_buffrate(void);
unsigned int mid_stream_hls_recordrate(void);

/*
    ����APP_TYPE_HTTP_MPA���ŵ�cookie
 */
void mid_stream_hmpa_cookie(int idx, char* cookie);

/*
    ����HTTP���ز��Ż�Ϊ��ʽ����key
 */
void mid_stream_http_pvrkey(int idx, long long key);
/*
    ����HTTP����gzip��ѹ����
 */
typedef int (* StrmHttpUnzip)(char* srcbuf, int srclen, char* dstbuf, int* dstlen);
void mid_stream_http_unzip(StrmHttpUnzip httpUnzip);

void mid_stream_set_tuner(int idx, int tuner);

/*
    ���epg�·���EMM���ݣ��ڵ��β���ǰҪ���øú���֪ͨ����
 */
#define mid_stream_emm mid_stream_emm_v1
void mid_stream_emm(void);

/*
    ���ö���ͳ������
    interval����λΪ��
 */
void mid_stream_statint_pklosts(int interval);
/*
    ��������ͳ������
    interval����λΪ��
 */
void mid_stream_statint_bitrate(int interval);

/*
    ��黺��������(�Ĵ��ֵ�Ҫ��)
    picwidth:
        <= 720 ����
        >  720 ����
    flow:
         1 ����
        -1 ����
 */
typedef void (* CallBack_StatFlow)(int multflg, int picwidth, int flow);
void mid_stream_callback_statflow(CallBack_StatFlow statflow);

typedef void (* CallBack_RtspInfo)(int multflg, char *info_buf, int info_len);
void mid_stream_callback_rtspinfo(CallBack_RtspInfo rtspinfo);

void mid_stream_nat(int mode);
void mid_stream_nat_heartbitperiod(int time);

/*
    ����RRS���ӳ�ʱʱ��
    C58����US �����ƻ�-20120227.xlsx
    1�������Ϊÿ�����α���ÿ��RRS��
    2������û����Ӧ2s��Ϊ��RRS���ϡ�
 */
void mid_stream_rrs_timeout(int ms);

void mid_stream_get_rtspInfo(int idx, char *url, char *method);

void mid_stream_set_apple_buffersize(int size);
//����ʹ��
void mid_stream_set_apple_bandwidth(int bandwidth);
void mid_stream_set_apple_level(int level);

/*
    AudioBitRate    ��Χ 0 ~ AudioChannels - 1
    AudioChannels
    CurBufferSize
    DownloadRate
    PacketLost ��λΪǧ��֮һ
    Playrate
    StreamBandwith  ��Χ 0 ~ StreamNum - 1
    StreamNum
    SegmentNum
    RemainPlaytime
    ToalBufferSize
    TransportProtocol
 */
int mid_stream_getInt(char *paramName, int arg);

/*
    CurSegment
    SegmentList     ��Χ 0 ~ SegmentNum - 1
    StreamURL       ��Χ 0 ~ StreamNum - 1
 */
void mid_stream_getString(char *paramName, int arg, char *buf, int size);

typedef void (*TSSyncNotifyCall)(void);
/*
    TSͬ�������ϱ�
    interval �ϱ����ڣ���λΪ��
 */
void mid_stream_set_tsSyncNotify(unsigned int interval, TSSyncNotifyCall notifyCall);

unsigned int mid_stream_hls_playrate(void);

typedef void (StreamRouteCall)(unsigned int addr);
void mid_stream_setRouteCallback(StreamRouteCall func);

/*
    addr: IPTV V1R3C56 �������-SQA��ƹ��V1.6.doc 4.6.3�������������� 3��UE��Ƶ����Ϣ�б�����ѡһ��Ƶ��������URL�н�����RRS��IP��ַ��
    port: sysSettingGetInt("fcc_port", &port, 0);
*/
void mid_stream_setRRS(unsigned int addr, int port);

/******************************���Ժ���**************************************/
/*
    flag��1 ��ʼ���棬0 ֹͣ����
 */
void mid_stream_save(int flag);
/*
    bandwidth��http���ش�������
 */
void mid_stream_bandwidth(int bandwidth);

#ifdef __cplusplus
}
#endif

#endif /* __MID_STREAM_H__ */
