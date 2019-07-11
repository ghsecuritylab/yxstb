
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/Assertions.h"
#include "ind_mem.h"
#include "ind_ts.h"

#define PTS_DIFF_DECODE			(1000 * 15)//解码器最多缓冲15s的数据
#define PTS_DIFF_CONTINUE		(1000 * 4)//正常码流相邻PTS差的极限值为4s

/*******************************************************************************

	快进快退基于以下假设来计算时间
		快进时PTS不停增加，循环处突变
		快退时PTS不停减小，循环处突变

 *******************************************************************************/

struct ts_stamp {
	unsigned int	base0;
	unsigned int	last0;
	unsigned int	base;
	unsigned int	last;
	unsigned int	except;

	int				times;

	int 			time;
};
typedef struct ts_stamp*	ts_stamp_t;

struct ts_pts {
	struct ts_stamp	pts;
	struct ts_stamp	pcr;

	int scale;
};

ts_pts_t ts_pts_create(void)
{
	ts_pts_t tspts = (ts_pts_t)IND_MALLOC(sizeof(struct ts_pts));
	if (tspts == NULL)
		ERR_OUT("malloc %d failed\n", sizeof(struct ts_pts));

	return tspts;
Err:
	return NULL;
}

void ts_pts_delete(ts_pts_t tspts)
{
    if (tspts)
        IND_FREE(tspts);
}

void ts_pts_reset(ts_pts_t tspts, int scale)
{
	IND_MEMSET(tspts, 0, sizeof(struct ts_pts));
	tspts->scale = scale;
}

static void int_pts_play(ts_stamp_t stamp, unsigned int pts)
{
	//PRINTF("PTS: %d / %d\n", pts,  pts / 1000);
	if (stamp->base == 0) {
		stamp->base = pts;
		stamp->last = pts;
		DBG_PRN("base: %d / %d\n", pts, pts / 1000);
		return;
	}

	if (pts + PTS_DIFF_CONTINUE/2 < stamp->last || pts > stamp->last + PTS_DIFF_CONTINUE) {
		if (stamp->except && (pts + PTS_DIFF_CONTINUE/2 > stamp->except && pts < stamp->except + PTS_DIFF_CONTINUE)) {
			WARN_PRN("pts = (%d, %d), last = %d, skip %d\n", stamp->except, pts, stamp->last, (int)(pts  - stamp->last));
			if (stamp->base0)
				stamp->time += ((int)(stamp->last0 - stamp->base0)) / 10;
			stamp->base0 = stamp->base;
			stamp->last0 = stamp->last;

			stamp->base = stamp->except;
			stamp->except = 0;
			stamp->last = pts;
			return;
		}
		stamp->except = pts;
		return;
	}

	if (stamp->except)
		stamp->except = 0;

	stamp->last = pts;
}

static void int_pts_fast(ts_stamp_t stamp, const int scale, unsigned int pts)
{
	int diff = abs(scale) * PTS_DIFF_CONTINUE;

	if (stamp->base == 0) {
		stamp->base = pts;
		stamp->times = 0;
		PRINTF("base: %d / %d\n", pts, pts / 1000);
	} else if (	(scale > 0 && (pts + PTS_DIFF_CONTINUE/2 < stamp->last || pts > stamp->last + diff)) || 
				(scale < 0 && (pts > stamp->last + PTS_DIFF_CONTINUE || pts + diff < stamp->last))	) {
		PRINTF("pts: %d / %d\n", pts, pts / 1000);
		PRINTF("last: %d / %d\n", stamp->last, stamp->last / 1000);

		diff = (int)(stamp->last - stamp->base) / 10;
		stamp->time += diff;
		if (stamp->times > 0)
			stamp->time += diff / stamp->times;

		stamp->times = 0;
		stamp->base = pts;
	} else {
		stamp->times ++;
	}
	stamp->last = pts;
}

/*
	
 */
static void int_pts_input(ts_pts_t tspts, ts_stamp_t stamp, unsigned int pts)
{
	pts = pts / 45;

	if (tspts->scale == 0)
		ERR_OUT("scale is zero\n");

	if (tspts->scale == 1)
		int_pts_play(stamp, pts);
	else
		int_pts_fast(stamp, tspts->scale, pts);
Err:
	return;
}

void ts_pts_input(struct ts_pts* tsp, uint32_t pts)
{
	int_pts_input(tsp, &tsp->pts, pts);
}

uint32_t ts_pcr_parse188(uint8_t *buf)
{
	uint32_t pcr;

	if (buf[0] != 0x47)
		return 0;

	if ((buf[3] & 0x20) == 0)//adaptation_field_control
		return 0;

	if ((buf[5] & 0x10) == 0)//PCR_flag
		return 0;

	pcr =	(uint32_t)(buf[6] << 24) |
			(uint32_t)(buf[7] << 16) |
			(uint32_t)(buf[8] << 8 ) |
			(uint32_t)(buf[9] << 0 );

	return pcr;
}

void ts_pts_input_pcr(struct ts_pts* tsp, uint8_t *buf)
{
	uint32_t pcr = ts_pcr_parse188(buf);

	if (pcr)
		int_pts_input(tsp, &tsp->pcr, pcr);
}

int ts_pts_time_play(ts_pts_t tspts, int pcrmode, uint32_t clk, uint32_t pts)
{
	int clks;
	ts_stamp_t stamp;

	if (pcrmode)
		stamp = &tspts->pcr;
	else
		stamp = &tspts->pts;

	if (tspts->scale == 1) {
		pts = pts / 45;
		clks = ((int)(stamp->last - stamp->base)) / 10;

		if (pts + PTS_DIFF_CONTINUE / 2 > stamp->base && pts < stamp->last + PTS_DIFF_CONTINUE / 2) {
			//PRINTF("#1\n");
			//PRINTF("#1 pts_base = %d, pts = %d, pts_last = %d\n", ts->base, pts, ts->last);
			if (pts <= stamp->base)
				clks = 0;
			else
				clks = ((int)(pts - stamp->base)) / 10;
			if (stamp->base0) {
				clks += stamp->time;
				clks += ((int)(stamp->last0 - stamp->base0)) / 10;
			}
		} else if (clks < PTS_DIFF_DECODE / 10 && pts < stamp->last0 + PTS_DIFF_CONTINUE && pts + PTS_DIFF_CONTINUE > stamp->base0) {
			//PRINTF("#2\n");
			clks = ((int)(pts - stamp->base0)) / 10;
			clks += stamp->time;
		} else {
			//PRINTF("#3\n");
			{
				static uint32_t print_clk = 0;
				if (print_clk < clk) {
					PRINTF("base = %d / last = %d / pts = %d\n", stamp->base, stamp->last, pts);
					print_clk = clk + 200;
				}
			}

			if (pts <= stamp->base)
				clks = 0;
			else
				clks = ((int)(stamp->last - stamp->base)) / 10;
			if (stamp->base0) {
				clks += stamp->time;
				clks += ((int)(stamp->last0 - stamp->base0)) / 10;
			}
		}
	} else {
		clks = stamp->time + (int)(stamp->last - stamp->base) / 10;
	}

	return clks;
}

int ts_pts_time_last(ts_pts_t tspts, int pcrmode)
{
	int clks;
	ts_stamp_t stamp;

	if (pcrmode)
		stamp = &tspts->pcr;
	else
		stamp = &tspts->pts;

	if (stamp->last <= stamp->base)
		clks = 0;
	else
		clks = ((int)(stamp->last - stamp->base)) / 10;
	if (stamp->base0) {
		clks += stamp->time;
		clks += ((int)(stamp->last0 - stamp->base0)) / 10;
	}

	return clks;
}


#define CLK_DIFF_CONTINUE		400 //正常码流相邻PCR差的极限值为4s

struct ts_pcr {
	uint32_t		clk_except;
	uint32_t		clk_diff;
	uint32_t		clk_last;
	uint32_t		clk_base;
	uint32_t		clk_time;
};

ts_pcr_t ts_pcr_create(void)
{
	ts_pcr_t tspcr = NULL;

	tspcr = (ts_pcr_t)IND_CALLOC(sizeof(struct ts_pcr), 1);
	if (tspcr == NULL)
		ERR_OUT("malloc ts_pcr\n");

	return tspcr;
Err:
	return NULL;
}

void ts_pcr_reset(ts_pcr_t tspcr, int half)
{
	uint32_t clk = tspcr->clk_last;

	IND_MEMSET(tspcr, 0, sizeof(struct ts_pcr));
	if (half) {
		tspcr->clk_last = clk;
		tspcr->clk_base = clk;
	}
}

void ts_pcr_delete(ts_pcr_t tspcr)
{
	if (tspcr)
		IND_FREE(tspcr);
}

static void int_pcr_fill(ts_pcr_t tspcr, uint32_t clk)
{
	//PRINTF("clk = %u / (%u, %u)\n", clk, tspcr->clk_base, tspcr->clk_last);
	if (0 == tspcr->clk_base) {
		tspcr->clk_base = clk;
		tspcr->clk_diff = 0;
	} else {
		if (clk == tspcr->clk_last)
			return;

		if (clk < tspcr->clk_last || clk > tspcr->clk_last + CLK_DIFF_CONTINUE) {
			if (tspcr->clk_except && clk >= tspcr->clk_except && clk < tspcr->clk_except + CLK_DIFF_CONTINUE) {
				PRINTF("WARN! %u > %u\n", tspcr->clk_last, clk);
	
				tspcr->clk_time += tspcr->clk_last - tspcr->clk_base;
				tspcr->clk_time += tspcr->clk_diff * 2;
	
				tspcr->clk_base = clk;
				tspcr->clk_except = 0;
			} else {
				tspcr->clk_except = clk;
				return;
			}
		} else {
			tspcr->clk_except = 0;
			tspcr->clk_diff = clk - tspcr->clk_last;
		}
	}
	tspcr->clk_last = clk;
}

int ts_pcr_fill(ts_pcr_t tspcr, char *buf, int len)
{
	int off;
	uint32_t pcr;

	if (tspcr == NULL)
		return -1;

	if (len % 188)
		ERR_OUT("len = %d\n", len);

	for (off = 0; off < len; off += 188) {
		pcr = ts_pcr_parse188((uint8_t *)buf + off);
		if (pcr)
			int_pcr_fill(tspcr, pcr / 450);
	}

	return 0;
Err:
	return -1;
}

uint32_t ts_pcr_time(ts_pcr_t tspcr)
{
	if (tspcr == NULL)
		return 0;
	return (tspcr->clk_time + (tspcr->clk_last - tspcr->clk_base));
}

