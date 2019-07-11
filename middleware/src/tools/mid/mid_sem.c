
#include "app/Assertions.h"

#include "mid_sem.h"

#include <stdlib.h>
#include <unistd.h>

#include <sys/select.h>

struct mid_sem {
	int fds[2];
};

mid_sem_t mid_sem_create(void)
{
	mid_sem_t sem;

	sem = (mid_sem_t)malloc(sizeof(struct mid_sem));
	if(sem == NULL)
		ERR_OUT("malloc\n"); 

	if(pipe(sem->fds)) {
		free(sem);
		ERR_OUT("pipe\n"); 
	}

	return sem;
Err:
	return NULL;
}

int mid_sem_take(mid_sem_t sem, int sec, int usec)
{
	int ret;
	char buf[1];

	fd_set rset;
	struct timeval tv;

	if(sem == NULL)
		ERR_OUT("sem = %p\n", sem);
	if(sec < 0 || usec < 0)
		;
	else{
		tv.tv_sec = sec;
		tv.tv_usec = usec;
	}

	FD_ZERO(&rset);
	FD_SET( sem->fds[0], &rset);

	if(sec < 0 || usec < 0)
		ret = select(sem->fds[0] + 1, &rset, NULL, NULL, NULL);
	else
		ret = select(sem->fds[0] + 1, &rset, NULL, NULL, &tv);
	if (ret == 0)
		goto Err;
	if(ret <= 0)
		ERR_OUT("select ret = %d\n", ret);

	ret = read(sem->fds[0], buf, 1);
	if (ret != 1)
		ERR_OUT("ret = %d\n", ret);

	return 0;
Err:
	return -1;
}

char mid_sem_take0(mid_sem_t sem, int sec, int usec)
{
	int ret;
	char buf[1];

	fd_set rset;
	struct timeval tv;

	if(sem == NULL || sec < 0 || usec < 0)
		ERR_OUT("sem = %p, sec = %d, usec = %d\n", sem, sec, usec); 

	tv.tv_sec = sec;
	tv.tv_usec = usec;

	FD_ZERO(&rset);
	FD_SET( sem->fds[0], &rset);

	ret = select(sem->fds[0] + 1, &rset, NULL, NULL, &tv);
	if (ret == 0)
		goto Err;
	if(ret <= 0)
		ERR_OUT("select ret = %d\n", ret);

	ret = read(sem->fds[0], buf, 1);
	if (ret != 1)
		ERR_OUT("ret = %d\n", ret);

	return buf[0];
Err:
	return -1;
}

int mid_sem_give(mid_sem_t sem)
{
	int ret;
	char buf[1];

	fd_set wset;
	struct timeval tv;

	if(sem == NULL)
		ERR_OUT("sem = %p\n", sem); 

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&wset);
	FD_SET( sem->fds[1], &wset);

	ret = select(sem->fds[1] + 1, NULL, &wset, NULL, &tv);
	if (ret == 0)
		goto Err;
	if(ret <= 0)
		ERR_OUT("select ret = %d\n", ret);

	ret = write(sem->fds[1], buf, 1);
	if (ret != 1)
		ERR_OUT("ret = %d\n", ret);

	return 0;
Err:
	return -1;
}

int mid_sem_give0(mid_sem_t sem, int type)
{
	int ret;
	char buf[1];

	fd_set wset;
	struct timeval tv;

	if(sem == NULL)
		ERR_OUT("sem is NULL!\n"); 
	if(type > 127 || type < -128)
		ERR_OUT("type = %d\n", type); 

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	buf[0] = (char)type;
	
	FD_ZERO(&wset);
	FD_SET( sem->fds[1], &wset);

	ret = select(sem->fds[1] + 1, NULL, &wset, NULL, &tv);
	if (ret == 0)
		goto Err;
	if(ret <= 0)
		ERR_OUT("select ret = %d\n", ret);

	ret = write(sem->fds[1], buf, 1);
	if (ret != 1)
		ERR_OUT("ret = %d\n", ret);

	return 0;
Err:
	return -1;
}

void mid_sem_delete(mid_sem_t sem)
{
	if(sem == NULL)
		ERR_OUT("sem = %p\n", sem); 

	close(sem->fds[0]);
	close(sem->fds[1]);
	free(sem);

Err:
	return;
}
