
#ifdef INCLUDE_PVR

#include "stream.h"
#include "stream_port.h"


typedef struct _AsyncMsg AsyncMsg;
typedef struct _AsyncMsg* AsyncMsg_t;
struct _AsyncMsg {
	AsyncMsg_t next;

	int cmd;
	union {
		char path[PVR_PATH_LEN];
		struct {
			uint32_t id;
			PvrMsgCall call;
		} pvr;
	} u;
};

static AsyncMsg_t g_queue = NULL;
static mid_msgq_t g_msgq = NULL;
static mid_mutex_t g_mutex = NULL;

static void strm_async_loop(void *handle)
{
	AsyncMsg_t msg;
	int cmd, msgfd;
	fd_set rset;
	struct timeval tv;

	msgfd = mid_msgq_fd(g_msgq);

	LOG_STRM_PRINTF("tid = 0x%x\n", (uint32_t)pthread_self());

	while(1) {
		FD_ZERO(&rset);
		FD_SET((uint32_t)msgfd, &rset);
		tv.tv_sec = 3600 * 24;
		tv.tv_usec = 0;
		if (select(msgfd + 1, &rset , NULL,  NULL, &tv) != 1)
			continue;

		mid_msgq_getmsg(g_msgq, (char *)(&cmd));

		while (g_queue) {
			mid_mutex_lock(g_mutex);
			msg = g_queue;
			g_queue = msg->next;
			msg->next = NULL;
			mid_mutex_unlock(g_mutex);

			switch(msg->cmd) {
			case ASYNC_CMD_MOUNT:
				LOG_STRM_PRINTF("ASYNC_CMD_MOUNT\n");
				{
					uint32_t clk = mid_10ms( );
					if (ind_pvr_mount(clk)) {
						LOG_STRM_ERROR("RECORD_MSG_MOUNT_FAILED\n");
						msg->u.pvr.call(0, ASYNC_MSG_MOUNT_FAILED);
					} else {
						LOG_STRM_PRINTF("RECORD_MSG_MOUNT_SUCCESS\n");
						msg->u.pvr.call(0, ASYNC_MSG_MOUNT_SUCCESS);
					}
				}
				break;
			case ASYNC_CMD_UNMOUNT:
				LOG_STRM_PRINTF("ASYNC_CMD_UNMOUNT\n");
				{
					uint32_t clk = mid_10ms( );
					ind_pvr_unmount(clk);
					msg->u.pvr.call(0, ASYNC_MSG_UNMOUNT_FINISH);
				}
				break;
			case ASYNC_CMD_DELETE_PVR:
				LOG_STRM_PRINTF("ASYNC_CMD_DELETE_PVR id = 0x%08x\n", msg->u.pvr.id);
				ind_pvr_delete(msg->u.pvr.id, msg->u.pvr.call);
				break;
			case ASYNC_CMD_DELETE_DIR:
				LOG_STRM_PRINTF("ASYNC_CMD_DELETE_DIR path = %s\n", msg->u.path);
				record_port_dir_delete(msg->u.path);
				break;
			case ASYNC_CMD_DELETE_FILE:
				LOG_STRM_PRINTF("ASYNC_CMD_DELETE_FILE path = %s\n", msg->u.path);
				record_port_file_delete(msg->u.path);
				break;
			default:
				LOG_STRM_WARN("msg = %d\n", msg->cmd);
				break;
			}

			IND_FREE(msg);
		}
	}
}

void strm_async_init(void)
{
	if (g_mutex)
		return;

	g_mutex = mid_mutex_create( );
	g_msgq = mid_msgq_create(10, sizeof(int));

	stream_port_task_create(strm_async_loop, NULL);
}

void strm_async_cmd(uint32_t id, int cmd, PvrMsgCall call)
{
	AsyncMsg_t msg = (AsyncMsg_t)IND_CALLOC(sizeof(AsyncMsg), 1);
	if (NULL == msg)
		LOG_STRM_ERROUT("malloc AsyncMsg!\n");

	msg->cmd = cmd;
	msg->u.pvr.id = id;
	msg->u.pvr.call = call;

	mid_mutex_lock(g_mutex);
	msg->next = g_queue;
	g_queue = msg;
	mid_mutex_unlock(g_mutex);

	mid_msgq_putmsg(g_msgq, (char*)&cmd);

Err:
	return;
}

void strm_async_delete(int cmd, char *path)
{
	AsyncMsg_t msg;

	if (NULL == path)
		LOG_STRM_ERROUT("path is NULL\n");
	if (strlen(path) >= PVR_PATH_LEN)
		LOG_STRM_ERROUT("path invalid!\n");

	msg = (AsyncMsg_t)IND_CALLOC(sizeof(AsyncMsg), 1);
	if (NULL == msg)
		LOG_STRM_ERROUT("malloc AsyncMsg!\n");

	msg->cmd = cmd;
	IND_STRCPY(msg->u.path, path);

	mid_mutex_lock(g_mutex);
	msg->next = g_queue;
	g_queue = msg;
	mid_mutex_unlock(g_mutex);

	mid_msgq_putmsg(g_msgq, (char*)&cmd);

Err:
	return;
}

#endif//INCLUDE_PVR
