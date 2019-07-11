
#ifndef __INF_INFLATE_H__
#define __INF_INFLATE_H__

enum
{
	FlateOk			= 0,
	FlateNoMem		= -1,
	FlateInputFail		= -2,
	FlateOutputFail		= -3,
	FlateCorrupted		= -4,
	FlateInternal		= -5
};

/*
 * zlib header fields
 */
enum
{
	ZlibMeth	= 0x0f,			/* mask of compression methods */
	ZlibDeflate	= 0x08,

	ZlibCInfo	= 0xf0,			/* mask of compression aux. info */
	ZlibWin32k	= 0x70			/* 32k history window */
};


void img_mkcrctab(uint32_t *crctab, uint32_t poly);
uint32_t img_blockcrc(uint32_t *crctab, uint32_t crc, void *vbuf, int n);
int img_inflateinit(void);
int img_inflatezlib(void *wr, int (*w)(void*, void*, int), void *getr, int (*get)(void*));
char *img_flateerr(int err);

#define	mkcrctab	img_mkcrctab
#define	inflateinit	img_inflateinit
#define	blockcrc	img_blockcrc
#define	inflatezlib	img_inflatezlib
#define	flateerr	img_flateerr

#endif//__INF_INFLATE_H__
