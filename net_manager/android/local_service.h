#ifndef _LOCAL_SERVICE_H_
#define _LOCAL_SERVICE_H_
#include <iostream>
#include <vector>
using namespace std;

typedef int (*local_get_value)(char *param,  char *buf, int buf_len);
typedef int (*local_set_value)(char *param, char *param_value);

typedef struct local_param_node{
	int hash_node;
	char *param_name;
	local_get_value value_get_call;
	local_set_value value_set_call;	
}local_param_node_t;

typedef vector<local_param_node_t*> local_param_vector_t;

//singleton
class nm_local_service{
	public:
		static nm_local_service *instance_get();
		int nm_local_param_set(char *param, local_get_value get, local_set_value set);
		local_get_value nm_local_param_check_read(char *param);
		local_set_value nm_local_param_check_write(char *param);
	private:
		nm_local_service();
		~nm_local_service();
		local_param_vector_t *p_local_param_vector;
		static nm_local_service *p_instance;
};
#endif
