/**********************************************************
	Copyright (c) 2008-2009, Yuxing Software Corporation
	All Rights Reserved
	Confidential Property of Yuxing Softwate Corporation

	Revision History:

	Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#include "stream.h"
#include "stream_port.h"

#include "ind_gfx.h"

#ifdef INCLUDE_FLASHPLAY

struct LOCAL {
	int				index;
	STRM_STATE		state;

	mid_msgq_t		msgq;
	int				msgfd;
	int				maxfd;

	ind_tlink_t		tlink;
};

typedef struct {
	char url[STREAM_URL_SIZE];
} FlashArg;

static int local_open_play(struct LOCAL *local, PlayArg *arg, FlashArg *flasharg)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	local->maxfd = local->msgfd + 1;

	if (codec_flash_open(flasharg->url)) {
		stream_port_message(0, FLASH_MSG_ERROR, 0, 0);
		LOG_STRM_ERROUT("#%d codec_mosaic_open\n", local->index);
	}

	local->state = STRM_STATE_IPTV;

	return 0;
Err:
	return -1;
}

static void local_close_play(struct LOCAL *local)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	codec_flash_close( );

	local->state = STRM_STATE_CLOSE;
}

static void local_cmd(struct LOCAL *local, StreamCmd* strmCmd)
{
	int cmd = strmCmd->cmd;

	if (stream_deal_cmd(local->index, strmCmd) == 1)
		return;

	switch(cmd) {
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
		FD_SET((uint32_t)local->msgfd, &rset);

		if (select(local->maxfd, &rset , NULL,  NULL, &tv) <= 0)
			continue;

		if (FD_ISSET((uint32_t)local->msgfd, &rset)) {
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
	struct LOCAL *local;

	local = (struct LOCAL *)IND_CALLOC(sizeof(struct LOCAL), 1);
	if (!local)
		LOG_STRM_ERROUT("#%d malloc failed!\n", idx);

	local->state = STRM_STATE_CLOSE;

	local->index = idx;
	local->msgq = msgq;
	local->msgfd = mid_msgq_fd(local->msgq);
	local->tlink = int_stream_tlink(idx);

	if (local_open_play(local, arg, (FlashArg *)argbuf))
		LOG_STRM_ERROUT("#%d pvr_recplay_local_open\n", idx);

	local_loop(local);
	LOG_STRM_PRINTF("#%d exit local_loop!\n", idx);

Err:
	if (local)
		IND_FREE(local);
	return;
}

static int local_argparse_play(int idx, PlayArg* arg, char *argbuf, const char* url, int shiftlen, int begin, int end)
{
	FlashArg *flasharg = (FlashArg *)argbuf;

	if (url == NULL)
		LOG_STRM_ERROUT("#%d url is NULL\n", idx);

	IND_STRCPY(flasharg->url, url);

	return 0;
Err:
	return -1;
}

int flash_create_stream(StreamCtrl *ctrl)
{
	ctrl->handle = ctrl;

	ctrl->loop_play = local_loop_play;
	ctrl->loop_record = NULL;

	ctrl->argsize = sizeof(FlashArg);
	ctrl->argparse_play = local_argparse_play;

	return 0;
}

#endif//INCLUDE_PVR
