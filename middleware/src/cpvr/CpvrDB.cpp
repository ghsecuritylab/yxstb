#include "CpvrAssertions.h"
#include "CpvrAuxTools.h"
#include "CpvrDB.h"

#include "sqlite3/sqlite3.h"
#include "config/pathConfig.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>

#define  CPVR_TABLE_V1 ((char*)"CREATE TABLE [cpvr_record] ([schedule_id] VARCHAR(64)  UNIQUE NOT NULL PRIMARY KEY, \
    [file_id] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL, \
    [bitrate] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL, \
    [content_source] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL, \
    [channel_num] VARCHAR(24)  NULL, \
    [channel_name] VARCHAR(512)  NULL, \
    [task_state] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL, \
    [last_task_state] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL, \
    [error_code] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL, \
    [prog_id] VARCHAR(64)  NULL, \
    [prog_title] VARCHAR(512)  NULL, \
    [main_task_start_time] VARCHAR(24)  NOT NULL, \
    [main_task_end_time] VARCHAR(24)  NOT NULL, \
    [start_time] VARCHAR(24)  NOT NULL, \
    [end_time] VARCHAR(24)  NOT NULL, \
    [real_start_time] VARCHAR(24)  NULL, \
    [real_end_time] VARCHAR(24)  NULL, \
    [time_stamp] VARCHAR(24)  NULL, \
    [package_id] VARCHAR(24)  NULL, \
    [package_name] VARCHAR(512)  NULL, \
    [protection_mode] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL, \
    [priority] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL, \
    [sync_flag] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL,\
    [parental_rating] VARCHAR(24)  NULL, \
    [task_type] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL, \
    [serial_episode_type] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL, \
    [serial_sn] VARCHAR(32)  NULL, \
    [serial_season_id] VARCHAR(32)  NULL, \
    [serial_prog_sn] VARCHAR(32)  NULL, \
    [serial_name] VARCHAR(512)  NULL, \
    [serial_record_rule] VARCHAR(512)  NULL, \
    [program_description] VARCHAR(2048)  NULL, \
    [period_type] VARCHAR(32)  NULL, \
    [period_every_week] VARCHAR(32)  NULL, \
    [period_every_month] VARCHAR(32)  NULL, \
    [period_name] VARCHAR(512)  NULL, \
    [delete_flg] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL, \
    [main_task_id] VARCHAR(512)  NULL, \
    [pltv_tag] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL, \
    [auto_delete] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL, \
    [reserved_duration] INTEGER DEFAULT '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''0''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' NOT NULL, \
    [subtask_num_total] INTEGER DEFAULT '''0''' NULL, \
    [subtask_num_unfinished] INTEGER DEFAULT '''0''' NULL)")

const int  current_db_version = 1;

struct sql_syntax {
    char *sql;
    int sql_ver;
};

struct sql_syntax g_sql_history[] = {{CPVR_TABLE_V1, 0}};

static sqlite3  *g_db_handle = NULL;

#define DB_VALID_CHECK() if (!g_db_handle) return -1

namespace Hippo {

static void inline s_db_msg_print(sqlite3* db_handle)
{
    char *error_msg = NULL;

    if (!db_handle) return ;
    error_msg = (char *)sqlite3_errmsg(db_handle);
    if (error_msg)
        LogCpvrError("db error:%s\n", error_msg);
}

static inline int s_sql_exec(sqlite3 *db_handle, const char *sql)
{
    int  i_ret = -1;
    char *err_msg = NULL;

    if (db_handle && sql) {
        LogCpvrDebug("db exec : %s\n", sql);
        i_ret = sqlite3_exec(db_handle, sql, 0, 0, &err_msg);
        if (i_ret != SQLITE_OK && err_msg != NULL)
            LogCpvrError("db exec error : %s\n", err_msg);
    }

    LogCpvrDebug("i_ret = %d\n", i_ret);
    return i_ret;
}

static inline int s_column_index_get(char **result, int column_cnt, const char *column_name)
{
    int i = 0;

    if (result && column_name && column_cnt > 0) {
        for (i = 0; i < column_cnt; i ++) {
            if (result[i] && strncmp(result[i], column_name, strlen(column_name)) == 0) {
                return i;
            }
        }
    }

    return -1;
}

static inline char *s_field_valud_get(char **result, int rows, int column_cnt, const char *column_name)
{
    int index = 0;

    index = s_column_index_get(result, column_cnt, column_name);
    if (index >= 0)
        return result[rows * column_cnt + index];
    else
        return NULL;
}

//return 0:not exist; 1:exists
static inline int s_table_exist(sqlite3 *handle, const char *table_name)
{
    int i_ret = 0, rows = 0, columns = 0;

    char buf[2048] = {0};
    char *err_msg = NULL, **result = NULL;
    char sql[] = "SELECT * FROM sqlite_master WHERE name == \'%s\' ";

    if (handle != NULL && table_name != NULL) {
        sprintf(buf, sql, table_name);

        if (sqlite3_get_table(handle, buf, &result, &rows, &columns, &err_msg) == SQLITE_OK) {
            if (rows > 0) {
                LogCpvrDebug("table %s is exists.\n", table_name);
                i_ret = 1;
            } else {
                LogCpvrDebug("table %s not exists.\n", table_name);
                i_ret = 0;
            }
        } else {
            if (err_msg) LogCpvrError("db error:%s\n", err_msg);
            i_ret = -1;
        }
    }

    sqlite3_free_table(result);

    LogCpvrDebug("i_ret :%d\n", i_ret);
    return i_ret;
}

static void s_table_update(sqlite3* handle, int ver)
{
    int i = 0;
    int update_cnt = sizeof(g_sql_history) / sizeof(struct sql_syntax);

    LogCpvrDebug("update cnt is %d\n", update_cnt);
    for(i = 0; i < update_cnt; i++) {
        if(handle != NULL && g_sql_history[i].sql_ver > ver)
            s_sql_exec(handle, g_sql_history[i].sql);
    }
}

static int s_db_update_string(const char *table_name, const char *key_name, const char *key, const char *field_name, const char *value)
{
    char sql_buf[2048] = {0};

    DB_VALID_CHECK();
    snprintf(sql_buf, sizeof(sql_buf)-1, "UPDATE %s SET %s=\'%s\' WHERE %s==\'%s\'", table_name, field_name, value, key_name, key);
    return s_sql_exec(g_db_handle, (const char *)sql_buf);
}

static int s_db_update_int(const char *table_name, const char *key_name, const char *key, const char *field_name, int value)
{
    char sql_buf[2048] = {0};

    DB_VALID_CHECK();
    snprintf(sql_buf, sizeof(sql_buf)-1, "UPDATE %s SET %s=%d WHERE %s==\'%s\'", table_name, field_name, value, key_name, key);
    return s_sql_exec(g_db_handle, (const char *)sql_buf);
}

static int s_db_ver_get(sqlite3* handle) //返回数据库的版本，如果刚创建则返回-1；所有表都要重建
{

    char **result = NULL;
    char *err_msg = NULL, *value = NULL;
    const char *sql = "SELECT max(version) FROM db_ver";

    int i_ret = -1, rows = 0, columns = 0;

    if (handle != NULL && s_table_exist(handle, "db_ver") == 1) {
        if (sqlite3_get_table(handle, sql, &result, &rows, &columns, &err_msg) == SQLITE_OK && rows > 0) {
            value = s_field_valud_get(result, 1, columns, "max(version)");
            if (value) {
                i_ret = atoi(value);
                LogCpvrDebug("db version number is %d.\n", i_ret);
            } else {
                LogCpvrError("failed to get max(version)");
                i_ret = -1;
            }
        } else {
            if (err_msg) LogCpvrError("db error:%s\n", err_msg);
            i_ret = -1;
        }

        sqlite3_free_table(result);
    }

    LogCpvrDebug("i_ret: %d\n", i_ret);
    return i_ret;
}

static void s_db_update(sqlite3* handle)
{
    int version = 0;
    char sql[128] = {0};
    char ver_table_sql[] = "CREATE TABLE db_ver(version INTEGER)";
    char ver_table_update[] = "INSERT INTO db_ver(version) VALUES(%d)"; //当前版本

    if (handle != NULL) {
        version = s_db_ver_get(handle);
        if (version >= 0) {
            if (version != current_db_version) {
                s_table_update(handle, version);
            }
        } else {
            s_sql_exec(handle, ver_table_sql);
            s_table_update(handle, -1);
        }

        sprintf(sql, ver_table_update, current_db_version);
        s_sql_exec(handle, sql);
    }
}

int db_cpvr_ability_set(char *buf)
{
    int i_ret = -1;
    char sql_buf[512] = {0};
    char ability_table_create[] = "CREATE TABLE cpvr_ability(ability_info VARCHAR(128)  NULL)";
    char ability_table_insert[] = "INSERT INTO cpvr_ability(ability_info) VALUES(%s)";
    char ability_table_update[] = "UPDATE cpvr_ability SET ability_info=\'%s\'";

    if (g_db_handle && buf) {
        if (s_table_exist(g_db_handle, "cpvr_ability") == 0) {
            strcpy(sql_buf, ability_table_create);
            i_ret = s_sql_exec(g_db_handle, sql_buf);
            if (i_ret != SQLITE_OK)
                return i_ret;

            sprintf(sql_buf, ability_table_insert, buf);
            i_ret = s_sql_exec(g_db_handle, sql_buf);
        } else {
            sprintf(sql_buf, ability_table_update, buf);
            i_ret = s_sql_exec(g_db_handle, sql_buf);
        }
    }

    LogCpvrDebug("i_ret: %d\n", i_ret);
    return i_ret;
}

int db_cpvr_ability_get(char *buf, int i_len)
{
    int i_ret = -1, rows = 0, columns = 0;
    char sql_buf[512] = {0};
    char ability_table_get[] = "SELECT ability_info FROM cpvr_ability";

    char **result = NULL;
    char *err_msg = NULL, *value = NULL;

    if (g_db_handle && buf && s_table_exist(g_db_handle, "cpvr_ability") == 1) {
        if (sqlite3_get_table(g_db_handle, ability_table_get, &result, &rows, &columns, &err_msg) == SQLITE_OK && rows > 0) {
            value = s_field_valud_get(result, 1, columns, "ability_info");
            if (value) {
                strncpy(buf, value, strlen(value) > i_len ? i_len - 1 : strlen(value));
                i_ret = 0;
            } else {
                LogCpvrError("failed to get ability_info");
                i_ret = -1;
            }
        }
    }

    LogCpvrDebug("i_ret: %d\n", i_ret);
    return i_ret;
}

int db_open(char *dir)
{
    int i_ret = -1;
    char db_path[] = DEFAULT_MODULE_CPVR_DATAPATH"/cpvr.s3db";

    if (g_db_handle) {
        LogCpvrDebug("db already opened\n");
        return i_ret;
    }

    //sprintf(db_path, "%s/cpvr.s3db", dir);
    mode_t oldMask = umask(0077);
    i_ret = sqlite3_open(db_path, &g_db_handle);
    umask(oldMask);
    if (i_ret == SQLITE_OK)
        s_db_update(g_db_handle);
    else
        s_db_msg_print(g_db_handle);

    LogCpvrDebug("i_ret: %d\n", i_ret);
    return i_ret;
}


int db_close()
{
    if (g_db_handle) {
        sqlite3_close(g_db_handle);
        g_db_handle = NULL;
    }
}

int db_cpvr_task_info_read(add_task_callback_func add_task_callback)
{
    int i = 0, j = 0;
    int rows = 0, columns = 0;

    char *error_msg = NULL;
    char **result = 0;

    char *value = NULL;
    char sql[] = "SELECT * FROM cpvr_record";

    cpvr_task_info_t  info;

    DB_VALID_CHECK();

    if (sqlite3_get_table(g_db_handle, sql, &result, &rows, &columns, &error_msg) == SQLITE_OK) {
        LogCpvrDebug("Has %d rows.\n",rows);
        for (i =1; i <= rows; i ++) {
            memset(&info, 0, sizeof(info));

            value = s_field_valud_get(result, i, columns, "schedule_id");
            if (value)
                strcpy(info.schedule_id, value);
            else
                continue;

            value = s_field_valud_get(result, i, columns, "file_id");
            if (value)
                info.file_id = atoi(value);
            value = s_field_valud_get(result, i, columns, "bitrate");
            if (value)
                info.bitrate = atoi(value);
            value = s_field_valud_get(result, i, columns, "task_type");
            if (value)
                info.task_type =(CPVR_TASK_TYPE)atoi(value);
            value = s_field_valud_get(result, i, columns, "channel_num");
            if (value)
                strcpy(info.channel_num, value);
            value = s_field_valud_get(result, i, columns, "channel_name");
            if (value)
                strcpy(info.channel_name, value);
            value = s_field_valud_get(result, i, columns, "task_state");
            if (value)
                info.task_state =(CPVR_TASK_STATE)atoi(value);
            value = s_field_valud_get(result, i, columns, "last_task_state");
            if (value)
                info.last_task_state =(CPVR_TASK_STATE) atoi(value);
            value = s_field_valud_get(result, i, columns, "error_code");
            if (value)
                info.error_code =(CPVR_TASK_ERROR_CODE) atoi(value);
            value = s_field_valud_get(result, i, columns, "delete_flg");
            if (value)
                info.delete_flg = atoi(value);
            value = s_field_valud_get(result, i, columns, "prog_id");
            if (value)
                strcpy(info.prog_id, value);
            value = s_field_valud_get(result, i, columns, "prog_title");
            if (value)
                strcpy(info.prog_title, value);
            value = s_field_valud_get(result, i, columns, "start_time");
            if (value)
                info.start_time = atoi(value);
            value = s_field_valud_get(result, i, columns, "end_time");
            if (value)
                info.end_time = atoi(value);
            value = s_field_valud_get(result, i, columns, "real_start_time");
            if (value)
                info.real_start_time = atoi(value);
            value = s_field_valud_get(result, i, columns, "real_end_time");
            if (value)
                info.real_end_time = atoi(value);
            value = s_field_valud_get(result, i, columns, "time_stamp");
            if (value)
                info.time_stamp = atoi(value);
            value = s_field_valud_get(result, i, columns, "package_id");
            if (value)
                strcpy(info.package_id, value);
            value = s_field_valud_get(result, i, columns, "package_name");
            if (value)
                strcpy(info.package_name, value);
            value = s_field_valud_get(result, i, columns, "protection_mode");
            if (value)
                info.protection_mode =(PROTECTION_MODE) atoi(value);
            value = s_field_valud_get(result, i, columns, "priority");
            if (value)
                info.priority = (CPVR_TASK_PRIORITY)atoi(value);
            value = s_field_valud_get(result, i, columns, "sync_flag");
            if (value)
                info.sync_flag = atoi(value);
            value = s_field_valud_get(result, i, columns, "parental_rating");
            if (value)
                strcpy(info.parental_rating, value);
            value = s_field_valud_get(result, i, columns, "content_source");
            if (value)
                info.content_source =(CPVR_CONTENT_SRC_TYPE) atoi(value);
            value = s_field_valud_get(result, i, columns, "serial_episode_type");
            if (value)
                info.serial_episode_type = atoi(value);
            value = s_field_valud_get(result, i, columns, "serial_sn");
            if (value)
                strcpy(info.serial_sn, value);
            value = s_field_valud_get(result, i, columns, "serial_season_id");
            if (value)
                strcpy(info.serial_season_id, value);
            value = s_field_valud_get(result, i, columns, "serial_prog_sn");
            if (value)
                strcpy(info.serial_prog_sn, value);
            value = s_field_valud_get(result, i, columns, "serial_name");
            if (value)
                strcpy(info.serial_name, value);
            value = s_field_valud_get(result, i, columns, "program_description");
            if (value)
                strcpy(info.program_description, value);
            value = s_field_valud_get(result, i, columns, "period_type");
            if (value)
                strcpy(info.period_type, value);
            value = s_field_valud_get(result, i, columns, "period_every_week");
            if (value)
                strcpy(info.period_every_week, value);
            value = s_field_valud_get(result, i, columns, "period_name");
            if (value)
                strcpy(info.period_name, value);
            value = s_field_valud_get(result, i, columns, "serial_record_rule");
            if (value)
                strcpy(info.serial_record_rule, value);
            value = s_field_valud_get(result, i, columns, "main_task_id");
            if (value) {
                int j = 0;
                char *ptr = value;

                while(*ptr != 0 && j<sizeof(info.main_task_id)/sizeof(info.main_task_id[0]))
                {
                    char *p = ptr;
                    ptr = strchr(p, '#');
                    if (ptr) {
                        memset(info.main_task_id[j], 0, sizeof(info.main_task_id[j]));
                        strncpy(info.main_task_id[j], p, ptr-p);
                        ptr ++;
                        j ++;
                    }
                }
            }
            value = s_field_valud_get(result, i, columns, "pltv_tag");
            if (value)
                info.pltv_tag = atoi(value);
            value = s_field_valud_get(result, i, columns, "auto_delete");
            if (value)
                info.auto_delete = atoi(value);
            value = s_field_valud_get(result, i, columns, "reserved_duration");
            if (value)
                info.reserved_duration = atoi(value);
            value = s_field_valud_get(result, i, columns, "subtask_num_total");
            if (value)
                info.subtask_num_total = atoi(value);
            value = s_field_valud_get(result, i, columns, "subtask_num_unfinished");
            if (value)
                info.subtask_num_unfinished = atoi(value);
            value = s_field_valud_get(result, i, columns, "main_task_start_time");
            if (value)
                info.main_task_start_time = atoi(value);
            value = s_field_valud_get(result, i, columns, "main_task_end_time");
            if (value)
                info.main_task_end_time = atoi(value);

            if (add_task_callback)
                add_task_callback(&info);

        }
    } else {
        if (error_msg) LogCpvrError("db error:%s\n", error_msg);
        return -1;
    }

    sqlite3_free_table(result);
    return 0;
}

int db_cpvr_task_info_write(cpvr_task_info_t * p_info)
{
    int  i = 0;
    char sql_buf[4096] = {0};

    const char add_sql[] = "INSERT INTO cpvr_record (schedule_id, file_id, bitrate, content_source, channel_num, channel_name, task_state, last_task_state, \
        error_code, prog_id, prog_title, start_time, end_time, real_start_time, real_end_time, time_stamp, package_id, package_name, protection_mode, \
        priority, sync_flag, parental_rating, task_type, serial_episode_type, serial_sn, serial_season_id, serial_prog_sn, serial_name, program_description, \
        period_type, period_every_week, period_name, main_task_id, pltv_tag, auto_delete, reserved_duration, subtask_num_total, subtask_num_unfinished, serial_record_rule, delete_flg, main_task_start_time, main_task_end_time) \
        VALUES('%s', %d, %d, %d, '%s','%s','%d','%d','%d','%s','%s','%d','%d','%d','%d','%d','%s', '%s','%d','%d','%d','%s','%d','%d','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%d','%d','%d','%d','%s','%d', '%d', '%d')";

    DB_VALID_CHECK();

    if (p_info && p_info->schedule_id[0] != 0) {
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

        sprintf(sql_buf, add_sql, p_info->schedule_id, p_info->file_id, p_info->bitrate, p_info->content_source, p_info->channel_num, p_info->channel_name,
            p_info->task_state, p_info->last_task_state, p_info->error_code, p_info->prog_id, p_info->prog_title, p_info->start_time, p_info->end_time,
            p_info->real_start_time, p_info->real_end_time, p_info->time_stamp, p_info->package_id, p_info->package_name, p_info->protection_mode,
            p_info->priority, p_info->sync_flag, p_info->parental_rating, p_info->task_type, p_info->serial_episode_type,
            p_info->serial_sn, p_info->serial_season_id, p_info->serial_prog_sn, p_info->serial_name, p_info->program_description,
            p_info->period_type, p_info->period_every_week, p_info->period_name, ptr, p_info->pltv_tag, p_info->auto_delete, p_info->reserved_duration,
            p_info->subtask_num_total, p_info->subtask_num_unfinished, p_info->serial_record_rule, p_info->delete_flg, p_info->main_task_start_time, p_info->main_task_end_time);

        while (i<strlen(sql_buf)) {
            if (sql_buf[i] == '\'' && sql_buf[i-1] == '\\')
                sql_buf[i-1] = '\'';
            i++;
        }

        if (ptr) MEM_FREE(ptr);
        return s_sql_exec(g_db_handle, (const char *)sql_buf);
    }
    else return -1;

}

int db_cpvr_task_info_delete(const char *schedule_id)
{
    char sql_buf[2048] = {0};
    const char delete_sql[] = "DELETE FROM cpvr_record WHERE (schedule_id =='%s')";

    DB_VALID_CHECK();

    if (schedule_id) {
        sprintf(sql_buf, delete_sql, schedule_id);
        return s_sql_exec(g_db_handle, (const char *)sql_buf);
    }
    else return -1;
}

int db_cpvr_task_info_update_string(const char *schedule_id, const char *field_name, char *value)
{
    return s_db_update_string("cpvr_record", "schedule_id", schedule_id, field_name, value);
}

int db_cpvr_task_info_update_int(const char *schedule_id, const char *field_name, int value)
{
    return s_db_update_int("cpvr_record", "schedule_id", schedule_id, field_name, value);
}

}

