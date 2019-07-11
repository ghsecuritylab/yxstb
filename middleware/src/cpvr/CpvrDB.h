#ifndef _CPVRDB_H
#define _CPVRDB_H

#include "CpvrTaskManager.h"

/* cpvr_db.h */
namespace Hippo {

typedef void (*add_task_callback_func)(void *arg);

int db_open(char *dir);
int db_close();
int db_cpvr_task_info_read(add_task_callback_func add_task_callback);
int db_cpvr_task_info_write(cpvr_task_info_t * p_info);
int db_cpvr_task_info_update_string(const char *schedule_id, const char *field_name, char *value);
int db_cpvr_task_info_update_int(const char *schedule_id, const char *field_name, int value);
int db_cpvr_task_info_delete(const char *schedule_id);

int db_cpvr_ability_set(char *buf);
int db_cpvr_ability_get(char *buf, int i_len);
}

#endif

