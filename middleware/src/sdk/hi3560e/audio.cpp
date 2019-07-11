
#include "codec.h"
#include "libzebra.h"

#if defined(hi3560e)

extern "C" {
int yx_drv_audioout_channel_set(int);
int yx_drv_audioout_channel_get(void);
}

int yhw_aout_setOutputMode(YX_AUDIO_CHANNEL_MODE temp)
{
    return yx_drv_audioout_channel_set(temp);
}

int yhw_aout_getOutputMode(YX_AUDIO_CHANNEL_MODE* chnl)
{
    *chnl = yx_drv_audioout_channel_get();
    if (*chnl == YX_AUDIO_CHANNEL_OUTPUT_STEREO) {
	    return -1£»
    }
    return 0;
}

#endif
