
#include "app/Assertions.h"

#include "mid_mutex.h"
#include "mid_task.h"

#include "pthread.h"
#include "unistd.h"

typedef void *(*pthread_routine) (void *);

#define MID_TASKNUM_MAX		64

static int g_index;
static pthread_t g_threads[MID_TASKNUM_MAX];

static mid_mutex_t g_mutex = NULL;

void mid_task_init(void)
{
	int i;

    if (g_mutex)
        return;
	g_mutex = mid_mutex_create( ); /*yanglw: 互斥量的初始化*/
	for(i = 0; i < MID_TASKNUM_MAX; i ++)
		g_threads[i] = -1;
	g_index = 0;
}

int mid_task_create(const char* name, mid_func_t func, void *arg)
{
	int tIndex, err;

	if(g_mutex == NULL)
		ERR_OUT("mid task not init\n");

	if (mid_mutex_lock(g_mutex))
		WARN_PRN("\n");
	if(g_index >= MID_TASKNUM_MAX) {
		mid_mutex_unlock(g_mutex);
		ERR_OUT("mid task not init %d, %d, %s\n", g_index, MID_TASKNUM_MAX, name);
	}
	tIndex = g_index;
	g_index ++;
	mid_mutex_unlock(g_mutex);

	err = pthread_create(&g_threads[tIndex], NULL, (pthread_routine)func, arg);
	if (err)
		ERR_OUT("pthread_create\n");

	return 0;
Err:
	return -1;
}


int mid_task_create_ex(const char* name, mid_func_t func, void *arg)
{
	int tIndex, err;

	if(g_mutex == NULL)
		ERR_OUT("mid task not init\n");

	if (mid_mutex_lock(g_mutex))
		WARN_PRN("\n");
	if(g_index >= MID_TASKNUM_MAX) {
		mid_mutex_unlock(g_mutex);
		ERR_OUT("mid task not init %d, %d, %s\n", g_index, MID_TASKNUM_MAX, name);
	}
	tIndex = g_index;
	g_index ++;
	mid_mutex_unlock(g_mutex);

	pthread_attr_t attrThread;
//	pthread_t  epThread;
	struct sched_param param;

	pthread_attr_init(&attrThread);
	pthread_attr_setdetachstate(&attrThread, PTHREAD_CREATE_DETACHED);//这句是设置线程为非绑定，不是必须的
	pthread_attr_setschedpolicy(&attrThread, SCHED_FIFO);
	pthread_attr_getschedparam(&attrThread, &param);
	param.sched_priority = (sched_get_priority_max(SCHED_FIFO) + sched_get_priority_min(SCHED_FIFO)) / 2;
	pthread_attr_setschedparam(&attrThread, &param);
    /**不设置堆栈大小**/
	//pthread_attr_setstacksize(&attrThread, stack_size);

	err = pthread_create(&g_threads[tIndex], &attrThread, (pthread_routine)func, arg);
	if (err)
		ERR_OUT("pthread_create\n");

	return 0;
Err:
	return -1;
}

void mid_task_delay(unsigned int ms)
{
    usleep(ms * 1000);
}

