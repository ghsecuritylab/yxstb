
#include "ts_iframe.h"
#include "ts_size.h"
#include "ind_ts.h"

/*
	1 一个帧一个PES
	2 PES前含填充包
	3 一个帧多个PES 只有第一个PES含PTS
 */

static ifrm_write_f		g_ifrm_write	= NULL;
static ifrm_begin_f		g_ifrm_begin	= NULL;
static ifrm_end_f		g_ifrm_end		= NULL;

static int int_parse_h264(ts_icontent_t icntnt, uint8_t *buf, int len)
{
	char ch;
	int i, type, prefix, result;

	result = -1;
	prefix = icntnt->frm_prefix;

	for (i = 0; i < len; i ++) {
		ch = buf[i];
		if (prefix == 3) {
			type = ch & 0x1f;
			if (type <= 5) {
				if ((type == 1 || type == 5) && len - i - 1 >= 4) {
					struct u_bits bits;

					bits.buf = buf + i + 1;
					bits.len = len - i - 1;
					bits.idx = 0;
					bits.bit = 0;

					if (ts_ue_v(&bits)) {
						prefix = 0;
						END_OUT("first_mb_in_slice\n");
					}
					if (ts_ue_v(&bits)) {
						prefix = 0;
						END_OUT("slice_type\n");
					}

					type = bits.ue;
					if (type == 2 || type == 4 || type == 7 || type == 9) {
						//PRINTF("IFRAME: 0x%08x\n", iparse->pack_count * 188);
						result = 1;
						goto End;
					}
				}
				result = 2;
				goto End;
			}
			prefix = 0;
			continue;
		}
		switch(ch) {
		case 0:
			if (prefix < 2)
				prefix ++;
			break;
		case 1:
			if (prefix == 2)
				prefix = 3;
			else
				prefix = 0;
			break;
		default:
			prefix = 0;
			break;
		}
	}

	result = 0;
End:
	icntnt->frm_prefix = prefix;
	return result;
}

static int int_parse_mpg2(ts_icontent_t icntnt, uint8_t *buf, int len)
{
	char ch;
	int i, result, prefix;

	result = -1;
	prefix = icntnt->frm_prefix;
	for (i = 0; i < len; i ++) {
		ch = buf[i];
		if (prefix == 3) {
			if (ch == 0 && len - i - 1 >= 2) {
				/*
				switch ((buf[i + 2] >> 3) & 0x07) {
				case 1: PRINTF("I\n");	break;
				case 2: PRINTF("P\n");	break;
				case 3: PRINTF("B\n");	break;
				default: PRINTF("U\n");	break;
				}
				*/
				if ((buf[i + 2] & 0x38) == 0x08)
					result = 1;
				else
					result = 2;
				goto End;
			}
			prefix = 0;
			continue;
		}
		switch(ch) {
		case 0:
			if (prefix < 2)
				prefix ++;
			break;
		case 1:
			if (prefix == 2)
				prefix = 3;
			else
				prefix = 0;
			break;
		default:
			prefix = 0;
			break;
		}
	}

	result = 0;
End:
	icntnt->frm_prefix = prefix;
	return result;
}

static int int_parse_audio(ts_icontent_t icntnt, uint8_t *buf, int len)
{
	return 1;
}

#define CLK_DIFF_CONTINUE		400 //正常码流相邻PTS差的极限值为4s

static void int_clk_play(ts_icontent_t icntnt, uint32_t clk)
{
	//PRINTF("PTS: %d / %d\n", pts,  pts / 1000);
	if (icntnt->clk_diff == 0xffffffff) {
		PRINTF("base: %d\n", clk);
		icntnt->clk_base = clk;
		icntnt->clk_diff = 0;
	} else {
		if (clk == icntnt->clk_last)
			return;

		if (clk < icntnt->clk_last || clk > icntnt->clk_last + CLK_DIFF_CONTINUE) {
			if (clk < icntnt->clk_last && clk + CLK_DIFF_CONTINUE/2 >= icntnt->clk_last)
				return;
			PRINTF("WARN! %u > %u\n",  icntnt->clk_last, clk);

			icntnt->clk_time += icntnt->clk_last - icntnt->clk_base;
			icntnt->clk_time += icntnt->clk_diff;

			icntnt->clk_base = clk;
		} else {
			icntnt->clk_diff = clk - icntnt->clk_last;
		}
	}
	icntnt->clk_last = clk;
}

inline static uint32_t int_clk_time(ts_icontent_t icntnt)
{
	return (icntnt->clk_time + (icntnt->clk_last - icntnt->clk_base));
}

int ts_iparse_pcr(ts_iparse_t iparse, char *buf, int len)
{
	int off, wlen;
	uint8_t *p;
	uint32_t clk, pid;

	ts_ifrm_t ifrm = &iparse->ifrm;
	ts_icontent_t icntnt = &iparse->icntnt;

	if (iparse->strm_pid == 0)
		ERR_OUT("parse not init\n");

	if (len % 188)
		ERR_OUT("len = %d\n", len);

	wlen = 0;
	for (off = 0; off < len; off += 188, icntnt->pack_count ++) {
		if (ifrm->ifrm_size > 0)
			ifrm->ifrm_size ++;
		p = (uint8_t *)buf + off;
		if (p[0] != 0x47)
			ERR_OUT("ts sync byte!\n");

		pid =(((uint32_t)p[1] & 0x1f) << 8) + p[2];

		if (pid == iparse->pcr_pid) {
			uint32_t pcr = ts_pcr_parse188(p);
			if (pcr)
				int_clk_play(icntnt, pcr / 450);
		}

		if ((p[1] & 0x40) == 0) //start indicator
			continue;

		if (pid != iparse->strm_pid)
			continue;

		clk = int_clk_time(icntnt);

		if (ifrm->ifrm_size > 0 && clk >= icntnt->ifrm_clk + 40) {
			ifrm->ifrm_size --;
			//PRINTF("@@@@@@@@@@@@@@@@: ifrm_clk = %d\n", iparse->ifrm_clk / 100);
			if (g_ifrm_end(iparse->rec, ifrm))
				ERR_OUT("g_ifrm_end\n");
			ifrm->ifrm_size = 0;
		}
		if (ifrm->ifrm_size == 0) {
			if (g_ifrm_write(iparse->rec, buf + wlen, off - wlen))
				ERR_OUT("ifrm_write1\n");
			wlen = off;

			ifrm->ifrm_size = 1;
			ifrm->ifrm_off = icntnt->pack_count;
			ifrm->ifrm_clk = clk;
			if (g_ifrm_begin(iparse->rec, ifrm, clk))
				ERR_OUT("g_ifrm_begin\n");
			if (ifrm->ifrm_off == 0)
				icntnt->pack_count = 0;

			icntnt->ifrm_clk = clk;
		}
	}

	if (g_ifrm_write(iparse->rec, buf + wlen, len - wlen))
		ERR_OUT("ifrm_write2\n");

	return 0;
Err:
	return -1;
}

int ts_iparse_pts(uint8_t* buf, uint32_t* pts)
{
	int thl, phl;

	if (buf[0] != 0x47)
		ERR_OUT("ts sync byte!\n");

	if (buf[3] & 0x20) //adaptation field
		thl = 5 + (int)((uint32_t)buf[4]);
	else
		thl = 4;

	if ((buf[1] & 0x40) == 0) //start indicator
		return 0;

	buf += thl;

	if (thl > 188 - 14)//pes 负载过短
		ERR_OUT("thl = %d\n", thl);

	if (buf[0] != 0 || buf[1] != 0 || buf[2] != 1)
		ERR_OUT("prefix1 %02x%02x%02x\n", (uint32_t)buf[0], (uint32_t)buf[1], (uint32_t)buf[2]);

	if (buf[3] < 0xC0 || buf[3] > 0xEF)
		ERR_OUT("stream id\n");

	if ((buf[6] & 0xC0) != 0x80)
		ERR_OUT("PES header error\n");

	phl = (int)((uint32_t)buf[8]) + 9;
	if (thl + phl > 188)
		ERR_OUT("PES header too large? thl = %d, phl = %d\n", thl, phl);

	if ((buf[7] & 0x80) != 0x80 || (buf[9] & 0x20) != 0x20)
		return 0;

	if (pts) {
		*pts =	(uint32_t)(buf[9]  & 0x0E) 	<< (32 - 3 - 1) |
				(uint32_t)(buf[10])			<< (29 - 7 - 1) |
				(uint32_t)(buf[11] & 0xFE)	<< (21 - 7 - 1) |
				(uint32_t)(buf[12])			<< (14 - 7 - 1) |
				(uint32_t)(buf[13] & 0xFE)	>> 2;
	}

	return (thl + phl);
Err:
	return -1;
}

int ts_iparse_frame(ts_iparse_t iparse, char *buf, int len)
{
	int hl, ret, off, wlen;
	uint8_t *p;
	uint32_t clk, pid, pts;

	ts_ifrm_t ifrm = &iparse->ifrm;
	ts_icontent_t icntnt = &iparse->icntnt;

	if (iparse->strm_pid == 0)
		ERR_OUT("parse not init\n");

	if (len % 188)
		ERR_OUT("len = %d\n", len);

	wlen = 0;
	for (off = 0; off < len; off += 188, icntnt->pack_count ++) {

		if (icntnt->frm_state == PARSE_STATE_FRAME || icntnt->frm_state == PARSE_STATE_IFRAME)
			ifrm->ifrm_size ++;

		p = (uint8_t*)(buf + off);
		pid =(((uint32_t)p[1] & 0x1f) << 8) + p[2];
		if (pid != iparse->strm_pid)
			continue;

		if ((p[1] & 0x40) == 0) {//start indicator
			if (icntnt->frm_state == PARSE_STATE_FRAME) {
				if (p[3] & 0x20) //adaptation field
					hl = 5 + (int)((uint32_t)p[4]);
				else
					hl = 4;
				ret = iparse->ifrm_parse(icntnt, (uint8_t*)p + hl, 188 - hl);
				switch(ret) {
				case 0:
					break;
				case 1:
					int_clk_play(icntnt, icntnt->frm_pts / 450);
					clk = int_clk_time(icntnt);

					if (g_ifrm_begin(iparse->rec, ifrm, clk))
						ERR_OUT("g_ifrm_begin1\n");
					if (ifrm->ifrm_off == 0) {
						icntnt->pack_count = (off - wlen) / 188;
						//PRINTF("@@@@@@@@@@@@@@@@: wlen = %d, off = %d, ifrm_off = %d, ifrm_size = %d\n", wlen, off, ifrm->ifrm_off, ifrm->ifrm_size);
						if (ifrm->ifrm_size > icntnt->pack_count + 1)
							ifrm->ifrm_size = icntnt->pack_count + 1;
						else
							ifrm->ifrm_off = icntnt->pack_count + 1 - ifrm->ifrm_size;
					}

					icntnt->frm_state = PARSE_STATE_IFRAME;
					break;
				default:
					icntnt->frm_state = PARSE_STATE_NONE;
					break;
				}
			}
			continue;
		}

		hl = ts_iparse_pts(p, &pts);
		if (hl <= 0)
			continue;

		clk = int_clk_time(icntnt);
		if (icntnt->frm_state == PARSE_STATE_IFRAME) {
			ifrm->ifrm_size --;
			//PRINTF("@@@@@@@@@@@@@@@@: offset = %08x, size = %u, end = %08x, clk = %d\n", ifrm->ifrm_off * 188, (uint32_t)ifrm->ifrm_size, (ifrm->ifrm_off + (uint32_t)ifrm->ifrm_size) * 188, clk);
			if (g_ifrm_end(iparse->rec, ifrm))
				ERR_OUT("g_ifrm_end\n");
			ifrm->ifrm_size = 0;

			if (g_ifrm_write(iparse->rec, buf + wlen, off - wlen))
				ERR_OUT("ifrm_write1\n");
			wlen = off;
		}

		icntnt->frm_prefix = -1;
		ret = iparse->ifrm_parse(icntnt, (uint8_t*)p + hl, 188 - hl);
		switch(ret) {
		case 0:
			ifrm->ifrm_size = 1;
			ifrm->ifrm_off = icntnt->pack_count;
			ifrm->ifrm_clk = clk;

			icntnt->frm_pts = pts;

			icntnt->frm_state = PARSE_STATE_FRAME;
			break;
		case 1:
			int_clk_play(icntnt, pts / 450);

			if (g_ifrm_write(iparse->rec, buf + wlen, off - wlen))
				ERR_OUT("ifrm_write1\n");
			wlen = off;
	
			ifrm->ifrm_size = 1;
			ifrm->ifrm_off = icntnt->pack_count;
			ifrm->ifrm_clk = clk;
			if (g_ifrm_begin(iparse->rec, ifrm, clk))
				ERR_OUT("g_ifrm_begin2\n");
			if (ifrm->ifrm_off == 0)
				icntnt->pack_count = 0;

			icntnt->frm_state = PARSE_STATE_IFRAME;
			break;
		default:
			icntnt->frm_state = PARSE_STATE_NONE;
			break;
		}
	}

	if (g_ifrm_write(iparse->rec, buf + wlen, len - wlen))
		ERR_OUT("ifrm_write2\n");

	return 0;
Err:
	return -1;
}

void ts_iparse_regist(ifrm_write_f ifrm_write, ifrm_begin_f ifrm_begin, ifrm_end_f ifrm_end)
{
	g_ifrm_write	= ifrm_write;
	g_ifrm_begin	= ifrm_begin;
	g_ifrm_end		= ifrm_end;
}

int ts_iparse_reset(ts_iparse_t iparse, PvrRecord_t rec, ts_psi_t psi, int reset)
{
	uint32_t strm_type;

	if (reset) {
		IND_MEMSET(iparse, 0, sizeof(struct ts_iparse));
		iparse->icntnt.clk_diff = 0xffffffff;
		iparse->icntnt.clk_time = 1;
	}

	iparse->rec = rec;

	if (psi->video_pid) {
		iparse->strm_pid = psi->video_pid;
		strm_type = psi->video_type;
	} else {
		iparse->strm_pid = psi->audio_pid[0];
		strm_type = psi->audio_type[0];
	}
	iparse->pcr_pid = psi->pcr_pid;

	switch(strm_type) {
	case ISO_IEC_11172_AUDIO:
	case ISO_IEC_13818_3_AUDIO:
	case ISO_IEC_13818_7_AUDIO:
	case ISO_IEC_MPEG4_AUDIO:
	case ISO_IEC_AC3_AUDIO:
		PRINTF("AUDIO\n");
		iparse->ifrm_parse = int_parse_audio;
		break;
	case ISO_IEC_13818_2_VIDEO:
		PRINTF("MPEG2\n");
		iparse->ifrm_parse = int_parse_mpg2;
		break;
	case ISO_IEC_H264:
		PRINTF("H264\n");
		iparse->ifrm_parse = int_parse_h264;
		break;
	default:
		ERR_OUT("video_type = %d\n", strm_type);
	}

	return 0;
Err:
	return -1;
}
