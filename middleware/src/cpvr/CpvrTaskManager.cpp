#include "CpvrAssertions.h"
#include "CpvrAuxTools.h"
#include "CpvrTaskManager.h"
#include "CpvrList.h"
#include "CpvrDB.h"

#include "BrowserBridge/Huawei/BrowserEventQueue.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/queue.h>
#include <semaphore.h>
#include <pthread.h>
#include <string>

#include "Business.h"
#include "SysSetting.h"

#include "json/json_public.h"
#include "app_sys.h"
#include "mid_stream.h"
#include "mid_record.h"
#include "record_port.h"
#include "disk_info.h"

const char *record_language[] = {
    "unknown",
    "LANGUAGE_LOCAL",
    "LANGUAGE_ENGLISH",
    NULL
};

const char *cpvr_task_state_str[] = {
    "UNINITED_STATE",
    "COMPLETED_STATE",
    "FAILED_STATE",
    "INCOMPLETE_STATE",
    "PENDING_WITH_CONFLICT_STATE",
    "PENDING_NO_CONFLICT_STATE",
    "IN_PROGRESS_INCOMPLETE_STATE",
    "IN_PROGRESS_STATE",
    "IN_PROGRESS_WITH_ERROR_STATE",
    "CANCELLED_STATE",
    "WILL_START_IN_ONE_MIUNTES",
    NULL
};

#ifndef FCC         // FCC
#define FCC_CODE 0x08        //4
#endif

static cpvr_ability_t g_cpvr_ability;

static pthread_mutex_t need_handle_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

static int cpvr_first_time = 1;

typedef struct serial_rec_program_info {
    time_t st;                                  /* the event's start time */
    time_t et;                                  /* the event's end time */
    char title[128];                            /* the event's title name */
    char descr[256];                            /* the event's subtitle description */
}serial_rec_program_info_t;

typedef struct need_handle_task_node {
    need_handle_task_t task_info;
    TAILQ_ENTRY(need_handle_task_node)list;
} need_handle_task_node_t;

TAILQ_HEAD(need_handle_task_hdr_t, need_handle_task_node);
static struct need_handle_task_hdr_t need_handle_task_hdr = TAILQ_HEAD_INITIALIZER(need_handle_task_hdr);

namespace Hippo {

static void sendEventPvrSubtasklistChangeToEpg()
{
    //add subtask list event
    std::string subtaskListEvent = "{\"type\":\"EVENT_PVR_SUBTASKLIST_CHANGE\"}";
    browserEventSend(subtaskListEvent.c_str(), NULL);
}

void need_handle_queue_node_add(int what, char *schedule_id, time_t tp)
{
    need_handle_task_node_t *p_node = NULL;

    pthread_mutex_lock(&need_handle_queue_mutex);
    // MEM_NEW(p_node);
    p_node = (need_handle_task_node_t*)MEM_CALLOC(1, (long)sizeof(need_handle_task_node_t));
    if (p_node) {
        need_handle_task_node_t *p_tmp_node = NULL;

        TAILQ_FOREACH(p_tmp_node, &need_handle_task_hdr, list) {
            if (what == p_tmp_node->task_info.what && strncmp(p_tmp_node->task_info.schedule_id, schedule_id, strlen(schedule_id)) == 0) {
                if (p_tmp_node->task_info.time_point != tp) {
                    TAILQ_REMOVE(&need_handle_task_hdr, p_tmp_node, list);
                    MEM_FREE(p_tmp_node);
                    p_tmp_node = NULL;
                    break;
                } else
                    goto EXIT_WAY ;
            }
        }

        memset(p_node, 0, sizeof(p_node));
        p_node->task_info.what = what;
        strcpy(p_node->task_info.schedule_id, schedule_id);
        p_node->task_info.time_point = tp;

        LogCpvrDebug("add msg, what is %d, schedule id is %s, time point is %d.\n", what, schedule_id, tp);
        TAILQ_FOREACH(p_tmp_node, &need_handle_task_hdr, list) {
            if (p_node->task_info.time_point < p_tmp_node->task_info.time_point) {
                TAILQ_INSERT_BEFORE(p_tmp_node, p_node, list);
                goto EXIT_WAY ;
            }
        }
        TAILQ_INSERT_TAIL(&need_handle_task_hdr, p_node, list);
    }

EXIT_WAY:
    pthread_mutex_unlock(&need_handle_queue_mutex);
}

void need_handle_queue_node_remove(int what, char *schedule_id)
{
    need_handle_task_node_t *p_tmp_node = NULL;

    pthread_mutex_lock(&need_handle_queue_mutex);
    TAILQ_FOREACH(p_tmp_node, &need_handle_task_hdr, list) {
        if (what == p_tmp_node->task_info.what && strncmp(p_tmp_node->task_info.schedule_id, schedule_id, strlen(schedule_id)) == 0) {
            LogCpvrDebug("remove msg, what is %d, schedule id is %s.\n", what, schedule_id);
            TAILQ_REMOVE(&need_handle_task_hdr, p_tmp_node, list);
            MEM_FREE(p_tmp_node);
            p_tmp_node = NULL;
            goto EXIT_WAY ;
        }
    }

EXIT_WAY:
    pthread_mutex_unlock(&need_handle_queue_mutex);
}

void need_handle_queue_node_remove_all(char *schedule_id)
{
    need_handle_task_node_t *p_node = NULL;
    need_handle_task_node_t *p_tmp_node = NULL;

    pthread_mutex_lock(&need_handle_queue_mutex);

    p_node = TAILQ_FIRST(&need_handle_task_hdr);
    while(p_node != NULL) {
        if (strncmp(p_node->task_info.schedule_id, schedule_id, strlen(schedule_id)) == 0) {
            LogCpvrDebug("remove msg, what is %d, schedule id is %s.\n", p_node->task_info.what, schedule_id);
            p_tmp_node = p_node;
            p_node = TAILQ_NEXT(p_node, list);
            TAILQ_REMOVE(&need_handle_task_hdr, p_tmp_node, list);
            MEM_FREE(p_tmp_node);
            p_tmp_node = NULL;
        }
        else p_node = TAILQ_NEXT(p_node, list);
    }

    pthread_mutex_unlock(&need_handle_queue_mutex);
}

int need_handle_queue_node_get(int index, need_handle_task_t *p_task)
{
    int i = 0, i_ret = -1;
    need_handle_task_node_t *p_node = NULL;

    pthread_mutex_lock(&need_handle_queue_mutex);

    TAILQ_FOREACH(p_node, &need_handle_task_hdr, list) {
        if (i == index) {
            if (p_task && p_node) {
                //如果达到最大并发数，暂停
                if (p_node->task_info.what == MSG_TASK_START && g_cpvr_ability.allow_concurrent > 0
                    && cpvr_list_active_get(NULL) >= g_cpvr_ability.allow_concurrent) {
                    if (getLocalTime() >= p_node->task_info.time_point)
                        cpvr_task_state_set(p_node->task_info.schedule_id, IN_PROGRESS_WITH_ERROR_STATE, OVERSTEP_ALLOWABLE_TASK);
                    continue;
                } else {
                    *p_task = p_node->task_info;
                    i_ret = 0;
                }
            }
            goto EXIT_WAY;
        }
        i++;
    }

EXIT_WAY:
    pthread_mutex_unlock(&need_handle_queue_mutex);
    return i_ret;
}

static int recordFileExists(int fileID)
{
    int i_ret = 0;
    if (fileID > 0 && ind_pvr_exist(fileID))
        i_ret = 1;

    LogCpvrDebug("i_ret %d\n", i_ret);
    return i_ret;
}

static int isDVBMode()
{
    int i_ret = 0;
    char *service_mode = (char *)business().getServiceMode();

    if (strncmp(service_mode, "DVB", 3) == 0)
        i_ret = 1;

    LogCpvrDebug("i_ret %d\n", i_ret);
    return i_ret;
}

static int isDVBChannelManageMode()
{
    int flag = 0;
#ifdef INCLUDE_DVBS
    sysSettingGetInt("DVBChannelManageFlag", &flag, 0);
#endif
    return flag;
}

CPVR_TASK_STATE cpvr_task_state_str2enum(char *state_str)
{
    int i = 0;

    if (state_str != NULL) {
        while(cpvr_task_state_str[i]) {
            if(!strncmp(state_str, cpvr_task_state_str[i], strlen(state_str)))
                return (CPVR_TASK_STATE)i;
            i++;
        }
    }

    return UNINITED_STATE;
}

void cpvr_task_state_parse(int *state_array, int array_size, char *state_str, char option)
{
    int i = 0;
    char *ps_buf = NULL;
    char *ps_parse = strdup(state_str);

    if (ps_parse) {
        char *p_start, *p_end, *p_tmp;

        p_tmp = (char*)MEM_ALLOC(strlen(ps_parse));
        ps_buf = ps_parse;
        p_start = strchr(ps_parse, option);
        while(p_tmp && i < array_size && p_start != NULL) {
            p_start ++;
            ps_parse = p_start;
            p_end = strchr(ps_parse, option);
            if (p_end == NULL)
                break;

            strncpy(p_tmp, p_start, p_end-p_start);
            p_tmp[p_end-p_start] = '\0';
            *(state_array+i) = cpvr_task_state_str2enum(p_tmp);

            i++;
            p_end++;
            ps_parse = p_end;
            p_start = strchr(ps_parse, option);
        }

        if (p_tmp) MEM_FREE(p_tmp);
    }

    if (ps_buf) MEM_FREE(ps_buf);
}

static void cpvr_task_state_change_event_send(cpvr_task_info_t *p_task_info)
{
    char tmp_buf[64] = {0};
    char send_buf[4096] = {0};

    if (p_task_info) {
        if (p_task_info->task_state == p_task_info->last_task_state) {
            LogCpvrDebug("The state has not changed. curr state is %d, prev state is %d\n", p_task_info->task_state, p_task_info->last_task_state);
            return ;
        }

        send_buf[0] = '{';
        json_add_str(send_buf, (char*)"type", (char*)"EVENT_PVR_TASK-STATUS_CHANGED");
        json_add_str(send_buf, (char*)"schedule_id",  p_task_info->schedule_id);
        json_add_str(send_buf, (char*)"Channel_no", p_task_info->channel_num);
        json_add_str(send_buf, (char*)"task_status", cpvr_task_state_str[p_task_info->task_state]);
        json_add_str(send_buf, (char*)"last_task_status", cpvr_task_state_str[p_task_info->last_task_state]);
        switch(p_task_info->error_code) {
        case UKNOW: json_add_str(send_buf, (char*)"exception_description", "UKNOW");break;
        case RESOURCES_REMOVED: json_add_str(send_buf, "exception_description", "RESOURCES_REMOVED");break;
        case ACCESS_FORBIDDEN_REBUILDING:json_add_str(send_buf, "exception_description", "ACCESS_FORBIDDEN_REBUILDING");break;
        case SPACE_FULL:json_add_str(send_buf, "exception_description", "SPACE_FULL");break;
        case ACCESS_FORBIDDEN:json_add_str(send_buf, "exception_description", "ACCESS_FORBIDDEN");break;
        case FILESYSTEM_UNAVAILABLE:json_add_str(send_buf, "exception_description", "FILESYSTEM_UNAVAILABLE");break;
        case OUT_OF_BANDWIDTH:json_add_str(send_buf, "exception_description", "OUT_OF_BANDWIDTH");break;
        case CA_REFUSAL:json_add_str(send_buf, "exception_description", "CA_REFUSAL");break;
        case POWER_INTERRUPTION:json_add_str(send_buf, "exception_description", "POWER_INTERRUPTION");break;
        case SERVICE_VANISHED:json_add_str(send_buf, "exception_description", "SERVICE_VANISHED");break;
        case SERVICE_UNAVAILABLE:json_add_str(send_buf, "exception_description", "SERVICE_UNAVAILABLE");break;
        case OVERSTEP_ALLOWABLE_TASK:json_add_str(send_buf, "exception_description", "OVERSTEP_ALLOWABLE_TASK");break;
        case DVB_SOURCE_SWITCHED:json_add_str(send_buf, "exception_description", "DVB_SOURCE_SWITCHED");break;
        default: break;
        }
        json_add_str(send_buf, "program_id", p_task_info->prog_id);
        timeSecNnumToString(p_task_info->start_time, tmp_buf, 0);
        json_add_str(send_buf, "record_starttime", tmp_buf);
        timeSecNnumToString(p_task_info->end_time, tmp_buf, 0);
        json_add_str(send_buf, "record_endtime", tmp_buf);
        json_add_str(send_buf, "prog_title", p_task_info->prog_title);
        sprintf(tmp_buf, "%d", p_task_info->bitrate);
        json_add_str(send_buf, "schedule_bitrate", tmp_buf);
        if (p_task_info->content_source == DVB_PVR)
            json_add_str(send_buf, "contentPath", "DVB");
        json_add_rightbrace(send_buf);
        browserEventSend(send_buf, NULL);
        LogCpvrDebug( "change event:%s\n", send_buf);
    }
}


void cpvr_task_state_set(char *schedule_id, CPVR_TASK_STATE state, CPVR_TASK_ERROR_CODE err_code)
{
    cpvr_task_info_t    task_info;

    if (schedule_id)
        LogCpvrDebug("schedule id is %s, state is %d, error code is %d\n",schedule_id, state, err_code);

    if (schedule_id && cpvr_list_task_get_by_schedule_id(schedule_id, &task_info, NULL) == 0) {
        if (state != task_info.task_state || err_code != task_info.error_code) {
            task_info.last_task_state = task_info.task_state;
            task_info.task_state = state;
            task_info.error_code = err_code;
            cpvr_list_task_state_update(task_info.schedule_id, state, err_code);
            cpvr_task_state_change_event_send(&task_info);
            cpvr_task_sync_flag_set(&task_info, SYNC_FLAG_CHANGE);
        }
    }
}

int cpvr_task_schedule_id_create(cpvr_task_info_t *p_info)
{
    static int id_suffix = 0;

    time_t l_time;
    struct tm *p;

    time(&l_time);
    p = localtime(&l_time);

    sprintf(p_info->schedule_id, "STB%04d%02d%02d%02d%02d%02d%04d%d", (1900 + p->tm_year),
        (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, id_suffix++, p_info->task_type);

    LogCpvrDebug("create schedule id is %s\n", p_info->schedule_id);
}

int cpvr_task_start_in_one_minute_state_set(char *schedule_id)
{
    cpvr_task_state_set(schedule_id, WILL_START_IN_ONE_MIUNTES, TASK_OK);
}

int cpvr_task_stop(char *schedule_id, CPVR_TASK_ERROR_CODE err_code)
{
    cpvr_task_info_t task_info;
    cpvr_task_live_t task_live;

    if (schedule_id)
        LogCpvrDebug("schedule id is %s\n",schedule_id);

    if (schedule_id && cpvr_list_task_get_by_schedule_id(schedule_id, &task_info, &task_live) == 0) {
        if (task_live.active == 1) {
            mid_record_close(task_live.index, task_info.file_id);
            cpvr_list_task_inactive(task_info.schedule_id);

            if ( task_info.task_state == IN_PROGRESS_STATE
                && task_info.task_state == IN_PROGRESS_INCOMPLETE_STATE ) {
                cpvr_task_state_set(task_info.schedule_id, IN_PROGRESS_WITH_ERROR_STATE, err_code);
            }
        }

        return 0;
    }
    else return -1;
}

static int cpvr_task_resume()
{
    int i_cnt = 0;
    cpvr_task_filter_param_t  param;
    struct _copied_hdr_t hdr = TAILQ_HEAD_INITIALIZER(hdr);
    _copied_node_t *_copied_node = NULL, *tmp_node = NULL;

    time_t tm_now = getLocalTime();
    memset(&param, 0, sizeof(param));
    param.task_state[0] = IN_PROGRESS_WITH_ERROR_STATE;
    param.sync_flag = SYNC_FLAG_ALL;
    i_cnt = cpvr_list_get(&param, 0, 0, &hdr);
    if (i_cnt > 0) {
        LogCpvrDebug("found %d tasks\n", i_cnt);
        TAILQ_FOREACH(_copied_node, &hdr, list) {
            if(_copied_node->info.start_time < tm_now && _copied_node->info.end_time > tm_now) {
                if (tmp_node == NULL || tmp_node->info.start_time > _copied_node->info.start_time)
                    tmp_node = _copied_node;
            }
        }
        if (tmp_node)
            cpvr_task_create(tmp_node->info.schedule_id);

        cpvr_copied_list_release(&hdr);
    }

    return 0;
}

int cpvr_task_stop_all (CPVR_TASK_TYPE type)
{
    int i_cnt = 0;
    cpvr_task_filter_param_t  param;

    struct _copied_hdr_t hdr = TAILQ_HEAD_INITIALIZER(hdr);

    memset(&param, 0, sizeof(param));

    param.task_state[0] = UNINITED_STATE;
    param.task_type[0] = type;
    param.sync_flag = SYNC_FLAG_ALL;
    i_cnt = cpvr_list_get(&param, 0, 0, &hdr);
    if (i_cnt > 0) {
        _copied_node_t *_copied_node = NULL;

        TAILQ_FOREACH(_copied_node, &hdr, list) {
            /*  If the task is over, ignore it. */
            if ( _copied_node->info.task_state == COMPLETED_STATE
                || _copied_node->info.task_state == FAILED_STATE
                || _copied_node->info.task_state == INCOMPLETE_STATE )
                continue;

            need_handle_queue_node_remove_all(_copied_node->info.schedule_id);
            need_handle_queue_node_add(MSG_TASK_CLOSE,_copied_node->info.schedule_id, getLocalTime());
        }
        cpvr_copied_list_release(&hdr);
    }

    return 0;
}

int cpvr_task_resume_all(CPVR_TASK_TYPE type)
{
    int i_cnt = 0;
    cpvr_task_filter_param_t  param;

    struct _copied_hdr_t hdr = TAILQ_HEAD_INITIALIZER(hdr);

    memset(&param, 0, sizeof(param));

    param.task_state[0] = UNINITED_STATE;
    param.task_type[0] = type;
    param.sync_flag = SYNC_FLAG_ALL;
    i_cnt = cpvr_list_get(&param, 0, 0, &hdr);
    LogCpvrDebug("Get %d Tasks, type is %d.\n", i_cnt, type);
    if (i_cnt > 0) {
        _copied_node_t *_copied_node = NULL;

        TAILQ_FOREACH(_copied_node, &hdr, list) {
            /*  If the task is over, ignore it. */
            if ( _copied_node->info.task_state == COMPLETED_STATE
                || _copied_node->info.task_state == FAILED_STATE
                || _copied_node->info.task_state == INCOMPLETE_STATE )
                continue;

            if (_copied_node->info.end_time <= getLocalTime()) {
                if ((_copied_node->info.file_id > 0 && ind_pvr_exist(_copied_node->info.file_id))) {
                    cpvr_task_state_set(_copied_node->info.schedule_id, INCOMPLETE_STATE, TASK_OK);
                } else {
                    cpvr_task_state_set(_copied_node->info.schedule_id, FAILED_STATE, UKNOW);
                }
                continue;
            }

            if ( _copied_node->info.task_state == IN_PROGRESS_STATE
                 || _copied_node->info.task_state == IN_PROGRESS_INCOMPLETE_STATE
                 || _copied_node->info.task_state == IN_PROGRESS_WITH_ERROR_STATE ) {
                if (cpvr_first_time == 1)
                    cpvr_task_state_set(_copied_node->info.schedule_id, IN_PROGRESS_WITH_ERROR_STATE, POWER_INTERRUPTION);

                cpvr_task_create(_copied_node->info.schedule_id);
            }

        }

        cpvr_copied_list_release(&hdr);
    }

    cpvr_first_time = 0;
    return 0;
}

int cpvr_task_end(char *schedule_id)
{
    cpvr_task_info_t task_info;
    cpvr_task_live_t task_live;

    if (schedule_id)
        LogCpvrDebug("schedule id is %s\n",schedule_id);

    if (schedule_id && cpvr_list_task_get_by_schedule_id(schedule_id, &task_info, &task_live) == 0) {
        switch (task_info.task_state) {
        case COMPLETED_STATE:
        case FAILED_STATE:
        case INCOMPLETE_STATE:
            break;
        case IN_PROGRESS_STATE:
            mid_record_close(task_live.index, task_info.file_id);
            cpvr_list_task_inactive(task_info.schedule_id);
            cpvr_task_state_set(task_info.schedule_id, COMPLETED_STATE, TASK_OK);
            break;
        default:
            if (task_live.active == 1)
            mid_record_close(task_live.index, task_info.file_id);
            cpvr_list_task_inactive(task_info.schedule_id);
            if (task_info.file_id > 0 && ind_pvr_exist(task_info.file_id)) {
                cpvr_task_state_set(schedule_id, INCOMPLETE_STATE, TASK_OK);
            } else {
                cpvr_task_state_set(schedule_id, FAILED_STATE, UKNOW);
            }
            break;
        }
        return 0;
    }
    else return -1;

}

int cpvr_task_start(char *schedule_id, cpvr_channel_info_t *p_ch_info)
{
    int i_ret = -1;
    int file_id = 0;
    int record_index = -1;

    cpvr_task_info_t task_info;
    cpvr_task_live_t task_live;

    CPVR_TASK_STATE task_state = IN_PROGRESS_STATE;
    CPVR_TASK_ERROR_CODE err_code = TASK_OK;

    tuner_result_t tuner_result;

    char record_type[128] = {0};

    if (schedule_id)
        LogCpvrDebug("schedule id is %s\n",schedule_id);

    if (schedule_id && cpvr_list_task_get_by_schedule_id(schedule_id, &task_info, &task_live) == 0) {
        int i = 0;
        int iCount = 0;
        struct _copied_hdr_t hdr = TAILQ_HEAD_INITIALIZER(hdr);

        if (task_info.task_state == IN_PROGRESS_STATE || task_info.task_state == IN_PROGRESS_INCOMPLETE_STATE)
            return 0;

        iCount = cpvr_list_active_get(&hdr);
        if (iCount > 0) {
            int iFound = 0;
            for (i=0; i<2; i++) {
                _copied_node_t *p_node = NULL;
                TAILQ_FOREACH(p_node, &hdr, list) {
                    if (p_node->live.index == i) {
                        iFound = 1;
                        break;
                    }
                }
                if (iFound == 1) {
                    iFound = 0;
                    continue;
                }
                else {
                    record_index = i;
                    break;
                }
            }
        } else
            record_index = 0;
        if (record_index < 0) {
            LogCpvrError("Get record index failed.\n");
            cpvr_task_state_set(task_info.schedule_id, IN_PROGRESS_WITH_ERROR_STATE, OVERSTEP_ALLOWABLE_TASK);
            //need_handle_queue_node_remove_all(task_info.schedule_id);
            need_handle_queue_node_add(MSG_TASK_RELEASE_RES, task_info.schedule_id, getLocalTime());
            need_handle_queue_node_add(MSG_TASK_START, task_info.schedule_id, getLocalTime()+5);
            return -1;
        }

        LogCpvrDebug("content source is %d, record index is %d, url is %s.\n", task_info.content_source, record_index, p_ch_info->ch_url);
        if (task_info.content_source == IPTV_PVR) {
            //mid_record_set_fcc(record_index, p_ch_info->sqacode & (0xFF - FCC_CODE), p_ch_info->retcode);//do not play channel that has pvr if not set fcc
			mid_record_set_fcc(record_index, p_ch_info->sqacode & (0xFF - FCC_CODE));

			strcpy(record_type, "{\"record_type\":0}");
            if (task_info.file_id)
                file_id = mid_record_open0(record_index, p_ch_info->ch_url, APP_TYPE_IPTV, record_type, strlen(record_type), task_info.file_id, getLocalTime(), task_info.end_time);
            else
                file_id = mid_record_open0(record_index, p_ch_info->ch_url, APP_TYPE_IPTV, record_type, strlen(record_type), 0, task_info.start_time, task_info.end_time);
        }
#ifdef INCLUDE_DVBS
        else if (task_info.content_source == DVB_PVR) {
            strcpy(record_type, "{\"record_type\":1}");
            mid_record_set_tuner(record_index, 1);
            if (task_info.file_id) {
                file_id = mid_record_open0(record_index, p_ch_info->ch_url, APP_TYPE_DVBS, record_type, strlen(record_type), task_info.file_id, getLocalTime(), task_info.end_time);
            } else {
                file_id = mid_record_open0(record_index, p_ch_info->ch_url, APP_TYPE_DVBS, record_type, strlen(record_type), 0, task_info.start_time, task_info.end_time);
            }
        }
#endif
        LogCpvrDebug("file id is %d[%x]\n",file_id, file_id);
        cpvr_list_task_file_id_update(task_info.schedule_id, file_id);
        if (p_ch_info->ch_bandwidth > 0)
            cpvr_list_task_bandwidth_update(task_info.schedule_id, p_ch_info->ch_bandwidth);
        if (p_ch_info->ch_parental_rating != 0) {
            sprintf(task_info.parental_rating, "%d", p_ch_info->ch_parental_rating);
            cpvr_list_task_parental_rating_update(task_info.schedule_id, task_info.parental_rating);
        }

        mid_record_open1(record_index);
        i_ret = 0;

    }

    if (i_ret != 0) {
        cpvr_task_state_set(task_info.schedule_id, task_state, err_code);
        return -1;
    } else {
        if (task_info.content_source == DVB_PVR)
            cpvr_list_task_active(task_info.schedule_id, record_index, &tuner_result);
        else
            cpvr_list_task_active(task_info.schedule_id, record_index, NULL);

        //cpvr_task_state_set(task_info.schedule_id, IN_PROGRESS_STATE, TASK_OK);
        return 0;
    }
}

void cpvr_task_create(char *schedule_id)
{
    time_t tm_now = 0;
    time_t tm_begin = 0;

    cpvr_task_info_t    task_info;

    if (schedule_id)
        LogCpvrDebug("schedule id is %s\n",schedule_id);

    if (schedule_id && cpvr_list_task_get_by_schedule_id(schedule_id, &task_info, NULL) == 0) {
        if (task_info.task_state == IN_PROGRESS_STATE
            || task_info.task_state == IN_PROGRESS_INCOMPLETE_STATE
            || task_info.task_state == FAILED_STATE
            || task_info.task_state == INCOMPLETE_STATE
            || task_info.task_state == COMPLETED_STATE) {
            return;
        }

        tm_now = getLocalTime();
        if (task_info.end_time <= tm_now) {
            if (task_info.file_id > 0 && ind_pvr_exist(task_info.file_id)) {
                cpvr_task_state_set(schedule_id, INCOMPLETE_STATE, TASK_OK);
            } else {
                cpvr_task_state_set(schedule_id, FAILED_STATE, UKNOW);
            }

            return ;
        }

        tm_begin = task_info.start_time > tm_now ? task_info.start_time : tm_now;

        LogCpvrDebug("the start time is %d, now is %d.\n", task_info.start_time, tm_now);
        LogCpvrDebug("The task [%s] will be starting after %d seconds.\n", task_info.schedule_id, tm_begin - tm_now);

        if (tm_begin - 60 > tm_now)
            need_handle_queue_node_add(MSG_SEND_START_IN_ONE_MINUTE_STATE, task_info.schedule_id, tm_begin - 60);
        else if ((tm_begin - tm_now) < 60 && (tm_begin - tm_now) > 0)
            need_handle_queue_node_add(MSG_SEND_START_IN_ONE_MINUTE_STATE, task_info.schedule_id, tm_now);
        need_handle_queue_node_add(MSG_TASK_START, task_info.schedule_id, tm_begin);
        need_handle_queue_node_add(MSG_TASK_END, task_info.schedule_id, task_info.end_time + 5);
    }
}

void cpvr_task_serial_record_update(char *schedule_id, int resume)
{
    int i = 0;
    cpvr_task_info_t main_task_info;
    struct json_object* obj  = NULL;

    if (schedule_id && cpvr_list_task_get_by_schedule_id(schedule_id, &main_task_info, NULL) == 0) {
        if (main_task_info.task_type != REC_TYPE_SERIAL_MAIN || main_task_info.subtask_num_unfinished >= 16)
            goto EXIT_WAY;

#ifdef INCLUDE_DVBS
        if (isDVBChannelManageMode() == 1 && isDVBMode() && main_task_info.content_source == IPTV_PVR)
            goto EXIT_WAY;
        if (isDVBChannelManageMode() == 0 && main_task_info.content_source == IPTV_PVR)
            goto EXIT_WAY;
#endif

        obj = (struct json_object*)json_tokener_parse_string(main_task_info.serial_record_rule);
        if (obj) {
            int i_count = json_get_object_array_length(obj);

            for (i=0; i<i_count; i++) {
                struct json_object* sub_obj = NULL;
                sub_obj = json_object_get_array_idx(obj, i);
                if (sub_obj) {
                    int prog_count = 0;
                    int matchtype = 0, event_id[128] = {0};
                    serial_match_rule_t match_rule;

                    memset(&match_rule, 0, sizeof(match_rule));
                    param_string_get(sub_obj, "key", match_rule.key, sizeof(match_rule.key)-1);
                    param_string_get(sub_obj, "value", match_rule.value, sizeof(match_rule.value)-1);
                    param_string_get(sub_obj, "matchRule", match_rule.rule, sizeof(match_rule.rule)-1);
                    json_object_delete(sub_obj);

                    if (strncmp(match_rule.rule, "exact", 5) == 0)
                        matchtype = 1;      //accurate
                    else matchtype = 2;     //fuzzy without ignore case
#ifdef INCLUDE_DVBSs /* Warning, need modify */
                    int lang;
                    sysSettingGetInt("lang", &lang, 0);

                    prog_count = sizeof(event_id)/sizeof(event_id[0]);
                    prog_count = ManageEpgEventSeriesCount(atoi(main_task_info.channel_num), match_rule.key, matchtype, match_rule.value,
                                (char*)record_language[lang], event_id, &prog_count);
                    if (prog_count > 0) {
                        int j = 0;

                        for (j=0; j<prog_count; j++) {
                            serial_rec_program_info_t  prog_info;

                            memset(&prog_info, 0, sizeof(prog_info));
                            if (0 == ManageEpgEventSeriesGet(atoi(main_task_info.channel_num),event_id[j], (char*)record_language[lang], &prog_info)) {
                                cpvr_ask_info_t new_subtask;

                                memcpy(&new_subtask, &main_task_info, sizeof(new_subtask));
                                new_subtask.task_type = REC_TYPE_SERIAL_SUB;
                                new_subtask.start_time = prog_info.st;
                                new_subtask.start_time = prog_info.et;
                                sprintf(new_subtask.prog_id, "%d", event_id[j]);
                                strncpy(new_subtask.prog_title, prog_info.title, sizeof(new_subtask.prog_title));
                                strncpy(new_subtask.program_description, prog_info.descr, sizeof(new_subtask.program_description));
                                strncpy(new_subtask.main_task_id[0], main_task_info.schedule_id, sizeof(new_subtask.main_task_id[0]));
                                cpvr_task_schedule_id_create(&new_subtask);

                                if (db_cpvr_task_info_write(&new_subtask) != 0)
                                    continue;

                                cpvr_list_task_add(&new_subtask);
                                cpvr_task_create(new_subtask.schedule_id);

                                main_task_info.subtask_num_total ++;
                                main_task_info.subtask_num_unfinished ++;
                                cpvr_list_subtask_num_update(main_task_info.schedule_id, main_task_info.subtask_num_total, main_task_info.subtask_num_unfinished);
                            }
                        }
                    }
#endif

                }
                else break;
            }

            json_object_delete(obj);
        }
        else goto EXIT_WAY;
    }
    sendEventPvrSubtasklistChangeToEpg();

EXIT_WAY:
    return ;
}

static void _period_record_wday_parse(int *array, int array_size, char *type_str)
{
    int i = 0, i_ret = -1;
    char *ps_parse = strdup(type_str);
    char *ps_buf = ps_parse;

    if (ps_parse) {
        ps_parse ++;

        while(ps_parse && i < array_size) {
            char *ptr = strchr(ps_parse, ',');
            if (ptr) {
                *ptr = 0;
                array[i++] = atoi(ps_parse);
                ps_parse = ptr + 1;
                continue;
            }

            ptr = strchr(ps_parse, ']');
            if (ptr) {
                *ptr = 0;
                array[i] = atoi(ps_parse);
            }

            break;
        }
    }

    if (ps_buf) MEM_FREE(ps_buf);
}

static time_t _get_time_next_wday(time_t base, int *wday_need_rec, int size, int next)
{
    int i = 0, one_day_sec = 24*60*60;

    if (next) base += one_day_sec;

    while(1) {
        int i = 0, get = 0;
        struct tm *p_tm;

        p_tm = localtime(&base);
        for (i=0; i<size; i++) {
            if (wday_need_rec[i] != 0 && p_tm->tm_wday == wday_need_rec[i]) {
                get = 1;
                break;
            }
        }

        if (get == 1) break;
        else base += one_day_sec;
    }

    return base;
}

void cpvr_task_period_record_update(char *schedule_id, int resume)
{
    cpvr_task_info_t    main_task_info;

    if (schedule_id && cpvr_list_task_get_by_schedule_id(schedule_id, &main_task_info, NULL) == 0) {
        int wday[8] = {0};
        time_t start_time_bak = 0;
        time_t end_time_bak = 0;

        cpvr_list_task_info_print(&main_task_info);

        if (main_task_info.task_type != REC_TYPE_PERIOD_MAIN || main_task_info.subtask_num_unfinished >= 16)
            goto EXIT_WAY;

#ifdef INCLUDE_DVBS
        if (isDVBChannelManageMode() == 1 && isDVBMode() == 1 && main_task_info.content_source == IPTV_PVR)
            goto EXIT_WAY;
        if (isDVBChannelManageMode() == 0 && main_task_info.content_source == IPTV_PVR)
            goto EXIT_WAY;
#endif

        while (main_task_info.subtask_num_unfinished < 16) {
            int one_day_sec = 24*60*60;

            cpvr_task_info_t    new_task, temp_task;
            cpvr_task_filter_param_t param;

            memcpy(&new_task, &main_task_info, sizeof(new_task));

            new_task.task_type = REC_TYPE_SUB;
            cpvr_task_schedule_id_create(&new_task);
            new_task.task_type = REC_TYPE_PERIOD_SUB;
            new_task.task_state = PENDING_NO_CONFLICT_STATE;
            if (strncmp(main_task_info.period_type, "weekly", 6) == 0) {
                if (start_time_bak == 0) {
                    start_time_bak = (main_task_info.main_task_start_time - main_task_info.main_task_start_time%one_day_sec) + main_task_info.start_time;
                    end_time_bak = (main_task_info.main_task_start_time - main_task_info.main_task_start_time%one_day_sec) + main_task_info.end_time;

                    _period_record_wday_parse(wday, sizeof(wday)/sizeof(wday[0]), main_task_info.period_every_week);
                    new_task.start_time = _get_time_next_wday(start_time_bak, wday, sizeof(wday)/sizeof(wday[0]), 0);
                    new_task.end_time = end_time_bak + (new_task.start_time - start_time_bak);
                } else {
                    new_task.start_time = _get_time_next_wday(start_time_bak, wday, sizeof(wday)/sizeof(wday[0]), 1);
                    new_task.end_time = end_time_bak + (new_task.start_time - start_time_bak);
                }
                start_time_bak = new_task.start_time;
                end_time_bak = new_task.end_time;
            } else if (strncmp(main_task_info.period_type, "daily", 5) == 0) {
                if (start_time_bak == 0) {
                    new_task.start_time = (main_task_info.main_task_start_time - main_task_info.main_task_start_time%one_day_sec) + main_task_info.start_time;
                    new_task.end_time = (main_task_info.main_task_start_time - main_task_info.main_task_start_time%one_day_sec) + main_task_info.end_time;
                } else {
                    new_task.start_time = start_time_bak + one_day_sec;
                    new_task.end_time = end_time_bak + one_day_sec;
                }
                start_time_bak = new_task.start_time;
                end_time_bak = new_task.end_time;
            }

            if (getLocalTime() > new_task.end_time){
                LogCpvrDebug("Now is %d, The end time of new task is %d.\n", getLocalTime(), new_task.end_time);
                continue;
            }

            if (main_task_info.main_task_end_time > 0 && new_task.start_time > main_task_info.main_task_end_time){
                LogCpvrDebug("The start time of new task is %d, The end time of main task is %d.\n", new_task.start_time, main_task_info.main_task_end_time);
                break;
            }

            memset(&param, 0, sizeof(param));
            param.start_time = new_task.start_time;
            param.end_time = new_task.end_time;
            if (cpvr_list_task_get(&param, &temp_task, NULL) == 0) {
                if (temp_task.delete_flg == 1 && resume == 1) {
                    temp_task.delete_flg = 0;
                    cpvr_list_task_delete_flg_update(temp_task.schedule_id, temp_task.delete_flg);
                    cpvr_task_create(temp_task.schedule_id);
                }
                continue;
            }

            strcpy(new_task.main_task_id[0], main_task_info.schedule_id);

            if (db_cpvr_task_info_write(&new_task) != 0)
                continue;
            cpvr_list_task_add(&new_task);
            cpvr_task_create(new_task.schedule_id);

            main_task_info.subtask_num_total ++;
            main_task_info.subtask_num_unfinished ++;
            cpvr_list_subtask_num_update(main_task_info.schedule_id, main_task_info.subtask_num_total, main_task_info.subtask_num_unfinished);
        }
    }
    sendEventPvrSubtasklistChangeToEpg();
EXIT_WAY:
    return ;
}

static int s_DeletePvrFileProgress = 0;
static unsigned int s_DeletePvrFileid = 0;
static void setDeletePvrFileProgress(unsigned int id, int msg)
{
    s_DeletePvrFileid = id;
    s_DeletePvrFileProgress = msg;
    LogCpvrDebug("fileid:%d,DeletePvrFilePrograss:[%d]\n",s_DeletePvrFileid , s_DeletePvrFileProgress);
}

int getDeletePvrFileProgressByFileid(unsigned int file_id)
{
    if (file_id == s_DeletePvrFileid)
        return s_DeletePvrFileProgress;

    return -2;
}

static int _record_delete(char *schedule_id, int real, int delete_file)
{
    cpvr_task_info_t task_info;
    cpvr_task_live_t task_live;

    if (schedule_id)
        LogCpvrDebug("schedule id is %s\n",schedule_id);

    if (schedule_id != NULL && cpvr_list_task_get_by_schedule_id(schedule_id, &task_info, &task_live) == 0) {
        need_handle_queue_node_remove_all(task_info.schedule_id);
        need_handle_queue_node_add(MSG_TASK_RELEASE_RES, task_info.schedule_id, getLocalTime());

        if (task_live.active == 1) {
            mid_record_close(task_live.index, task_info.file_id);
            cpvr_list_task_inactive(task_info.schedule_id);
        }

        if (real == 1) {
            if (delete_file == 1 && task_info.file_id > 0) {
                LogCpvrDebug( "delete file : %d\n", task_info.file_id);
                mid_record_delete(task_info.file_id, setDeletePvrFileProgress);
            }

            if (cpvr_task_sync_flag_set(&task_info, SYNC_FLAG_DELETED) != 0)
                cpvr_list_task_remove(schedule_id);
            else
                cpvr_list_task_delete_flg_update(schedule_id, 1);
        } else {
            cpvr_task_sync_flag_set(&task_info, SYNC_FLAG_DELETED);
            cpvr_list_task_delete_flg_update(schedule_id, 1);
        }

        return 0;
    }
    else return -1;
}

static int _subtask_to_normal_task(cpvr_task_info_t *p_info)
{
    if (p_info) {
        p_info->task_type == REC_TYPE_NORMAL;
        memset(p_info->main_task_id, 0, sizeof(p_info->main_task_id));
        cpvr_task_info_update(p_info);
    }
}

int cpvr_task_delete(cpvr_task_info_t *p_info, char *subtask_state, int delete_file)
{
    if (p_info != NULL) {
        if (p_info->task_type == REC_TYPE_PERIOD_MAIN || p_info->task_type == REC_TYPE_SERIAL_MAIN) {
            int i_cnt = 0;
            int state[32] = {0};
            cpvr_task_filter_param_t param;
            struct _copied_hdr_t hdr = TAILQ_HEAD_INITIALIZER(hdr);

            memset(&param, 0, sizeof(param));
            param.sync_flag = SYNC_FLAG_ALL;
            param.delete_flg = 2;
            strcpy(param.main_task_id, p_info->schedule_id);

            if (subtask_state)
                cpvr_task_state_parse(state, sizeof(state)/sizeof(state[0]), subtask_state, '"');

            i_cnt = cpvr_list_get(&param, 0, 0, &hdr);
            if (i_cnt) {
                 _copied_node_t *p_node = NULL;

                TAILQ_FOREACH(p_node, &hdr, list) {
                    int i = 0, find = 0;

                    if (p_node->info.delete_flg == 1)  {//已经删除的任务
                        _record_delete(p_node->info.schedule_id, 1, 1);
                    }

                    if (state[0] == UNINITED_STATE) {
                        find = 1;
                    } else {
                        for (i=0; i<sizeof(state)/sizeof(state[0]); i++) {
                            if (p_node->info.task_state == state[i]) {
                                find = 1;
                                break;
                            }
                        }
                    }
                    if (find == 1)
                        _record_delete(p_node->info.schedule_id, 1, 1);  //删除指定状态的子任务
                    else
                        _subtask_to_normal_task(&p_node->info);       //未删除的子任务转成普通任务
                }

                cpvr_copied_list_release(&hdr);
            }

            //delete the father task.
            _record_delete(p_info->schedule_id, 1, 1);
        } else if (p_info->task_type == REC_TYPE_PERIOD_SUB || p_info->task_type == REC_TYPE_SERIAL_SUB) {
            _record_delete(p_info->schedule_id, 0, 0);
            sendEventPvrSubtasklistChangeToEpg();
        } else if (p_info->task_type == REC_TYPE_NORMAL || p_info->task_type == REC_TYPE_UKNOW)
            _record_delete(p_info->schedule_id, 1, delete_file);

        return 0;
    }
    else return -1;
}

static int s_delete_all_files_progress = 0;
static pthread_mutex_t s_delete_all_files_mutex = PTHREAD_MUTEX_INITIALIZER;

void* cpvr_task_delete_all_files(void *p)
{
    int i_cnt = 0;
    cpvr_task_filter_param_t param;
    struct _copied_hdr_t hdr = TAILQ_HEAD_INITIALIZER(hdr);

    memset((char *)&param, 0, sizeof(param));
    param.sync_flag = SYNC_FLAG_ALL;
    param.delete_flg = 2;
    pthread_mutex_lock(&s_delete_all_files_mutex);
    s_delete_all_files_progress = 0;
    i_cnt = cpvr_list_get(&param, 0, 0, &hdr);
    if (i_cnt > 0) {
        int i = 0;
        _copied_node_t *p_node = NULL;

        TAILQ_FOREACH(p_node, &hdr, list) {
            if (p_node->info.file_id > 0 && taskHasEnded(&p_node->info)) {
                if (p_node->info.task_type == REC_TYPE_NORMAL)
                    cpvr_task_delete(&p_node->info, NULL, 1);
            }
            i++;
            s_delete_all_files_progress = (i*100)/i_cnt;
        }

        cpvr_copied_list_release(&hdr);
    }
    pthread_mutex_unlock(&s_delete_all_files_mutex);
    return NULL;
}

int cpvr_task_delete_all_files_progress_get()
{
    LogCpvrDebug("The progress of delete all files is %d.\n", s_delete_all_files_progress);
    return s_delete_all_files_progress;
}

static int _task_time_adjust(cpvr_task_info_t *p_info, cpvr_task_live_t *p_live, time_t tm_start, time_t tm_end)
{
    int     i_ret = -1;

    if (p_info != NULL && p_live != NULL) {
        if (tm_start != 0) {
            if (taskHasStarted(p_info) == 0) {
                p_info->start_time = tm_start;
                cpvr_list_task_time_update(p_info->schedule_id, tm_start, 0);
                cpvr_task_create(p_info->schedule_id);
                cpvr_task_sync_flag_set(p_info, SYNC_FLAG_CHANGE);
                i_ret = 0;
            }
        }
        if (tm_end != 0 && taskHasEnded(p_info) == 0) {
            if (p_live->index >= 0 && p_info->end_time != tm_end) {
                mid_record_set_endtime(p_live->index, 0, tm_end);
            }

            p_info->end_time = tm_end;
            need_handle_queue_node_add(MSG_TASK_END, p_info->schedule_id, p_info->end_time + 5);
            cpvr_list_task_time_update(p_info->schedule_id, 0, tm_end);
            cpvr_task_sync_flag_set(p_info, SYNC_FLAG_CHANGE);
            i_ret = 0;
        }
    }

    return i_ret;
}

int cpvr_task_time_adjust(char *schedule_id, time_t tm_start, time_t tm_end)
{
    int i_ret = -1;

    cpvr_task_info_t task_info;
    cpvr_task_live_t task_live;

    if (schedule_id != NULL && cpvr_list_task_get_by_schedule_id(schedule_id, &task_info, &task_live) == 0) {
        if (task_info.task_type == REC_TYPE_PERIOD_MAIN) {
            int i_cnt = 0;
            cpvr_task_filter_param_t param;
            struct _copied_hdr_t hdr = TAILQ_HEAD_INITIALIZER(hdr);

            memset(&param, 0, sizeof(param));
            strcpy(param.main_task_id, task_info.schedule_id);
            param.sync_flag = SYNC_FLAG_ALL;
            i_cnt = cpvr_list_get(&param, 0, 0, &hdr);
            if (i_cnt > 0) {
                _copied_node_t *p_node = NULL;

                TAILQ_FOREACH(p_node, &hdr, list) {
                    _task_time_adjust(&p_node->info, &p_node->live, tm_start, tm_end);
                }
                cpvr_copied_list_release(&hdr);
            }

        }

        return _task_time_adjust(&task_info, &task_live, tm_start, tm_end);

    }

    return i_ret;
}

int cpvr_task_sync_flag_set(cpvr_task_info_t *p_info, int sync_flg)
{
    int i_ret = -1;

    if (isDVBMode()) {//dvb mode
        /*** 平台不管理模式，忽略DVB 录制 ***/
        if (isDVBChannelManageMode() != 1 && p_info->content_source == DVB_PVR)
            return i_ret;

        i_ret = cpvr_list_task_sync_flag_update(p_info->schedule_id, sync_flg);
    }

    return i_ret;
}

int cpvr_task_sync_flag_clear(char *need_clear_id_str, int need_clear_flg)
{
    if (need_clear_id_str) {
        array_t *p_array = array_new(12, 72);
        if (p_array) {
            int i = 0;

            param_to_array(need_clear_id_str, p_array);
            for (i = 0; i < p_array->length; i ++) {
                char *p_id = (char*)array_get(p_array, i);
                if (p_id && p_id[0] != 0) {
                    cpvr_task_info_t task_info;

                    cpvr_list_task_get_by_schedule_id(p_id, &task_info, NULL);
                    if (task_info.sync_flag == SYNC_FLAG_DELETED)
                        cpvr_list_task_remove(p_id);
                    else cpvr_list_task_sync_flag_update(p_id, SYNC_FLAG_NONEED);
                }
            }

            array_free(p_array);
        }
    } else if (need_clear_flg != -1) {
        int i_cnt = 0;
        cpvr_task_filter_param_t  param;
        struct _copied_hdr_t hdr = TAILQ_HEAD_INITIALIZER(hdr);

        memset(&param, 0, sizeof(param));
        param.sync_flag = need_clear_flg;
        param.delete_flg = 2;
        i_cnt = cpvr_list_get(&param, 0, 0, &hdr);
        if (i_cnt) {
            _copied_node_t *p_node = NULL;

            TAILQ_FOREACH(p_node, &hdr, list) {
                if (p_node->info.sync_flag == SYNC_FLAG_DELETED)
                    cpvr_list_task_remove(p_node->info.schedule_id);
                else
                    cpvr_list_task_sync_flag_update(p_node->info.schedule_id, SYNC_FLAG_NONEED);
            }

            cpvr_copied_list_release(&hdr);
        }
    }

    return 0;
}

static void cpvr_message_process(int file_id, CPVR_TASK_STATE state, CPVR_TASK_ERROR_CODE err_code)
{
    int need_stop = 0;
    cpvr_task_info_t info;

    if ((int)file_id >0) {
        cpvr_task_filter_param_t param;

        memset(&param, 0, sizeof(param));
        param.file_id = file_id;
        if (cpvr_list_task_get(&param, &info, NULL) == 0) {
            if (state == IN_PROGRESS_STATE) {
                if (info.task_state == IN_PROGRESS_WITH_ERROR_STATE)
                    info.task_state = IN_PROGRESS_INCOMPLETE_STATE;
                else
                    info.task_state = IN_PROGRESS_STATE;

                info.error_code = TASK_OK;

                cpvr_task_state_set(info.schedule_id, info.task_state, info.error_code);
                return ;
            }

            if (state == COMPLETED_STATE) {
                if (ind_pvr_exist(info.file_id)) {
                    if (info.task_state == IN_PROGRESS_STATE)
                        info.task_state = COMPLETED_STATE;
                    else
                        info.task_state = INCOMPLETE_STATE;

                    info.error_code = TASK_OK;
                } else {
                    info.task_state = FAILED_STATE;
                    info.error_code = UKNOW;
                }
                need_stop = 1;
            } else if (state == IN_PROGRESS_WITH_ERROR_STATE) {
                if (getLocalTime() > info.end_time) {
                    if (ind_pvr_exist(info.file_id))
                        info.task_state = INCOMPLETE_STATE;
                    else
                        info.task_state = FAILED_STATE;
                } else
                    info.task_state = IN_PROGRESS_WITH_ERROR_STATE;

                info.error_code = err_code;
                need_stop = 1;
            }
            else return ;

            cpvr_task_state_set(info.schedule_id, info.task_state, info.error_code);
            if (need_stop == 1) {
                //need_handle_queue_node_remove_all(info.schedule_id);
                need_handle_queue_node_add(MSG_TASK_CLOSE,info.schedule_id, getLocalTime());
                if (info.task_state == IN_PROGRESS_WITH_ERROR_STATE) {
                    LogCpvrDebug("The task %s will be restart after 5 s.", info.schedule_id);
                    need_handle_queue_node_add(MSG_TASK_START, info.schedule_id, getLocalTime() + 5);
                }
            } else {
                need_handle_queue_node_remove_all(info.schedule_id);
                need_handle_queue_node_add(MSG_TASK_RELEASE_RES,info.schedule_id, getLocalTime());
            }
        } else
            LogCpvrError("Can not found the task by file id %d[%x].\n", file_id, file_id);
    }
}

static void record_msg_callback(int index, unsigned int pvr_id, STRM_MSG msg, int arg)
{
    cpvr_task_info_t info;

    LogCpvrDebug( "file id is %d[%x], msg is %d\n", pvr_id, pvr_id, msg);
    switch (msg){
    case RECORD_MSG_NOT_ENOUGH:
    case RECORD_MSG_DISK_ERROR:
        cpvr_message_process(pvr_id, IN_PROGRESS_WITH_ERROR_STATE, ACCESS_FORBIDDEN);
        break;
    case RECORD_MSG_NET_TIMEOUT:
        cpvr_message_process(pvr_id, IN_PROGRESS_WITH_ERROR_STATE, SERVICE_UNAVAILABLE);
        break;
    case RECORD_MSG_DISK_FULL:
        cpvr_message_process(pvr_id, IN_PROGRESS_WITH_ERROR_STATE, SPACE_FULL);
        break;
    case RECORD_MSG_SUCCESS_BEGIN:
        cpvr_message_process(pvr_id, IN_PROGRESS_STATE, TASK_OK);
        break;
    case RECORD_MSG_ERROR:
        cpvr_message_process(pvr_id, IN_PROGRESS_WITH_ERROR_STATE, SERVICE_UNAVAILABLE);
        break;
    case RECORD_MSG_SUCCESS_END:
        cpvr_message_process(pvr_id, COMPLETED_STATE, TASK_OK);
        break;
    case RECORD_MSG_REFUSED_SOCKET:
    case RECORD_MSG_REFUSED_SERVER:
    case RECORD_MSG_NOT_FOUND:
    case RECORD_MSG_DATA_DAMAGE:
        cpvr_message_process(pvr_id, IN_PROGRESS_WITH_ERROR_STATE, SERVICE_VANISHED);
        break;
    case RECORD_MSG_DISK_WARN:
        //storage_state_event_send_by_index(AVAILABLE_SPACE_REACH_90, 1);
        break;
    default:
        break;
    }
}

void cpvr_ability_set(cpvr_ability_t *p_ability)
{
    if (p_ability) {
        char tbuf[256] = {0};

        memcpy(&g_cpvr_ability, p_ability, sizeof(g_cpvr_ability));
        sprintf(tbuf, "%d#%d#%d", g_cpvr_ability.allow_pvr, g_cpvr_ability.allow_concurrent, g_cpvr_ability.disk_quta);
        LogCpvrDebug("Set cpvr ability to %s.\n", tbuf);
        db_cpvr_ability_set(tbuf);
    }
}

void cpvr_ability_get(cpvr_ability_t *p_ability)
{
    if (p_ability) {
        char tbuf[256] = {0};
        db_cpvr_ability_get(tbuf, sizeof(tbuf));
        sscanf(tbuf, "%d#%d#%d", &g_cpvr_ability.allow_pvr, &g_cpvr_ability.allow_concurrent, &g_cpvr_ability.disk_quta);
        memcpy(p_ability, &g_cpvr_ability, sizeof(g_cpvr_ability));
    }
}

void cpvrTaskUpdateFromMEM(void)
{
    int iCount = 0;
    int index = 0;
    cpvr_task_info_t  newTask;
    cpvr_task_filter_param_t  filter;
    struct _copied_hdr_t hdr = TAILQ_HEAD_INITIALIZER(hdr);

    memset(&filter, 0, sizeof(filter));
    filter.sync_flag = SYNC_FLAG_ALL;
    filter.delete_flg = 2;

    if (isDVBMode()) {
        LogCpvrDebug("is dvb mode.\n");
        return ;
    }

    iCount = cpvr_list_get(&filter, 0, 0, &hdr);
    if (iCount > 0) {
        _copied_node_t *pNode = NULL;

        TAILQ_FOREACH(pNode, &hdr, list) {
            cpvr_task_info_t taskInfo;

            if (!isDVBChannelManageMode() && pNode->info.content_source == DVB_PVR)
                continue;
            if (pNode->info.task_type == REC_TYPE_SERIAL_MAIN || pNode->info.task_type == REC_TYPE_PERIOD_MAIN)
                continue;

            if (cpvr_tmp_list_task_get(pNode->info.schedule_id, &taskInfo) == 0) {
                if (pNode->info.sync_flag || taskHasEnded(&pNode->info)) {
                    cpvr_tmp_list_task_remove(taskInfo.schedule_id);
                    continue;
                }

                if (recordFileExists(pNode->info.file_id)) {
                    int i = 0;

                    if (taskHasStarted(&pNode->info)) {
                        if (taskInfo.end_time != pNode->info.end_time)
                            _task_time_adjust(&pNode->info, &pNode->live, 0, taskInfo.end_time);
                        for (i=0; i<sizeof(taskInfo.main_task_id)/sizeof(taskInfo.main_task_id[0]); i++) {
                            strncpy(pNode->info.main_task_id[i], taskInfo.main_task_id[i], sizeof(taskInfo.main_task_id[i]));
                        }
                        cpvr_task_info_update(&pNode->info);
                        cpvr_tmp_list_task_remove(taskInfo.schedule_id);
                        continue;
                    }
                } else {
                    _record_delete(pNode->info.schedule_id, 1, 1);
                }
            } else {
                if (pNode->info.sync_flag || recordFileExists(pNode->info.file_id))
                    continue;
                else
                    _record_delete(pNode->info.schedule_id, 1, 1);
            }
        }

        cpvr_copied_list_release(&hdr);
    }

    while (1) {
        if (cpvr_tmp_list_task_get_by_index(index, &newTask) != 0)
            break;

        newTask.task_state = PENDING_NO_CONFLICT_STATE;
        if (db_cpvr_task_info_write(&newTask) != 0) {
            index++;
            continue;
        }

        if (cpvr_list_task_add(&newTask) == 0)
            cpvr_task_create(newTask.schedule_id);

        index++;
    }
}

static int cpvr_opened = 0;

int cpvr_open()
{
    struct pvr_partition pvr_partition;

    if (cpvr_opened != 0)
        return 0;

    if (pvr_partition_info_get(&pvr_partition) != 0) {
        LogCpvrError("pvr partition not found.\n");
        return -1;
    } else {
        cpvr_ability_t ability;
        char recordDIR[128] = {0};

        memset(&ability, 0, sizeof(ability));
        ability.allow_pvr = 1;
        ability.allow_concurrent = 0;
        ability.disk_quta = pvr_partition.partition_free_size;

        sprintf(recordDIR, "%s/record", pvr_partition.mounted_dir);
        LogCpvrDebug("Set record dir to %s\n", recordDIR);
        record_port_set_param(recordDIR);
        mid_record_mount();
        record_port_msg_hdl_set(record_msg_callback);
        cpvr_list_init();
        cpvr_ability_set(&ability);
        cpvr_opened = 1;
        return 0;
    }
}

void cpvr_close()
{
    cpvr_opened = 0;
    cpvr_list_destroy();
    record_port_msg_hdl_delete(record_msg_callback);
    cpvr_task_stop_all(REC_TYPE_UKNOW);
    mid_record_unmount();
}
}

extern "C" int cpvrOpen()
{
    return Hippo::cpvr_open();
}

