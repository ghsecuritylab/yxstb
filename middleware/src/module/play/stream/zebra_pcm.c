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
	ZebraPCM zpcm;
} ZebraPcmArg;

static void local_state(struct LOCAL *local, int state, int scale)
{
	local->state = state;
	local->scale = scale;
	stream_post_state(local->index, state, scale);
}

static void local_time_sync(void *arg)
{
}

static int local_open_play(struct LOCAL *local, PlayArg *arg, ZebraPcmArg *zebraarg)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	local->totaltime = 0;
	local->currenttime = 0;
	local->totalbyte = 0;
	local->currentbyte = 0;

	local->maxfd = local->msgfd + 1;

	codec_zebra_pcm_open(0, &zebraarg->zpcm, 0);

	ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_1000MS, INTERVAL_CLK_1000MS, local_time_sync, local);
	local_state(local, STRM_STATE_PLAY, 1);

	return 0;
}

static void local_close_play(struct LOCAL *local)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	codec_zebra_pcm_close(0);

	local_state(local, STRM_STATE_CLOSE, 0);
}

static void local_resume(struct LOCAL *local)
{
	switch(local->state) {
	case STRM_STATE_PAUSE:
		break;
	default:
		LOG_STRM_ERROUT("#%d state = %d\n", local->index, local->state);
	}

	ymm_audio_setPausedPCMStream(0, 0);

Err:
	return;
}

static void local_pause(struct LOCAL *local)
{
	switch(local->state) {
	case STRM_STATE_PLAY:
		break;
	default:
		LOG_STRM_ERROUT("#%d state = %d\n", local->index, local->state);
	}

	ymm_audio_setPausedPCMStream(0, 1);

	local_state(local, STRM_STATE_PAUSE, 0);

Err:
	return;
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

	if (local_open_play(local, arg, (ZebraPcmArg *)argbuf))
		LOG_STRM_ERROUT("#%d pvr_recplay_local_open\n", local->index);

	local_loop(local);
	LOG_STRM_PRINTF("#%d exit local_loop!\n", local->index);

Err:
	return;
}

static int local_argparse_play(int idx, PlayArg* arg, char *argbuf, const char* url, int shiftlen, int begin, int end)
{
	ZebraPCM* zpcm;
	ZebraPcmArg *zebraarg = (ZebraPcmArg *)argbuf;

	if (url == NULL)
		LOG_STRM_ERROUT("#%d url is NULL\n", idx);

	zpcm = (ZebraPCM*)url;
	if (zpcm->size != sizeof(ZebraPCM))
		LOG_STRM_ERROUT("#%d size = %d / %d\n", idx, zpcm->size, sizeof(ZebraPCM));

	zebraarg->zpcm = *zpcm;

	return 0;
Err:
	return -1;
}

int zebra_pcm_create_stream(StreamCtrl *ctrl)
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

	ctrl->argsize = sizeof(ZebraPcmArg);
	ctrl->argparse_play = local_argparse_play;

	return 0;
Err:
	ctrl->handle = 0;
	return -1;
}
