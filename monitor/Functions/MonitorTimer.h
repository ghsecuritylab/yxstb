#ifndef MonitorTimer_h
#define MonitorTimer_h


typedef void (*monitor_timer_f)(int arg);
typedef struct monitor_timer_t monitor_timer_t;
typedef struct monitor_timer_q monitor_timer_q;

struct monitor_timer_t {
	monitor_timer_t* next;
	monitor_timer_f func;
	int arg;
	unsigned int loops;
	unsigned int intval;
	unsigned int clock;
};

struct monitor_timer_q {
	monitor_timer_t* head;
};

void monitorTimerInit(void);
int monitorTimerCreate(int intval, int loops, monitor_timer_f func, int arg);
void monitorTimerDelete(monitor_timer_f func, int arg);
void monitorTimerDeleteAll(monitor_timer_f func);
void monitorTimerPrint(void);

#endif //MonitorTimer_h

