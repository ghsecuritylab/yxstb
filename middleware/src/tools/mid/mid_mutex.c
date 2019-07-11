
#include "app/Assertions.h"

#include "mid_mutex.h"

#include "independs/ind_mem.h"

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

struct mid_mutex {
	pthread_mutex_t id;
};

mid_mutex_t mid_mutex_create(void)
{
	mid_mutex_t mutex;

	mutex = (mid_mutex_t)IND_MALLOC(sizeof(struct mid_mutex));
	if(mutex == NULL)
		ERR_OUT("\n");
	if (pthread_mutex_init(&mutex->id, NULL)) {
		IND_FREE(mutex);
		ERR_OUT("\n");
	}
	return mutex;
Err:
	return NULL;
}
int mid_mutex_lock0(mid_mutex_t mutex,const char* file,const char *fun,int line)
{
	if(mutex == NULL)
		ERR_OUT("<%s:%s:%d>\n", file, fun, line);

	pthread_mutex_lock(&mutex->id);

	return 0;
Err:
	return -1;
}

int mid_mutex_unlock(mid_mutex_t mutex)
{
	if(mutex == NULL)
		ERR_OUT("\n");
	pthread_mutex_unlock(&mutex->id);

	return 0;
Err:
	return -1;
}

