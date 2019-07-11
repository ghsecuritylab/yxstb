
#ifndef __IND_RTP_H__
#define __IND_RTP_H__

#ifdef __cplusplus
extern "C" {
#endif

#define RTP_INVALID_SEQ			0x10000

/*
	int ind_rtp_parse(char* buffer, int length, unsigned int* seq, int* payload);
	去掉掉不通用的最后一个回参
 */
#define ind_rtp_parse	ind_rtp_parse_v1
int ind_rtp_parse(char* buffer, int length, unsigned int* seq);

int ind_rtp_parse_ex(char* buffer, int length, unsigned int* seq, unsigned int* ssrc);

#ifdef __cplusplus
}
#endif

#endif//__IND_RTP_H__

