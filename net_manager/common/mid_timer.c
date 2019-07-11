
//#include "net_include.h"
#include <pthread.h>

#include "mid_timer.h"
#include "mid_select.h"
#include "mid_msgq.h"

#include "nm_dbg.h"

#define TIMER_POOL_SIZE		128

#define HOUR_CLOCK_NUM		(60*60*100)
#define YEAR_CLOCK_NUM		(365*60*60*100)

typedef struct _msg_timer_t msg_timer_t;
typedef struct _msg_timer_q msg_timer_q;
struct _msg_timer_t
{
	msg_timer_t *next;
	mid_timer_f func;
	int arg;
	unsigned int loops;
	unsigned int intval;
	unsigned int clock;
};
struct _msg_timer_q
{
	msg_timer_t *head;
};

static msg_timer_q	g_timer_q_pool;
static msg_timer_q	g_timer_q_time;


static mid_timer_f	g_deal_funcs[TIMER_POOL_SIZE];
static int			g_deal_args[TIMER_POOL_SIZE];
static int			g_deal_num = 0;

static pthread_mutex_t	g_mutex;
static mid_msgq_t	g_msgq = NULL;

static void msg_timer_queue(msg_timer_t *timer, unsigned int pClock)
{
	msg_timer_t *next, *prev;

	while(timer->clock <= pClock && timer->intval > 0)
		timer->clock += timer->intval;

	next = g_timer_q_time.head;
	prev = NULL;
	while(next && next->clock <= timer->clock) {
		prev = next;
		next = next->next;
	}

	timer->next = next;
	if (prev)
		prev->next = timer;
	else
		g_timer_q_time.head = timer;
}

static msg_timer_t *msg_timer_dequeue(mid_timer_f func, int arg, int any)
{
	msg_timer_t *timer, *prev;

	timer = g_timer_q_time.head;
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
		g_timer_q_time.head = timer->next;

	return timer;
}

static msg_timer_t *msg_timer_alloc(void)
{
	msg_timer_t *timer = g_timer_q_pool.head;
	if (timer == NULL)
		return NULL;
	g_timer_q_pool.head = timer->next;
	timer->next = NULL;
	return timer;
}

static void msg_timer_free(msg_timer_t *timer)
{
	if (timer == NULL)
		return;
	timer->next = g_timer_q_pool.head;
	g_timer_q_pool.head = timer;
}

static unsigned int mid_10ms(void)
{
    unsigned int clk;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    clk = (unsigned int)tp.tv_sec * 100 + (unsigned int)tp.tv_nsec / 10000000;

    return clk;
}

static unsigned int msg_timer_deal(void)
{
	unsigned int i;
	unsigned int tClock, remain;
	msg_timer_t *timer;
	mid_timer_f func;
	int arg;

	pthread_mutex_lock(&g_mutex);
	g_deal_num = 0;
	tClock = mid_10ms( );

	timer = g_timer_q_time.head;
	while(timer && timer->clock <= tClock) {
		g_deal_funcs[g_deal_num] = timer->func;
		g_deal_args[g_deal_num] = timer->arg;
		g_deal_num ++;
		msg_timer_dequeue(timer->func, timer->arg, 0);
		if (timer->loops == 0) {
			msg_timer_queue(timer, tClock);
		} else {
			timer->loops --;
			if (timer->loops > 0)
				msg_timer_queue(timer, tClock);
			else
				msg_timer_free(timer);
		}
		//��ס ����һ���滻��timer = timer->next�Ǵ����
		timer = g_timer_q_time.head;
	}
	timer = g_timer_q_time.head;
	if (timer)
		remain = timer->clock - tClock;
	else
		remain = HOUR_CLOCK_NUM;

	for (i = 0; i < g_deal_num; i ++) {
		func = g_deal_funcs[i];
		arg = g_deal_args[i];
		if (func) {
			//LogUserOperDebug("func = %p, arg = %d/0x%x\n", func, arg, arg);
			pthread_mutex_unlock(&g_mutex);
			//LogUserOperDebug("--------: func func = %p, arg = %d enter \n", func, arg);
			func(arg);
			//LogUserOperDebug("--------: func exit\n");
			pthread_mutex_lock(&g_mutex);
		}
	}
	g_deal_num = 0;

	pthread_mutex_unlock(&g_mutex);

	if (remain > HOUR_CLOCK_NUM)
		return HOUR_CLOCK_NUM;

	return remain;
}

void mid_timer_refresh(void)
{
	char ch;
	mid_msgq_putmsg(g_msgq, &ch);
}

/*
	intval: �������
	loops����ʱ��ִ��ѭ������
	func����ʱ��ִ����
	arg����ʱ����ʱ����func�Ĳ���
	һ����ʱ��������������ʾfunc��arg
	�ڶ�ʱ��ִ�е��Ǹ������ﲻҪ��̫����£������ֻ�����Լ�����һ����Ϣ���ѡ�
 */
int int_timer_create(int intval, int loops, mid_timer_f func, int arg)
{
	msg_timer_t *timer;
	unsigned int tClock;

	if (intval < 0 || loops < 0 || (intval == 0 && loops != 1) || func == NULL){
		nm_msg("param error! %d:%d:%p\n", intval, loops, func);
		goto Err;
	}
	pthread_mutex_lock(&g_mutex);

	timer = msg_timer_dequeue(func, arg, 0);
	if (timer == NULL)
		timer = msg_timer_alloc( );
	if (timer == NULL) {
		pthread_mutex_unlock(&g_mutex);
		nm_msg("timer pool is empty!\n");
		goto Err;
	}
	tClock = mid_10ms( );
	//LogUserOperDebug("++++++++ %d  %d %p\n", intval, msec, func);
	timer->intval = intval;
	timer->clock = tClock;
	timer->loops = loops;
	timer->func = func;
	timer->arg = arg;
	msg_timer_queue(timer, tClock);

	pthread_mutex_unlock(&g_mutex);
	nm_msg_level(LOG_DEBUG, "func = %p, arg = %d/0x%x\n", func, arg, arg);

	mid_timer_refresh( );

	return 0;
Err:
	return -1;
}

int mid_timer_micro(int intval, mid_timer_f func, int arg)
{
	return int_timer_create(intval / 10, 1, func, arg);
}

int mid_timer_create(int intval, int loops, mid_timer_f func, int arg)
{
	return int_timer_create(intval * 100, loops, func, arg);
}

void mid_timer_print(void)
{
	msg_timer_t *timer;

	nm_msg_level(LOG_DEBUG, "TIMER:\n");
	pthread_mutex_lock(&g_mutex);
	timer = g_timer_q_time.head;
	while(timer) {
		nm_msg_level(LOG_DEBUG, "%p: func = %p, arg = %d/0x%x\n", timer, timer->func, timer->arg, timer->arg);
		timer = timer->next;
	}
	pthread_mutex_unlock(&g_mutex);
}

void mid_timer_delete_all(mid_timer_f func)
{
	int i;
	msg_timer_t *timer;

	pthread_mutex_lock(&g_mutex);

	for (;;) {
		timer = msg_timer_dequeue(func, 0, 1);
		if (timer == NULL)
			break;
		msg_timer_free(timer);
	}
	if (g_deal_num > 0) {
		for (i = 0; i < g_deal_num; i ++) {
			if (g_deal_funcs[i] == func)
				g_deal_funcs[i] = NULL;
		}
	}

	pthread_mutex_unlock(&g_mutex);
	nm_msg_level(LOG_DEBUG, "func = %p\n", func);
}

void mid_timer_delete(mid_timer_f func, int arg)
{
	int i;
	msg_timer_t *timer;

	pthread_mutex_lock(&g_mutex);

	timer = msg_timer_dequeue(func, arg, 0);
	if (timer)
		msg_timer_free(timer);
	if (g_deal_num > 0) {
		for (i = 0; i < g_deal_num; i ++) {
			if (g_deal_funcs[i] == func && g_deal_args[i] == arg) {
				nm_msg_level(LOG_DEBUG, "func = %p, arg = %d/0x%x\n", func, arg, arg);
				g_deal_funcs[i] = NULL;
			}
		}
	}

	pthread_mutex_unlock(&g_mutex);
}

static void mid_timer_loop(void *arg)
{
	unsigned int clk;
	for (;;)
	{
		clk = msg_timer_deal( );
		mid_select_exec(clk);
	}
}

static void mid_timer_select(int flag)
{
	char ch;
	mid_msgq_getmsg(g_msgq, &ch);
}

void mid_timer_init(void)
{
	int i;
	pthread_t timer_pid;


	nm_msg_level(LOG_DEBUG, "SIZE: _msg_timer_t = %d\n", sizeof(struct _msg_timer_t));

	mid_select_init( );

	g_timer_q_pool.head = NULL;
	g_timer_q_time.head = NULL;


	for (i = 0; i < TIMER_POOL_SIZE; i ++)
		msg_timer_free((msg_timer_t *)malloc(sizeof(msg_timer_t)));
	pthread_mutex_init(&g_mutex, NULL);
	g_msgq = mid_msgq_create(16, 1);

	mid_select_regist(mid_msgq_fd(g_msgq), mid_timer_select, 0);
	pthread_create(&timer_pid, NULL, mid_timer_loop, NULL);
}
