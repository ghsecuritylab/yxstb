/**********************************************************
	Copyright (c) 2008-2009, Yuxing Software Corporation
	All Rights Reserved
	Confidential Property of Yuxing Softwate Corporation

	Revision History:

	Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#include "stream.h"


#include "ind_gfx.h"

#include "libzebra.h"

struct LOCAL {
	int				index;
	STRM_STATE		state;
	int				scale;

	uint32_t			totaltime;
	uint32_t			currenttime;
	long long		totalbyte;
	long long		currentbyte;

	mid_msgq_t		msgq;
	int				msgfd;
	int				maxfd;

	ind_tlink_t		tlink;
};

typedef struct {
	char url[STREAM_URL_SIZE];
} ZebraArg;

extern void* ymm_stream_handle_get(int index);

extern int ymm_stream_playerSeekToOffset(int index, long long offset);
extern int ymm_stream_getOffset(int stream_handle, long long *offset);
extern int ymm_stream_getLength(int stream_handle, long long *length);

static void local_state(struct LOCAL *local, int state, int scale)
{
	local->state = state;
	local->scale = scale;
	stream_post_state(local->index, state, scale);
}

static void local_time_sync(void *arg)
{
	int handle;
	uint32_t totaltime, currenttime;
	long long totalbyte, currentbyte;
	struct LOCAL* local = (struct LOCAL*)arg;

	totaltime = 0;
	currenttime = 0;
	totalbyte = 0;
	currentbyte = 0;

	ymm_stream_playerGetTotalTime(0, &totaltime);
	totaltime /= 1000;
	ymm_stream_playerGetPlaytime(0, &currenttime);
	currenttime /= 1000;
	LOG_STRM_PRINTF("#%d totaltime = %u/%u, currenttime = %u/%u\n", local->index, totaltime, local->totaltime, currenttime, local->currenttime);

	handle = (int)ymm_stream_handle_get(0);
	ymm_stream_getLength(handle, &totalbyte);
	ymm_stream_getOffset(handle, &currentbyte);

	if (totaltime && local->totaltime != totaltime) {
		local->totaltime = totaltime;
		stream_back_totaltime(local->index, totaltime);
	}
	if (local->currenttime != currenttime) {
		local->currenttime = currenttime;
		stream_back_currenttime(local->index, currenttime);
	}
	if (totalbyte && local->totalbyte != totalbyte) {
		local->totalbyte = totalbyte;
		stream_back_totalbyte(local->index, totalbyte);
	}
	if (local->currentbyte != currentbyte) {
		local->currentbyte = currentbyte;
		stream_back_currentbyte(local->index, currentbyte);
	}
}

static int local_open_play(struct LOCAL *local, PlayArg *arg, ZebraArg *zebraarg)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	local->totaltime = 0;
	local->currenttime = 0;
	local->totalbyte = 0;
	local->currentbyte = 0;

	local->maxfd = local->msgfd + 1;

	codec_zebra_open(0, zebraarg->url, 0);

	ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_1000MS, INTERVAL_CLK_1000MS, local_time_sync, local);
	local_state(local, STRM_STATE_PLAY, 1);

	return 0;
}

static void local_close_play(struct LOCAL *local)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	codec_zebra_close(0);

	local_state(local, STRM_STATE_CLOSE, 0);
}

static void local_resume(struct LOCAL *local)
{
	LOG_STRM_PRINTF("#%d ymm_stream_playerSetTrickMode YX_NORMAL_PLAY\n", local->index);
	ymm_stream_playerSetTrickMode(0, YX_NORMAL_PLAY, -1);

	local_state(local, STRM_STATE_PLAY, 1);
}

static void local_pause(struct LOCAL *local)
{
	LOG_STRM_PRINTF("#%d ymm_stream_playerSetTrickMode YX_PAUSE\n", local->index);
	ymm_stream_playerSetTrickMode(0, YX_PAUSE, -1);

	local_state(local, STRM_STATE_PAUSE, 0);
}

static void local_fast(struct LOCAL *local, int arg)
{
	if (arg < 0) {
		LOG_STRM_PRINTF("#%d ymm_stream_playerSetTrickMode YX_FAST_REW\n", local->index);
		ymm_stream_playerSetTrickMode(0, YX_FAST_REW, arg);
	} else {
		LOG_STRM_PRINTF("#%d ymm_stream_playerSetTrickMode YX_FAST_FORWARD\n", local->index);
		ymm_stream_playerSetTrickMode(0, YX_FAST_FORWARD, arg);
	}

	local_state(local, STRM_STATE_FAST, arg);
}

static void local_seek(struct LOCAL *local, int arg)
{
    if (0 == local->totaltime) {
        ymm_stream_playerGetTotalTime(0, &local->totaltime);
        local->totaltime /= 1000;
    }
	LOG_STRM_PRINTF("#%d ymm_stream_seekToMs second = %d / %d\n", local->index, arg, local->totaltime);
//	if (arg == local->totaltime && local->totaltime >= 3) {
//	    LOG_STRM_PRINTF("#%d SEEK_END\n", local->index);
//	    arg -= 3;
//	}
	ymm_stream_playerSetTrickMode(0, YX_SEEK, arg * 1000);

	local_state(local, STRM_STATE_PLAY, 1);
	ind_timer_create(local->tlink, mid_10ms( ) + 180, INTERVAL_CLK_1000MS, local_time_sync, local);
}

static void local_lseek(struct LOCAL *local, int arg0, int arg1)
{
	uint32_t off0, off1;
	long long offset;

	off0 = (uint32_t)arg0;
	off1 = (uint32_t)arg1;
	offset = (long long)(((uint64_t)off0 << 32) + (uint64_t)off1);

	LOG_STRM_PRINTF("#%d ymm_stream_playerSeekToOffset offset = %lld\n", local->index, offset);

	ymm_stream_playerSeekToOffset(0, offset);

	local_state(local, STRM_STATE_PLAY, 1);
}

static void local_trickmode(struct LOCAL* local, int cmd, int arg0, int arg1)
{
	switch(cmd) {
	case STREAM_CMD_RESUME:
		LOG_STRM_PRINTF("#%d STREAM_CMD_RESUME\n", local->index);
		local_resume(local);
		break;

	case STREAM_CMD_PAUSE:
		LOG_STRM_PRINTF("#%d STREAM_CMD_PAUSE\n", local->index);
		local_pause(local);
		break;

	case STREAM_CMD_FAST:
		LOG_STRM_PRINTF("#%d STREAM_CMD_FAST\n", local->index);
		local_fast(local, arg0);
		break;

	case STREAM_CMD_STOP:
		LOG_STRM_PRINTF("#%d STREAM_CMD_STOP\n", local->index);
	    local_seek(local, local->totaltime);
        break;

	case STREAM_CMD_SEEK:
		LOG_STRM_PRINTF("#%d STREAM_CMD_SEEK\n", local->index);
		local_seek(local, arg0);
		break;

	case STREAM_CMD_LSEEK:
		LOG_STRM_PRINTF("#%d STREAM_CMD_LSEEK\n", local->index);
		local_lseek(local, arg0, arg1);
		break;

	default:
		LOG_STRM_ERROR("#%d cmd = %d\n", local->index, cmd);
		break;
	}
}

static void local_cmd(struct LOCAL *local, StreamCmd* strmCmd)
{
	int cmd = strmCmd->cmd;

	if (stream_deal_cmd(local->index, strmCmd) == 1)
		return;

	switch(cmd) {
	case STREAM_CMD_RESUME:
	case STREAM_CMD_PAUSE:
	case STREAM_CMD_FAST:
	case STREAM_CMD_SEEK:
	case STREAM_CMD_LSEEK:
	case STREAM_CMD_STOP:
		{
			int cmdsn = strmCmd->arg3;
			local_trickmode(local, cmd, strmCmd->arg0, strmCmd->arg1);
			if (cmdsn)
				stream_back_cmd(local->index, cmdsn);
		}
		break;

	case STREAM_CMD_CLOSE:
		LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE\n", local->index);
		local_close_play(local);
		return;

	default:
		LOG_STRM_ERROUT("#%d Unkown CMD %d\n", local->index, cmd);
	}

Err:
	return;
}

static void local_loop(struct LOCAL* local)
{
	uint32_t			clk, clks, out;
	fd_set			rset;
	struct timeval	tv;

	while (local->state != STRM_STATE_CLOSE) {

		clk = mid_10ms( );
		out = ind_timer_clock(local->tlink);
		if (out <= clk) {
			ind_timer_deal(local->tlink, clk);
			continue;
		}

		clks = out - clk;
		tv.tv_sec = clks / 100;
		tv.tv_usec = clks % 100 * 10000;

		FD_ZERO(&rset);
		FD_SET(local->msgfd, &rset);

		if (select(local->maxfd, &rset , NULL,  NULL, &tv) <= 0)
			continue;

		if (FD_ISSET(local->msgfd, &rset)) {
			StreamCmd strmCmd;

			memset(&strmCmd, 0, sizeof(strmCmd));
			mid_msgq_getmsg(local->msgq, (char *)(&strmCmd));
			local_cmd(local, &strmCmd);
			continue;
		}
	}
	ind_timer_delete_all(local->tlink);
}

static void local_loop_play(void *handle, int idx, mid_msgq_t msgq, PlayArg *arg, char* argbuf)
{
	struct LOCAL * local = (struct LOCAL *)handle;

	if (local == NULL)
		LOG_STRM_ERROUT("#%d local is NULL\n", local->index);

	local->index = idx;
	local->msgq = msgq;
	local->msgfd = mid_msgq_fd(local->msgq);
	local->tlink = int_stream_tlink(idx);

	if (local_open_play(local, arg, (ZebraArg *)argbuf))
		LOG_STRM_ERROUT("#%d pvr_recplay_local_open\n", local->index);

	local_loop(local);
	LOG_STRM_PRINTF("#%d exit local_loop!\n", local->index);

Err:
	return;
}

static int local_argparse_play(int idx, PlayArg* arg, char *argbuf, const char* url, int shiftlen, int begin, int end)
{
	ZebraArg *zebraarg = (ZebraArg *)argbuf;

	if (url == NULL)
		LOG_STRM_ERROUT("#%d url is NULL\n", idx);

	strcpy(zebraarg->url, url);
    if (!strncasecmp(zebraarg->url, "http://", 7))
        memcpy(zebraarg->url, "http", 4);

	return 0;
Err:
	return -1;
}

int zebra_create_stream(StreamCtrl *ctrl)
{
	struct LOCAL *local;

	local = (struct LOCAL *)IND_MALLOC(sizeof(struct LOCAL));
	if (local == NULL)
		LOG_STRM_ERROUT("malloc failed!\n");
	IND_MEMSET(local, 0, sizeof(struct LOCAL));

	local->state = STRM_STATE_CLOSE;

	ctrl->handle = local;

	ctrl->loop_play = local_loop_play;
	ctrl->loop_record = NULL;

	ctrl->argsize = sizeof(ZebraArg);
	ctrl->argparse_play = local_argparse_play;

	return 0;
Err:
	ctrl->handle = 0;
	return -1;
}
