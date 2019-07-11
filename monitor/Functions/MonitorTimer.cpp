#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "MonitorTimer.h"

#define TIMER_POOL_SIZE		128
#define HOUR_CLOCK_NUM		(60*60*100)
#define YEAR_CLOCK_NUM		(365*60*60*100)



static monitor_timer_q	g_timerQueuePool;
static monitor_timer_q	g_timerQueue;


static monitor_timer_f	g_deal_funcs[TIMER_POOL_SIZE];
static int			g_deal_args[TIMER_POOL_SIZE];
static int			g_deal_num = 0;

static pthread_mutex_t  g_monitorTimerMutex = PTHREAD_MUTEX_INITIALIZER;

//static mid_mutex_t	&g_monitorTimerMutex = NULL;
//static mid_msgq_t	g_msgq = NULL;

static void monitorTimerQueue(monitor_timer_t *timer, unsigned int pClock)
{
	monitor_timer_t *next, *prev;

	while(timer->clock <= pClock && timer->intval > 0)
		timer->clock += timer->intval;

	next = g_timerQueue.head;
	prev = NULL;
	while(next && next->clock <= timer->clock) {
		prev = next;
		next = next->next;
	}

	timer->next = next;
	if (prev)
		prev->next = timer;
	else
		g_timerQueue.head = timer;
}

static monitor_timer_t *monitorTimerDequeue(monitor_timer_f func, int arg, int any)
{
	monitor_timer_t *timer, *prev;

	timer = g_timerQueue.head;
	prev = NULL;
	while(timer && (timer->func != func || (any == 0 && timer->arg != arg))) {
		prev = timer;
		timer = timer->next;
	}
	if (timer == NULL)
		return NULL;
	if (prev)
		prev->next = timer->next;
	else
		g_timerQueue.head = timer->next;

	return timer;
}

static monitor_timer_t *monitorTimerAlloc(void)
{
	monitor_timer_t *timer = g_timerQueuePool.head;
	if (timer == NULL)
		return NULL;
	g_timerQueuePool.head = timer->next;
	timer->next = NULL;
	return timer;
}

static void monitorTimerFree(monitor_timer_t *timer)
{
	if (timer == NULL)
		return;
	timer->next = g_timerQueuePool.head;
	g_timerQueuePool.head = timer;
}

static unsigned int monitorTimerDeal(void)
{
	int i;
	unsigned int tClock, remain;
	monitor_timer_t *timer;
	monitor_timer_f func;
	int arg;

	pthread_mutex_lock(&g_monitorTimerMutex);
	g_deal_num = 0;

    
    unsigned int clk;
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    tClock = (unsigned int)tp.tv_sec * 100 + (unsigned int)tp.tv_nsec / 10000000;


	timer = g_timerQueue.head;
	while(timer && timer->clock <= tClock) {
		g_deal_funcs[g_deal_num] = timer->func;
		g_deal_args[g_deal_num] = timer->arg;
		g_deal_num ++;
		monitorTimerDequeue(timer->func, timer->arg, 0);
		if (timer->loops == 0) {
			monitorTimerQueue(timer, tClock);
		} else {
			timer->loops --;
			if (timer->loops > 0)
				monitorTimerQueue(timer, tClock);
			else
				monitorTimerFree(timer);
		}
		//记住 下面一行替换成timer = timer->next是错误的
		timer = g_timerQueue.head;
	}
	timer = g_timerQueue.head;
	if (timer)
		remain = timer->clock - tClock;
	else
		remain = HOUR_CLOCK_NUM;

	for (i = 0; i < g_deal_num; i ++) {
		func = g_deal_funcs[i];
		arg = g_deal_args[i];
		if (func) {
			//printf("func = %p, arg = %d/0x%x\n", func, arg, arg);
			pthread_mutex_unlock(&g_monitorTimerMutex);
			//printf("--------: func func = %p, arg = %d enter \n", func, arg);
			func(arg);
			//printf("--------: func exit\n");
			pthread_mutex_lock(&g_monitorTimerMutex);
		}
	}
	g_deal_num = 0;

	pthread_mutex_unlock(&g_monitorTimerMutex);

	if (remain > HOUR_CLOCK_NUM)
		return HOUR_CLOCK_NUM;

	return remain;
}

void monitorTimerRefresh(void)
{
	char ch;
	//mid_msgq_putmsg(g_msgq, &ch);
}

/*
	intval: 间隔秒数
	loops：定时器执行循环次数
	func：定时到执函数
	arg：定时器到时传给func的参数
	一个定时器由连个参数标示func和arg
	在定时器执行的那个函数里不要干太多的事，最好是只给你自己任务发一个消息而已。
 */
int TimerCreate(int intval, int loops, monitor_timer_f func, int arg)
{
	monitor_timer_t *timer;
	unsigned int tClock;

	if (intval < 0 || loops < 0 || (intval == 0 && loops != 1) || func == NULL)
		printf("param error! %d:%d:%p\n", intval, loops, func);

	pthread_mutex_lock(&g_monitorTimerMutex);

	timer = monitorTimerDequeue(func, arg, 0);
	if (timer == NULL)
		timer = monitorTimerAlloc( );
	if (timer == NULL) {
		pthread_mutex_unlock(&g_monitorTimerMutex);
		printf("timer pool is empty!\n");
        return -1;
	}

    
    unsigned int clk;
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    tClock = (unsigned int)tp.tv_sec * 100 + (unsigned int)tp.tv_nsec / 10000000;
	
	//printf("++++++++ %d  %d %p\n", intval, msec, func);
	timer->intval = intval;
	timer->clock = tClock;
	timer->loops = loops;
	timer->func = func;
	timer->arg = arg;
	monitorTimerQueue(timer, tClock);

	pthread_mutex_unlock(&g_monitorTimerMutex);
	printf("func = %p, arg = %d/0x%x\n", func, arg, arg);

	monitorTimerRefresh( );

	return 0;

}

int monitorTimerMicro(int intval, monitor_timer_f func, int arg)
{
	return TimerCreate(intval / 10, 1, func, arg);
}

int monitorTimerCreate(int intval, int loops, monitor_timer_f func, int arg)
{
	return TimerCreate(intval * 100, loops, func, arg);
}

void monitorTimerPrint(void)
{
	monitor_timer_t *timer;

	printf("TIMER:\n");
	pthread_mutex_lock(&g_monitorTimerMutex);
	timer = g_timerQueue.head;
	while(timer) {
		printf("%p: func = %p, arg = %d/0x%x\n", timer, timer->func, timer->arg, timer->arg);
		timer = timer->next;
	}
	pthread_mutex_unlock(&g_monitorTimerMutex);
}

void monitorTimerDeleteAll(monitor_timer_f func)
{
	int i;
	monitor_timer_t *timer;

	pthread_mutex_lock(&g_monitorTimerMutex);

	for (;;) {
		timer = monitorTimerDequeue(func, 0, 1);
		if (timer == NULL)
			break;
		monitorTimerFree(timer);
	}
	if (g_deal_num > 0) {
		for (i = 0; i < g_deal_num; i ++) {
			if (g_deal_funcs[i] == func)
				g_deal_funcs[i] = NULL;
		}
	}

	pthread_mutex_unlock(&g_monitorTimerMutex);
	printf("func = %p\n", func);
}

void monitorTimerDelete(monitor_timer_f func, int arg)
{
	int i;
	monitor_timer_t *timer;

	pthread_mutex_lock(&g_monitorTimerMutex);

	timer = monitorTimerDequeue(func, arg, 0);
	if (timer)
		monitorTimerFree(timer);
	if (g_deal_num > 0) {
		for (i = 0; i < g_deal_num; i ++) {
			if (g_deal_funcs[i] == func && g_deal_args[i] == arg) {
				printf("func = %p, arg = %d/0x%x\n", func, arg, arg);
				g_deal_funcs[i] = NULL;
			}
		}
	}

	pthread_mutex_unlock(&g_monitorTimerMutex);
}

static void* monitorTimerLoop(void *arg)
{
	unsigned int clk;
    pthread_detach(pthread_self());
    
	for (;;)
	{
		clk = monitorTimerDeal( );
        usleep(10 * 1000);
	}
    return NULL;
}

static void monitorTimerSelect(int flag)
{
	char ch;
	//mid_msgq_getmsg(g_msgq, &ch);
}

void monitorTimerInit(void)
{
	int i;
    pthread_t monitorTimerThreadID = 0;
	printf("SIZE: monitor_timer_t = %d\n", sizeof(struct monitor_timer_t));

	//mid_select_init( );

	g_timerQueuePool.head = NULL;
	g_timerQueue.head = NULL;

	for (i = 0; i < TIMER_POOL_SIZE; i ++)
		monitorTimerFree((monitor_timer_t *)malloc(sizeof(monitor_timer_t)));

	//&g_monitorTimerMutex = mid_mutex_create( );
	//g_msgq = mid_msgq_create(16, 1);

	//mid_select_regist(mid_msgq_fd(g_msgq), monitorTimerSelect, 0);

	pthread_create(&monitorTimerThreadID, NULL, monitorTimerLoop, NULL);
}
