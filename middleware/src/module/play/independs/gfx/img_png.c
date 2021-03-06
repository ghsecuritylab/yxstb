
#include "img_decode.h"
#include "img_inflate.h"


enum
{
	IDATSIZE = 1000000,

	/* filtering algorithms */
	FilterNone =	0,	/* new[x][y] = buf[x][y] */
	FilterSub =	1,	/* new[x][y] = buf[x][y] + new[x-1][y] */ 
	FilterUp =		2,	/* new[x][y] = buf[x][y] + new[x][y-1] */ 
	FilterAvg =	3,	/* new[x][y] = buf[x][y] + (new[x-1][y]+new[x][y-1])/2 */ 
	FilterPaeth =	4,	/* new[x][y] = buf[x][y] + paeth(new[x-1][y],new[x][y-1],new[x-1][y-1]) */
	FilterLast =	5,

	PropertyBit = 1<<5,
};

typedef struct ZlibR ZlibR;
typedef struct ZlibW ZlibW;

struct ZlibW
{
	uint8_t *data;		/* Rawimage data */
	int ndata;
	int noutchan;
	int chandesc;
	int nchan;

	uint8_t *scan;		/* new scanline */
	uint8_t *lastscan;	/* previous scan line */
	int scanlen;		/* scan line length */
	int scanpos;		/* scan position */

	int dx;			/* width of image */
	int dy;			/* height of image */
	int bpc;			/* bits per channel (per pixel) */
	int y;				/* current scan line */
	int pass;			/* adam7 pass#; 0 means no adam7 */
	uint8_t palette[3*256];	/* color palette */
	int palsize;		/* number of palette entries */
};

struct ZlibR
{
	Biobuf *io;		/* input buffer */
	uint8_t *buf;		/* malloc'ed staging buffer */
	uint8_t *p;			/* next byte to decompress */
	uint8_t *e;			/* end of buffer */
	ZlibW *w;
};

static uint32_t crctab[256];
static uint8_t PNGmagic[] = { 137, 'P', 'N', 'G', '\r', '\n', 26, '\n'};

static jmp_buf	errlab;

#ifdef DEBUG_BUILD
#define sysfatal(X,...)								\
do{													\
    ERR_PRN(X, ## __VA_ARGS__);						\
	longjmp(errlab, 1);								\
}while(0)
#else
#define sysfatal(X...)		longjmp(errlab, 1)
#endif

static unsigned long int
get4(uint8_t *a)
{
	return (a[0]<<24) | (a[1]<<16) | (a[2]<<8) | a[3];
}

static
void
pnginit(void)
{
	static int inited = 0;

	if (inited)
		return;
	inited = 1;
	mkcrctab(crctab, 0xedb88320);
	inflateinit();
}

static
void*
pngmalloc(uint32_t n, int clear)
{
	void *p;

	p = IND_MALLOC(n);
	if (p == nil)
		sysfatal("ReadPNG: malloc failed");
	if (clear)
		IND_MEMSET(p, 0, n);
	return p;
}

static int
getchunk(Biobuf *b, char *type, uint8_t *d, int m)
{
	uint8_t buf[8];
	unsigned long int crc = 0, crc2;
	int n, nr;

	if (Bread(b, buf, 8) != 8)
		return -1;
	n = get4(buf);
	memmove(type, buf+4, 4);
	type[4] = 0;
	if (n > m)
		sysfatal("getchunk needed %d, had %d\n", n, m);
	nr = Bread(b, d, n);
	if (nr != n)
		sysfatal("getchunk read %d, expected %d\n", nr, n);
	crc = blockcrc(crctab, crc, type, 4);
	crc = blockcrc(crctab, crc, d, n);
	if (Bread(b, buf, 4) != 4)
		sysfatal("getchunk tlr failed\n");
	crc2 = get4(buf);
	if (crc != crc2)
		sysfatal("getchunk crc failed\n");
	return n;
}

static int
zread(void *va)
{
	ZlibR *z = va;
	char type[5];
	int n;

	if (z->p >= z->e){
	Again:
		z->p = z->buf;
		z->e = z->p;
		n = getchunk(z->io, type, z->p, IDATSIZE);
		if (n < 0 || strncmp(type, "IEND", 4) == 0)
			return -1;
		z->e = z->p + n;
		if (!strncmp(type,"PLTE", 4)){
			if (n < 3 || n > 3*256 || n%3)
				sysfatal("invalid PLTE chunk len %d\n", n);
			IND_MEMCPY(z->w->palette, z->p, n);
			z->w->palsize = 256;
			goto Again;
		}
		if (type[0] & PropertyBit)
			goto Again;  /* skip auxiliary chunks fornow */
		if (strncmp(type,"IDAT", 4)){
			sysfatal("unrecognized mandatory chunk %s\n", type);
			goto Again;
		}
	}
	return *z->p++;
}

static uint8_t 
paeth(uint8_t a, uint8_t b, uint8_t c)
{
	int p, pa, pb, pc;

	p = a + b - c;
	pa = abs(p - a);
	pb = abs(p - b);
	pc = abs(p - c);

	if (pa <= pb && pa <= pc)
		return a;
	else if (pb <= pc)
		return b;
	return c;
}

static void
unfilter(int alg, uint8_t *buf, uint8_t *up, int len, int bypp)
{
	int i;

	switch(alg){
	case FilterNone:
		break;

	case FilterSub:
		for (i = bypp; i < len; ++i)
			buf[i] += buf[i-bypp];
		break;

	case FilterUp:
		for (i = 0; i < len; ++i)
			buf[i] += up[i];
		break;

	case FilterAvg:
		for (i = 0; i < bypp; ++i)
			buf[i] += (0+up[i])/2;
		for (; i < len; ++i)
			buf[i] += (buf[i-bypp]+up[i])/2;
		break;

	case FilterPaeth:
		for (i = 0; i < bypp; ++i)
			buf[i] += paeth(0, up[i], 0);
		for (; i < len; ++i)
			buf[i] += paeth(buf[i-bypp], up[i], up[i-bypp]);
		break;

	default:
		sysfatal("unknown filtering scheme %d\n", alg);
	}
}

struct {
	int x;
	int y;
	int dx;
	int dy;
} adam7[] = {
	{0,0,1,1},	/* eve alone */
	{0,0,8,8},	/* pass 1 */
	{4,0,8,8},	/* pass 2 */
	{0,4,4,8},	/* pass 3 */
	{2,0,4,4},	/* pass 4 */
	{0,2,2,4},	/* pass 5 */
	{1,0,2,2},	/* pass 6 */
	{0,1,1,2},	/* pass 7 */
};

static void
scan(int len, ZlibW *z)
{
	int chan, i, j, nbit, off, val;
	uint8_t pixel[4], *p, *w;

	unfilter(z->scan[0], z->scan+1, z->lastscan+1, len-1, (z->nchan*z->bpc+7)/8);

	/*
	 * loop over raw bits extracting pixel values and converting to 8-bit
	 */
	nbit = 0;
	chan = 0;
	val = 0;
	off = z->y*z->dx + adam7[z->pass].x;
	w = z->data + z->noutchan*off;
	p = z->scan+1;	/* skip alg byte */
	len--;
	for (i=0; i<len*8; i++){
		val <<= 1;
		if (p[i>>3] & (1<<(7-(i&7))))
			val++;
		if (++nbit == z->bpc){
			/* finished the value */
			pixel[chan++] = (val*255)/((1<<z->bpc)-1);
			val = 0;
			nbit = 0;
			if (chan == z->nchan){
				/* finished the pixel */
				if (off < z->dx*z->dy){
					if (z->nchan < 3 && z->palsize){
						j = pixel[0];
						if (z->bpc < 8)
							j >>= 8-z->bpc;
						if (j >= z->palsize)
							sysfatal("index %d >= palette size %d\n", j, z->palsize);
						pixel[3] = pixel[1];	/* alpha */
						pixel[0] = z->palette[3*j];
						pixel[1] = z->palette[3*j+1];
						pixel[2] = z->palette[3*j+2];
					}
					switch(z->chandesc){
					case CYA16:
					//	print("%.2x%.2x ", pixel[0], pixel[1]);
						*w++ = pixel[1];
						*w++ += (pixel[0]*pixel[1])/255;
						break;
					case CRGBA32:
					//	print("%.2x%.2x%.2x%.2x ", pixel[0], pixel[1], pixel[2], pixel[3]);
						*w++ += pixel[3];
						*w++ += (pixel[2]*pixel[3])/255;
						*w++ += (pixel[1]*pixel[3])/255;
						*w++ += (pixel[0]*pixel[3])/255;
						break;
					case CRGB24:
						*w++ = pixel[2];
						*w++ = pixel[1];
					case CY_:
						*w++ = pixel[0];
						break;
					}
					w += (adam7[z->pass].dx-1)*z->noutchan;
				}
				off += adam7[z->pass].dx;
				if (off >= (z->y+1)*z->dx){
					/* finished the line */
					return;
				}
				chan = 0;
			}
		}
	}
	sysfatal("scan line too short\n");
}

static int
scanbytes(ZlibW *z)
{
	int bits, n, adx, dx;

	if (adam7[z->pass].y >= z->dy || adam7[z->pass].x >= z->dx)
		return 0;
	adx = adam7[z->pass].dx;
	dx = z->dx - adam7[z->pass].x;
	if (dx <= 0)
		n = 1;
	else
		n = (dx+adx-1)/adx;
	if (n != 1 + (z->dx - (adam7[z->pass].x+1)) / adam7[z->pass].dx){
		PRINTF("%d/%d != 1+(%d-1)/%d = %d\n",
			z->dx - adam7[z->pass].x - 1 + adx, adx,
			z->dx - (adam7[z->pass].x+1), adam7[z->pass].dx,
			1 + (z->dx - (adam7[z->pass].x+1)) / adam7[z->pass].dx);
	}
	bits = n*z->bpc*z->nchan;
	return 1 + (bits+7)/8;
}

static int
nextpass(ZlibW *z)
{
	int len;

	IND_MEMSET(z->lastscan, 0, z->scanlen);
	do{
		z->pass = (z->pass+1)%8;
		z->y = adam7[z->pass].y;
		len = scanbytes(z);
	}while(len < 2);
	return len;
}

static int
zwrite(void *vz, void *vbuf, int n)
{
	int oldn, m, len;
	uint8_t *buf, *t;
	ZlibW *z;

	z = vz;
	buf = vbuf;
	oldn = n;

	len = scanbytes(z);
	if (len < 2)
		len = nextpass(z);

	while(n > 0){
		m = len - z->scanpos;
		if (m > n){
			/* save final partial line */
			memmove(z->scan+z->scanpos, buf, n);
			z->scanpos += n;
			break;
		}

		/* fill line */
		memmove(z->scan+z->scanpos, buf, m);
		buf += m;
		n -= m;

		/* process line */
		scan(len, z);
		t = z->scan;
		z->scan = z->lastscan;
		z->lastscan = t;

		z->scanpos = 0;
		z->y += adam7[z->pass].dy;
		if (z->y >= z->dy)
			len = nextpass(z);
	}
	return oldn;
}

static Rawimage*
readslave(Biobuf *b)
{
	char type[5];
	int bpc, colorfmt, dx, dy, err, n, nchan, nout, useadam7;
	uint8_t *buf, *h;
	Rawimage *image;
	ZlibR zr;
	ZlibW zw;

	zr.w = &zw;

	image = NULL;
	buf = NULL;
	zw.scan = NULL;
	zw.lastscan = NULL;

	if (setjmp(errlab)) {
		if (image) {
			int i;
			for (i=0; i<4; i++) {
				if (image->chans[i])
					IND_FREE(image->chans[i]);
			}
			IND_FREE(image);
		}
		if (buf)
			IND_FREE(buf);
		if (zw.scan)
			IND_FREE(zw.scan);
		if (zw.lastscan)
			IND_FREE(zw.lastscan);

		return nil;
	}

	buf = pngmalloc(IDATSIZE, 0);
	Bread(b, buf, sizeof PNGmagic);
	if (memcmp(PNGmagic, buf, sizeof PNGmagic) != 0)
		sysfatal("bad PNGmagic\n");

	n = getchunk(b, type, buf, IDATSIZE);
	if (n < 13 || strncmp(type,"IHDR", 4) != 0)
		sysfatal("missing IHDR chunk\n");
	h = buf;
	dx = get4(h);
	h += 4;
	dy = get4(h);
	h += 4;
	if (dx <= 0 || dy <= 0)
		sysfatal("impossible image size %dx%d\n", dx, dy);

	bpc = *h++;
	colorfmt = *h++;
	nchan = 0;
	if (*h++ != 0)
		sysfatal("only deflate supported for now [%d]\n", h[-1]);
	if (*h++ != FilterNone)
		sysfatal("only FilterNone supported for now [%d]\n", h[-1]);
	useadam7 = *h++;

	image = pngmalloc(sizeof(Rawimage), 1);
	IND_MEMSET(image, 0, sizeof(Rawimage));
	image->width = dx;
	image->height = dy;
	image->gif_trindex = 0;
	nout = 0;
	switch(colorfmt){
	case 0:	/* grey */
		image->nchans = 1;
		image->chandesc = CY_;
		nout = 1;
		nchan = 1;
		break;
	case 2:	/* rgb */
		image->nchans = 1;
		image->chandesc = CRGB24;
		nout = 3;
		nchan = 3;
		break;
	case 3: /* indexed rgb with PLTE */
		image->nchans = 1;
		image->chandesc = CRGB24;
		nout = 3;
		nchan = 1;
		break;
	case 4:	/* grey+alpha */
		image->nchans = 1;
		image->chandesc = CYA16;
		nout = 2;
		nchan = 2;
		break;
	case 6:	/* rgb+alpha */
		image->nchans = 1;
		image->chandesc = CRGBA32;
		nout = 4;
		nchan = 4;
		break;
	default:
		sysfatal("unsupported color scheme %d\n", h[-1]);
	}
	image->chanlen = dx*dy*nout;
	image->chans[0] = pngmalloc(image->chanlen, 0);
	IND_MEMSET(image->chans[0], 0, image->chanlen);

	memset(&zr, 0, sizeof zr);
	zr.w = &zw;
	zr.io = b;
	zr.buf = buf;

	memset(&zw, 0, sizeof zw);
	if (useadam7)
		zw.pass = 1;
	zw.data = image->chans[0];
	zw.ndata = image->chanlen;
	zw.chandesc = image->chandesc;
	zw.noutchan = nout;

	zw.dx = dx;
	zw.dy = dy;
	zw.scanlen = (nchan*dx*bpc+7)/8+1;
	zw.scan = pngmalloc(zw.scanlen, 1);
	zw.lastscan = pngmalloc(zw.scanlen, 1);
	zw.nchan = nchan;
	zw.bpc = bpc;

	err = inflatezlib(&zw, zwrite, &zr, zread);

	if (err)
		sysfatal("inflatezlib %s\n", flateerr(err));

	IND_FREE(buf);
	IND_FREE(zw.scan);
	IND_FREE(zw.lastscan);
	return image;
}

Rawimage*
img_decode_png(uint8_t *imgbuf, int imglen)
{
	Rawimage* image;
	Biobuf b;

	if (Binit(&b, imgbuf, imglen) < 0)
		return nil;
	pnginit();
	image = readslave(&b);
	Bterm(&b);

	return image;
}

int img_info_png(uint8_t *imgbuf, int imglen, int *pwidth, int *pheight)
{
	uint8_t buf[4];
	Biobuf b;

	if (imgbuf == 0 || imglen < 32)
		ERR_OUT("imgbuf = %p, imglen = %d\n", imgbuf, imglen);

	if (Binit(&b, imgbuf, imglen) < 0)
		ERR_OUT("Binit\n");
	if (Bread(&b, NULL, 16) != 16)
		ERR_OUT("Bread 16\n");

	if (Bread(&b, buf, 4) != 4)
		ERR_OUT("Bread\n");
	*pwidth = (int)(((uint32_t)buf[2] << 8) + (uint32_t)buf[3]);
	if (Bread(&b, buf, 4) != 4)
		ERR_OUT("Bread\n");
	*pheight = (int)(((uint32_t)buf[2] << 8) + (uint32_t)buf[3]);

	PRINTF("width = %d, height = %d\n", *pwidth, *pheight);

	return 0;
Err:
	return -1;
}
