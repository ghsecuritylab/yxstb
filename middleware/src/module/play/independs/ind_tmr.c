
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "app/Assertions.h"
#include "ind_tmr.h"
#include "ind_mem.h"

struct ind_timer {
	struct ind_timer *next;
	Ontimer ontimer;
	uint32_t clock;
	int interval;
	void *arg;
};

typedef struct ind_timer* ind_timer_t;

struct ind_tlink {
	struct ind_timer *timer_pool;
	struct ind_timer *timer_queue;
	struct ind_timer timer_array[1];
};

static void ind_timer_queue(ind_tlink_t tlink, ind_timer_t timer)
{
	ind_timer_t next, prev;

	next = tlink->timer_queue;
	prev = NULL;
	while(next && next->clock <= timer->clock) {
		prev = next;
		next = next->next;
	}

	timer->next = next;
	if (prev)
		prev->next = timer;
	else
		tlink->timer_queue = timer;
}

static ind_timer_t ind_timer_dequeue(ind_tlink_t tlink, Ontimer ontimer, void *arg)
{
	ind_timer_t timer, prev;

	timer = tlink->timer_queue;
	prev = NULL;
	while(timer && (timer->ontimer != ontimer || timer->arg != arg)) {
		prev = timer;
		timer = timer->next;
	}
	if (timer) {
		if (prev)
			prev->next = timer->next;
		else
			tlink->timer_queue = timer->next;
	}

	return timer;
}

static ind_timer_t ind_timer_alloc(ind_tlink_t tlink)
{
	ind_timer_t timer = tlink->timer_pool;
	if (timer == NULL)
		return NULL;
	tlink->timer_pool = timer->next;
	timer->next = NULL;
	return timer;
}

static void ind_timer_free(ind_tlink_t tlink, struct ind_timer *timer)
{
	if (timer == NULL)
		return;
	timer->next = tlink->timer_pool;
	tlink->timer_pool = timer;
}

uint32_t ind_timer_clock(ind_tlink_t tlink)
{
	if (tlink->timer_queue)
		return tlink->timer_queue->clock;
	return 0xBBF81E00;//365*24*3600*100
}

void ind_timer_deal(ind_tlink_t tlink, uint32_t clock)
{
	ind_timer_t timer;
	Ontimer ontimer;
	void *arg;

	//PRINTF("tlink->timer_queue->clock = %d, clock = %d\n", (int)tlink->timer_queue->clock, (int)clock);
	while(tlink->timer_queue && tlink->timer_queue->clock <= clock) {
		timer = tlink->timer_queue;
		tlink->timer_queue = timer->next;

		ontimer = timer->ontimer;
		arg = timer->arg;
		if (timer->interval) {
			timer->clock = clock + timer->interval;
			ind_timer_queue(tlink, timer);
		} else {
			ind_timer_free(tlink, timer);
		}

		ontimer(arg);
	}
}

void ind_timer_print(ind_tlink_t tlink)
{
	ind_timer_t timer;

	timer = tlink->timer_queue;
	while(timer) {
		PRINTF("ontimer = %p, interval = %d\n", timer->ontimer, timer->interval);
		timer = timer->next;
	}
}

int ind_timer_create(ind_tlink_t tlink, uint32_t clock, int interval, Ontimer ontimer, void *arg)
{
	ind_timer_t timer;

	//PRINTF("clock = %d, interval = %d\n", (int)clock, (int)interval);
	if (!ontimer || !arg)
		ERR_OUT("param error!\n");

	timer = ind_timer_dequeue(tlink, ontimer, arg);

	if (timer == NULL)
		timer = ind_timer_alloc(tlink);
	if (timer == NULL)
		ERR_OUT("timer pool is empty!\n");

	timer->arg = arg;
	timer->clock = clock;
	timer->ontimer = ontimer;
	timer->interval = interval;

	ind_timer_queue(tlink, timer);

	return 0;
Err:
	return -1;
}

void ind_timer_delete(ind_tlink_t tlink, Ontimer ontimer, void *arg)
{
	ind_timer_t timer = ind_timer_dequeue(tlink, ontimer, arg);
	ind_timer_free(tlink, timer);
}

void ind_timer_delete_all(struct ind_tlink *tlink)
{
	ind_timer_t timer, next;

	timer = tlink->timer_queue;
	while(timer) {
		next = timer->next;
		ind_timer_free(tlink, timer);
		timer = next;
	}
	tlink->timer_queue = NULL;
}

ind_tlink_t ind_tlink_create(int num)
{
	int i;
	ind_tlink_t tlink = NULL;
	ind_timer_t timer;

	PRINTF("SIZE: ind_timer = %d\n", sizeof(struct ind_timer));
	if (num <= 0)
		ERR_OUT("num = %d\n", num);
	tlink = (struct ind_tlink *)IND_MALLOC(sizeof(struct ind_tlink) + sizeof(struct ind_timer) * (num - 1));
	if (tlink == NULL)
		ERR_OUT("malloc ind_tlink!\n");

	tlink->timer_pool = NULL;
	tlink->timer_queue = NULL;

	for (i = 0; i < num; i ++) {
		timer = &tlink->timer_array[i];
		ind_timer_free(tlink, timer);
	}

	return tlink;
Err:
	if (tlink)
		IND_FREE(tlink);
	return NULL;
}

void ind_tlink_delete(ind_tlink_t tlink)
{
	if (tlink == NULL)
		return;

	IND_FREE(tlink);
}
