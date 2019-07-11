
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <fcntl.h>

#include "app/Assertions.h"
#include "ind_string.h"
#include "ind_ts.h"
#include "ind_pvr.h"
#include "ind_cfg.h"
#include "../ts/ts_iframe.h"

#include "int_pvr.h"

/*
 版本纪录
 0 增加版本号
 1 增加byte_len
 2 增加byte_length
 3 增加iframe_clk
 4 增加flags，简化断点续传，由上层保持无缝衔接
 5 修改配置保存形式
 6 增加w_info 文件
 7 修改加密方式
 8 w_info文件更名为winfo
 9 
     1秒记录1帧 -> 1秒记录2.5帧
     media.idx -> media.ifr
10 合并
11 SECOND_PER_FRAGMENT 由 600 修改为 60
 */

#define PVR_VERSION             11

#define REC_ARRAY_SIZE          3

#define PVR_OFFSET_BEGIN       -3
#define PVR_OFFSET_END         -2
#define PVR_OFFSET_CURRENT     -1

#define IFRAME_ARRAY_SIZE       (600 * 100 / IFRAME_CLK_INTERVAL)

#define IFRAME_PER_BREAK        (6 * 100 / IFRAME_CLK_INTERVAL)
#define BYTES_PER_POINT         (2048 * 1316)
#define BYTES_PER_BREAK         (128 * 1316)

#define SHARE_INFO_SIZE         (128 * 1024)
#define SHARE_PATH_NAME         "00000001"

#define IFRAME_SIZE             sizeof(struct ts_iframe)

#define FLAGS_WRITE             (O_WRONLY|O_CREAT|O_TRUNC)
#define FLAGS_APPEND            (O_WRONLY|O_CREAT|O_APPEND)

//原子互斥
#define MUTEX_LOCK( )           g_op->mutex_lock(g_mutex, __FUNCTION__, __LINE__)
#define MUTEX_UNLOCK( )         g_op->mutex_unlock(g_mutex)

//读
#define READ_MUTEX_LOCK( )      g_op->mutex_lock(g_r_mutex, __FUNCTION__, __LINE__)
#define READ_MUTEX_UNLOCK( )    g_op->mutex_unlock(g_r_mutex)

//读
#define WRITE_MUTEX_LOCK( )     g_op->mutex_lock(g_w_mutex, __FUNCTION__, __LINE__)
#define WRITE_MUTEX_UNLOCK( )   g_op->mutex_unlock(g_w_mutex)

//录制
#define RECORD_MUTEX_LOCK( )    g_op->mutex_lock(rec->mutex, __FUNCTION__, __LINE__)
#define RECORD_MUTEX_UNLOCK( )  g_op->mutex_unlock(rec->mutex)

typedef struct tagPvrBPoint {
    int        bp_taglen;

    int        bp_fill_num;
    int        bp_break_off;
    int        bp_break_num;

    long long  bp_byte_len;
    int        bp_strm_len;

    int        bp_strm_sn;
    int        bp_ifrm_num;
    int        bp_break_ifrm;

    struct ts_iframe    bp_ifrm;
    struct ts_icontent  bp_icntnt;

    uint32_t    bp_ifrm_clk;
    uint32_t    bp_ifrm_off;
    uint32_t    bp_ifrm_size;

    uint32_t    bp_cksum;
} PvrBPoint;

typedef struct tagPvrWInfo {
    int         version;
    uint32_t        crc32;

    int         w_sn;
    int         w_sn_base;

    int         w_strm_len;
    int         w_fill_num;//断点填充包个数

    long long   w_byte_len;

    struct ts_iframe    w_ifrm_buf[IFRAME_ARRAY_SIZE];
    int                 w_ifrm_num;
    int                 w_ifrm_valid;

    struct ts_iparse    w_iparse;
//--------------------------------------------------------
    int         w_strm_fd;
    int         w_ifrm_fd;

    int         w_break_num;//断点个数
    int         w_break_len;
    int         w_reserve2;

    int         w_break_off;
    int         w_break_ifrm;
    int         w_break_size;
} PvrWInfo;
typedef PvrWInfo* PvrWInfo_t;

typedef struct tagPvrRInfo {
    uint32_t    r_clk_base;
    uint32_t    r_clk_last;

    int     r_scale;

    int     r_fast_end;
    int     r_fast_flag;
    int     r_fast_times;

    int     r_strm_off;
    int     r_strm_len;

    int     r_byte_size;
    int     r_byte_len;

    char    r_ts_buf[188];
    int     r_ts_len;

    ts_pcr_t            r_tspcr;

    struct ts_iframe    r_ifrm_buf[IFRAME_ARRAY_SIZE];
    int                 r_ifrm_num;
    int                 r_ifrm_off;
    int                 r_ifrm_off1;

    int                 r_strm_fd;
    int                 r_strm_sn;
} PvrRInfo;
typedef PvrRInfo* PvrRInfo_t;

typedef struct tagPvrSInfo_ {
    uint32_t    prev_clks;
    uint32_t    prev_date;

    uint32_t    time_len;
    int     fill_num;

    long long byte_len;
    long long byte_length;

    int     record;
    uint32_t    crc;
} PvrSInfo_;

struct PvrElem {
    PvrElem_t next;

    PvrElem_t    list_first;
    PvrElem_t    list_next;

    PvrSInfo    s_info;
    int         s_info_sn;

    int         index;
    int         announce;

    char        path[PVR_PATH_LEN];

    char*       info_buf;
    int         info_len;

    //分片下载时，分片个数
    int         slice_num;

    int         fragment_time;
    int         fragment_size;

    PvrRecord_t w_rec;

    PvrRInfo_t  r_info;
};

typedef struct PvrRecElem_ PvrRecElem;
typedef struct PvrRecElem_* PvrRecElem_t;
struct PvrRecElem_ {
    PvrRecElem_t    next;
    PvrElem_t       pvr;
    uint32_t            clk;
};

struct PvrRecord {
    uint32_t        id;
    int         pcr;
    int         index;
    void*       mutex;
    int         encrypt;
    int         networkid;

    char        path[PVR_PATH_LEN];

    PvrWInfo    w_info;
    PvrWInfo    w_break;

    int         info_sn;

    PvrRecElem      shift;
    PvrRecElem_t    record;
};

typedef struct tagPvrStamp {
    int         flag;

    int         strm_sn;
    int         strm_off;
    int         ifrm_off;
} PvrStamp;

typedef struct tagPvrConfig {
    long long       key;

    CfgContext_t    context;
    CfgContext_t    context_;
    char            w_info_buf[PVR_INFO_SIZE];
    int             w_info_len;
    char            r_info_buf[PVR_INFO_SIZE];
    int             r_info_len;

    char*           share_buf;
    int             share_size;

    PvrWInfo        w_info;

    PvrElem_t       pvr_hash[256];
    PvrElem_t*      pvr_table;
    int             pvr_size;
    int             pvr_num;

    //id_array用于外部读取使用
    uint32_t*           id_array;
    int             id_num;

    char            path[PVR_PATH_LEN];
    int            pathlen;

    PvrRecord_t    rec_array[REC_ARRAY_SIZE];

    PvrElem_t       pvr;

    int             mount_flg;
} PvrConfig;
typedef PvrConfig* PvrConfig_t;

static int      g_virtual = 0;
static int      g_fill = 1;

static uint32_t g_shift_id = 0;

static void*    g_mutex = NULL;
static void*    g_r_mutex = NULL;
static void*    g_w_mutex = NULL;
static void*    g_rec_mutex[REC_ARRAY_SIZE] = {NULL, NULL, NULL};

PvrStamp        g_stamp;
PvrConfig_t     g_cfg = NULL;
PvrOperation_t  g_op = NULL;

static int int_pvr_hash_insert(PvrElem_t pvr);

static PvrElem_t int_pvr_malloc(void);
static void int_pvr_free(PvrElem_t pvr);
static int int_break_read(PvrElem_t pvr, PvrWInfo_t winfo);
static void int_brk_write(PvrElem_t pvr, PvrWInfo_t winfo);
static int int_brk_read(PvrElem_t pvr);
static int int_proginfo_write_(PvrElem_t pvr);
static int int_proginfo_sum(int fd, uint32_t* psum);

static int int_ifrm_convert(PvrElem_t pvr);
static int int_ifrm_write(PvrRecord_t rec, char* buf, int len);
static int int_ifrm_begin(PvrRecord_t rec, ts_ifrm_t ifrm, uint32_t clk);
static int int_ifrm_end(PvrRecord_t rec, ts_ifrm_t ifrm);

static int int_media_used(PvrElem_t list, int sn);
static void int_media_delete(char* path, int sn);

static int int_pvr_seek(PvrElem_t pvr, int seek, int scale);

static int int_pvr_rec_write(PvrRecord_t rec, char* buf, int len);
static void int_pvr_rec_close(PvrRecord_t rec, int end, uint32_t cls_clk, uint32_t cls_date);

int ind_pvr_init(PvrOperation_t op)
{
    int i;

    if (g_cfg)
        ERR_OUT("already inited\n");

    g_cfg = (PvrConfig_t)IND_CALLOC(sizeof(PvrConfig), 1);
    g_op = (PvrOperation_t)IND_CALLOC(sizeof(PvrOperation), 1);
    IND_MEMCPY(g_op, op, sizeof(PvrOperation));

    g_cfg->context = ind_cfg_context_create(sizeof(PvrSInfo));

    {
        PvrSInfo sinfo;

        ind_cfg_obj_offset(g_cfg->context, "id",            (int)((char*)(&sinfo.id)            - (char*)(&sinfo)), CFG_TYPE_UINT,  0);

        ind_cfg_obj_offset(g_cfg->context, "version",       (int)((char*)(&sinfo.version)       - (char*)(&sinfo)), CFG_TYPE_INT,   0);
        ind_cfg_obj_offset(g_cfg->context, "encrypt",       (int)((char*)(&sinfo.encrypt)       - (char*)(&sinfo)), CFG_TYPE_INT,   0);
        ind_cfg_obj_offset(g_cfg->context, "networkid",     (int)((char*)(&sinfo.networkid)     - (char*)(&sinfo)), CFG_TYPE_INT,   0);
        ind_cfg_obj_offset(g_cfg->context, "record",        (int)((char*)(&sinfo.record)        - (char*)(&sinfo)), CFG_TYPE_INT,   0);
        ind_cfg_obj_offset(g_cfg->context, "breaktype",     (int)((char*)(&sinfo.breaktype)     - (char*)(&sinfo)), CFG_TYPE_INT,   0);
        ind_cfg_obj_offset(g_cfg->context, "pcr",           (int)((char*)(&sinfo.pcr)           - (char*)(&sinfo)), CFG_TYPE_INT,   0);
        ind_cfg_obj_offset(g_cfg->context, "key",           (int)((char*)(&sinfo.key)           - (char*)(&sinfo)), CFG_TYPE_INT64, 0);

        ind_cfg_obj_offset(g_cfg->context, "prev_clks",     (int)((char*)(&sinfo.prev_clks)     - (char*)(&sinfo)), CFG_TYPE_UINT,  0);
        ind_cfg_obj_offset(g_cfg->context, "prev_date",     (int)((char*)(&sinfo.prev_date)     - (char*)(&sinfo)), CFG_TYPE_UINT,  0);

        ind_cfg_obj_offset(g_cfg->context, "time_len",      (int)((char*)(&sinfo.time_len)      - (char*)(&sinfo)), CFG_TYPE_UINT,  0);
        ind_cfg_obj_offset(g_cfg->context, "time_base",     (int)((char*)(&sinfo.time_base)     - (char*)(&sinfo)), CFG_TYPE_UINT,  0);
        ind_cfg_obj_offset(g_cfg->context, "time_bmark",    (int)((char*)(&sinfo.time_bmark)    - (char*)(&sinfo)), CFG_TYPE_UINT,  0);
        ind_cfg_obj_offset(g_cfg->context, "time_length",   (int)((char*)(&sinfo.time_length)   - (char*)(&sinfo)), CFG_TYPE_UINT,  0);

        ind_cfg_obj_offset(g_cfg->context, "fill_num",      (int)((char*)(&sinfo.fill_num)      - (char*)(&sinfo)), CFG_TYPE_INT,   0);

        ind_cfg_obj_offset(g_cfg->context, "base_len",      (int)((char*)(&sinfo.base_len)      - (char*)(&sinfo)), CFG_TYPE_INT,   0);
        ind_cfg_obj_offset(g_cfg->context, "byte_len",      (int)((char*)(&sinfo.byte_len)      - (char*)(&sinfo)), CFG_TYPE_INT64, 0);
        ind_cfg_obj_offset(g_cfg->context, "byte_base",     (int)((char*)(&sinfo.byte_base)     - (char*)(&sinfo)), CFG_TYPE_INT64, 0);
        ind_cfg_obj_offset(g_cfg->context, "byte_rate",     (int)((char*)(&sinfo.byte_rate)     - (char*)(&sinfo)), CFG_TYPE_UINT,  0);
        ind_cfg_obj_offset(g_cfg->context, "byte_bmark",    (int)((char*)(&sinfo.byte_bmark)    - (char*)(&sinfo)), CFG_TYPE_UINT,  0);
        ind_cfg_obj_offset(g_cfg->context, "byte_length",   (int)((char*)(&sinfo.byte_length)   - (char*)(&sinfo)), CFG_TYPE_INT64, 0);
    }

    g_cfg->context_ = ind_cfg_context_create(sizeof(PvrSInfo_));

    {
        PvrSInfo_ sinfo_;

        ind_cfg_obj_offset(g_cfg->context_, "prev_clks",    (int)((char*)(&sinfo_.prev_clks)    - (char*)(&sinfo_)), CFG_TYPE_UINT,     0);
        ind_cfg_obj_offset(g_cfg->context_, "prev_date",    (int)((char*)(&sinfo_.prev_date)    - (char*)(&sinfo_)), CFG_TYPE_UINT,     0);

        ind_cfg_obj_offset(g_cfg->context_, "time_len",     (int)((char*)(&sinfo_.time_len)     - (char*)(&sinfo_)), CFG_TYPE_UINT,     0);
        ind_cfg_obj_offset(g_cfg->context_, "fill_num",     (int)((char*)(&sinfo_.fill_num)     - (char*)(&sinfo_)), CFG_TYPE_INT,      0);

        ind_cfg_obj_offset(g_cfg->context_, "byte_len",     (int)((char*)(&sinfo_.byte_len)     - (char*)(&sinfo_)), CFG_TYPE_INT64,    0);
        ind_cfg_obj_offset(g_cfg->context_, "byte_length",  (int)((char*)(&sinfo_.byte_length)  - (char*)(&sinfo_)), CFG_TYPE_INT64,    0);

        ind_cfg_obj_offset(g_cfg->context_, "record",       (int)((char*)(&sinfo_.record)       - (char*)(&sinfo_)), CFG_TYPE_INT,      0);
        ind_cfg_obj_offset(g_cfg->context_, "crc",          (int)((char*)(&sinfo_.crc)          - (char*)(&sinfo_)), CFG_TYPE_UINT,     0);
    }

    g_op->cfg_param(g_cfg->path, PVR_PATH_LEN, &g_cfg->pvr_size);
    g_cfg->pathlen = strlen(g_cfg->path);

    PRINTF("pvr_size = %d\n", g_cfg->pvr_size);
    g_cfg->pvr_table = (PvrElem_t *)IND_MALLOC(sizeof(PvrElem_t) * g_cfg->pvr_size);
    g_cfg->pvr_num = 0;

    g_cfg->id_array = (uint32_t *)IND_MALLOC(sizeof(uint32_t) * g_cfg->pvr_size);
    g_cfg->id_num = 0;

    g_cfg->share_buf = (char*)IND_MALLOC(PVR_INFO_SIZE);
    g_cfg->share_size = PVR_INFO_SIZE;

    g_mutex = g_op->mutex_create( );
    g_r_mutex = g_op->mutex_create( );
    g_w_mutex = g_op->mutex_create( );

    for (i = 0; i < REC_ARRAY_SIZE; i ++)
        g_rec_mutex[i] = g_op->mutex_create( );

    ts_iparse_regist(int_ifrm_write, int_ifrm_begin, int_ifrm_end);

    return 0;
Err:
    return -1;
}

void ind_pvr_virtual(int flag)
{
    PRINTF("virtual = %d\n", flag);
    g_virtual = flag;
}

#define READ_BUF_SIZE    256
static int int_proginfo_read(int fd, uint32_t* psum)
{
    int i, len;
    uint32_t *uip;
    uint32_t infolen, infosize;
    uint32_t sum;

    len = g_op->file_read(fd, (char *)&infolen, 4);
    if (len != 4)
        ERR_OUT("file_read infolen\n");
    if (infolen > PVR_INFO_SIZE)
        ERR_OUT("infolen = %d\n", infolen);
    sum = infolen;

    g_cfg->r_info_len = infolen;
    infosize = (infolen + 3) & 0xFFFFFFFC;

    len = g_op->file_read(fd, g_cfg->r_info_buf, infosize);
    if (len != infosize)
        ERR_OUT("file_read infosize = %d, len = %d\n", infosize, len);
    uip = (uint32_t*) g_cfg->r_info_buf;
    for (i = 0; i < infosize; i += 4, uip ++)
        sum += *uip;

    if (psum)
        *psum += sum;

    return 0;
Err:
    return -1;
}

static int int_proginfo_read_(PvrElem_t pvr)
{
    int fd, len;
    uint32_t crc;
    PvrSInfo_ sinfo_;
    PvrSInfo_t sinfo = &pvr->s_info;

#ifdef DEBUG_BUILD
    uint32_t id = sinfo->id;
#endif

    fd = -1;

    if (pvr->s_info_sn == -1)
        sprintf(pvr->path + g_cfg->pathlen + 9, "/info_");
    else
        sprintf(pvr->path + g_cfg->pathlen + 9, "/info%d_", pvr->s_info_sn);

    fd = g_op->file_open(pvr->path, O_RDONLY);
    if (fd == -1)
        ERR_OUT("id = %08x file_open %s\n", id, pvr->path);

    if (g_op->file_read(fd, (char *)&len, 4) != 4)
        ERR_OUT("id = %08x file_read len\n", id);
    if (len >= PVR_INFO_SIZE)
        ERR_OUT("id = %08x len = %d\n", id, len);
    if (g_op->file_read(fd, (char *)g_cfg->r_info_buf, len) != len)
        ERR_OUT("id = %08x file_read sinfo_\n", id);

    g_cfg->r_info_buf[len] = 0;
    memset (&sinfo_, 0, sizeof(sinfo_));
    if (ind_cfg_obj_input(g_cfg->context_, &sinfo_, g_cfg->r_info_buf, len))
        ERR_OUT("id = %08x ind_cfg_obj_input\n", id);
    crc = sinfo_.crc;

    sinfo_.crc = 0;
    sinfo_.crc = ind_ts_crc32((uint8_t *)&sinfo_, sizeof(sinfo_));
    if (crc != sinfo_.crc)
        ERR_OUT("id = %08x crc = %08x / %08x\n", id, crc, sinfo_.crc);

    sinfo->prev_clks    = sinfo_.prev_clks;
    sinfo->prev_date    = sinfo_.prev_date;
    
    sinfo->time_len        = sinfo_.time_len;
    sinfo->fill_num        = sinfo_.fill_num;
    
    sinfo->byte_len        = sinfo_.byte_len;
    sinfo->byte_length    = sinfo_.byte_length;

    sinfo->record        = sinfo_.record;

    g_op->file_sync(fd);
    g_op->file_close(fd);

    return 0;
Err:
    if (fd >= 0)
        g_op->file_close(fd);
    return -1;
}

static int int_proginfo_check(int fd, PvrElem_t pvr)
{
    uint32_t sum, readsum, id;
    PvrSInfo_t sinfo;

    sum = 0;
    id = pvr->s_info.id;
    //g_op->file_seek(fp, 0); 多余的操作

    if (int_proginfo_read(fd, &sum))
        ERR_OUT("int_proginfo_read 1\n");

    sinfo = &pvr->s_info;
    IND_MEMSET(sinfo, 0, sizeof(PvrSInfo));
    g_cfg->r_info_buf[g_cfg->r_info_len] = 0;
    if (ind_cfg_obj_input(g_cfg->context, sinfo, g_cfg->r_info_buf, g_cfg->r_info_len))
        ERR_OUT("id = %08x ind_cfg_obj_input\n", id);

    if (pvr->s_info.version < 11) {
        if (sinfo->version < 5)
            ERR_OUT("id = %08x version = %d\n", id, sinfo->version);
        pvr->fragment_time = FRAGMENT_TIME_DEFAULT;
        pvr->fragment_size = (FRAGMENT_TIME_DEFAULT * 100 / IFRAME_CLK_INTERVAL);
    }

    if (int_proginfo_read(fd, &sum))
        ERR_OUT("id = %08x int_proginfo_read 2\n", id);

    pvr->info_buf = (char *)IND_MALLOC(g_cfg->r_info_len);
    if (pvr->info_buf == NULL)
        ERR_OUT("id = %08x malloc info_buf\n", id);
    IND_MEMCPY(pvr->info_buf, g_cfg->r_info_buf, g_cfg->r_info_len);
    pvr->info_len = g_cfg->r_info_len;

    if (g_op->file_read(fd, (char *)&readsum, 4) != 4)
        ERR_OUT("id = %08x file_read infosize\n", id);
    if (sum != readsum)
        ERR_OUT("id = %08x sum = %08x, realsum = %08x\n", id, sum, readsum);

    if (sinfo->version >= 10)
        int_proginfo_read_(pvr);

    if (sinfo->record == 1) {
        if (int_break_read(pvr, NULL))
            ERR_PRN("id = %08x int_break_read\n", id);

        sinfo->record = 0;
        if (int_proginfo_write_(pvr))
            ERR_OUT("id = %08x int_proginfo_write_\n", id);
    }

    return 0;
Err:
    return -1;
}

static int int_break_read(PvrElem_t pvr, PvrWInfo_t winfo)
{
    int fd = -1;
    int result = -1;
    int    fill_num = 0;
    ts_ifrm_t ifrm_buf = NULL;
    PvrSInfo_t sinfo = &pvr->s_info;
    PvrWInfo_t w_info = &g_cfg->w_info;

    uint8_t buf[188];
    uint32_t id, base_id;
    int sn, off, offset, size, minsn, maxsn;
    PvrBPoint bp;

    id = sinfo->id;
    base_id = sinfo->base_id;
    if (0 == base_id)
        base_id = id;

    if (sinfo->breaktype <= 0) {
        sprintf(pvr->path + g_cfg->pathlen + 9, "/%08x.brk", base_id);
        g_op->file_delete(pvr->path);
        IND_MEMSET(w_info, 0, sizeof(PvrWInfo));
    } else {
        int_brk_read(pvr);
    }

    memset(&bp, 0, sizeof(PvrBPoint));

    minsn = sinfo->time_base / pvr->fragment_time;
    maxsn = sinfo->time_len / pvr->fragment_time;
    if (sinfo->record == 1) {
        while (1) {
            sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d", maxsn + 1);
            if (g_op->file_size(pvr->path, &size))
                break;
            maxsn ++;
        }
    } else {
        if (sinfo->time_len == maxsn * pvr->fragment_time && maxsn > 0)
            maxsn --;
    }

    offset = -1;
    for (sn = maxsn; sn >= minsn; sn --) {
        sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d", sn);
        if (g_op->file_size(pvr->path, &size)) {
            WARN_PRN("id = %08x size %s\n", id, pvr->path);
            continue;
        }
        if (size % 188) {
            size -= size % 188;
            if (g_op->file_truncate(pvr->path, size))
                ERR_OUT("id = %08x truncate %s\n", id, pvr->path);
        }

        fd = g_op->file_open(pvr->path, O_RDONLY);
        if (fd == -1)
            ERR_OUT("id = %08x file_open %s\n", id, pvr->path);

        if (g_op->file_seek(fd, size - 188) < 0)
            ERR_OUT("id = %08x file_seek %d %s\n", id, size - 188, pvr->path);
        if (g_op->file_read(fd, (char*)buf, 188) != 188)
            ERR_OUT("id = %08x file_read 188 %s\n", id, pvr->path);

        if (buf[1] == 0x1F && buf[2] == 0xFF) {
            if (buf[6] == 'E') {
                offset = size - (1 + (buf[7] >> 4)) * 188;
                break;
            }
            if (buf[6] == 'P') {
                memcpy(&offset, buf + 8, 4);
                if (offset > 0)
                    break;
                continue;
            }
        }

        PRINTF("id = %08x size = %d / %d\n", id, size, BYTES_PER_BREAK);
        off = size - size % BYTES_PER_BREAK;
        if (sinfo->breaktype > 0 && w_info->w_sn == sn && w_info->w_strm_len >= off && w_info->w_strm_len <= size) {
            offset = w_info->w_break_off;
            break;
        }

        for (; off > 0; off -= BYTES_PER_BREAK) {
            if (g_op->file_seek(fd, off) < 0)
                ERR_OUT("id = %08x file_seek %d %s\n", id, off, pvr->path);
            if (g_op->file_read(fd, (char*)buf, 188) != 188)
                continue;

            if (buf[1] == 0x1F && buf[2] == 0xFF) {
                if (buf[6] == 'P') {
                    memcpy(&offset, buf + 8, 4);
                    break;
                }
                if (buf[6] == 'B') {
                    offset = off;
                    break;
                }
            }
        }
        if (offset > 0)
            break;
    }

    if (offset <= 0) {
        if (fd != -1) {
            g_op->file_close(fd);
            fd = -1;
        }
        g_op->file_delete(pvr->path);
    } else {
        if (g_op->file_seek(fd, offset) < 0)
            ERR_OUT("id = %08x file_seek %d %s\n", id, offset, pvr->path);
        if (g_op->file_read(fd, (char*)buf, 188) != 188)
            ERR_OUT("id = %08x file_read 188 %s\n", id, pvr->path);
        if (buf[1] != 0x1F && buf[2] != 0xFF && buf[6] != 'B')
            ERR_OUT("id = %08x buf[6] = %02x\n", id, (uint32_t)buf[6]);

        memcpy(&bp, buf + 8, sizeof(PvrBPoint));
        if (offset != bp.bp_strm_len)
            ERR_OUT("id = %08x offset = %d, strm_len = %d, %s\n", id, offset, bp.bp_strm_len, pvr->path);
        bp.bp_break_off = offset;
        bp.bp_break_num ++;

        fill_num = 1;

        while (1) {
            fill_num ++;

            if (g_op->file_read(fd, (char*)buf, 188) != 188)
                ERR_OUT("id = %08x file_read 188 %s\n", id, pvr->path);
            if (buf[1] != 0x1F && buf[2] != 0xFF)
                ERR_OUT("id = %08x sync = %02x%02x\n", id, (uint32_t)buf[1], (uint32_t)buf[1]);
            if (buf[6] == 'E')
                break;
            if (buf[6] != 'C')
                ERR_OUT("id = %08x buf[6] = %02x\n", id, (uint32_t)buf[6]);
        }
        bp.bp_fill_num += fill_num;
        bp.bp_strm_len += fill_num * 188;

        g_op->file_close(fd);
        fd = -1;

        off = offset + fill_num * 188;
        if (size > off) {
            WARN_PRN("id = %08x %d > %d\n", id, size, off);
            if (g_op->file_truncate(pvr->path, off))
                ERR_OUT("id = %08x truncate %s\n", id, pvr->path);
        }

        off = bp.bp_ifrm_num * IFRAME_SIZE;
        size = 0;
        sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d.ifr", sn);
        g_op->file_size(pvr->path, &size);
        if (size > off) {
            if (g_op->file_truncate(pvr->path, off))
                ERR_OUT("id = %08x truncate %s\n", id, pvr->path);
        }
        if (size < off) {
            uint32_t n, num;
            PvrBPoint b;
            int break_off;

            num = bp.bp_ifrm_num;

            ifrm_buf = (ts_ifrm_t)IND_MALLOC(IFRAME_SIZE * pvr->fragment_size);
            if (ifrm_buf == NULL)
                ERR_OUT("id = %08x malloc %s\n", id, pvr->path);

            sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d", sn);
            fd = g_op->file_open(pvr->path, O_RDONLY);
            if (fd == -1)
                ERR_OUT("id = %08x file_open %s\n", id, pvr->path);

            break_off = offset;
            while (break_off > 0) {
                if (g_op->file_seek(fd, break_off) < 0)
                    ERR_OUT("id = %08x file_seek %d %s\n", id, break_off, pvr->path);
                if (g_op->file_read(fd, (char*)buf, 188) != 188)
                    ERR_OUT("id = %08x file_read 188 %s\n", id, pvr->path);
                if (buf[1] != 0x1F && buf[2] != 0xFF && buf[6] != 'B')
                    ERR_OUT("id = %08x buf[6] = %02x\n", id, (uint32_t)buf[6]);
                memcpy(&b, buf + 8, sizeof(PvrBPoint));
                break_off = b.bp_break_off;

                off = b.bp_ifrm_num - b.bp_break_ifrm;
                while (1) {
                    if (g_op->file_read(fd, (char*)buf, 188) != 188)
                        ERR_OUT("id = %08x file_read 188 %s\n", id, pvr->path);
                    if (buf[1] != 0x1F && buf[2] != 0xFF)
                        ERR_OUT("id = %08x sync = %02x%02x\n", id, (uint32_t)buf[1], (uint32_t)buf[1]);
                    if (buf[6] != 'C' && buf[6] != 'E')
                        ERR_OUT("id = %08x buf[6] = %02x\n", id, (uint32_t)buf[6]);

                    n = (uint32_t)(buf[7] & 0xf);
                    memcpy(&ifrm_buf[off], buf + 8, IFRAME_SIZE * n);
                    off += n;

                    if (buf[6] == 'E')
                        break;
                }
            }
            g_op->file_close(fd);
            fd = -1;

            sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d.ifr", sn);
            fd = g_op->file_open(pvr->path, FLAGS_WRITE);
            if (fd == -1)
                ERR_OUT("id = %08x file_open %s\n", id, pvr->path);
            size = IFRAME_SIZE * num;
            if (g_op->file_write(fd, (char*)ifrm_buf, size) != size)
                ERR_OUT("id = %08x file_write %s\n", id, pvr->path);
            g_op->file_close(fd);
            fd = -1;
        }
    }

    pvr->slice_num = bp.bp_break_num;
    if (sinfo->breaktype > 0)
        PRINTF("id = %08x slice_num = %d\n", id, pvr->slice_num);

    sinfo->fill_num = bp.bp_fill_num;
    sinfo->byte_len = bp.bp_byte_len + bp.bp_strm_len;
    sinfo->time_len = bp.bp_strm_sn * pvr->fragment_time + bp.bp_ifrm_num * IFRAME_CLK_INTERVAL / 100;

    if (pvr->s_info.breaktype <= 0 || w_info->w_sn != sn || offset != w_info->w_break_off) {
        ts_iparse_t iparse = &w_info->w_iparse;

        w_info->w_sn = bp.bp_strm_sn;

        w_info->w_strm_len = bp.bp_strm_len;

        w_info->w_fill_num = bp.bp_fill_num;
        w_info->w_break_off = bp.bp_break_off;
        w_info->w_break_num = bp.bp_break_num;

        w_info->w_break_ifrm = bp.bp_ifrm_num;

        w_info->w_byte_len = bp.bp_byte_len;

        w_info->w_ifrm_num = bp.bp_ifrm_num;
        w_info->w_ifrm_valid = 0;

        //--------------------------------------------------------
        PRINTF("id = %08x clk_last = %d, clk_base = %d, clk_time = %d, clk_diff = %d\n", id, bp.bp_icntnt.clk_last, bp.bp_icntnt.clk_base, bp.bp_icntnt.clk_time, bp.bp_icntnt.clk_diff);

        iparse->ifrm = bp.bp_ifrm;
        iparse->icntnt = bp.bp_icntnt;

        {
            ts_ifrm_t ifrm = &iparse->ifrm;
            ts_icontent_t icntnt = &iparse->icntnt;

            icntnt->pack_count = bp.bp_strm_len / 188;
            if (sinfo->pcr) {
                if (sinfo->breaktype > 0) {
                    ifrm->ifrm_size = 0;
                } else {
                    if (ifrm->ifrm_size > 0)
                        ifrm->ifrm_size += fill_num;
                } 
            } else {
                if (sinfo->breaktype > 0) {
                    icntnt->frm_state = PARSE_STATE_NONE;
                } else {
                    if (icntnt->frm_state == PARSE_STATE_FRAME || icntnt->frm_state == PARSE_STATE_IFRAME)
                        ifrm->ifrm_size += fill_num;
                } 
            }
        }
        if (sinfo->breaktype > 0)
            int_brk_write(pvr, w_info);
    }

    if (winfo)
        IND_MEMCPY(winfo, w_info, sizeof(PvrWInfo));

    result = 0;
Err:

    if (ifrm_buf)
        IND_FREE(ifrm_buf);
    if (fd != -1)
        g_op->file_close(fd);
    return result;
}

static int int_break_write(PvrRecord_t rec)
{
    uint32_t sn, num, size;
    uint8_t buf[188];

    PvrBPoint bp;
    PvrWInfo_t winfo;
    ts_iparse_t iparse;

    if (rec == NULL)
        ERR_OUT("rec is NULL\n");

#ifdef DEBUG_BUILD
    uint32_t id = rec->id;
#endif

    winfo = &rec->w_info;

    num = winfo->w_ifrm_num - winfo->w_break_ifrm;

    memset(&bp, 0, sizeof(bp));
    bp.bp_taglen = sizeof(bp);

    bp.bp_fill_num = winfo->w_fill_num;
    bp.bp_break_off = winfo->w_break_off;
    bp.bp_break_num = winfo->w_break_num;
    bp.bp_break_ifrm = num;

    bp.bp_byte_len = winfo->w_byte_len;
    bp.bp_strm_len = winfo->w_strm_len;

    bp.bp_strm_sn = winfo->w_sn;
    bp.bp_ifrm_num = winfo->w_ifrm_num;

    iparse = &winfo->w_iparse;

    PRINTF("id = %08x clk_last = %d, clk_base = %d, clk_time = %d, clk_diff = %d\n", id, iparse->icntnt.clk_last, iparse->icntnt.clk_base, iparse->icntnt.clk_time, iparse->icntnt.clk_diff);

    bp.bp_ifrm = iparse->ifrm;
    bp.bp_icntnt = iparse->icntnt;

    bp.bp_cksum = ind_ts_crc32((uint8_t *)&bp, sizeof(bp));

    buf[0] = 0x47;
    buf[1] = 0x1F;
    buf[2] = 0xFF;
    buf[3] = 0x30;
    buf[5] = 0x00;

    sn = 0;
    winfo->w_break_off = winfo->w_strm_len;
    while (num > 0) {
        if (sn == 0) {
            buf[6] = 'B';
            size = sizeof(PvrBPoint);
            buf[7] = size;
            IND_MEMCPY(buf + 8, &bp, size);
        } else {
            uint32_t n = 180 / IFRAME_SIZE;
            if (n > num)
                n = num;
            if (n > 15)
                n = 15;

            if (n >= num)
                buf[6] = 'E';
            else
                buf[6] = 'C';
            size = IFRAME_SIZE * n;
            buf[7] = (sn << 4) | n;

            IND_MEMCPY(buf + 8, &winfo->w_ifrm_buf[winfo->w_ifrm_num - num], size);

            num -= n;
        }

        sn ++;
        {
            uint32_t off;

            buf[4] = size + 3;
            for (off = 8 + size; off < 188; off ++)
                buf[off] = 0xFF;
        }

        if (g_fill) {
            if (int_pvr_rec_write(rec, (char*)buf, 188))
                ERR_OUT("id = %08x int_pvr_rec_write 188\n", id);
            winfo->w_fill_num ++;
        }
    }
    winfo->w_break_ifrm = winfo->w_ifrm_num;

    winfo->w_break_num ++;

    return 0;
Err:
    return -1;
}

#if 0
static int int_iframe_save(PvrRecord_t rec)
{
    uint32_t sn, num, size;
    uint8_t buf[188];

    PvrWInfo_t winfo;

    if (rec == NULL)
        ERR_OUT("rec is NULL\n");

#ifdef DEBUG_BUILD
    uint32_t id = rec->id;
#endif

    winfo = &rec->w_info;

    num = IFRAME_PER_FRAGMENT - winfo->w_break_ifrm;

    buf[0] = 0x47;
    buf[1] = 0x1F;
    buf[2] = 0xFF;
    buf[3] = 0x30;
    buf[5] = 0x00;

    sn = 0;
    while (num > 0) {
        uint32_t n = 180 / IFRAME_SIZE;
        if (n > num)
            n = num;
        if (n > 15)
            n = 15;

        if (n >= num)
            buf[6] = 'F';
        else
            buf[6] = 'C';
        size = IFRAME_SIZE * n;
        buf[7] = (sn << 4) | n;

        IND_MEMCPY(buf + 8, &winfo->w_ifrm_buf[winfo->w_ifrm_num - num], size);

        num -= n;

        sn ++;
        {
            uint32_t off;

            buf[4] = size + 3;
            for (off = 8 + size; off < 188; off ++)
                buf[off] = 0xFF;
        }

        if (g_fill) {
            if (int_pvr_rec_write(rec, (char*)buf, 188))
                ERR_OUT("id = %08x int_pvr_rec_write 188\n", id);
            winfo->w_fill_num ++;
        }
    }

    return 0;
Err:
    return -1;
}
#endif

static int int_iframe_restore(PvrElem_t pvr, int sn)
{
    int fd = -1;

    PvrRInfo_t rinfo = pvr->r_info;
    PvrSInfo_t sinfo = &pvr->s_info;

    uint8_t buf[188];
    uint32_t id;
    int num, off, offset, size;
    PvrBPoint bp;

    id = sinfo->id;

    PRINTF("%08x sn = %d\n", id, sn);

    if (sinfo->version < 10)//version = 10，记录断点
        ERR_OUT("%08x version = %d\n", id, sinfo->version);

    IND_MEMSET(rinfo->r_ifrm_buf, 0, IFRAME_SIZE * pvr->fragment_size);

    sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d", sn);
    if (g_op->file_size(pvr->path, &size))
        ERR_OUT("id = %08x file_size %s\n", id, pvr->path);
    if (size % 188)
        size -= size % 188;

    fd = g_op->file_open(pvr->path, O_RDONLY);
    if (fd == -1)
        ERR_OUT("id = %08x file_open %s\n", id, pvr->path);

    offset = -1;
    for (off = size - size % BYTES_PER_BREAK; off > 0; off -= BYTES_PER_BREAK) {
        if (g_op->file_seek(fd, off) < 0)
            ERR_OUT("id = %08x file_seek %d %s\n", id, off, pvr->path);
        if (g_op->file_read(fd, (char*)buf, 188) != 188)
            ERR_OUT("id = %08x file_read 188 %s\n", id, pvr->path);

        if (buf[1] == 0x1F && buf[2] == 0xFF) {
            if (buf[6] == 'P') {
                memcpy(&offset, buf + 8, 4);
                break;
            }
            if (buf[6] == 'B') {
                offset = off;
                break;
            }
        }
    }

    num = 0;
    off = offset;
    while (offset > 0) {
        int n;

        if (g_op->file_seek(fd, offset) < 0)
            ERR_OUT("id = %08x file_seek %d %s\n", id, offset, pvr->path);
        if (g_op->file_read(fd, (char*)buf, 188) != 188)
            ERR_OUT("id = %08x file_read 188 %s\n", id, pvr->path);
        if (buf[1] != 0x1F && buf[2] != 0xFF && buf[6] != 'B')
            ERR_OUT("id = %08x buf[6] = %02x\n", id, (uint32_t)buf[6]);
        memcpy(&bp, buf + 8, sizeof(PvrBPoint));

        if (bp.bp_break_off > offset)//老版本存在这个bug，bp_break_off是上一个分片的残留
            offset = 0;
        else
            offset = bp.bp_break_off;
        if (0 == num)
            num = bp.bp_ifrm_num;
        off = bp.bp_ifrm_num - bp.bp_break_ifrm;
        while (1) {
            if (g_op->file_read(fd, (char*)buf, 188) != 188)
                ERR_OUT("id = %08x file_read 188 %s\n", id, pvr->path);
            if (buf[1] != 0x1F && buf[2] != 0xFF)
                ERR_OUT("id = %08x sync = %02x%02x\n", id, (uint32_t)buf[1], (uint32_t)buf[1]);
            if (buf[6] != 'C' && buf[6] != 'E')
                ERR_OUT("id = %08x buf[6] = %02x\n", id, (uint32_t)buf[6]);

            n = (uint32_t)(buf[7] & 0xf);
            memcpy(&rinfo->r_ifrm_buf[off], buf + 8, IFRAME_SIZE * n);
            off += n;

            if (buf[6] == 'E')
                break;
        }
    }

    return num;
Err:
    if (fd != -1)
        g_op->file_close(fd);
    return 0;
}

static int int_break_point(PvrRecord_t rec)
{
    uint32_t i;
    uint8_t buf[188];

    PvrWInfo_t winfo = &rec->w_info;

    if (winfo->w_fill_num <= 0)
        return 0;

    buf[0] = 0x47;
    buf[1] = 0x1F;
    buf[2] = 0xFF;
    buf[3] = 0x30;
    buf[4] = 3 + 4;
    buf[5] = 0x00;
    buf[6] = 'P';
    buf[7] = 0x04;

    IND_MEMCPY(buf + 8, &winfo->w_break_off, 4);
    for (i = 16; i < 188; i ++)
        buf[i] = 0xFF;
    if (g_fill) {
        if (int_pvr_rec_write(rec, (char*)buf, 188))
            ERR_OUT("id = %08x int_pvr_rec_write!\n", rec->id);
        winfo->w_fill_num ++;
    }

    return 0;
Err:
    return -1;
}

static void int_brk_write(PvrElem_t pvr, PvrWInfo_t winfo)
{
    int fd = -1;
    uint32_t id, base_id;

    id = pvr->s_info.id;
    base_id = pvr->s_info.base_id;
    if (0 == base_id)
        base_id = id;

    winfo->version = PVR_VERSION;//10
    winfo->crc32 = 0;
    winfo->crc32 = ind_ts_crc32((uint8_t *)winfo, sizeof(PvrWInfo));

    sprintf(pvr->path + g_cfg->pathlen + 9, "/%08x.brk", base_id);
    fd = g_op->file_open(pvr->path, FLAGS_WRITE);
    if (fd < 0)
        ERR_OUT("id = %08x file_open %s\n", id, pvr->path);

    g_op->file_write(fd, (char *)winfo, sizeof(PvrWInfo));
Err:
    if (fd >= 0)
        g_op->file_close(fd);
}

static int int_brk_read(PvrElem_t pvr)
{
    PvrWInfo_t w_info = &g_cfg->w_info;
    int size, fd;
    uint32_t crc0, crc1, id, base_id;

    id = pvr->s_info.id;
    base_id = pvr->s_info.base_id;
    if (0 == base_id)
        base_id = id;
    sprintf(pvr->path + g_cfg->pathlen + 9, "/%08x.brk", base_id);
    size = 0;
    g_op->file_size(pvr->path, &size);
    if (size != sizeof(PvrWInfo))
        ERR_OUT("id = %08x file_size %d / %d, %s\n", id, size, sizeof(PvrWInfo), pvr->path);

    fd = g_op->file_open(pvr->path, O_RDONLY);
    if (fd < 0)
        ERR_OUT("id = %08x file_open %s\n", id, pvr->path);


    size = g_op->file_read(fd, (char *)w_info, sizeof(PvrWInfo));
    g_op->file_close(fd);

    if (size != sizeof(PvrWInfo))
        ERR_OUT("id = %08x file_read %d / %d\n", id, size, sizeof(PvrWInfo));

    crc0 = w_info->crc32;
    w_info->crc32 = 0;
    crc1 = ind_ts_crc32((uint8_t *)w_info, sizeof(PvrWInfo));
    if (crc0 != crc1)
        ERR_OUT("id = %08x crc %08x / %08x\n", id, crc0, crc1);

    return 0;
Err:
    IND_MEMSET(w_info, 0, sizeof(PvrWInfo));
    return -1;
}

static int int_brk_check(PvrElem_t pvr, PvrWInfo_t winfo)
{
    PvrWInfo_t w_info = &g_cfg->w_info;
    int size, len;

#ifdef DEBUG_BUILD
    uint32_t id = pvr->s_info.id;
#endif

    if (int_brk_read(pvr))
        ERR_OUT("id = %08x int_brk_read_winfo\n", id);

    sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d", w_info->w_sn);
    size = 0;
    if (g_op->file_size(pvr->path, &size))
        ERR_OUT("id = %08x file_size %d / %d / %s\n", id, size, sizeof(PvrWInfo), pvr->path);

    len = w_info->w_strm_len;
    if (len != size) {
        if (len < size) {
            PRINTF("id = %08x truncate %d > %d %s\n", id, size, len, pvr->path);
            if (g_op->file_truncate(pvr->path, len))
                ERR_OUT("id = %08x truncate %s\n", id, pvr->path);
        } else {
            ERR_OUT("id = %08x media size %d / %d\n", id, size, w_info->w_strm_len);
        }
    }

    sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d.ifr", w_info->w_sn);
    size = 0;
    if (g_op->file_size(pvr->path, &size))
        ERR_OUT("id = %08x file_size %d / %d / %s\n", id, size, sizeof(PvrWInfo), pvr->path);

    len = w_info->w_ifrm_num * IFRAME_SIZE;
    if (len != size) {
        if (len < size) {
            PRINTF("id = %08x truncate %d > %d %s\n", id, size, len, pvr->path);
            if (g_op->file_truncate(pvr->path, len))
                ERR_OUT("id = %08x truncate %s\n", id, pvr->path);
        } else {
            ERR_OUT("id = %08x ifrm_num = %d / size = %d\n", id, w_info->w_ifrm_num, size);
        }
    }

    if (winfo)
        IND_MEMCPY(winfo, w_info, sizeof(PvrWInfo));

    return 0;
Err:
    return -1;
}

PvrElem_t int_pvr_create(int fd, uint32_t base_id, int info_sn)
{
    PvrElem_t pvr = NULL;

    pvr = int_pvr_malloc( );
    if (pvr == NULL)
        ERR_OUT("int_pvr_malloc\n");

    pvr->s_info_sn = info_sn;
    sprintf(pvr->path, "%s/%08x", g_cfg->path, base_id);

    if (int_proginfo_check(fd, pvr))
        ERR_OUT("int_proginfo_check\n");

    pvr->s_info.base_id = base_id;
    int_pvr_hash_insert(pvr);

    return pvr;
Err:
    if (pvr)
        int_pvr_free(pvr);
    return NULL;
}

static void int_list_insert(PvrElem_t list, PvrElem_t pvr)
{
    PvrElem_t elem;

    elem = list->list_first;
    pvr->list_next = elem;
    while (elem) {
        elem->list_first = pvr;
        elem = elem->list_next;
    }
    pvr->list_first = pvr;
}

static PvrElem_t int_list_remove(PvrElem_t pvr)
{
    PvrElem_t list, elem;

    list = pvr->list_first;
    if (list == NULL)
        return NULL;

    if (list == pvr) {
        list = pvr->list_next;
        elem = list;
        while (elem) {
            elem->list_first = list;
            elem = elem->list_next;
        }
    } else {
        elem = list;
        while (elem->list_next && elem->list_next != pvr)
            elem = elem->list_next;

        if (elem->list_next != pvr)
            ERR_OUT("list_next = %p, pvr = %p!\n", elem->list_next, pvr);

        elem->list_next = pvr->list_next;
    }

    pvr->list_first = pvr;

    return list;
Err:
    pvr->list_first = pvr;
    pvr->list_next = NULL;
    return NULL;
}

static int int_list_write(PvrElem_t pvr)
{
    PvrElem_t elem;
#ifdef DEBUG_BUILD
    uint32_t id = pvr->s_info.id;
#endif

    uint32_t sum;
    int *p, num;
    int fd = -1;
    int result = -1;

    p = (int *)g_cfg->w_info_buf;

    p[0] = 0;//version
    if (pvr->w_rec)
        p[1] = 1;
    else
        p[1] = 0;

    num = 0;
    elem = pvr->list_first;

    while (elem && num < PVR_INFO_SIZE / 4 - 3) {
        if (elem->s_info.breaktype != -1) {
            p[3 + num] = elem->s_info_sn;
            num ++;
        }
        elem = elem->list_next;
    }
    if (num == 0) {
        p[3] = pvr->s_info_sn;
        num = 1;
    }
    p[2] = num;
    g_cfg->w_info_len = (num + 3) * 4;

    sprintf(pvr->path + g_cfg->pathlen + 9, "/info");
    fd = g_op->file_open(pvr->path, FLAGS_WRITE);
    if (fd < 0)
        ERR_OUT("id = %08x file_open = %s\n", id, pvr->path);

    if (g_op->file_write(fd, "LIST", 4) != 4)
        ERR_OUT("id = %08x file_write LIST\n", id);

    sum = 0;
    if (int_proginfo_sum(fd, &sum))
        ERR_OUT("id = %08x int_proginfo_read 1\n", id);

    if (g_op->file_write(fd, (char *)&sum, 4) != 4)
        ERR_OUT("id = %08x file_write sum\n", id);

    result = 0;
Err:
    if (fd >= 0) {
        g_op->file_sync(fd);
        g_op->file_close(fd);
    }
    return result;
}

static int int_list_create(int fd, char* path, uint32_t base_id)
{
    PvrElem_t pvr, list;
    int *p, array[64], i, num, valid, recording;

    valid = 0;

    {
        uint32_t sum, readsum;

        sum = 0;
        if (int_proginfo_read(fd, &sum))
            ERR_OUT("int_proginfo_read 1\n");

        if (g_op->file_read(fd, (char *)&readsum, 4) != 4)
            ERR_OUT("id = %08x file_read\n", base_id);
        if (sum != readsum)
            ERR_OUT("id = %08x sum = %08x, realsum = %08x\n", base_id, sum, readsum);
    }

    if (g_cfg->r_info_len < 16)
        ERR_OUT("id = %08x info_len = %d\n", base_id, g_cfg->r_info_len);
    p = (int *)g_cfg->r_info_buf;
/*{
    for (i = 0; i < g_cfg->r_info_len / 4; i ++)
        PRINTF("==========[%d] = %08x\n", i, p[i]);
}*/
    //version = array[0];
    recording = p[1];
    num = (g_cfg->r_info_len - 12) / 4;
    if (num != p[2])
        ERR_OUT("id = %08x num = %d / %d\n", base_id, num, p[2]);

    if (num > 64)
        num = 64;
    for (i = 0; i < num; i ++)
        array[i] = p[3 + i];

    list = NULL;
    for (i = 0; i < num; i ++) {
        int fd;

        sprintf(path + g_cfg->pathlen + 9, "/info%d", array[i]);
        fd = g_op->file_open(path, O_RDONLY);
        if (fd >= 0) {
            pvr = int_pvr_create(fd, base_id, array[i]);
            if (pvr)
                valid = 1;
            if (list)
                int_list_insert(list, pvr);
            list = pvr;
            g_op->file_close(fd);
        }
    }

    if (NULL == list)
        goto Err;

    if (recording == 1) {
        void* dir;
        int sn;
        char file[PVR_PATH_LEN];

        int_list_write(list);//recording修改为0并回写

        path[g_cfg->pathlen + 9] = 0;
        dir = g_op->dir_open(path);
        if (dir == NULL)
            ERR_OUT("dir_open\n");

        num = 0;

        while(1) {
            if (g_op->dir_read(dir, file)) {
                ERR_PRN("dir_read\n");
                break;
            }
            if (file[0] == 0)
                break;

            if (strncmp(file, "media", 5) == 0) {
                sn = atoi(file + 5);
                if (0 == int_media_used(list, sn) && num < 64) {
                    array[num] = sn;
                    num ++;
                }
            }
        }
        g_op->dir_close(dir);

        for (i = 0; i < num; i ++)
            int_media_delete(path, array[i]);
    }

Err:
    return valid;

}

static int int_dir_check(char* name)
{
    int fd, valid;
    uint32_t id;
    char path[PVR_PATH_LEN];

    id = -1;
    if (ind_str8tohex(name, &id))
        ERR_OUT("ind_str8tohex %s\n", name);
    if (id == -1)
        return -1;

    if (id < 10)
        return 0;

    valid = 0;
    sprintf(path, "%s/%s/info", g_cfg->path, name);
    fd = g_op->file_open(path, O_RDONLY);
    if (fd >= 0) {
        char data[4];
        IND_MEMSET(data, 0, 4);

        g_op->file_read(fd, data, 4);

        if (memcmp(data, "LIST", 4) == 0) {
            valid = int_list_create(fd, path, id);
        } else {
            g_op->file_seek(fd, 0);
            if (int_pvr_create(fd, id, -1))
                valid = 1;
        }
        g_op->file_close(fd);
    }

    if (valid == 0) {
        sprintf(path, "%s/%s", g_cfg->path, name);
        g_op->dir_delete(path);
    }

    return 0;
Err:
    if (id != -1)
        g_op->strm_error(id);
    return -1;
}

int ind_pvr_mount(uint32_t clk)
{
    int i, result = -1;

    uint32_t size, space;
    void* rootdir;
    char name[PVR_PATH_LEN];

    if (g_op == NULL)
        return -1;

    if (g_cfg->pvr_num > 0)
        ind_pvr_unmount(clk);

    g_cfg->mount_flg = 1;
    PRINTF("\n");

    g_op->cfg_param(g_cfg->path, PVR_PATH_LEN, &g_cfg->pvr_size);
    g_cfg->pathlen = strlen(g_cfg->path);

    if (g_op->disk_size(&size, &space)) {
        ERR_PRN("disk_szie\n");
        return -1;
    }

    WRITE_MUTEX_LOCK( );
    for (i = 0; i < REC_ARRAY_SIZE; i ++)
        g_op->mutex_lock(g_rec_mutex[i], __FUNCTION__, __LINE__);
    READ_MUTEX_LOCK( );
    MUTEX_LOCK( );

    g_op->strm_key(&g_cfg->key);
    PRINTF("key = %lld\n", g_cfg->key);

    rootdir = g_op->dir_open(g_cfg->path);

    if (rootdir == NULL) {
        if (g_op->dir_create(g_cfg->path))
            ERR_OUT("dir_create\n");
        rootdir = g_op->dir_open(g_cfg->path);
        if (rootdir == NULL)
            ERR_OUT("dir_open\n");
    }

    while(1) {
        if (g_op->dir_read(rootdir, name)) {
            ERR_PRN("dir_read\n");
            break;
        }

        if (name[0] == 0)
            break;
        PRINTF("dirname = %s\n", name);
        if ("." == name[0])// . or ..
            continue;

        if (int_dir_check(name)) 
            WARN_PRN("int_dir_check %s\n", name);
    }
    g_op->dir_close(rootdir);

    sprintf(name, "%s/%s", g_cfg->path,SHARE_PATH_NAME);
    g_op->dir_delete(name);
    g_op->dir_create(name);

    result = 0;
Err:
    MUTEX_UNLOCK( );
    READ_MUTEX_UNLOCK( );
    for (i = REC_ARRAY_SIZE - 1; i >= 0; i --)
        g_op->mutex_unlock(g_rec_mutex[i]);
    WRITE_MUTEX_UNLOCK( );

    g_op->file_sync(-1);
    PRINTF("file_sync -1\n");

    g_cfg->mount_flg = 0;

    return result;
}

//------------------------------------------------------------------------------
static PvrElem_t int_pvr_hash_find(uint32_t id)
{
    PvrElem_t pvr;
    uint32_t idx;

    idx = (id >> 1) & 0xff;
    pvr = g_cfg->pvr_hash[idx];
    while(pvr && id != pvr->s_info.id)
        pvr = pvr->next;

    return pvr;
}

static int int_pvr_hash_insert(PvrElem_t pvr)
{
    int i;
    PvrElem_t tmp;
    uint32_t idx;
    uint32_t id = pvr->s_info.id;

    if (g_cfg->pvr_num >= g_cfg->pvr_size)
        ERR_OUT("id = %08x pvr_num = %d, pvr_size = %d\n", id, g_cfg->pvr_num, g_cfg->pvr_size);

    if (pvr->index != -1)
        ERR_OUT("id = %08x already insert table!\n", id);

    if (pvr->s_info.breaktype == -1) {
        pvr->index = 0;
    } else {
        for (i = g_cfg->pvr_num; i > 0; i --) {
            tmp = g_cfg->pvr_table[i - 1];
            if (tmp->s_info.id <= pvr->s_info.id)
                break;
            g_cfg->pvr_table[i] = tmp;
            tmp->index = i;
        }
        g_cfg->pvr_table[i] = pvr;
        pvr->index = i;
        g_cfg->pvr_num ++;
    }

    idx = (id >> 1) & 0xff;
    pvr->next = g_cfg->pvr_hash[idx];
    g_cfg->pvr_hash[idx] = pvr;

    return 0;
Err:
    return -1;
}

static int int_pvr_hash_remove(PvrElem_t pvr)
{
    PvrElem_t temp, prev;
    uint32_t idx, id;

    id = pvr->s_info.id;

    if (pvr->s_info.breaktype == -1) {
        if (pvr->index != 0)
            ERR_OUT("id = %08x index = %d\n", id, pvr->index);
    } else {
        int i = pvr->index;
        if (i < 0 || i >= g_cfg->pvr_num)
            ERR_OUT("id = %08x i = %d, pvr_num = %d\n", id, i, g_cfg->pvr_num);

        g_cfg->pvr_num --;
        for (; i < g_cfg->pvr_num; i ++) {
            temp = g_cfg->pvr_table[i + 1];
            temp->index = i;
            g_cfg->pvr_table[i] = temp;
        }
    }
    pvr->index = -1;

    idx = (id >> 1) & 0xff;
    prev = NULL;
    temp = g_cfg->pvr_hash[idx];
    while(temp && temp != pvr) {
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL)
        ERR_OUT("id = %08x not exist\n", id);
    if (prev)
        prev->next = pvr->next;
    else
        g_cfg->pvr_hash[idx] = pvr->next;

    return 0;
Err:
    return -1;
}

static PvrRecord_t int_rec_malloc(void)
{
    int index;
    PvrRecord_t rec = NULL;

    MUTEX_LOCK( );

    if (g_cfg->mount_flg)
        ERR_OUT("mounting\n");

    for (index = 0; index < REC_ARRAY_SIZE; index ++) {
        if (g_cfg->rec_array[index] == NULL)
            break;
    }
    if (index >= REC_ARRAY_SIZE)
        ERR_OUT("index = %d\n", index);

    rec = (PvrRecord_t)IND_CALLOC(sizeof(struct PvrRecord), 1);
    if (rec == NULL)
        ERR_OUT("malloc PvrRecord\n");

    rec->index = index;
    rec->mutex = g_rec_mutex[index];
    rec->w_info.w_strm_fd = -1;
    rec->w_info.w_ifrm_fd = -1;

    g_cfg->rec_array[index] = rec;

Err:
    MUTEX_UNLOCK( );
    return rec;
}

static void int_rec_free(PvrRecord_t rec)
{
    int index;

    MUTEX_LOCK( );

    index = rec->index;
    if (index < 0 || index >= REC_ARRAY_SIZE)
        ERR_OUT("index = %d\n", index);

    if (rec != g_cfg->rec_array[index])
        ERR_OUT("index = %d rec = %p / %p\n", index, rec, g_cfg->rec_array[index]);

    g_cfg->rec_array[index] = NULL;
    IND_FREE(rec);

Err:
    MUTEX_UNLOCK( );
}

static PvrRecord_t int_rec_get(int index)
{
    PvrRecord_t rec = NULL;

    MUTEX_LOCK( );

    if (g_cfg->mount_flg)
        ERR_OUT("mounting\n");
    if (index < 0 || index >= REC_ARRAY_SIZE)
        goto Err;

    rec = g_cfg->rec_array[index];
Err:
    MUTEX_UNLOCK( );
    return rec;
}

static void int_rec_close_write(PvrRecord_t rec)
{
    PvrWInfo_t winfo = &rec->w_info;

    if (winfo->w_strm_fd != -1) {
        g_op->file_close(winfo->w_strm_fd);
        winfo->w_strm_fd = -1;
    }
    if (winfo->w_ifrm_fd != -1) {
        g_op->file_close(winfo->w_ifrm_fd);
        winfo->w_ifrm_fd = -1;
    }
}

static void int_pvr_close_read(PvrElem_t pvr)
{
    PvrRInfo_t rinfo = pvr->r_info;

    if (rinfo) {
        if (rinfo->r_strm_fd != -1) {
            g_op->file_close(rinfo->r_strm_fd);
            rinfo->r_strm_fd = -1;
        }
        ts_pcr_delete(rinfo->r_tspcr);
        IND_FREE(rinfo);
        pvr->r_info = NULL;
    }

    g_cfg->pvr = NULL;
}

static PvrElem_t int_pvr_malloc(void)
{
    PvrElem_t pvr;

    pvr = (PvrElem_t)IND_CALLOC(sizeof(struct PvrElem), 1);
    if (pvr == NULL)
        ERR_OUT("malloc!\n");

    pvr->list_first = pvr;

    pvr->s_info.version = PVR_VERSION;
    pvr->fragment_time = FRAGMENT_TIME_CURRENT;
    pvr->fragment_size = (FRAGMENT_TIME_CURRENT * 100 / IFRAME_CLK_INTERVAL);

    pvr->s_info.key = g_cfg->key;

    pvr->s_info_sn = -1;

    pvr->index = -1;

    return pvr;
Err:
    return NULL;
}

static void int_pvr_free(PvrElem_t pvr)
{
    if (pvr->index != -1)
        int_pvr_hash_remove(pvr);

    if (pvr->info_buf) {
        IND_FREE(pvr->info_buf);
        pvr->info_buf = NULL;
    }

    IND_FREE(pvr);
}

void ind_pvr_unmount(uint32_t clk)
{
    int i;

    if (g_op == NULL)
        return;

    PRINTF("\n");

    MUTEX_LOCK( );
    g_cfg->mount_flg = 1;
    MUTEX_UNLOCK( );

    WRITE_MUTEX_LOCK( );
    for (i = 0; i < REC_ARRAY_SIZE; i ++)
        g_op->mutex_lock(g_rec_mutex[i], __FUNCTION__, __LINE__);
    {
        PvrRecord_t rec, *rec_array;

        rec_array = g_cfg->rec_array;
        for (i = 0; i < REC_ARRAY_SIZE; i ++) {
            rec = rec_array[i];
            if (rec) {
                int_pvr_rec_close(rec, 0, clk, 0);
                int_rec_free(rec);
            }
        }
    }

    READ_MUTEX_LOCK( );
    if (g_cfg->pvr) {
        int_pvr_close_read(g_cfg->pvr);
        g_cfg->pvr = NULL;
    }
    READ_MUTEX_UNLOCK( );

    {
        PvrElem_t pvr;
        for (i = 0; i < g_cfg->pvr_num; i ++) {
            pvr = g_cfg->pvr_table[i];
            int_pvr_free(pvr);
        }
        g_cfg->pvr_num = 0;
    }

    for (i = 0; i < 256; i ++)
        g_cfg->pvr_hash[i] = NULL;

    for (i = REC_ARRAY_SIZE - 1; i >= 0; i --)
        g_op->mutex_unlock(g_rec_mutex[i]);
    WRITE_MUTEX_UNLOCK( );

    MUTEX_LOCK( );
    g_cfg->mount_flg = 0;
    MUTEX_UNLOCK( );
}

static int int_proginfo_sum(int fd, uint32_t* psum)
{
    int i, len;
    uint32_t *uip;
    uint32_t infolen, infosize;
    uint32_t sum;

    infolen = g_cfg->w_info_len;
    if (infolen < 0 || infolen > PVR_INFO_SIZE)
        ERR_OUT("infolen = %d\n", infolen);

    if (g_op->file_write(fd, (char *)&infolen, 4) != 4)
        ERR_OUT("file_write infolen\n");
    sum = infolen;

    infosize = (infolen + 3) & 0xFFFFFFFC;

    len = g_op->file_write(fd, g_cfg->w_info_buf, infosize);
    if (len != infosize)
        ERR_OUT("file_write infosize = %d, len = %d\n", infosize, len);
    uip = (uint32_t*)g_cfg->w_info_buf;
    for (i = 0; i < infosize; i += 4, uip ++)
        sum += *uip;

    if (psum)
        *psum += sum;

    return 0;
Err:
    return -1;
}

static int int_proginfo_write(PvrElem_t pvr)
{
    int result = -1;
    int fd = -1;
    uint32_t sum;
    PvrSInfo_t sinfo = &pvr->s_info;
#ifdef DEBUG_BUILD
    uint32_t id = sinfo->id;
#endif

    if (pvr->info_len <= 0) {
        PRINTF("id = %08x info_len = %d\n", id, pvr->info_len);
        return 0;
    }

    if (pvr->s_info_sn == -1) {
        sprintf(pvr->path + g_cfg->pathlen + 9, "/info");
    } else {
        if (int_list_write(pvr))
            ERR_OUT("id = %08x int_list_write\n", id);
        sprintf(pvr->path + g_cfg->pathlen + 9, "/info%d", pvr->s_info_sn);
    }

    fd = g_op->file_open(pvr->path, FLAGS_WRITE);
    if (fd == -1)
        ERR_OUT("id = %08x file_open %s\n", id, pvr->path);

    sum = 0;

    g_cfg->w_info_len = ind_cfg_obj_output(g_cfg->context, sinfo, g_cfg->w_info_buf, PVR_INFO_SIZE);
    if (g_cfg->w_info_len <= 0 || g_cfg->w_info_len >= PVR_INFO_SIZE)
        ERR_OUT("id = %08x ind_cfg_obj_output ret = %d\n", id, g_cfg->w_info_len);

    if (int_proginfo_sum(fd, &sum))
        ERR_OUT("id = %08x int_proginfo_read 1\n", id);

    IND_MEMCPY(g_cfg->w_info_buf, pvr->info_buf, pvr->info_len);
    g_cfg->w_info_len = pvr->info_len;

    if (int_proginfo_sum(fd, &sum))
        ERR_OUT("id = %08x int_proginfo_read 1\n", id);

    if (g_op->file_write(fd, (char *)&sum, 4) != 4)
        ERR_OUT("id = %08x file_write sum\n", id);

    result = 0;
Err:
    if (fd != -1) {
        g_op->file_sync(fd);
        g_op->file_close(fd);
    }

    return result;
}

static int int_proginfo_write_(PvrElem_t pvr)
{
    int result = -1;
    int fd = -1;
    int len;
    PvrSInfo_t sinfo = &pvr->s_info;
    PvrSInfo_ sinfo_;
#ifdef DEBUG_BUILD
    uint32_t id = sinfo->id;
#endif

    if (pvr->s_info_sn == -1)
        sprintf(pvr->path + g_cfg->pathlen + 9, "/info_");
    else
        sprintf(pvr->path + g_cfg->pathlen + 9, "/info%d_", pvr->s_info_sn);

    fd = g_op->file_open(pvr->path, FLAGS_WRITE);
    if (fd == -1)
        ERR_OUT("id = %08x file_open %s\n", id, pvr->path);

    memset (&sinfo_, 0, sizeof(sinfo_));

    sinfo_.prev_clks    = sinfo->prev_clks;
    sinfo_.prev_date    = sinfo->prev_date;

    sinfo_.time_len        = sinfo->time_len;
    sinfo_.fill_num        = sinfo->fill_num;

    sinfo_.byte_len        = sinfo->byte_len;
    sinfo_.byte_length    = sinfo->byte_length;

    sinfo_.record        = sinfo->record;

    sinfo_.crc = 0;
    sinfo_.crc = ind_ts_crc32((uint8_t *)&sinfo_, sizeof(sinfo_));

    len = ind_cfg_obj_output(g_cfg->context_, &sinfo_, g_cfg->w_info_buf, PVR_INFO_SIZE);
    if (len <= 0 || len >= PVR_INFO_SIZE)
        ERR_OUT("id = %08x ind_cfg_obj_output ret = %d\n", id, len);

    if (g_op->file_write(fd, (char *)&len, 4) != 4)
        ERR_OUT("id = %08x file_write len\n", id);
    if (g_op->file_write(fd, g_cfg->w_info_buf, len) != len)
        ERR_OUT("id = %08x file_write sinfo_\n", id);

    result = 0;
Err:
    if (fd != -1) {
        g_op->file_sync(fd);
        g_op->file_close(fd);
    }

    return result;
}

static int int_media_used(PvrElem_t list, int sn)
{
    PvrSInfo_t sinfo;
    int sn_min, sn_max, used = 0;

    while (list) {
        sinfo = &list->s_info;

        sn_min = sinfo->time_base / list->fragment_time;
        sn_max = sinfo->time_len / list->fragment_time;

        if (1 == sinfo->record) {
            PvrRecord_t rec = list->w_rec;

            if (list == rec->shift.pvr && sinfo->time_base + sinfo->time_length < sinfo->time_len)
                sn_min = (sinfo->time_len - sinfo->time_length) / list->fragment_time;

            if (sn >= sn_min) {
                used = 1;
                break;
            }
        } else {
            if (sn >= sn_min && sn <= sn_max) {
                used = 1;
                break;
            }
        }
        list = list->list_next;
    }

    return used;
}

static void int_media_delete(char* path, int sn)
{
    sprintf(path + g_cfg->pathlen + 9, "/media%d", sn);
    g_op->file_delete(path);
    sprintf(path + g_cfg->pathlen + 9, "/media%d.ifr", sn);
    g_op->file_delete(path);
}

static PvrElem_t int_pvr_mix_open(PvrRecord_t rec, PvrArgument_t arg, uint32_t clk)
{
    uint32_t id;
    PvrElem_t list, pvr = NULL;
    PvrWInfo_t winfo;
    PvrSInfo_t sinfo;

    winfo = &rec->w_info;

    id = arg->id;
    PRINTF("-------- id = %08x arg = %08x\n", id, rec->id);

    if (arg->time_shift > 0 && rec->shift.pvr)
        ERR_OUT("id = %08x shift used!\n", id);

    pvr = int_pvr_malloc( );
    if (pvr == NULL)
        ERR_OUT("id = %08x int_pvr_malloc\n", id);

    sinfo = &pvr->s_info;

    if (arg->time_shift <= 0) {
        if (arg->info_len > 0) {
            if (arg->info_len >= PVR_INFO_SIZE)
                ERR_OUT("id = %08x info_len = %d\n", id, arg->info_len);
            pvr->info_buf = (char *)IND_MALLOC(arg->info_len);
            if (pvr->info_buf == NULL)
                ERR_OUT("id = %08x malloc info_buf\n", id);
            IND_MEMCPY(pvr->info_buf, arg->info_buf, arg->info_len);
        }
        pvr->info_len = arg->info_len;
    }

    sinfo->id = arg->id;
    sinfo->base_id = rec->id;

    sinfo->pcr = rec->pcr;
    sinfo->encrypt = rec->encrypt;
    sinfo->networkid = rec->networkid;
    sinfo->byte_rate = arg->byte_rate;
    sinfo->byte_length = arg->byte_length;

    sinfo->fill_num = winfo->w_fill_num;
    sinfo->byte_len = winfo->w_byte_len + winfo->w_strm_len;
    sinfo->time_len = winfo->w_sn * pvr->fragment_time + winfo->w_ifrm_num * IFRAME_CLK_INTERVAL / 100;

    sinfo->base_len = winfo->w_strm_len;

    sinfo->byte_base = sinfo->byte_len;
    sinfo->time_base = sinfo->time_len;

    list = rec->shift.pvr;
    if (NULL == list && rec->record)
        list = rec->record->pvr;

    if (arg->time_shift > 0) {
        sinfo->breaktype = -1;
        sinfo->time_length = arg->time_shift;

        if (list) {
            int base = list->s_info.time_base;

            if (base + sinfo->time_length < sinfo->time_len)
                sinfo->time_base = sinfo->time_len - sinfo->time_length;
             else
                sinfo->time_base = base;
        }
        g_stamp.flag = 0;
    } else {
        if (arg->breaktype < 0)
            sinfo->breaktype = 0;
        else
            sinfo->breaktype = arg->breaktype;
        sinfo->time_length = arg->time_length;
    }

    sprintf(pvr->path, "%s/%08x", g_cfg->path, rec->id);

    sinfo->record = 1;

    if (list)
        int_list_insert(list, pvr);

    if (arg->time_shift > 0) {
        if (rec->record) {
            PvrElem_t record = rec->record->pvr;
            if (record && record->s_info_sn == -1)
                ERR_OUT("id = %08x not realtime!\n", id);
        }
    } else {
        if (arg->realtime) {
            pvr->s_info_sn = rec->info_sn;
            rec->info_sn ++;
        }

        if (int_proginfo_write(pvr))
            ERR_OUT("id = %08x int_proginfo_write\n", id);
    }

    {
        PvrRecElem_t recElem;
        if (arg->time_shift <= 0) {
            recElem = (PvrRecElem_t)IND_CALLOC(sizeof(PvrRecElem), 1);
            if (!recElem)
                ERR_OUT("id = %08x malloc PvrRecElemt!\n", id);

            recElem->next = rec->record;
            rec->record = recElem;
        } else {
            if (rec->shift.pvr)
                ERR_OUT("id = %08x shift used!\n", id);
            recElem = &rec->shift;
            g_shift_id = id;
        }

        recElem->pvr = pvr;//放int_proginfo_write之前，若后者出错会导致死机
        if (pvr == rec->shift.pvr)
            winfo->w_sn_base = winfo->w_sn;
        recElem->clk = clk;
    }

    pvr->w_rec = rec;

    READ_MUTEX_LOCK( );
    int_pvr_hash_insert(pvr);
    READ_MUTEX_UNLOCK( );

    return pvr;
Err:
    if (pvr) {
        int_list_remove(pvr);
        int_pvr_free(pvr);
    }

    return NULL;
}

static void int_pvr_mix_close(PvrRecord_t rec, uint32_t id, int end, uint32_t cls_clk, uint32_t cls_date)
{
    PvrElem_t pvr = NULL;
    PvrSInfo_t sinfo;
    PvrRecElem_t prev, recElem;

    prev = NULL;
    if (rec->shift.pvr && id == rec->shift.pvr->s_info.id) {
        recElem = &rec->shift;
    } else {
        recElem = rec->record;
        while (recElem && recElem->pvr && id != recElem->pvr->s_info.id) {
            prev = recElem;
            recElem = recElem->next;
        }
    }
    if (!recElem) {
        if (rec->shift.pvr)
            ERR_PRN("id = %08x shift_id = %08x\n", id, rec->shift.pvr->s_info.id);
        recElem = rec->record;
        while (recElem && recElem->pvr) {
            ERR_PRN("id = %08x record_id = %08x\n", id, recElem->pvr->s_info.id);
            recElem = recElem->next;
        }
        goto Err;
    }
    pvr = recElem->pvr;
    sinfo = &pvr->s_info;

    PRINTF("id = %08x / %08x -------- end = %d\n", id, rec->id, end);

    if (recElem == &rec->shift || 0 >= pvr->info_len) {
        PvrElem_t list;
        PvrWInfo_t winfo = &rec->w_info;

        READ_MUTEX_LOCK( );
        if (pvr == g_cfg->pvr)
            int_pvr_close_read(pvr);
        READ_MUTEX_UNLOCK( );

        list = int_list_remove(pvr);

        if (NULL == list) {
            pvr->path[g_cfg->pathlen + 9] = 0;
            PRINTF("id = %08x delete %s\n", id, pvr->path);
            g_op->dir_delete(pvr->path);
        } else if (pvr == rec->shift.pvr) {
            int sn, sn_min;

            if (sinfo->time_base + sinfo->time_length < sinfo->time_len)
                sn_min = (sinfo->time_len - sinfo->time_length) / list->fragment_time;
            else
                sn_min = sinfo->time_base / list->fragment_time;

            for (sn = sn_min; sn <= winfo->w_sn; sn ++) {
                if (0 == int_media_used(list, sn))
                    int_media_delete(pvr->path, sn);
            }
            g_shift_id = 0;
        }

        int_pvr_free(pvr);
        recElem->pvr = NULL;
    } else {
        PvrWInfo_t winfo = &rec->w_info;

        if (cls_clk)
            sinfo->prev_clks += (cls_clk - recElem->clk);
        sinfo->prev_date = cls_date;

        sinfo->fill_num = winfo->w_fill_num;
        sinfo->byte_len = winfo->w_byte_len + winfo->w_strm_len;

        if (sinfo->byte_base == 0)
            PRINTF("id = %08x time_len = %d / %d, byte_len = %lld, byte_length = %lld\n", id, sinfo->time_len, sinfo->time_length, sinfo->byte_len - sinfo->fill_num * 188, sinfo->byte_length);
        else
            PRINTF("id = %08x time_len = %d / %d, byte_len = %lld, byte_length = %lld\n", id, sinfo->time_len, sinfo->time_length, sinfo->byte_len, sinfo->byte_length);

        if (end) {
            if (sinfo->byte_length == 0)
                sinfo->byte_length = sinfo->byte_len;
            sinfo->record = 2;
        } else {
            if (sinfo->breaktype > 0)
                int_brk_write(pvr, &rec->w_break);
            else
                int_brk_write(pvr, &rec->w_info);
            sinfo->record = 0;
        }

        if (int_proginfo_write_(pvr))
            ERR_PRN("id = %08x int_proginfo_write_\n", id);
        recElem->pvr = NULL;
    }

    pvr->w_rec = NULL;
    if (recElem != &rec->shift) {
        if (prev)
            prev->next = recElem->next;
        else
            rec->record = recElem->next;
        IND_FREE(recElem);
    }

Err:
    return;
}
//PvrRecord_t
int ind_pvr_rec_open(PvrArgument_t arg, uint32_t clk)
{
    uint32_t id;
    ts_psi_t psi;
    PvrElem_t pvr;
    PvrWInfo_t winfo;

    int index = -2;
    PvrRecord_t rec = NULL;

    if (g_cfg == NULL || arg == NULL || arg->psi == NULL || g_cfg->pvr_num >= g_cfg->pvr_size) {
        ERR_PRN("g_cfg = %p, arg = %p, pvr_num = %d, pvr_size = %d\n", g_cfg, arg, g_cfg->pvr_num, g_cfg->pvr_size);
        return -1;
    }

    id = arg->id;

    rec = int_rec_malloc( );
    if (rec == NULL) {
        ERR_PRN("id = %08x malloc PvrRecord\n", id);
        return -1;
    }

    WRITE_MUTEX_LOCK( );
    RECORD_MUTEX_LOCK( );
    if (g_cfg->mount_flg)
        ERR_OUT("mounting\n");
    index = -1;

    rec->id = id;
    rec->networkid = arg->networkid;

    sprintf(rec->path, "%s/%08x", g_cfg->path, id);
    if (g_op->dir_create(rec->path))
        ERR_OUT("id = %08x dir_create %s\n", id, rec->path);

    psi = arg->psi;

    winfo = &rec->w_info;

    sprintf(rec->path + g_cfg->pathlen + 9, "/media0");
    winfo->w_strm_fd = g_op->file_open(rec->path, FLAGS_WRITE);
    if (winfo->w_strm_fd == -1)
        ERR_OUT("id = %08x file_open %s\n", id, rec->path);
    sprintf(rec->path + g_cfg->pathlen + 9, "/media0.ifr");
    winfo->w_ifrm_fd = g_op->file_open(rec->path, FLAGS_WRITE);
    if (winfo->w_ifrm_fd == -1)
        ERR_OUT("id = %08x file_open %s\n", id, rec->path);

    pvr = int_pvr_mix_open(rec, arg, clk);
    if (pvr == NULL)
        ERR_OUT("id = %08x int_pvr_mix_open\n", id);

    ts_iparse_reset(&winfo->w_iparse, rec, psi, 1);

    index = rec->index;
Err:
    if (index == -1) {
        int_rec_close_write(rec);

        sprintf(rec->path, "%s/%08x", g_cfg->path, id);
        PRINTF("id = %08x delete %s\n", id, rec->path);
        g_op->dir_delete(rec->path);
    }

    RECORD_MUTEX_UNLOCK( );
    WRITE_MUTEX_UNLOCK( );

    if (index < 0) {
        int_rec_free(rec);
    } else {
        PRINTF("id = %08x file_sync\n", id);
    }

    return index;
}

static void int_pvr_rec_close(PvrRecord_t rec, int end, uint32_t cls_clk, uint32_t cls_date)
{
    uint32_t id;

    int_rec_close_write(rec);

    if (rec->shift.pvr)
        int_pvr_mix_close(rec, rec->shift.pvr->s_info.id, 0, cls_clk, cls_date);

    while (rec->record) {
        id = rec->record->pvr->s_info.id;
        PRINTF("id = %08x / %08x -------- end = %d\n", id, rec->id, end);
        int_pvr_mix_close(rec, id, end, cls_clk, cls_date);
    }

    return;
}

void ind_pvr_rec_close(int index, int end, uint32_t cls_clk, uint32_t cls_date)
{
    PvrRecord_t rec;

    if (g_cfg == NULL)
        return;

    rec = int_rec_get(index);
    if (rec == NULL) {
        ERR_PRN("int_rec_get(%d)!\n", index);
        return;
    }

    WRITE_MUTEX_LOCK( );
    RECORD_MUTEX_LOCK( );

    PRINTF("id = %08x -------- end = %d\n", rec->id, end);

    int_pvr_rec_close(rec, end, cls_clk, cls_date);

    RECORD_MUTEX_UNLOCK( );
    WRITE_MUTEX_UNLOCK( );

    int_rec_free(rec);//放在RECORD_MUTEX_UNLOCK之前会导致死机
}

int ind_pvr_mix_open(int index, PvrArgument_t arg, uint32_t clk)
{
    uint32_t id;
    PvrRecord_t rec;
    PvrElem_t pvr = NULL;

    if (g_cfg == NULL)
        return -1;

    rec = int_rec_get(index);
    if (rec == NULL) {
        ERR_PRN("int_rec_get(%d)\n", index);
        return -1;
    }

    PRINTF("id = %08x --------\n", arg->id);

    WRITE_MUTEX_LOCK( );
    RECORD_MUTEX_LOCK( );
    if (g_cfg->mount_flg)
        ERR_OUT("mounting\n");

    id = rec->id;

    pvr = int_pvr_hash_find(arg->id);
    if (pvr) {
        PvrSInfo_t sinfo = &pvr->s_info;
        if (sinfo->base_id != id) {
            ERR_PRN("-------- id = %08x base_id = %08x\n", id, sinfo->base_id);
            pvr = NULL;
        } else {
            PvrRecElem_t recElem;

            recElem = (PvrRecElem_t)IND_CALLOC(sizeof(PvrRecElem), 1);
            if (!recElem)
                ERR_OUT("id = %08x malloc PvrRecElemt!\n", id);

            sinfo->record = 1;

            recElem->clk = clk;
            recElem->pvr = pvr;

            recElem->next = rec->record;
            rec->record = recElem;

            pvr->w_rec = rec;

            int_proginfo_write_(pvr);
        }
    } else {
        pvr = int_pvr_mix_open(rec, arg, clk);
        if (pvr == NULL)
            ERR_PRN("-------- id = %08x int_pvr_mix_open = %08x\n", id, arg->id);
    }
Err:
    RECORD_MUTEX_UNLOCK( );
    WRITE_MUTEX_UNLOCK( );
    if (pvr == NULL)
        return -1;

    return 0;
}

void ind_pvr_mix_close(int index, uint32_t id, int end, uint32_t cls_clk, uint32_t cls_date)
{
    PvrRecord_t rec;

    if (g_cfg == NULL)
        return;

    rec = int_rec_get(index);
    if (rec == NULL) {
        ERR_PRN("int_rec_get(%d)\n", index);
        return;
    }

    PRINTF("id = %08x -------- end = %d\n", id, end);

    WRITE_MUTEX_LOCK( );
    RECORD_MUTEX_LOCK( );
    if (g_cfg->mount_flg)
        ERR_OUT("mounting\n");

    int_pvr_mix_close(rec, id, end, cls_clk, cls_date);
Err:
    RECORD_MUTEX_UNLOCK( );
    WRITE_MUTEX_UNLOCK( );

    return;
}

int ind_pvr_rec_rebreak(uint32_t id)
{
    int valid;
    PvrElem_t pvr;
    PvrSInfo_t sinfo = NULL;

    PRINTF("id = %08x\n", id);

    if (g_cfg == NULL)
        return -1;

    valid = 0;

    WRITE_MUTEX_LOCK( );

    pvr = int_pvr_hash_find(id);
    if (pvr == NULL)
        ERR_OUT("id = %08x int_pvr_hash_find\n", id);

    valid = 1;

    sinfo = &pvr->s_info;
    if (sinfo->version <= 8)
        int_ifrm_convert(pvr);

    if (sinfo->record == 2 || int_brk_check(pvr, NULL) == 0 || int_break_read(pvr, NULL) == 0)
        valid = 2;
Err:
    PRINTF("id = %08x valid = %d\n", id, valid);

    if (sinfo && valid == 1) {
        valid = 0;
        sinfo->time_len = 0;
        sinfo->byte_len = 0;
        sinfo->fill_num = 0;
        if (int_proginfo_write_(pvr))
            ERR_OUT("int_proginfo_write_\n");
        valid = 2;
    }
    WRITE_MUTEX_UNLOCK( );

    if (valid)
        return 0;
    return -1;
}

int ind_pvr_rec_reopen(uint32_t id, uint32_t clk, int* announce)
{
    int index = -2;

    PvrElem_t pvr;
    PvrWInfo_t winfo;
    PvrSInfo_t sinfo;

    PvrRecord_t rec;

    if (g_cfg == NULL)
        return -1;

    rec = int_rec_malloc( );
    if (rec == NULL) {
        ERR_PRN("id = %08x malloc PvrRecord\n", id);
        return -1;
    }

    WRITE_MUTEX_LOCK( );
    RECORD_MUTEX_LOCK( );
    if (g_cfg->mount_flg)
        ERR_OUT("mounting\n");
    index = -1;

    *announce = PVR_ANNOUNCE_ERROR;

    pvr = int_pvr_hash_find(id);
    if (pvr == NULL)
        ERR_OUT("int_pvr_hash_find\n");

    sinfo = &pvr->s_info;
    if (sinfo->record == 2) {
        *announce = PVR_ANNOUNCE_END;
        ERR_OUT("id = %08x record finished!\n", id);
    }

    if (sinfo->encrypt && sinfo->key != g_cfg->key)
        ERR_OUT("id = %08x unauthorized! key = %lld / %lld\n", id, sinfo->key, g_cfg->key);

    if (pvr->w_rec)
        ERR_OUT("id = %08x is recording!\n", id);

    if (sinfo->version <= 8)
        int_ifrm_convert(pvr);

    {
        uint32_t base_id = sinfo->base_id;
        if (0 == base_id)
            base_id = id;
        rec->id = base_id;
    }

    rec->pcr = sinfo->pcr;
    rec->encrypt = sinfo->encrypt;
    rec->networkid = sinfo->networkid;

    if (pvr->s_info_sn >= 0) {
        PvrElem_t list = pvr->list_first;

        rec->info_sn = pvr->s_info_sn;
        while (list) {
            if (list->s_info_sn > rec->info_sn)
                rec->info_sn = list->s_info_sn;
            list = list->list_next;
        }
        rec->info_sn ++;
    }

    winfo = &rec->w_info;

    if (sinfo->time_len == 0 && sinfo->byte_len == 0) {
        WARN_PRN("empty!\n");
    } else {
        if (int_brk_check(pvr, winfo)) {
            if (int_break_read(pvr, winfo)) {
                *announce = PVR_ANNOUNCE_DAMAGE;
                ERR_OUT("id = %08x int_break_read\n", id);
            }
        }
    }

    rec->w_break = rec->w_info;

    {
        PvrRecElem_t recElem;

        recElem = (PvrRecElem_t)IND_CALLOC(sizeof(PvrRecElem), 1);
        if (!recElem)
            ERR_OUT("id = %08x malloc PvrRecElemt!\n", id);
        rec->record = recElem;
    }
    rec->record->pvr = pvr;
    rec->record->clk = clk;

    sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d", winfo->w_sn);
    if (winfo->w_strm_len == 0)
        winfo->w_strm_fd = g_op->file_open(pvr->path, FLAGS_WRITE);
    else
        winfo->w_strm_fd = g_op->file_open(pvr->path, FLAGS_APPEND);
    if (winfo->w_strm_fd == -1)
        ERR_OUT("id = %08x file_open %s\n", id, pvr->path);
    sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d.ifr", winfo->w_sn);
    if (winfo->w_ifrm_num > 0) {
        int len;
        int fd = g_op->file_open(pvr->path, O_RDONLY);
        if (fd == -1)
            ERR_OUT("file_open %s rb\n", pvr->path);
        len = g_op->file_read(fd, (char *)winfo->w_ifrm_buf, IFRAME_SIZE * IFRAME_ARRAY_SIZE);
        g_op->file_close(fd);
        if (len != winfo->w_ifrm_num * IFRAME_SIZE)
            ERR_OUT("len = %d / %d, ifrm_num = %d\n", len, len / IFRAME_SIZE, winfo->w_ifrm_num);
    }
    if (winfo->w_ifrm_num == 0)
        winfo->w_ifrm_fd = g_op->file_open(pvr->path, FLAGS_WRITE);
    else
        winfo->w_ifrm_fd = g_op->file_open(pvr->path, FLAGS_APPEND);
    if (winfo->w_ifrm_fd == -1)
        ERR_OUT("id = %08x file_open %s\n", id, pvr->path);

    sinfo->record = 1;
    if (int_proginfo_write_(pvr))//将 "record = 1" 写入记录
        ERR_OUT("id = %08x int_proginfo_write_\n", id);

    pvr->w_rec = rec;

    index = rec->index;
Err:
    if (index == -1)
        int_rec_close_write(rec);

    RECORD_MUTEX_UNLOCK( );
    WRITE_MUTEX_UNLOCK( );

    if (index == -1)
        int_rec_free(rec);

    return index;
}

int ind_pvr_rec_psi(int index, struct ts_psi* psi)
{
    PvrRecord_t rec;

    if (g_cfg == NULL)
        return -1;

    rec = int_rec_get(index);
    if (rec == NULL) {
        ERR_PRN("int_rec_get(%d)\n", index);
        return -1;
    }

    RECORD_MUTEX_LOCK( );
    if (g_cfg->mount_flg)
        ERR_OUT("mounting\n");

    PRINTF("-------- id = %08x\n", rec->id);

    ts_iparse_reset(&rec->w_info.w_iparse, rec, psi, 0);
Err:
    RECORD_MUTEX_UNLOCK( );
    return 0;
}

int ind_pvr_rec_pcr(int index, int pcr)
{
    PvrElem_t pvr;
    PvrRecord_t rec;

    if (g_cfg == NULL)
        return -1;

    rec = int_rec_get(index);
    if (rec == NULL) {
        ERR_PRN("int_rec_get(%d)\n", index);
        return -1;
    }

    WRITE_MUTEX_LOCK( );
    RECORD_MUTEX_LOCK( );
    if (g_cfg->mount_flg)
        ERR_OUT("mounting\n");

    PRINTF("-------- id = %08x, pcr = %d\n", rec->id, pcr);

    rec->pcr = pcr;

    {
        PvrRecElem_t recElem = rec->record;

        while (recElem) {
            pvr = recElem->pvr;
            if (pvr) {
                pvr->s_info.pcr = pcr;
                int_proginfo_write(pvr);
            }

            recElem = recElem->next;
        }
    }

    pvr = rec->shift.pvr;
    if (pvr)
        pvr->s_info.pcr = pcr;
Err:
    RECORD_MUTEX_UNLOCK( );
    WRITE_MUTEX_UNLOCK( );
    return 0;
}

int ind_pvr_rec_arg(int index, PvrArgument_t arg)
{
    PvrElem_t pvr;
    PvrSInfo_t sinfo;
    PvrRecord_t rec;

    if (g_cfg == NULL)
        return -1;

    rec = int_rec_get(index);
    if (rec == NULL) {
        ERR_PRN("int_rec_get(%d)\n", index);
        return -1;
    }

    WRITE_MUTEX_LOCK( );
    RECORD_MUTEX_LOCK( );
    if (g_cfg->mount_flg)
        ERR_OUT("mounting\n");

    PRINTF("-------- id = %08x\n", rec->id);

    if (rec->shift.pvr || !rec->record)
        ERR_OUT("-------- id = %08x shift = %p\n", rec->id, rec->shift.pvr);

    pvr = rec->record->pvr;
    if (!pvr || rec->record->next)
        ERR_OUT("-------- id = %08x pvr = %p, next = %p\n", rec->id, pvr, rec->record->next);

    if (pvr->info_len > 0 || arg->info_len <= 0)
        ERR_OUT("-------- id = %08x info_len = %d / %d\n", rec->id, arg->info_len, pvr->info_len);

    if (arg->info_len > 0) {
        if (arg->info_len >= PVR_INFO_SIZE)
            ERR_OUT("id = %08x info_len = %d\n", rec->id, arg->info_len);
        pvr->info_buf = (char *)IND_MALLOC(arg->info_len);
        if (pvr->info_buf == NULL)
            ERR_OUT("id = %08x malloc info_buf\n", rec->id);
        IND_MEMCPY(pvr->info_buf, arg->info_buf, arg->info_len);
    }
    pvr->info_len = arg->info_len;

    sinfo = &pvr->s_info;

    //VOD书签下载使用
    sinfo->byte_bmark = arg->byte_bmark;
    sinfo->time_bmark = arg->time_bmark;

    sinfo->byte_rate = arg->byte_rate;
    sinfo->byte_length = arg->byte_length;
    if (arg->breaktype < 0)
        sinfo->breaktype = 0;
    else
        sinfo->breaktype = arg->breaktype;
    sinfo->time_length = arg->time_length;

    READ_MUTEX_LOCK( );
    {
        uint32_t time_len = sinfo->time_len;
        long long byte_len = sinfo->byte_len;

        sinfo->time_len = 0;
        sinfo->byte_len = 0;
        int_proginfo_write(pvr);
        sinfo->time_len = time_len;
        sinfo->byte_len = byte_len;
    }
    READ_MUTEX_UNLOCK( );

Err:
    RECORD_MUTEX_UNLOCK( );
    WRITE_MUTEX_UNLOCK( );
    return 0;
}

/*
    增加断点
 */
int ind_pvr_rec_break(int index)
{
    uint32_t id;
    int result = -1;
    PvrElem_t record;
    PvrRecord_t rec;
    PvrWInfo_t winfo;

    rec = int_rec_get(index);
    if (rec == NULL) {
        ERR_PRN("int_rec_get(%d)!\n", index);
        return -1;
    }

    RECORD_MUTEX_LOCK( );
    if (g_cfg->mount_flg)
        ERR_OUT("mounting\n");

    id = rec->id;

    if (rec->record)
        record = rec->record->pvr;
    else
        record = NULL;
    if (!record)
        ERR_OUT("id = %08x record is NULL\n", id);
    if (record->s_info.breaktype <= 0)
        ERR_OUT("id = %08x breaktype = %d\n", id, record->s_info.breaktype);

    winfo = &rec->w_info;

    PRINTF("id = %08x w_sn = %d, w_ifrm_num = %d, time_len = %d\n",
            id, winfo->w_sn, winfo->w_ifrm_num, winfo->w_sn * record->fragment_time + winfo->w_ifrm_num * IFRAME_CLK_INTERVAL / 100);

    if (int_break_write(rec))
        ERR_OUT("id = %08x int_break_write\n", id);

    winfo->w_iparse.icntnt.frm_state = PARSE_STATE_NONE;
    record->slice_num ++;

    rec->w_break = rec->w_info;

    result = 0;
Err:
    RECORD_MUTEX_UNLOCK( );

    return result;
}

/*
    记录当前录制位置，用于直播转时移时从暂停点继续给解码器送数据
 */
int ind_pvr_rec_stamp(int index)
{
    uint32_t id;
    PvrRecord_t rec;
    PvrWInfo_t winfo;

    int result = -1;

    rec = int_rec_get(index);
    if (rec == NULL) {
        ERR_PRN("int_rec_get(%d)!\n", index);
        return -1;
    }

    RECORD_MUTEX_LOCK( );

    id = rec->id;
    winfo = &rec->w_info;

    if (rec->shift.pvr == NULL)
        ERR_OUT("id = %08x shift is NULL\n", id);

    g_stamp.strm_sn = winfo->w_sn;
    g_stamp.strm_off = winfo->w_strm_len;
    g_stamp.ifrm_off = winfo->w_ifrm_num;

    PRINTF("id = %08x sn = %d, strm_off = %d, ifrm_off = %d\n", id, g_stamp.strm_sn, g_stamp.strm_off, g_stamp.ifrm_off);

    g_stamp.flag = 1;

    result = 0;
Err:
    RECORD_MUTEX_UNLOCK( );
    return -1;
}

int ind_pvr_fragment(void)
{
    return FRAGMENT_TIME_CURRENT;
}

static int int_ifrm_convert(PvrElem_t pvr)
{
    uint32_t id;
    int fd;
    int i, sn, max, off, len, num, result = -1;
    ts_ifrm_t array = NULL;
    struct ts_iframe iframe;
    PvrSInfo_t sinfo = &pvr->s_info;

    id = sinfo->id;

    if (sinfo->version >= 9)
        ERR_OUT("id = %08x version = %d\n", id, sinfo->version);

    memset(&iframe, 0, IFRAME_SIZE);
    array = (ts_ifrm_t)IND_MALLOC(IFRAME_SIZE * FRAGMENT_TIME_DEFAULT);
    if (array == NULL)
        ERR_OUT("id = %08x malloc iframe array!\n", id);

    PRINTF("id = %08x convert iframe\n", id);

    max = sinfo->time_len / FRAGMENT_TIME_DEFAULT;
    for (sn = 0; sn <= max; sn ++) {
        sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d.idx", sn);
        fd = g_op->file_open(pvr->path, O_RDONLY);
        if (fd == -1)
            ERR_OUT("id = %08x file_open %s\n", id, pvr->path);
        len = g_op->file_read(fd, (char*)array, IFRAME_SIZE * FRAGMENT_TIME_DEFAULT);
        g_op->file_close(fd);

        num = len / IFRAME_SIZE;

        sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d.ifr", sn);
        fd = g_op->file_open(pvr->path, FLAGS_WRITE);
        if (fd == -1)
            ERR_OUT("id = %08x file_open %s\n", id, pvr->path);

        off = 0;
        for (i = 0; i < num; i ++) {
            g_op->file_write(fd, (char*)&array[i], IFRAME_SIZE);

            if (array[i].ifrm_off > off)
                off = array[i].ifrm_off;
            iframe.ifrm_off = off;
            g_op->file_write(fd, (char*)&iframe, IFRAME_SIZE);
            if (i % 2)
                g_op->file_write(fd, (char*)&iframe, IFRAME_SIZE);
        }

        g_op->file_close(fd);
    }

    PRINTF("id = %08x convert info\n", id);

    sinfo->version = 9;
    int_proginfo_write_(pvr);

    result = 0;
Err:
    if (array)
        IND_FREE(array);
    return result;
}

static int int_ifrm_write(PvrRecord_t rec, char* buf, int len)
{
    int l;
    PvrWInfo_t winfo = &rec->w_info;
#ifdef DEBUG_BUILD
    uint32_t id = rec->id;
#endif

    if (len == 0)
        return 0;

    if (len < 0)
        ERR_OUT("id = %08x len = %d\n", id, len);

    if (rec->encrypt)
        l = g_op->strm_write(winfo->w_strm_fd, buf, len);
    else
        l = g_op->file_write(winfo->w_strm_fd, buf, len);

    if (l < len) {
        if (l < 0)
            ERR_OUT("id = %08x write len = %d\n", id, len);
        WARN_PRN("id = %08x write len = %d, l = %d\n", id, len, l);
    }

    winfo->w_strm_len += l;

    return 0;
Err:
    return -1;
}

static void int_ifrm_sync(PvrElem_t pvr, PvrWInfo_t winfo)
{
    PvrRInfo_t rinfo;
    PvrSInfo_t sinfo = &pvr->s_info;

    sinfo->fill_num = winfo->w_fill_num;
    sinfo->byte_len = winfo->w_byte_len + winfo->w_strm_len;
    sinfo->time_len = winfo->w_sn * pvr->fragment_time + winfo->w_ifrm_num * IFRAME_CLK_INTERVAL / 100;

    if (pvr == g_cfg->pvr) {
        rinfo = pvr->r_info;
        if (rinfo->r_strm_sn == winfo->w_sn) {
            while (rinfo->r_ifrm_num < winfo->w_ifrm_num) {
                    rinfo->r_ifrm_buf[rinfo->r_ifrm_num] = winfo->w_ifrm_buf[rinfo->r_ifrm_num];
                rinfo->r_ifrm_num ++;
            }
            rinfo->r_strm_len = winfo->w_strm_len;
        }
    }
}

static int int_ifrm_begin(PvrRecord_t rec, ts_ifrm_t ifrm, uint32_t clk)
{
    uint32_t id;
    int num;
    PvrElem_t pvr;
    PvrWInfo_t winfo = &rec->w_info;

    id = rec->id;
    pvr = rec->shift.pvr;
    if (pvr == NULL)
        pvr = rec->record->pvr;

    num = clk / IFRAME_CLK_INTERVAL - winfo->w_sn * pvr->fragment_size;

    if (num < winfo->w_ifrm_num || (ifrm->ifrm_size != 1 && num >= pvr->fragment_size))
        winfo->w_ifrm_valid = 0;
    else
        winfo->w_ifrm_valid = 1;
    //PRINTF("id = %08x w_ifrm_valid = %d, num = %d / %d, sec = %d / %d\n", id, winfo->w_ifrm_valid, num, winfo->w_ifrm_num, num * IFRAME_CLK_INTERVAL / 100, winfo->w_ifrm_num * IFRAME_CLK_INTERVAL / 100);

    while (num >= winfo->w_ifrm_num) {

        if (winfo->w_ifrm_num >= pvr->fragment_size) {
            int n = winfo->w_ifrm_num - pvr->fragment_size;

            winfo->w_ifrm_num = pvr->fragment_size;
            READ_MUTEX_LOCK( );
            {
                PvrRecElem_t recElem = rec->record;
                while (recElem) {
                    int_ifrm_sync(recElem->pvr, winfo);
                    recElem = recElem->next;
                }
            }
            if (rec->shift.pvr)
                int_ifrm_sync(rec->shift.pvr, winfo);
            READ_MUTEX_UNLOCK( );

            winfo->w_sn ++;

            winfo->w_ifrm_num = n;
            winfo->w_break_off = 0;

            num -= pvr->fragment_size;

            if (winfo->w_strm_fd != -1) {
                PRINTF("id = %08x close w_strm_fd\n", id);
                g_op->file_close(winfo->w_strm_fd);
                winfo->w_strm_fd = -1;
            }
            if (winfo->w_ifrm_fd != -1) {
                PRINTF("id = %08x close w_ifrm_fd\n", id);
                g_op->file_close(winfo->w_ifrm_fd);
                winfo->w_ifrm_fd = -1;
            }

            if (rec->shift.pvr) {//本地时移删除前面无用分片
                PvrSInfo_t sinfo;

                sinfo = &pvr->s_info;
                while ((winfo->w_sn - winfo->w_sn_base - 1) * pvr->fragment_time >= sinfo->time_length) {
                    READ_MUTEX_LOCK( );
                    {
                        PvrRInfo* rinfo = pvr->r_info;
                        if (rinfo && rinfo->r_strm_sn <= winfo->w_sn_base) {
                            rinfo->r_ifrm_num = 0;
                            int_pvr_seek(pvr, (winfo->w_sn_base + 1) * pvr->fragment_time, rinfo->r_scale);
                            rinfo->r_clk_base = 0;
                        }
                    }
                    READ_MUTEX_UNLOCK( );

                    if (0 == int_media_used(pvr->list_first, winfo->w_sn_base))
                        int_media_delete(pvr->path, winfo->w_sn_base);

                    winfo->w_sn_base ++;
                }
            }

            sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d", winfo->w_sn);
            winfo->w_strm_fd = g_op->file_open(pvr->path, FLAGS_WRITE);
            if (winfo->w_strm_fd == -1)
                ERR_OUT("id = %08x file_open %s\n", id, pvr->path);
            sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d.ifr", winfo->w_sn);
            winfo->w_ifrm_fd = g_op->file_open(pvr->path, FLAGS_WRITE);
            if (winfo->w_ifrm_fd == -1)
                ERR_OUT("id = %08x file_open %s\n", id, pvr->path);

            winfo->w_fill_num = 0;
            winfo->w_break_ifrm = 0;

            winfo->w_byte_len += winfo->w_strm_len;
            winfo->w_strm_len = 0;
            winfo->w_break_len = 0;

            ifrm->ifrm_off = 0;
        }

        if (num <= winfo->w_ifrm_num)
            break;

        {
            struct ts_iframe iframe;
            memset(&iframe, 0, IFRAME_SIZE);
            iframe.ifrm_off = ifrm->ifrm_off;

            winfo->w_ifrm_buf[winfo->w_ifrm_num] = iframe;
            winfo->w_ifrm_num ++;
            //PRINTF("-------------------------------------------------------\n");
            if (g_op->file_write(winfo->w_ifrm_fd, (char *)&iframe, IFRAME_SIZE) != IFRAME_SIZE)
                ERR_OUT("id = %08x file_write iframe\n", id);
        }
    }

    return 0;
Err:
    return -1;
}

static int int_ifrm_end(PvrRecord_t rec, ts_ifrm_t ifrm)
{
    int l;
    uint32_t id;
    PvrWInfo_t winfo = &rec->w_info;

    id = rec->id;

    if (winfo->w_ifrm_valid == 0)
        return 0;

    //PRINTF("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    memcpy(&winfo->w_ifrm_buf[winfo->w_ifrm_num], ifrm, IFRAME_SIZE);
    winfo->w_ifrm_num ++;


    l = g_op->file_write(winfo->w_ifrm_fd, (char*)ifrm, IFRAME_SIZE);
    if (l != IFRAME_SIZE)
        ERR_OUT("id = %08x file_write iframe len = %d, ret = %d\n", id, IFRAME_SIZE, l);

    READ_MUTEX_LOCK( );
    {
        PvrRecElem_t recElem = rec->record;
        while (recElem) {
            int_ifrm_sync(recElem->pvr, winfo);
            recElem = recElem->next;
        }
    }
    if (rec->shift.pvr)
        int_ifrm_sync(rec->shift.pvr, winfo);
    READ_MUTEX_UNLOCK( );

    return 0;
Err:
    return -1;
}

static int int_pvr_rec_write(PvrRecord_t rec, char* buf, int len)
{
    PvrWInfo_t winfo = &rec->w_info;
#ifdef DEBUG_BUILD
    uint32_t id = rec->id;
#endif

    if (rec->pcr) {
        if (ts_iparse_pcr(&winfo->w_iparse, buf, len))
            ERR_OUT("id = %08x ts_iparse_pcr\n", id);
    } else {
        if (ts_iparse_frame(&winfo->w_iparse, buf, len))
            ERR_OUT("id = %08x ts_iparse_frame\n", id);
    }

    return 0;
Err:
    return -1;
}

int ind_pvr_rec_write(int index, char* buf, int len)
{
    uint32_t id;
    int i, l, result = -1;
    PvrElem_t record;
    PvrWInfo_t winfo;
    PvrRecord_t rec;

    if (g_cfg == NULL)
        return -1;

    rec = int_rec_get(index);
    if (rec == NULL) {
        ERR_PRN("int_rec_get(%d)!\n", index);
        return -1;
    }

    RECORD_MUTEX_LOCK( );
    if (g_cfg->mount_flg)
        ERR_OUT("mounting\n");

    id = rec->id;

    winfo = &rec->w_info;
    if (winfo->w_strm_fd == -1)
        ERR_OUT("id = %08x closed!\n", id);

    if (len <= 0 || len % 188)
        ERR_OUT("id = %08x len = %d, len%%188 = %d\n", id, len, len % 188);
    for (i = 0; i < len; i += 188) {
        if (buf[i] != 0x47)
            ERR_OUT("id = %08x buf[%d] = %02x sync\n", id, i, (u_char)buf[i]);
    }

    if (rec->record)
        record = rec->record->pvr;
    else
        record = NULL;

    while (len > 0) {
        if (winfo->w_break_len <= winfo->w_strm_len)
            winfo->w_break_len += BYTES_PER_BREAK;

        l = winfo->w_break_len - winfo->w_strm_len;
        if (l > len)
            l = len;

        if (l > 0) {
            if (int_pvr_rec_write(rec, buf, l))
                ERR_OUT("id = %08x int_pvr_rec_write0\n", id);
            buf += l;
            len -= l;
        }

        if (winfo->w_break_len == winfo->w_strm_len) {
            if (record && record->s_info.breaktype > 0) {
                if (record->slice_num < winfo->w_break_num || (winfo->w_strm_len % BYTES_PER_POINT) == 0) {
                    if (int_break_point(rec))
                        ERR_OUT("id = %08x int_break_point\n", id);
                    record->slice_num = winfo->w_break_num;
                }
            } else  {
                PvrElem_t pvr = record;
                if (NULL == pvr)
                    pvr = rec->shift.pvr;

                if (winfo->w_break_ifrm + IFRAME_PER_BREAK <= winfo->w_ifrm_num) {
                    PRINTF("id = %08x, w_sn = %d, w_break_ifrm = %d, w_ifrm_num = %d, time_len = %d\n",
                            id, winfo->w_sn, winfo->w_break_ifrm, winfo->w_ifrm_num, winfo->w_sn * pvr->fragment_time + winfo->w_ifrm_num * IFRAME_CLK_INTERVAL / 100);
                    if (int_break_write(rec))
                        ERR_OUT("id = %08x int_break_write\n", id);
                } else if ((winfo->w_strm_len % BYTES_PER_POINT) == 0) {
                    if (int_break_point(rec))
                        ERR_OUT("id = %08x int_break_point1\n", id);
                }
            }
        }
    }

    result = 0;
Err:
    RECORD_MUTEX_UNLOCK( );

    return result;
}

void ind_pvr_delete(uint32_t id, PvrMsgCall call)
{
    PvrElem_t pvr, list = NULL;
    PvrSInfo_t sinfo;
    PvrRecord_t rec;
    char path[PVR_PATH_LEN];

    int sn_num = -1;
    int *sn_array = NULL;

    if (g_cfg == NULL)
        return;

    PRINTF("id = %08x --------\n", id);

    WRITE_MUTEX_LOCK( );

    pvr = int_pvr_hash_find(id);
    if (pvr == NULL)
        ERR_OUT("id = %08x int_pvr_hash_find\n", id);

    rec = pvr->w_rec;
    if (rec) {
        if (pvr == rec->shift.pvr)
            ERR_OUT("id = %08x delete shift\n", id);

        if (rec->shift.pvr || (rec->record && rec->record->next))
            int_pvr_mix_close(rec, id, 0, 0, 0);
        else
            int_pvr_rec_close(rec, 0, 0, 0);
    }

    READ_MUTEX_LOCK( );
    if (pvr == g_cfg->pvr)
        int_pvr_close_read(pvr);
    READ_MUTEX_UNLOCK( );

    sinfo = &pvr->s_info;
    list = int_list_remove(pvr);

    if (pvr->s_info_sn == -1)
        sprintf(pvr->path + g_cfg->pathlen + 9, "/info");
    else
        sprintf(pvr->path + g_cfg->pathlen + 9, "/info%d", pvr->s_info_sn);
    g_op->file_delete(pvr->path);
    if (pvr->s_info_sn == -1)
        sprintf(pvr->path + g_cfg->pathlen + 9, "/info_");
    else
        sprintf(pvr->path + g_cfg->pathlen + 9, "/info%d_", pvr->s_info_sn);
    g_op->file_delete(pvr->path);

    if (list && pvr->s_info_sn != -1) {
        if (list->list_next == NULL && list->s_info.breaktype == -1) {
            sprintf(pvr->path + g_cfg->pathlen + 9, "/info");
            g_op->file_delete(pvr->path);
        } else {
            int_list_write(list);
        }
    }

    IND_STRCPY(path, pvr->path);
    int_pvr_free(pvr);

    sn_num = 0;
    {
        int sn, sn_min, sn_max;

        sn_min = sinfo->time_base / pvr->fragment_time;
        sn_max = sinfo->time_len / pvr->fragment_time;

        sn_array = IND_MALLOC(sizeof(int) * (sn_max + 1 - sn_min));
        if (sn_array) {
            for (sn = sn_min; sn <= sn_max; sn ++) {
                if (NULL == list || 0 == int_media_used(list, sn)) {
                    sn_array[sn_num] = sn;
                    sn_num ++;
                }
            }
        }
    }

Err:
    WRITE_MUTEX_UNLOCK( );

    if (sn_num > 0) {
        int i, prog, step;//进度

        step = (sn_num + 9) / 10;
        prog = step;

        for (i = 0; i < sn_num; i ++) {
            int_media_delete(path, sn_array[i]);

            if (i >= prog && i < sn_num - 1) {
                prog += step;
                if (call)
                    call(id, 100 * (i + 1) / sn_num);
            }
        }
    }
    if (sn_array)
        IND_FREE(sn_array);

    if (sn_num >= 0 && NULL == list) {
        path[g_cfg->pathlen + 9] = 0;
        PRINTF("id = %08x delete %s\n", id, path);
        g_op->dir_delete(path);
    }
    if (call)
        call(id, 100);
}

#define CHECK_READ_HANDLE( )                        \
do {                                                \
    if (pvr == NULL)                                \
        ERR_OUT("pvr is NULL\n");                   \
    if (pvr != g_cfg->pvr)                          \
        ERR_OUT("int_pvr_array_check = %p\n", pvr); \
} while(0)

int ind_pvr_get_num(void)
{
    int i, pvr_num, id_num;
    uint32_t *id_array;
    PvrElem_t pvr, *pvr_table;

    if (g_cfg == NULL)
        return 0;

    READ_MUTEX_LOCK( );

    id_num = 0;
    id_array = g_cfg->id_array;

    pvr_num = g_cfg->pvr_num;
    pvr_table = g_cfg->pvr_table;

    for (i = 0; i < pvr_num; i ++) {
        pvr = pvr_table[i];
        if (pvr->info_len <= 0 || pvr->s_info.key != g_cfg->key)
            continue;
        id_array[id_num] = pvr->s_info.id;
        id_num ++;
    }

    g_cfg->id_num = id_num;

    READ_MUTEX_UNLOCK( );

    return id_num;
}

static void int_pvr_get_info(PvrElem_t pvr, struct PVRInfo* info)
{
    uint32_t id;
    PvrRecord_t rec;
    PvrSInfo_t sinfo = &pvr->s_info;

    id = sinfo->id;

    info->id = id;

    if (sinfo->encrypt && sinfo->key != g_cfg->key) {
        WARN_PRN("id = %08x encrypt = %d, key = %lld / %lld\n", id, sinfo->encrypt, sinfo->key, g_cfg->key);
        info->unauthorized = 1;
    } else {
        info->unauthorized = 0;
    }
    if (sinfo->time_len <= sinfo->time_base)
        info->time_len = 0;
    else
        info->time_len = sinfo->time_len - sinfo->time_base;

    rec = pvr->w_rec;
    if (rec) {
        if (pvr == rec->shift.pvr) {
            info->open_clk = rec->shift.clk;
        } else {
            PvrRecElem_t recElem = rec->record;

            while (recElem) {
                if (pvr == recElem->pvr) {
                    info->open_clk = recElem->clk;
                    break;
                }
                recElem = recElem->next;
            }
        }

        info->recording = 1;
    } else {
        info->open_clk = 0;

        if (sinfo->record == 2 || (sinfo->byte_length > 0 && sinfo->byte_len >= sinfo->byte_length) || (sinfo->time_length > 0 && sinfo->time_len >= sinfo->time_length))
            info->recording = 2;
        else
            info->recording = 0;
    }
    if (sinfo->time_len <= sinfo->time_base) {
        info->byte_len = 0;
    } else {
        info->byte_len = sinfo->byte_len - sinfo->byte_base;
        if (sinfo->byte_base == 0)
            info->byte_len -= sinfo->fill_num * 188;
    }
    info->prev_clks = sinfo->prev_clks;
    info->prev_date = sinfo->prev_date;

    info->byte_bmark = sinfo->byte_bmark;
    info->time_bmark = sinfo->time_bmark;

    info->time_length = sinfo->time_length;
    info->byte_length = sinfo->byte_length;

    info->networkid = sinfo->networkid;
}

int ind_pvr_breaktype(uint32_t id)
{
    int breaktype = -1;
    PvrElem_t pvr;

    if (g_cfg == NULL)
        return -1;

    READ_MUTEX_LOCK( );

    pvr = int_pvr_hash_find(id);

    if (pvr == NULL)
        ERR_OUT("id = %08x int_pvr_hash_find\n", id);

    breaktype = pvr->s_info.breaktype;

Err:
    READ_MUTEX_UNLOCK( );
    return breaktype;
}

int ind_pvr_breaknum(uint32_t id)
{
    int breaknum = 0;
    PvrElem_t pvr;

    if (g_cfg == NULL)
        return 0;

    READ_MUTEX_LOCK( );

    pvr = int_pvr_hash_find(id);
    if (pvr == NULL)
        ERR_OUT("id = %08x int_pvr_hash_find\n", id);

    breaknum = pvr->slice_num;

Err:
    READ_MUTEX_UNLOCK( );
    return breaknum;
}

void ind_pvr_set_fill(int fill)
{
    PRINTF("fill = %d\n", fill);
    g_fill = fill;
}

uint32_t ind_pvr_get_base(uint32_t id)
{
    uint32_t base_id = 0;
    PvrElem_t pvr;

    if (g_cfg == NULL)
        return 0;

    READ_MUTEX_LOCK( );

    pvr = int_pvr_hash_find(id);
    if (pvr == NULL)
        ERR_OUT("id = %08x int_pvr_hash_find\n", id);

    base_id = pvr->s_info.base_id;

Err:
    READ_MUTEX_UNLOCK( );
    return base_id;
}

int ind_pvr_get_info(uint32_t id, struct PVRInfo* info)
{
    int result = -1;
    PvrElem_t pvr;

    if (g_cfg == NULL)
        return -1;

    READ_MUTEX_LOCK( );

    if (info == NULL)
        ERR_OUT("info is NULL\n");

    if (0 == id)
        id = g_shift_id;
    pvr = int_pvr_hash_find(id);
    if (pvr == NULL)
        ERR_OUT("id = %08x int_pvr_hash_find\n", id);

    int_pvr_get_info(pvr, info);

    result = 0;
Err:
    READ_MUTEX_UNLOCK( );
    return result;
}

int ind_pvr_get_time(uint32_t id)
{
    int sec = 0;
    PvrElem_t pvr;
    PvrSInfo_t sinfo;

    if (g_cfg == NULL)
        return 0;

    READ_MUTEX_LOCK( );

    pvr = int_pvr_hash_find(id);
    if (pvr == NULL)
        ERR_OUT("id = %08x int_pvr_hash_find\n", id);

    sinfo = &pvr->s_info;
    if (sinfo->time_len <= sinfo->time_base)
        sec = 0;
    else
        sec = sinfo->time_len - sinfo->time_base;

Err:
    READ_MUTEX_UNLOCK( );
    return sec;
}

uint32_t ind_pvr_get_info_id(int index)
{
    uint32_t id = 0;

    if (g_cfg == NULL)
        return 0;

    READ_MUTEX_LOCK( );

    if (g_cfg == NULL)
        ERR_OUT("g_cfg = %p\n", g_cfg);
    if (index < 0 || index >= g_cfg->id_num)
        ERR_OUT("index = %d, id_num = %d\n", index, g_cfg->id_num);

    id = g_cfg->id_array[index];

Err:
    READ_MUTEX_UNLOCK( );
    return id;
}

int ind_pvr_get_info_ex(int index, struct PVRInfo* info, char* info_buf, int* pinfo_len)
{
    int result = -1;
    PvrElem_t pvr;
    uint32_t id;

    if (g_cfg == NULL)
        return -1;

    READ_MUTEX_LOCK( );

    if (g_cfg == NULL)
        ERR_OUT("g_cfg = %p\n", g_cfg);
    if (index < 0 || index >= g_cfg->id_num)
        ERR_OUT("index = %d, id_num = %d\n", index, g_cfg->id_num);

    id = g_cfg->id_array[index];
    pvr = int_pvr_hash_find(id);
    if (pvr == NULL)
        ERR_OUT("id = %08x int_pvr_hash_find\n", id);

    if (info_buf && pinfo_len) {
        if (*pinfo_len < pvr->info_len || pvr->info_len <= 0) {
            ERR_PRN("id = %08x len = %d / %d\n", id, *pinfo_len, pvr->info_len);
            *pinfo_len = 0;
        } else {
            IND_MEMCPY(info_buf, pvr->info_buf, pvr->info_len);
            *pinfo_len = pvr->info_len;
        }
    }

    if (info)
        int_pvr_get_info(pvr, info);

    result = 0;
Err:
    READ_MUTEX_UNLOCK( );
    return result;
}

uint32_t ind_pvr_get_byterate(uint32_t id)
{
    uint32_t result = 0;
    PvrElem_t pvr;

    if (g_cfg == NULL)
        return -1;

    READ_MUTEX_LOCK( );

    pvr = int_pvr_hash_find(id);
    if (pvr == NULL)
        ERR_OUT("id = %08x int_pvr_hash_find\n", id);

    result = pvr->s_info.byte_rate;
Err:
    READ_MUTEX_UNLOCK( );
    return result;
}

int ind_pvr_exist(uint32_t id)
{
    int result = 0;
    PvrElem_t pvr;

    if (g_cfg == NULL)
        return -1;

    READ_MUTEX_LOCK( );

    pvr = int_pvr_hash_find(id);
    if (pvr == NULL)
        goto Err;

    if (2 == pvr->s_info.record)
        result = 2;
    else
        result = 1;
Err:
    READ_MUTEX_UNLOCK( );
    return result;
}

static void int_pvr_iframe(PvrElem_t pvr, int sn)
{
    int fd, len, num, size;
    uint32_t id;
    PvrRInfo_t rinfo = pvr->r_info;
    PvrSInfo_t sinfo = &pvr->s_info;

    id = pvr->s_info.id;

    rinfo->r_ifrm_num = 0;

    sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d.ifr", sn);
    size = 0;
    g_op->file_size(pvr->path, &size);
    fd = g_op->file_open(pvr->path, O_RDONLY);

    if (fd < 0 && sinfo->record != 1) {
        num = int_iframe_restore(pvr, sn);
        if (num > 0) {
            sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d.ifr", sn);
            fd = g_op->file_open(pvr->path, FLAGS_WRITE);
            if (fd >= 0) {
                g_op->file_write(fd, (char*)rinfo->r_ifrm_buf, IFRAME_SIZE * num);
                g_op->file_close(fd);
                fd = g_op->file_open(pvr->path, O_RDONLY);
            }
        }
    }

    if (fd >= 0) {
        if (size > IFRAME_SIZE * pvr->fragment_size)
            g_op->file_seek(fd, size - IFRAME_SIZE * pvr->fragment_size);
        len = g_op->file_read(fd, (char *)rinfo->r_ifrm_buf, IFRAME_SIZE * pvr->fragment_size);
        g_op->file_close(fd);
        if (len > 0)
            rinfo->r_ifrm_num = len / IFRAME_SIZE;
    }

    sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d", sn);
    g_op->file_size(pvr->path, &rinfo->r_strm_len);

    {
        int maxsn, time_len;

        maxsn = sinfo->time_len / pvr->fragment_time;
        if (sinfo->time_len == maxsn * pvr->fragment_time && maxsn > 0) {
            time_len = pvr->fragment_time;
            maxsn --;
        } else {
            time_len = sinfo->time_len % pvr->fragment_time;
        }

        if (sn < maxsn) {
            if (rinfo->r_ifrm_num < pvr->fragment_size)
                WARN_PRN("id = %08x sn = %d / %d, ifrm_num = %d\n", id, sn, maxsn, rinfo->r_ifrm_num);
        } else if (sinfo->record != 1) {
            num = time_len % pvr->fragment_time * 100 / IFRAME_CLK_INTERVAL;
            if (rinfo->r_ifrm_num < num)
                WARN_PRN("id = %08x num = %d, ifrm_num = %d\n", id, num, rinfo->r_ifrm_num);
        }
    }

    if (sinfo->version < 10) {
        ts_ifrm_t ifrm;
        int i, num = rinfo->r_ifrm_num;
        ts_ifrm_t ifrm_buf = rinfo->r_ifrm_buf;

        for (i = 0; i < num; i ++) {
            ifrm = &ifrm_buf[i];
            //PRINTF("ifrm[%d] off = %u, size = %u, clk = %u\n", i, ifrm->ifrm_off, (uint32_t)ifrm->ifrm_size, (uint32_t)ifrm->ifrm_clk);
            if (ifrm->ifrm_size > 0)
                ifrm->ifrm_clk = sn * pvr->fragment_time * 100 + i * IFRAME_CLK_INTERVAL;
        }
    }

    PRINTF("id = %08x ifrm_num = %d, strm_len = %d\n", id, rinfo->r_ifrm_num, rinfo->r_strm_len);
}

static int int_pvr_fragment(PvrElem_t pvr, int sn)
{
    PvrRInfo_t rinfo = pvr->r_info;
#ifdef DEBUG_BUILD
    uint32_t id = pvr->s_info.id;
#endif

    if (rinfo->r_strm_sn == sn && rinfo->r_strm_fd != -1) {
        int sn, num;
        PvrSInfo_t sinfo = &pvr->s_info;

        sn = sinfo->time_len / pvr->fragment_time;
        num = sinfo->time_len % pvr->fragment_time * 100 / IFRAME_CLK_INTERVAL;

        if ((rinfo->r_strm_sn >= sn && rinfo->r_ifrm_num < num) || (rinfo->r_strm_sn < sn && rinfo->r_ifrm_num < pvr->fragment_size))
            int_pvr_iframe(pvr, rinfo->r_strm_sn);
    } else {
        if (rinfo->r_strm_fd != -1) {
            g_op->file_close(rinfo->r_strm_fd);
            rinfo->r_strm_fd = -1;
        }
    
        sprintf(pvr->path + g_cfg->pathlen + 9, "/media%d", sn);
        rinfo->r_strm_fd = g_op->file_open(pvr->path, O_RDONLY);
        if (rinfo->r_strm_fd == -1)
            ERR_OUT("id = %08x file_open %s\n", id, pvr->path);
    
        rinfo->r_strm_off = 0;
        rinfo->r_strm_sn = sn;
    
        rinfo->r_ifrm_num = 0;
        int_pvr_iframe(pvr, sn);
    }

    return 0;
Err:
    return -1;
}

static int int_pvr_seek(PvrElem_t pvr, int seek, int scale)
{
    uint32_t id;
    int result = -1;
    int total, off, sn;

    PvrSInfo_t sinfo = &pvr->s_info;
    PvrRInfo_t rinfo = pvr->r_info;

    id = pvr->s_info.id;
    PRINTF("id = %08x seek = %d, scale = %d\n", id, seek, scale);

    total = sinfo->time_len;
    if (total <= 0) {
        if (seek != 0 || scale != 1)
            ERR_OUT("id = %08x total = %d, scale = %d\n", id, total, scale);
        total = 0;
    }

    if (seek > total)
        seek = total;
    else if (seek < 0)
        seek = 0;

    if (pvr->w_rec && pvr == pvr->w_rec->shift.pvr) {
        int begin = total - sinfo->time_length + 1;
        if (begin < 0)
            begin = 0;
        if (seek < begin)
            seek = begin;
    }

    sn = seek / pvr->fragment_time;
    seek = seek % pvr->fragment_time;

    if (int_pvr_fragment(pvr, sn))
        ERR_OUT("id = %08x int_pvr_fragment sn = %d\n", id, sn);

    rinfo->r_strm_sn = sn;

    rinfo->r_ifrm_off = seek * 100 / IFRAME_CLK_INTERVAL;
    PRINTF("r_ifrm_num = %d, r_ifrm_off = %d\n", rinfo->r_ifrm_num, rinfo->r_ifrm_off);

    if (rinfo->r_ifrm_num > 0) {
        ts_ifrm_t ifrm;

        if (rinfo->r_ifrm_off >= rinfo->r_ifrm_num)
            rinfo->r_ifrm_off = rinfo->r_ifrm_num - 1;

        ifrm = &rinfo->r_ifrm_buf[rinfo->r_ifrm_off];
        off = ifrm->ifrm_off * 188;
    
        if (scale == 1 && rinfo->r_ifrm_off == 0)
                off = 0;
        if (g_op->file_seek(rinfo->r_strm_fd, off))
            ERR_OUT("id = %08x file_seek %d %s\n", id, off, pvr->path);

        rinfo->r_strm_off = off;
    } else {
        if (rinfo->r_ifrm_off >= rinfo->r_ifrm_num)
            rinfo->r_ifrm_off = 0;
        rinfo->r_strm_off = 0;
    }

    rinfo->r_byte_size = 0;
    rinfo->r_byte_len = 0;

    rinfo->r_ts_len = 0;

    result = 0;
Err:
    return result;
}

int ind_pvr_open(uint32_t id, PvrElem_t* ppvr)
{
    int announce = PVR_ANNOUNCE_ERROR;
    PvrElem_t pvr;
    PvrSInfo_t sinfo;
    PvrRInfo_t rinfo = NULL;

    if (g_cfg == NULL)
        return -1;

    WRITE_MUTEX_LOCK( );
    READ_MUTEX_LOCK( );

    if (ppvr == NULL)
        ERR_OUT("id = %08x ppvr is NULL\n", id);
    pvr = int_pvr_hash_find(id);
    if (pvr == NULL) {
        announce = PVR_ANNOUNCE_LOST;
        ERR_OUT("id = %08x\n", id);
    }

    if (pvr == g_cfg->pvr)
        ERR_OUT("id = %08x pvr = %p already read\n", id, pvr);

    sinfo = &pvr->s_info;

    if (sinfo->encrypt && sinfo->key != g_cfg->key)
        ERR_OUT("id = %08x unauthorized! key = %lld / %lld\n", id, sinfo->key, g_cfg->key);

    if (sinfo->version <= 8)
        int_ifrm_convert(pvr);

    rinfo = (PvrRInfo*)IND_CALLOC(sizeof(PvrRInfo), 1);
    if (rinfo == NULL)
        ERR_OUT("id = %08x malloc PvrRInfo\n", id);

    rinfo->r_strm_fd = -1;
    rinfo->r_tspcr = ts_pcr_create( );

    rinfo->r_scale = 0;
    rinfo->r_clk_base = 0;

    pvr->r_info = rinfo;

    pvr->announce = PVR_ANNOUNCE_NONE;
    if (g_cfg->pvr)
        int_pvr_close_read(g_cfg->pvr);
    g_cfg->pvr = pvr;

    *ppvr = pvr;
    announce = PVR_ANNOUNCE_NONE;

Err:
    if (announce != PVR_ANNOUNCE_NONE && rinfo) {
        IND_FREE(rinfo);
        pvr->r_info = NULL;
    }
    READ_MUTEX_UNLOCK( );
    WRITE_MUTEX_UNLOCK( );
    return announce;
}

/*
    offset
    >=0：定位播放
    -1 ：从当前位置播放
    -2 ：从直播位置播放
 */
int ind_pvr_play(PvrElem_t pvr, int offset, int scale)
{
    uint32_t id;
    int result = -1;
    int total;
    PvrRInfo_t rinfo;
    PvrSInfo_t sinfo;

    if (g_cfg == NULL)
        return -1;

    READ_MUTEX_LOCK( );

    if (abs(scale) > 64)
        ERR_OUT("scale = %d\n", scale);

    CHECK_READ_HANDLE( );

    rinfo = pvr->r_info;
    sinfo = &pvr->s_info;
    id = pvr->s_info.id;

    PRINTF("id = %08x offset = %d, scale = %d\n", id, offset, scale);

    total = sinfo->time_len;

    if (offset < 0) {
        PvrRecord_t rec = pvr->w_rec;
        if (offset == -2) {
            if (g_stamp.flag != 1 || scale != 1)
                ERR_OUT("id = %08x stamp_flag = %d, scale = %d\n", id, g_stamp.flag, scale);
            offset = g_stamp.strm_sn * pvr->fragment_time + g_stamp.ifrm_off * IFRAME_CLK_INTERVAL / 100;
        } else {
            if (g_stamp.flag) {
                if (scale == 1)
                    goto End;
                PRINTF("id = %08x g_stamp scale = %d\n", id, scale);
                g_stamp.flag = 0;
            }
            offset = rinfo->r_strm_sn * pvr->fragment_time + rinfo->r_ifrm_off * IFRAME_CLK_INTERVAL / 100;
        }
        if (rec && pvr == rec->shift.pvr) {
            int shiftoffset = total - pvr->s_info.time_length;
            if (shiftoffset > 0 && offset <= shiftoffset)
                offset = shiftoffset + 1;
            else
            if (rinfo->r_scale == 1 && scale == 1)
                goto End;
        } else {
            if (rinfo->r_scale == 1 && scale == 1)
                goto End;
        }
    } else {
        offset += sinfo->time_base;
        if (g_stamp.flag) {
            PRINTF("id = %08x g_stamp offset = %d\n", id, offset);
            g_stamp.flag = 0;
        }
    }

    if (offset > total || offset < sinfo->time_base) {
        WARN_PRN("id = %08x offset = %d, total = %d\n", id, offset, total);
        if (offset > total)
            offset = total;
        else
            offset = sinfo->time_base;
    }
    PRINTF("id = %08x offset = %d, total = %d / %d\n", id, offset, total, sinfo->time_base);

    rinfo->r_ifrm_num = 0;
    result = int_pvr_seek(pvr, offset, scale);
    if (result)
        ERR_OUT("id = %08x ind_pvr_seek\n", id);

    rinfo->r_scale = scale;
    pvr->announce = PVR_ANNOUNCE_NONE;
    rinfo->r_clk_base = 0;
    ts_pcr_reset(rinfo->r_tspcr, 0);

End:

    rinfo->r_fast_end = 0;
    rinfo->r_fast_flag = 0;
    rinfo->r_fast_times = 0;

    result = 0;
Err:
    READ_MUTEX_UNLOCK( );
    return result;
}

static int int_rec_read(PvrElem_t pvr, char* buf, int len)
{
    int l, bytes;

    PvrRInfo *rinfo = pvr->r_info;
#ifdef DEBUG_BUILD
    uint32_t id = pvr->s_info.id;
#endif

    if (len <= 0)
        return 0;

    if (rinfo->r_ts_len > 0) {
        if (len <= rinfo->r_ts_len)
            return 0;
        IND_MEMCPY(buf, rinfo->r_ts_buf, rinfo->r_ts_len);
        bytes = rinfo->r_ts_len;
        rinfo->r_ts_len = 0;
    } else {
        bytes = 0;
    }

    if (pvr->s_info.encrypt)
        l = g_op->strm_read(rinfo->r_strm_fd, buf + bytes, len - bytes);
    else
        l = g_op->file_read(rinfo->r_strm_fd, buf + bytes, len - bytes);
    if (l < 0)
        ERR_OUT("id = %08x read len = %d, ret = %d\n", id, len - bytes, l);
    if (l == 0)
        goto End;

    bytes += l;

End:
    l = bytes % 188;
    if (l) {
        IND_MEMCPY(rinfo->r_ts_buf, buf, l);
        rinfo->r_ts_len = l;
        bytes -= l;
    }
    return bytes;
Err:
    return -1;
}

static void int_pvr_read_range(PvrElem_t pvr)
{
    ts_ifrm_t ifrm, ifrm_buf;
    int num, ifrm_off, ifrm_off1;

    PvrRInfo_t rinfo = pvr->r_info;

    num = rinfo->r_ifrm_num;
    ifrm_buf = rinfo->r_ifrm_buf;

    rinfo->r_byte_len = 0;

    for (ifrm_off = rinfo->r_ifrm_off; ifrm_off < num; ifrm_off ++) {
        ifrm = &ifrm_buf[ifrm_off];
        if (ifrm->ifrm_clk == 0)
            continue;
        if (ifrm->ifrm_size > 0)
            break;
    }
    rinfo->r_ifrm_off = ifrm_off;
    if (ifrm_off >= num) {
        if (rinfo->r_ifrm_off >= num)
            rinfo->r_byte_size = 0;
        else
            rinfo->r_byte_size = rinfo->r_strm_len - rinfo->r_strm_off;
        rinfo->r_ifrm_off1 = num;
    } else {
        for (ifrm_off1 = ifrm_off + 1; ifrm_off1 < num; ifrm_off1 ++) {
            ifrm = &ifrm_buf[ifrm_off1];
            if (ifrm->ifrm_clk == 0)
                continue;
            if (ifrm->ifrm_off <= rinfo->r_strm_off / 188)
                continue;
            if (ifrm->ifrm_size > 0)
                break;
        }
        if (ifrm_off1 >= num) {
            rinfo->r_byte_size = rinfo->r_strm_len - rinfo->r_strm_off;
            rinfo->r_ifrm_off1 = num;
        } else {
            rinfo->r_byte_size = (int)((uint32_t)ifrm_buf[ifrm_off1].ifrm_off) * 188 - rinfo->r_strm_off;
            rinfo->r_ifrm_off1 = ifrm_off1;
        }
        if (rinfo->r_byte_size < 0)
            rinfo->r_byte_size = 0;
    }
}

static int int_pvr_read_play(PvrElem_t pvr, uint32_t clk, char* buf, int len)
{
    uint32_t id;
    int bytes, length;

    PvrSInfo_t sinfo = &pvr->s_info;
    PvrRInfo_t rinfo = pvr->r_info;

    id = sinfo->id;

    if (sinfo->time_len < sinfo->time_base + 2) {
        if (pvr->w_rec) {
            DBG_PRN("id = %08x PVR_ANNOUNCE_WRITE sn = %d, time_len = %d / %d\n", id, rinfo->r_strm_sn, sinfo->time_len, sinfo->time_base);
            pvr->announce = PVR_ANNOUNCE_WRITE;
        } else {
            PRINTF("id = %08x PVR_ANNOUNCE_END sn = %d, time_len = %d / %d\n", id, rinfo->r_strm_sn, sinfo->time_len, sinfo->time_base);
            pvr->announce = PVR_ANNOUNCE_END;
        }
        return 0;
    }

    if (rinfo->r_clk_base == 0) {
        rinfo->r_clk_base = clk;
        rinfo->r_clk_last = clk;
        int_pvr_read_range(pvr);
    } else {
        uint32_t rclk;

        if (clk < rinfo->r_clk_last || clk > rinfo->r_clk_last + 300) {
            rinfo->r_clk_base = clk;
            ts_pcr_reset(rinfo->r_tspcr, 1);
        }
        rinfo->r_clk_last = clk;
        rclk = rinfo->r_clk_base + ts_pcr_time(rinfo->r_tspcr);
        if (clk < rclk)
            return 0;
    }

    if (rinfo->r_byte_size <= rinfo->r_byte_len) {
        int sec, endflag;

        sec = rinfo->r_strm_sn * pvr->fragment_time + rinfo->r_ifrm_off * IFRAME_CLK_INTERVAL / 100;
        if (sec > sinfo->time_len)
            endflag = 1;
        else
            endflag = 0;

        if (endflag == 0) {
            rinfo->r_ifrm_off = rinfo->r_ifrm_off1;
            int_pvr_read_range(pvr);
    
            if (rinfo->r_byte_size <= 0) {
                int maxsn = sinfo->time_len / pvr->fragment_time;
                if (sinfo->time_len <= maxsn * pvr->fragment_time && maxsn > 0)
                    maxsn --;
    
                if (rinfo->r_strm_sn < maxsn) {
                    if (int_pvr_seek(pvr, (rinfo->r_strm_sn + 1) * pvr->fragment_time, 1))
                        ERR_OUT("id = %08x int_pvr_seek\n", id);
                    int_pvr_read_range(pvr);
                }
            }
        }
        if (endflag == 1 || rinfo->r_byte_size <= 0) {
            if (pvr->w_rec) {
                DBG_PRN("id = %08x PVR_ANNOUNCE_WRITE sn = %d, endflag = %d\n", id, rinfo->r_strm_sn, endflag);
                pvr->announce = PVR_ANNOUNCE_WRITE;
            } else {
                PRINTF("id = %08x PVR_ANNOUNCE_END sn = %d, endflag = %d\n", id, rinfo->r_strm_sn, endflag);
                pvr->announce = PVR_ANNOUNCE_END;
            }
            return 0;
        }
        DBG_PRN("id = %08x sec = %d, endflag = %d, ifrm_off = %d\n", id, sec, endflag, rinfo->r_ifrm_off);
    }

    length = rinfo->r_byte_size - rinfo->r_byte_len;
    if (len > length)
        len = length;
    bytes = int_rec_read(pvr, buf, len);
    if (bytes < 0)
        ERR_OUT("id = %08x file_read %d / %d\n", id, len, length);

    if (bytes == 0) {
        PRINTF("id = %08x file_read %d / %d\n", id, len, length);
        rinfo->r_byte_size = 0;
        return 0;
    }
    ts_pcr_fill(rinfo->r_tspcr, buf, bytes);

    //PRINTF("bytes = %d / %d\n", bytes, bytes / 188);
    rinfo->r_strm_off += bytes;
    rinfo->r_byte_len += bytes;

    return bytes;
Err:
    return -1;
}

static int int_pvr_read_fast(PvrElem_t pvr, uint32_t clk, char* buf, int len)
{
    uint32_t id;
    int l;

    PvrSInfo_t sinfo = &pvr->s_info;
    PvrRInfo_t rinfo = pvr->r_info;

    id = sinfo->id;

    if (rinfo->r_byte_len >= rinfo->r_byte_size) {
        uint32_t rclk;
        int sn, offset, ifrm_off;

        if (rinfo->r_clk_base == 0) {
            rinfo->r_clk_base = clk;
            rinfo->r_fast_times = 0;
        }
        //以2倍速做为基准进行计算 200 两秒
        rclk = rinfo->r_clk_base + rinfo->r_fast_times * IFRAME_CLK_INTERVAL / 2;
        if (rclk > clk)
            return 0;
        rinfo->r_fast_times ++;

        if (rinfo->r_fast_end) {
            if (rinfo->r_scale > 0) {
                if (pvr->w_rec) {
                    PRINTF("id = %08x PVR_ANNOUNCE_WRITE\n", id);
                    pvr->announce = PVR_ANNOUNCE_WRITE;
                } else {
                    PRINTF("id = %08x PVR_ANNOUNCE_END\n", id);
                    pvr->announce = PVR_ANNOUNCE_END;
                }
            } else {
                PRINTF("id = %08x PVR_ANNOUNCE_BEGIN\n", id);
                pvr->announce = PVR_ANNOUNCE_BEGIN;
            }
            return 0;
        }

        if (sinfo->time_len < sinfo->time_base + 2) {
            PRINTF("id = %08x fast end time_len = %d / %d\n", id, sinfo->time_len, sinfo->time_base);
            rinfo->r_fast_end = 1;
            return 0;
        }
        if (sinfo->pcr) {
            int flag = 0;
            ifrm_off = rinfo->r_ifrm_off + 1;
            if ((rinfo->r_fast_times % 5) == 0) {
                flag = 1;
                rinfo->r_strm_off = 0;
                ifrm_off += (rinfo->r_scale/2 - 1) * 5;
            }
            if (g_virtual == 0) {
                    rinfo->r_fast_flag = 0;
            } else {
                if (flag == 1)
                    rinfo->r_fast_flag = 1;
                else if (rinfo->r_fast_flag == 1)
                    rinfo->r_fast_flag = 2;
                else
                    rinfo->r_fast_flag = 3;
            }
        } else {
            if (g_virtual == 0 || abs(rinfo->r_scale) <= 8) {
                ifrm_off = rinfo->r_ifrm_off + rinfo->r_scale / 2;
            } else {
                if (rinfo->r_scale < 0) {
                    ifrm_off = rinfo->r_ifrm_off - 2;
                    if ((rinfo->r_fast_times % 5) == 0)
                        ifrm_off += (rinfo->r_scale/2 + 2) * 5;
                } else {
                    ifrm_off = rinfo->r_ifrm_off + 2;
                    if ((rinfo->r_fast_times % 5) == 0)
                        ifrm_off += (rinfo->r_scale/2 - 2) * 5;
                }
            }
        }

        //边界检查
        offset = rinfo->r_strm_sn * pvr->fragment_time + ifrm_off * IFRAME_CLK_INTERVAL / 100;
        sn = rinfo->r_strm_sn;
        PRINTF("id = %08x sn = %d, ifrm_off = %d, ifrm_num = %d\n", id, sn, ifrm_off, rinfo->r_ifrm_num);

        if (rinfo->r_scale > 0) {
            int maxsn = sinfo->time_len / pvr->fragment_time;

            if (offset >= sinfo->time_len || (sn >= maxsn && ifrm_off >= rinfo->r_ifrm_num)) {
                PRINTF("id = %08x fast end offset = %d, time_len = %d, sn = %d / %d, ifrm_off = %d / %d\n", id, offset, sinfo->time_len, sn, maxsn, ifrm_off, rinfo->r_ifrm_num);
                rinfo->r_fast_end = 1;
            }

            if (ifrm_off >= rinfo->r_ifrm_num) {
                if (rinfo->r_fast_end == 0 && ifrm_off >= pvr->fragment_size && sn < maxsn) {
                    int_pvr_seek(pvr, (sn + 1) * pvr->fragment_time, rinfo->r_scale);
                    ifrm_off -= pvr->fragment_size;
                }
                if (ifrm_off >= rinfo->r_ifrm_num)
                    ifrm_off = rinfo->r_ifrm_num - 1;
            }
        } else {//rinfo->r_scale < 0
            int begin;
            PvrRecord_t rec;

            rec = pvr->w_rec;
            if (rec && pvr == rec->shift.pvr && sinfo->time_length < sinfo->time_len - sinfo->time_base)
                begin = sinfo->time_len - sinfo->time_length;//时移左端点
            else
                begin = sinfo->time_base;
            if (offset <= begin) {
                PRINTF("id = %08x fast end offset = %d, begin = %d\n", id, offset, begin);
                rinfo->r_fast_end = 1;
            }

            if (ifrm_off < 0) {
                if (sn > begin / pvr->fragment_time) {
                    int_pvr_seek(pvr, (sn - 1) * pvr->fragment_time, rinfo->r_scale);
                    ifrm_off += pvr->fragment_size;
                }
                if (ifrm_off < 0)
                    ifrm_off = 0;
            }
        }

        rinfo->r_ifrm_off = ifrm_off;
        {
            int i, r, num;
            ts_ifrm_t ifrm, ifrm_buf;

            r = abs(rinfo->r_scale) / 2 - 1;
            num = rinfo->r_ifrm_num - 1;
            ifrm_buf = rinfo->r_ifrm_buf;
            for (i = 0; i < r && ifrm_off < num; i ++, ifrm_off ++) {
                if (ifrm_buf[ifrm_off].ifrm_size != 0)
                    break;
            }

            ifrm = &rinfo->r_ifrm_buf[ifrm_off];

            offset = ifrm->ifrm_off * 188;
            if (sinfo->pcr && offset < rinfo->r_strm_off) {
                rinfo->r_byte_len = 0;
                rinfo->r_byte_size = 0;
            } else {
                if (g_op->file_seek(rinfo->r_strm_fd, offset))
                    ERR_OUT("id = %08x file_seek %d, ifrm_off = %d / %d, num = %d\n", id, offset, ifrm_off, ifrm->ifrm_off, num);

                rinfo->r_byte_len = 0;
                rinfo->r_byte_size = (uint32_t)(ifrm->ifrm_size) * 188;
                rinfo->r_strm_off = offset + rinfo->r_byte_size;
            }
        }

        //PRINTF("@@@@@@@@: begin = %d, end = %d, size = %d\n", offset, rinfo->r_strm_off, rinfo->r_byte_size);

        if (rinfo->r_fast_flag == 1 || rinfo->r_fast_flag == 2) {
            int i;

            buf[0] = 0x47;
            buf[1] = 0x1F;
            buf[2] = 0xFF;
            buf[3] = 0x30;
            buf[4] = 3;
            buf[5] = 0x00;
            buf[6] = 'F';

            for (i = 8; i < 188; i ++)
                buf[i] = 0xFF;

            if (rinfo->r_fast_flag == 1)
                buf[7] = '1';
            else
                buf[7] = '2';

            return 188;
        }
    }
    if (rinfo->r_byte_size <= 0)
        return 0;

    l = rinfo->r_byte_size - rinfo->r_byte_len;
    if (l > len)
        l = len;

    len = int_rec_read(pvr, buf, l);
    if (len == 0)
        rinfo->r_byte_size = rinfo->r_byte_len;
    else if (len < 0)
        ERR_OUT("id = %08x file_read ret = %d\n", id, len);
    rinfo->r_byte_len += len;

    return len;
Err:
    return -1;
}

int ind_pvr_read(PvrElem_t pvr, uint32_t clk, char* buf, int len)
{
    int result = -1;
    uint32_t id;
    PvrRInfo_t rinfo;
    PvrSInfo_t sinfo;

    if (g_cfg == NULL)
        return -1;

    READ_MUTEX_LOCK( );

    CHECK_READ_HANDLE( );

    id = pvr->s_info.id;
    if (pvr->announce && pvr->announce != PVR_ANNOUNCE_WRITE) {
        ERR_OUT("id = %08x announce = %d\n", id, pvr->announce);
        goto Err;
    }
    pvr->announce = PVR_ANNOUNCE_NONE;

    if (buf == NULL || len < 188)
        ERR_OUT("id = %08x buf = %p, len = %d\n", id, buf, len);
    len = len - len % 188;

    rinfo = pvr->r_info;
    sinfo = &pvr->s_info;

    if (rinfo->r_scale == 1) {
        if (g_stamp.flag) {
            if (g_stamp.strm_sn < rinfo->r_strm_sn) {
                PRINTF("id = %08x g_stamp strm_sn = %d / %d\n", id, rinfo->r_strm_sn, g_stamp.strm_sn);
                g_stamp.flag = 0;
            } else if (g_stamp.strm_sn == rinfo->r_strm_sn) {
                while (g_stamp.strm_off > rinfo->r_strm_off) {
                    int bytes = g_stamp.strm_off - rinfo->r_strm_off;
                    if (bytes > len)
                        bytes = len;
                    //PRINTF("id = %08x strm_off = %d, r_strm_off = %d, bytes = %d, len = %d\n", id, g_stamp.strm_off, rinfo->r_strm_off, bytes, len);
                    bytes = int_pvr_read_play(pvr, clk, buf, bytes);
                    if (bytes < 0)
                        ERR_OUT("id = %08x ind_pvr_read_play\n", id);
                    if (bytes == 0) {
                        result = 0;
                        goto Err;
                    }
                }
                PRINTF("id = %08x g_stamp strm_off = %d / %d\n", id, rinfo->r_strm_off, g_stamp.strm_off);
                g_stamp.flag = 0;
            }
        }
        result = int_pvr_read_play(pvr, clk, buf, len);
        if (result < 0)
            ERR_OUT("id = %08x ind_pvr_read_play\n", id);
    } else {
        result = int_pvr_read_fast(pvr, clk, buf, len);
        if (result < 0)
            ERR_OUT("id = %08x ind_pvr_read_fast\n", id);
    }

Err:
    if (result <= 0 && pvr->announce)
        result = pvr->announce;

    READ_MUTEX_UNLOCK( );
    return result;
}

void ind_pvr_close(PvrElem_t pvr)
{
    if (g_cfg == NULL)
        return;

    WRITE_MUTEX_LOCK( );
    READ_MUTEX_LOCK( );

    CHECK_READ_HANDLE( );

    PRINTF("id = %08x --------\n", pvr->s_info.id);

    int_pvr_close_read(pvr);
Err:
    READ_MUTEX_UNLOCK( );
    WRITE_MUTEX_UNLOCK( );
    return;
}

static void int_pvr_user_size(int size)
{
    int share_size;

    share_size = g_cfg->share_size;
    if (size <= share_size)
        return;

    if (size > 128 * 1024) {
        ERR_PRN("size = %d\n", size);
        return;
    }

    while (share_size < size)
        share_size *= 2;

    if (g_cfg->share_buf)
        IND_FREE(g_cfg->share_buf);
    g_cfg->share_buf = (char*)IND_MALLOC(share_size);
    g_cfg->share_size = share_size;

    PRINTF("share_buf = %p, share_size = %d\n", g_cfg->share_buf, g_cfg->share_size);
}

int ind_pvr_user_write(uint32_t id, char* buf, int len)
{
    int result = -1;
    int fd = -1;

    PvrElem_t pvr;
    uint32_t crc32;

    if (g_cfg == NULL)
        return -1;

    WRITE_MUTEX_LOCK( );

    if (buf == NULL || len <= 4)
        ERR_OUT("id = %08x buf = %p, len = %d\n", id, buf, len);

    pvr = int_pvr_hash_find(id);
    if (pvr == NULL)
        ERR_OUT("id = %08x int_pvr_hash_find\n", id);

    sprintf(pvr->path + g_cfg->pathlen + 9, "/%08x", id);

    fd = g_op->file_open(pvr->path, FLAGS_WRITE);
    if (fd == -1)
        ERR_OUT("id = %08x file_open %s\n", id, pvr->path);

    crc32 = ind_ts_crc32((unsigned char *)buf, len);
    if (g_op->file_write(fd, (char*)&crc32, 4) != 4)
        ERR_OUT("id = %08x file_write crc32\n", id);

    if (g_op->file_write(fd, buf, len) != len)
        ERR_OUT("id = %08x file_write %d\n", id, len);

    int_pvr_user_size(len);

    result = 0;
Err:
    WRITE_MUTEX_UNLOCK( );

    if (fd != -1) {
        g_op->file_sync(fd);
        g_op->file_close(fd);
        PRINTF("id = %08x file_sync\n", id);
    }

    return result;
}

static int int_pvr_user_read(PvrElem_t pvr, char* buf, int size)
{
    int fd = -1, result = -1;

    uint32_t id, crc0, crc1;
    int len;

    id = pvr->s_info.id;

    sprintf(pvr->path + g_cfg->pathlen + 9, "/%08x", id);
    fd = g_op->file_open(pvr->path, O_RDONLY);
    if (fd < 0) {
        ERR_PRN("id = %08x open %s\n", id, pvr->path);
        sprintf(pvr->path + g_cfg->pathlen + 9, "/data");//向前兼容
        fd = g_op->file_open(pvr->path, O_RDONLY);
    }
    if (fd < 0)
        ERR_OUT("id = %08x open %s\n", id, pvr->path);

    if (g_op->file_read(fd, (char*)&crc0, 4) != 4)
        ERR_OUT("id = %08x file_read crc32\n", id);

    len = g_op->file_read(fd, buf, size);
    if (len <= 4)
        ERR_OUT("id = %08x file_read ret = %d\n", id, len);

    crc1 = ind_ts_crc32((unsigned char *)buf, len);
    if (crc1 != crc0)
        ERR_OUT("id = %08x crc32 = %08x / %08x\n", id, crc0, crc1);

    result = len;
Err:
    if (fd >= 0)
        g_op->file_close(fd);
    return result;
}

int ind_pvr_user_read(uint32_t id, char* buf, int size)
{
    PvrElem_t pvr;
    int result = -1;

    if (g_cfg == NULL)
        return -1;

    WRITE_MUTEX_LOCK( );

    if (buf == NULL || size <= 0)
        ERR_OUT("id = %08x buf = %p, len = %d\n", id, buf, size);

    pvr = int_pvr_hash_find(id);
    if (pvr == NULL)
        ERR_OUT("id = %08x int_pvr_hash_find\n", id);

    result = int_pvr_user_read(pvr, buf, size);
    if (result < 0)
        ERR_OUT("id = %08x int_pvr_user_read\n", id);

    int_pvr_user_size(result);
Err:
    WRITE_MUTEX_UNLOCK( );
    return result;
}

int ind_pvr_user_share(uint32_t id, int flag)
{
    PvrElem_t pvr;
    char path[PVR_PATH_LEN];
    int l, fd, len, result = -1;

    PRINTF("id = %08x, flag = %d\n", id, flag);
    if (g_cfg == NULL)
        return -1;

    WRITE_MUTEX_LOCK( );

    sprintf(path, "%s/%s/%08x", g_cfg->path, SHARE_PATH_NAME, id);

    if (1 == flag) {
        if (NULL == g_cfg->share_buf)
            ERR_OUT("share_buf is NULL\n");

        pvr = int_pvr_hash_find(id);
        if (pvr == NULL)
            ERR_OUT("id = %08x int_pvr_hash_find\n", id);
    
        len = int_pvr_user_read(pvr, g_cfg->share_buf, g_cfg->share_size);
        if (len <= 0)
            ERR_OUT("id = %08x int_pvr_user_read\n", id);
    
        fd = g_op->file_open(path, FLAGS_WRITE);
        if (fd < 0)
            ERR_OUT("id = %08x file_open %s\n", id, path);
    
        l = sizeof(pvr->s_info);
        g_op->file_write(fd, (char*)&l, sizeof(l));
        g_op->file_write(fd,  (char*)&pvr->s_info, sizeof(pvr->s_info));
    
        g_op->file_write(fd,  (char*)&len, sizeof(len));
        g_op->file_write(fd, g_cfg->share_buf, len);

        g_op->file_close(fd);
    } else {
        g_op->file_delete(path);
    }

    result = 0;
Err:
    WRITE_MUTEX_UNLOCK( );
    return result;
}
