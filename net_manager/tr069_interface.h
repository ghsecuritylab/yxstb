#ifndef TR069_INTERFACE_H_
#define TR069_INTERFACE_H_

#include "tr069_api.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef int (*tr069_platform_get)(char *name, char *value, int int_value);
typedef int  (*tr069_platform_set)(char *name, char *value, int int_value);

typedef struct acs_info{
    int UpgradeFailFlag;
	char url[512 + 1];
	char user_name[64 + 1];
	char user_pass[64 + 1];
	char peridic_enable[4 + 1];
	char peridic_interval[8 + 1];
	char connect_user_name[64 + 1];
	char connect_user_pass[64 + 1];
}acs_info_t;

void tr069_platform_callback_set(tr069_platform_get platform_get, tr069_platform_set platform_set);
int android_platform_getValue(char *name, char *str, int str_lenl);
int android_platform_setValue(char *name, char *str, int pval);
int tr069_start(acs_info_t *info);
int tr069_suspend();
int tr069_resume();
int tr069_acs_set(char *url, char *user_name, char *user_pass, char *periodic_enable, char * peridic_interval, char *cpe_name, char *cpe_pass);
int getAcsPassword(char * value);
int setAcsPassword(const char *value);

#ifdef __cplusplus
};
#endif

#endif
