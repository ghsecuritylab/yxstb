
#include <unistd.h>
#include "mid_msgq.h"
#include "nm_dbg.h"

struct mid_msgq {
	int size;
	int fds[2];
};

mid_msgq_t mid_msgq_create(int msg_num, int msg_size)
{
	mid_msgq_t msgq;

	msgq = (mid_msgq_t)malloc(sizeof(struct mid_msgq));
	if(msgq == NULL){
		nm_msg_level(LOG_DEBUG, "malloc\n");
		goto Err;
	}

	if(pipe(msgq->fds)) {
		free(msgq);
		nm_msg_level(LOG_DEBUG, "pipe\n");
		goto Err;
	}
	//ind_net_noblock(msgq->fds[0]);
	//ind_net_noblock(msgq->fds[1]);
	msgq->size = msg_size;

	return msgq;
Err:
	return NULL;
}

int mid_msgq_delete( mid_msgq_t msgq)
{
	if(msgq == NULL)
		return -1;
	close(msgq->fds[0]);
	close(msgq->fds[1]);
	free(msgq);
	return 0;
}

int mid_msgq_fd(mid_msgq_t msgq)
{
	if(msgq == NULL){
		nm_msg_level(LOG_DEBUG, "msgq = %p\n", msgq);
		goto Err;
	}
	return msgq->fds[0];
Err:
	return -1;
}

int mid_msgq_put(mid_msgq_t msgq, char *msg, int sec)
{
	int ret;
	fd_set wset;
	struct timeval tv;

	if(msgq == NULL || sec < 0){
		nm_msg("msgq = %p, sec = %d\n", msgq, sec);
		goto Err;
	}
	
	tv.tv_sec = sec;
	tv.tv_usec = 0;

	FD_ZERO(&wset);
	FD_SET( msgq->fds[1], &wset);

	ret = select(msgq->fds[1] + 1, NULL, &wset, NULL, &tv);
	if(ret <= 0){
		nm_msg("select ret = %d\n", ret);
		goto Err;
	}
	ret = write(msgq->fds[1], msg, msgq->size);
	if (ret != msgq->size){
		nm_msg("ret = %d, msgq->size = %d\n", ret, msgq->size);
		goto Err;
	}

	return 0;
Err:
	return -1;
}

int mid_msgq_get(mid_msgq_t msgq, char *msg, int sec, int usec)
{
	int ret;

	if(msgq == NULL || sec < 0 || usec < 0){
		nm_msg("msgq = %p, sec = %d, usec = %d\n", msgq, sec, usec);
		goto Err;
	}

	if (sec > 0 || usec > 0) {
		fd_set rset;
		struct timeval tv;

		tv.tv_sec = sec;
		tv.tv_usec = usec;

		FD_ZERO(&rset);
		FD_SET( msgq->fds[0], &rset);

		ret = select(msgq->fds[0] + 1, &rset, NULL, NULL, &tv);
		if (ret == 0)
			return -1;
		if(ret <= 0){
			nm_msg("select ret = %d\n", ret);
		}
	}
	ret = read(msgq->fds[0], msg, msgq->size);
	if (ret != msgq->size){
		nm_msg("ret = %d, msgq->size = %d\n", ret, msgq->size);
		goto Err;
	}
	return msgq->size;
Err:
	return -1;
}

int mid_msgq_putmsg(mid_msgq_t msgq, char *msg)
{
	int ret;

	if(msgq == NULL){
		nm_msg("msgq = %p\n", msgq);
		goto Err;
	}

	ret = write(msgq->fds[1], msg, msgq->size);
	if (ret != msgq->size){
		nm_msg("ret = %d, msgq->size = %d\n", ret, msgq->size);
		goto Err;
	}
	return 0;
Err:
	return -1;
}

int mid_msgq_getmsg(mid_msgq_t msgq, char *msg)
{
	int ret;

	if(msgq == NULL){
		nm_msg("msgq = %p\n", msgq);
		goto Err;
	}

	ret = read(msgq->fds[0], msg, msgq->size);
	if (ret != msgq->size){
		nm_msg("ret = %d, msgq->size = %d\n", ret, msgq->size);
		goto Err;
	}

	return msgq->size;
Err:
	return -1;
}
