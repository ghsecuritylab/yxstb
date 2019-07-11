//============================================================================
// Name        : param_map.cpp
// Author      : chenbaoqiang
// Version     :
// Copyright   : hybroad.com
//============================================================================

#include <iostream>
#include <vector>
#include "hash_func.h"
using namespace std;
using std::vector;
#include "param_map.h"


#include "json_public.h"
#include "json_object.h"
extern "C"{
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
};
#include "local_service.h"
#include "nm_dbg.h"

#define MODULE_NAME_LEN 64

typedef struct param_node{
	int hash_node;
	char *param_name;
}param_node_t;

typedef std::vector<param_node_t*> param_vector_t;

typedef struct module_node{
	char module_name[MODULE_NAME_LEN + 1];
	void *param_vector;
}module_node_t;

void * module_param_get(char *param) {

	const char *module_name = NULL;
	const char *param_name = NULL;
	struct json_object *parse_obj = NULL,*in_obj = NULL, *sub_obj = NULL, *param_obj = NULL;
	param_node_t *p_node = NULL;
	module_node_t *p_module = NULL;
	int i,param_cnt = 0;
	param_vector_t *p_vector;

	nm_track();
	//if(param)
		//file_dump("/data/param.txt", param, strlen(param));
	parse_obj = json_tokener_parse(param);
	if (!parse_obj){
		nm_msg("error!,param is invalid\n");
		if(param)
			nm_msg("error!,param is %s\n", param);
	    return NULL;
	}
	p_module = (module_node_t *)malloc(sizeof(module_node_t));
	in_obj = json_object_object_get(parse_obj, "module_name");
	if (!in_obj){
	   json_object_put(parse_obj);
	   goto error;
	}
	module_name = json_object_get_string(in_obj);
	if(!module_name){
		//json_object_put(in_obj);
		json_object_put(parse_obj);
		goto error;
	}
	strncpy(p_module->module_name, module_name, MODULE_NAME_LEN);
	nm_msg("module name is %s\n", p_module->module_name);
	//json_object_put(in_obj);
	#if 1
	in_obj = json_object_object_get(parse_obj, "params");
	if(in_obj){
		param_cnt = json_get_object_array_length(in_obj);
		p_vector = new param_vector_t;
		for(i = 0; i < param_cnt; i ++){
			sub_obj = json_object_get_array_idx(in_obj, i);
			if(sub_obj){
				param_obj = json_object_object_get(sub_obj, "param");
				if(param_obj){
					param_name = json_object_get_string(param_obj);
					if(!param_name){
						//json_object_put(param_obj);
			      			//json_object_put(sub_obj);
						continue;
					}
					p_node = (param_node_t *)malloc(sizeof(param_node_t));
					p_node->param_name = strdup(param_name);
					p_node->hash_node = ILibGetHashValueEx(p_node->param_name, strlen(p_node->param_name), 0);
		  			p_vector->push_back(p_node);
					//json_object_put(param_obj);
				}
			}
		}
	}
	#endif
	nm_msg_level(LOG_DEBUG, "release parse_obj\n");
	json_object_put(parse_obj);
	p_module->param_vector = (void *)p_vector;
	return (void *)p_module;
error:
	return NULL;
}

int param_check(void *module_handle, char *param)
{
	module_node_t *p_module;
	param_vector_t *p_vector;
	int hash_value = 0;
	param_vector_t::iterator it;

	if(!module_handle || !param){
		return -1;
	}
	p_module = (module_node_t *)module_handle;
	hash_value = ILibGetHashValueEx(param, strlen(param), 0);
	p_vector = (param_vector_t *)p_module->param_vector;
	for (it = p_vector->begin(); it != p_vector->end(); ++it) {
	    if ((*it)->hash_node == hash_value) {
	    	if(strcmp(param, (*it)->param_name) == 0){
				//nm_msg("%s found %s\n", p_module->module_name, param);
	    		return 1;
	    	}
	    }
	}
	return 0;
}

int module_name_check(void *module_handle, char *module_name)
{
	module_node_t *p_module;

	if(!module_handle || !module_name){
		return -1;
	}
	p_module = (module_node_t *)module_handle;
	if(strcmp(module_name, p_module->module_name) == 0){
		return 1;
	}
	return 0;
}

static nm_local_service *service = NULL;
extern "C"{

	int nm_local_param_init()
	{
		if(!service)
			service = nm_local_service::instance_get();
		return 0;
	}
	
	int nm_local_param_set(char *param, local_get_value get, local_set_value set)
	{
		if(service){
			service->nm_local_param_set(param, get, set);
			return 0;
		}
		return -1;
	}

	local_get_value nm_local_param_check_read(char *param)
	{
		if(service){
			return service->nm_local_param_check_read(param);
		}
		return NULL;	
	}

	local_set_value nm_local_param_check_write(char *param)
	{
		if(service){
			return service->nm_local_param_check_write(param);
		}
		return NULL;	
	}
	/*
	本地参数设置的例子
	int main(int argc, char **argv)
	{
		local_get_value value_get = NULL;
		local_set_value value_set = NULL;
		char buf[1204 + 1];
		char *test = "lsdjfsldfkjlsd";

		nm_local_param_init();
		nm_local_param_set("Device.X_00E0FC.LogParaConfiguration.logLevel", test_read, test_write);
		nm_local_param_set("Device.X_00E0FC.LogParaConfiguration.logType", test_read1, NULL);

		value_get = nm_local_param_check_read("Device.X_00E0FC.LogParaConfiguration.logLevel");
		if(value_get) value_get("Device.X_00E0FC.LogParaConfiguration.logLevel", buf, 1024);
		value_set = nm_local_param_check_write("Device.X_00E0FC.LogParaConfiguration.logLevel");
		if(value_set) value_set("Device.X_00E0FC.LogParaConfiguration.logLevel", test);

		value_get = nm_local_param_check_read("Device.X_00E0FC.LogParaConfiguration.logType");
		if(value_get) value_get("Device.X_00E0FC.LogParaConfiguration.logLevel", buf, 1024);
		value_get = nm_local_param_check_write("Device.X_00E0FC.LogParaConfiguration.logType");
		if(value_get) value_set("Device.X_00E0FC.LogParaConfiguration.logLevel", test);
		else printf("logType write is NULL");
		
		return 0;
	}
	
	*/
};
