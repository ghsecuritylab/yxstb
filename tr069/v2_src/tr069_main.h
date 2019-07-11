
#ifndef __TR069_MAIN_H__
#define __TR069_MAIN_H__

struct Message {
	u_int id;
	int arg0;
	int arg1;
};

void tr069_schedule_timer(struct TR069 *tr069);
void tr069_periodic_timer(struct TR069 *tr069);
void tr069_load_timer(struct TR069 *tr069);

void tr069_event_push(const struct TR069 *tr069, struct Message *msg);

struct TR069 *tr069_struct_create(void);
void tr069_task_loop(struct TR069 *tr069);

void tr069_test_parse(struct TR069 *tr069, const char *buf, uint32_t len);

extern int g_tr069_digestAuth;
extern int g_tr069_bootstrap;
extern int g_tr069_targetUpgrade;

extern int g_tr069_paramPedant;
extern int g_tr069_holdCookie;
extern int g_tr069_httpTimeout;
extern char *g_tr069_paramPath;

#endif//__TR069_MAIN_H__
