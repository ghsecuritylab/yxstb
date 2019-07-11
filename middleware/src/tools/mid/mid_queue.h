#ifndef __MID_QUEUE_H__
#define __MID_QUEUE_H__

typedef struct mid_queue* mid_queue_t;

mid_queue_t mid_queue_create(int msg_num, int msg_size);
void mid_queue_delete(mid_queue_t queue);

int mid_queue_put(mid_queue_t queue, char *msg);
int mid_queue_get(mid_queue_t queue, char *msg, int usec);

#endif /* __MID_QUEUE_H__ */
