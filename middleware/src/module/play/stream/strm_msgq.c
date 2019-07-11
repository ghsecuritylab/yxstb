
#include "stream.h"

struct __StreamMsgQ {
	int				index;

	int				msg_size;
	StreamMsg*		msg_pool;
	StreamMsg*		msg_queue;
	mid_mutex_t		msg_mutex;
	StreamMsg		msg_array[1];
};

void int_msgq_delete(StreamMsgQ* msgq)
{
	if (msgq == NULL)
		return;

	if (msgq->msg_array)
		IND_FREE(msgq->msg_array);

	IND_FREE(msgq);
}

StreamMsgQ* strm_msgq_create(int size)
{
	StreamMsgQ* msgq = NULL;

	if (size <= 0 || size > 100)
		LOG_STRM_ERROUT("size = %d\n", size);

	msgq = (StreamMsgQ*)IND_MALLOC(sizeof(StreamMsgQ) + sizeof(StreamMsg) * size);
	if (msgq == NULL)
		LOG_STRM_ERROUT("malloc StreamMsgQ\n");
	IND_MEMSET(msgq, 0, sizeof(StreamMsgQ));
	msgq->msg_size = size;

	msgq->msg_mutex = mid_mutex_create( );
	if (msgq->msg_mutex == NULL)
		LOG_STRM_ERROUT("mid_mutex_create\n");

	return msgq;
Err:
	int_msgq_delete(msgq);
	return NULL;
}
/*
void strm_msgq_delete(StreamMsgQ* msgq)
{
	if (msgq == NULL)
		return;
	mid_mutex_delete(msgq->msg_mutex);
	int_msgq_delete(msgq);
}
 */
void strm_msgq_reset(StreamMsgQ* msgq, int idx)
{
	int i;
	StreamMsg *msg_array = msgq->msg_array;

	msgq->index = idx;

	mid_mutex_lock(msgq->msg_mutex);

	for (i = 1; i < msgq->msg_size; i ++)
		msg_array[i - 1].next = &msg_array[i];

	msg_array[msgq->msg_size - 1].next = NULL;
	msgq->msg_pool = &msg_array[0];

	msgq->msg_queue = NULL;

	mid_mutex_unlock(msgq->msg_mutex);
}

int strm_msgq_valid(StreamMsgQ* msgq)
{
	if (msgq->msg_queue)
		return 1;
	return 0;
}

int strm_msgq_pump(StreamMsgQ* msgq, StreamMsg* msg)
{
	int ret = 0;
	StreamMsg* m;

	mid_mutex_lock(msgq->msg_mutex);

	m = msgq->msg_queue;
	if (m) {
		msgq->msg_queue = m->next;
		m->next = NULL;

		IND_MEMCPY(msg, m, sizeof(StreamMsg));

		m->next = msgq->msg_pool;
		msgq->msg_pool = m;

		ret = 1;
	}

	mid_mutex_unlock(msgq->msg_mutex);
	return ret;
}

void strm_msgq_print(StreamMsgQ* msgq)
{
	int i;
	StreamMsg *q;

	mid_mutex_lock(msgq->msg_mutex);

	i = 0;
	q = msgq->msg_queue;
	while(q) {
		LOG_STRM_DEBUG("#%d msg[%d] = %d\n", msgq->index, i, q->msg);
		q = q->next;
		i ++;
	}

	mid_mutex_unlock(msgq->msg_mutex);
}

void strm_msgq_queue(StreamMsgQ* msgq, STRM_MSG msgno, int arg)
{
	StreamMsg *msg, *queue;

	mid_mutex_lock(msgq->msg_mutex);

	msg = msgq->msg_pool;
	if (msg == NULL)
		LOG_STRM_ERROUT("#%d msg pool empty!\n", msgq->index);

	queue = msgq->msg_queue;
	while(queue && queue->next)
		queue = queue->next;

	msg = msgq->msg_pool;
	msgq->msg_pool = msg->next;
	msg->next = NULL;

	msg->msg = msgno;
	msg->arg = arg;

	if (queue)
		queue->next = msg;
	else
		msgq->msg_queue = msg;

Err:
	mid_mutex_unlock(msgq->msg_mutex);
	return;
}
