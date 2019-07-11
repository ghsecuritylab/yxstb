
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/Assertions.h"
#include "ind_rtp.h"

/*
	输入：
		buffer, length 也就是任意一个UDP包及长度（无论是否带RTP头）
	输出：
		seq：RTP序号，可以传NULL
		payload：RTP里有效负载长度，也就是TS流长度，可以传NULL
	返回值：
		RTP头的长度 等于 length - payload
 */
int ind_rtp_parse(char* buffer, int length, unsigned int* seq)
{
	unsigned char *buf;
	int hdr, cc, len;

	buf = (unsigned char *)buffer;
	len = length;

	if (len < 188)
		ERR_OUT("len = %d\n", len);
	if (buf[0] == 0x47)
		return 0;

	if (0x80 != (buf[0] & 0xc0))
		ERR_OUT("version = %02x not support\n", (unsigned int)buf[0]);
	cc = buf[0] & 0xf;
	//if (0x21 != (buf[1] & 0x7f))
	//	ERR_OUT("PT = %x not support\n", (unsigned int)buf[1]);
	if (seq)
		*seq = ((unsigned int)buf[2] << 8) + (unsigned int)buf[3];
	hdr = 12 + cc * 4;
	if (hdr + 4 >= len)
		ERR_OUT("hdr = %d, len = %d", hdr, len);
	if (buf[0] & 0x10) {
	    buf += hdr;
		hdr += 4 + (int)(((unsigned int)buf[2] << 8) | (unsigned int)buf[3]) * 4;
	}

	return hdr;
Err:
	return -1;
}

int ind_rtp_parse_ex(char* buffer, int length, unsigned int* seq, unsigned int* ssrc)
{
	unsigned char *buf;
	int hdr, cc, len;

	buf = (unsigned char *)buffer;
	len = length;

	if (len < 188)
		ERR_OUT("len = %d\n", len);
	if (buf[0] == 0x47)
		return 0;

	if (0x80 != (buf[0] & 0xc0))
		ERR_OUT("version = %02x not support\n", (unsigned int)buf[0]);
	cc = buf[0] & 0xf;
	//if (0x21 != (buf[1] & 0x7f))
	//	ERR_OUT("PT = %x not support\n", (unsigned int)buf[1]);
	if (seq)
		*seq = ((unsigned int)buf[2] << 8) + (unsigned int)buf[3];

	hdr = 12 + cc * 4;
	if (hdr + 4 >= len)
		ERR_OUT("hdr = %d, len = %d", hdr, len);
	if (buf[0] & 0x10)
		hdr += 4 + (int)(((unsigned int)buf[hdr + 2] << 8) | (unsigned int)buf[hdr + 3]) * 4;

    if (0x21 != (buf[1] & 0x7f))
        ERR_PRN("PT = %x\n", (unsigned int)buf[1]);

    if ((buf[1] & 0x7f) == 0x60) {
        if (ssrc)
            *ssrc = ((unsigned int)buf[8] << 24) + ((unsigned int)buf[9] << 16) + ((unsigned int)buf[10] << 8) + (unsigned int)buf[11];
        if (seq)
            *seq = ((unsigned int)buf[hdr] << 8) + (unsigned int)buf[hdr + 1];
    }

	return hdr;
Err:
	return -1;
}
