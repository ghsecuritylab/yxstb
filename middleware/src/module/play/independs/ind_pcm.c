
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/Assertions.h"
#include "ind_pcm.h"
#include "ind_mem.h"

/*
        + read_idx
        |
        |       + read_max_idx = write_idx
        |       |
+-------+=======+-------+


        + write_idx
        |       + read_idx
        |       |       + read_max_idx
        |       |       |
+=======+-------+=======+
 */

#define PCM_BUFFER_SIZE		(4 * 1024)

struct IndPCM {
	int write_idx;
	int read_idx;
	int read_max_idx;

	int src_samplerate;
	int src_bitsperSample;
	int src_channels;

	int dst_samplemult;

	int	trs_mult;
	int	trs_size;//转换单元长度

	char pcm_buf[PCM_BUFFER_SIZE];
	int pcm_len;
	int pcm_off;

	int rng_size;
	char rng_buf[4];
};

ind_pcm_t ind_pcm_create(int buf_size)
{
	ind_pcm_t pcm = NULL;

	if ((buf_size <= 0) || (buf_size & 0x11))
		ERR_OUT("buf_size = %d\n", buf_size);

	pcm = (ind_pcm_t)IND_MALLOC(sizeof(struct IndPCM) + buf_size);
	if (pcm == NULL)
		return NULL;
	IND_MEMSET(pcm, 0, sizeof(struct IndPCM));
	pcm->rng_size = buf_size;

	return pcm;
Err:
	return NULL;
}

void ind_pcm_delete(ind_pcm_t pcm)
{
	if (pcm)
		IND_FREE(pcm);
}

void ind_pcm_src_set(ind_pcm_t pcm, int samplerate, int bitsperSample, int channels)
{
	if (pcm == NULL)
		return;
	pcm->write_idx = 0;
	pcm->read_max_idx = 0;
	pcm->read_idx = 0;

	pcm->src_samplerate = samplerate;
	pcm->src_bitsperSample = bitsperSample;
	pcm->src_channels = channels;

	pcm->dst_samplemult = 0;
	if (samplerate) {
		if (8000 == samplerate || 11025 == samplerate || 12000 == samplerate)
			pcm->dst_samplemult = 4;
		else if (16000 == samplerate || 22050 == samplerate || 24000 == samplerate)
			pcm->dst_samplemult = 2;
		else if (32000 == samplerate || 44100 == samplerate || 48000 == samplerate)
			pcm->dst_samplemult = 1;

		//PRINTF("samplerate = %d, bitsperSample = %d, channels = %d, trs_mult = %d, trs_size = %d\n", pcm->src_samplerate, pcm->src_bitsperSample, pcm->src_channels, pcm->trs_mult, pcm->trs_size);
	}

	pcm->pcm_len = 0;
	pcm->pcm_off = 0;
}

int ind_pcm_dst_get(ind_pcm_t pcm, int *samplerate, int *bitsperSample, int *channels)
{
	if (pcm->dst_samplemult == 0)
		ERR_OUT("samplemult = %d\n", pcm->dst_samplemult);

	if (samplerate)
		*samplerate = pcm->src_samplerate * pcm->dst_samplemult;
	if (bitsperSample)
		*bitsperSample = 16;
	if (bitsperSample)
		*channels = 2;

	return 0;
Err:
	return -1;
}

int ind_pcm_length(ind_pcm_t pcm)
{
	int len;

	if (pcm == NULL)
		return 0;

	if (pcm->read_max_idx > pcm->write_idx)
		len = pcm->read_max_idx + pcm->write_idx - pcm->read_idx;
	else
		len = pcm->write_idx - pcm->read_idx;
	//PRINTF("len = %d, write_idx = %d, read_idx = %d, read_max_idx = %d\n", len, pcm->write_idx, pcm->read_idx, pcm->read_max_idx);
	return len;
}

void ind_pcm_print(ind_pcm_t pcm)
{
	PRINTF("write_idx = %d, read_idx = %d, read_max_idx = %d\n", pcm->write_idx, pcm->read_idx, pcm->read_max_idx);
}

int ind_pcm_space(ind_pcm_t pcm)
{
	int space;

	if (pcm == NULL)
		return 0;

	space = pcm->rng_size - ind_pcm_length(pcm);

	return space;
}

int int_pcm_write(ind_pcm_t pcm, char *buf, int len)
{
	int max;

	if (pcm->read_max_idx > pcm->write_idx)
		max = pcm->read_idx - pcm->write_idx;
	else
		max = pcm->rng_size - pcm->write_idx;
	if (len >= max)
		len = max;
	if (len <= 0)
		return 0;

	IND_MEMCPY(pcm->rng_buf + pcm->write_idx, buf, len);
	pcm->write_idx += len;
	if (pcm->read_max_idx < pcm->write_idx)
		pcm->read_max_idx = pcm->write_idx;
	if (pcm->write_idx >= pcm->rng_size)
		pcm->write_idx = 0;

	return len;
}

int ind_pcm_write(ind_pcm_t pcm, char *buf, int len)
{
	int l, bytes;

	if (pcm == NULL)
		return 0;

	bytes = 0;
	while (len > 0) {
		l = int_pcm_write(pcm, buf, len);
		if (l <= 0)
			break;
		bytes += l;
		buf += l;
		len -= l;
	}

	return bytes;
}

void int_pcm_trans(ind_pcm_t pcm)
{
	int i, size, channels, samplemult, samplewidth;
	char *src, *dst;

	channels = pcm->src_channels;
	samplemult = pcm->dst_samplemult;
	samplewidth = pcm->src_bitsperSample / 8;

	size = channels * samplewidth;
	while (pcm->pcm_len < PCM_BUFFER_SIZE) {
		src = pcm->rng_buf + pcm->read_idx;
		if (pcm->read_max_idx - pcm->read_idx < size)
			break;

		for (i = 0; i < samplemult; i ++) {
			dst = pcm->pcm_buf + pcm->pcm_len;
			pcm->pcm_len += size;
			if (samplewidth == 2) {
				dst[0] = src[0];
				dst[1] = src[1];
			} else {
				dst[0] = 0;
				if (src[0] & 0x80)
					dst[1] = src[0] - 128;
				else
					dst[1] = src[0] - 127;
				pcm->pcm_len += size;
			}
			if (channels == 1) {
				dst = pcm->pcm_buf + pcm->pcm_len;
				pcm->pcm_len += size;
				if (samplewidth == 2) {
					dst[0] = src[0];
					dst[1] = src[1];
				} else {
					dst[0] = 0;
					if (src[0] & 0x80)
						dst[1] = src[0] - 128;
					else
						dst[1] = src[0] - 127;
					pcm->pcm_len += size;
				}
			}
		}

		pcm->read_idx += size;

		if (pcm->read_max_idx > pcm->write_idx && pcm->read_max_idx == pcm->read_idx) {
			pcm->read_max_idx = pcm->write_idx;
			pcm->read_idx = 0;
		}
	}
	if (pcm->pcm_len > PCM_BUFFER_SIZE)
		ERR_PRN("pcm_len = %d\n", pcm->pcm_len);
}

int ind_pcm_read_get(ind_pcm_t pcm, char** pbuf, int* plen)
{
	if (pcm->src_samplerate > 0 && pcm->pcm_len < PCM_BUFFER_SIZE)
		int_pcm_trans(pcm);

	if (pcm->pcm_len > 0) {
		if (pbuf)
			*pbuf = pcm->pcm_buf + pcm->pcm_off;
		if (plen)
			*plen = pcm->pcm_len - pcm->pcm_off;
	} else {
		if (pbuf)
			*pbuf = NULL;
		if (plen)
			*plen = 0;
	}

	return 0;
}

int ind_pcm_read_pop(ind_pcm_t pcm, int len)
{
	if (len < 0 || len > pcm->pcm_len)
		ERR_OUT("len = %d, pcm_len = %d\n", len, pcm->pcm_len);
	pcm->pcm_off += len;
	if (pcm->pcm_off >= pcm->pcm_len) {
		pcm->pcm_off = 0;
		pcm->pcm_len = 0;
	}

	return 0;
Err:
	return -1;
}

int ind_wav_parse(uint8_t* buf, int len, int* sampleRate, int* bitWidth, int* channels)
{
	int size;
	int hdr = 0;

	if (len < 44)
		ERR_OUT("len = %d\n", len);
	if (memcmp(buf, "RIFF", 4))
		ERR_OUT("sync = %02x %02x %02x %02x\n", (uint32_t)buf[0], (uint32_t)buf[1], (uint32_t)buf[2], (uint32_t)buf[3]);
	hdr += 8;
	buf += 8;
	hdr -= 8;
	if (memcmp(buf, "WAVE", 4))
		ERR_OUT("sync = %02x %02x %02x %02x\n", (uint32_t)buf[0], (uint32_t)buf[1], (uint32_t)buf[2], (uint32_t)buf[3]);
	hdr += 4;
	buf += 4;
	hdr -= 4;
	if (memcmp(buf, "fmt ", 4))
		ERR_OUT("sync = %02x %02x %02x %02x\n", (uint32_t)buf[0], (uint32_t)buf[1], (uint32_t)buf[2], (uint32_t)buf[3]);
	size = (uint32_t)buf[4] + ((uint32_t)buf[5] << 8) + ((uint32_t)buf[6] << 16) + ((uint32_t)buf[7] << 24);
	if (size + 4 > len)
		ERR_OUT("size = %d, hdr = %d, len = %d\n", size, hdr, len);
	hdr += 8;
	buf += 8;
	len -= 8;

    if (0x01 != buf[0] || 0x00 != buf[1])
        ERR_OUT("not WAVE_FORMAT_PCM!\n");

	if (channels)
		*channels = (uint32_t)buf[2] + ((uint32_t)buf[3] << 8);
	if (sampleRate)
		*sampleRate = (uint32_t)buf[4] + ((uint32_t)buf[5] << 8) + ((uint32_t)buf[6] << 8) + ((uint32_t)buf[7] << 8);
	if (bitWidth)
		*bitWidth = (uint32_t)buf[14] + ((uint32_t)buf[15] << 8);

	hdr += size;
	buf += size;
	len -= size;

	while(memcmp(buf, "data", 4)) {
		WARN_PRN("sync = %c%c%c%c %02x%02x%02x%02x\n", buf[0], buf[1], buf[2], buf[3], (uint32_t)buf[0], (uint32_t)buf[1], (uint32_t)buf[2], (uint32_t)buf[3]);
		size = (uint32_t)buf[4] + ((uint32_t)buf[5] << 8) + ((uint32_t)buf[6] << 16) + ((uint32_t)buf[7] << 24);
		size += 8;
		if (size > len)
			ERR_OUT("size = %d, hdr = %d, len = %d\n", size, hdr, len);
		hdr += size;
		buf += size;
		len -= size;
	}
	hdr += 8;

	return hdr;
Err:
	return -1;
}
