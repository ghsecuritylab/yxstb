
typedef struct Biobuf		Biobuf;

struct	Biobuf
{
	uint8_t *buffer;
	int offset;
	int length;
};

#define	Beof		-1

int Binit(Biobuf *bp, uint8_t *buffer, int length);
int Boffset(Biobuf *bp);
int Bseek(Biobuf *bp, int offset, int base);
int Bread(Biobuf *bp, uint8_t *buf, int count);
int Bgetc(Biobuf *bp);
int Bungetc(Biobuf *bp);
void Bterm(Biobuf *bp);
