#include "CpvrAssertions.h"
#include "CpvrAuxTools.h"
#include "CpvrList.h"
#include "CpvrDB.h"

#include "disk_info.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/queue.h>
#include <semaphore.h>

typedef struct _info_node {
    cpvr_task_info_t info;
    cpvr_task_live_t live;
    TAILQ_ENTRY(_info_node)list;
}_info_node_t;
TAILQ_HEAD(info_hdr_t, _info_node);
static struct info_hdr_t info_hdr = TAILQ_HEAD_INITIALIZER(info_hdr);
static struct info_hdr_t temp_info_hdr = TAILQ_HEAD_INITIALIZER(temp_info_hdr);

typedef struct _active_info_node {
    cpvr_task_info_t info;
    cpvr_task_live_t live;
    TAILQ_ENTRY(_active_info_node)list;
}_active_info_node_t;
TAILQ_HEAD(active_info_hdr_t, _active_info_node);
static struct active_info_hdr_t active_info_hdr = TAILQ_HEAD_INITIALIZER(active_info_hdr);

static sem_t    info_sem, active_info_sem, tmp_info_sem;;

static char cpvr_list_valid = 0;
#define cpvr_list_check() if (!cpvr_list_valid) return -1

namespace Hippo {
static int hash_value_get(void *key, int key_len)
{
    int hash_value = 0;
    char tmp_value[4];

    if (key_len <= 4) {
        //
        // If the key length is <= 4, the hash is just the key expressed as an integer
        //
        memset(tmp_value, 0, 4);
        memcpy(tmp_value, key, key_len);

        hash_value = *((int*)tmp_value);
    } else {
        //
        // If the key length is >4, the hash is the first 4 bytes XOR with the last 4
        //
        memcpy(tmp_value, key, 4);
        hash_value = *((int*)tmp_value);
        memcpy(tmp_value, (char*)key + (key_len-4), 4);
        hash_value = hash_value^(*((int*)tmp_value));
        //
        // If the key length is >= 10, the hash is also XOR with the middle 4 bytes
        //
        if(key_len >= 10) {
            memcpy(tmp_value, (char*)key+(key_len/2), 4);
            hash_value = hash_value^(*((int*)tmp_value));
        }
    }

    return hash_value;
}

static inline _info_node_t *_node_get(char *schedule_id)
{
    int hash_id = 0;
    _info_node_t *p_node = NULL;

    if (schedule_id) {
        LogCpvrDebug( "schedule id is %s\n", schedule_id );
        hash_id = hash_value_get(schedule_id, strlen(schedule_id));
        TAILQ_FOREACH(p_node, &info_hdr, list) {
            if (is_deleted_sub_task(&p_node->info))
                continue;

            if (p_node->live.hash_id == hash_id && strncmp(schedule_id, p_node->info.schedule_id, strlen(schedule_id)) == 0)
                break;
        }
    }

    return p_node;
}

static inline _active_info_node_t *_active_node_get(char *schedule_id)
{
    int hash_id = 0;
    _active_info_node_t *p_node = NULL;

    if (schedule_id) {
        LogCpvrDebug( "schedule id is %s\n", schedule_id );
        hash_id = hash_value_get(schedule_id, strlen(schedule_id));
        TAILQ_FOREACH(p_node, &active_info_hdr, list) {
            if (p_node->live.hash_id == hash_id && strncmp(schedule_id, p_node->info.schedule_id, strlen(schedule_id)) == 0)
                break;
        }
    }

    return p_node;
}

int task_is_running(cpvr_task_info_t *p_info)
{
    if (p_info && (p_info->task_state == IN_PROGRESS_INCOMPLETE_STATE || p_info->task_state == IN_PROGRESS_STATE))
        return 1;
    else
        return 0;
}

int taskHasStarted(cpvr_task_info_t *p_info)
{
    int i_ret = 0;

    if (p_info && (p_info->start_time <= getLocalTime() && p_info->end_time > getLocalTime()))
        i_ret = 1;

    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

int taskHasEnded(cpvr_task_info_t *p_info)
{
    int i_ret = 0;

    if (p_info && (p_info->end_time < getLocalTime()))
        i_ret = 1;

    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

int is_deleted_sub_task(cpvr_task_info_t *p_info)
{
    int i_ret = 0;

    if (p_info && (p_info->task_type == REC_TYPE_SERIAL_SUB || p_info->task_type == REC_TYPE_PERIOD_SUB)
        && p_info->delete_flg == 1)
    i_ret = 1;

    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

int cpvr_list_task_info_print(cpvr_task_info_t *p_info)
{
    if (p_info) {
        int i = 0;

        LogCpvrDebug("schedule id is %s\n", p_info->schedule_id);
        LogCpvrDebug("task type is %d\n", p_info->task_type);
        LogCpvrDebug("task state is %d\n", p_info->task_state);
        LogCpvrDebug("task start time is %d\n", p_info->start_time);
        LogCpvrDebug("task end time is %d\n", p_info->end_time);
        LogCpvrDebug("task file id is %d[%x]\n", p_info->file_id, p_info->file_id);
        LogCpvrDebug("content source is %d\n", p_info->content_source);
        LogCpvrDebug("sync flag is %d\n", p_info->sync_flag);
        LogCpvrDebug("channel num is %s\n", p_info->channel_num);
        LogCpvrDebug("channel name is %s\n", p_info->channel_name);

        for (i=0; i<sizeof(p_info->main_task_id)/sizeof(p_info->main_task_id[0]); i++)
            if (p_info->main_task_id[i][0] != 0)
                LogCpvrDebug("main task id is %s\n", p_info->main_task_id[i]);

    }
}

int cpvr_list_task_active(char *schedule_id, int record_index, void *result)
{
    int i_ret = -1;
    int hash_id = 0;

    _info_node_t *p_node = NULL;
    _active_info_node_t *pa_node = NULL;

    tuner_result_t  *p_tuner_result = (tuner_result_t *)result;

    cpvr_list_check();

    sem_wait(&info_sem);
    p_node = _node_get(schedule_id);
    if (p_node) {
        p_node->live.hash_id = hash_value_get(schedule_id, strlen(schedule_id));
        p_node->live.active = 1;
        p_node->live.index = record_index;

        if (p_tuner_result) {
            p_node->live.tuner_index = p_tuner_result->tuner_index;
            p_node->live.service_index = p_tuner_result->service_index;
            p_node->live.freq_10khz = p_tuner_result->freq_10khz;
        }

        pa_node = (_active_info_node_t *)MEM_ALLOC(sizeof(_active_info_node_t));
        memcpy(pa_node, p_node, sizeof(_active_info_node_t));
        i_ret = 0;
    }
    sem_post(&info_sem);

    if (i_ret == 0) {
        sem_wait(&active_info_sem);
        TAILQ_INSERT_TAIL(&active_info_hdr, pa_node, list);
        sem_post(&active_info_sem);
    }

    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

int cpvr_list_task_inactive(char *schedule_id)
{
    int i_ret = -1;

    _active_info_node_t *pa_node = NULL;

    cpvr_list_check();

    sem_wait(&active_info_sem);
    pa_node = _active_node_get(schedule_id);
    if (pa_node) {
        pa_node->live.active = 0;
        pa_node->live.index = 0;

        pa_node->live.tuner_index = 0;
        pa_node->live.service_index = 0;
        pa_node->live.freq_10khz = 0;

        TAILQ_REMOVE(&active_info_hdr, pa_node, list);
        MEM_FREE(pa_node);
        i_ret = 0;
    }

    sem_post(&active_info_sem);
    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

int cpvr_list_active_lowpri_task_get(cpvr_task_info_t *p_info, int *index)
{
    int i_ret = -1;

    _active_info_node_t *pa_node = NULL;
    _active_info_node_t *low_level_node = NULL;

    if (p_info == NULL || index == NULL)
        return i_ret;

    cpvr_list_check();

    *index = -1;
    sem_wait(&active_info_sem);
    TAILQ_FOREACH(pa_node, &active_info_hdr, list) {
        if(low_level_node == NULL) {
            *index = pa_node->live.index;
            low_level_node = pa_node;
        } else {
            if (low_level_node->info.priority > pa_node->info.priority) {
                low_level_node = pa_node;
                *index = pa_node->live.index;
            }
        }
    }
    if (pa_node){
        *p_info = pa_node->info;
    }
    sem_post(&active_info_sem);
    i_ret = 0;
    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

int cpvr_list_active_get(struct _copied_hdr_t *hdr)
{
    int i_cnt = 0;

    _active_info_node_t *p_node = NULL;
    _copied_node_t *p_copied_node = NULL;

    sem_wait(&active_info_sem);

    TAILQ_FOREACH(p_node, &active_info_hdr, list) {
        i_cnt ++;
        if (hdr) {
            p_copied_node = (_copied_node_t *)MEM_ALLOC(sizeof(_copied_node_t));
            p_copied_node->info = p_node->info;
            p_copied_node->live = p_node->live;
            TAILQ_INSERT_TAIL(hdr, p_copied_node, list);
        }
    }

    sem_post(&active_info_sem);

    LogCpvrDebug("i_cnt : %d\n", i_cnt);
    return i_cnt;
}

int cpvr_list_get_by_file_exist(int begin_index, int count, struct _copied_hdr_t *hdr)
{
    int cnt = 0;
    int index = 0;
    _info_node_t *p_node = NULL;
    _copied_node_t *p_copied_node = NULL;

    cpvr_list_check();

    sem_wait(&info_sem);
    TAILQ_FOREACH(p_node, &info_hdr, list) {
        index ++;
        if(index <= begin_index) continue;
        if(p_node->info.file_id > 0) {
            cnt ++;
            if (count > 0 && count == cnt) break;
            if (hdr) {
                p_copied_node = (_copied_node_t *)MEM_ALLOC(sizeof(_copied_node_t));
                p_copied_node->info = p_node->info;
                p_copied_node->live = p_node->live;
                TAILQ_INSERT_TAIL(hdr, p_copied_node, list);
            }
        }
    }

    sem_post(&info_sem);
    LogCpvrDebug("i_ret : %d\n", cnt);
    return cnt;
}

int cpvr_list_get(cpvr_task_filter_param_t *p_param, int begin_idx, int count, struct _copied_hdr_t *hdr)
{
    int i_cnt = 0, i_idx = 0;

    _info_node_t *p_node = NULL;
    _copied_node_t *p_copied_node = NULL;

    if (p_param == NULL) return -1;

    cpvr_list_check();

    sem_wait(&info_sem);

    LogCpvrDebug("schedule id ---------- %s\n", p_param->schedule_id);
    LogCpvrDebug("main task id --------- %s\n", p_param->main_task_id);
    LogCpvrDebug("delete flag ---------- %d\n", p_param->delete_flg);
    LogCpvrDebug("sync flag ------------ %d\n", p_param->sync_flag);
    LogCpvrDebug("start time ----------- %d\n", p_param->start_time);
    LogCpvrDebug("end time ------------- %d\n", p_param->end_time);

    TAILQ_FOREACH(p_node, &info_hdr, list) {
        int found = 0;

        cpvr_list_task_info_print(&p_node->info);

        if (p_param->delete_flg != 2 && p_node->info.delete_flg != p_param->delete_flg)
            continue;

        if (p_param->sync_flag != SYNC_FLAG_ALL && p_param->sync_flag != p_node->info.sync_flag)
            continue;

        if (p_param->main_task_id[0] != 0) {
            int i = 0;

            for (i=0; i<sizeof(p_node->info.main_task_id)/sizeof(p_node->info.main_task_id[0]); i++) {
                if (strncmp(p_param->main_task_id, p_node->info.main_task_id[i], strlen(p_param->main_task_id)) == 0) {
                    found = 1;
                    break;
                }
            }

            if (found == 0) continue;
        }

        if (p_param->schedule_id[0] != 0 && strncmp(p_param->schedule_id, p_node->info.schedule_id, strlen(p_param->schedule_id)) != 0)
            continue;

        if (p_param->channel_num[0] != 0 && strncmp(p_param->channel_num, p_node->info.channel_num, strlen(p_param->channel_num)) != 0)
            continue;

        if (p_param->prog_id[0] != 0 && strncmp(p_param->prog_id, p_node->info.prog_id, strlen(p_param->prog_id)) != 0)
            continue;

        if ((p_param->end_time > 0 && p_param->end_time < p_node->info.start_time)
            || (p_param->start_time > 0 && p_param->start_time > p_node->info.end_time))
            continue;

        if (p_param->task_state[0] != UNINITED_STATE) {
            int i = 0, i_found = 0;

            for (i=0; i<sizeof(p_param->task_state)/sizeof(p_param->task_state[0]); i++) {
                if (p_param->task_state[i] != UNINITED_STATE && p_param->task_state[i] == p_node->info.task_state) {
                    i_found = 1;
                    break;
                }
            }

            if (i_found == 0) continue;
        }

        if (p_param->task_type[0] != REC_TYPE_UKNOW) {
            int i = 0, i_found = 0;

            for (i=0; i<sizeof(p_param->task_type)/sizeof(p_param->task_type[0]); i++) {
                if (p_param->task_type[i] != REC_TYPE_UKNOW
                    && (p_param->task_type[i] == p_node->info.task_type
                        || (p_param->task_type[i] == REC_TYPE_SUB && (p_node->info.task_type == REC_TYPE_SERIAL_SUB || p_node->info.task_type == REC_TYPE_PERIOD_SUB)))) {
                    i_found = 1;
                    break;
                }
            }

            if (i_found == 0) continue;
        }

        if (p_param->content_source != UNKNOWN_PVR && p_param->content_source != p_node->info.content_source)
            continue;

        if (p_param->last_time > 0 && p_node->info.end_time > p_param->last_time)
            continue;

        i_idx ++;
        if (i_idx <= begin_idx) continue;

        i_cnt ++;
        if (hdr) {
            p_copied_node = (_copied_node_t *)MEM_ALLOC(sizeof(_copied_node_t));
            p_copied_node->info = p_node->info;
            p_copied_node->live = p_node->live;
            TAILQ_INSERT_TAIL(hdr, p_copied_node, list);
        }
        if (count > 0 && count == i_cnt) break;
    }

    sem_post(&info_sem);

    LogCpvrDebug("i_ret : %d\n", i_cnt);
    return i_cnt;
}

int cpvr_list_task_get_by_schedule_id(char *schedule_id, cpvr_task_info_t *p_info, cpvr_task_live_t *p_live)
{
    int i_ret = -1;

    _info_node_t *p_node = NULL;

    cpvr_list_check();

    sem_wait(&active_info_sem);
    p_node = (_info_node_t *)_active_node_get(schedule_id);
    if (p_node) {
        if (p_info) *p_info = p_node->info;
        if (p_live) *p_live = p_node->live;
        i_ret = 0;
    }
    sem_post(&active_info_sem);

    if (i_ret != 0) {
        sem_wait(&info_sem);
        p_node = _node_get(schedule_id);
        if (p_node) {
            if (p_info) *p_info = p_node->info;
            if (p_live) *p_live = p_node->live;
            i_ret = 0;
        }
        sem_post(&info_sem);
    }

    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

int cpvr_list_task_sync_flag_update(char *schedule_id, int sync_flag)
{
    _info_node_t *p_node;

    cpvr_list_check();

    sem_wait(&info_sem);
    p_node = _node_get(schedule_id);
    if (p_node) {
        p_node->info.sync_flag = sync_flag;
        db_cpvr_task_info_update_int(schedule_id, "sync_flag",p_node->info.sync_flag);
    }
    sem_post(&info_sem);

    return 0;
}

int cpvr_list_task_file_id_update(char *schedule_id, int file_id)
{
    _info_node_t *p_node;

    cpvr_list_check();

    sem_wait(&info_sem);
    p_node = _node_get(schedule_id);
    if (p_node ) {
        p_node->info.file_id = file_id;
        db_cpvr_task_info_update_int(schedule_id, "file_id",p_node->info.file_id);
    }
    sem_post(&info_sem);

    return 0;
}

int cpvr_list_task_bandwidth_update(char *schedule_id, int bandwidth)
{
    _info_node_t *p_node;

    cpvr_list_check();

    sem_wait(&info_sem);
    p_node = _node_get(schedule_id);
    if (p_node ) {
        p_node->info.bandwidth = bandwidth;
    }
    sem_post(&info_sem);

    return 0;
}

int cpvr_list_task_delete_flg_update(char *schedule_id, int delete_flg)
{
    _info_node_t *p_node;

    cpvr_list_check();

    sem_wait(&info_sem);
    p_node = _node_get(schedule_id);
    if (p_node ) {
        p_node->info.delete_flg = delete_flg;
        db_cpvr_task_info_update_int(schedule_id, "delete_flg",p_node->info.delete_flg);
    }
    sem_post(&info_sem);

    return 0;
}

int cpvr_list_task_parental_rating_update(char *schedule_id, char *parental_rating)
{
    _info_node_t *p_node;

    cpvr_list_check();

    sem_wait(&info_sem);
    p_node = _node_get(schedule_id);
    if (p_node ) {
        strncpy(p_node->info.parental_rating, parental_rating, sizeof(p_node->info.parental_rating)-1);
    }
    sem_post(&info_sem);

    return 0;
}

int cpvr_list_subtask_num_update(char *schedule_id, int total, int unfinished)
{
    _info_node_t *p_node;

    cpvr_list_check();

    sem_wait(&info_sem);
    p_node = _node_get(schedule_id);
    if (p_node) {
        p_node->info.subtask_num_total = total;
        db_cpvr_task_info_update_int(schedule_id, "subtask_num_total",p_node->info.subtask_num_total);

        p_node->info.subtask_num_unfinished = unfinished;
        db_cpvr_task_info_update_int(schedule_id, "subtask_num_unfinished",p_node->info.subtask_num_unfinished);

    }
    sem_post(&info_sem);

    return 0;
}

int cpvr_list_task_time_update(char *schedule_id, time_t tm_start, time_t tm_end)
{
    int i_ret = -1;

    _info_node_t *p_node = NULL;
    _active_info_node_t *pa_node = NULL;

    cpvr_list_check();

    sem_wait(&info_sem);
    p_node = _node_get(schedule_id);
    if (p_node) {
        if (tm_start > 0) {
            p_node->info.start_time = tm_start;
            db_cpvr_task_info_update_int(schedule_id, "start_time",p_node->info.start_time);
        }

        if (tm_end > 0) {
            p_node->info.end_time = tm_end;
            db_cpvr_task_info_update_int(schedule_id, "end_time",p_node->info.end_time);
        }

        i_ret = 0;
    }
    sem_post(&info_sem);

    if (i_ret == 0) {
        sem_wait(&active_info_sem);
        pa_node = _active_node_get(schedule_id);
        if (pa_node) {
            if (tm_start > 0) {
                pa_node->info.start_time = tm_start;
            }

            if (tm_end > 0) {
                pa_node->info.end_time = tm_end;
            }
        }
        sem_post(&active_info_sem);
    }

    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

int cpvr_list_task_state_update(char *schedule_id, CPVR_TASK_STATE state, CPVR_TASK_ERROR_CODE err_code)
{
    int i_ret = -1;

    _info_node_t *p_node = NULL;
    _active_info_node_t *pa_node = NULL;

    cpvr_list_check();

    sem_wait(&info_sem);
    p_node = _node_get(schedule_id);
    if (p_node) {
        p_node->info.last_task_state = p_node->info.task_state;
        p_node->info.task_state = state;
        p_node->info.error_code = err_code;
        db_cpvr_task_info_update_int(schedule_id, "task_state",p_node->info.task_state);
        db_cpvr_task_info_update_int(schedule_id, "error_code",p_node->info.error_code);
        db_cpvr_task_info_update_int(schedule_id, "last_task_state",p_node->info.last_task_state);
        i_ret = 0;
    }
    sem_post(&info_sem);

    sem_wait(&active_info_sem);
    pa_node = _active_node_get(schedule_id);
    if (pa_node) {
        pa_node->info.last_task_state = pa_node->info.task_state;
        pa_node->info.task_state = state;
        pa_node->info.error_code = err_code;
    }
    sem_post(&active_info_sem);

    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

int cpvr_list_task_get(cpvr_task_filter_param_t *p_param, cpvr_task_info_t *p_info, cpvr_task_live_t *p_live)
{
    int i_ret = -1;

    _info_node_t *p_node = NULL;

    cpvr_list_check();

    sem_wait(&info_sem);

    TAILQ_FOREACH(p_node, &info_hdr, list) {
        if (is_deleted_sub_task(&p_node->info))
            continue;

        if ((p_param->schedule_id[0] != 0 && strncmp(p_param->schedule_id, p_node->info.schedule_id, strlen(p_param->schedule_id)) == 0)
            || (p_param->prog_id[0] != 0 && strncmp(p_param->prog_id, p_node->info.prog_id, strlen(p_param->prog_id)) == 0)
            || (p_param->start_time != 0 && p_param->start_time == p_node->info.start_time && p_param->end_time != 0 && p_param->end_time == p_node->info.end_time)
            || (p_param->file_id > 0 && p_param->file_id == p_node->info.file_id)
            || (p_param->serial_prog_sn[0] != 0 && strncmp(p_param->serial_sn, p_node->info.serial_sn, strlen(p_param->serial_sn)) == 0
                && strncmp(p_param->serial_season_id, p_node->info.serial_season_id, strlen(p_param->serial_season_id)) == 0 && strncmp(p_param->serial_prog_sn, p_node->info.serial_prog_sn, strlen(p_param->serial_prog_sn)) == 0)) {
            if (p_info) *p_info = p_node->info;
            if (p_live) *p_live = p_node->live;
            i_ret = 0;
            break;
        }
    }

    sem_post(&info_sem);

    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

int cpvr_task_info_update(cpvr_task_info_t *p_info)
{
    _info_node_t *p_node = NULL;

    p_node = _node_get(p_info->schedule_id);
    if (p_info && p_node) {
        int j = 0;
        char *p = NULL, *ptr = NULL;

        ptr = (char*)MEM_ALLOC(sizeof(p_info->main_task_id) + sizeof(p_info->main_task_id)/sizeof(p_info->main_task_id[0]));
        if (ptr == NULL) return -1;

        p = ptr;
        for (j=0; j<sizeof(p_info->main_task_id)/sizeof(p_info->main_task_id[0]); j++) {
            if (p_info->main_task_id[j][0] != 0) {
                strncpy(p, p_info->main_task_id[j], strlen(p_info->main_task_id[j]));
                p += strlen(p_info->main_task_id[j]);
                *p ++ = '#';
            }
        }
        *p = 0;

        p_node->info = *p_info;
        db_cpvr_task_info_update_int(p_info->schedule_id, "task_type",p_node->info.task_type);
        db_cpvr_task_info_update_string(p_info->schedule_id, "main_task_id", ptr);

        if (ptr) MEM_FREE(ptr);
    }
}

int cpvr_list_task_add(cpvr_task_info_t *p_info)
{
    int hash_id = 0;

    _info_node_t *p_node = NULL;
    _info_node_t *p_temp = NULL;

    cpvr_list_check();

    if (p_info) {
        if (cpvr_list_task_get_by_schedule_id(p_info->schedule_id, NULL, NULL) == 0) {
            LogCpvrDebug( "The task %s already added.\n", p_info->schedule_id);
            return 1;
        }

        hash_id = hash_value_get(p_info->schedule_id, strlen(p_info->schedule_id));

        sem_wait(&info_sem);

        p_node = (_info_node_t *)MEM_ALLOC(sizeof( _info_node_t));
        memset(p_node, 0, sizeof(_info_node_t));
        p_node->live.hash_id = hash_id;
        p_node->info = *p_info;
        TAILQ_FOREACH(p_temp, &info_hdr, list) {
            if (p_node->info.start_time > p_temp->info.start_time) {
                TAILQ_INSERT_BEFORE(p_temp, p_node, list);
                LogCpvrDebug("The task %s successfully added to the queue.\n", p_info->schedule_id);
                sem_post(&info_sem);
                return 0;
            }
        }
        TAILQ_INSERT_TAIL(&info_hdr, p_node, list);
        LogCpvrDebug("The task %s successfully added to the queue.\n", p_info->schedule_id);
        sem_post(&info_sem);
    }

    return 0;
}

int cpvr_list_task_remove(char *schedule_id)
{
    int i_ret = -1, hash_id = 0;
    _info_node_t *p_node = NULL;

    cpvr_list_check();

    sem_wait(&info_sem);
    p_node = _node_get(schedule_id);
    if (p_node) {
        db_cpvr_task_info_delete(schedule_id);
        TAILQ_REMOVE(&info_hdr, p_node, list);
        MEM_FREE(p_node);

        i_ret = 0;
    }
    sem_post(&info_sem);

    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

int cpvr_copied_list_release(struct _copied_hdr_t *p_hdr)
{
    _copied_node_t *p_node;

    if (p_hdr) {
        p_node = TAILQ_FIRST(p_hdr);
        while (p_node) {
            TAILQ_REMOVE(p_hdr, p_node, list);
            MEM_FREE(p_node);
            p_node = NULL;
            p_node = TAILQ_FIRST(p_hdr);
        }
    }

    return 0;
}

int cpvr_list_release()
{
    _info_node_t *p_node = NULL;

    cpvr_list_check();

    sem_wait(&info_sem);
    p_node = TAILQ_FIRST(&info_hdr);
    while(p_node) {
        TAILQ_REMOVE(&info_hdr, p_node, list);
        MEM_FREE(p_node);
        p_node = NULL;
        p_node = TAILQ_FIRST(&info_hdr);
    }

    sem_post(&info_sem);
    return 0;
}

int cpvr_list_init()
{
    int i_ret = 0;

    if (cpvr_list_valid == 0) {
        struct pvr_partition pvr_partition;

        if (pvr_partition_info_get(&pvr_partition) != 0) {
            LogCpvrError("pvr partition not found.\n");
            i_ret = -1;
        } else {
            sem_init(&info_sem, 0, 1);
            sem_init(&tmp_info_sem, 0, 1);
            sem_init(&active_info_sem, 0, 1);
            cpvr_list_valid = 1;
            db_open(pvr_partition.mounted_dir);
            db_cpvr_task_info_read((add_task_callback_func)cpvr_list_task_add);
            i_ret = 0;
        }
    }

    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

void cpvr_list_destroy()
{
    cpvr_list_release();
    db_close();
    cpvr_list_valid = 0;
    sem_destroy(&info_sem);
    sem_destroy(&tmp_info_sem);
    sem_destroy(&active_info_sem);
}

static inline _info_node_t *_tmp_node_get(char *schedule_id)
{
    int hash_id;
    _info_node_t *p_node;

    if (!schedule_id)
        return NULL;
    LogCpvrDebug( "schedule id is %s\n", schedule_id );
    hash_id = hash_value_get(schedule_id, strlen(schedule_id));
    TAILQ_FOREACH(p_node, &temp_info_hdr, list) {
        if (p_node->live.hash_id == hash_id && strncmp(schedule_id, p_node->info.schedule_id, strlen(schedule_id)) == 0) {
            return p_node;
        }
    }
    return NULL;
}

int cpvr_tmp_list_task_get_by_index(int index, cpvr_task_info_t *p_info)
{
    int i_ret = -1;
    int i_idx = -1;
    _info_node_t *p_node = NULL;

    sem_wait(&tmp_info_sem);

    p_node = TAILQ_FIRST(&temp_info_hdr);
    while (p_node) {
        i_idx++;
        if (i_idx == index)
            break;

        p_node = TAILQ_NEXT(p_node, list);
    }

    if (p_node && i_idx == index){
        if (p_info) *p_info = p_node->info;
        i_ret = 0;
    }
    sem_post(&tmp_info_sem);
    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

int cpvr_tmp_list_task_get(char *schedule_id, cpvr_task_info_t *p_info)
{
    int i_ret = -1;
    _info_node_t *p_node = NULL;

    sem_wait(&tmp_info_sem);
    p_node = _tmp_node_get(schedule_id);
    if (p_node){
        if (p_info) *p_info = p_node->info;
        i_ret = 0;
    }
    sem_post(&tmp_info_sem);
    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

int cpvr_tmp_list_task_remove(char *schedule_id)
{
    int i_ret = -1, hash_id = 0;
    _info_node_t *p_node = NULL;

    cpvr_list_check();

    sem_wait(&tmp_info_sem);
    p_node = _tmp_node_get(schedule_id);
    if (p_node) {
        TAILQ_REMOVE(&temp_info_hdr, p_node, list);
        MEM_FREE(p_node);
        LogCpvrDebug("The task %s has removed from tmp list.\n", schedule_id);
        i_ret = 0;
    }
    sem_post(&tmp_info_sem);

    LogCpvrDebug("i_ret : %d\n", i_ret);
    return i_ret;
}

int cpvr_tmp_list_task_add(cpvr_task_info_t *p_info)
{
    int hash_id = 0;
    _info_node_t *p_node;

    sem_wait(&tmp_info_sem);

    if (p_info->content_source != DVB_PVR)
        p_info->content_source = IPTV_PVR;

    p_node = (_info_node_t *)MEM_ALLOC(sizeof( _info_node_t));
    memset(p_node, 0, sizeof(_info_node_t));

    hash_id = hash_value_get(p_info->schedule_id, strlen(p_info->schedule_id));
    p_node->live.hash_id = hash_id;
    p_node->info = *p_info;

    TAILQ_INSERT_TAIL(&temp_info_hdr, p_node, list);
    sem_post(&tmp_info_sem);
    return 0;
}

void cpvr_tmp_list_release()
{
    _info_node_t *p_node = NULL;

    sem_wait(&tmp_info_sem);
    p_node = TAILQ_FIRST(&temp_info_hdr);
    while (p_node) {
        TAILQ_REMOVE(&temp_info_hdr, p_node, list);
        MEM_FREE(p_node);
        p_node = NULL;
        p_node = TAILQ_FIRST(&temp_info_hdr);
    }
    sem_post(&tmp_info_sem);
}

}

