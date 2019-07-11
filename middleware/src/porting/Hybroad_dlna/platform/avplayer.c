#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "mid_dlna_ex.h"
#include "hitTime.h"

#include "libzebra.h"
#include "HySDK.h"
#include "sys_basic_macro.h"

#include "ind_mem.h"

typedef struct __ZebraPCM {
	int size;
	char url[2048];
	int sampleRate;
	int bitWidth;
	int channels;
	int b_signed;
	int b_bigendian;
} ZebraPCM;

typedef struct _hysdk_av_player_ {
	t_PLAYER_EVENT	callback;
	void*				user;
	t_MEDIA_INFO		minfo;
	int					PlayState;
} HYSDK_AV_PLAYER;

#define IS_MEDIA_VIDEO(player)	( ((HYSDK_AV_PLAYER*)player)->minfo.majorType == 1 )
#define IS_MEDIA_AUDIO(player)	( ((HYSDK_AV_PLAYER*)player)->minfo.majorType == 2 )

#define INIT_RET(x)		int x = 0
#define RETURN_RET(x)	return x

#if 1
#include "mid_stream.h"



static void post_state(int index, STRM_STATE state, int rate, uint magic, int callarg)
{
	//PRINTF("----------------------------------------\nindex = %d, state = %d, rate = %d, magic = %d, callarg = %d\n", index, state, rate, magic, callarg);
}

static void post_msg(int index, STRM_MSG msg, int arg, uint magic, int callarg)
{
	//PRINTF("----------------------------------------\nindex = %d, msg = %d, arg = %d, magic = %d, callarg = %d\n", index, msg, arg, magic, callarg);
	switch (msg) {
	case STRM_MSG_OPEN_ERROR:
		yhw_board_systemEvent(STREAM_STATUS_FAILURE, 0);
		break;
	case STRM_MSG_STREAM_BEGIN:
		yhw_board_systemEvent(STREAM_BOUND_BEGIN, 0);
		break;
	case STRM_MSG_STREAM_END:
		yhw_board_systemEvent(STREAM_BOUND_END, 0);
		break;
    case STRM_MSG_RECV_TIMEOUT:
		yhw_board_systemEvent(STREAM_STATUS_FAILURE, 0);
		break;
    case STRM_MSG_RECV_RESUME:
		break;
	default:
		break;
	}
}


int HySDK_AVPlayer_Play(HYSDK_AVP player, char *url, t_MEDIA_INFO *mInfo)
{
	int ret = -1;
	HYSDK_AV_PLAYER* me = (HYSDK_AV_PLAYER*)player;
	if(mInfo)
		IND_MEMCPY( &(me->minfo), mInfo, sizeof(t_MEDIA_INFO));

	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, me->minfo.pvr_flag, url);

	me->PlayState = 1;
	if( IS_MEDIA_VIDEO(player) )
	{
		if(me->minfo.pvr_flag)
		{
			mid_stream_http_pvrkey(0, atoll(me->minfo.pvr_keys));
			mid_stream_set_call(0, post_state, post_msg, 0);
			ret = mid_stream_open_range(0, url, APP_TYPE_HTTP, 0, 0, me->minfo.xduration / 1000);
		}
		else
			ret = mid_stream_open(0, (char *)url, APP_TYPE_ZEBRA, 0);	
	}
	else if( IS_MEDIA_AUDIO(player) )
	{
		WAVEFORMATEX *ai = &(me->minfo.audioInfo);
		if( ai->wFormatTag )
		{
			ZebraPCM zpcm = {0};
			
			IND_STRNCPY(zpcm.url, url, sizeof(zpcm.url));
			zpcm.size		= sizeof(ZebraPCM);
			zpcm.sampleRate = ai->nSamplesPerSec; //sample_rate
			zpcm.bitWidth	= ai->wBitsPerSample;//bits_per_sample;
			zpcm.channels	= ai->nChannels;//channels;
			zpcm.b_signed	= -1;//b_signed;
			zpcm.b_bigendian = 1;//b_big_endian;
			
			ret = mid_stream_open(0, (char*)&zpcm, APP_TYPE_ZEBRA_PCM, 0);
		}
		else
			ret = mid_stream_open(0, (char *)url, APP_TYPE_ZEBRA, 0);	
	}
		
	HT_DBG_FUNC_END(me->minfo.majorType, "majorType = ");
	RETURN_RET(ret);
}


int HySDK_AVPlayer_Forward(HYSDK_AVP player, int speed)
{
	INIT_RET(ret);
	
	if( IS_MEDIA_VIDEO(player) )
		mid_stream_fast(0, speed);
	else
		mid_stream_fast(0, speed);

	RETURN_RET(ret);
}
int HySDK_AVPlayer_Reverse(HYSDK_AVP player, int speed)
{
	INIT_RET(ret);
	
	if( IS_MEDIA_VIDEO(player) )
		mid_stream_fast(0, -speed);
	else
		mid_stream_fast(0, -speed);
	
	RETURN_RET(ret);
}
int HySDK_AVPlayer_Slow(HYSDK_AVP player, int speed)
{
	INIT_RET(ret);
	
	RETURN_RET(ret);
}
int HySDK_STB_GetMacAddress(char *value)
{   
	//char value[8] = {0};
    if (value) {
        char ifname[URL_LEN] = { 0 };
        network_default_ifname(ifname, URL_LEN);
        const char* pmac = network_ifacemac_get(ifname, value, URL_LEN);
        if (pmac)
            sprintf(value, "%02x:%02x:%02x:%02x:%02x:%02x", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
    }
	return 0;
}

int HySDK_AVPlayer_Pause(HYSDK_AVP player)
{
	INIT_RET(ret);
	
#if 0
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, 0, 0);
	int code;
	char lang[64];

	code = 0;
	ret=HySDK_AudioTrack_GetCount(player,&code);
	HT_DBG_FUNC(code, "audio channel count = ");
	
	code =0;
	lang[0] = 0;
	HySDK_AudioTrack_GetLanViaIndex(player, 0, &code, lang, sizeof(lang));
	HT_DBG_FUNC(0, (char*)(&code));
	HT_DBG_FUNC(0, lang);

	code = 0;
	ret=HySDK_Subtitle_GetCount(player,&code);
	HT_DBG_FUNC(code, "subtitle channel count = ");
	
	code =0;
	lang[0] = 0;
	HySDK_Subtitle_GetLanViaIndex(player, 0, &code, lang, sizeof(lang));
	HT_DBG_FUNC(0, (char*)(&code));
	HT_DBG_FUNC(0, lang);
	
	HT_DBG_FUNC_END(0, lang);
#endif

	mid_stream_pause(0);
	
	RETURN_RET(ret);
}
int HySDK_AVPlayer_Resume(HYSDK_AVP player)
{
	INIT_RET(ret);
	
	mid_stream_resume(0);
	
	RETURN_RET(ret);
}
int HySDK_AVPlayer_Seek(HYSDK_AVP player, int seekMode, long long target)
{
	INIT_RET(ret);
	
	if(seekMode == 1)
		mid_stream_lseek(0,  target);
	else if(seekMode == 0)
	{
		mid_stream_seek(0, (int)target);
	}
	else
	{
		ret = -1;
	}
	
	RETURN_RET(ret);
}
int HySDK_AVPlayer_Stop(HYSDK_AVP player)
{
	INIT_RET(ret);
	
	HYSDK_AV_PLAYER* me = (HYSDK_AV_PLAYER*)player;
	int state = me->PlayState;
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, state, "me->PlayState = ");
	if( state )
	{
#if 1
		mid_stream_close(0, 0); 
#else
		mid_stream_close(0, 1); 
		ymm_decoder_stopBuffer();
		ymm_decoder_stopAudio();
		ymm_decoder_stopVideo();
		ymm_decoder_stopTeleText();
#endif
		me->PlayState = 0;
	}
	HT_DBG_FUNC_END(0, 0);
	
	RETURN_RET(ret);
}

int HySDK_AVPlayer_GetTotalTime(HYSDK_AVP player, int *total)
{
	INIT_RET(ret);
	*total = (int)mid_stream_get_totaltime(0);
	RETURN_RET(ret);
}
int HySDK_AVPlayer_GetCurrentTime(HYSDK_AVP player, int *now)
{
	INIT_RET(ret);
	*now = (int)mid_stream_get_currenttime(0);
	RETURN_RET(ret);
}
int HySDK_AVPlayer_GetTotalLength(HYSDK_AVP player, long long *total)
{
	INIT_RET(ret);
	*total = mid_stream_get_totalbyte(0);
	RETURN_RET(ret);
}
int HySDK_AVPlayer_GetCurrentLength(HYSDK_AVP player, long long *now)
{
	INIT_RET(ret);
	*now = mid_stream_get_currentbyte(0);
	RETURN_RET(ret);
}

PLAYER_STATE HySDK_AVPlayer_GetState(HYSDK_AVP player)
{
	return PLAYER_STATE_IDLE;
}
int HySDK_AVPlayer_GetMediaInfo(HYSDK_AVP player, t_MEDIA_INFO* minfo)
{
	int ret = -1;

	RETURN_RET(ret);
}
int HySDK_AVPlayer_GetPlaySpeed(HYSDK_AVP player,  char *buf, int size)
{
	int ret = -1;

	RETURN_RET(ret);
}


#else


int HySDK_AVPlayer_Play(HYSDK_AVP player, char *url, t_MEDIA_INFO *mInfo)
{
	HYSDK_AV_PLAYER* me = (HYSDK_AV_PLAYER*)player;
	if( mInfo )
		IND_MEMCPY( &(me->minfo), mInfo, sizeof(t_MEDIA_INFO));
	
	me->PlayState = 1;
	if( IS_MEDIA_VIDEO(player) )
	{
		ymm_stream_playerStart(0, (char *)url, 0);
	}
	else if( IS_MEDIA_AUDIO(player) )
	{
		WAVEFORMATEX *ai = &(me->minfo.audioInfo);
		
		if( ai->wFormatTag )
			ymm_audio_playPCMStream(0, url, 0, ai->nSamplesPerSec ,ai->wBitsPerSample, ai->nChannels, -1, 1);
		else
			ymm_stream_playerStart(0, (char *)url, 0);
	}
		
	return 0;
}


void HySDK_AVPlayer_Forward(HYSDK_AVP player, int speed)
{
	if( IS_MEDIA_VIDEO(player) )
		ymm_stream_playerSetTrickMode(0, YX_FAST_FORWARD, speed);
	else
	{
	}
}
void HySDK_AVPlayer_Reverse(HYSDK_AVP player, int speed)
{
	if( IS_MEDIA_VIDEO(player) )
		ymm_stream_playerSetTrickMode(0, YX_FAST_REW, speed);
	else
	{
	}	
}
void HySDK_AVPlayer_Slow(HYSDK_AVP player, int speed)
{
}
void HySDK_AVPlayer_Pause(HYSDK_AVP player)
{
	if( IS_MEDIA_VIDEO(player) )
		ymm_stream_playerSetTrickMode(0, YX_PAUSE, -1);
	else
		ymm_audio_setPausedPCMStream(0, 1); 
}
void HySDK_AVPlayer_Resume(HYSDK_AVP player)
{
	if( IS_MEDIA_VIDEO(player) )
		ymm_stream_playerSetTrickMode(0, YX_NORMAL_PLAY, -1);	
	else
		ymm_audio_setPausedPCMStream(0, 0); 
}
void HySDK_AVPlayer_Seek(HYSDK_AVP player, int seekMode, unsigned long long target)
{
	ymm_decoder_flush();
	ymm_stream_playerSetTrickMode(0, YX_NORMAL_PLAY, -1);
	if(seekMode == 1)
	{
		ymm_stream_playerSeekToOffset(0,  (long long)target);
	}
	else if(seekMode == 0)
	{
		int handle = (int)ymm_stream_handle_get(0);
		ymm_stream_seekToMs(handle, (int)(target));
	}
}
void HySDK_AVPlayer_Stop(HYSDK_AVP player)
{
	HYSDK_AV_PLAYER* me = (HYSDK_AV_PLAYER*)player;
	if( me->PlayState )
	{
		if( IS_MEDIA_VIDEO(player) )
			ymm_stream_playerStop(0);
		else
			ymm_audio_stopPCMStream(0); 
		me->PlayState = 0;
	}
}

unsigned int HySDK_AVPlayer_GetTotalTime(HYSDK_AVP player)
{
	unsigned int timeLen = 0;
	ymm_stream_playerGetTotalTime(0, &timeLen);
	timeLen = timeLen/1000;
	return timeLen;
}
unsigned int HySDK_AVPlayer_GetCurrentTime(HYSDK_AVP player)
{
	unsigned int timeLen = 0;
	ymm_stream_playerGetPlaytime(0, &timeLen);
	timeLen = timeLen/1000;
	return timeLen;
}
long long HySDK_AVPlayer_GetTotalLength(HYSDK_AVP player)
{
	long long count = 0;
	int handle = (int)ymm_stream_handle_get(0);
	ymm_stream_getLength(handle, &count);
	return count;
}
long long HySDK_AVPlayer_GetCurrentLength(HYSDK_AVP player)
{
	long long count = 0;
	int handle = (int)ymm_stream_handle_get(0);
	ymm_stream_getOffset(handle, &count);
	return count;
}

PLAYER_STATE HySDK_AVPlayer_GetState(HYSDK_AVP player)
{
	return PLAYER_STATE_IDLE;
}
t_MEDIA_INFO* HySDK_AVPlayer_GetMediaInfo(HYSDK_AVP player)
{
	return NULL;
}
void HySDK_AVPlayer_GetPlaySpeed(HYSDK_AVP player,  char *buf, int size)
{
}


#endif

int HySDK_Subtitle_GetCount(HYSDK_AVP player, int *count)
{
	INIT_RET(ret);
	ret = ymm_decoder_getNumOfSubtitle(count);
	RETURN_RET(ret);
}
int HySDK_Subtitle_GetPidViaIndex(HYSDK_AVP player, int index, int *pid)
{
	INIT_RET(ret);
	ret = ymm_decoder_getSubtitlePIDFromIndex(index, pid);
	RETURN_RET(ret);
}
int HySDK_Subtitle_GetLanViaIndex(HYSDK_AVP player, int index, int *lang_code, char*language, int language_len )
{
	INIT_RET(ret);
	char *langcode = NULL, lang[32];
	
	*lang_code = 0;
	language[0] = 0;
	
	ret = ymm_decoder_getSubtitleLangFromIndex(index, &langcode);
	if( langcode && langcode[0] )
	{
		IND_MEMCPY( lang_code, langcode , 3);
		
		mid_get_language_full_name_from_iso639(langcode, lang, sizeof(lang));
		if( language_len > strlen(lang) )
			IND_STRCPY(language, lang);
	}
	
	RETURN_RET(ret);
}
int HySDK_Subtitle_GetCurrentPID(HYSDK_AVP player, int *pid)
{
	INIT_RET(ret);
	unsigned short x =0;
	ret = ymm_decoder_getSubtitlePID(&x);
	*pid = x;
	RETURN_RET(ret);
}
int HySDK_Subtitle_SetCurrentPID(HYSDK_AVP player, int pid)
{
	INIT_RET(ret);
	ret = ymm_decoder_setSubtitlePID( (unsigned short)pid);
	RETURN_RET(ret);
}

int HySDK_AudioTrack_GetCount(HYSDK_AVP player, int *count)
{
	INIT_RET(ret);
	ret = ymm_decoder_getNumOfAudio(count);
	RETURN_RET(ret);
}	
int HySDK_AudioTrack_GetPidViaIndex(HYSDK_AVP player, int index, int *pid)
{
	INIT_RET(ret);
	ret = ymm_decoder_getAudioPIDFromIndex(index, pid);
	RETURN_RET(ret);
}
int HySDK_AudioTrack_GetLanViaIndex(HYSDK_AVP player, int index, int *lang_code, char*language, int language_len )
{
	INIT_RET(ret);
	char *langcode = NULL, lang[32];

	*lang_code = 0;
	language[0] = 0;
	
	ret = ymm_decoder_getAudioLanguageFromIndex(index, &langcode);
	if( langcode && langcode[0] )
	{
		IND_MEMCPY( lang_code, langcode , 3);
		
		mid_get_language_full_name_from_iso639(langcode, lang, sizeof(lang));
		if( language_len > strlen(lang) )
			IND_STRCPY(language, lang);
	}

	RETURN_RET(ret);
}
int HySDK_AudioTrack_GetCurrentPID(HYSDK_AVP player, int *pid)
{
	INIT_RET(ret);
	ret = ymm_decoder_getAudioPID(pid);
	RETURN_RET(ret);
}
int HySDK_AudioTrack_SetCurrentPID(HYSDK_AVP player, int pid)
{
	INIT_RET(ret);
	ret = ymm_decoder_setAudioPID( (unsigned short)pid);
	RETURN_RET(ret);
}

#include <semaphore.h>
static sem_t		s_avplayer_sem;
static int			s_avplayer_sem_init = 0;
static HYSDK_AVP	s_avplayer_handle = 0;

void HySDK_AVPlayer_Release(HYSDK_AVP player)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)player, "player = ");
	HYSDK_AV_PLAYER* me = (HYSDK_AV_PLAYER*)player;
	if( me )
	{
		HySDK_AVPlayer_Stop(player);
		
		sem_wait(&s_avplayer_sem);
#ifdef STBTYPE_QTEL
		IND_FREE( me );
#else
		IND_FREE(me);
#endif
		s_avplayer_handle = 0;
		sem_post(&s_avplayer_sem);
	}
	HT_DBG_FUNC_END(0, 0);
}

HYSDK_AVP HySDK_AVPlayer_Create(t_PLAYER_EVENT callBack, void *user)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)callBack, "callBack = ");
	if( s_avplayer_sem_init == 0 )
	{
		sem_init(&s_avplayer_sem,0,1);
		s_avplayer_sem_init = 1;
	}

#ifdef STBTYPE_QTEL
	HYSDK_AV_PLAYER* me = (HYSDK_AV_PLAYER*)IND_MALLOC(sizeof(HYSDK_AV_PLAYER));
#else
	HYSDK_AV_PLAYER* me = (HYSDK_AV_PLAYER*)IND_MALLOC(sizeof(HYSDK_AV_PLAYER));
#endif
	

	IND_MEMSET(me, 0, sizeof(HYSDK_AV_PLAYER));
	me->callback		= callBack;
	me->user			= user;

	s_avplayer_handle	= (int)me;
	
	HT_DBG_FUNC_END((int)user, "user = ");
	return (int)me;
}


#if 0
void HySDK_AVPlayer_Callback(int index, int eventType, int value, int arg)
{
	if( !s_avplayer_sem_init )
		return;
	
	sem_wait(&s_avplayer_sem);
	HYSDK_AV_PLAYER* me = (HYSDK_AV_PLAYER*)s_avplayer_handle;
	
	if(s_IsPlayingDLNAStream(me, index))
	{
		me->callback(eventType, value, arg, me->user);
	}
	sem_post(&s_avplayer_sem);
}
#else
#include "libzebra.h"
#include "mid_stream.h"
int s_IsPlayingDLNAStream(HYSDK_AV_PLAYER* me)
{
	if( me && me->callback && me->PlayState )
		return 1;
	else
		return 0;
}
void HYSDK_AVPlayer_GetEvent(unsigned int msgno, int type, int stat)
{
	if( !s_avplayer_sem_init )
		return;
	
	sem_wait(&s_avplayer_sem);
	HYSDK_AV_PLAYER* me = (HYSDK_AV_PLAYER*)s_avplayer_handle;
	if(s_IsPlayingDLNAStream(me))
	{
		HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, type, "type = ");
		HT_DBG_FUNC(s_avplayer_handle, "s_avplayer_handle = ");
		
		switch(msgno)
		{
			case STREAM_STATUS_PLAY:
				me->callback( 0, PLAYER_STATE_PLAY, 1, me->user);
				break;

			case STREAM_STATUS_STOP:
                me->PlayState=0;
				me->callback( 0, PLAYER_STATE_IDLE, 0, me->user);
				break;
				
			case STREAM_BOUND_BEGIN:
				me->callback( enum_MPLAYER_EVENT_TRICKTOHEAD, 0, 0, me->user);
				break;
				
			case STREAM_BOUND_END:
				me->callback( enum_MPLAYER_EVENT_TOEND, 0, 0, me->user);
				break;
				
			case STREAM_STATUS_FAILURE:
				me->callback( enum_MPLAYER_EVENT_DISCONNECT, 0, 0, me->user);
				break;
				
			case STREAM_STATUS_READ_ERROR:
				me->callback( enum_MPLAYER_EVENT_NO_DATA, 0, 0, me->user);
				break;

			case STREAM_STATUS_OP_UNSUPPORT:
				me->callback( enum_MPLAYER_EVENT_TRICKPLAY_UNSUPPORTED, 0, 0, me->user);
				break;
				
			case STREAM_UNSUPPORT_VIDEO:
				me->callback( enum_MPLAYER_EVENT_VIDEO_UNSUPPORTED, 0, 0, me->user);
				break;
			case STREAM_UNSUPPORT_AUDIO:
				me->callback( enum_MPLAYER_EVENT_AUDIO_UNSUPPORTED, 0, 0, me->user);
				break;
			case STREAM_UNSUPPORT_ALL:	
				me->callback( enum_MPLAYER_EVENT_BOTH_UNSUPPORTED, 0, 0, me->user);
				break;

			default:
				//me->callback( 1, msgno, stat, me->user);
				break;
		}
		
		HT_DBG_FUNC_END(msgno, "msgno = ");
	}
	sem_post(&s_avplayer_sem);
}

#endif


