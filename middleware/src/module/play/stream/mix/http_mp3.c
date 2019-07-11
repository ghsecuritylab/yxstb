/**********************************************************
	Copyright (c) 2008-2009, Yuxing Software Corporation
	All Rights Reserved
	Confidential Property of Yuxing Softwate Corporation

	Revision History:

	Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#ifdef INCLUDE_MP3
#include "../http/http.h"

#include "mad/mad.h"

#define MIX_SIZE_PCM		(1152 * 2 * 2)
#define MIX_SIZE_MP3		(1024 * 32)
#define MIX_SIZE_ELEM		(1024 * 4)
#define MIX_SIZE_FRAME		(1024 * 2)

typedef struct {
	char url[STREAM_URL_SIZE];
	int loop;
}  HttpMp3Arg;

struct LOCAL {
	int				index;

    HTTPLoop_t      loop;
	struct HTTP*	http;
	int				loop_flg;

	int				end_flg;
	uint32_t			end_clk;

	ts_audio_t		audio;

	int				codec;
	int				codec_open;

	char				mixbuf[MIX_SIZE_PCM];
	int					mixlen;
	int					mixoff;

	uint8_t				mp3buf[MIX_SIZE_MP3];
	int					mp3len0;
	int					mp3len1;
	int					mp3off;

	int					mp3_open;
	struct mad_stream	mp3_stream;
	struct mad_frame	mp3_frame;
	struct mad_synth	mp3_synth;
};

static void local_recv_sync(void *handle);

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

	if (local->codec_open == 0) {
		codec_pcm_open(local->codec, pcm->samplerate, 16, pcm->channels);
		local->codec_open = 1;
	}

	/* pcm->samplerate contains the sampling frequency */

	nchannels = pcm->channels;
	nsamples  = pcm->length;
	left_ch   = pcm->samples[0];
	right_ch  = pcm->samples[1];

	if (nsamples > 1152)
		LOG_STRM_ERROUT("#%d nsamples = %d\n", local->index, nsamples);

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

static int int_mp3_push(struct LOCAL *local)
{
	char *buf;
	int len, ret;

	len = 0;
	strm_http_buf_get(local->http, &buf, &len);
	if (len <= 0) {
		if (local->end_flg == 1) {
			LOG_STRM_PRINTF("#%d loop = %d\n", local->index, local->loop_flg);
			if (local->loop_flg) {
				strm_http_request(local->http, NULL, 0, 0);
				LOG_STRM_PRINTF("#%d CLR end_flag\n", local->index);
				local->end_flg = 0;
			} else {
				LOG_STRM_PRINTF("#%d SET end_flag 2\n", local->index);
				local->end_flg = 2;
			}
		}
		return 0;
	}

	ret = ts_audio_write(local->audio, buf, len);
	if (ret < 0) {
		LOG_STRM_ERROR("#%d ret = %d, len = %d\n", local->index, ret, len);
		strm_httploop_break(local->loop);
	} else {
		if (ret > 0)
			strm_http_buf_pop(local->http, ret);
	}

	return ret;
}

static int int_mp3_pop(struct LOCAL *local)
{
	int len;
	char *buf;

	if (local->mp3len1 > 0)
		return 0;

	if (local->mp3off + local->mp3len0 > MIX_SIZE_MP3 - MIX_SIZE_FRAME) {
		if (local->mp3len0 > 0)
			IND_MEMCPY(local->mp3buf, local->mp3buf + local->mp3off, local->mp3len0);
		local->mp3off = 0;
	}

	buf = (char*)(local->mp3buf + local->mp3off + local->mp3len0);
	len = ts_audio_frame(local->audio, buf, MIX_SIZE_FRAME);
	if (len <= 0) {
		if (local->end_flg == 2) {
			LOG_STRM_PRINTF("#%d SET end_flag 3\n", local->index);
			local->end_flg = 3;
			local->end_clk = strm_httploop_clk(local->loop) + 200;
		}
	} else {
	#ifdef ENABLE_SAVE_PLAY
		fwrite(buf, 1, len, local->save_fp);
		local->save_len += len;
	#endif
		if (local->mp3len0 == 0)
			local->mp3len0 = len;
		else
			local->mp3len1 = len;
	}

	return len;
}

static int int_mp3_mix(struct LOCAL *local)
{
	int len;
	struct mad_stream *stream;
	struct mad_frame *frame;
	struct mad_synth *synth;

	if (local->mixlen > 0) {
		len = codec_pcm_push(local->codec, local->mixbuf + local->mixoff, local->mixlen);
		if (len > 0) {
			local->mixoff += len;
			local->mixlen -= len;
		}
		return len;
	}

	if (local->mp3len1 <= 0)
		return 0;

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

	return 1;
}

static void int_sync(struct LOCAL *local)
{
	int pushlen, poplen, mixlen;

	for (;;) {
		pushlen = int_mp3_push(local);
		poplen = int_mp3_pop(local);
		mixlen = int_mp3_mix(local);

		if (pushlen <= 0 && poplen <= 0 && mixlen <= 0)
			break;
	}
}

static void local_100ms(void *handle)
{
	struct LOCAL *local = (struct LOCAL *)handle;

	if (local->end_flg >= 3) {
		uint32_t clk = strm_httploop_clk(local->loop);
		if (local->end_clk <= clk) {
			LOG_STRM_PRINTF("#%d STRM_MSG_STREAM_END\n", local->index);
			stream_post_msg(local->index, STRM_MSG_STREAM_END, 0);
			strm_httploop_break(local->loop);
		}
		return;
	}

	int_sync(local);
}

static void local_1000ms(void *handle)
{
#ifdef DEBUG_BUILD
	struct LOCAL *local = (struct LOCAL *)handle;

	if (local->end_flg >= 3) {
		uint32_t clk = strm_httploop_clk(local->loop);
		LOG_STRM_PRINTF("#%d timeout = %d\n", local->index, local->end_clk - clk);
	}
#endif
}

static int local_open(struct LOCAL *local, PlayArg *arg, HttpMp3Arg *mp3arg)
{
	LOG_STRM_PRINTF("#%d url = %s\n", local->index, mp3arg->url);

	local->codec_open = 0;
	local->end_flg = 0;

	ts_audio_reset(local->audio);

	local->loop_flg = mp3arg->loop;
	if (strm_http_request(local->http, mp3arg->url, 0, 0))
		LOG_STRM_ERROUT("#%d local_connect\n", local->index);

	local->mixlen = 0;
	local->mixoff = 0;

	local->mp3len0 = 0;
	local->mp3len1 = 0;
	local->mp3off = 0;

	mad_stream_init(&local->mp3_stream);
	mad_frame_init(&local->mp3_frame);
	mad_synth_init(&local->mp3_synth);

	mad_stream_options(&local->mp3_stream, 0);

	local->mp3_open = 1;

	return 0;
Err:
	return -1;
}

static int local_recv_begin(void *handle)
{
	return 0;
}

static void local_recv_sync(void *handle)
{
	struct LOCAL *local = (struct LOCAL *)handle;

	int_sync(local);
}

static void local_recv_end(void *handle)
{
	struct LOCAL *local = (struct LOCAL *)handle;
	LOG_STRM_PRINTF("#%d SET end_flag 1\n", local->index);
	local->end_flg = 1;
}

static int local_close(struct LOCAL *local)
{
	LOG_STRM_PRINTF("#%d\n", local->index);

	if (local->mp3_open) {
		mad_synth_finish(&local->mp3_synth);
		mad_frame_finish(&local->mp3_frame);
		mad_stream_finish(&local->mp3_stream);
		local->mp3_open = 0;
	}

	if (local->codec_open) {
		codec_pcm_close(local->codec);
		local->codec_open = 0;
	}

	strm_httploop_break(local->loop);

	return 0;
}

static void local_error(void *handle, HTTP_MSG msgno)
{
	struct LOCAL *local = (struct LOCAL *)handle;

	LOG_STRM_PRINTF("#%d STRM_MSG_OPEN_ERROR\n", local->index);
	stream_post_msg(local->index, STRM_MSG_OPEN_ERROR, 0);
	strm_httploop_break(local->loop);
}

static void local_cmd(void *handle, StreamCmd* strmCmd)
{
	struct LOCAL *local = (struct LOCAL *)handle;
	int cmd = strmCmd->cmd;

	LOG_STRM_PRINTF("#%d cmd = %d\n", local->index, cmd);

	if (stream_deal_cmd(local->index, strmCmd) == 1)
		return;

	switch(cmd) {
	case STREAM_CMD_CLOSE:
		LOG_STRM_ERROR("#%d STREAM_CMD_CLOSE\n", local->index);
		strm_httploop_break(local->loop);
		break;

	default:
		LOG_STRM_ERROUT("#%d Unkown CMD %d\n", local->index, cmd);
		break;
	}

Err:
	return;
}

static void local_msg(void *handle, STRM_MSG msgno, int arg)
{
}

static void local_loop_play(void* handle, int idx, mid_msgq_t msgq, PlayArg* arg, char* argbuf)
{
	struct LOCAL *local;

	local = (struct LOCAL *)IND_CALLOC(sizeof(struct LOCAL), 1);
	if (!local)
		LOG_STRM_ERROUT("#%d malloc failed!\n", idx);

	local->audio = ts_audio_create( );
	if (!local->audio)
		LOG_STRM_ERROUT("#%d ts_audio_create!\n", idx);

    {
        HttpLoopOp op;

        memset(&op, 0, sizeof(op));

        op.deal_cmd = local_cmd;
        op.deal_msg = local_msg;
        op.local_100ms = local_100ms;
        op.local_1000ms = local_1000ms;

        local->loop = strm_httploop_create(idx, &op, local, msgq);
        if (!local->loop)
            LOG_STRM_ERROUT("strm_httploop_create!\n");
    }
	local->http = strm_http_create(local->loop, HTTP_RECV_SIZE);
	if (!local->http)
		LOG_STRM_ERROUT("#%d strm_http_create!\n", idx);

	local->index = idx;
	{
		HttpOp op;

		memset(&op, 0, sizeof(op));
		op.recv_begin = local_recv_begin;
		op.recv_sync = local_recv_sync;
		op.recv_end = local_recv_end;
	
		op.deal_error = local_error;
	
        strm_http_set_opset(local->http, &op, local);
	}

	if (idx == STREAM_INDEX_PIP)
		local->codec = 1;
	else
		local->codec = 0;

	if (local_open(local, arg, ( HttpMp3Arg *)argbuf))
		LOG_STRM_ERROUT("#%d local_open\n", idx);

	strm_httploop_loop(local->loop);
	LOG_STRM_PRINTF("#%d exit local_loop!\n", idx);

	local_close(local);

Err:
	if (local) {
		if (local->loop)
			strm_httploop_delete(local->loop);
		if (local->audio)
			ts_audio_delete(local->audio);
		IND_FREE(local);
	}
	return;
}

static int local_argparse_play(int idx, PlayArg* arg, char *argbuf, const char* url, int shiftlen, int begin, int end)
{
	 HttpMp3Arg *mp3arg = ( HttpMp3Arg *)argbuf;

	if (url == NULL)
		LOG_STRM_ERROUT("#%d url is NULL\n", idx);
	LOG_STRM_PRINTF("#%d url = %s, loop = %d\n", idx, url, shiftlen);

	IND_STRCPY(mp3arg->url, url);
	if (shiftlen == 1)
		mp3arg->loop = 1;
	else
		mp3arg->loop = 0;

	return 0;
Err:
	return -1;
}

int http_mp3_create_stream(StreamCtrl *ctrl)
{
	ctrl->handle = ctrl;

	ctrl->loop_play = local_loop_play;

	ctrl->argsize = sizeof( HttpMp3Arg);
	ctrl->argparse_play = local_argparse_play;

	return 0;
}
#endif//INCLUDE_MP3
