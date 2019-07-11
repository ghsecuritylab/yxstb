
#ifndef __TS_IFRM_H__
#define __TS_IFRM_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "app/Assertions.h"
#include "ind_mem.h"
#include "ind_ts.h"
#include "ind_pvr.h"

enum {
	PARSE_STATE_NONE = 0,
	PARSE_STATE_FRAME,
	PARSE_STATE_IFRAME
};

/*
	PVR version 小于8的旧I帧结构
 */
struct ts_iframe {
	uint32_t	ifrm_off;
	uint16_t	ifrm_size;
	uint16_t	reserve1;
	uint32_t	ifrm_clk;
};

typedef struct ts_iframe* ts_ifrm_t;
typedef struct ts_iparse* ts_iparse_t;
typedef struct ts_icontent* ts_icontent_t;

typedef int (*ifrm_write_f)(PvrRecord_t rec, char* buf, int len);
typedef int (*ifrm_begin_f)(PvrRecord_t rec, ts_ifrm_t ifrm, uint32_t clk);
typedef int (*ifrm_end_f)(PvrRecord_t rec, ts_ifrm_t ifrm);

typedef int (*ifrm_parse_f)(ts_icontent_t icntnt, uint8_t *buf, int len);

struct ts_icontent {
	uint32_t		pack_count;

	uint32_t		ifrm_clk;

	uint32_t		clk_diff;
	uint32_t		clk_last;
	uint32_t		clk_base;
	uint32_t		clk_time;

	int			frm_prefix;
	int			frm_state;
	int			frm_pts;
};

struct ts_iparse {
	uint32_t		pcr_pid;
	uint32_t		strm_pid;
	PvrRecord_t	rec;

	struct ts_iframe	ifrm;
	struct ts_icontent	icntnt;

	ifrm_parse_f		ifrm_parse;
};

void ts_iparse_regist(ifrm_write_f ifrm_write, ifrm_begin_f ifrm_begin, ifrm_end_f ifrm_end);
int ts_iparse_reset(ts_iparse_t iparse, PvrRecord_t rec, ts_psi_t psi, int reset);
int ts_iparse_frame(ts_iparse_t iparse, char *buf, int len);
int ts_iparse_pcr(ts_iparse_t iparse, char *buf, int len);

int ts_iparse_pts(uint8_t* buf, uint32_t* pts);

int ts_index_2off(char *buf, int len);

#endif//__TS_IFRM_H__
