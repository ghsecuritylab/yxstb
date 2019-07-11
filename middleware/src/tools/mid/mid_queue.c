/* removeded by teddy, 2011-4-1 19:29:09.
 * mid_queue_xxx has moved to mid_msgq.c.
 */
#if 0 //defined(USE_MALLOC_QUEUE) 

#if 0
#include "app_include.h"

#include <unistd.h>
#include <pthread.h>

typedef struct int_elem* int_elem_t;
struct int_elem {
	int_elem_t next;
	int_elem_t prev;
	char* buf;
};

struct mid_queue {
	pthread_mutex_t mutex;
	int num;
	int size;
	int_elem_t pool;
	int_elem_t head;
	int_elem_t tail;
};

static pthread_mutex_t	g_mutex = PTHREAD_MUTEX_INITIALIZER;

mid_queue_t mid_queue_create(int msg_num, int msg_size)
{
	int i;
	int_elem_t elem;
	mid_queue_t queue;

	queue = (mid_queue_t)ind_malloc(sizeof(struct mid_queue));
	if(queue == NULL)
		ERR_OUT("ind_malloc\n"); 
	memset(queue, 0, sizeof(struct mid_queue));

	pthread_mutex_init(&queue->mutex, NULL);

	queue->num = msg_num;
	queue->size = msg_size;

	for (i = 0; i < msg_num; i ++) {
		elem = (int_elem_t)malloc(sizeof(struct int_elem) + msg_size);
		elem->buf = (char*)elem + sizeof(struct int_elem);
		elem->next = queue->pool;
		queue->pool = elem;
	}

	return queue;
Err:
	return NULL;
}

static void int_elem_free(int_elem_t elem)
{
	int_elem_t next;

	while(elem) {
		next = elem->next;
		free(elem);
		elem = next;
	}
}

void mid_queue_delete(mid_queue_t queue)
{
	if(queue == NULL)
		ERR_OUT("queue = %p\n", queue);

	int_elem_free(queue->pool);
	queue->pool = NULL;

	int_elem_free(queue->head);
	queue->head = NULL;

	queue->tail = NULL;

Err:
	return;
}

int mid_queue_put(mid_queue_t queue, char *msg)
{
	int_elem_t elem;

	if (queue == NULL)
		ERR_OUT("msgq = %p\n", queue);

	pthread_mutex_lock(&g_mutex);

	if (queue->pool) {
		elem = queue->pool;
		queue->pool = elem->next;
	
		elem->prev = NULL;
		if (queue->head)
			queue->head->prev = elem;
		else
			queue->tail = elem;

		elem->next = queue->head;
		queue->head = elem;

		memcpy(elem->buf, msg, queue->size);
	} else {
		WARN_PRN("queue is full!\n");
	}

	pthread_mutex_unlock(&g_mutex);

	return 0;
Err:
	return -1;
}

int mid_queue_get(mid_queue_t queue, char *msg, int usec)
{
	int ret = -1;
	int timeout = usec;

	if (queue == NULL)
		ERR_OUT("msgq = %p\n", queue);

//	pthread_mutex_lock(&g_mutex);

	while (queue->head == NULL && timeout > 0) {
//		pthread_mutex_unlock(&g_mutex);
		usleep(1000);
		timeout -= 1000;
//		pthread_mutex_lock(&g_mutex);
	}

	if (queue->head) {
		int_elem_t prev, elem;

		elem = queue->tail;
		prev = elem->prev;
		if (prev)
			prev->next = NULL;
		else
			queue->head = NULL;

		queue->tail = prev;

		elem->next = queue->pool;
		queue->pool = elem;

		memcpy(msg, elem->buf, queue->size);

		ret = queue->size;
	} else {
		ret = 0;
	}

//	pthread_mutex_unlock(&g_mutex);

Err:
	return ret;
}
#else
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "app_include.h"

//#define C_FIXEDQ_DEBUG 1
struct _cfixedq {
	unsigned int 	node_num; 	// queue length.
	unsigned int 	node_size; 	// size of each node.
	unsigned int 	head; 		// head node offset.
	unsigned int 	tail; 		// tail node offset.
	char *		buf; 		// queue array base.
	int		full_flag; 	// 1/0 ==> no/yes more storage space.
};

int c_fixedq_init(CFixedQ *p, unsigned int node_num, unsigned int node_size)
{
	if(node_num == 0)
		return -1;
	p->buf = malloc(node_num * node_size);
	if(p->buf == NULL)
		return -2;
	p->head = 0;
	p->tail = 0;
	p->full_flag = 0;
	p->node_num = node_num;
	p->node_size = node_size;
	return 0;
}

void c_fixedq_fina(CFixedQ *p)
{
	free(p->buf);
}
int c_fixedq_empty(CFixedQ *p)
{
return(p->full_flag == 0 && p->tail == p->head);
}
int c_fixedq_recv(CFixedQ *p, void *data)
{
	char *q;
	if(p->full_flag == 0 && p->tail == p->head) {
#ifdef C_FIXEDQ_DEBUG
		PRINTF("[%u %s %u %s] queue empty!!!\n", time(NULL), __FILE__, __LINE__, __func__);
#endif
		return -3;
	}
	q = p->buf + p->head * p->node_size;
	memcpy(data, q, p->node_size);
	if(++p->head == p->node_num)
		p->head = 0;
	if(p->tail == p->head)
		p->full_flag = 0;
#ifdef C_FIXEDQ_DEBUG
	c_fixedq_dump(p, __func__);
#endif
	return 0;
}

int c_fixedq_send(CFixedQ *p, const void *data)
{
	char *q;
	if(p->full_flag) {
#ifdef C_FIXEDQ_DEBUG
		PRINTF("[%u %s %u %s] queue full!!!\n", time(NULL), __FILE__, __LINE__, __func__);
#endif
		return -4;
	}
	q = p->buf + p->tail * p->node_size;
	memcpy(q, data, p->node_size);
	if(++p->tail == p->node_num)
		p->tail = 0;
	if(p->tail == p->head)
		p->full_flag = 1;
#ifdef C_FIXEDQ_DEBUG
	c_fixedq_dump(p, __func__);
#endif
	return 0;
}

#ifdef C_FIXEDQ_DEBUG
void c_fixedq_dump(CFixedQ *p, const char *prefix)
{
	int i;
	PRINTF("%s dumping... [", prefix);
	for(i=0; i<p->node_num; i++) {
		char *j = p->buf + i * p->node_size;
		PRINTF("%2d ", *(int *)j);
	}
	PRINTF("] [ ");
	if(p->head < p->tail) 
		for(i=p->head; i<p->tail; i++) {
			char *j = p->buf + i * p->node_size;
			PRINTF("%2d ", *(int *)j);
		}
	else
		for(i=p->tail; i<p->head; i++) {
			char *j = p->buf + i * p->node_size;
			PRINTF("-%d ", *(int *)j);
		}
	PRINTF("] done.\n");
}

	int
main ( int argc, char *argv[] )
{
#define N_MAX 10
	int i;
	CFixedQ Q, *q = &Q;
	c_fixedq_init(q, N_MAX, sizeof(int));

	for(i=0; i<N_MAX/2; i++) {
		int j;
		c_fixedq_recv(q, &j);
	}
	for(i=0; i<N_MAX+5; i++) {
		c_fixedq_send(q, &i);
	}
	for(i=0; i<N_MAX+10; i++) {
		int j;
		c_fixedq_recv(q, &j);
		PRINTF("[%u %s %u %s] j=%d\n", time(NULL), __FILE__, __LINE__, __func__, j);
	}

	PRINTF("overlapping...\n");
	for(i=0; i<N_MAX*4; i++) {
		int j;
		if(i%3)
			c_fixedq_send(q, &i);
		else
			c_fixedq_recv(q, &j);
	}
	PRINTF("overlapping...done\n");
	for(i=0; i<N_MAX; i++) {
		int j;
		c_fixedq_recv(q, &j);
		PRINTF("[%u %s %u %s] i=%d, j=%d\n", time(NULL), __FILE__, __LINE__, __func__, i, j);
	}


	return 0;
}
#endif
mid_queue_t mid_queue_create(int msg_num, int msg_size)
{
	int i;
	mid_queue_t queue;

	queue = (mid_queue_t)ind_malloc(sizeof(struct _cfixedq));
	if(queue == NULL)
		ERR_OUT("ind_malloc\n"); 
	c_fixedq_init(queue, msg_num, msg_size);
	return queue;
Err:
	return NULL;
}


void mid_queue_delete(mid_queue_t queue)
{
	if(queue == NULL)
		ERR_OUT("queue = %p\n", queue);

	c_fixedq_fina(queue);
	ind_free(queue);
Err:
	return;
}

int mid_queue_put(mid_queue_t queue, char *msg)
{
	int ret;
	if (queue == NULL)
		ERR_OUT("msgq = %p\n", queue);

	unsigned long long s,e;
//	PRINTF("[%s %u %s] sending msg @ %llu...", __FILE__, __LINE__, __func__, s = mid_clock());
	ret = c_fixedq_send(queue, msg);
//	PRINTF("done @ %llu, using %llu milliseconds, ret = %d\n", e = mid_clock(), e-s, ret);
	return ret;
Err:
	return -1;
}

int mid_queue_get(mid_queue_t queue, char *msg, int usec)
{
	int ret = -1;
	int timeout = usec;

	if (queue == NULL)
		ERR_OUT("msgq = %p\n", queue);
	
	unsigned long long s,e;
//	PRINTF("[%s %u %s] recving msg @ %llu...", __FILE__, __LINE__, __func__, s = mid_clock());

	#if 0
	PRINTF("[%s %u %s] sleeping %d microseconds @ %llu...", __FILE__, __LINE__, __func__, usec, s = mid_clock());
	while (c_fixedq_empty(queue) && timeout > 0) {
		usleep(5000);
		timeout -= 5000;
	}
	PRINTF("done @ %llu, using %llu milliseconds\n", e = mid_clock(), e-s);
	#endif
	ret = c_fixedq_recv(queue, msg);
//	PRINTF("done @ %llu, using %llu milliseconds, ret = %d\n", e = mid_clock(), e-s, ret);
	return ret;
Err:
	return ret;
}
#endif

#endif
