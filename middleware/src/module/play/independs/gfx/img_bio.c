
#include "img_decode.h"

int	Binit(Biobuf* bp, uint8_t *buffer, int length)
{
	if (bp == NULL || buffer == NULL || length < 0 || length > 16*1024*1024)
		ERR_OUT("bp = %p, buffer = %p, length = %d\n", bp, buffer, length);
	bp->buffer = buffer;
	bp->length = length;
	bp->offset = 0;

	return 0;
Err:
	return Beof;
}

int Boffset(Biobuf *bp)
{
	if (bp == NULL)
		ERR_OUT("bp is NULL\n");
	return bp->offset;
Err:
	return Beof;
}

int Bseek(Biobuf *bp, int offset, int base)
{
	if (bp == NULL)
		ERR_OUT("bp is NULL\n");
	if (base == 1)
		offset += bp->offset;
	if (offset < 0 || offset > bp->length)
		ERR_OUT("offset = %d, length = %d\n", offset, bp->length);
	bp->offset = offset;
	return 0;
Err:
	return Beof;
	
}

int Bread(Biobuf *bp, uint8_t *buf, int count)
{
	int len;

	if (bp == NULL || count < 0)
		ERR_OUT("bp = %p, length = %d\n", bp, count);

	//PRINTF("@@@@@@@@: offset = %d, length = %d, count = %d/%d\n", bp->offset, bp->length, count, bp->length - bp->offset);
	len = bp->length - bp->offset;

	if (len == 0)
		return Beof;

	if (len > count)
		len = count;
	if (buf)
		IND_MEMCPY(buf, bp->buffer + bp->offset, len);
	bp->offset += len;

	return len;
Err:
	return Beof;
}

int	Bgetc(Biobuf *bp)
{
	int ch;

	if (bp == NULL)
		ERR_OUT("bp = %p\n", bp);

	//PRINTF("@@@@@@@@: offset = %d, length = %d\n", bp->offset, bp->length);
	if (bp->offset >= bp->length)
		ERR_OUT("reach end!\n");
	ch = (int)bp->buffer[bp->offset];
	bp->offset ++;

	return ch;
Err:
	return Beof;
}

int
Bungetc(Biobuf *bp)
{
	if (bp == NULL)
		ERR_OUT("bp = %p\n", bp);

	if (bp->offset > 0)
		bp->offset--;
	return 0;
Err:
	return Beof;
}

void Bterm(Biobuf *bp)
{
}
