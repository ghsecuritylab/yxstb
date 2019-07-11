
#ifndef __TR069_PARAM_H__
#define __TR069_PARAM_H__

enum {
    NOTIFICATION_OFF = 0,
    NOTIFICATION_PASSIVE,
    NOTIFICATION_ACTIVE
};

#ifdef __cplusplus
extern "C" {
#endif

char *tr069_fmt_time(struct TR069 *, int);
char *tr069_fmt_id(struct TR069 *);
char *tr069_fmt_int(struct TR069 *, const char *, int);
char *tr069_fmt_uint(struct TR069 *, const char *, unsigned int);
char *tr069_fmt_param(struct TR069 *, const char *);

unsigned int tr069_param_hash_value(const unsigned char *);
struct Param* tr069_short_hash_find(const struct TR069 *, const char *);

struct Param* tr069_param_hash_find(const struct TR069 *, const char *);
int tr069_param_hash_inset(struct TR069 *, struct Param *);
/*
    为了参数扩展，而不导致已经使用的项目出现问题
    tr069_param_create 更名为 tr069_param_new
 */
int tr069_param_new(struct TR069 *, char *, char *,unsigned int, unsigned int, unsigned int, 
                    Tr069ValueFunc, Tr069ValueFunc, Tr069ValueFunc, OnChgFunc);
int tr069_param_virtual(struct TR069 *, char *, char *, unsigned int, unsigned int, unsigned int, 
                    Tr069ValueFunc, Tr069ValueFunc);

int tr069_param_write(struct TR069 *, char *, char *, unsigned int);
int tr069_param_write_try(struct TR069 *, int, char *);
int tr069_param_write_attr(struct TR069 *, char *, unsigned int);

int tr069_param_read(struct TR069 *, char *, char *, unsigned int);
int tr069_param_read_cksum(struct TR069 *, struct Param *, unsigned int*);

void tr069_param_check(const struct TR069 *);
int tr069_param_index(struct TR069 *tr069, char *name);

int tr069_object_add(struct TR069 *, char *, int);
int tr069_object_delete(struct TR069 *, struct Param *);

void tr069_paramChange_inset(struct TR069 *tr069, struct Param *param);

void tr069_paramObject_reset(struct TR069 *);
int tr069_paramObject_inset(struct TR069 *, struct Param *, int);
int tr069_paramObject_inset_leaf(struct TR069 *, const struct Param *);
int tr069_paramObject_inset_index(struct TR069 *, int);
int tr069_paramObject_apply_values(struct TR069 *, int);

int tr069_param_boot_regist(struct TR069 *, char *);
int tr069_param_inform_regist(struct TR069 *, char *);
int tr069_param_restore_regist(struct TR069 *, char *);
int tr069_param_restore_prepare(struct TR069 *);
int tr069_param_restore_complete(struct TR069 *);

void tr069_param_load(struct TR069 *tr069);

#ifdef __cplusplus
}
#endif

#endif //__TR069_PARAM_H__
