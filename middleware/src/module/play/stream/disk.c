/**********************************************************
	Copyright (c) 2008-2009, Yuxing Software Corporation
	All Rights Reserved
	Confidential Property of Yuxing Softwate Corporation

	Revision History:

	Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#include "stream.h"

#ifdef ENABLE_DISK

#define INTERVAL_CLK_MESSAGE			500

struct LOCAL
{
	int					index;
	STRM_STATE			state;
	uint32_t				clk;
	ind_tlink_t			tlink;

	long long			byte_length;

	int					time_base;
	int					time_length;
	int					time_current;

	char				push_buf[TS_BLOCK_SIZE];
	uint32_t				push_len;
	int					push_end;
	int					push_msgno;

	int					psi_flag_play;

	mid_msgq_t			msgq;
	int					msgfd;

	void*				local_fp;

	StreamPlay*	strm_play;
};

typedef struct {
	char	url[STREAM_URL_SIZE];
} DiskArg;

static void local_msg_back(void *handle, int msgno, int arg);
static void local_push(void *arg);

static void local_set_currenttime(struct LOCAL *local, uint32_t current);

static void local_state(struct LOCAL *local, STRM_STATE state, int scale)
{
	local->state = state;
	//stream_post_state(local->index, state, scale);
}

static void local_sync(void *arg)
{
	int sec;
	struct LOCAL* local = (struct LOCAL*)arg;

	if (local->state != STRM_STATE_PLAY)
		return;

	sec = strm_play_time(local->strm_play, local->index) / 100;

	LOG_STRM_DEBUG("#%d: sec = %d\n", local->index, sec);

	if (sec < 0)
		sec = 0;

	local_set_currenttime(local, (uint32_t)sec);
}

static int local_open(struct LOCAL *local, PlayArg *arg, DiskArg* diskarg)
{
	local->time_base = 0;
	if (disk_port_fsize(diskarg->url, &local->time_length, &local->byte_length))
		LOG_STRM_ERROUT("#%d disk_port_fsize %s\n", local->index, diskarg->url);

	LOG_STRM_PRINTF("#%d time_length = %d, byte_length = %lld\n", local->index, local->time_length, local->byte_length);

	local->local_fp = disk_port_fopen(diskarg->url);
	if (local->local_fp == NULL)
		LOG_STRM_ERROUT("#%d disk_port_fopen %s\n", local->index, diskarg->url);
	strm_play_open(local->strm_play, local->index, local, local_msg_back, 0);
	strm_play_resume(local->strm_play, local->index, 0);

	ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_100MS, INTERVAL_CLK_100MS, local_push, local);
	ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_1000MS, INTERVAL_CLK_1000MS, local_sync, local);

	local->push_end = 0;
	local->push_len = 0;
	local->push_msgno = 0;
	local->psi_flag_play = 0;

	local_state(local, STRM_STATE_PLAY, 1);

	return 0;
Err:
	return -1;
}

static int local_pause(struct LOCAL *local)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	switch(local->state) {
	case STRM_STATE_PLAY:	break;
	default:				LOG_STRM_ERROUT("#%d %d\n", local->index, local->state);
	}

	strm_play_pause(local->strm_play, local->index);
	local_state(local, STRM_STATE_PAUSE, 0);
	if (local->push_end == 0)
		ind_timer_delete(local->tlink, local_push, local);
	ind_timer_delete(local->tlink, local_sync, local);

	return 0;
Err:
	return -1;
}

static int local_resume(struct LOCAL *local)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	switch(local->state) {
	case STRM_STATE_PAUSE:	break;
	default:				LOG_STRM_ERROUT("#%d %d\n", local->index, local->state);
	}

	strm_play_resume(local->strm_play, local->index, 0);
	local_state(local, STRM_STATE_PLAY, 1);
	if (local->push_end == 0)
		ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_100MS, INTERVAL_CLK_100MS, local_push, local);
	ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_1000MS, INTERVAL_CLK_1000MS, local_sync, local);

	return 0;
Err:
	return -1;
}

static int local_seek(struct LOCAL *local, int seek)
{
	long long offset;
	LOG_STRM_PRINTF("#%d seek = %d\n", local->index, seek);

	if (local->time_length <= 0 || local->byte_length <= 0)
		LOG_STRM_ERROUT("#%d time_length = %d, byte_length = %lld\n", local->index, local->time_length, local->byte_length);

	switch (local->state) {
	case STRM_STATE_PLAY:	break;
	case STRM_STATE_PAUSE:	break;
	default:				LOG_STRM_ERROUT("#%d %d\n", local->index, local->state);
	}

	if (seek < 0 || local->time_length <= 3)
		seek = 0;
	else if (seek > local->time_length - 3)
		seek = local->time_length - 3;
	offset = local->byte_length * seek / local->time_length;
	offset = offset - offset % 188;

	LOG_STRM_PRINTF("seek = %d, offset = %d\n", seek, offset);
	disk_port_fseek(local->local_fp, offset);

	strm_play_reset(local->strm_play, local->index, 1);
	strm_play_resume(local->strm_play, local->index, 0);
	local_state(local, STRM_STATE_PLAY, 1);
	ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_100MS, INTERVAL_CLK_100MS, local_push, local);
	ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_1000MS, INTERVAL_CLK_1000MS, local_sync, local);

	return 0;
Err:
	return -1;
}

static int local_close(struct LOCAL *local)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	if (local->local_fp) {
		disk_port_fclose(local->local_fp);
		local->local_fp = NULL;
	}
	strm_play_close(local->strm_play, local->index, 1);

	ind_timer_delete(local->tlink, local_push, local);
	ind_timer_delete(local->tlink, local_sync, local);

	local_state(local, STRM_STATE_CLOSE, 0);

	return 0;
}

static void local_msg_back(void *handle, int msgno, int arg)
{
	struct LOCAL *local = (struct LOCAL *)handle;
	local->push_msgno = msgno;
}

static void local_msg(struct LOCAL *local, int msgno)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	switch(msgno) {
	case STRM_MSG_PLAY_ERROR:
		LOG_STRM_PRINTF("#%d STRM_MSG_PLAY_ERROR psi_flag_play = %d\n", local->index, local->psi_flag_play);
		ind_timer_delete(local->tlink, local_push, local);
		//stream_post_msg(local->index, STRM_MSG_OPEN_ERROR, 0);
		break;
	case STRM_MSG_STREAM_END:
		LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", local->index);
		local_close(local);

		break;
	case STRM_MSG_STREAM_MUSIC:
	case STRM_MSG_STREAM_VIDEO:
		local->psi_flag_play = 1;
		break;
	case STRM_MSG_RECV_FIRST:
	case STRM_MSG_RECV_TIMEOUT:
	case STRM_MSG_RECV_RESUME:
	case STRM_MSG_PTS_VIEW:
		break;

	default:
		LOG_STRM_ERROUT("#%d msgno = %d\n", local->index, msgno);
	}

Err:
	return;
}

static void local_trickmode(struct LOCAL* local, int cmd, int arg)
{
	switch(cmd) {
	case STREAM_CMD_PAUSE:
		LOG_STRM_PRINTF("#%d STREAM_CMD_PAUSE\n", local->index);
		local_pause(local);
		break;
	case STREAM_CMD_RESUME:
		LOG_STRM_PRINTF("#%d STREAM_CMD_RESUME\n", local->index);
		local_resume(local);
		break;
	case STREAM_CMD_SEEK:
		LOG_STRM_PRINTF("#%d STREAM_CMD_SEEK\n", local->index);
		local_seek(local, arg);
		break;
	default:
		break;
	}
}

static void local_cmd(struct LOCAL* local, StreamCmd* strmCmd)
{
	int cmd = strmCmd->cmd;

	LOG_STRM_PRINTF("#%d %d %d\n", local->index, local->state, cmd);

	if (stream_deal_cmd(local->index, strmCmd) == 1)
		return;

	switch(cmd) {
	case STREAM_CMD_RESUME:
	case STREAM_CMD_PAUSE:
	case STREAM_CMD_FAST:
	case STREAM_CMD_SEEK:
	case STREAM_CMD_STOP:
		{
			int cmdsn = msg->arg3;
			local_trickmode(local, cmd, msg->arg0);
			if (cmdsn)
				stream_back_cmd(local->index, cmdsn);
		}
		break;

	case STREAM_CMD_CLOSE:
		LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE\n", local->index);
		local_close(local);
		break;
	default:
		LOG_STRM_PRINTF("#%d cmd = %d\n", local->index, cmd);
		break;
	}
}

static void local_push(void *arg)
{
	int diff, len;
	char *buf;
	struct LOCAL *local = (struct LOCAL *)arg;

	strm_play_loop(local->strm_play, local->index, local->clk);
	if (local->push_msgno) {
		local_msg(local, local->push_msgno);
		local->push_msgno = 0;
		return;
	}
	if (local->push_end == 1)
		return;

	for (;;) {
		diff = strm_play_last(local->strm_play, local->index) - strm_play_time(local->strm_play, local->index);
		if (diff >= 200)
			return;

		buf = local->push_buf;
		len = disk_port_fread(local->local_fp, buf, TS_BLOCK_SIZE);

		if (len == 0) {
			if (local->push_len == 0)
				LOG_STRM_ERROUT("#%d disk_port_fread\n", local->index);

			local->push_end = 1;
			strm_play_end(local->strm_play, local->index);
			return;
		}
		if (len < 0)
			LOG_STRM_ERROUT("#%d rtsp_port_fread ret = %d\n", local->index, len);
		local->push_len += len;

		if (strm_play_push(local->strm_play, local->index, local->push_buf, len, local->clk) < 0)
			LOG_STRM_ERROUT("#%d strm_play_push\n", local->index);
		strm_play_loop(local->strm_play, local->index, local->clk);
		if (local->push_msgno) {
			local_msg(local, local->push_msgno);
			local->push_msgno = 0;
			return;
		}
	}

	return;
Err:
	local_close(local);
	return;
}

static void local_loop(struct LOCAL *local)
{
	uint32_t			clk, clks, out;
	fd_set			rset;
	struct timeval	tv;

	while (local->state != STRM_STATE_CLOSE) {
		clk = mid_10ms( );
		local->clk = clk;
		out = ind_timer_clock(local->tlink);
		if (out <= clk) {
			ind_timer_deal(local->tlink, clk);
			continue;
		}

		clks = out - clk;
		tv.tv_sec = clks / 100;
		tv.tv_usec = clks % 100 * 10000;

		FD_ZERO(&rset);
		FD_SET((uint32_t)local->msgfd, &rset);
		if (select(local->msgfd + 1, &rset , NULL,  NULL, &tv) <= 0)
			continue;

		if (FD_ISSET((uint32_t)local->msgfd, &rset)) {
			StreamCmd strmCmd;

			LOG_STRM_PRINTF("#%d\n", local->index);
			memset(&strmCmd, 0, sizeof(strmCmd));
			mid_msgq_getmsg(local->msgq, (char *)(&strmCmd));

			local_cmd(local, &strmCmd);

			LOG_STRM_PRINTF("#%d cmd = %d\n", local->index, msg.cmd);
		}
	}
}

static void local_loop_play(void *handle, int indx, mid_msgq_t msgq, PlayArg *arg, char *argbuf)
{
	struct LOCAL *local;

	local = (struct LOCAL *)IND_MALLOC(sizeof(struct LOCAL));
	if (!local)
		LOG_STRM_ERROUT("#%d malloc failed!\n", indx);

	local->index = indx;
	local->msgq = msgq;
	local->msgfd = mid_msgq_fd(local->msgq);
	local->tlink = int_stream_tlink(indx);

	local->strm_play = int_strm_play(indx);

	if (local_open(local, arg, (DiskArg *)argbuf))
		LOG_STRM_ERROUT("#%d local_open\n", indx);

	local_loop(local);
	LOG_STRM_PRINTF("#%d exit local_loop!\n", indx);

Err:
	if (local)
		IND_FREE(local);

	return;
}

static void local_set_currenttime(struct LOCAL *local, uint32_t current)
{
	local->time_current = current;
	stream_back_currenttime(local->index, current);
}

static int local_argparse_play(int idx, PlayArg* arg, char *argbuf, char* url, int shiftlen, int begin, int end)
{
	DiskArg *diskarg = (DiskArg *)argbuf;

	if (url == NULL)
		LOG_STRM_ERROUT("#%d url is NULL\n", idx);

	IND_STRCPY(diskarg->url, url);

	return 0;
Err:
	return -1;
}

int disk_create_stream(StreamCtrl *ctrl)
{
	ctrl->handle = ctrl;

	ctrl->loop_play = local_loop_play;

	ctrl->argsize = sizeof(DiskArg);
	ctrl->argparse_play = local_argparse_play;

	return 0;
}

#endif//#ifdef ENABLE_DISK
