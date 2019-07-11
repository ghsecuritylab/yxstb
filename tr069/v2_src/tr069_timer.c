/*******************************************************************************
	公司：
			Yuxing software
	纪录：
			2008-1-26 21:12:26 create by Liu Jianhua
	模块：
			tr069
	简述：
			定时器
 *******************************************************************************/
#include "tr069_header.h"

#define TIMER_POOL_SIZE		20

struct Timer {
	struct Timer *next;
	Ontimer ontimer;
	int datetime;
};

static void tr069_timer_queue(struct TR069 *tr069, struct Timer *timer)
{
	struct Timer *next, *prev;

	next = tr069->timer_queue;
	prev = NULL;
	while (next && next->datetime <= timer->datetime) {
		prev = next;
		next = next->next;
	}

	timer->next = next;
	if (prev)
		prev->next = timer;
	else
		tr069->timer_queue = timer;
}

static struct Timer *tr069_timer_dequeue(struct TR069 *tr069, Ontimer ontimer)
{
	struct Timer *timer, *prev;

	timer = tr069->timer_queue;
	prev = NULL;
	while (timer && timer->ontimer != ontimer) {
		prev = timer;
		timer = timer->next;
	}
	if (!timer)
		return NULL;
	if (prev)
		prev->next = timer->next;
	else
		tr069->timer_queue = timer->next;

	return timer;
}

static struct Timer *tr069_timer_alloc(struct TR069 *tr069)
{
	struct Timer *timer = tr069->timer_pool;
	if (!timer)
		return NULL;
	tr069->timer_pool = timer->next;
	timer->next = NULL;
	return timer;
}

static void tr069_timer_free(struct TR069 *tr069, struct Timer *timer)
{
	if (!timer)
		return;
	timer->next = tr069->timer_pool;
	tr069->timer_pool = timer;
}

void tr069_timer_deal(struct TR069 *tr069, unsigned int current)
{
	struct Timer *timer;

	while (tr069->timer_queue && tr069->timer_queue->datetime <= current) {
		timer = tr069->timer_queue;
		tr069->timer_queue = tr069->timer_queue->next;

		timer->ontimer(tr069);
		tr069_timer_free(tr069, timer);
	}
}

int tr069_timer_time(const struct TR069 *tr069)
{
	if (tr069->timer_queue)
		return tr069->timer_queue->datetime;

	return -1;
}

int tr069_timer_create(struct TR069 *tr069, unsigned int datetime, Ontimer ontimer)
{
	struct Timer *timer;

	timer = tr069_timer_dequeue(tr069, ontimer);
	if (!timer)
		timer = tr069_timer_alloc(tr069);
	if (!timer)
		TR069ErrorOut("timer pool is empty!\n");

	timer->datetime = datetime;
	timer->ontimer = ontimer;
	tr069_timer_queue(tr069, timer);

	return 0;
Err:
	return -1;
}

void tr069_timer_delete(struct TR069 *tr069, Ontimer ontimer)
{
	tr069_timer_free(tr069, tr069_timer_dequeue(tr069, ontimer));
}

void tr069_timer_init(struct TR069 *tr069)
{
	int i;

	tr069->timer_pool = NULL;
	tr069->timer_queue = NULL;

	for (i = 0; i < TIMER_POOL_SIZE; i ++)
		tr069_timer_free(tr069, (struct Timer *)IND_MALLOC(sizeof(struct Timer)));
}
