
#ifndef __MIX_H__
#define __MIX_H__

#include "../stream.h"

int stream_mix_pcm_space(int idx, uint32_t magic);
int stream_mix_pcm_push(int idx, uint32_t magic, char* buf, int len);
int stream_mix_mp3_space(int idx, uint32_t magic);
int stream_mix_mp3_push(int idx, uint32_t magic, char* buf, int len);

#endif//__MIX_H__
