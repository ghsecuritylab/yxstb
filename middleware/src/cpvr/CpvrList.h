#ifndef _CPVRLIST_H
#define _CPVRLIST_H

#include <sys/queue.h>
#include "CpvrTaskManager.h"

typedef struct tuner_result {
    int tuner_index;
    int service_index;
    int freq_10khz;
}tuner_result_t;

typedef struct cpvr_task_live {
    int hash_id;
    int index;
    int res_id;
    int active;

    int tuner_index;
    int service_index;
    int freq_10khz;

    char channel_url[1024];
    void * timer_task_start_in_one_minute;
    void * timer_task_begin;
    void * timer_task_end;
}cpvr_task_live_t;

typedef struct cpvr_task_filter_param {
    char schedule_id[72];
    char main_task_id[72];
    char channel_num[24];
    char prog_id[64];
    CPVR_TASK_TYPE task_type[8];
    CPVR_CONTENT_SRC_TYPE content_source;
    time_t start_time;
    time_t end_time;
    time_t last_time;
    CPVR_TASK_STATE task_state[16];
    int sync_flag;
    int file_id;
    int file_exist;

    char serial_sn[32];
    char serial_season_id[32];
    char serial_prog_sn[32];

    int include_subtask;
    int delete_flg;
}cpvr_task_filter_param_t;

typedef struct _copied_node {
    cpvr_task_info_t info;
    cpvr_task_live_t live;
    TAILQ_ENTRY(_copied_node)list;
}_copied_node_t;
TAILQ_HEAD(_copied_hdr_t, _copied_node);

namespace Hippo {

int task_is_running(cpvr_task_info_t *p_info);
int taskHasStarted(cpvr_task_info_t *p_info);
int taskHasEnded(cpvr_task_info_t *p_info);
int is_deleted_sub_task(cpvr_task_info_t *p_info);

int cpvr_list_active_get(struct _copied_hdr_t *hdr);
int cpvr_list_task_file_id_update(char *schedule_id, int file_id);
int cpvr_list_task_bandwidth_update(char *schedule_id, int bandwidth);
int cpvr_list_task_sync_flag_update(char *schedule_id, int sync_flag);
int cpvr_list_task_delete_flg_update(char *schedule_id, int delete_flg);
int cpvr_list_task_parental_rating_update(char *schedule_id, char *parental_rating);
int cpvr_list_subtask_num_update(char *schedule_id, int total, int unfinished);
int cpvr_list_task_time_update(char *schedule_id, time_t tm_start, time_t tm_end);
int cpvr_list_task_state_update(char *schedule_id, CPVR_TASK_STATE state, CPVR_TASK_ERROR_CODE err_code);
int cpvr_list_task_get(cpvr_task_filter_param_t *p_param, cpvr_task_info_t *p_info, cpvr_task_live_t *p_live);
int cpvr_task_info_update(cpvr_task_info_t *p_info);
int cpvr_list_task_add(cpvr_task_info_t *p_info);
int cpvr_list_task_remove(char *schedule_id);
int cpvr_copied_list_release(struct _copied_hdr_t *p_hdr);
int cpvr_list_release();
int cpvr_list_init();
void cpvr_list_destroy();
int cpvr_tmp_list_task_remove(char *schedule_id);
int cpvr_tmp_list_task_get(char *schedule_id, cpvr_task_info_t *p_info);
int cpvr_tmp_list_task_add(cpvr_task_info_t *p_info);
int cpvr_tmp_list_task_get_by_index(int index, cpvr_task_info_t *p_info);
void cpvr_tmp_list_release();

int cpvr_list_task_get_by_schedule_id(char *schedule_id, cpvr_task_info_t *p_info, cpvr_task_live_t *p_live);
int cpvr_list_get(cpvr_task_filter_param_t *p_param, int begin_idx, int count, struct _copied_hdr_t *hdr);
int cpvr_list_get_by_file_exist(int begin_index, int count, struct _copied_hdr_t *hdr);

int cpvr_list_task_inactive(char *schedule_id);
int cpvr_list_task_active(char *schedule_id, int record_index, void *result);
int cpvr_list_task_info_print(cpvr_task_info_t *p_info);

}
#endif

