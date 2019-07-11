#ifndef _TR069_LOG_H_
#define _TR069_LOG_H_

void tr069_log_param_set(char *name, char *str);
int tr069_log_param_get(char *name, char *buf, int buf_len);

void tr069_log_param_init();
void tr069_log_param_save();
// void tr069_log_param_load();

#endif
