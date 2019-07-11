/**********************************************************
	Copyright (c) 2008-2009, Yuxing Software Corporation
	All Rights Reserved
	Confidential Property of Yuxing Softwate Corporation

	Revision History:

	Created: 2009-10-22 9:30:31 by liujianhua

 **********************************************************/

#include "mix.h"


int mid_stream_mix_space(int idx, uint32_t magic)
{
	int apptype, space = 0;

	apptype = mid_stream_get_apptype(idx);

	if (apptype == APP_TYPE_MIX_PCM)
		space = stream_mix_pcm_space(idx, magic);
#ifdef INCLUDE_MP3
	else if (apptype == APP_TYPE_MIX_MP3)
		space = stream_mix_mp3_space(idx, magic);
#endif
	else
		LOG_STRM_ERROUT("#%d apptype = %d\n", idx, apptype);

Err:
	return space;
}

int mid_stream_mix_push(int idx, uint32_t magic, char* buf, int len)
{
	int apptype, ret = -1;

	apptype = mid_stream_get_apptype(idx);

	if (apptype == APP_TYPE_MIX_PCM)
		ret = stream_mix_pcm_push(idx, magic, buf, len);
#ifdef INCLUDE_MP3
	else if (apptype == APP_TYPE_MIX_MP3)
		ret = stream_mix_mp3_push(idx, magic, buf, len);
#endif
	else
		LOG_STRM_ERROUT("#%d apptype = %d\n", idx, apptype);

Err:
	return ret;
}
