/**********************************************************
	Copyright (c) 2008-2009, Yuxing Software Corporation
	All Rights Reserved
	Confidential Property of Yuxing Softwate Corporation

	Revision History:

	Created: 2009-10-22 9:30:31 by liujianhua

	±æµÿ“Ù¿÷≤•∑≈

 **********************************************************/
#include <sys/select.h>

#include "stream.h"
#include "stream_port.h"

#define INTERVAL_CLK_MESSAGE			500

struct LOCAL
{
	int					index;
	STRM_STATE			state;
	uint32_t				clk;
	ind_tlink_t			tlink;
	int					time_length;
	int					time_current;

    StrmBuffer*         sb;

	int					push_len;
	int					push_end;
	int					push_msgno;

	int					loop_flg;

	mid_msgq_t			msgq;
	int					msgfd;

	void*				local_fp;

	StreamPlay*	        strm_play;

	struct ts_audio*	ts_audio;
};

typedef struct {
	char url[STREAM_URL_SIZE];
	int loop_flg;
} AudioArg;

static void local_msg_back(void *handle, STRM_MSG msgno, int arg);
static void local_push(void *arg);

static void local_set_totaltime(struct LOCAL *local, uint32_t length);
static void local_set_currenttime(struct LOCAL *local, uint32_t current);

static void local_state(struct LOCAL *local, STRM_STATE state, int scale)
{
	local->state = state;
	stream_post_state(local->index, state, scale);
}

static void local_sync(void *arg)
{
	int sec, length;
	struct LOCAL* local = (struct LOCAL*)arg;

	if (local->state != STRM_STATE_PLAY)
		return;

	length = local->time_length;
	sec = strm_play_time(local->strm_play) / 100;

	LOG_STRM_DEBUG("#%d: length = %d, sec = %d\n", local->index, length, sec);

	if (sec < 0)
		sec = 0;
	else if (sec > length)
		sec = length;

	local_set_currenttime(local, (uint32_t)sec);
}

static int local_open(struct LOCAL *local, PlayArg *arg, AudioArg *audarg)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	local_set_totaltime(local, mid_stream_auido_length(audarg->url));

	ts_audio_reset(local->ts_audio);

	local->local_fp = stream_port_fopen(audarg->url, "rb");
	if (local->local_fp == NULL)
		LOG_STRM_ERROUT("#%d local_port_fopen\n", local->index);
	strm_play_open(local->strm_play, local->index, local, local_msg_back, APP_TYPE_AUDIO, 1316);
	strm_play_resume(local->strm_play, local->index, 0);

	local->loop_flg = audarg->loop_flg;

	ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_100MS, INTERVAL_CLK_100MS, local_push, local);
	ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_1000MS, INTERVAL_CLK_1000MS, local_sync, local);

	local->push_end = 0;
	local->push_len = 0;
	local->push_msgno = 0;

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

static int local_close(struct LOCAL *local)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	if (local->local_fp) {
		stream_port_fclose(local->local_fp);
		local->local_fp = NULL;
	}
	strm_play_close(local->strm_play, local->index, 0);

	ind_timer_delete(local->tlink, local_push, local);

	local_state(local, STRM_STATE_CLOSE, 0);

	return 0;
}

static void local_msg_back(void *handle, STRM_MSG msgno, int arg)
{
	struct LOCAL *local = (struct LOCAL *)handle;
	local->push_msgno = msgno;
}

static void local_msg(struct LOCAL *local, int msgno)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	switch(msgno) {
	case STRM_MSG_PLAY_ERROR:
		LOG_STRM_PRINTF("#%d STRM_MSG_PLAY_ERROR > STRM_MSG_OPEN_ERROR\n", local->index);

		ind_timer_delete(local->tlink, local_push, local);

		stream_post_msg(local->index, STRM_MSG_OPEN_ERROR, 0);
		break;
	case STRM_MSG_STREAM_END:
		LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", local->index);
		if (local->loop_flg) {
			LOG_STRM_PRINTF("#%d stream_port_fseek 0\n", local->index);
			if (stream_port_fseek(local->local_fp, 0))
				LOG_STRM_ERROUT("#%d stream_port_fseek!\n", local->index);
			ts_audio_buf_clr(local->ts_audio);
			strm_play_reset(local->strm_play, local->index, 1);
			strm_play_resume(local->strm_play, local->index, 0);
			local_sync(local);

			local->push_len = 0;
			local->push_end = 0;
			ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_100MS, INTERVAL_CLK_100MS, local_push, local);
			ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_1000MS, INTERVAL_CLK_1000MS, local_sync, local);
		} else {
			ind_timer_delete(local->tlink, local_sync, local);
			local_set_currenttime(local, local->time_length);
			stream_post_msg(local->index, STRM_MSG_STREAM_END, 0);
		}
		break;
	case STRM_MSG_STREAM_MUSIC:
	case STRM_MSG_STREAM_VIDEO:
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

static void local_cmd(struct LOCAL* local, StreamCmd* strmCmd)
{
	int cmd = strmCmd->cmd;

	LOG_STRM_PRINTF("#%d %d %d\n", local->index, local->state, cmd);

	if (stream_deal_cmd(local->index, strmCmd) == 1)
		return;

	switch(cmd) {
	case STREAM_CMD_PAUSE:
		LOG_STRM_PRINTF("#%d STREAM_CMD_PAUSE\n", local->index);
		local_pause(local);
		break;
	case STREAM_CMD_RESUME:
		LOG_STRM_PRINTF("#%d STREAM_CMD_RESUME\n", local->index);
		local_resume(local);
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
	int buffer, len, ret;
	char *buf;
	StrmBuffer* sb;
	struct LOCAL *local = (struct LOCAL *)arg;

	if (local->push_msgno) {
		local_msg(local, local->push_msgno);
		local->push_msgno = 0;
		return;
	}
	if (local->push_end == 1)
		return;

	for (;;) {
		buffer = strm_play_buffer(local->strm_play);
		if (buffer >= 200)
			return;
		static int err = 0;

		if (err >= 3)
			LOG_STRM_ERROUT("#%d ts_audio_read\n", local->index);

        sb = local->sb;
		ret = ts_audio_read(local->ts_audio, sb->buf, 1316);
		if (ret < 0) {
			err ++;
			LOG_STRM_WARN("#%d ts_audio_read\n", local->index);
			ts_audio_reset(local->ts_audio);
			continue;
		}
		if (ret > 0) {
	        sb->off = 0;
		    sb->len = ret;
			strm_play_push(local->strm_play, local->index, &local->sb);
			if (local->push_msgno) {
				local_msg(local, local->push_msgno);
				local->push_msgno = 0;
				return;
			}
			continue;
		}

		if (ts_audio_buf_get(local->ts_audio, &buf, &len)) {
			LOG_STRM_WARN("#%d ts_audio_buf_get\n", local->index);
			ts_audio_reset(local->ts_audio);
			continue;
		}

		if (len <= 0) {
			LOG_STRM_WARN("#%d len = %d\n", local->index, len);
			continue;
		}

		ret = stream_port_fread(local->local_fp, buf, len);

		if (ret == 0) {
			if (local->push_len <= 0)
				LOG_STRM_ERROUT("#%d stream_port_fread\n", local->index);

			local->push_end = 1;
			strm_play_end(local->strm_play, local->index);
			return;
		}
		if (ret <= 0)
			LOG_STRM_ERROUT("#%d rtsp_port_fread ret = %d\n", local->index, ret);

		local->push_len += ret;
		if (ts_audio_buf_put(local->ts_audio, ret)) {
			LOG_STRM_WARN("#%d ts_audio_buf_put\n", local->index);
			ts_audio_reset(local->ts_audio);
			continue;
		}

		err = 0;
	}

Err:
	ind_timer_delete(local->tlink, local_push, local);
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

			LOG_STRM_PRINTF("#%d cmd = %d\n", local->index, strmCmd.cmd);
		}
	}
}

static void local_loop_play(void *handle, int idx, mid_msgq_t msgq, PlayArg *arg, char *argbuf)
{
	struct LOCAL *local;

	local = (struct LOCAL *)IND_CALLOC(sizeof(struct LOCAL), 1);
	if (!local)
		LOG_STRM_ERROUT("#%d malloc failed!\n", idx);

    local->sb = strm_buf_malloc(1316);
	local->ts_audio = ts_audio_create( );
	if (!local->ts_audio)
		LOG_STRM_ERROUT("#%d ts_audio_create", idx);

	local->index = idx;
	local->msgq = msgq;
	local->msgfd = mid_msgq_fd(local->msgq);
	local->tlink = int_stream_tlink(idx);
	local->strm_play = int_strm_play(idx);

	if (local_open(local, arg, (AudioArg *)argbuf))
		LOG_STRM_ERROUT("#%d local_open\n", idx);

	local_loop(local);
	LOG_STRM_PRINTF("#%d exit local_loop!\n", idx);

Err:
	if (local) {
	    if (local->sb)
	        strm_buf_free(local->sb);
		if (local->ts_audio)
			ts_audio_delete(local->ts_audio);
		IND_FREE(local);
	}
	return;
}

static void local_set_totaltime(struct LOCAL *local, uint32_t length)
{
	local->time_length = length;
	stream_back_totaltime(local->index, length);
}

static void local_set_currenttime(struct LOCAL *local, uint32_t current)
{
	local->time_current = current;
	stream_back_currenttime(local->index, current);
}

static int local_argparse_play(int idx, PlayArg* arg, char *argbuf, const char* url, int shiftlen, int begin, int end)
{
	AudioArg *audarg = (AudioArg *)argbuf;

	if (url == NULL)
		LOG_STRM_ERROUT("#%d url is NULL\n", idx);

	IND_STRCPY(audarg->url, url);
	audarg->loop_flg = shiftlen;

	return 0;
Err:
	return -1;
}

int audio_create_stream(StreamCtrl *ctrl)
{
	ctrl->handle = ctrl;

	ctrl->loop_play = local_loop_play;

	ctrl->argsize = sizeof(AudioArg);
	ctrl->argparse_play = local_argparse_play;

	return 0;
}

