
#ifndef __TS_SIZE__
#define __TS_SIZE__

struct u_bits {
	unsigned char *buf;
	int len;
	int idx;
	int bit;
	unsigned int ue;
	int se;
};

unsigned int ts_u_v(struct u_bits *bits, int n);
int ts_ue_v(struct u_bits *bits);
int ts_se_v(struct u_bits *bits);

#endif//__TS_SIZE__

