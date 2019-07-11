/**********************************************************
    Copyright (c) 2008-2009, Yuxing Software Corporation
    All Rights Reserved
    Confidential Property of Yuxing Softwate Corporation

    Revision History:

    Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#ifdef INCLUDE_PVR

#include "stream.h"
#include "stream_port.h"
#include "config/pathConfig.h"


//#define ENABLE_SAVE_RECORD

#define RECORD_INSTANCE_NUM     3

#define CMD_QUEUE_POOL          10

enum
{
    RECORD_STATE_CLOSE = 0,
    RECORD_STATE_OPEN,
    RECORD_STATE_CHECK,
    RECORD_STATE_NORMAL,
};

enum
{
    RECORD_CMD_NONE = 0,
    RECORD_CMD_OPEN,
    RECORD_CMD_CLOSE,
    RECORD_CMD_PAUSE,
    RECORD_CMD_RESUME,
    RECORD_CMD_MIX_OPEN,
    RECORD_CMD_MIX_CLOSE,
    RECORD_CMD_ARG,
};

typedef struct _PvrOpen PvrOpen;
typedef struct _PvrOpen* PvrOpen_t;
struct _PvrOpen {
    PvrOpen_t   next;
    PvrArgument pvrarg;
};

typedef struct _PvrClose PvrClose;
typedef struct _PvrClose* PvrClose_t;
struct _PvrClose {
    PvrClose_t  next;
    uint32_t        id;
    int         end;
};

typedef struct _RecCmd RecCmd;
typedef struct _RecCmd* RecCmd_t;
struct _RecCmd {
    RecCmd_t next;
    int cmd;
};

struct _StreamRecord {
    int         index;
    uint32_t        clk;

    mid_mutex_t loop_mutex;
    mid_mutex_t data_mutex;

    void*       msg_handle;
    MsgCallBack msg_callback;

#ifdef ENABLE_SAVE_RECORD
    FILE*       save_fp;
    long long   save_len;
#endif

    int         add;
    int         rec;
    int         end_flg;

    int         temp_flg;

    PvrOpen_t   pvr_arg;
    PvrArgument rec_arg;

    int         strm_off;
    int         strm_stamp;

    uint32_t        parse_clk;
    uint32_t        check_clk;
    uint32_t        over_clk;

    uint32_t        data_clk;
    int         data_tmout;

    int         disk_warn;//磁盘快满告警

    RecCmd_t    cmd_queue;
    RecCmd_t    cmd_pool;

    uint32_t        open_sn;
    void*       open_handle;
    MsgCallBack open_callback;

    PvrOpen_t   open_arg;
    PvrClose_t  close_arg;

    uint32_t        rec_sn;
    int         rec_end;

    int         state;
    int         encrypt;
    int         networkid;

    int         announce;
    int         realtime;
    uint32_t        checkbytes;

    ts_buf_t    ts_buf;
    int         ts_buf_size;

    ts_parse_t      ts_parse;
    struct ts_psi   ps_psi;
    struct ts_psi   ts_psi;

    int         pcr_mode;

    int         psi_exceps;
    int         crc_exceps;

    int         conaxlive;

    uint32_t        cat_clk;

    int         loops;
};

#define CAT_TIMEOUT_CLK        200

static void int_record_error(StreamRecord* sr);
static void int_record_close(StreamRecord* sr, int end);
static void int_record_loop(void *arg);
static void int_disk_push(StreamRecord* sr, int lock);
static int int_disk_check_open(StreamRecord* sr, PvrArgument_t pvr_arg);

//8M码流 流量为 1M字节/每秒
static int  g_timeshiftSec = 0;
static uint32_t g_timeshiftMB = 0;
static uint32_t g_fragmentMB = 60;

//static int g_shiftsize = 0;//时移数据大小

StreamRecord* strm_record_create(int idx, int buf_size)
{
    StreamRecord *sr = NULL;

    if (0 == g_timeshiftMB) {
        int fragment = ind_pvr_fragment( );

        g_timeshiftSec = 2 * 3600;
        g_timeshiftMB = 2 * 3600 + fragment * 2;
    }

    if (idx < 0 || idx >= RECORD_INSTANCE_NUM)
        LOG_STRM_ERROUT("index = %d\n", idx);

    sr = (StreamRecord *)IND_CALLOC(sizeof(StreamRecord), 1);
    if (sr == NULL)
        LOG_STRM_ERROUT("index = %d malloc\n", idx);

    sr->index = idx;

    sr->ts_buf_size = buf_size;

    sr->loop_mutex = mid_mutex_create( );
    sr->data_mutex = mid_mutex_create( );

    {
        int i;
        for (i = 0; i < CMD_QUEUE_POOL; i ++) {
            RecCmd_t rc = (RecCmd_t)IND_CALLOC(sizeof(RecCmd), 1);
            rc->next = sr->cmd_pool;
            sr->cmd_pool = rc;
        }
    }
    sr->rec = -1;

    stream_port_task_create(int_record_loop, sr);

    return sr;
Err:
    if (sr)
        IND_FREE(sr);
    return NULL;
}

void int_message(StreamRecord* sr, STRM_MSG msgno, int arg)
{
    if (sr->msg_callback == NULL)
        LOG_STRM_ERROR("msg_callback is NULL\n");
    else
        sr->msg_callback(sr->msg_handle, msgno, arg);
}

static void int_cmd_push(StreamRecord* sr, int cmd)
{
    RecCmd_t rc, prev;

    rc = sr->cmd_pool;
    if (rc) {
        rc = sr->cmd_pool;
        sr->cmd_pool = rc->next;
    } else {
        LOG_STRM_WARN("cmd array full!\n");
        rc = sr->cmd_queue;
        sr->cmd_queue = rc->next;
    }

    rc->cmd = cmd;
    rc->next = NULL;

    prev = sr->cmd_queue;
    if (prev) {
        while (prev->next)
            prev = prev->next;
        prev->next = rc;
    } else {
        sr->cmd_queue = rc;
    }
}

static int int_cmd_pop(StreamRecord* sr)
{
    int cmd;
    RecCmd_t rc;

    rc = sr->cmd_queue;
    if (!rc)
        return 0;
    cmd = rc->cmd;

    sr->cmd_queue = rc->next;

    rc->next = sr->cmd_pool;
    sr->cmd_pool = rc;

    return cmd;
}

void strm_record_open(StreamRecord* sr, void* msg_handle, MsgCallBack msg_callback, RecordArg_t arg)
{
    LOG_STRM_PRINTF("#%d ----\n", sr->index);

    mid_mutex_lock(sr->loop_mutex);

    sr->clk = mid_10ms( );

    mid_mutex_lock(sr->data_mutex);
    if (sr->state != RECORD_STATE_CLOSE) {
        LOG_STRM_WARN("#%d state = %d\n", sr->index, sr->state);
        int_record_close(sr, 0);
    }
    sr->ts_buf = ts_buf_create(sr->ts_buf_size);
    sr->ts_parse = ts_parse_create(&sr->ps_psi);
    sr->data_clk = sr->clk;//必须放在sr->clk = mid_10ms( );后面
    sr->data_tmout = 0;
    mid_mutex_unlock(sr->data_mutex);

    sr->open_sn ++;
    sr->open_handle     = msg_handle;
    sr->open_callback   = msg_callback;

    sr->open_arg = (PvrOpen_t)IND_CALLOC(sizeof(PvrOpen), 1);
    sr->open_arg->pvrarg = arg->pvrarg;
    sr->close_arg = NULL;

    sr->add = arg->add;
    sr->realtime = arg->pvrarg.realtime;
    sr->networkid = 0;

    sr->end_flg = 0;
    sr->temp_flg = 0;

    sr->parse_clk = 0;
    sr->check_clk = 0;
    sr->over_clk = 0;
    sr->strm_stamp = 0;

    sr->checkbytes = 5 * 1024 * 1024;//

    while (sr->cmd_queue)
        int_cmd_pop(sr);

    int_cmd_push(sr, RECORD_CMD_OPEN);
    mid_mutex_unlock(sr->loop_mutex);
}

void strm_record_netwkid(StreamRecord* sr, int networkid)
{
    sr->networkid = networkid;
}

void strm_record_mix_open(StreamRecord* sr, RecordArg_t arg)
{
    PvrOpen_t openarg;
    LOG_STRM_PRINTF("#%d ---- id = 0x%08x\n", sr->index, arg->pvrarg.id);

    mid_mutex_lock(sr->loop_mutex);

    openarg = (PvrOpen_t)IND_CALLOC(sizeof(PvrOpen), 1);
    openarg->pvrarg = arg->pvrarg;

    openarg->next = sr->open_arg;
    sr->open_arg = openarg;

    int_cmd_push(sr, RECORD_CMD_MIX_OPEN);

    mid_mutex_unlock(sr->loop_mutex);
}

static void int_record_mix_open(StreamRecord* sr)
{
    uint32_t id;
    PvrOpen_t openarg;

    LOG_STRM_PRINTF("#%d rec = %d\n", sr->index, sr->rec);
    if (sr->rec < 0)
        return;

    while (sr->open_arg) {
        openarg = sr->open_arg;
        sr->open_arg = openarg->next;

        if (int_disk_check_open(sr, &openarg->pvrarg)) {
            IND_FREE(openarg);
            continue;
        }
        id = openarg->pvrarg.id;
        openarg->pvrarg.psi = NULL;

        openarg->next = sr->pvr_arg;
        sr->pvr_arg = openarg;

        ind_pvr_mix_open(sr->rec, &openarg->pvrarg, sr->clk);

        LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_BEGIN id = %08x\n", sr->index, id);
        int_message(sr, RECORD_MSG_SUCCESS_BEGIN, id);
    }
}

static void int_record_mix_close(StreamRecord* sr)
{
    int end;
    uint32_t id;
    PvrOpen_t openarg, prev;
    PvrClose_t closearg;

    while (sr->close_arg) {
        closearg = sr->close_arg;
        sr->close_arg = closearg->next;

        id = closearg->id;
        end = closearg->end;
        IND_FREE(closearg);

        if (0 == id || NULL == sr->pvr_arg || !sr->pvr_arg->next) {
            LOG_STRM_ERROR("#%d id = %08x pvr_arg = %p\n", sr->index, id, sr->pvr_arg);
            continue;
        }

        prev = NULL;
        openarg = sr->pvr_arg;
        while (openarg) {
            if (id == openarg->pvrarg.id)
                break;
            prev = openarg;
            openarg = openarg->next;
        }

        if (NULL == openarg) {
            LOG_STRM_ERROR("#%d id = %08x\n", sr->index, id);
            continue;
        }

        if (sr->rec >= 0) {
            if (end == -1) {
                ind_pvr_mix_close(sr->rec, id, 0, sr->clk, 0);
                LOG_STRM_PRINTF("delete id = %08x\n", id);
                ind_pvr_delete(id, NULL);
            } else {
                ind_pvr_mix_close(sr->rec, id, end, sr->clk, mid_time());
                if (1 == end) {
                    LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_END\n", sr->index);
                    int_message(sr, RECORD_MSG_SUCCESS_END, id);
                }
            }
            //sr->rec = -1;此处不能对rec置-1，否则录制将被出错停止
        }
        if (prev)
            prev->next = openarg->next;
        else
            sr->pvr_arg = openarg->next;
        IND_FREE(openarg);
    }
}

static void int_record_open(StreamRecord* sr)
{
    LOG_STRM_PRINTF("#%d ----\n", sr->index);

    sr->rec_end = 0;

    mid_mutex_lock(sr->data_mutex);

    sr->msg_handle = sr->open_handle;
    sr->msg_callback = sr->open_callback;
    sr->rec_sn = sr->open_sn;

    mid_mutex_unlock(sr->data_mutex);

    sr->strm_off = 0;

    sr->disk_warn = 0;

    memset(&sr->ts_psi, 0, sizeof(struct ts_psi));
    memset(&sr->ps_psi, 0, sizeof(struct ts_psi));

    ts_parse_reset(sr->ts_parse);

    sr->ps_psi.emm_num = -1;
    sr->ts_psi.emm_num = -1;

    sr->pcr_mode = 0;

    sr->psi_exceps = 0;
    sr->crc_exceps = 0;

#ifdef ENABLE_SAVE_RECORD
    sr->save_len = 0;
    sr->save_fp = fopen(DEFAULT_EXTERNAL_DATAPATH"/play.ts", "wb");
    LOG_STRM_PRINTF("@@@@@@@@: fp = %p\n", sr->save_fp);
#endif

    sr->announce = PVR_ANNOUNCE_NONE;

    sr->state = RECORD_STATE_OPEN;
}

static int int_record_first(StreamRecord* sr)
{
    int flg;
    uint32_t clk;
    PvrOpen_t openarg;

    struct ts_psi *psi = &sr->ts_psi;

    clk = mid_10ms( );

    mid_mutex_lock(sr->data_mutex);
    sr->pvr_arg = sr->open_arg;
    sr->open_arg = NULL;
    mid_mutex_unlock(sr->data_mutex);

    if (NULL == sr->pvr_arg)
        LOG_STRM_ERROUT("#%d open_arg is NULL\n", sr->index);

    flg = 0;
    for (openarg = sr->pvr_arg; openarg; openarg = openarg->next) {
        openarg->pvrarg.psi = psi;
        openarg->pvrarg.networkid = sr->networkid;

        if (int_disk_check_open(sr, &openarg->pvrarg))
            continue;

        if (0 == flg) {
            if (openarg == sr->pvr_arg && sr->add == 1) {
                uint32_t id = openarg->pvrarg.id;
                int announce = PVR_ANNOUNCE_ERROR;

                sr->rec = ind_pvr_rec_reopen(id, clk, &announce);
                if (sr->rec < 0) {
                    if (announce == PVR_ANNOUNCE_END) {
                        LOG_STRM_ERROR("#%d RECORD_MSG_SUCCESS_END id = %08x\n", sr->index, id);
                    } else if (announce == PVR_ANNOUNCE_DAMAGE) {
                        LOG_STRM_ERROR("#%d RECORD_MSG_DATA_DAMAGE\n", sr->index);
                        int_message(sr, RECORD_MSG_DATA_DAMAGE, 0);
                    } else if (announce == PVR_ANNOUNCE_WRITE) {
                        LOG_STRM_ERROR("#%d RECORD_MSG_PVR_CONFLICT\n", sr->index);
                        int_message(sr, RECORD_MSG_PVR_CONFLICT, 0);
                    } else {
                        LOG_STRM_ERROR("#%d RECORD_MSG_ERROR announce = %d\n", sr->index, announce);
                        int_message(sr, RECORD_MSG_ERROR, 0);
                    }
                } else {
                    ind_pvr_rec_psi(sr->rec, psi);
                    flg = 1;
                }
            } else {
                sr->rec = ind_pvr_rec_open(&openarg->pvrarg, clk);
                if (sr->rec < 0) {
                    LOG_STRM_ERROR("#%d RECORD_MSG_DISK_ERROR ind_pvr_rec_open\n", sr->index);
                    int_message(sr, RECORD_MSG_DISK_ERROR, 0);
                } else {
                    flg = 1;
                }
            }
        } else {
            ind_pvr_mix_open(sr->rec, &openarg->pvrarg, clk);
        }

        LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_BEGIN id = %08x\n", sr->index, openarg->pvrarg.id);
        int_message(sr, RECORD_MSG_SUCCESS_BEGIN, openarg->pvrarg.id);
    }

    if (sr->rec < 0) {
        PRINTF("#%d rec = %d\n", sr->index, sr->rec);
        int_record_close(sr, 0);
        return -1;
    }

    return 0;
Err:
    int_record_error(sr);
    return -1;
}

static void int_record_close(StreamRecord* sr, int end)
{
    PvrOpen_t openarg;

    if (sr->state == RECORD_STATE_CLOSE)
        return;
    LOG_STRM_PRINTF("#%d ---- end = %d\n", sr->index, end);

#ifdef ENABLE_SAVE_RECORD
    LOG_STRM_PRINTF("@@@@@@@@: save_len = %d\n", sr->save_len);
    if (sr->save_fp) {
        fclose(sr->save_fp);
        sr->save_fp = NULL;
    }
#endif

    if (sr->rec >= 0) {
        if (end == -1) {
            uint32_t id;
            ind_pvr_rec_close(sr->rec, 0, sr->clk, 0);
            openarg = sr->pvr_arg;
            while (openarg) {
                id = openarg->pvrarg.id;
                LOG_STRM_PRINTF("delete id = %08x\n", id);
                ind_pvr_delete(id, NULL);
                openarg = openarg->next;
            }
        } else {
            ind_pvr_rec_close(sr->rec, end, sr->clk, mid_time( ));
        }
        sr->rec = -1;
    }
    while (sr->pvr_arg) {
        openarg = sr->pvr_arg;
        sr->pvr_arg = openarg->next;
        IND_FREE(openarg);
    }

    if (sr->ts_buf) {
        ts_buf_delete(sr->ts_buf);
        sr->ts_buf = NULL;
    }
    if (sr->ts_parse) {
        ts_parse_delete(sr->ts_parse);
        sr->ts_parse = NULL;
    }
    sr->state = RECORD_STATE_CLOSE;
}

static void int_record_arg(StreamRecord* sr)
{
    LOG_STRM_PRINTF("#%d ---- state = %d\n", sr->index, sr->state);

    if (RECORD_STATE_CLOSE == sr->state)
        return;
    if (RECORD_STATE_OPEN == sr->state) {
        if (sr->open_arg)
            sr->open_arg->pvrarg = sr->rec_arg;
    } else {
        ind_pvr_rec_arg(sr->rec, &sr->rec_arg);
    }
}

#define WAIT_IDLE_TIMES        100
void strm_record_break_point(StreamRecord* sr)
{
    int i;

    LOG_STRM_PRINTF("#%d ----\n", sr->index);

    for (i = 0; i < WAIT_IDLE_TIMES; i ++)  {
        if (i)
            usleep(5000);
        mid_mutex_lock(sr->loop_mutex);
        if (!sr->cmd_queue) {
            int_disk_push(sr, 1);//将最后一点数据写到磁盘上去
            if (sr->rec >= 0)
                ind_pvr_rec_break(sr->rec);
            i = WAIT_IDLE_TIMES;
        }
        mid_mutex_unlock(sr->loop_mutex);
    }
}

void strm_record_close(StreamRecord* sr, int end)
{
    LOG_STRM_PRINTF("#%d ---- state = %d, end = %d\n", sr->index, sr->state, end);

    mid_mutex_lock(sr->loop_mutex);

    if (sr->rec_sn == sr->open_sn)
        sr->rec_end = end;
    int_cmd_push(sr, RECORD_CMD_CLOSE);

    mid_mutex_unlock(sr->loop_mutex);
}

void strm_record_end(StreamRecord* sr)
{
    LOG_STRM_PRINTF("#%d ---- state = %d\n", sr->index, sr->state);
    sr->end_flg = 1;
}

void strm_record_arg(StreamRecord* sr, PvrArgument_t arg)
{
    LOG_STRM_PRINTF("#%d ---- state = %d\n", sr->index, sr->state);

    mid_mutex_lock(sr->loop_mutex);
    sr->rec_arg = *arg;
    int_cmd_push(sr, RECORD_CMD_ARG);
    mid_mutex_unlock(sr->loop_mutex);
}

void strm_record_pause(StreamRecord* sr)
{
    LOG_STRM_PRINTF("#%d ---- state = %d\n", sr->index, sr->state);

    mid_mutex_lock(sr->loop_mutex);
    int_cmd_push(sr, RECORD_CMD_PAUSE);
    mid_mutex_unlock(sr->loop_mutex);
}

void strm_record_resume(StreamRecord* sr)
{
    LOG_STRM_PRINTF("#%d ---- state = %d\n", sr->index, sr->state);

    mid_mutex_lock(sr->loop_mutex);
    int_cmd_push(sr, RECORD_CMD_RESUME);
    mid_mutex_unlock(sr->loop_mutex);
}

void strm_record_mix_close(StreamRecord* sr, uint32_t id, int end)
{
    PvrOpen_t prev, openarg;

    mid_mutex_lock(sr->loop_mutex);

    LOG_STRM_PRINTF("#%d id = %08x end = %d\n", sr->index, id, end);
    for (openarg = sr->open_arg; openarg; openarg = openarg->next)
        LOG_STRM_DEBUG("#%d open_arg = %08x\n", sr->index, openarg->pvrarg.id);
    for (openarg = sr->pvr_arg; openarg; openarg = openarg->next)
        LOG_STRM_DEBUG("#%d pvr_arg = %08x\n", sr->index, openarg->pvrarg.id);

    prev = NULL;
    openarg = sr->open_arg;
    while (openarg) {
        if (id == openarg->pvrarg.id)
            break;
        prev = openarg;
        openarg = openarg->next;
    }
    if (openarg) {
        if (prev)
            prev->next = openarg->next;
        else
            sr->open_arg = openarg->next;
        IND_FREE(openarg);
    } else {
        PvrClose_t closearg = (PvrClose_t)IND_CALLOC(sizeof(PvrClose), 1);

        closearg->id = id;
        closearg->end = end;

        closearg->next = sr->close_arg;
        sr->close_arg = closearg;

        int_cmd_push(sr, RECORD_CMD_MIX_CLOSE);
    }

    mid_mutex_unlock(sr->loop_mutex);
}

static void int_record_error(StreamRecord* sr)
{
    LOG_STRM_PRINTF("#%d ---- state = %d\n", sr->index, sr->state);

    int_record_close(sr, 0);

    LOG_STRM_PRINTF("#%d RECORD_MSG_ERROR\n", sr->index);
    int_message(sr, RECORD_MSG_ERROR, 0);
}

void strm_record_emm(StreamRecord* sr, int *emm_pid, int *emm_num)
{
    int i;
    struct ts_psi *psi = &sr->ts_psi;

    mid_mutex_lock(sr->data_mutex);

    if (psi->emm_num > 0) {
        int num = psi->emm_num;
        LOG_STRM_PRINTF("#%d emm_num = %d! pcr_pid = 0x%x / (0x%x, 0x%x)\n", sr->index, psi->emm_num, psi->pcr_pid, psi->video_pid, psi->audio_pid[0]);

        if (psi->pcr_pid != psi->video_pid && psi->pcr_pid != psi->audio_pid[0]) {
            if (num >= TS_EMM_NUM)
                num = TS_EMM_NUM - 1;
            for (i = 0; i < num; i ++)
                emm_pid[i] = (int)((uint32_t)psi->emm_array[i].pid);
            emm_pid[num] = psi->pcr_pid;
            *emm_num = num + 1;
        } else {
            *emm_num = num;
            for (i = 0; i < psi->emm_num; i ++)
                emm_pid[i] = (int)((uint32_t)psi->emm_array[i].pid);
        }
    } else {
        *emm_num = 0;
    }

    mid_mutex_unlock(sr->data_mutex);
}

static int int_disk_check_open(StreamRecord* sr, PvrArgument_t arg)
{
    uint32_t totalMB, freeMB;

    if (record_port_disk_size(&totalMB, &freeMB)) {
        int_message(sr, RECORD_MSG_DISK_ERROR, arg->id);
        LOG_STRM_ERROUT("#%d RECORD_MSG_DISK_ERROR\n", sr->index);
    }

    LOG_STRM_DEBUG("#%d total = %d, free = %d, reserveMB = %u\n", sr->index, totalMB, freeMB, g_timeshiftMB);

    if (arg->info_len <= 0) {
        if (freeMB <= g_fragmentMB * 2) {
            int_message(sr, RECORD_MSG_DISK_FULL, arg->id);
            LOG_STRM_ERROUT("#%d RECORD_MSG_DISK_FULL shift totalMB = %u, freeMB = %u, timeshiftMB = %u\n", sr->index, totalMB, freeMB, g_timeshiftMB);
        }
    } else {
        uint32_t reserveMB = g_timeshiftMB + g_fragmentMB * 3 + totalMB / 100;
        if (freeMB <= reserveMB) {
            int_message(sr, RECORD_MSG_DISK_FULL, arg->id);
            LOG_STRM_ERROUT("#%d RECORD_MSG_DISK_FULL record totalMB = %u, freeMB = %u, reserveMB = %u\n", sr->index, totalMB, freeMB, reserveMB);
        }
    }

    return 0;
Err:
    return -1;
}

static int int_disk_check_loop(StreamRecord* sr)
{
    int ret = -1;
    PvrOpen_t openarg;
    PvrArgument_t arg;
    uint32_t totalMB, freeMB;

    if (record_port_disk_size(&totalMB, &freeMB)) {
        int_record_close(sr, 0);
        LOG_STRM_ERROR("#%d RECORD_MSG_DISK_ERROR\n", sr->index);
        int_message(sr, RECORD_MSG_DISK_ERROR, 0);
        goto Err;
    }

    LOG_STRM_PRINTF("#%d total = %d, free = %d, reserveMB = %u\n", sr->index, totalMB, freeMB, g_timeshiftMB);

    ret = 0;
    for (openarg = sr->pvr_arg; openarg; openarg = openarg->next) {
        arg = &openarg->pvrarg;

        if (arg->info_len <= 0) {
            if (freeMB <= g_fragmentMB * 2) {
                LOG_STRM_ERROR("#%d RECORD_MSG_DISK_FULL shift totalMB = %u, freeMB = %u, timeshiftMB = %u\n", sr->index, totalMB, freeMB, g_timeshiftMB);
                int_message(sr, RECORD_MSG_DISK_FULL, arg->id);
                ret = -1;
            }
        } else {
            uint32_t reserveMB = g_timeshiftMB + g_fragmentMB * 3 + totalMB / 100;
            if (freeMB <= reserveMB) {
                LOG_STRM_ERROR("#%d RECORD_MSG_DISK_FULL record totalMB = %u, freeMB = %u, reserveMB = %u\n", sr->index, totalMB, freeMB, g_timeshiftMB);
                int_message(sr, RECORD_MSG_DISK_FULL, arg->id);
                ret = -1;
            } else {
                //百分之九十报警
                if (sr->disk_warn == 0 && (freeMB - reserveMB) * 10 < (totalMB - reserveMB)) {
                    sr->disk_warn = 1;
                    LOG_STRM_WARN("#%d RECORD_MSG_DISK_WARN totalMB = %u, freeMB = %u, reserveMB = %u\n", sr->index, totalMB, freeMB, g_timeshiftMB);
                    int_message(sr, RECORD_MSG_DISK_WARN, 0);
                }
            }
        }
    }

Err:
    return ret;
}

static int int_disk_put(StreamRecord* sr, char *buf, int len)
{
    int ret;
    struct ts_psi *psi = &sr->ps_psi;

    ret = ts_parse_psi(sr->ts_parse, (u_char *)buf, len, NULL);
    //if (ret == 1)
    //    LOG_STRM_PRINTF("#%d ca_flag = %d, dvdspu_num = %d, subtitle_num = %d\n", sr->index, psi->ca_flag, psi->dvdspu_num, psi->subtitle_num);

    if (ret == 1) {
        if (psi->video_pid == 0 && psi->audio_num == 0) {
            int_record_error(sr);
            LOG_STRM_ERROUT("#%d empty media\n", sr->index);
        }
        if (ts_psi_equal(&sr->ts_psi, psi) == 0) {
            sr->psi_exceps ++;
            sr->crc_exceps = 0;
            if (sr->psi_exceps >= 5) {
                sr->psi_exceps = 0;

                LOG_STRM_PRINTF("#%d STRM_MSG_CHANGE_PSI\n", sr->index);
                sr->msg_callback(sr->msg_handle, STRM_MSG_CHANGE_PSI, 0);

                ts_psi_copy(&sr->ts_psi, psi);
                psi->emm_num = -1;
                sr->cat_clk = sr->clk + CAT_TIMEOUT_CLK;

                ret = ind_pvr_rec_psi(sr->rec, psi);

                if (ret) {
                    int_record_error(sr);
                    LOG_STRM_ERROUT("#%d ind_pvr_rec_psi\n", sr->index);
                }
            }
        } else {
            sr->psi_exceps = 0;

            if (psi->pmt_crc != sr->ts_psi.pmt_crc) {
                sr->crc_exceps ++;
                if (sr->crc_exceps >= 5) {
                    sr->crc_exceps = 0;
                    LOG_STRM_PRINTF("#%d change pmt_crc: %08x > %08x\n", sr->index, sr->ts_psi.pmt_crc, psi->pmt_crc);
                    ts_psi_copy(&sr->ts_psi, psi);
                    LOG_STRM_PRINTF("#%d PMT STRM_MSG_CHANGE_CRC\n", sr->index);
                    sr->msg_callback(sr->msg_handle, STRM_MSG_CHANGE_CRC, 2);
                }
            } else {
                sr->crc_exceps = 0;

                if (psi->emm_num > 0) {
                    if (ts_cat_equal(&sr->ts_psi, psi) == 0) {
                        LOG_STRM_PRINTF("#%d change emm_num: %d > %d\n", sr->index, sr->ts_psi.emm_num, psi->emm_num);
                        ts_psi_copy(&sr->ts_psi, psi);
                        ind_pvr_rec_psi(sr->rec, psi);
                        LOG_STRM_PRINTF("#%d CAT STRM_MSG_CHANGE_CRC\n", sr->index);
                        sr->msg_callback(sr->msg_handle, STRM_MSG_CHANGE_CRC, 1);
                    }
                    psi->emm_num = 0;
                }
            }
        }
    }

#ifdef ENABLE_SAVE_RECORD
    if (sr->save_fp) {
        ret = fwrite(buf, 1, len, sr->save_fp);
        if (ret != len)
            LOG_STRM_WARN("fwrite ret = %d, len = %d\n", ret, len);
        sr->save_len += len;
    }
#endif

    if (sr->conaxlive) {
        psi = &sr->ts_psi;

        uint32_t pid;
        uint8_t *ubuf;
        int i, j, idx, v, val;

        idx = 0;
        val = 0;
        ubuf = (uint8_t *)buf;
        for (i = 0; i < len; i += 188, ubuf += 188) {
            pid =(((uint32_t)ubuf[1] & 0x1f) << 8) + ubuf[2];
            v = 0;

            if (pid == psi->video_pid || pid == psi->pcr_pid || pid == 0 || pid == psi->pmt_pid)
                v = 1;

            if (v == 0) {
                int audio_num = psi->audio_num;
                unsigned int *audio_pid = psi->audio_pid;
                for (j = 0; j < audio_num; j ++) {
                    if (pid == audio_pid[j]) {
                        v = 1;
                        break;
                    }
                }
            }
            if (v == 0) {
                int ecm_num = psi->ecm_num;
                ts_ca_t ecm_array = psi->ecm_array;
                for (j = 0; j < ecm_num; j ++) {
                    if (pid == (uint32_t)ecm_array[j].pid) {
                        v = 1;
                        break;
                    }
                }
            }
            //if (v == 0 && psi->dr_teletext && pid == psi->dr_teletext.pid)
            //    v = 1;

            if (v) {
                if (val == 0) {
                    idx = i;
                    val = 1;
                }
            } else {
                if (val) {
                    //LOG_STRM_PRINTF("pid = %04x\n", pid);
                    ret = ind_pvr_rec_write(sr->rec, buf + idx, i - idx);
                    if (ret) {
                        int_record_close(sr, 0);
                        int_message(sr, RECORD_MSG_DISK_ERROR, 0);
                        LOG_STRM_ERROUT("#%d RECORD_MSG_DISK_ERROR ind_rec_write ret = %d\n", sr->index, ret);
                    }
                    val = 0;
                }
            }
        }

        if (val == 1) {
            //LOG_STRM_PRINTF("pid = %04x\n", pid);
            ret = ind_pvr_rec_write(sr->rec, buf + idx, i - idx);
            if (ret) {
                int_record_close(sr, 0);
                int_message(sr, RECORD_MSG_DISK_ERROR, 0);
                LOG_STRM_ERROUT("#%d RECORD_MSG_DISK_ERROR ind_rec_write ret = %d\n", sr->index, ret);
            }
        }
    } else {
        ret = ind_pvr_rec_write(sr->rec, buf, len);
        if (ret) {
            int_record_close(sr, 0);
            int_message(sr, RECORD_MSG_DISK_ERROR, 0);
            LOG_STRM_ERROUT("#%d RECORD_MSG_DISK_ERROR ind_rec_write ret = %d\n", sr->index, ret);
        }
    }

    return 0;
Err:
    return -1;
}

static void int_disk_push(StreamRecord* sr, int lock)
{
    int ret, length, len;
    char *buf;
    ts_buf_t tb = sr->ts_buf;
    struct ts_psi *psi = &sr->ps_psi;

    if (sr->data_clk && 0 == sr->end_flg) {
        uint32_t clk = mid_10ms( );
        if (lock)
            mid_mutex_lock(sr->data_mutex);
        if (!sr->data_tmout && clk > sr->data_clk + INTERVAL_CLK_DATA_TIMEOUT) {
            LOG_STRM_PRINTF("#%d RECORD_MSG_DATA_TIMEOUT, %d\n", sr->index, clk - sr->data_clk);
            int_message(sr, RECORD_MSG_DATA_TIMEOUT, 0);
            sr->data_tmout = 1;
            sr->data_clk = clk;
        }
        if (lock)
            mid_mutex_unlock(sr->data_mutex);
    }

    if (sr->state == RECORD_STATE_OPEN) {

        if (sr->parse_clk < sr->clk) {
            if (sr->parse_clk)
                LOG_STRM_WARN("#%d parse timeout!\n", sr->index);
            sr->parse_clk = sr->clk + 200;
        }

        if (lock)
            mid_mutex_lock(sr->data_mutex);
        ret = 0;
        length = ts_buf_length(tb);
        if (sr->strm_off < length) {
            ts_buf_read_get(tb, &buf, &len);
            ret = ts_parse_psi(sr->ts_parse, (u_char *)(buf + sr->strm_off), (len - sr->strm_off), NULL);
            if (sr->realtime) {
                ts_buf_reset(tb);
                length = 0;
            }
        }
        if (lock)
            mid_mutex_unlock(sr->data_mutex);

        if (ret == 1) {
            if (psi->video_pid == 0 && psi->audio_num == 0) {
                int_record_error(sr);
                LOG_STRM_ERROUT("#%d empty media\n", sr->index);
            }
            if (sr->realtime) {
                int i;

                sr->conaxlive = 0;
                for (i = 0; i < psi->ecm_num; i ++) {
                    if ((psi->ecm_array[i].system_id & 0xff00) == 0x0b00) {
                        //sr->conaxlive = 1;
                        break;
                    }
                }
                LOG_STRM_DEBUG("#%d conaxlive = %d, ecm_num = %d\n", sr->index, sr->conaxlive, psi->ecm_num);
            }

            ts_psi_copy(&sr->ts_psi, psi);
            LOG_STRM_PRINTF("#%d strm_off = %d, video = %d/%d, audio = %d/%d\n",
                    sr->index, sr->strm_off, psi->video_pid, psi->video_type, psi->audio_pid[0], psi->audio_type[0]);
            int_message(sr, RECORD_MSG_PSI_VIEW, 0);
            sr->strm_off = 0;
            psi->emm_num = -1;

            ret = int_record_first(sr);
            if (ret)
                LOG_STRM_ERROUT("#%d int_record_first\n", sr->index);
            ts_parse_reset(sr->ts_parse);

            sr->state = RECORD_STATE_CHECK;

        } else {
            if (length >= STREAM_BLOCK_LEVEL_RECORD * 7 / 8) {
                LOG_STRM_ERROR("#%d unknow media type\n", sr->index);
                ts_buf_reset(tb);
                length = 0;
            }
            sr->strm_off = length;
        }
    }

    if (sr->state == RECORD_STATE_CHECK) {
        length = ts_buf_length(tb);

        if (sr->strm_off < length) {
            psi = &sr->ts_psi;
            ts_buf_read_get(tb, &buf, &len);
            ret = ts_index_check(buf + sr->strm_off, len - sr->strm_off, psi->video_pid, psi->audio_pid[0]);
            if (ret || length >= STREAM_BLOCK_LEVEL_RECORD * 9 / 10) {
                struct ts_psi *p = &sr->ts_psi;

                LOG_STRM_PRINTF("#%d ts_index_check = %d\n", sr->index, ret);
                if (ret == -1 || (ret == 0 && p->ecm_num > 0 && (p->ecm_array[0].system_id & 0xff00) == 0x0b00)) {

                    sr->pcr_mode = 1;//由PCR计算时间
                    ind_pvr_rec_pcr(sr->rec, 1);//不检测I帧
                }
                sr->strm_off = 0;
                sr->state = RECORD_STATE_NORMAL;
            } else {
                sr->strm_off = length;
            }
        }
    }

    if (sr->state != RECORD_STATE_NORMAL)
        return;

    if (sr->check_clk <= sr->clk) {
        sr->check_clk = sr->clk + 500;
        if (int_disk_check_loop(sr)) {
            LOG_STRM_ERROR("#%d int_disk_check\n", sr->index);
            return;
        }
    }

    for (;;) {
        if (lock)
            mid_mutex_lock(sr->data_mutex);
        ts_buf_read_get(tb, &buf, &len);
        if (lock)
            mid_mutex_unlock(sr->data_mutex);

        if (len <= 0 || len % 188)
            break;
        if (len > 1316 * 32)//2011-5-17 22:40:37 当片源加扰时，底层PVR算法需要按小块来处理
            len = 1316 * 32;

        if (sr->strm_stamp > 0) {
            int l = sr->strm_stamp - sr->strm_off;
            if (l > 0 && len > l)
                len = l;
        }

        if (int_disk_put(sr, buf, len))
            LOG_STRM_ERROUT("#%d int_disk_put\n", sr->index);

        if (sr->strm_stamp > 0) {
            sr->strm_off += len;
            if (sr->strm_off >= sr->strm_stamp) {
                sr->strm_off = 0;
                sr->strm_stamp = 0;
                ind_pvr_rec_stamp(sr->rec);
            }
        }

        if (lock)
            mid_mutex_lock(sr->data_mutex);
        ts_buf_read_pop(tb, len);
        if (lock)
            mid_mutex_unlock(sr->data_mutex);

        if (sr->cmd_queue) {
            LOG_STRM_PRINTF("#%d cmd = %d\n", sr->index, sr->cmd_queue->cmd);
            break;
        }
    }
Err:
    return;
}

uint32_t strm_record_shifttime(void)
{
    return g_timeshiftSec;
}

int strm_timeshift_size(void)
{
    uint32_t reserveMB, totalMB, freeMB;

    if (record_port_disk_size(&totalMB, &freeMB))
        LOG_STRM_ERROUT("record_port_disk_size\n");
    reserveMB = g_timeshiftMB;
    if (g_fragmentMB * 2 >= totalMB / 100)
        reserveMB += g_fragmentMB * 2;
    else
        reserveMB += totalMB / 100;
    if (freeMB <= reserveMB)
        return 0;
    return (freeMB - reserveMB);
Err:
    return -1;
}

int strm_record_size(void)
{
    uint32_t reserveMB, totalMB, freeMB;

    if (record_port_disk_size(&totalMB, &freeMB)) {
        LOG_STRM_ERROR("record_port_disk_size\n");
        return -1;
    }
    reserveMB = g_timeshiftMB + g_fragmentMB * 3 + totalMB / 100;
    if (totalMB < reserveMB)
        return -2;
    if (freeMB <= reserveMB)
        return 0;
    return (int)(freeMB - reserveMB);
}

int strm_record_space(StreamRecord* sr)
{
    int len = 0;

    mid_mutex_lock(sr->data_mutex);
    if (sr->ts_buf)
        len = ts_buf_size(sr->ts_buf) - ts_buf_length(sr->ts_buf);
    mid_mutex_unlock(sr->data_mutex);

    return len;
}

void strm_record_push(StreamRecord* sr, char *data_buf, int data_len, uint32_t clk)
{
    int len;
    char *buf;

    if (clk == 0)
        clk = mid_10ms( );
    sr->clk = clk;

    if (data_len <= 0)
        return;

    mid_mutex_lock(sr->data_mutex);
    if (!sr->ts_buf)
        goto Err;

    sr->data_clk = clk;
    if (sr->data_tmout) {
        LOG_STRM_PRINTF("#%d RECORD_MSG_DATA_RESUME\n", sr->index);
        int_message(sr, RECORD_MSG_DATA_RESUME, 0);
        sr->data_tmout = 0;
    }

    if (data_len % 188)
        LOG_STRM_ERROUT("#%d data_len = %d\n", sr->index, data_len);

    for (len = 0; len < data_len; len += 188) {
        if (data_buf[len] != 0x47)
            LOG_STRM_ERROUT("#%d sync[%d] = 0x%02x\n", sr->index, len, (uint32_t)((uint8_t)data_buf[len]));
    }

    len = ts_buf_length(sr->ts_buf);

    if (sr->over_clk) {
        if (sr->over_clk > clk)
            goto Err;
        sr->over_clk = 0;
    }

    if (data_len + len > ts_buf_size(sr->ts_buf)) {
        if (sr->over_clk == 0) {
            sr->over_clk = mid_10ms( ) + 100;
            LOG_STRM_ERROR("#%d ########: OVERFLOW! data_len = %d, len = %d, loops = %d\n", sr->index, data_len, len, sr->loops);
            ts_buf_reset(sr->ts_buf);
        }
        goto Err;
    }

    while(data_len > 0) {
        ts_buf_write_get(sr->ts_buf, &buf, &len);
        if (len <= 0)
            LOG_STRM_ERROUT("#%d ts_buf_write_get %d\n", sr->index, len);
        if (len > data_len)
            len = data_len;
        IND_MEMCPY(buf, data_buf, len);
        data_buf += len;
        data_len -= len;
        ts_buf_write_put(sr->ts_buf, len);
    }

Err:
    mid_mutex_unlock(sr->data_mutex);
    return;
}

void strm_record_stamp(StreamRecord* sr)//直播转时移平滑处理
{
    LOG_STRM_PRINTF("#%d ---- state = %d\n", sr->index, sr->state);

    mid_mutex_lock(sr->loop_mutex);
    mid_mutex_lock(sr->data_mutex);

    if (RECORD_STATE_NORMAL == sr->state) {
        int_disk_push(sr, 0);
        ind_pvr_rec_stamp(sr->rec);
        sr->strm_stamp = 0;
    } else {
        sr->strm_stamp = ts_buf_length(sr->ts_buf);
        LOG_STRM_PRINTF("#%d ---- strm_stamp = %d\n", sr->index, sr->strm_stamp);
    }
    mid_mutex_unlock(sr->data_mutex);
    mid_mutex_unlock(sr->loop_mutex);
}

static void int_record_loop(void *arg)
{
    int cmd;
    StreamRecord *sr = (StreamRecord*)arg;

    LOG_STRM_PRINTF("#%d tid = 0x%x\n", sr->index, (uint32_t)pthread_self());

    while(1) {
        cmd = 0;

        mid_mutex_lock(sr->loop_mutex);

        sr->loops ++;

        if (sr->cmd_queue) {
            cmd = int_cmd_pop(sr);

            switch(cmd) {
            case RECORD_CMD_OPEN:
                sr->loops = 0;
                sr->check_clk = 0;//检测磁盘
                LOG_STRM_PRINTF("#%d RECORD_CMD_OPEN\n", sr->index);
                int_record_open(sr);
                break;
            case RECORD_CMD_CLOSE:
                LOG_STRM_PRINTF("#%d RECORD_CMD_CLOSE rec_end = %d\n", sr->index, sr->rec_end);
                if (sr->state != RECORD_STATE_CLOSE)
                    int_disk_push(sr, 1);//HTTP下载完毕，将最后一点数据写到磁盘上去
                if (sr->state != RECORD_STATE_CLOSE) {
                    uint32_t id = 0;
                    if (sr->pvr_arg)
                        id = sr->pvr_arg->pvrarg.id;
                    else if (sr->open_arg)
                        id = sr->open_arg->pvrarg.id;
                    int_record_close(sr, sr->rec_end);
                    if (sr->rec_end == 1) {
                        LOG_STRM_PRINTF("#%d RECORD_MSG_SUCCESS_END id = 0x%08x\n", sr->index, id);
                        int_message(sr, RECORD_MSG_SUCCESS_END, id);
                    }
                }
                break;

            case RECORD_CMD_PAUSE:
                sr->data_clk = 0;
                break;
            case RECORD_CMD_RESUME:
                sr->data_clk = mid_10ms( );
                break;

            case RECORD_CMD_MIX_OPEN:
                LOG_STRM_PRINTF("#%d RECORD_CMD_MIX_OPEN\n", sr->index);
                if (sr->open_arg) {
                    sr->check_clk = 0;
                    int_record_mix_open(sr);
                }
                break;
            case RECORD_CMD_MIX_CLOSE:
                LOG_STRM_PRINTF("#%d RECORD_CMD_MIX_CLOSE\n", sr->index);
                if (sr->close_arg)
                    int_record_mix_close(sr);
                break;
            case RECORD_CMD_ARG:
                int_record_arg(sr);
                break;
            default:
                break;
            }
        } else {
            if (sr->state != RECORD_STATE_CLOSE && sr->ts_buf)
                int_disk_push(sr, 1);
        }

        mid_mutex_unlock(sr->loop_mutex);
        if (cmd == 0) {
            if (RECORD_STATE_CLOSE == sr->state)
                usleep(1000 * 100);
            else
                usleep(1000 * 10);
        }
    }
}

//最大码率 bitrate，本地时移时长 second
void mid_record_set_reserve(uint32_t bitrate, uint32_t second)
{
    int fragment = ind_pvr_fragment( );

    LOG_STRM_PRINTF("bitrate = %d, second = %d\n", bitrate, second);
    if (bitrate < 8*1024 || bitrate > 25 * 1024 * 1024)
        LOG_STRM_ERROUT("bitrate = %d", bitrate);
    if (second > 24 * 3600)
        LOG_STRM_ERROUT("second = %d", second);
    g_fragmentMB = (bitrate / 8 /1024 * fragment) / 1024;
    g_timeshiftMB = (bitrate / 8 /1024 * (second + fragment * 2)) / 1024;
    g_timeshiftSec = second;

Err:
    return;
}

uint32_t mid_record_get_reserve(uint32_t total)
{
    struct PVRInfo info;
    uint32_t reserve, shift;

    memset(&info, 0, sizeof(info));
    ind_pvr_get_info(0, &info);
    shift = (uint32_t)(info.byte_len / 1024 / 1024);

    reserve = g_timeshiftMB + g_fragmentMB * 3 + total / 100;
    if (reserve > shift)
        reserve -= shift;
    return reserve;
}
#endif//INCLUDE_PVR
