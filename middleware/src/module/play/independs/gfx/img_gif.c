
#include "img_decode.h"
#include "ind_mem.h"

typedef struct Entry Entry;
typedef struct Header Header;

struct Entry{
	int		prefix;
	int		exten;
};

struct Header{
	Biobuf	*fd;
	jmp_buf	errlab;
	char 	vers[8];
	uint8_t 	globalcmap[3*256];
	int		globalcmaplen;
	int		screenw;
	int		screenh;
	int		globalfields;
	int		fields;
	int		bgrnd;
	int		aspect;
	int		flags;
	int		delay;
	int		trindex;
	int		loopcount;
	Entry	tbl[4096];
	Rawimage	*image;

	uint8_t	*pic;
};

#ifdef DEBUG_BUILD
static char		readerr[] = "ReadGIF: read error";
static char		extreaderr[] = "ReadGIF: can't read extension";
static char		memerr[] = "ReadGIF: malloc failed";
#endif

static void			readarray(Header*);
static Rawimage*	readone(Header*);
static void			readheader(Header*);
static void			skipextension(Header*);
static void			readcmap(Header*, uint8_t*, int);
static uint8_t*		decode(Header*, Rawimage*, Entry*);
static void			interlace(Header*, Rawimage*);

static
void
clear(void *pp)
{
	void **p = (void**)pp;

	if (*p){
		IND_FREE(*p);
		*p = nil;
	}
}

static
void
giffreeall(Header *h, int freeimage)
{
	if (h->fd){
		Bterm(h->fd);
		h->fd = nil;
	}
	clear(&h->pic);
	if (h->image){
		clear(&h->image->cmap);
		clear(&h->image->chans[0]);
		clear(&h->image);
	}
}

#ifdef DEBUG_BUILD
#define giferror(h, X, ...) \
do {                        \
    ERR_PRN(X, ##__VA_ARGS__); \
    giffreeall(h, 1);       \
    longjmp(h->errlab, 1);  \
}while(0)
#else
#define giferror(h, X...)							\
do{													\
	giffreeall(h, 1);								\
	longjmp(h->errlab, 1);							\
}while(0)
#endif


Rawimage*
img_decode_gif (uint8_t *imgbuf, int imglen)
{
	Rawimage *image;
	Biobuf b;
	Header *h;

	if (Binit(&b, imgbuf, imglen) < 0)
		return nil;
	h = IND_MALLOC(sizeof(Header));
	if (h == nil){
		Bterm(&b);
		return nil;
	}
	IND_MEMSET(h, 0, sizeof(Header));
	h->fd = &b;

	if (setjmp(h->errlab)) {
		image = nil;
	} else {
		readarray(h);
		image = h->image;
		h->image = nil;
	}
	giffreeall(h, 0);
	IND_FREE(h);
	return image;
}

static
void
inittbl(Header *h)
{
	int i;
	Entry *tbl;

	tbl = h->tbl;
	for (i=0; i<258; i++) {
		tbl[i].prefix = -1;
		tbl[i].exten = i;
	}
}

static
void
readarray(Header *h)
{
	Entry *tbl;
	Rawimage *image;
	int c;

	tbl = h->tbl;

	readheader(h);

	if (h->globalfields & 0x80) {
		h->globalcmaplen = 3 * (1 << ((h->globalfields&7)+1));
		readcmap(h, h->globalcmap, h->globalcmaplen);
	}

	for (;;){
		switch(c = Bgetc(h->fd)){
		case Beof:
			goto Return;

		case 0x21:	/* Extension (ignored) */
			skipextension(h);
			break;

		case 0x2C:	/* Image Descriptor */
			inittbl(h);
			image = readone(h);
			h->image = image;
			image->width = h->screenw;
			image->height = h->screenh;
			image->cmap = IND_MALLOC(3*256);
			if (h->fields & 0x80){
				image->cmaplen = 3 * (1 << ((h->fields&7)+1));
				readcmap(h, image->cmap, image->cmaplen);
			}else{
				image->cmaplen = h->globalcmaplen;
				memmove(image->cmap, h->globalcmap, h->globalcmaplen);
			}
			image->chans[0] = decode(h, image, tbl);
			if (h->fields & 0x40)
				interlace(h, image);
			//new->giffields = h->fields;
			//new->gifflags = h->flags;
			//new->gifdelay = h->delay;
			if (h->flags & 0x01)
				image->gif_trindex = h->trindex;
			else
				image->gif_trindex = -1;
			//new->gifloopcount = h->loopcount;

			goto Return;

		case 0x3B:	/* Trailer */
			goto Return;

		default:
			fprint(2, "ReadGIF: unknown block type: 0x%.2x\n", c);
			goto Return;
		}
	}

   Return:
	if (h->image==nil || h->image->chans[0] == nil)
		giferror(h, "ReadGIF: no picture in file");
}

static
void
readheader(Header *h)
{
	uint8_t 	buf[16];

	if (Bread(h->fd, buf, 13) != 13)
		giferror(h, "ReadGIF: can't read header");
	memmove(h->vers, buf, 6);
	if (strncmp(h->vers, "GIF87a", 6)!=0 &&  strncmp(h->vers, "GIF89a", 6)!=0)
		giferror(h, "ReadGIF: can't recognize format %s", h->vers);
	h->screenw = buf[6]+(buf[7]<<8);
	h->screenh = buf[8]+(buf[9]<<8);
	h->globalfields = buf[10];
	h->bgrnd = buf[11];
	h->aspect = buf[12];
	h->flags = 0;
	h->delay = 0;
	h->trindex = 0;
	h->loopcount = -1;
}

static
void
readcmap(Header *h, uint8_t *cmap, int size)
{
	if (Bread(h->fd, cmap, size) != size)
		giferror(h, "ReadGIF: short read on color map");
}

static
Rawimage*
readone(Header *h)
{
	Rawimage *i;
	uint8_t buf[12];

	if (Bread(h->fd, buf, 9) != 9)
		giferror(h, "ReadGIF: can't read image descriptor");
	i = IND_MALLOC(sizeof(Rawimage));
	if (i == nil)
		giferror(h, memerr);
	IND_MEMSET(i, 0, sizeof(Rawimage));
	i->gif_x = buf[0]+(buf[1]<<8);
	i->gif_y = buf[2]+(buf[3]<<8);
	i->gif_width = buf[4]+(buf[5]<<8);
	i->gif_height = buf[6]+(buf[7]<<8);
	h->fields = buf[8];
	i->nchans = 1;
	i->chandesc = CRGB1;
	return i;
}


static
int
readdata(Header *h, uint8_t *data)
{
	int nbytes, n;

	nbytes = Bgetc(h->fd);
	if (nbytes < 0)
		giferror(h, "ReadGIF: can't read data");
	if (nbytes == 0)
		return 0;
	n = Bread(h->fd, data, nbytes);
	if (n < 0)
		giferror(h, "ReadGIF: can't read data");
	if (n != nbytes)
		fprint(2, "ReadGIF: short data subblock\n");
	return n;
}

static
void
graphiccontrol(Header *h)
{
	uint8_t buf[8];
	if (Bread(h->fd, buf, 5+1) != 5+1)
		giferror(h, readerr);
	h->flags = buf[1];
	h->delay = buf[2]+(buf[3]<<8);
	h->trindex = buf[4];
}

static
void
skipextension(Header *h)
{
	int type, hsize, hasdata, n;
	uint8_t data[256] = {0};

	hsize = 0;
	hasdata = 0;

	type = Bgetc(h->fd);
	switch(type){
	case Beof:
		giferror(h, extreaderr);
		break;
	case 0x01:	/* Plain Text Extension */
		hsize = 13;
		hasdata = 1;
		break;
	case 0xF9:	/* Graphic Control Extension */
		graphiccontrol(h);
		return;
	case 0xFE:	/* Comment Extension */
		hasdata = 1;
		break;
	case 0xFF:	/* Application Extension */
		hsize = Bgetc(h->fd);
		/* standard says this must be 11, but Adobe likes to put out 10-byte ones,
		 * so we pay attention to the field. */
		hasdata = 1;
		break;
	default:
		giferror(h, "ReadGIF: unknown extension");
	}
	if (hsize>0 && Bread(h->fd, data, hsize) != hsize)
		giferror(h, extreaderr);
	if (!hasdata)
		return;

	/* loop counter: Application Extension with NETSCAPE2.0 as string and 1 <loop.count> in data */
	if (type == 0xFF && hsize==11 && memcmp(data, "NETSCAPE2.0", 11)==0){
		n = readdata(h, data);
		if (n == 0)
			return;
		if (n==3 && data[0]==1)
			h->loopcount = data[1] | (data[2]<<8);
	}
	while(readdata(h, data) != 0)
		;
}

static
uint8_t*
decode(Header *h, Rawimage *i, Entry *tbl)
{
	int c, incode, codesize, CTM, EOD, pici, datai, stacki, nbits, sreg, fc, code, piclen;
	int csize, nentry, maxentry, first, ocode, ndata, nb;
	uint8_t *pic;
	uint8_t stack[4096], data[256];

	if (Bread(h->fd, data, 1) != 1)
		giferror(h, "ReadGIF: can't read data");
	codesize = data[0];
	if (codesize>8 || 0>codesize)
		giferror(h, "ReadGIF: can't handle codesize %d", codesize);
	if (i->cmap!=nil && i->cmaplen!=3*(1<<codesize)
	  && (codesize!=2 || i->cmaplen!=3*2)) /* peculiar GIF bitmap files... */
		giferror(h, "ReadGIF: codesize %d doesn't match color map 3*%d", codesize, i->cmaplen/3);

	CTM =1<<codesize;
	EOD = CTM+1;

	piclen = i->gif_width*i->gif_height;
	i->chanlen = piclen;
	pic = IND_MALLOC(piclen);
	if (pic == nil)
		giferror(h, memerr);
	h->pic = pic;
	pici = 0;
	ndata = 0;
	datai = 0;
	nbits = 0;
	sreg = 0;
	fc = 0;

    Loop:
	for (;;){
		csize = codesize+1;
		nentry = EOD+1;
		maxentry = (1<<csize)-1;
		first = 1;
		ocode = -1;

		for (;; ocode = incode) {
			while(nbits < csize) {
				if (datai == ndata){
					ndata = readdata(h, data);
					if (ndata == 0)
						goto Return;
					datai = 0;
				}
				c = data[datai++];
				sreg |= c<<nbits;
				nbits += 8;
			}
			code = sreg & ((1<<csize) - 1);
			sreg >>= csize;
			nbits -= csize;

			if (code == EOD){
				ndata = readdata(h, data);
				if (ndata != 0)
					fprint(2, "ReadGIF: unexpected data past EOD");
				goto Return;
			}

			if (code == CTM)
				goto Loop;

			stacki = (sizeof stack)-1;

			incode = code;

			/* special case for KwKwK */
			if (code == nentry) {
				stack[stacki--] = fc;
				code = ocode;
			}

			if (code > nentry)
				giferror(h, "ReadGIF: bad code %x %x", code, nentry);

			for (c=code; c>=0; c=tbl[c].prefix)
				stack[stacki--] = tbl[c].exten;

			nb = (sizeof stack)-(stacki+1);
			if (pici+nb > piclen){
				/* this common error is harmless
				 * we have to keep reading to keep the blocks in sync */
				;
			}else{
				memmove(pic+pici, stack+stacki+1, sizeof stack - (stacki+1));
				pici += nb;
			}

			fc = stack[stacki+1];

			if (first){
				first = 0;
				continue;
			}
			#define early 0 /* peculiar tiff feature here for reference */
			if (nentry == maxentry-early) {
				if (csize >= 12)
					continue;
				csize++;
				maxentry = (1<<csize);
				if (csize < 12)
					maxentry--;
			}
			tbl[nentry].prefix = ocode;
			tbl[nentry].exten = fc;
			nentry++;
		}
	}

Return:
	h->pic = nil;
	return pic;
}

static
void
interlace(Header *h, Rawimage *image)
{
	uint8_t *pic;
	int width, height, yy, y;
	uint8_t *ipic;

	pic = image->chans[0];
	width = image->gif_width;
	height = image->gif_height;
	ipic = IND_MALLOC(width*height);
	if (ipic == nil)
		giferror(h, "ReadGIF: malloc");

	/* Group 1: every 8th row, starting with row 0 */
	yy = 0;
	for (y=0; y<height; y+=8){
		memmove(&ipic[y*width], &pic[yy*width], width);
		yy++;
	}

	/* Group 2: every 8th row, starting with row 4 */
	for (y=4; y<height; y+=8){
		memmove(&ipic[y*width], &pic[yy*width], width);
		yy++;
	}

	/* Group 3: every 4th row, starting with row 2 */
	for (y=2; y<height; y+=4){
		memmove(&ipic[y*width], &pic[yy*width], width);
		yy++;
	}

	/* Group 4: every 2nd row, starting with row 1 */
	for (y=1; y<height; y+=2){
		memmove(&ipic[y*width], &pic[yy*width], width);
		yy++;
	}

	IND_FREE(image->chans[0]);
	image->chans[0] = ipic;
}

int img_info_gif (uint8_t *imgbuf, int imglen, int *pwidth, int *pheight)
{
	if (imgbuf == 0 || imglen < 13)
		ERR_OUT("imgbuf = %p, imglen = %d\n", imgbuf, imglen);

	*pwidth = (int)((uint32_t)imgbuf[6] + ((uint32_t)imgbuf[7] << 8));
	*pheight = (int)((uint32_t)imgbuf[8] + ((uint32_t)imgbuf[9] << 8));

	return 0;
Err:
	return -1;
}
