
#ifndef __TR069_CONFIG_H__
#define __TR069_CONFIG_H__

void tr069_config_init(struct TR069 *tr069);
int tr069_config_reset(struct TR069 *tr069);
int tr069_config_load(struct TR069 *tr069);
int tr069_config_save(struct TR069 *tr069);

int tr069_config_file_create(struct TR069 *tr069, char *buffer, int length);
int tr069_config_file_apply(struct TR069 *tr069, char *buffer, int length);
void tr069_config_file_print(struct TR069 *tr069);

#endif//__TR069_CONFIG_H__
