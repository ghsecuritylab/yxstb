#include <unistd.h>
#include <stdlib.h>
#include <string.h>


#include "libzebra.h"
#include "HySDK.h"

typedef struct _hysdk_rendering_control_ {
	int type;
} HYSDK_RND_CTRL;


#define STATIC_CHAR		char
#define STATIC_INT		int
#define STATIC_SHORT	unsigned short


STATIC_CHAR *HySDK_RC_ListPresets(RenderingCS RCS)
{
	return NULL;
}

STATIC_INT HySDK_RC_SelectPreset(RenderingCS RCS, char *PresetName)
{
	return 0;
}

STATIC_SHORT HySDK_RC_GetBrightness(RenderingCS RCS)
{	
	int brightness;
	yhw_vout_getVideoBrightness(&brightness);
	return brightness;
}

STATIC_INT HySDK_RC_SetBrightness(RenderingCS RCS, unsigned short DesiredBrightness)
{
	return yhw_vout_setVideoBrightness(DesiredBrightness);
}

STATIC_SHORT HySDK_RC_GetContrast(RenderingCS RCS)
{
	int contrast;
	yhw_vout_getVideoContrast(&contrast);
	return contrast;
}

STATIC_INT HySDK_RC_SetContrast(RenderingCS RCS, unsigned short DesiredContrast)
{
	return yhw_vout_setVideoContrast(DesiredContrast);
}

STATIC_SHORT HySDK_RC_GetSharpness(RenderingCS RCS)
{
	int sharpness;
	yhw_vout_getVideoSharpness(&sharpness);
	return sharpness;
}

STATIC_INT HySDK_RC_SetSharpness(RenderingCS RCS, unsigned short DesiredSharpness)
{
	yhw_vout_setVideoSharpness(DesiredSharpness);
	return 0;
}

STATIC_SHORT HySDK_RC_GetRedVideoGain(RenderingCS RCS)
{
	return 0;
}

STATIC_INT HySDK_RC_SetRedVideoGain(RenderingCS RCS, unsigned short DesiredRedVideoGain)
{
	return 0;
}

STATIC_SHORT HySDK_RC_GetGreenVideoGain(RenderingCS RCS)
{
	return 0;
}

STATIC_INT HySDK_RC_SetGreenVideoGain(RenderingCS RCS, unsigned short DesiredGreenVideoGain)
{
	return 0;
}

STATIC_SHORT HySDK_RC_GetBlueVideoGain(RenderingCS RCS)
{
	return 0;
}

STATIC_INT HySDK_RC_SetBlueVideoGain(RenderingCS RCS, unsigned short DesiredBlueVideoGain)
{
	return 0;
}

STATIC_SHORT HySDK_RC_GetRedVideoBlackLevel(RenderingCS RCS)
{
	return 0;
}

STATIC_INT HySDK_RC_SetRedVideoBlackLevel(RenderingCS RCS, unsigned short DesiredRedVideoBlackLevel)
{
	return 0;
}

STATIC_SHORT HySDK_RC_GetGreenVideoBlackLevel(RenderingCS RCS)
{
	return 0;
}

STATIC_INT HySDK_RC_SetGreenVideoBlackLevel(RenderingCS RCS, unsigned short DesiredGreenVideoBlackLevel)
{
	return 0;
}

STATIC_SHORT HySDK_RC_GetBlueVideoBlackLevel(RenderingCS RCS)
{
	return 0;
}

STATIC_INT HySDK_RC_SetBlueVideoBlackLevel(RenderingCS RCS, unsigned short DesiredBlueVideoBlackLevel)
{
	return 0;
}

STATIC_SHORT HySDK_RC_GetColorTemperature(RenderingCS RCS)
{
	int colorTemp;
	yhw_vout_getVideoColorTemp(&colorTemp);
	return colorTemp;
}

STATIC_INT HySDK_RC_SetColorTemperature(RenderingCS RCS, unsigned short DesiredColorTemperature)
{
	return yhw_vout_setVideoColorTemp(DesiredColorTemperature);
}

STATIC_SHORT HySDK_RC_GetHorizontalKeystone(RenderingCS RCS)
{
	return 0;
}

STATIC_INT HySDK_RC_SetHorizontalKeystone(RenderingCS RCS, short DesiredHorizontalKeystone)
{
	return 0;
}

STATIC_SHORT HySDK_RC_GetVerticalKeystone(RenderingCS RCS)
{
	return 0;
}

STATIC_INT HySDK_RC_SetVerticalKeystone(RenderingCS RCS, short DesiredVerticalKeystone)
{
	return 0;
}

STATIC_SHORT HySDK_RC_GetLoudness(RenderingCS RCS, enum_CHANNEL_TYPE Channel)
{
	return 0;
}

STATIC_INT HySDK_RC_SetLoudness(RenderingCS RCS, enum_CHANNEL_TYPE Channel, short DesiredLoudness)
{
	return 0;
}

STATIC_SHORT HySDK_RC_GetMute(RenderingCS RCS, enum_CHANNEL_TYPE Channel)
{
	int mute = 0;
	yhw_aout_getMute(&mute);
	return mute;
}

STATIC_INT HySDK_RC_SetMute(RenderingCS RCS, enum_CHANNEL_TYPE Channel, int DesiredMute)
{
	return yhw_aout_setMute(DesiredMute);
}

STATIC_SHORT HySDK_RC_GetVolume(RenderingCS RCS, enum_CHANNEL_TYPE Channel)
{
	int volume = 0;
	yhw_aout_getVolume(&volume);
	return volume;
}

STATIC_INT HySDK_RC_SetVolume(RenderingCS RCS, enum_CHANNEL_TYPE Channel, unsigned short DesiredVolume)
{
	return yhw_aout_setVolume(DesiredVolume);
}

STATIC_SHORT HySDK_RC_GetVolumeDB(RenderingCS RCS, enum_CHANNEL_TYPE Channel)
{
	return 0;
}

STATIC_INT HySDK_RC_SetVolumeDB(RenderingCS RCS, enum_CHANNEL_TYPE Channel, short DesiredVolume)
{
	return 0;
}

STATIC_SHORT HySDK_RC_GetVolumeDBMin(RenderingCS RCS, enum_CHANNEL_TYPE Channel)
{
	return 0;
}

STATIC_SHORT HySDK_RC_GetVolumeDBMax(RenderingCS RCS, enum_CHANNEL_TYPE Channel)
{
	return 0;
}

STATIC_INT HySDK_RC_GetAudioChannel(RenderingCS RCS)
{
	int channel = 0;
	ymm_decoder_getAudioChannel(&channel);
	return channel;
}
STATIC_INT HySDK_RC_SetAudioChannel(RenderingCS RCS, int DesiredChannel)
{
	return ymm_decoder_setAudioChannel(DesiredChannel);
}

