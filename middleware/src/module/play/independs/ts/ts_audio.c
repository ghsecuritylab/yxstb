
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/Assertions.h"
#include "ind_ts.h"
#include "ind_mem.h"

#define ES_FRAME_SIZE		8*1024 //最大帧长 8064
#define ES_BUFFER_SIZE		32*1024

#define TS_PSI_SECOND		45000

#define TS_PAT_LENGTH		(1 + 12 + 4)
#define TS_PAT_ADAPT		(184 - TS_PAT_LENGTH)
#define TS_PAT_OFFSET		(188 - TS_PAT_LENGTH)

#define TS_PMT_LENGTH		(1 + 17 + 4)
#define TS_PMT_ADAPT		(184 - TS_PMT_LENGTH)
#define TS_PMT_OFFSET		(188 - TS_PMT_LENGTH)

#define TS_PES_LENGTH		(6 + 8)

#define	TS_PMT_PID			0x42
#define	TS_PROG_NUM			0x01
#define	TS_AUDIO_PID		0x44

struct ts_audio {
	char es_buf[ES_BUFFER_SIZE];
	int es_suff;
	int es_off;
	int es_len;
	int es_skip;
	unsigned int es_type;
	unsigned int es_counter;

	unsigned int pat_counter;
	unsigned int pmt_counter;
	unsigned int psi_pts;

	int frame_length;
	int frame_remain;
	int frame_first;

	unsigned int pts;

	uint32_t bitrate;
	uint32_t duration;
};

void ind_ts_header(char *packet, unsigned int pid, int start, unsigned int conter)
{
	unsigned char *p = (unsigned char *)packet;

	p[0] = 0x47;
	if (start)
		p[1] = 0x40;
	else
		p[1] = 0x00;
	p[1] |= (unsigned char)(pid >> 8);
	p[2] = (unsigned char)pid;
	p[3] = 0x10 | (unsigned char)(conter & 0xf);
}

//len = 0 pcr and playload
//len = 188 only pcr;
void ind_ts_adaptation(char *packet, int len, unsigned int pcr)
{
	int i, max;
	unsigned char *p = (unsigned char *)packet;

	if (len < 0 || len > 184)
		ERR_OUT("adaptation len = %d\n", len);

	if (len == 0)
		return;

	p[3] |= 0x20;

	if (len == 184)
		p[3] &= 0xef;//no data_byte

	p[4] = (unsigned char)(len - 1);
	if (len <= 1)
		return;
	p[5] = 0;
	i = 6;

	if (len >= 8 && pcr) {
		p[5] |= 0x10;
		i += 6;

		p[6] = (unsigned char)(pcr >> 24);
		p[7] = (unsigned char)(pcr >> 16);
		p[8] = (unsigned char)(pcr >>  8);
		p[9] = (unsigned char)(pcr >>  0);

		p[10] = 0x00;
		p[11] = 0x00;
	}

	max = 4 + len;
	for (; i < max; i ++)
		p[i] = 0xff;
Err:
	return;
}

void ind_ts_pat(char *packet, unsigned int conter, uint32_t prognum, uint32_t pmtpid)
{
	unsigned int crc;
	unsigned char *p;

	ind_ts_header(packet, 0, 1, conter);
	packet[4] = 0;//pointer_field
	IND_MEMSET(packet + 5, 0xff, 183);

	p = (unsigned char *)(packet + 5);

	p[0] = 0x00;//table_id -- program_association_section
	p[1] = 0x80 | 0x30;//section_syntax_indicator + reserved
	p[2] = 0x0d;//section_length
	p[3] = 0x26;
	p[4] = 0x11;//transport_stream_id
	p[5] = 0xc0 | 0x08 | 0x01;//reserved + version_number + current_next_indicator
	p[6] = 0x00;//section_number
	p[7] = 0x00;//last_section_number

	p[ 8] = (unsigned char)(prognum >> 8);
	p[ 9] = (unsigned char)prognum;//program_number
	p[10] = 0xe0 | (unsigned char)(pmtpid >> 8);//reserved
	p[11] = (unsigned char)(pmtpid & 0xff);//program_map_PID

	crc = ind_ts_crc32(p, 12);
	p[12] = (unsigned char)(crc >> 24);
	p[13] = (unsigned char)(crc >> 16);
	p[14] = (unsigned char)(crc >> 8);
	p[15] = (unsigned char)(crc >> 0);//CRC
}

void ind_ts_pmt(char *packet, unsigned int conter, uint32_t prognum, ts_elems_t elems)
{
	unsigned int i, proglen, off, crc;
	unsigned char *p;

	ind_ts_header(packet, (unsigned int)elems->pmt_pid, 1, conter);
	packet[4] = 0;//pointer_field
	IND_MEMSET(packet + 5, 0xff, 183);

	proglen = elems->elem_num * 5;
	p = (unsigned char *)(packet + 5);

	p[0] = 0x02;//table_id -- TS_program_map_section
	p[1] = 0x80 | 0x30;//section_syntax_indicator + reserved
	p[2] = (unsigned char)(9 + proglen + 4);//section_length
	p[3] = 0x00;
	p[4] = (unsigned char)prognum;//program_number
	p[5] = 0xc0 | 0x02 | 0x01;//reserved + version_number + current_next_indicator
	p[6] = 0x00;//section_number
	p[7] = 0x00;//last_section_number

	p[ 8] = 0xe0 | (elems->pcr_pid >> 8);//reserved
	p[ 9] = (unsigned char)elems->pcr_pid;//PCR_PID
	p[10] = 0xf0;//reserved
	p[11] = 0x00;//program_info_length

	off = 12;
	for (i = 0; i < elems->elem_num; i ++) {
		p[off ++] = (unsigned char)elems->elem_type[i];//stream_type
		p[off ++] = 0xe0 | (elems->elem_pid[i] >> 8);//reserved
		p[off ++] = (unsigned char)elems->elem_pid[i];//elementary_PID
		p[off ++] = 0xf0;//reserved
		p[off ++] = 0x00;//ES_info_length
	}

	crc = ind_ts_crc32(p, 12 + proglen);
	p[off ++] = (unsigned char)(crc >> 24);
	p[off ++] = (unsigned char)(crc >> 16);
	p[off ++] = (unsigned char)(crc >> 8);
	p[off ++] = (unsigned char)(crc >> 0);//CRC
}

int ind_ts_filter(unsigned char *sbuf, int slen, uint32_t fpid, unsigned char *dbuf, int dlen)
{
	int bytes;
	uint32_t pid;

	bytes = 0;
	for ( ; slen >= 188; sbuf += 188, slen -= 188) {
		pid =(((uint32_t)sbuf[1] & 0x1f) << 8) + sbuf[2];
		if (pid != fpid)
			continue;
		if (dlen < bytes + 188)
			ERR_OUT("dlen = %d, bytes = %d\n", dlen, bytes);
		IND_MEMCPY(dbuf + bytes, sbuf, 188);
		bytes += 188;
	}

	return bytes;
Err:
	return -1;
}

//#define ENABLE_VIDEO_PCR

int ind_ts_cpid(ts_elems_t ses, ts_elems_t des)
{
	int i, num;

	num = ses->elem_num;
	des->elem_num = num;

	des->pmt_pid = TS_DVB_PMT_PID;

	for (i = 0; i < num; i ++) {
		des->elem_pid[i] = (unsigned short)(TS_DVB_ELEM_PID + i);
		des->elem_type[i] = ses->elem_type[i];
	}
#ifdef ENABLE_VIDEO_PCR
	des->pcr_pid = des->elem_pid[0];
#else
	des->pcr_pid = TS_DVB_ELEM_PID + TS_DVB_ELEM_NUM;
#endif

	return 0;
}

int ind_ts_xpid(unsigned char *sbuf, unsigned char *dbuf, ts_elems_t ses, ts_elems_t des)
{
	int i, num;
	uint32_t pid;

	num = ses->elem_num;

	pid =(((unsigned short)sbuf[1] & 0x1f) << 8) + sbuf[2];

	for (i = 0; i < num; i ++) {
		if (pid == ses->elem_pid[i]) {
			pid = des->elem_pid[i];

			IND_MEMCPY(dbuf, sbuf, 188);

			dbuf[1] = (dbuf[1] & 0xe0) | (unsigned char)(pid >> 8);
			dbuf[2] = (unsigned char)pid;

			return 1;
		}
	}

	return 0;
}

int ind_ts_xpcr(unsigned char *sbuf, unsigned char *dbuf, ts_elems_t ses, ts_elems_t des, unsigned int counter)
{
	int i, num;
	unsigned int length;
	unsigned short pid, chg;

	if (des->pcr_pid == des->elem_pid[0])
		return 0;

	num = ses->elem_num;

	chg = 0;
	pid =(((unsigned short)sbuf[1] & 0x1f) << 8) + sbuf[2];
	if (pid != ses->pcr_pid)
		return 0;

	for (i = 0; i < num; i ++) {
		if (pid == ses->elem_pid[i]) {
			if ((sbuf[3] & 0x20) == 0)
				return 0;
	
			if (sbuf[4] < 7)
				return 0;
	
			if ((sbuf[5] & 0x10) == 0)
				return 0;

			chg = 1;
			break;
		}
	}

	IND_MEMCPY(dbuf, sbuf, 188);

	pid = des->pcr_pid;
	dbuf[1] = (sbuf[1] & 0xe0) | (unsigned char)(pid >> 8);
	dbuf[2] = (unsigned char)pid;
	dbuf[3] = 0x10 | (unsigned char)(counter & 0xf);

	if (chg == 1) {
		length = (unsigned int)sbuf[4];
		if (length < 183)
			IND_MEMSET(dbuf + 5 + length, 0xff, 183 - length);
	}

	return 1;
}

void ind_ts_pmt_e1(char *packet, unsigned int es_type, unsigned int conter)
{
	unsigned int crc;
	unsigned char *p = (unsigned char *)packet;

	ind_ts_header(packet, TS_PMT_PID, 1, conter);
	ind_ts_adaptation(packet, TS_PMT_ADAPT, 0);
	p[TS_PMT_OFFSET + 0] = 0x00;//pointer_field
	p[TS_PMT_OFFSET + 1] = 0x02;//table_id -- TS_program_map_section
	p[TS_PMT_OFFSET + 2] = 0x80 | 0x30;//section_syntax_indicator + reserved
	p[TS_PMT_OFFSET + 3] = 0x12;//section_length
	p[TS_PMT_OFFSET + 4] = 0x00;
	p[TS_PMT_OFFSET + 5] = TS_PROG_NUM;//program_number
	p[TS_PMT_OFFSET + 6] = 0xc0 | 0x02 | 0x01;//reserved + version_number + current_next_indicator
	p[TS_PMT_OFFSET + 7] = 0x00;//section_number
	p[TS_PMT_OFFSET + 8] = 0x00;//last_section_number

	p[TS_PMT_OFFSET +  9] = 0xe0;//reserved
	p[TS_PMT_OFFSET + 10] = TS_AUDIO_PID;//PCR_PID
	p[TS_PMT_OFFSET + 11] = 0xf0;//reserved
	p[TS_PMT_OFFSET + 12] = 0x00;//program_info_length

	p[TS_PMT_OFFSET + 13] = (unsigned char)es_type;//stream_type
	p[TS_PMT_OFFSET + 14] = 0xe0;//reserved
	p[TS_PMT_OFFSET + 15] = TS_AUDIO_PID;//elementary_PID
	p[TS_PMT_OFFSET + 16] = 0xf0;//reserved
	p[TS_PMT_OFFSET + 17] = 0x00;//ES_info_length

	crc = ind_ts_crc32(p + TS_PMT_OFFSET + 1, 17);
	p[TS_PMT_OFFSET + 18] = (unsigned char)(crc >> 24);
	p[TS_PMT_OFFSET + 19] = (unsigned char)(crc >> 16);
	p[TS_PMT_OFFSET + 20] = (unsigned char)(crc >> 8);
	p[TS_PMT_OFFSET + 21] = (unsigned char)(crc >> 0);//CRC
}

void ind_ts_pes(char *pes, unsigned int id, unsigned int len, unsigned int pts)
{
	unsigned char *p = (unsigned char *)pes;

#if (TS_PES_HLEN != 14)
#error TS_PES_HLEN error!
#endif

	p[0] = 0x00;
	p[1] = 0x00;
	p[2] = 0x01;//packet_start_code_prefix
	p[3] = (unsigned char)id;//stream_id
	p[4] = (unsigned char)(len >> 8);
	p[5] = (unsigned char)(len >> 0);//PES_packet_length

	p[6] = 0x84;
	p[7] = 0x80;//PTS_DTS_flags
	p[8] = 0x05;//PES_header_data_length
	p[9] = 0x20 |	(pts >> 28)	| 0x01;
	p[10] = 		(pts >> 21);
	p[11] = 		(pts >> 13)	| 0x01;
	p[12] = 		(pts >> 6);
	p[13] = 		(pts << 2)	| 0x01;
}


//bits V1,L1 V1,L2 V1,L3 V2,L1 V2,L2 V2,L3
static unsigned int g_mpg_bitrate_sss[16][2][3] = {
	{{0  ,  0 ,  0 ,}, {0  , 0  , 0  }},
	{{32 , 32 , 32 ,}, {32 , 8  , 8  }},
	{{64 , 48 , 40 ,}, {48 , 16 , 16 }},
	{{96 , 56 , 48 ,}, {56 , 24 , 24 }},
	{{128, 64 , 56 ,}, {64 , 32 , 32 }},
	{{160, 80 , 64 ,}, {80 , 40 , 40 }},
	{{192, 96 , 80 ,}, {96 , 48 , 48 }},
	{{224, 112, 96 ,}, {112, 56 , 56 }},
	{{256, 128, 112,}, {128, 64 , 64 }},
	{{288, 160, 128,}, {144, 80 , 80 }},
	{{320, 192, 160,}, {160, 96 , 96 }},
	{{352, 224, 192,}, {176, 112, 112}},
	{{384, 256, 224,}, {192, 128, 128}},
	{{416, 320, 256,}, {224, 144, 144}},
	{{448, 384, 320,}, {256, 160, 160}},
	{{0,   0,   0,  }, {0,   0,   0  }}
};

static unsigned int g_mpg_samples_ss[3][3] = {
	{384 , 384 , 384 },
	{1152, 1152, 1152},
	{1152, 576 , 576 }
};

//bits MPEG1 MPEG2 MPEG2.5
static unsigned int g_mpg_samrate_ss[4][3] = {
	{44100, 22050, 11025},
	{48000, 24000, 12000},
	{32000, 16000, 8000 },
	{0,	 0,	 0,   }
};

struct ind_mpg_info {
	uint32_t version;//MPEG Audio version
	uint32_t layer;//Layer description
	uint32_t protection;//Protected by CRC
	uint32_t bitrate;
	uint32_t samrate;
	uint32_t padding;
	uint32_t channel;
};

static int ind_frame_mpg(struct ts_audio *a2t, unsigned char *buffer, int length, int *p_len, unsigned int *p_type, unsigned int *p_pts)
{
	unsigned char *p;
	unsigned int l, len;
	struct ind_mpg_info info;
	unsigned int bitrate, samrate, samples;

	p = buffer;
	l = length;

	info.version	= (p[1] & 0x18) >> 3;
	info.layer		= (p[1] & 0x06) >> 1;
	info.bitrate	= (p[2] & 0xf0) >> 4;
	info.samrate	= (p[2] & 0x0c) >> 2;
	info.padding	= (p[2] & 0x02) >> 1;
	info.channel	= (p[3] & 0xc0) >> 6;
	p += 4;
	l -= 4;

	if (info.layer == 0)
		ERR_OUT("layer is 0\n");

	info.layer = 3 - info.layer;

	if (info.version == 1)
		ERR_OUT("version is 1\n");

	if (info.version == 0 || info.version == 2) {
		bitrate = g_mpg_bitrate_sss[info.bitrate][1][info.layer] * 1000;
	} else {
		bitrate = g_mpg_bitrate_sss[info.bitrate][0][info.layer] * 1000;
	}
	if (bitrate == 0)
		ERR_OUT("bitrate is 0\n");

	if (info.version == 0) {
		samrate = g_mpg_samrate_ss[info.samrate][2];
		samples = g_mpg_samples_ss[info.layer][2];
	} else if (info.version == 2) {
		samrate = g_mpg_samrate_ss[info.samrate][1];
		samples = g_mpg_samples_ss[info.layer][1];
	} else {
		samrate = g_mpg_samrate_ss[info.samrate][0];
		samples = g_mpg_samples_ss[info.layer][0];
	}
	if (samrate == 0)
		ERR_OUT("samrate is 0\n");

	//max samples 1152
	//max bitrate 448 * 1000
	//min samrate 8000
	//1152 / 8 * (448 * 1000) / 8000 = 8064
	len = (((samples / 8) * bitrate) / samrate) + info.padding;
	if (l < len)
		return 0;

	*p_len = len;
	if (info.version == 3)
		*p_type = ISO_IEC_11172_AUDIO;
	else
		*p_type = ISO_IEC_13818_3_AUDIO;
	*p_pts = samples * 45000 / samrate;

	if (info.version == 3) {//mpeg1
		if (info.channel == 3)
			p += 17;
		else
			p += 32;
	} else {//mpeg2
		if (info.channel == 3)
			p += 9;
		else
			p += 17;
	}

	if (a2t->bitrate == 0) {
		if (info.version == 0 || info.version == 2)
			a2t->bitrate = g_mpg_bitrate_sss[info.bitrate][1][info.layer] * 1000;
		else
			a2t->bitrate = g_mpg_bitrate_sss[info.bitrate][0][info.layer] * 1000;
	}
	if (memcmp(p, "Info", 4) == 0 || memcmp(p, "Xing", 4) == 0) {
		PRINTF("LAME VBR! flag = 0x%02x, framesize = %d\n", (uint32_t)p[7], len);
		uint32_t frames =	(((uint32_t)p[ 8]) << 24) + 
						(((uint32_t)p[ 9]) << 16) + 
						(((uint32_t)p[10]) << 8) + 
						 ((uint32_t)p[11]);
		a2t->duration = frames * samples / samrate;
		return 2;
	}

	return 1;
Err:
	return -1;
}

static unsigned int g_aac_samrates[16] = 
{	96000,	88200,	64000,	48000,
	44100,	32000,	24000,	22050,
	16000,	12000,	11025,	8000, 
	0,		0,		0,		0
};

static int ind_frame_aac(struct ts_audio *a2t, unsigned char *buffer, int length, int *p_len, unsigned int *p_type, unsigned int *p_pts)
{
	unsigned char *p;
	int i, len;
	unsigned int samrate;

	p = buffer;

	if (length <= 7)
		return 0;
	//sampling_frequency_index
	i = (p[2] >> 2) & 0xf;
	samrate = g_aac_samrates[i];
	if (samrate == 0)
		ERR_OUT("sampling_frequency_index = %d\n", i);

	len = ((unsigned int)(p[3] & 0x3) << 11) | ((unsigned int)p[4] << 3) | ((unsigned int)p[5] >> 5);
	if (len > length)
		return 0;
	*p_len = len;
	*p_type = ISO_IEC_13818_7_AUDIO;
	*p_pts = 1024 * 45000 / samrate;

	return 1;
Err:
	return -1;
}

static int ind_frame_lame(u_char *buf)
{
	int len = 8;
	if (buf[7] & 0x01)
		len += 4;
	if (buf[7] & 0x02)
		len += 4;
	if (buf[7] & 0x04)
		len += 100;
	if (buf[7] & 0x08)
		len += 4;
	if (memcmp(buf + len, "LAME", 4) == 0) {
		if (buf[len + 4] < '3' || buf[len + 6] < '9')
			len += 20;
		else
			len += 36;
	}

	return len;
}

static int ind_frame_parse(struct ts_audio *a2t, int *p_off, int *p_len, unsigned int *p_type, unsigned int *p_pts)
{
	int ret;
	unsigned char *p;
	int l, len, skip;

	p = (unsigned char *)a2t->es_buf + a2t->es_off;
	l = a2t->es_len;

	len = 0;
	*p_off = 0;
	*p_len = 0;
	*p_pts = 0;
	*p_type = ISO_IEC_11172_AUDIO;

	if (l <= 256)
		return 0;

	skip = 0;
	while(p[0] == 0 && l > 0) {
		skip ++;
		p ++;
		l --;
	}
	if (skip > 0) {
		WARN_PRN("skip = %d\n", skip);
		*p_off = skip;
		return 0;
	}

	if (memcmp(p, "Info", 4) == 0 || memcmp(p, "Xing", 4) == 0) {
		skip = ind_frame_lame(p);
		WARN_PRN("Skip LAME frame %d\n", skip);
		*p_off = skip;
		return 0;
	}

	if (p[0] == 'I' && p[1] == 'D' && p[2] == '3') {//mp3 ID3v2
		int size;

		size =  (((unsigned int)(p[6]&0x7f)) << 21) + 
				(((unsigned int)(p[7]&0x7f)) << 14) + 
				(((unsigned int)(p[8]&0x7f)) << 7) + 
				 ((unsigned int)(p[9]&0x7f));
		if (p[5] & 0x10)
			size += 20;
		else
			size += 10;

		*p_off = size;
		PRINTF("ID3v2 = %d\n", size);
		return 0;
	}
	if (p[0] == 'T' && p[1] == 'A' && p[2] == 'G') {//mp3 ID3v1
		*p_off = 128;
		PRINTF("ID3v1 = 128\n");
		return 0;
	}

	skip = 0;
	while(l > 0 && (p[0] != 0xff || (p[1] & 0xe0) != 0xe0)) {
Skip:
		skip ++;
		p ++;
		l --;
	}
	if (skip > 0) {
		WARN_PRN("skip = %d\n", skip);
		*p_off = skip;
		return 0;
	}

	if ((p[1] & 0x06) == 0)
		ret = ind_frame_aac(a2t, p, l, &len, p_type, p_pts);
	else
		ret = ind_frame_mpg(a2t, p, l, &len, p_type, p_pts);
	if (ret < 0) {
		if ((p[1] & 0x06) == 0)
			ERR_PRN("ind_a2t_frame_aac\n");
		else
			ERR_PRN("ind_a2t_frame_mpg\n");
		goto Skip;//重新同步
	}
	if (ret == 0)
		return 0;
	if (ret == 2) {
		*p_off = len;
		return 0;
	}
	if (len != l && len + 2 > l)
		return 0;
	if (p[len + 0] != 0xff || (p[len + 1] & 0xe0) != 0xe0) {
		WARN_PRN("not align! sync = %02x%02x, len = %d, l = %d\n", (uint32_t)p[len + 0], (uint32_t)p[len + 1], len, l);
		goto Skip;
	}
	*p_len = len;
	*p_off = a2t->es_len - l;

	return 1;
}

struct ts_audio* ts_audio_create(void)
{
	struct ts_audio *a2t = NULL;

	a2t = (struct ts_audio *)IND_MALLOC(sizeof(struct ts_audio));
	if (a2t == NULL)
		ERR_OUT("malloc ind_a2t\n");
	ts_audio_reset(a2t);

	return a2t;
Err:
	if (a2t)
		IND_FREE(a2t);
	return NULL;
}

void ts_audio_delete(struct ts_audio* a2t)
{
	if (a2t)
		IND_FREE(a2t);
}

int ts_audio_reset(struct ts_audio *a2t)
{
	if (a2t == NULL)
		ERR_OUT("a2t is NULL\n");

	IND_MEMSET(a2t, 0, sizeof(struct ts_audio));
	a2t->pts = 60*1000*45;

	a2t->bitrate = 0;
	a2t->duration = 0;

	return 0;
Err:
	return -1;
}

int ts_audio_buf_clr(struct ts_audio *a2t)
{
	if (a2t == NULL)
		ERR_OUT("a2t is NULL\n");

	a2t->es_off = 0;
	a2t->es_len = 0;
	a2t->es_suff = 0;

	a2t->frame_length = 0;
	a2t->frame_remain = 0;
	a2t->frame_first = 0;

	return 0;
Err:
	return -1;
}

int ts_audio_space(struct ts_audio *a2t)
{
	int off, suff, space = 0;

	if (a2t == NULL)
		ERR_OUT("a2t is NULL\n");
	if (a2t->frame_remain)
		ERR_OUT("frame_remain = %d\n", a2t->frame_remain);

	off = a2t->es_off + a2t->es_len;
	if (a2t->es_off > ES_FRAME_SIZE) {
		if (a2t->es_suff < ES_FRAME_SIZE)
			suff = a2t->es_off - ES_FRAME_SIZE;
		else
			suff = a2t->es_off - a2t->es_suff;
	} else {
		suff = 0;
	}

	if (a2t->es_off > ES_FRAME_SIZE && off >= ES_BUFFER_SIZE) {
		space = suff;
	} else {
		space = ES_BUFFER_SIZE - off;
		if (space < suff)
			space = suff;
	}

Err:
	return space;
}

int ts_audio_buf_get(struct ts_audio *a2t, char **pbuf, int *plen)
{
	int len;
	char *buf;

	if (a2t == NULL)
		ERR_OUT("a2t is NULL\n");
	if (a2t->frame_remain) {
		buf = NULL;
		len = 0;
	} else {
		int off = a2t->es_off + a2t->es_len;
		if (a2t->es_off > ES_FRAME_SIZE && off >= ES_BUFFER_SIZE) {
			if (a2t->es_suff < ES_FRAME_SIZE)
				a2t->es_suff = ES_FRAME_SIZE;
			buf = a2t->es_buf + a2t->es_suff;
			len = a2t->es_off - a2t->es_suff;
		} else {
			buf = a2t->es_buf + off;
			len = ES_BUFFER_SIZE - off;
		}
	}
	if (pbuf)
		*pbuf = buf;
	if (plen)
		*plen = len;

	return 0;
Err:
	return -1;
}

int ts_audio_buf_put(struct ts_audio *a2t, int len)
{
	int space;

	if (a2t == NULL)
		ERR_OUT("a2t is NULL\n");
	if (a2t->frame_remain)
		ERR_OUT("es_off = %d\n", a2t->es_off);

	if (a2t->es_suff >= ES_FRAME_SIZE)
		space = a2t->es_off - a2t->es_suff;
	else
		space = ES_BUFFER_SIZE - a2t->es_off - a2t->es_len;
	if (len > space)
		ERR_OUT("len = %d, es_suff = %d, es_off = %d, es_len = %d\n", len, a2t->es_suff, a2t->es_off, a2t->es_len);

	if (a2t->es_suff >= ES_FRAME_SIZE)
		a2t->es_suff += len;
	else
		a2t->es_len += len;

	return 0;
Err:
	return -1;
}

int ts_audio_write(struct ts_audio *a2t, char *buf, int len)
{
	int bytes;
	char *buffer;

	if (a2t == NULL)
		ERR_OUT("a2t is NULL\n");

	bytes = 0;
	ts_audio_buf_get(a2t, &buffer, &bytes);
	if (bytes > 0) {
		if (bytes > len)
			bytes = len;
		IND_MEMCPY(buffer, buf, bytes);
		ts_audio_buf_put(a2t, bytes);
	}

	return bytes;
Err:
	return -1;
}

int ts_audio_read(struct ts_audio *a2t, char *buf, int size)
{
	int ret, es_off, es_len, ret_len;

	if (a2t == NULL)
		ERR_OUT("a2t is NULL\n");
	if (size < 188*2)
		ERR_OUT("size = %d\n", size);

	if (a2t->frame_remain <= 0) {
		unsigned int type;
		unsigned int pts;

Skip:
		if (a2t->es_skip > 0) {
			if (a2t->es_skip >= a2t->es_len) {
				a2t->es_skip -= a2t->es_len;
				if (a2t->es_suff >= ES_FRAME_SIZE) {
					a2t->es_off = ES_FRAME_SIZE;
					a2t->es_len += a2t->es_suff - ES_FRAME_SIZE;
					a2t->es_suff = 0;
				} else {
					a2t->es_off = 0;
					a2t->es_len = 0;
				}
			} else {
				a2t->es_off += a2t->es_skip;
				a2t->es_len -= a2t->es_skip;
				a2t->es_skip = 0;
			}
		}

		if (a2t->es_len <= 0)
			return 0;

		ret = ind_frame_parse(a2t, &es_off, &es_len, &type, &pts);
		if (ret == -1)
			ERR_OUT("paser_frame\n");
		if (ret == 0) {
			if (es_off > 0) {
				a2t->es_skip = es_off;
				goto Skip;
			} else if (a2t->es_off >= ES_BUFFER_SIZE - ES_FRAME_SIZE) {
				if (a2t->es_suff >= ES_FRAME_SIZE) {
					es_off = ES_FRAME_SIZE - a2t->es_len;
					IND_MEMCPY(a2t->es_buf + es_off, a2t->es_buf + a2t->es_off, a2t->es_len);
					a2t->es_off = es_off;
					a2t->es_len += a2t->es_suff - ES_FRAME_SIZE;
					a2t->es_suff = 0;
				} else {
					IND_MEMCPY(a2t->es_buf, a2t->es_buf + a2t->es_off, a2t->es_len);
					a2t->es_off = 0;
				}
			}
			return 0;
		}
		a2t->es_len -= es_off;
		a2t->es_off += es_off;
		if (a2t->es_type && type != a2t->es_type)
			ERR_OUT("type = 0x%02x, type = 0x%02x\n", type, a2t->es_type);
		if (es_len == 0)
			return 0;
		a2t->es_type = type;
		a2t->frame_length = es_len;
		a2t->frame_remain = es_len;
		a2t->frame_first = 1;

		a2t->pts += pts;
		if (a2t->pts > 0xefffffff) {
			a2t->pts = 0;
			a2t->psi_pts = 0;
		}
	}

	ret_len = 0;

	if (a2t->psi_pts < a2t->pts) {
		ind_ts_pat(buf, a2t->pat_counter, TS_PROG_NUM, TS_PMT_PID);
		a2t->pat_counter ++;
		buf += 188;
		size -= 188;
		ret_len += 188;
		ind_ts_pmt_e1(buf, a2t->es_type, a2t->pmt_counter);
		a2t->pmt_counter ++;
		buf += 188;
		size -= 188;
		ret_len += 188;
		a2t->psi_pts = a2t->pts + TS_PSI_SECOND;
	}

	if (size >= 188 && a2t->frame_first == 1) {
		ind_ts_header(buf, TS_AUDIO_PID, 1, a2t->es_counter);
		a2t->pmt_counter ++;
		if (a2t->psi_pts == a2t->pts + TS_PSI_SECOND)
			ind_ts_adaptation(buf, 188 - 4 - 14, a2t->pts - 300*45);
		else
			ind_ts_adaptation(buf, 188 - 4 - 14, 0);
		ind_ts_pes(buf + 188 - 14, 0xC0, 8 + a2t->frame_length, a2t->pts);
		buf += 188;
		size -= 188;
		ret_len += 188;
		a2t->frame_first = 0;
	}

	while (size >= 188 && a2t->frame_remain > 0) {
		ind_ts_header(buf, TS_AUDIO_PID, 0, a2t->es_counter);
		a2t->es_counter ++;

		if (a2t->frame_remain < 184) {
			ind_ts_adaptation(buf, 184 - a2t->frame_remain, 0);
			es_len = a2t->frame_remain;
			es_off = 188 - a2t->frame_remain;
		} else {
			es_len = 184;
			es_off = 4;
		}

		IND_MEMCPY(buf + es_off, a2t->es_buf + a2t->es_off, es_len);
		a2t->es_len -= es_len;
		a2t->es_off += es_len;
		a2t->frame_remain -= es_len;

		buf += 188;
		size -= 188;
		ret_len += 188;
	}

	return ret_len;
Err:
	return -1;
}

int ts_audio_frame(struct ts_audio *a2t, char *buf, int size)
{
	int ret, es_off, es_len;

	if (a2t == NULL)
		ERR_OUT("a2t is NULL\n");
	if (buf == NULL || size < 0)
		ERR_OUT("buf = %p, size = %d\n", buf, size);

	if (a2t->frame_remain <= 0) {
		unsigned int type;
		unsigned int pts;

Skip:
		if (a2t->es_skip > 0) {
			if (a2t->es_skip >= a2t->es_len) {
				a2t->es_skip -= a2t->es_len;
				if (a2t->es_suff >= ES_FRAME_SIZE) {
					a2t->es_off = ES_FRAME_SIZE;
					a2t->es_len += a2t->es_suff - ES_FRAME_SIZE;
					a2t->es_suff = 0;
				} else {
					a2t->es_off = 0;
					a2t->es_len = 0;
				}
			} else {
				a2t->es_off += a2t->es_skip;
				a2t->es_len -= a2t->es_skip;
				a2t->es_skip = 0;
			}
		}

		if (a2t->es_len <= 0) {
			a2t->es_off = 0;
			return 0;
		}

		ret = ind_frame_parse(a2t, &es_off, &es_len, &type, &pts);
		if (ret == -1)
			ERR_OUT("paser_frame\n");
		if (ret == 0) {
			if (es_off > 0) {
				a2t->es_skip = es_off;
				goto Skip;
			} else if (a2t->es_off >= ES_BUFFER_SIZE - ES_FRAME_SIZE) {
				if (a2t->es_suff >= ES_FRAME_SIZE) {
					es_off = ES_FRAME_SIZE - a2t->es_len;
					IND_MEMCPY(a2t->es_buf + es_off, a2t->es_buf + a2t->es_off, a2t->es_len);
					a2t->es_off = es_off;
					a2t->es_len += a2t->es_suff - ES_FRAME_SIZE;
					a2t->es_suff = 0;
				} else {
					IND_MEMCPY(a2t->es_buf, a2t->es_buf + a2t->es_off, a2t->es_len);
					a2t->es_off = 0;
				}
			}
			return 0;
		}
		a2t->es_len -= es_off;
		a2t->es_off += es_off;
		if (a2t->es_type && type != a2t->es_type) {
			ERR_PRN("type = 0x%02x, type = 0x%02x\n", type, a2t->es_type);
			a2t->es_skip = es_len;
			goto Skip;
		}
		if (es_len == 0)
			return 0;
		a2t->es_type = type;
		a2t->frame_length = es_len;
		a2t->frame_remain = es_len;
	}

	ret = a2t->frame_remain;
	if (ret > size)
		ERR_OUT("size = %d, frame = %d\n", size, ret);

	IND_MEMCPY(buf, a2t->es_buf + a2t->es_off, ret);
	a2t->es_len -= ret;
	a2t->es_off += ret;
	a2t->frame_remain -= ret;

	return ret;
Err:
	return -1;
}

int ts_audio_second(struct ts_audio *a2t, int size)
{
	int duration;

	if (a2t == NULL)
		ERR_OUT("a2t is NULL\n");

	if (a2t->duration)
		duration = a2t->duration;
	else if (a2t->bitrate)
		duration = size * 8 / a2t->bitrate;
	else
		duration = 0;

	return duration;
Err:
	return 0;
}

#define TS_READ_SIZE		4096
#define TS_READ_SKIP		1024

typedef struct {
	void*		handle;
	ts_read_f	readcall;

	int len;
	int off;
	char* buf;
} TsBuf;

static TsBuf* ts_read_create(void* handle, ts_read_f readcall)
{
	TsBuf* tsbuf = NULL;

	if (handle == NULL || readcall == NULL)
		ERR_OUT("handle = %p, readcall = %p\n", handle, readcall);
	tsbuf = (TsBuf*)IND_MALLOC(sizeof(TsBuf));
	if (tsbuf == NULL)
		ERR_OUT("malloc TsBuf\n");
	IND_MEMSET(tsbuf, 0, sizeof(TsBuf));
	tsbuf->handle = handle;
	tsbuf->readcall = readcall;

	tsbuf->buf = (char *)IND_MALLOC(TS_READ_SIZE);
	if (tsbuf->buf == NULL)
		ERR_OUT("malloc %d\n", TS_READ_SIZE);

	return tsbuf;
Err:
	if (tsbuf)
		IND_FREE(tsbuf);
	return NULL;
}

static void ts_read_destroy(TsBuf* tsbuf)
{
	if (tsbuf == NULL)
		return;
	if (tsbuf->buf)
		IND_FREE(tsbuf->buf);
	IND_FREE(tsbuf);
}

static int ts_read_peek(TsBuf* tsbuf, int len)
{
	int l, off;

	if (len > TS_READ_SKIP)
		ERR_OUT("len = %d\n", len);

	while (tsbuf->len < len) {
		if (tsbuf->off > TS_READ_SIZE - TS_READ_SKIP) {
			if (tsbuf->len > 0)
				IND_MEMCPY(tsbuf->buf, tsbuf->buf + tsbuf->off, tsbuf->len);
			tsbuf->off = 0;
		}
		off = tsbuf->off + tsbuf->len;
		l = tsbuf->readcall(tsbuf->handle, tsbuf->buf + off, TS_READ_SIZE - off);
		if (l <= 0)
			ERR_OUT("readcall = %d\n", l);
		tsbuf->len += l;
	}

	return 0;
Err:
	return -1;
}

static int ts_read_skip(TsBuf* tsbuf, int len)
{
	int l;

	if (len <= 0)
		return 0;

	if (len < tsbuf->len) {
		tsbuf->len -= len;
		tsbuf->off += len;
		return 0;
	}
	len -= tsbuf->len;
	tsbuf->len = 0;
	tsbuf->off = 0;
	if (len <= 0)
		return 0;

	while (len > 0) {
		l = tsbuf->readcall(tsbuf->handle, tsbuf->buf, TS_READ_SIZE);
		if (l <= 0)
			ERR_OUT("readcall = %d\n", l);
		if (len <= l) {
			tsbuf->len = l - len;
			tsbuf->off = len;
			return 0;
		}
		len -= l;
	}

	return 0;
Err:
	return -1;
}

static void ts_read_info(TsBuf* tsbuf, u_char** pbuf, int* plen)
{
	*pbuf =(u_char*)tsbuf->buf + tsbuf->off;
	*plen = tsbuf->len;
}

int ts_audio_duration(void *handle, ts_read_f readcall, int filesize)
{
	u_char *buf;
	int l, len, length = 0;

	uint32_t bitrate, samrate;
	uint32_t samples;//msec
	u_char *mark;

	struct ind_mpg_info info;
	TsBuf* tsbuf = NULL;

	if (filesize < TS_READ_SIZE)
		ERR_OUT("filesize = %d\n", filesize);

	memset(&info, 0, sizeof(struct ind_mpg_info));

	tsbuf = ts_read_create(handle, readcall);
	if (tsbuf == NULL)
		ERR_OUT("ts_read_create\n");

Sync:
	if (ts_read_peek(tsbuf, TS_READ_SKIP))
		ERR_OUT("ts_read_peek %d\n", TS_READ_SKIP);

	ts_read_info(tsbuf, &buf, &len);

	for (l = 0; buf[0] == 0x00 && l < len; buf ++, l ++) ; //略去开头的零填充
	if (l > 0) {
		//PRINTF("SKIP %d\n", l);
		if (ts_read_skip(tsbuf, l))
			ERR_OUT("ts_read_skip %d\n", l);
		goto Sync;
	}

	if (buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3') {
		uint32_t id3v2size;

		id3v2size = (((uint32_t)(buf[6]&0x7f)) << 21) + 
					(((uint32_t)(buf[7]&0x7f)) << 14) + 
					(((uint32_t)(buf[8]&0x7f)) << 7) + 
					((uint32_t)(buf[9]&0x7f)) + ((buf[5]&0x10)?20:10);

		//PRINTF("ID3v2 %d\n", id3v2size);
		if (ts_read_skip(tsbuf, (int)id3v2size))
			ERR_OUT("ts_read_skip %d\n", id3v2size);
		goto Sync;
	}
	if (memcmp(buf, "Info", 4) == 0 || memcmp(buf, "Xing", 4) == 0) {
		//PRINTF("LAME VBR\n");
		len = ind_frame_lame(buf);
		//PRINTF("SKIP %d\n", len);
		if (ts_read_skip(tsbuf, len))
			ERR_OUT("ts_read_skip %d\n", len);
		goto Sync;
	}

	for (l = 0; (buf[0] != 0xff || (buf[1] & 0xe0) != 0xe0) && l < len - 1; buf ++, l ++) ;
	if (l > 0) {
		//PRINTF("SKIP %d\n", l);
		if (ts_read_skip(tsbuf, l))
			ERR_OUT("ts_read_skip %d\n", l);
		goto Sync;
	}
	if (buf[0] != 0xff || (buf[1] & 0xe0) != 0xe0)
		ERR_OUT("Frame sync error %02x%02x\n", (uint32_t)buf[0], (uint32_t)buf[1]);

	info.version = (buf[1] & 0x18) >> 3;
	info.layer = (buf[1] & 0x06) >> 1;
	info.protection = buf[1] & 0x01;
	info.bitrate = (buf[2] & 0xf0) >> 4;
	info.samrate = (buf[2] & 0x0c) >> 2;
	info.padding = (buf[2] & 0x02) >> 1;
	info.channel = (buf[3] & 0xc0) >> 6;

	if (info.layer == 0)
		ERR_OUT("layer = %d\n", info.version);
	info.layer = 3 - info.layer;

	if (info.version == 1)
		ERR_OUT("version = %d\n", info.version);

	if (info.version == 0 || info.version == 2) {
		bitrate = g_mpg_bitrate_sss[info.bitrate][1][info.layer] * 1000;
	} else {
		bitrate = g_mpg_bitrate_sss[info.bitrate][0][info.layer] * 1000;
	}
	if (bitrate == 0)
		ERR_OUT("bitrate = %d\n", bitrate);

	if (info.version == 0) {
		samrate = g_mpg_samrate_ss[info.samrate][2];
		samples = g_mpg_samples_ss[info.layer][2];
	} else if (info.version == 2) {
		samrate = g_mpg_samrate_ss[info.samrate][1];
		samples = g_mpg_samples_ss[info.layer][1];
	} else {
		samrate = g_mpg_samrate_ss[info.samrate][0];
		samples = g_mpg_samples_ss[info.layer][0];
	}
	if (samrate == 0)
		ERR_OUT("samrate = %d\n", samrate);

	if (info.protection)
		info.protection = 0;
	else
		info.protection = 1;
	
	if (info.version == 3) {//mpeg1
		if (info.channel == 3)
			mark = buf + 21;
		else
			mark = buf + 36;
	} else {//mpeg2
		if (info.channel == 3)
			mark = buf + 13;
		else
			mark = buf + 21;
	}

	if (memcmp(buf, "Info", 4) == 0 || memcmp(mark, "Xing", 4) == 0) {
		uint32_t frames;

		//PRINTF("LAME VBR\n");
		frames = (((uint32_t)mark[8]) << 24) + 
					(((uint32_t)mark[9]) << 16) + 
					(((uint32_t)mark[10]) << 8) + 
					((uint32_t)mark[11]);
		length = frames * samples / samrate;
		//PRINTF("frames=%d, duration=%d\n", frames, duration);
	} else {
		//PRINTF("CBR\n");
		length = (uint32_t)(filesize * 8 / bitrate);
		//PRINTF("duration = %d\n", duration);
	}

Err:
	if (tsbuf)
		ts_read_destroy(tsbuf);
	return length;
}

#ifdef __WIN32__

#define	BUFFER_SIZE			1316

#define FILENAME_MP3		"C:\\aac.aac"
#define FILENAME_TS			"C:\\1.ts"

static unsigned char g_buf[TS_PAT_LENGTH] = {0x00, 0x00, 0xb0, 0x0d, 0x5a, 0x06, 0xf5, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x42, 0x00, 0x00, 0x00, 0x00};
//0x81, 0x59, 0x59, 0x8e
int main(int argc, char* argv[])
{
	FILE *mp3fp = NULL, *tsfp = NULL;
	struct ts_audio *a2t = NULL;
	char *buf;
	int len;
	char buffer[BUFFER_SIZE];

	PRINTF("open file %s\n", FILENAME_MP3);
	mp3fp = fopen(FILENAME_MP3, "rb");
	if (mp3fp == NULL)
		ERR_OUT("Cannot open file %s\n", FILENAME_MP3);

	PRINTF("open file %s\n", FILENAME_TS);
	tsfp = fopen(FILENAME_TS, "wb");
	if (tsfp == NULL)
		ERR_OUT("Cannot open file %s\n", FILENAME_TS);

	a2t = ts_audio_create( );
	if (a2t == NULL)
		ERR_OUT("ind_a2t_create\n");
	ts_audio_reset(a2t);

	for (;;) {
		len = ts_audio_read(a2t, buffer, BUFFER_SIZE);
		if (len < 0)
			ERR_OUT("ind_a2t_read\n");
		if (len > 0) {
			fwrite(buffer, 1, len, tsfp);
			continue;
		}
		if (ts_audio_buf_get(a2t, &buf, &len))
			ERR_OUT("ind_a2t_buf_get\n");
		len = fread(buf, 1, len, mp3fp);
		if (len <= 0)
			break;
		if (ts_audio_buf_put(a2t, len))
			ERR_OUT("ind_a2t_buf_put\n");
	}
	ts_audio_delete(a2t);
	fclose(mp3fp);
	fclose(tsfp);

	return 0;
Err:
	if (a2t)
		ind_a2t_delete(a2t);
	if (mp3fp != NULL)
		fclose(mp3fp);
	if (tsfp != NULL)
		fclose(tsfp);
	return -1;
}
#endif//__WIN32__

