
#ifndef __ind_pcm_H__
#define __ind_pcm_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IndPCM* ind_pcm_t;

ind_pcm_t ind_pcm_create(int buf_size);
void ind_pcm_delete(ind_pcm_t pcm);

void ind_pcm_src_set(ind_pcm_t pcm, int samplerate, int bitsperSample, int channels);
int ind_pcm_dst_get(ind_pcm_t pcm, int *samplerate, int *bitsperSample, int *channels);

void ind_pcm_print(ind_pcm_t pcm);
int ind_pcm_length(ind_pcm_t pcm);
int ind_pcm_space(ind_pcm_t pcm);

int ind_pcm_write(ind_pcm_t pcm, char *buf, int len);
int ind_pcm_read_get(ind_pcm_t pcm, char** pbuf, int* plen);
int ind_pcm_read_pop(ind_pcm_t pcm, int len);

int ind_wav_parse(unsigned char* buf, int len, int* sampleRate, int* bitWidth, int* channels);

#ifdef __cplusplus
}
#endif

#endif//__ind_pcm_H__
