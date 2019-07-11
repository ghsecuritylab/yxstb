
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <assert.h>
#include <setjmp.h>
#include <stddef.h>
#include <math.h>
#include <ctype.h>	/* for tolower */

#include "app/Assertions.h"

#include "ind_mem.h"
#include "img_bio.h"

#define nil	NULL

typedef struct Rawimage Rawimage;

struct Rawimage
{
	int		width;
	int		height;
	uint8_t	*cmap;
	int		cmaplen;
	int		nchans;
	uint8_t	*chans[4];
	int		chandesc;
	int		chanlen;

	int		gif_trindex;
	int		gif_x;
	int		gif_y;
	int		gif_width;
	int		gif_height;
};

enum
{
	/* Channel descriptors */
	CRGB_	= 0,	/* three channels, no map */
	CYCbCr	= 1,	/* three channels, no map, level-shifted 601 color space */
	CY_		= 2,	/* one channel, luminance */
	CRGB1	= 3,	/* one channel, map present */
	CRGBV	= 4,	/* one channel, map is RGBV, understood */
	CRGB24	= 5,	/* one channel in correct data order for loadimage(RGB24) */
	CRGBA32	= 6,	/* one channel in correct data order for loadimage(RGBA32) */
	CYA16	= 7,	/* one channel in correct data order for loadimage(Grey8+Alpha8) */
	CRGBVA16= 8,	/* one channel in correct data order for loadimage(CMAP8+Alpha8) */
};

#ifdef  __cplusplus
extern "C" {
#endif

int img_info_bmp(uint8_t *imgbuf, int imglen, int *pwidth, int *pheight);
int img_info_gif (uint8_t *imgbuf, int imglen, int *pwidth, int *pheight);
int img_info_jpg(uint8_t *imgbuf, int imglen, int *pwidth, int *pheight);
int img_info_png(uint8_t *imgbuf, int imglen, int *pwidth, int *pheight);

Rawimage*	img_decode_bmp(uint8_t *, int);
Rawimage*	img_decode_gif (uint8_t *, int);
Rawimage*	img_decode_jpg(uint8_t *, int);
Rawimage*	img_decode_png(uint8_t *, int);

int int_jpg_check(uint8_t *buf, int len);
int int_gif_check(uint8_t *buf, int len);

#ifdef DEBUG_BUILD
#define fprint(f, X, ...) PRINTF(X, ##__VA_ARGS__);
#else
#define fprint(f, X...)
#endif

#ifdef  __cplusplus
}
#endif

