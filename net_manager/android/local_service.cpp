#include <stdlib.h>
#include "local_service.h"
#include "hash_func.h"

nm_local_service* nm_local_service::p_instance = new nm_local_service();

nm_local_service* nm_local_service::instance_get()
{
	return p_instance;
}

nm_local_service::nm_local_service()
{
	p_local_param_vector = new local_param_vector_t;
}

int nm_local_service::nm_local_param_set(char *param, local_get_value get, local_set_value set)
{
	local_param_node_t *p_node;

	if(!param) 
		return -1;
	p_node = (local_param_node_t *)malloc(sizeof(local_param_node_t));
	memset(p_node, 0, sizeof(local_param_node_t));
	p_node->param_name = strdup(param);
	p_node->hash_node = ILibGetHashValueEx(p_node->param_name, strlen(p_node->param_name), 0);
	p_node->value_get_call = get;
	p_node->value_set_call = set;
	p_local_param_vector->push_back(p_node);
	return 0;
}


local_get_value nm_local_service::nm_local_param_check_read(char *param)
{
	int i = 0;
	int hash_value = 0;
	local_param_vector_t::iterator it;


	if(!param)
		return 0;
	hash_value = ILibGetHashValueEx(param, strlen(param), 0);
	for(it = p_local_param_vector->begin(); it != p_local_param_vector->end(); ++it){
	    if ((*it)->hash_node == hash_value) {
	    	if(strcmp(param, (*it)->param_name) == 0){
			printf(" found %s\n", param);
			return ((*it)->value_get_call);
	    	}
	    }		
	}
	return NULL;
}


local_set_value nm_local_service::nm_local_param_check_write(char *param)
{
	int i = 0;
	int hash_value = 0;
	local_param_vector_t::iterator it;


	if(!param)
		return 0;
	hash_value = ILibGetHashValueEx(param, strlen(param), 0);
	for(it = p_local_param_vector->begin(); it != p_local_param_vector->end(); ++it){
	    if ((*it)->hash_node == hash_value) {
	    	if(strcmp(param, (*it)->param_name) == 0){
			printf(" found %s\n", param);
			return ((*it)->value_set_call);
	    	}
	    }		
	}
	return NULL;
}

