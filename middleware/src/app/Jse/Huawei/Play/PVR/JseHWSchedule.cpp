
#include "JseHWSchedule.h"

#include "JseFunctionCall.h"

#include "JseAssertions.h"

#include "CpvrTaskManager.h"
#include "CpvrAuxTools.h"
#include "CpvrList.h"
#include "CpvrDB.h"

#include "json_object.h"
#include "json_public.h"
#include "disk_info.h"
#include "mid_record.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace Hippo;

const char* cpvr_task_type_str[] = {
    "unknownRecord",
    "normalRecord",
    "serialRecord",
    "periodRecord",
    "subRecord",
    NULL
};


extern char* cpvr_task_state_str[];


// 此函数使用了cpvr_task_state_str， 只被函数cpvr_json_build调用。
static int cpvr_json_build_one_record(char* aFieldValue, cpvr_task_info_t* p_info)
{
    char tbuf[1024] = {0};

    json_add_str(aFieldValue, "schedule_id", p_info->schedule_id);
    json_add_str(aFieldValue, "channel_num", p_info->channel_num);
    json_add_str(aFieldValue, "channel_name", p_info->channel_name);

    if (p_info->task_type == REC_TYPE_SERIAL_SUB || p_info->task_type == REC_TYPE_PERIOD_SUB)
        json_add_str(aFieldValue, "recordType", cpvr_task_type_str[REC_TYPE_SUB]);
    else
        json_add_str(aFieldValue, "recordType", cpvr_task_type_str[p_info->task_type]);

    json_add_str(aFieldValue, "prog_id", p_info->prog_id);
    json_add_str(aFieldValue, "prog_title", p_info->prog_title);
    json_add_str(aFieldValue, "programDescription", p_info->program_description);
    json_add_str(aFieldValue, "parentalRating", p_info->parental_rating);
    json_add_str(aFieldValue, "PackageId", p_info->package_id);
    json_add_str(aFieldValue, "PackageName", p_info->package_name);
    json_add_str(aFieldValue, "PackageName", p_info->package_name);
    sprintf(tbuf, "%d", p_info->protection_mode);
    json_add_str(aFieldValue, "protectionModel", tbuf);
    sprintf(tbuf, "%d", p_info->priority);
    json_add_str(aFieldValue, "priority", tbuf);
    sprintf(tbuf, "%d", p_info->pltv_tag);
    json_add_str(aFieldValue, "pltvTag", tbuf);
    sprintf(tbuf, "%d", p_info->auto_delete);
    json_add_str(aFieldValue, "autoDelete", tbuf);
    sprintf(tbuf, "%d", p_info->reserved_duration);
    json_add_str(aFieldValue, "reservedDuation", tbuf);
    sprintf(tbuf, "%d", p_info->sync_flag);
    json_add_str(aFieldValue, "needSync", tbuf);

    if (p_info->content_source == IPTV_PVR)
        strcpy(tbuf, "IP");
    else if (p_info->content_source == DVB_PVR)
        strcpy(tbuf, "DVB");
    json_add_str(aFieldValue, "contentPath", tbuf);

    json_add_str(aFieldValue, "state", cpvr_task_state_str[p_info->task_state]);
    sprintf(tbuf, "%d", p_info->error_code);
    json_add_str(aFieldValue, "exception_code", tbuf);

    timeSecNnumToString(p_info->start_time, tbuf, 0);
    json_add_str(aFieldValue, "starttime", tbuf);
    timeSecNnumToString(p_info->end_time, tbuf, 0);
    json_add_str(aFieldValue, "endtime", tbuf);

    timeSecNnumToString(p_info->real_start_time, tbuf, 0);
    json_add_str(aFieldValue, "record_starttime", tbuf);
    timeSecNnumToString(p_info->real_end_time, tbuf, 0);
    json_add_str(aFieldValue, "record_endtime", tbuf);

    if (p_info->task_type == REC_TYPE_SERIAL_MAIN || p_info->task_type == REC_TYPE_SERIAL_SUB) {
        int i = 0;
        char sub_str[1024] = {0};

        timeSecNnumToString(p_info->main_task_start_time, tbuf, 0);
        json_add_str(aFieldValue, "serialRecordStartTime", tbuf);
        timeSecNnumToString(p_info->main_task_end_time, tbuf, 0);
        json_add_str(aFieldValue, "serialRecordEndTime", tbuf);

        json_add_str(aFieldValue, "serialRecordRule", p_info->serial_record_rule);

        json_add_str(aFieldValue, "serialSn", p_info->serial_sn);
        json_add_str(aFieldValue, "seasonID", p_info->serial_season_id);
        json_add_str(aFieldValue, "serialName", p_info->serial_name);

        sprintf(tbuf, "%d", p_info->serial_episode_type);
        json_add_str(aFieldValue, "episodeType", tbuf);
    } else if (p_info->task_type == REC_TYPE_PERIOD_MAIN || p_info->task_type == REC_TYPE_PERIOD_SUB) {
        timeSecNnumToString(p_info->main_task_start_time, tbuf, 0);
        json_add_str(aFieldValue, "periodRecordStarttime", tbuf);
        timeSecNnumToString(p_info->main_task_end_time, tbuf, 0);
        json_add_str(aFieldValue, "periodRecordEndtime", tbuf);

        json_add_str(aFieldValue, "periodType", p_info->period_type);
        if (strncmp(p_info->period_type, "weekly", 6) == 0)
            json_add_str(aFieldValue, "periodEveryWeek", p_info->period_every_week);
        else if (strncmp(p_info->period_type, "monthly", 7) == 0)
            json_add_str(aFieldValue, "periodEveryMonth", p_info->period_every_month);
        json_add_str(aFieldValue, "periodName", p_info->period_name);
    }

    if (p_info->task_type == REC_TYPE_SERIAL_SUB || p_info->task_type == REC_TYPE_PERIOD_SUB) {
        int i = 0;

        memset(tbuf, 0, sizeof(tbuf));

        strcat(tbuf, "[");
        for (i=0; i<sizeof(p_info->main_task_id)/sizeof(p_info->main_task_id[0]); i++) {
            strcat(tbuf, "\"");
            strcat(tbuf, p_info->main_task_id[i]);
            strcat(tbuf, "\",");
        }
        tbuf[strlen(tbuf) - 1] = 0;
        strcat(tbuf, "]");
        LogJseDebug("father id is %s.\n", tbuf);
        json_add_str(aFieldValue, "fatherScheduleID", tbuf);
    }

    backspace_comma(aFieldValue);

}

// 此函数只被函数cpvr_porting_task_get_list（更名为JsePVRTaskListRead）调用
static int cpvr_json_build(char* aFieldValue, struct _copied_hdr_t* hdr)
{
    int build_num = 0;
    _copied_node_t* _copied_node = NULL;

    strcat(aFieldValue, ",\"Schedules\":[");
    TAILQ_FOREACH(_copied_node, hdr, list) {
        char tbuf[4096] = {0};

        strcat(tbuf, "{");
        cpvr_json_build_one_record(tbuf, &_copied_node->info);
        strcat(tbuf, "},");
        if ((strlen(aFieldValue)+strlen(tbuf)) > 4000) {
            LogJseDebug("the length is %d\n", strlen(aFieldValue)+strlen(tbuf));
            break;
        } else {
            strcat(aFieldValue, tbuf);
            build_num++;
        }
    }
    backspace_comma(aFieldValue);
    strcat(aFieldValue, "]");

    LogJseDebug("build_num : %d\n", build_num);
    return build_num;
}

// CpvrJsCall.cpp
static CPVR_TASK_PRIORITY cpvr_task_priority_str2enum(char* priority)
{
    if (!strncmp(priority, "RECORD_IF_NO_CONFLICTS", 22))
        return RECORD_IF_NO_CONFLICTS;
    else if (!strncmp(priority, "RECORD_WITH_CONFLICT", 20))
        return RECORD_WITH_CONFLICT;
    else if (!strncmp(priority, "RECORD_ANYWAY", 13))
        return RECORD_ANYWAY;
    else
        return RECORD_IF_NO_CONFLICTS;
}

// 此函数只为注册的函数间接调用
static CPVR_TASK_TYPE cpvr_task_type_str2enum(char* type_str)
{
    if (strncmp(type_str, "normalRecord", 12) == 0)
        return REC_TYPE_NORMAL;
    else if (strncmp(type_str, "serialRecord", 12) == 0)
        return REC_TYPE_SERIAL_MAIN;
    else if (strncmp(type_str, "periodRecord", 12) == 0)
        return REC_TYPE_PERIOD_MAIN;
    else if (strncmp(type_str, "subRecord", 9) == 0)
        return REC_TYPE_SUB;
    else
        return REC_TYPE_NORMAL;
}
// 此函数只为注册的函数间接调用
static void cpvr_task_type_parse(int* type_array, int array_size, char* type_str, char option)
{
    int i = 0, i_ret = -1;
    char* ps_buf = NULL;
    char* ps_parse = strdup(type_str);

    if (ps_parse) {
        char    *p_start, *p_end, *p_tmp;

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
            *(type_array+i) = cpvr_task_type_str2enum(p_tmp);

            i ++;
            p_end ++;
            ps_parse = p_end;
            p_start = strchr(ps_parse, option);
        }

        if (p_tmp) MEM_FREE(p_tmp);
    }

    if (ps_buf) MEM_FREE(ps_buf);
}


static int JsePVRUpdateWrite(const char* aFieldParam, char* value, int len)
{
	printf("_____tanf test:JsePVRUpdateWrite,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    struct json_object* object = NULL;

    cpvr_task_info_t new_task, tmp_task;

    if (value == NULL) return -1;
    LogJseDebug("param value is %s\n", value);

    object = (struct json_object*)json_tokener_parse_string(value);
    if (object) {
        char param_str[256] = {0};

        memset(&new_task, 0, sizeof(new_task));

        if (param_string_get(object, "recordType", param_str, sizeof(param_str)-1) == 0){
            new_task.task_type = cpvr_task_type_str2enum(param_str);
        } else {
            new_task.task_type = REC_TYPE_NORMAL;
        }

        if (param_string_get(object, "schedule_id", new_task.schedule_id, sizeof(new_task.schedule_id)-1) == -1){
            cpvr_task_schedule_id_create(&new_task);
        }

        if (param_string_get(object, "channel_num", new_task.channel_num, sizeof(new_task.channel_num)-1) == -1)
            goto ret_error;

        param_string_get(object, "prog_id", new_task.prog_id, sizeof(new_task.prog_id)-1);
        param_string_get(object, "prog_title", new_task.prog_title, sizeof(new_task.prog_title)-1);
        param_string_get(object, "programDescription", new_task.program_description, sizeof(new_task.program_description)-1);

        param_string_get(object, "PackageId", new_task.package_id, sizeof(new_task.package_id)-1);
        param_string_get(object, "PackageName", new_task.package_name, sizeof(new_task.package_name)-1);
        param_string_get(object, "parentalRating", new_task.parental_rating, sizeof(new_task.parental_rating)-1);

        new_task.content_source = IPTV_PVR;
        if(param_string_get(object, "contentPath", param_str, sizeof(param_str)-1) == 0) {
            if (strncmp(param_str, "DVB", 3) == 0)
                new_task.content_source = DVB_PVR;
        }

        param_string_get(object, "PackageName", new_task.package_name, sizeof(new_task.package_name)-1);
        if (param_string_get(object, "protectionModel", param_str, sizeof(param_str)-1) == -1)
            new_task.protection_mode = MODE_SAFE;
        else
            new_task.protection_mode = (PROTECTION_MODE)atoi(param_str);

        if (param_string_get(object, "priority", param_str, sizeof(param_str)-1) == -1)
            new_task.priority = RECORD_IF_NO_CONFLICTS;
        else
            new_task.priority = cpvr_task_priority_str2enum(param_str);

        if(param_string_get(object, "pltvTag", param_str, sizeof(param_str)-1) == 0) {
            new_task.pltv_tag = atoi(param_str);
        } else
            new_task.pltv_tag = 0;

        if(param_string_get(object, "autoDelete", param_str, sizeof(param_str)-1) == 0) {
            new_task.auto_delete = atoi(param_str);
        } else
            new_task.auto_delete = 0;

        if(param_string_get(object, "reservedDuation", param_str, sizeof(param_str)-1) == 0) {
            new_task.reserved_duration = atoi(param_str);
        } else
            new_task.reserved_duration = -1;

        if (param_string_get(object, "starttime", param_str, sizeof(param_str)-1) == 0) {
            if (new_task.task_type == REC_TYPE_PERIOD_MAIN) {
                int h = 0, m = 0, s = 0;

                if (strlen(param_str) != 6 || sscanf(param_str, "%02d%02d%02d", &h, &m, &s) != 3)
                    goto ret_error;

                new_task.start_time = h*3600 + m*60 + s;
            } else
                new_task.start_time = timeStringToSecNum(param_str);
        }
        else if (new_task.task_type == REC_TYPE_NORMAL) {
            new_task.start_time = getLocalTime();
        }

        if (param_string_get(object, "endtime", param_str, sizeof(param_str)-1) == 0) {
            if (new_task.task_type == REC_TYPE_PERIOD_MAIN) {
                int h = 0, m = 0, s = 0;

                if (strlen(param_str) != 6 || sscanf(param_str, "%02d%02d%02d", &h, &m, &s) != 3)
                    goto ret_error;

                new_task.end_time = h*3600 + m*60 + s;
            } else
                new_task.end_time = timeStringToSecNum(param_str);
        }

        if (new_task.task_type == REC_TYPE_SERIAL_MAIN) {
            if (param_string_get(object, "serialRecordStartTime", param_str, sizeof(param_str)-1) == 0) {
                new_task.main_task_start_time = timeStringToSecNum(param_str);
            } else {
                new_task.main_task_start_time = getLocalTime();
            }
            if (param_string_get(object, "serialRecordEndTime", param_str, sizeof(param_str)-1) == 0) {
                new_task.main_task_end_time = timeStringToSecNum(param_str);
            }

            param_string_get(object, "serialSn", new_task.serial_sn, sizeof(new_task.serial_sn)-1);
            param_string_get(object, "seasonID", new_task.serial_season_id, sizeof(new_task.serial_season_id)-1);
            param_string_get(object, "serialName", new_task.serial_name, sizeof(new_task.serial_name)-1);
            param_string_get(object, "serialRecordRule", new_task.serial_record_rule, sizeof(new_task.serial_record_rule)-1);
        }

        if (new_task.task_type == REC_TYPE_PERIOD_MAIN) {
            if (param_string_get(object, "periodRecordStartTime", param_str, sizeof(param_str)-1) == 0) {
                new_task.main_task_start_time = timeStringToSecNum(param_str);
            } else {
                new_task.main_task_start_time = getLocalTime();
            }
            if (param_string_get(object, "periodRecordEndTime", param_str, sizeof(param_str)-1) == 0) {
                new_task.main_task_end_time = timeStringToSecNum(param_str);
            }

            param_string_get(object, "periodType", new_task.period_type, sizeof(new_task.period_type)-1);
            if (strncmp(new_task.period_type, "weekly", 6) == 0)
                param_string_get(object, "periodEveryWeek", new_task.period_every_week, sizeof(new_task.period_every_week)-1);
            else if (strncmp(new_task.period_type, "monthly", 7) == 0)
                param_string_get(object, "periodEveryMonth", new_task.period_every_month, sizeof(new_task.period_every_month)-1);

            param_string_get(object, "periodName", new_task.period_name, sizeof(new_task.period_name)-1);
        }

        json_object_delete(object);

        if (cpvr_list_task_get_by_schedule_id(new_task.schedule_id, &tmp_task, NULL) == 0) {
            LogJseDebug( "The task %s already added.\n", new_task.schedule_id);
            if (tmp_task.task_state == CANCELLED_STATE)
                cpvr_task_create(new_task.schedule_id);
            return 0;
        }

        new_task.task_state = PENDING_NO_CONFLICT_STATE;
        if (db_cpvr_task_info_write(&new_task) != 0)
            goto ret_error;

        if (cpvr_list_task_add(&new_task) == 0) {
            cpvr_task_sync_flag_set(&new_task, SYNC_FLAG_CHANGE);
            if (new_task.task_type == REC_TYPE_PERIOD_MAIN)
                cpvr_task_period_record_update(new_task.schedule_id, 1);
            else if (new_task.task_type == REC_TYPE_SERIAL_MAIN)
                cpvr_task_serial_record_update(new_task.schedule_id, 1);
            else
                cpvr_task_create(new_task.schedule_id);
        } else
            goto ret_error;
    }
    return 0;

ret_error:
    if (object) json_object_delete(object);
    return -1;

}

static int JsePVRCancelWrite(const char* aFieldParam, char* value, int len)
{
	printf("_____tanf test:JsePVRCancelWrite,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    struct json_object* object = NULL;
    cpvr_task_live_t task_live;
    cpvr_task_info_t new_task, tmp_task;

    if (value == NULL) return -1;
    LogJseDebug("param value is %s\n", value);

    object = (struct json_object*)json_tokener_parse_string(value);
    if (object) {
        char param_str[256] = {0};

        memset(&new_task, 0, sizeof(new_task));
        if (param_string_get(object, "schedule_id", new_task.schedule_id, sizeof(new_task.schedule_id)-1) == -1)
            goto ret_error;
        json_object_delete(object);

        if (cpvr_list_task_get_by_schedule_id(new_task.schedule_id, &tmp_task, &task_live) == 0) {
            if (tmp_task.task_state == WILL_START_IN_ONE_MIUNTES
                || tmp_task.task_state == PENDING_NO_CONFLICT_STATE
                || tmp_task.task_state == PENDING_WITH_CONFLICT_STATE
                || tmp_task.task_state == IN_PROGRESS_WITH_ERROR_STATE
                || tmp_task.task_state == IN_PROGRESS_STATE
                || tmp_task.task_state == IN_PROGRESS_INCOMPLETE_STATE) {
                if (task_is_running(&tmp_task))
                    mid_record_close(task_live.index, tmp_task.file_id);

                cpvr_list_task_inactive(tmp_task.schedule_id);
                cpvr_task_state_set(tmp_task.schedule_id, CANCELLED_STATE, TASK_OK);
                return 0;
            } else {
                LogJseError("The task %s state is %d, can't stop task.\n", tmp_task.schedule_id, tmp_task.task_state);
                goto ret_error;
            }
        } else {
            LogJseError("Can't find the task %s.\n", new_task.schedule_id);
            goto ret_error;
        }

    }

ret_error:
    if (object) json_object_delete(object);
    return -1;
}

static int JsePVRTaskModifyWrite(const char* aFieldParam, char* value, int len)
{
	printf("_____tanf test:JsePVRTaskModifyWrite,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    time_t tm_start = 0;
    time_t tm_end = 0;
    char param_str[256] = {0};
    char schedule_id[128] = {0};

    cpvr_task_info_t task_info;
    struct json_object* object = NULL;

    if (value) {
        object = (struct json_object*)json_tokener_parse_string(value);
        if (object) {
            if(param_string_get(object, "schedule_id", schedule_id, sizeof(schedule_id)-1) != 0) {
                json_object_delete(object);
                return -1;
            }

            if (param_string_get(object, "starttime",  param_str, sizeof(param_str)-1) == 0) {
                tm_start = timeStringToSecNum(param_str);
            }
            if (param_string_get(object, "endtime",  param_str, sizeof(param_str)-1) == 0) {
                tm_end = timeStringToSecNum(param_str);
            }

            cpvr_task_time_adjust(schedule_id, tm_start, tm_end);
            json_object_delete(object);
        }
    }

    return 0;
}

static int JsePVRGetFileURLRead(const char* aFieldParam, char* value, int len)
{
	printf("_____tanf test:JsePVRGetFileURLRead1,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    cpvr_task_info_t info;
    char schedule_id[128] = {0};
    struct json_object* object = NULL;

    if (aFieldParam) {
        object = (struct json_object*)json_tokener_parse_string(aFieldParam);
        if (object) {
            if(param_string_get(object, "schedule_id", schedule_id, sizeof(schedule_id)-1) == -1){
                json_object_delete(object);
                return -1;
            }
            json_object_delete(object);

            if (cpvr_list_task_get_by_schedule_id(schedule_id, &info, NULL) == 0) {
                if(info.file_id != 0)
                    sprintf(value, "{\"fileURL\":\"file:///%x\",\"schedule_id\":\"%s\"}", info.file_id, schedule_id);
                else
                    sprintf(value, "{\"fileURL\":\"\",\"schedule_id\":\"%s\"}", schedule_id);

                LogJseDebug("return value is %s\n", value);
            }
        }
    }
    printf("_____tanf test:JsePVRGetFileURLRead2,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    return 0;
}

static int JsePVRSyncStatClearWrite(const char* aFieldParam, char* value, int len)
{
	printf("_____tanf test:JsePVRSyncStatClearWrite,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    struct json_object* object = NULL;

    int need_sync = 0;
    char* schedule_id_str = NULL;

    if (value == NULL) return -1;
    LogJseDebug("param value is %s\n", value);

    schedule_id_str = (char*)MEM_ALLOC(strlen(value) + 1);
    object = (struct json_object*)json_tokener_parse_string(value);
    if (object) {
        char param_str[256] = {0};

        if (schedule_id_str && param_string_get(object, "schedule_id", schedule_id_str, sizeof(schedule_id_str)-1) == 0) {
            cpvr_task_sync_flag_clear(schedule_id_str, -1);
        } else if (param_string_get(object, "needSync", param_str, sizeof(param_str)-1) == 0) {
            need_sync = atoi(param_str);
            cpvr_task_sync_flag_clear(NULL, need_sync);
        }

        json_object_delete(object);
    }

    return 0;
}


static int JsePVRTaskCountRead(const char* aFieldParam, char* value, int len)
{
	printf("_____tanf test:JsePVRTaskCountRead1,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    int i_cnt = 0;

    cpvr_task_filter_param_t param;

    struct json_object* object = NULL;

    if (aFieldParam == NULL) return -1;
    LogJseDebug("param value is %s\n", aFieldParam);

    memset(&param, 0, sizeof(param));
    object = (struct json_object*)json_tokener_parse_string(aFieldParam);
    if (object) {
        char param_str[256] = {0};

        cpvr_task_info_t task_info;

        param_string_get(object, "schedule_id", param.schedule_id, sizeof(param.schedule_id)-1);

        if (param.schedule_id[0] != 0 && cpvr_list_task_get_by_schedule_id(param.schedule_id, &task_info, NULL) == 0
            && (task_info.task_type == REC_TYPE_PERIOD_MAIN || task_info.task_type == REC_TYPE_SERIAL_MAIN)) {
            if (param_string_get(object, "subScheduleState", param_str, sizeof(param_str)-1) == 0) {
                if(strncmp(param_str, "[]", 2) == 0 || strncmp(param_str, "\"\"", 2) == 0 || param_str[0] == 0)
                    param.task_state[0] = UNINITED_STATE;
                else {
                    cpvr_task_state_parse((int *)param.task_state, sizeof(param.task_state)/sizeof(param.task_state[0]), param_str, '"');
                }
            } else {
                param.task_state[0] = UNINITED_STATE;
            }

            strcpy(param.main_task_id, param.schedule_id);
        } else {
            if (param_string_get(object, "state", param_str, sizeof(param_str)-1) == 0)
            {
                if(strncmp(param_str, "[]", 2) == 0 || strncmp(param_str, "\"\"", 2) == 0 || param_str[0] == 0)
                    param.task_state[0] = UNINITED_STATE;
                else {
                    cpvr_task_state_parse((int *)param.task_state, sizeof(param.task_state)/sizeof(param.task_state[0]), param_str, '"');
                }
            } else {
                param.task_state[0] = UNINITED_STATE;
            }
        }

        if (param_string_get(object, "startTime", param_str, sizeof(param_str)-1) == 0) {
            if (strlen(param_str) != 14) {
                LogJseDebug("start time is %s\n", param_str);
                json_object_delete(object);
                return -1;
            }
            param.start_time = timeStringToSecNum(param_str);
        }
        if (param_string_get(object, "endTime", param_str, sizeof(param_str)-1) == 0) {
            if (strlen(param_str) != 14){
                LogJseDebug("end time is %s\n", param_str);
                json_object_delete(object);
                return -1;
            }
            param.end_time = timeStringToSecNum(param_str);
        }

        if (param_string_get(object, "recordType", param_str, sizeof(param_str)-1) == 0) {
            if(strncmp(param_str, "\"\"", 2) == 0 || param_str[0] == 0)
                param.task_type[0] = REC_TYPE_UKNOW;
            else {
                cpvr_task_type_parse((int *)param.task_type, sizeof(param.task_type)/sizeof(param.task_type[0]), param_str, '"');
            }
        }

        if (param_string_get(object, "needSync", param_str, sizeof(param_str)-1) == 0) {
            param.sync_flag = atoi(param_str);
        }

        if (param_string_get(object, "contentPath", param_str, sizeof(param_str)-1) == 0) {
            if (strncmp(param_str, "DVB", 3) == 0)
                param.content_source = DVB_PVR;
            else if (strncmp(param_str, "IP", 2) == 0)
                param.content_source = IPTV_PVR;
        }
        json_object_delete(object);
        param.sync_flag = SYNC_FLAG_ALL;
        i_cnt = cpvr_list_get(&param, 0, 0, NULL);
    }

    sprintf(value, "%d", i_cnt);

    LogJseDebug("return value is %s\n", value);
    printf("_____tanf test:JsePVRTaskCountRead2,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    return 0;
}

static int JsePVRTaskListRead(const char* aFieldParam, char* value, int len)
{
	printf("_____tanf test:JsePVRTaskListRead1,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    int i_total = 0, i_pos = 0, i_count = 0;

    cpvr_task_filter_param_t param;

    struct json_object* object = NULL;
    struct _copied_hdr_t hdr = TAILQ_HEAD_INITIALIZER(hdr);

    if (aFieldParam == NULL) return -1;
    LogJseDebug("param value is %s\n", aFieldParam);

    memset(&param, 0, sizeof(param));
    object = (struct json_object*)json_tokener_parse_string(aFieldParam);
    if (object) {
        char param_str[256] = {0};

        cpvr_task_info_t task_info;

        if(param_string_get(object, "position", param_str, sizeof(param_str)-1) == 0) {
            i_pos = atoi(param_str);
        }

        if(param_string_get(object, "count", param_str, sizeof(param_str)-1) == 0) {
            i_count = atoi(param_str);
        }

        param_string_get(object, "prog_id", param.prog_id, sizeof(param.prog_id)-1);
        param_string_get(object, "channel_num", param.channel_num, sizeof(param.channel_num)-1);
        param_string_get(object, "schedule_id", param.schedule_id, sizeof(param.schedule_id)-1);

        if (param_string_get(object, "startTime", param_str, sizeof(param_str)-1) == 0) {
            if (strlen(param_str) != 14){
                LogJseDebug("start time is %s\n", param_str);
                json_object_delete(object);
                return -1;
            }
            param.start_time = timeStringToSecNum(param_str);
        }
        if (param_string_get(object, "endTime", param_str, sizeof(param_str)-1) == 0) {
            if (strlen(param_str) != 14){
                LogJseDebug("end time is %s\n", param_str);
                json_object_delete(object);
                return -1;
            }
            param.end_time = timeStringToSecNum(param_str);
        }

        if (param.schedule_id[0] != 0 && cpvr_list_task_get_by_schedule_id(param.schedule_id, &task_info, NULL) == 0
            && (task_info.task_type == REC_TYPE_PERIOD_MAIN || task_info.task_type == REC_TYPE_SERIAL_MAIN)) {
            if (param_string_get(object, "subScheduleState", param_str, sizeof(param_str)-1) == 0) {
                if(strncmp(param_str, "[]", 2) == 0 || strncmp(param_str, "\"\"", 2) == 0 || param_str[0] == 0)
                    param.task_state[0] = UNINITED_STATE;
                else {
                    cpvr_task_state_parse((int *)param.task_state, sizeof(param.task_state)/sizeof(param.task_state[0]), param_str, '"');
                }
            } else {
                param.task_state[0] = UNINITED_STATE;
            }

            strcpy(param.main_task_id, param.schedule_id);
        } else {
            if (param_string_get(object, "state", param_str, sizeof(param_str)-1) == 0) {
                if(strncmp(param_str, "[]", 2) == 0 || strncmp(param_str, "\"\"", 2) == 0 || param_str[0] == 0)
                    param.task_state[0] = UNINITED_STATE;
                else {
                    cpvr_task_state_parse((int *)param.task_state, sizeof(param.task_state)/sizeof(param.task_state[0]), param_str, '"');
                }
            } else {
                param.task_state[0] = UNINITED_STATE;
            }
        }

        if (param_string_get(object, "recordType", param_str, sizeof(param_str)-1) == 0) {
            if(strncmp(param_str, "\"\"", 2) == 0 || param_str[0] == 0)
                param.task_type[0] = REC_TYPE_UKNOW;
            else {
                cpvr_task_type_parse((int *)param.task_type, sizeof(param.task_type)/sizeof(param.task_type[0]), param_str, '"');
            }
        }

        if (param_string_get(object, "needSync", param_str, sizeof(param_str)-1) == 0) {
            param.sync_flag = atoi(param_str);
        }
        else param.sync_flag = SYNC_FLAG_ALL;

        if (param_string_get(object, "contentPath", param_str, sizeof(param_str)-1) == 0) {
            if (strncmp(param_str, "DVB", 3) == 0)
                param.content_source = DVB_PVR;
            else if (strncmp(param_str, "IP", 2) == 0)
                param.content_source = IPTV_PVR;
        }
        json_object_delete(object);

        if (param.sync_flag == SYNC_FLAG_DELETED)
            param.delete_flg = 1;

        i_total = cpvr_list_get(&param, i_pos, i_count, &hdr);
        if (i_total > 0) {
            int  retNum = 0;
            char tmpBuf[4000] = {0};
            retNum = cpvr_json_build(tmpBuf, &hdr);
            sprintf(value, "{\"Schedules_count\":%d", retNum);
            if (retNum > 0) {
                strcat(value, tmpBuf);
            }
            cpvr_copied_list_release(&hdr);
        } else {
            sprintf(value, "{\"Schedules_count\":%d", i_total);
        }
        json_add_rightbrace(value);
        LogJseDebug("return value is %s\n", value);
    }
printf("_____tanf test:JsePVRTaskListRead2,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    return 0;
}

static int JsePVRTaskDeleteWrite(const char* aFieldParam, char* value, int len)
{
	printf("_____tanf test:JsePVRTaskDeleteWrite,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    int i_total = 0;

    cpvr_task_filter_param_t param;

    struct json_object* object = NULL;
    struct _copied_hdr_t hdr = TAILQ_HEAD_INITIALIZER(hdr);

    char delete_flag[12] = {0};
    char subtask_state[256] = {0};

    if (value == NULL) return -1;
    LogJseDebug("param value is %s\n", value);

    memset(&param, 0, sizeof(param));
    object = (struct json_object*)json_tokener_parse_string(value);
    if (object) {
        char param_str[256] = {0};

        param_string_get(object, "schedule_id", param.schedule_id, sizeof(param.schedule_id)-1);
        param_string_get(object, "deleteRecordFile", delete_flag, sizeof(delete_flag)) ;

        if (param_string_get(object, "contentPath", param_str, sizeof(param_str)-1) == 0) {
            if (strncmp(param_str, "DVB", 3) == 0)
                param.content_source = DVB_PVR;
            else if (strncmp(param_str, "IP", 2) == 0)
                param.content_source = IPTV_PVR;
        }

        if (param_string_get(object, "lasttime", param_str, sizeof(param_str)-1) == 0) {
            if (strlen(param_str) != 14){
                LogJseDebug("last time is %s\n", param_str);
                json_object_delete(object);
                return -1;
            }
            param.last_time = timeStringToSecNum(param_str);
        }

        if (param_string_get(object, "state", param_str, sizeof(param_str)-1) == 0)
            param.task_state[0] = cpvr_task_state_str2enum(param_str);
        else
            param.task_state[0] = UNINITED_STATE;

        if (param_string_get(object, "subScheduleState", subtask_state, sizeof(subtask_state)-1) != 0)
            strcpy(subtask_state, "[\"UNINITED_STATE\"]");

        json_object_delete(object);
        param.sync_flag = SYNC_FLAG_ALL;
        i_total = cpvr_list_get(&param, 0, 0, &hdr);
        if (i_total > 0) {
            _copied_node_t* p_node = NULL;

            TAILQ_FOREACH(p_node, &hdr, list) {
                cpvr_task_delete(&p_node->info, subtask_state, atoi(delete_flag));
            }

            cpvr_copied_list_release(&hdr);
        }

    }

    return 0;
}

static int JsePVRTaskDeleteProWrite(const char* aFieldParam, char* value, int len)
{
	printf("_____tanf test:JsePVRTaskDeleteProWrite,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    int progress = 0;
    cpvr_task_filter_param_t param;
    struct json_object* object = NULL;

    if (aFieldParam == NULL) return -1;
    LogJseDebug("param value is %s\n", aFieldParam);

    memset(&param, 0, sizeof(param));
    object = (struct json_object*)json_tokener_parse_string(aFieldParam);
    if (object) {
        char param_str[256] = {0};

        cpvr_task_info_t task_info;

        param_string_get(object, "schedule_id", param.schedule_id, sizeof(param.schedule_id)-1);
        json_object_delete(object);

        if (param.schedule_id[0] != 0 && cpvr_list_task_get_by_schedule_id(param.schedule_id, &task_info, NULL) == 0
            && (task_info.task_type == REC_TYPE_PERIOD_MAIN || task_info.task_type == REC_TYPE_SERIAL_MAIN)) {
            progress = getDeletePvrFileProgressByFileid(task_info.file_id);
            if (-2 == progress)
                sprintf(value, "{\"formatProgress\":\"N/A\",\"schedule_id\":\"%s\"}", param.schedule_id);
            else
                sprintf(value, "{\"formatProgress\":\"%d%\",\"schedule_id\":\"%s\"}", progress, param.schedule_id);
            LogJseDebug("return value is %s\n", value);
        } else {
            LogJseError("no this schedule_id[%s]\n", param.schedule_id);
            return -1;
        }
    }
    return 0;
}


// 此函数没有被调用，原本在CpvrJsCall.cpp，由于调用了函数cpvr_porting_task_get_list（更名为JsePVRTaskListRead），固移到这里，原处未删除，只注释
int dlnaGetPvrInfoByScheduleID(char* ScheduleID, char* pvrinfo)
{
    std::string param = "{\"schedule_id\":\"";
    param += ScheduleID;
    param += "\"}";
    return JsePVRTaskListRead(param.c_str(), pvrinfo, 0);
}

/*************************************************
Description: 初始化华为PVR.Schedule模块配置定义的接口，由JseHWVR.cpp调用.这里的字段全部直接注册在PVR.下
Input: JseHWPVR的指针
Return: 无
 *************************************************/
int JseHWScheduleInit(JseGroupCall* callPVR)
{
    JseCall* call;

    // C20 regist
    call = new JseFunctionCall("Schedule.Update", 0, JsePVRUpdateWrite);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("Schedule.Cancel", 0, JsePVRCancelWrite);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("Schedule.TaskModify", 0, JsePVRTaskModifyWrite);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("Schedule.GetFileURL", JsePVRGetFileURLRead, 0);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("ScheduleSyncStat.Clear", 0, JsePVRSyncStatClearWrite);
    callPVR->regist(call->name(), call);


    // C20 regist , 以下字段目前调用同一函数JsePVRTaskListRead，为可读和不重复实现函数起见，放在一起不分多文件。
    call = new JseFunctionCall("ScheduleList.Count", JsePVRTaskCountRead, 0);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("SubScheduleList.Count", JsePVRTaskCountRead, 0);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("Schedule.GetCount.byRecordType", JsePVRTaskCountRead, 0);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("ScheduleListByTime.Count", JsePVRTaskCountRead, 0);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("needSyncScheduleList.Count", JsePVRTaskCountRead, 0);
    callPVR->regist(call->name(), call);

    //C20 regist ，以下字段目前调用同一函数JsePVRTaskListRead，为可读和不重复实现函数起见，放在一起不分多文件。
    call = new JseFunctionCall("SubScheduleList.Get", JsePVRTaskListRead, 0);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("TaskConflictList.Get", JsePVRTaskListRead, 0);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("ScheduleList.GetAllByTime", JsePVRTaskListRead, 0);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("ScheduleList.Get", JsePVRTaskListRead, 0);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("ScheduleList.GetByProgID", JsePVRTaskListRead, 0);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("ScheduleList.GetByScheduleID", JsePVRTaskListRead, 0);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("needSyncScheduleList.Get", JsePVRTaskListRead, 0);
    callPVR->regist(call->name(), call);

    //C20 regist ，以下字段目前调用同一函数JsePVRTaskDeleteWrite
    call = new JseFunctionCall("Schedule.Delete", 0, JsePVRTaskDeleteWrite);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("Schedule.Delete.ByState", 0, JsePVRTaskDeleteWrite);
    callPVR->regist(call->name(), call);
    call = new JseFunctionCall("Schedule.Delete.Before", 0, JsePVRTaskDeleteWrite);
    callPVR->regist(call->name(), call);
    //C20 regist  JsePVRTaskDeleteProWrite
    call = new JseFunctionCall("Schedule.Delete.Progress", 0, JsePVRTaskDeleteProWrite);
    callPVR->regist(call->name(), call);

    return 0;
}

