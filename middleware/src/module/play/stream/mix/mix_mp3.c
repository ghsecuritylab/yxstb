/**********************************************************
	Copyright (c) 2008-2009, Yuxing Software Corporation
	All Rights Reserved
	Confidential Property of Yuxing Softwate Corporation

	Revision History:

	Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/
#ifdef INCLUDE_MP3
#include <sys/select.h>

#include "mix.h"
#include "mad/mad.h"
#include "config/pathConfig.h"

#define MIX_SIZE_PCM		(1152 * 2 * 2)
#define MIX_SIZE_MP3		(1024 * 32)
#define MIX_SIZE_ELEM		(1024 * 4)
#define MIX_SIZE_FRAME		(1024 * 2)

//#define ENABLE_SAVE_PLAY

struct LOCAL
{
	uint32_t				clk;
	uint32_t				warnclk;

	int					index;
	uint32_t				magic;

	int					codec;

	STRM_STATE			state;
	ind_tlink_t			tlink;

	ts_audio_t			audio;

	mid_msgq_t			msgq;
	int					msgfd;

	char				mixbuf[MIX_SIZE_PCM];
	int					mixlen;
	int					mixoff;

	uint8_t				mp3buf[MIX_SIZE_MP3];
	int					mp3len0;
	int					mp3len1;
	int					mp3off;

	struct mad_stream	mp3_stream;
	struct mad_frame	mp3_frame;
	struct mad_synth	mp3_synth;

#ifdef ENABLE_SAVE_PLAY
	FILE*				save_fp;
	int					save_len;
#endif
};

static mid_mutex_t g_mutex = NULL;

static struct LOCAL* g_array[2] = {NULL, NULL};

static void local_push(void *arg);

static int local_open(struct LOCAL *local, PlayArg *arg)
{
	ind_timer_create(local->tlink, mid_10ms( ) + INTERVAL_CLK_100MS, INTERVAL_CLK_100MS, local_push, local);

	mid_mutex_lock(g_mutex);

	ts_audio_reset(local->audio);
	local->magic = arg->magic;
	local->state = STRM_STATE_OPEN;

	mid_mutex_unlock(g_mutex);

#ifdef ENABLE_SAVE_PLAY
	local->save_fp = fopen(DEFAULT_DEBUG_DATAPATH"/mp3.frm", "wb");
	local->save_len = 0;
	LOG_STRM_PRINTF("@@@@@@@@: fp = %p\n", local->save_fp);
#endif

	local->mixlen = 0;
	local->mixoff = 0;

	local->mp3len0 = 0;
	local->mp3len1 = 0;
	local->mp3off = 0;

	mad_stream_init(&local->mp3_stream);
	mad_frame_init(&local->mp3_frame);
	mad_synth_init(&local->mp3_synth);

	mad_stream_options(&local->mp3_stream, 0);

	return 0;
}

static int local_close(struct LOCAL *local)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	if (local->state == STRM_STATE_CLOSE)
		LOG_STRM_ERROUT("STRM_STATE_CLOSE\n");

	#ifdef ENABLE_SAVE_PLAY
		LOG_STRM_PRINTF("@@@@@@@@: save_len = %d\n", local->save_len);
		if (local->save_fp) {
			fclose(local->save_fp);
			local->save_fp = NULL;
		}
	#endif

	if (local->state == STRM_STATE_PLAY)
		codec_pcm_close(local->codec);

	ind_timer_delete(local->tlink, local_push, local);

	mad_synth_finish(&local->mp3_synth);
	mad_frame_finish(&local->mp3_frame);
	mad_stream_finish(&local->mp3_stream);

	mid_mutex_lock(g_mutex);
	local->state = STRM_STATE_CLOSE;
	mid_mutex_unlock(g_mutex);

	return 0;
Err:
	return -1;
}


static void local_cmd(struct LOCAL* local, StreamCmd* strmCmd)
{
	int cmd = strmCmd->cmd;

	LOG_STRM_PRINTF("#%d %d %d\n", local->index, local->state, cmd);

	if (stream_deal_cmd(local->index, strmCmd) == 1)
		return;

	switch(cmd) {
	case STREAM_CMD_CLOSE:
		LOG_STRM_PRINTF("#%d STREAM_CMD_CLOSE\n", local->index);
		local_close(local);
		break;
	default:
		LOG_STRM_PRINTF("#%d cmd = %d\n", local->index, cmd);
		break;
	}
}

static inline signed int scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static int local_output(struct LOCAL* local, struct mad_header const *header, struct mad_pcm *pcm)
{
	int len;
	char *buf;
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;

	if (local->state == STRM_STATE_OPEN) {
		codec_pcm_open(local->codec, pcm->samplerate, 16, pcm->channels);
		local->state = STRM_STATE_PLAY;
	}

	/* pcm->samplerate contains the sampling frequency */

	nchannels = pcm->channels;
	nsamples  = pcm->length;
	left_ch   = pcm->samples[0];
	right_ch  = pcm->samples[1];

	if (nsamples > 1152)
		LOG_STRM_ERROUT("nsamples = %d\n", nsamples);

	len = 0;
	buf = local->mixbuf;

	while (nsamples--) {
		signed int sample;

		/* output sample(s) in 16-bit signed little-endian PCM */

		sample = scale(*left_ch++);
		buf[0] = (sample >> 0) & 0xff;
		buf[1] = (sample >> 8) & 0xff;
		buf += 2;
		len += 2;

		if (nchannels == 2) {
			sample = scale(*right_ch++);
			buf[0] = (sample >> 0) & 0xff;
			buf[1] = (sample >> 8) & 0xff;
			buf += 2;
			len += 2;
		}
	}
	local->mixlen = len;
	local->mixoff = 0;

	return 0;
Err:
	return -1;
}

static void local_push(void *arg)
{
	int len;
	struct mad_stream *stream;
	struct mad_frame *frame;
	struct mad_synth *synth;
	struct LOCAL *local = (struct LOCAL *)arg;

	for (;;) {
		if (local->mixlen > 0) {
			len = codec_pcm_push(local->codec, local->mixbuf + local->mixoff, local->mixlen);
			if (len <= 0)
				return;
			local->mixoff += len;
			local->mixlen -= len;
			continue;
		}

		while (local->mp3len1 <= 0) {
			char *buf;
			int space = MIX_SIZE_MP3 - local->mp3off - local->mp3len0;
			if (space < MIX_SIZE_FRAME) {
				if (local->mp3len0 > 0)
					IND_MEMCPY(local->mp3buf, local->mp3buf + local->mp3off, local->mp3len0);
				local->mp3off = 0;
				continue;
			}
			buf = (char*)(local->mp3buf + local->mp3off + local->mp3len0);
			mid_mutex_lock(g_mutex);
			len = ts_audio_frame(local->audio, buf, MIX_SIZE_FRAME);
			mid_mutex_unlock(g_mutex);
			if (len <= 0)
				break;
#ifdef ENABLE_SAVE_PLAY
			fwrite(buf, 1, len, local->save_fp);
			local->save_len += len;
#endif
			if (local->mp3len0 == 0)
				local->mp3len0 = len;
			else
				local->mp3len1 = len;
		}
		if (local->mp3len1 <= 0)
			break;

		stream = &local->mp3_stream;
		frame = &local->mp3_frame;
		synth = &local->mp3_synth;
		mad_stream_buffer(stream, local->mp3buf + local->mp3off, local->mp3len0 + local->mp3len1);

		if (mad_frame_decode(frame, stream) == -1) {
			LOG_STRM_WARN("#%d error = 0x%04x\n", local->index, stream->error);
		} else {
			mad_synth_frame(synth, frame);
			local_output(local, &frame->header, &synth->pcm);
		}

		local->mp3off += local->mp3len0;
		local->mp3len0 = local->mp3len1;
		local->mp3len1 = 0;
	}
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

static void local_loop_play(void *handle, int idx, mid_msgq_t msgq, PlayArg* arg, char* argbuf)
{
	int codec = -1;
	struct LOCAL *local;

	local = (struct LOCAL *)IND_CALLOC(sizeof(struct LOCAL), 1);
	if (!local)
		LOG_STRM_ERROUT("#%d malloc failed!\n", idx);
	local->audio = ts_audio_create( );
	if (!local->audio)
		LOG_STRM_ERROUT("#%d ts_audio_create!\n", idx);

	local->index = idx;
	local->msgq = msgq;
	local->msgfd = mid_msgq_fd(local->msgq);
	local->tlink = int_stream_tlink(idx);

	if (idx == STREAM_INDEX_PIP)
		codec = 1;
	else
		codec = 0;
	local->codec = codec;

	mid_mutex_lock(g_mutex);
	g_array[codec] = local;
	mid_mutex_unlock(g_mutex);

	if (local_open(local, arg))
		LOG_STRM_ERROUT("#%d local_open\n", idx);

	local_loop(local);
	LOG_STRM_PRINTF("#%d exit local_loop!\n", idx);

Err:
    if (codec >= 0) {
        mid_mutex_lock(g_mutex);
        g_array[codec] = NULL;
        mid_mutex_unlock(g_mutex);
    }
	if (local) {
		if (local->audio)
			ts_audio_delete(local->audio);
		IND_FREE(local);
	}
	return;
}

static int local_argparse_play(int idx, PlayArg* arg, char *argbuf, const char* url, int shiftlen, int begin, int end)
{
	return 0;
}

int mix_mp3_create_stream(StreamCtrl *ctrl)
{
	if (!g_mutex)
		g_mutex = mid_mutex_create( );

	ctrl->handle = ctrl;

	ctrl->loop_play = local_loop_play;

	ctrl->argsize = 0;
	ctrl->argparse_play = local_argparse_play;

	return 0;
}

int stream_mix_mp3_space(int idx, uint32_t magic)
{
	int space = -1;
	struct LOCAL *local;

	mid_mutex_lock(g_mutex);

	if (idx != 0 && idx != 1)
		LOG_STRM_ERROUT("index = %d\n", idx);

	local = g_array[idx];
	if (!local) {
		space = 0;
	} else if (magic == local->magic && local->state != STRM_STATE_CLOSE) {
		space = ts_audio_space(local->audio);
		//LOG_STRM_PRINTF("index = %d, space = %d\n", index, space);
	} else {
		if (local->warnclk < local->clk) {
			LOG_STRM_WARN("index = %d, magic = %u/%u, state = %d\n", idx, magic, local->magic, local->state);
			local->warnclk = local->clk + 500;
		}
		space = -2;
	}

Err:
	mid_mutex_unlock(g_mutex);
	return space;
}

int stream_mix_mp3_push(int idx, uint32_t magic, char* buf, int len)
{
	int bytes = -1;
	char *buffer;
	struct LOCAL *local;

	mid_mutex_lock(g_mutex);

	if (idx != 0 && idx != 1)
		LOG_STRM_ERROUT("index = %d\n", index);
	if (buf == NULL || len <= 0)
		LOG_STRM_ERROUT("buf = %p, len = %d\n", buf, len);

	local = g_array[idx];
	if (!local)
		LOG_STRM_ERROUT("#%d local is NULL\n", idx);

	bytes = 0;
	if (magic == local->magic && local->state != STRM_STATE_CLOSE && ts_audio_buf_get(local->audio, &buffer, &bytes) == 0) {
		if (bytes > len)
			bytes = len;
		IND_MEMCPY(buffer, buf, bytes);
		ts_audio_buf_put(local->audio, bytes);
		//LOG_STRM_PRINTF("index = %d, bytes = %d\n", index, bytes);
	}

Err:
	mid_mutex_unlock(g_mutex);
	return bytes;
}
#endif//INCLUDE_MP3
