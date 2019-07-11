#ifndef __HY_SDK_H__
#define __HY_SDK_H__

#include <linux/types.h>
#include "dlna_type.h"

/* Image Player interface */
typedef int HYSDK_IMGP;
HYSDK_IMGP HySDK_IPlayer_Create(void);
void HySDK_IPlayer_Release(HYSDK_IMGP player);
int HySDK_IPlayer_Play(HYSDK_IMGP player, char *path, int slideMode);
int HySDK_IPlayer_GetMediaInfo(HYSDK_IMGP player, int *w, int *h);
int HySDK_IPlayer_Zoom(HYSDK_IMGP player, int percent);
int HySDK_IPlayer_Move(HYSDK_IMGP player, int x, int y);
int HySDK_IPlayer_Rotate(HYSDK_IMGP player, int radian);
int HySDK_IPlayer_Stop(HYSDK_IMGP player);


/*AV player interface */
typedef int HYSDK_AVP;
HYSDK_AVP HySDK_AVPlayer_Create(t_PLAYER_EVENT callBack, void *user);
void HySDK_AVPlayer_Release(HYSDK_AVP player);
int HySDK_AVPlayer_Play(HYSDK_AVP player, char *url, t_MEDIA_INFO *mInfo);
int HySDK_AVPlayer_Stop(HYSDK_AVP player);
int HySDK_AVPlayer_Pause(HYSDK_AVP player);
int HySDK_AVPlayer_Resume(HYSDK_AVP player);
int HySDK_AVPlayer_Forward(HYSDK_AVP player, int speed);
int HySDK_AVPlayer_Reverse(HYSDK_AVP player, int speed);
int HySDK_AVPlayer_Slow(HYSDK_AVP player, int speed);
int HySDK_AVPlayer_Seek(HYSDK_AVP player, int seekMode, long long position);
int HySDK_STB_GetMacAddress(char *value);

int HySDK_AVPlayer_GetTotalTime(HYSDK_AVP player, int *total);
int HySDK_AVPlayer_GetCurrentTime(HYSDK_AVP player, int *now);
int HySDK_AVPlayer_GetTotalLength(HYSDK_AVP player, long long *total);
int HySDK_AVPlayer_GetCurrentLength(HYSDK_AVP player, long long *now);
PLAYER_STATE HySDK_AVPlayer_GetState(HYSDK_AVP player);
int HySDK_AVPlayer_GetMediaInfo(HYSDK_AVP player, t_MEDIA_INFO *minfo);
int HySDK_AVPlayer_GetPlaySpeed(HYSDK_AVP player,  char *buf, int size);

int HySDK_Subtitle_GetCount(HYSDK_AVP player, int *count);
int HySDK_Subtitle_GetPidViaIndex(HYSDK_AVP player, int index, int *pid);
int HySDK_Subtitle_GetLanViaIndex(HYSDK_AVP player, int index, int *lang_code, char*language, int language_len);
int HySDK_Subtitle_GetCurrentPID(HYSDK_AVP player, int *pid);
int HySDK_Subtitle_SetCurrentPID(HYSDK_AVP player, int pid);

int HySDK_AudioTrack_GetCount(HYSDK_AVP player, int *count);
int HySDK_AudioTrack_GetPidViaIndex(HYSDK_AVP player, int index, int *pid);
int HySDK_AudioTrack_GetLanViaIndex(HYSDK_AVP player, int index, int *lang_code, char*language, int language_len );
int HySDK_AudioTrack_GetCurrentPID(HYSDK_AVP player, int *pid);
int HySDK_AudioTrack_SetCurrentPID(HYSDK_AVP player, int pid);


/* rendering  interface */
typedef int RenderingCS;

#define EXTERN_CHAR		extern char
#define EXTERN_INT		extern int
#define EXTERN_SHORT	extern unsigned short

EXTERN_CHAR *HySDK_RC_ListPresets(RenderingCS RCS);
EXTERN_INT HySDK_RC_SelectPreset(RenderingCS RCS, char *PresetName);

EXTERN_SHORT HySDK_RC_GetBrightness(RenderingCS RCS);
EXTERN_INT HySDK_RC_SetBrightness(RenderingCS RCS, unsigned short DesiredBrightness);

EXTERN_SHORT HySDK_RC_GetContrast(RenderingCS RCS);
EXTERN_INT HySDK_RC_SetContrast(RenderingCS RCS, unsigned short DesiredContrast);

EXTERN_SHORT HySDK_RC_GetSharpness(RenderingCS RCS);
EXTERN_INT HySDK_RC_SetSharpness(RenderingCS RCS, unsigned short DesiredSharpness);

EXTERN_SHORT HySDK_RC_GetRedVideoGain(RenderingCS RCS);
EXTERN_INT HySDK_RC_SetRedVideoGain(RenderingCS RCS, unsigned short DesiredRedVideoGain);

EXTERN_SHORT HySDK_RC_GetGreenVideoGain(RenderingCS RCS);
EXTERN_INT HySDK_RC_SetGreenVideoGain(RenderingCS RCS, unsigned short DesiredGreenVideoGain);

EXTERN_SHORT HySDK_RC_GetBlueVideoGain(RenderingCS RCS);
EXTERN_INT HySDK_RC_SetBlueVideoGain(RenderingCS RCS, unsigned short DesiredBlueVideoGain);

EXTERN_SHORT HySDK_RC_GetRedVideoBlackLevel(RenderingCS RCS);
EXTERN_INT HySDK_RC_SetRedVideoBlackLevel(RenderingCS RCS, unsigned short DesiredRedVideoBlackLevel);

EXTERN_SHORT HySDK_RC_GetGreenVideoBlackLevel(RenderingCS RCS);
EXTERN_INT HySDK_RC_SetGreenVideoBlackLevel(RenderingCS RCS, unsigned short DesiredGreenVideoBlackLevel);

EXTERN_SHORT HySDK_RC_GetBlueVideoBlackLevel(RenderingCS RCS);
EXTERN_INT HySDK_RC_SetBlueVideoBlackLevel(RenderingCS RCS, unsigned short DesiredBlueVideoBlackLevel);

EXTERN_SHORT HySDK_RC_GetColorTemperature(RenderingCS RCS);
EXTERN_INT HySDK_RC_SetColorTemperature(RenderingCS RCS, unsigned short DesiredColorTemperature);

EXTERN_SHORT HySDK_RC_GetHorizontalKeystone(RenderingCS RCS);
EXTERN_INT HySDK_RC_SetHorizontalKeystone(RenderingCS RCS, short DesiredHorizontalKeystone);

EXTERN_SHORT HySDK_RC_GetVerticalKeystone(RenderingCS RCS);
EXTERN_INT HySDK_RC_SetVerticalKeystone(RenderingCS RCS, short DesiredVerticalKeystone);

EXTERN_SHORT HySDK_RC_GetLoudness(RenderingCS RCS, enum_CHANNEL_TYPE Channel);
EXTERN_INT HySDK_RC_SetLoudness(RenderingCS RCS, enum_CHANNEL_TYPE Channel, short DesiredLoudness);

EXTERN_SHORT HySDK_RC_GetMute(RenderingCS RCS, enum_CHANNEL_TYPE Channel);
EXTERN_INT HySDK_RC_SetMute(RenderingCS RCS, enum_CHANNEL_TYPE Channel, int DesiredMute);

EXTERN_SHORT HySDK_RC_GetVolume(RenderingCS RCS, enum_CHANNEL_TYPE Channel);
EXTERN_INT HySDK_RC_SetVolume(RenderingCS RCS, enum_CHANNEL_TYPE Channel, unsigned short DesiredVolume);

EXTERN_SHORT HySDK_RC_GetVolumeDB(RenderingCS RCS, enum_CHANNEL_TYPE Channel);
EXTERN_INT HySDK_RC_SetVolumeDB(RenderingCS RCS, enum_CHANNEL_TYPE Channel, short DesiredVolume);

EXTERN_SHORT HySDK_RC_GetVolumeDBMin(RenderingCS RCS, enum_CHANNEL_TYPE Channel);
EXTERN_SHORT HySDK_RC_GetVolumeDBMax(RenderingCS RCS, enum_CHANNEL_TYPE Channel);

EXTERN_INT HySDK_RC_GetAudioChannel(RenderingCS RCS);
EXTERN_INT HySDK_RC_SetAudioChannel(RenderingCS RCS, int DesiredChannel);


/* for PVR */
EXTERN_INT HySDK_PvrInit(int is_ipc);
EXTERN_INT HySDK_PvrGetInfo(const char *fullpath, C_UD_MI *minfo);
extern PVR_FS_HND HySDK_PvrOpen(const char *filename, int mode);
EXTERN_INT HySDK_PvrRead(PVR_FS_HND fileHnd, char *buf, size_t buflen);
EXTERN_INT HySDK_PvrWrite(PVR_FS_HND fileHnd, char *buf, size_t buflen);
EXTERN_INT HySDK_PvrSeek(PVR_FS_HND fileHnd, off_t offset, int origin);
EXTERN_INT HySDK_PvrClose(PVR_FS_HND fileHnd);


#endif /* __HY_SDK_H__ */

