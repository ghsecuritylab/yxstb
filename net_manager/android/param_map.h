/*
 * param_map.h
 *
 *  Created on: 2013Äê11ÔÂ26ÈÕ
 *      Author: Administrator
 */
#ifndef PARAM_MAP_H_
#define PARAM_MAP_H_
#include "local_service.h"

void * module_param_get(char *param);
int module_name_check(void *module_handle, char *module_name);
int param_check(void *module_handle, char *param);
extern "C"{
	int nm_local_param_init();
	int nm_local_param_set(char *param, local_get_value get, local_set_value set);
	local_get_value nm_local_param_check_read(char *param);
	local_set_value nm_local_param_check_write(char *param);
}
#endif
