
#ifndef __IND_TS_H__
#define __IND_TS_H__

//MPEG Program Specific Information (PSI)

#define ISO_IEC_11172_VIDEO     0x01        // ISO11172 Video
#define ISO_IEC_13818_2_VIDEO   0x02        // ISO13818-2 Video
#define ISO_IEC_11172_AUDIO     0x03        // ISO11172 Audio
#define ISO_IEC_13818_3_AUDIO   0x04        // ISO13818-2 Audio

#define ISO_IEC_13818_1_PRI     0x05
#define ISO_IEC_PES_DATA        0x06

#define ISO_IEC_13818_6_A       0x0a        // ISO/IEC 13818-6 type A
#define ISO_IEC_13818_6_B       0x0b        // ISO/IEC 13818-6 type B
#define ISO_IEC_13818_6_C       0x0c        // ISO/IEC 13818-6 type C
#define ISO_IEC_13818_6_D       0x0d        // ISO/IEC 13818-6 type D

#define ISO_IEC_13818_7_AUDIO   0x0f        // ISO/IEC 13818-7 Audio with ADTS transport syntax

#define ISO_IEC_MPEG4_VIDEO     0x10        // MPEG4 (video)
#define ISO_IEC_MPEG4_AUDIO     0x11        // MPEG4 (audio)
#define ISO_IEC_H264            0x1B        // H264 <- check transport syntax/needed descriptor
#define ISO_IEC_H264_SVC        0x1F        // ISO/IEC 14496-10 AnnexG Video
#define ISO_IEC_H264_MVC        0x20        // ISO/IEC 14496-10 AnnexH Video

#define ISO_IEC_AVS_VIDEO       0x42
#define ISO_IEC_AVS_AUDIO       0x43

#define ISO_IEC_AC3_AUDIO       0x81        // A52 (audio) AC3
#define ISO_IEC_DVB_SPU         0x82        /* DVD_SPU (sub) */

//非标准
#define ISO_EXT_AUDIO           0xE0

#define ISO_EXT_AC3_AUDIO       0xE1        // E-AC3

#define ISO_EXT_DVD_SPU         0xE2        // DVD_SPU (sub)
#define ISO_EXT_DTS_AUDIO       0xE3        // Digital Digital Surround

//Windows Media Video
#define ISO_IEC_VC1             0xEA        // VC-1 Advanced Profile
#define ISO_IEC_VC1_SM          0xEB        // VC-1 Simple&Main Profile

#define ISO_IEC_AAC_PLUS_ADTS   0x111       //AAC plus ADTS 
#define ISO_IEC_AAC_LOAS        0x10f       //AAC LOAS

#define ISO_IEC_DIVX            0x311       // DIVX

#define TS_DVB_PMT_PID          0x21
#define TS_DVB_ELEM_PID         0x22
#define TS_DVB_ELEM_NUM         16

#define TS_EMM_NUM              16
#define TS_ECM_NUM              16
#define TS_AUDIO_NUM            16
#define TS_SUBTITLE_NUM         32
#define TS_TRANSPORT_SIZE       1316

#define TS_SECTION_SIZE         1024

#define TS_PES_HLEN             14

#define TS_INVALID_VERSION      0xff

typedef struct ts_audio*    ts_audio_t;
typedef struct ts_elems*    ts_elems_t;
typedef struct ts_psi*      ts_psi_t;
typedef struct ts_pts*      ts_pts_t;
typedef struct ts_pcr*      ts_pcr_t;
typedef struct ts_parse*    ts_parse_t;
typedef struct ts_buf*      ts_buf_t;
typedef struct ts_rtp2ts*   ts_rtp2ts_t;
typedef struct ts_rtp2es*   ts_rtp2es_t;
typedef struct ts_mosaic*   ts_mosaic_t;

typedef struct ts_ca*           ts_ca_t;
typedef struct ts_iso693*       ts_iso693_t;
typedef struct ts_dr_subtitle*  ts_dr_subtitle_t;
typedef struct ts_dr_teletext*  ts_dr_teletext_t;
typedef struct ts_dr_dvdspu*    ts_dr_dvdspu_t;

struct ts_ca
{
    unsigned short    system_id;
    unsigned short    pid;
};

struct ts_mosaic
{
    int key;

    int x;
    int y;
    int width;
    int height;

    unsigned int    pcr;//PCR PID
    unsigned int    vpid;
    unsigned int    vtype;
    unsigned int    apid;
    unsigned int    atype;

    struct ts_ca    ts_ca;
};

struct ts_elems {
    unsigned short pmt_pid;
    unsigned short pcr_pid;

    unsigned int elem_num;
    unsigned short elem_pid[TS_DVB_ELEM_NUM];
    unsigned short elem_type[TS_DVB_ELEM_NUM];
};

struct ts_subtitle
{
    unsigned char language[3];
    unsigned char type;
    unsigned short composition;
    unsigned short ancillary;
};

struct ts_iso693
{
    unsigned char language[3];
    unsigned char type;
};

struct ts_teletextpage
{
    unsigned char    language[3];    /* 24 bits */
    unsigned char    type;            /*  5 bits */
    unsigned char    magazine;        /*  3 bits */
    unsigned char    page;            /*  8 bits */

};

struct ts_dr_subtitle {
    unsigned int subtitle_num;
    unsigned short subtitle_pid[TS_SUBTITLE_NUM];
    struct ts_subtitle subtitle[TS_SUBTITLE_NUM];
};

struct ts_dr_dvdspu {
    unsigned int dvdspu_num;
    unsigned short dvdspu_pid[TS_SUBTITLE_NUM];
    struct ts_iso693 dvdspu_iso693[TS_SUBTITLE_NUM];
};

struct ts_dr_teletext {
    unsigned int pid;
    unsigned int page_num;
    struct ts_teletextpage page[64];
};

struct ts_psi {
    unsigned int video_pid;
    unsigned int video_type;

    int audio_num;
    unsigned int audio_pid[TS_AUDIO_NUM];
    unsigned int audio_type[TS_AUDIO_NUM];
    struct ts_iso693 audio_iso693[TS_AUDIO_NUM];

    unsigned int reserve;//有些片源没有PCR
    unsigned int pcr_pid;
    unsigned int pmt_pid;
    unsigned int pmt_crc;

    ts_dr_subtitle_t    dr_subtitle;
    ts_dr_teletext_t    dr_teletext;
    ts_dr_dvdspu_t      dr_dvdspu;

    int             ecm_num;
    struct ts_ca    ecm_array[TS_ECM_NUM];

    int             emm_num;
    struct ts_ca    emm_array[TS_EMM_NUM];
};

#ifdef __cplusplus
extern "C" {
#endif

//ts_psi
void ts_parse_debug(int enable);
ts_parse_t ts_parse_create(ts_psi_t psi);
void ts_parse_reset(ts_parse_t parse);
void ts_parse_delete(ts_parse_t parse);
/*
    分析PSI信息
    -1 错误
    0 未分析出PSI
    1 分析到新PSI
    2 码流不连续

    v3 stream 于independs同步
    v5 修改CA信息
    v6 去掉最后一个无用参数clk
 */
#define ts_parse_psi ts_parse_psi_v6
int ts_parse_psi(ts_parse_t parse, unsigned char *buf, int len, ts_pts_t tsp);
int ts_parse_getpmt(ts_parse_t parse, char* pmtbuf, int size);
int ts_parse_getcat(ts_parse_t parse, char* pmtbuf, int size);

unsigned int ts_parse_pts(unsigned char *buf);

int ts_parse_error(ts_parse_t parse);

unsigned int ts_parse_pmt_prognum(unsigned char *buf);

int ts_cat_equal(ts_psi_t src_psi, ts_psi_t dst_psi);
int ts_psi_equal(ts_psi_t dst_psi, ts_psi_t src_psi);
int ts_psi_copy(ts_psi_t dst_psi, ts_psi_t src_psi);

void ts_psi_print(ts_psi_t psi);

//ts_pcr
ts_pcr_t ts_pcr_create(void);
void ts_pcr_delete(ts_pcr_t tspcr);
#define ts_pcr_reset ts_pcr_reset_v1
void ts_pcr_reset(ts_pcr_t tspcr, int half);
void ts_pcr_delete(ts_pcr_t tspcr);
int ts_pcr_fill(ts_pcr_t tspcr, char *buf, int len);
unsigned int ts_pcr_time(ts_pcr_t tspcr);

unsigned int ts_pcr_parse188(unsigned char *buf);

//ts_pts
ts_pts_t ts_pts_create(void);
void ts_pts_delete(ts_pts_t tspts);
void ts_pts_reset(ts_pts_t tspts, int scale);
/*
    将码流中的所有PTS依次输入进ts_pts_t结构
 */
void ts_pts_input_pcr(ts_pts_t tsp, unsigned char *buf);
void ts_pts_input(ts_pts_t tsp, unsigned int pid);

#define ts_pts_time_play    ts_pts_time_play_v1
int ts_pts_time_play(ts_pts_t tspts, int pcrmode, unsigned int clk, unsigned int pts);
int ts_pts_time_last(ts_pts_t tspts, int pcrmode);

//ts_size
/*
    分析TS流画面尺寸
    -1：错误
     0：未分析出
     1：标清
     2：高清
 */
int ts_size_parse(char *buf, int len, unsigned int video_pid, unsigned int video_type, int* pwidth, int* pheight);
int ts_ifrm_offset(char *buf, int len, unsigned int video_pid, unsigned int video_type);

//ts_audio
ts_audio_t ts_audio_create(void);
void ts_audio_delete(ts_audio_t a2t);
int ts_audio_reset(ts_audio_t a2t);
int ts_audio_space(struct ts_audio *a2t);
int ts_audio_buf_get(ts_audio_t a2t, char **buf, int *len);
int ts_audio_buf_put(ts_audio_t a2t, int len);
int ts_audio_write(struct ts_audio *a2t, char *buf, int len);
int ts_audio_buf_clr(ts_audio_t a2t);
int ts_audio_read(ts_audio_t a2t, char *buf, int size);
int ts_audio_frame(struct ts_audio *a2t, char *buf, int size);

typedef int (*ts_read_f)(void* handle, char* buf, int size);
int ts_audio_duration(void *handle, ts_read_f readcall, int filesize);

int ts_audio_second(struct ts_audio *a2t, int size);

unsigned int ind_ts_crc32(const unsigned char *buf, unsigned int len);
void ind_ts_header(char *packet, unsigned int pid, int start, unsigned int conter);
void ind_ts_adaptation(char *packet, int len, unsigned int pcr);
void ind_ts_pat(char *packet, unsigned int conter, unsigned int prognum, unsigned int pmtpid);
void ind_ts_pmt(char *packet, unsigned int conter, unsigned int prognum, ts_elems_t elems);
void ind_ts_pmt_e1(char *packet, unsigned int es_type, unsigned int conter);
void ind_ts_pes(char *pes, unsigned int id, unsigned int len, unsigned int pts);

int ind_ts_filter(unsigned char *sbuf, int slen, unsigned int fpid, unsigned char *dbuf, int dlen);

int ind_ts_cpid(ts_elems_t ses, ts_elems_t des);
int ind_ts_xpid(unsigned char *sbuf, unsigned char *dbuf, ts_elems_t ses, ts_elems_t des);
int ind_ts_xpcr(unsigned char *sbuf, unsigned char *dbuf, ts_elems_t ses, ts_elems_t des, unsigned int counter);

//ts_buf
typedef void* (*ts_malloc_f)(int size);
typedef void (*ts_free_f)(void* addr);

#define ts_buf_create ts_buf_create_v2
ts_buf_t ts_buf_create(int size);
ts_buf_t ts_buf_reload(char* buffer, int size);
void ts_buf_delete(ts_buf_t tb);
/*
    临时释放大块缓存，ts_buf_reset执行后会自动重新申请
 */
int ts_buf_size(ts_buf_t tb);
void ts_buf_reset(ts_buf_t tb);
void ts_buf_print(ts_buf_t tb);
int ts_buf_length(ts_buf_t tb);

void ts_buf_temp_set(ts_buf_t tb, int flag);

void ts_buf_read_get(ts_buf_t tb, char **buf, int *len);
int ts_buf_read_pop(ts_buf_t tb, int len);
void ts_buf_write_get(ts_buf_t tb, char **buf, int *len);
int ts_buf_write_put(ts_buf_t tb, int len);
void ts_buf_reload_get(ts_buf_t tb, char **buf, int *len);
int ts_buf_reload_mark(ts_buf_t tb, int len);

int ts_buf_read(ts_buf_t sb, char *buf, int size);
int ts_buf_peek(ts_buf_t tb, int off, char *buf, int size);
int ts_buf_memstr(ts_buf_t sb, char* str);

//H.264 RTP格式 转 TS格式
typedef int (*ts_write_f)(const char* buf, int len, void* arg);

#define ts_rtp2ts_create ts_rtp2ts_create_v1
ts_rtp2ts_t ts_rtp2ts_create(ts_write_f wr_func, void* wr_arg);
void ts_rtp2ts_delete(ts_rtp2ts_t handle);
int ts_rtp2ts_trans(ts_rtp2ts_t handle, unsigned int clk, unsigned int diff, char* rtp_buf, int rtp_len);

ts_rtp2es_t ts_rtp2es_create(ts_write_f wr_func, int wr_arg);
void ts_rtp2es_delete(ts_rtp2es_t handle);
int ts_rtp2es_transact(ts_rtp2es_t handle, char* rtp_buf, int rtp_len);

#define ts_index_check ts_index_check_v1
int ts_index_check(char *buf, int len, unsigned int vpid, unsigned int apid);

#ifdef __cplusplus
}
#endif

#endif//__IND_TS_H__
