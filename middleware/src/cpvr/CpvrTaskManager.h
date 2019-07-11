#ifndef _CPVRTASKMANAGER_H
#define _CPVRTASKMANAGER_H

/* cpvr_task.h */
/*{{{*/
typedef enum {
    REC_TYPE_UKNOW = 0,
    REC_TYPE_NORMAL,
    REC_TYPE_SERIAL_MAIN,
    REC_TYPE_PERIOD_MAIN,
    REC_TYPE_SUB,
    REC_TYPE_SERIAL_SUB,
    REC_TYPE_PERIOD_SUB
}CPVR_TASK_TYPE;

typedef enum {
    UNKNOWN_PVR = 0,
    IPTV_PVR,
    DVB_PVR
}CPVR_CONTENT_SRC_TYPE;

typedef enum {
    RECORD_IF_NO_CONFLICTS = 0,
    RECORD_WITH_CONFLICT,
    RECORD_ANYWAY
}CPVR_TASK_PRIORITY;

typedef enum {
    UNINITED_STATE = 0,
    COMPLETED_STATE = 1,
    FAILED_STATE,
    INCOMPLETE_STATE,
    PENDING_WITH_CONFLICT_STATE,
    PENDING_NO_CONFLICT_STATE,
    IN_PROGRESS_INCOMPLETE_STATE,
    IN_PROGRESS_STATE,
    IN_PROGRESS_WITH_ERROR_STATE,
    CANCELLED_STATE,
    WILL_START_IN_ONE_MIUNTES
}CPVR_TASK_STATE;

typedef enum {
    UKNOW = -1,
    TASK_OK = 0,
    RESOURCES_REMOVED,
    ACCESS_FORBIDDEN_REBUILDING,
    SPACE_FULL,
    ACCESS_FORBIDDEN,
    FILESYSTEM_UNAVAILABLE,
    OUT_OF_BANDWIDTH = 11,
    CA_REFUSAL = 21,
    POWER_INTERRUPTION = 31,
    SERVICE_VANISHED = 41,
    SERVICE_UNAVAILABLE,
    OVERSTEP_ALLOWABLE_TASK = 51,
    DVB_SOURCE_SWITCHED = 61
}CPVR_TASK_ERROR_CODE;

typedef enum {
    MODE_SOURCE = 1,
    MODE_SAFE
}PROTECTION_MODE;

enum {
    SYNC_FLAG_NONEED = 0,
    SYNC_FLAG_CHANGE,
    SYNC_FLAG_DELETED,
    SYNC_FLAG_NEED,
    SYNC_FLAG_ALL
};

typedef struct serial_match_rule
{
    char key[64];
    char value[512];
    char rule[32];
}serial_match_rule_t;

typedef struct cpvr_task_info_struct 
{
    char schedule_id[72];
    char channel_num[24];
    char channel_name[512];
    CPVR_TASK_TYPE task_type;

    time_t main_task_start_time;
    time_t main_task_end_time;

    char serial_sn[32];
    char serial_season_id[32];
    char serial_prog_sn[32];
    char serial_name[512];
    int serial_episode_type;
    char serial_record_rule[512];

    char period_type[32];
    char period_every_week[32];
    char period_every_month[32];
    char period_name[512];

    char prog_id[64];
    char prog_title[512];
    char program_description[2048];

    time_t start_time;
    time_t end_time;
    time_t real_start_time;
    time_t real_end_time;

    int file_id;
    int bitrate;
    CPVR_CONTENT_SRC_TYPE content_source;

    CPVR_TASK_STATE task_state;
    CPVR_TASK_STATE last_task_state;
    CPVR_TASK_ERROR_CODE error_code;

    time_t time_stamp;
    char package_id[24];
    char package_name[512];
    PROTECTION_MODE protection_mode;
    CPVR_TASK_PRIORITY priority;
    int sync_flag;
    char parental_rating[24];

    int delete_flg;
    char main_task_id[12][72];
    int pltv_tag;
    int auto_delete;
    int reserved_duration;
    int bandwidth;

    int subtask_num_total;
    int subtask_num_unfinished;
}cpvr_task_info_t;
/*}}}*/

/* cpvr_mng.h */
enum {/*{{{*/
    MSG_UNKNOWN = 0,
    MSG_TASK_START,
    MSG_TASK_STOP,
    MSG_TASK_CLOSE,
    MSG_TASK_END,
    MSG_TASK_DELETE,
    MSG_TASK_RELEASE_RES,
    MSG_SEND_START_IN_ONE_MINUTE_STATE,
    MSG_TASK_START_WITHOUT_REQ_RES
};

typedef struct cpvr_ability {
    int allow_pvr;
    int allow_concurrent;
    unsigned int disk_quta;
} cpvr_ability_t;

typedef struct need_handle_task {
    int what;
    char schedule_id[72];
    time_t time_point;
} need_handle_task_t;

typedef struct cpvr_channel_info {
    int ch_num;
    char ch_url[1024];
    int ch_bandwidth;
    int ch_parental_rating;
    int freq;
    int sqacode;
    int retcode;
} cpvr_channel_info_t;
/*}}}*/

namespace Hippo {

int cpvr_open();
void cpvr_close();

void cpvr_task_create(char *schedule_id);
int cpvr_task_start(char *schedule_id, cpvr_channel_info_t *p_ch_info);
int cpvr_task_end(char *schedule_id);
int cpvr_task_stop(char *schedule_id, CPVR_TASK_ERROR_CODE err_code);
int cpvr_task_start_in_one_minute_state_set(char *schedule_id);
void cpvr_task_state_set(char *schedule_id, CPVR_TASK_STATE state, CPVR_TASK_ERROR_CODE err_code);
CPVR_TASK_STATE cpvr_task_state_str2enum(char *state_str);
int cpvr_task_sync_flag_set(cpvr_task_info_t *p_info, int sync_flg);

int cpvr_task_schedule_id_create(cpvr_task_info_t *p_info);
int cpvr_task_stop_all (CPVR_TASK_TYPE type);
int cpvr_task_resume_all (CPVR_TASK_TYPE type);
void* cpvr_task_delete_all_files(void* param);
int cpvr_task_delete_all_files_progress_get();
int cpvr_task_time_adjust(char *schedule_id, time_t tm_start, time_t tm_end);
int cpvr_task_sync_flag_clear(char *need_clear_id_str, int need_clear_flg);
int cpvr_task_delete(cpvr_task_info_t *p_info, char *subtask_state, int delete_file);
void cpvr_task_serial_record_update(char *schedule_id, int resume);

void need_handle_queue_node_add(int what, char *schedule_id, time_t tp);
void need_handle_queue_node_remove(int what, char *schedule_id);
void need_handle_queue_node_remove_all(char *schedule_id);
int need_handle_queue_node_get(int index, need_handle_task_t *p_task);

void cpvrTaskUpdateFromMEM(void);
void cpvr_ability_get(cpvr_ability_t *p_ability);
void cpvr_ability_set(cpvr_ability_t *p_ability);
void cpvr_task_state_parse(int *state_array, int array_size, char *state_str, char option);
void cpvr_task_period_record_update(char *schedule_id, int resume);

int getDeletePvrFileProgressByFileid(unsigned int file_id);
}
#endif

