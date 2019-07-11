
#include "UltraPlayer.h"

#include "UltraPlayerClient.h"
#include "UltraPlayerAssertions.h"

#include "BrowserPlayerReporter.h"
#include "ProgramChannel.h"
#include "SystemManager.h"

#include "NativeHandler.h"

#include "AppSetting.h"
#include "SysSetting.h"

#include "app_tool.h"
#include "mid_sys.h"
#include "codec.h"
#include "libzebra.h"
#include "json/json.h"
#include "json/json_object.h"
#include "json/json_public.h"

#include <pthread.h>
extern "C" {
	int ygp_layer_getScreenSize(int *x, int *y);
}

namespace Hippo {

static uint32_t gMagicCount = 0;

bool UltraPlayer::mUIEnabled = false;
uint32_t UltraPlayer::mUIFlags = 0;
uint32_t UltraPlayer::mUIFlagsForcedMask = 0;
uint32_t UltraPlayer::mUIFlagsForcedValue = 0;

PlayStateWidget *UltraPlayer::mPlayState = NULL;
ChannelNOWidget *UltraPlayer::mChannelNO = NULL;
ProgressBarWidget *UltraPlayer::mProgressBar = NULL;
AudioMuteWidget *UltraPlayer::mAudioMute = NULL;
AudioTrackWidget *UltraPlayer::mAudioTrack = NULL;
AudioVolumeWidget *UltraPlayer::mAudioVolume = NULL;
DolbyWidget *UltraPlayer::mDolbyIcon = NULL;
DolbyDownmixWidget *UltraPlayer::mDolbyDownmixIcon = NULL;
UltraPlayerStatistic UltraPlayer::s_statistic;

int UltraPlayer::mVideoClearFlag = 0;

UltraPlayer::UltraPlayer(UltraPlayerClient *client, BrowserPlayerReporter *pReporter, Program *pProgram)
	: mClient(client)
	, mReporter(pReporter)
	, mProgram(pProgram)
	, mIndex(0)
	, mIsFake(0)
	, mPlaylistMode(0)
	, mPlayCycleFlag(1) // 根据电信规范0为循环播放，1为播放一次.默认值为1
	, mPlayRandomFlag(0) // 0为顺序播放，1为随机播放。默认值为0
	, mMagicNumber(++gMagicCount)
	, mCurrentStatus(-1)
	, mCurrentSpeed(-1)
	, mCurrentChannelNum(-1)
	, mResourceUserType(Unknown)
	, mResourceRequirement(0)
	, mDisplayMode(1)
	, m_VideoDisplaytop(-1)
	, m_VideoDisplayleft(-1)
	, m_VideoDisplaywidth(-1)
	, m_VideoDisplayheight(-1)
	, m_subtitleFlag(0)
	, m_muteFlag(0)
	, m_PlayerInstanceType(0)
	, mMediaType(APP_TYPE_MAX)
	, mInstanceId(0)
{
    m_mediaCode.clear();
    m_entryId.clear();
    if (mReporter) {
        mReporter->ref();
        mReporter->setPlayer(this);
    }
    int mute= 0;
    appSettingGetInt("mute", &mute, 0);
    if(mute){
        appSettingSetInt("mute", 1);
        if( mAudioMute
			&& UIFlagIsEnabled(AudioMute_Mask)
		    && (defNativeHandler().getState() != NativeHandler::Config && defNativeHandler().getState() != NativeHandler::Local)){
            mAudioMute->setState(AudioMuteWidget::StateMute);
            mAudioMute->setVisibleP(true);
        }
    }
}

UltraPlayer::~UltraPlayer()
{
    mReporter->safeUnref();
}


static pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;

#define MaxPlayers	12
static UltraPlayer *gPlayers[MaxPlayers] = { 0 };

int
UltraPlayer::registerPlayer(int pIndex, UltraPlayer *player)
{
    if (pIndex >= MaxPlayers)
        return -1;

    pthread_mutex_lock(&gMutex);
    if (gPlayers[pIndex]) {
        pthread_mutex_unlock(&gMutex);
        return -1;
    }
    else {
        gPlayers[pIndex] = player;
        pthread_mutex_unlock(&gMutex);
        return 0;
    }
}

int
UltraPlayer::unregisterPlayer(int pIndex, UltraPlayer *player)
{
    if (pIndex >= MaxPlayers)
        return -1;

    pthread_mutex_lock(&gMutex);

    if (gPlayers[pIndex] == player)
        gPlayers[pIndex] = NULL;

    pthread_mutex_unlock(&gMutex);

    return 0;
}

UltraPlayer *
UltraPlayer::lockPlayer(int pIndex)
{
    if (pIndex >= MaxPlayers)
        return NULL;

    pthread_mutex_lock(&gMutex);

    if (gPlayers[pIndex])
        return gPlayers[pIndex];
    else {
        pthread_mutex_unlock(&gMutex);
        return 0;
    }
}

void
UltraPlayer::unlockPlayer(int pIndex, UltraPlayer *player)
{
    if (pIndex >= MaxPlayers)
        return;

    if (player && player == gPlayers[pIndex])
        pthread_mutex_unlock(&gMutex);
}


void
UltraPlayer::setVideoClearFlag(int videoClearFlag)
{
    mVideoClearFlag = videoClearFlag;

}

int
UltraPlayer::getVideoClearFlag()
{
    return mVideoClearFlag;
}

void
UltraPlayer::setUIFlags(uint32_t flags)
{
    mUIFlags |= flags;
}

void
UltraPlayer::clearUIFlags(uint32_t flags)
{
    uint32_t dirr = mUIFlags & flags;
    mUIFlags ^= dirr;
}

bool
UltraPlayer::getUIFlagState(uint32_t flag)
{
    if (mUIFlagsForcedMask & flag) {
        return (mUIFlagsForcedValue & flag);
    }
    return (mUIFlags & flag);
}

void
UltraPlayer::setUIFlagsForcedMask(uint32_t flags)
{
    mUIFlagsForcedMask = flags;
}

void
UltraPlayer::setUIFlagsForcedValue(uint32_t flags)
{
    mUIFlagsForcedValue = flags;
}

bool
UltraPlayer::UIFlagIsEnabled(uint32_t flag)
{
    PLAYER_LOG("mUIFlagsForcedMask 0x%08x\n", mUIFlagsForcedMask);
    PLAYER_LOG("mUIFlagsForcedValue 0x%08x\n", mUIFlagsForcedValue);
    PLAYER_LOG("mUIEnabled 0x%08x\n", mUIEnabled);
    PLAYER_LOG("flag 0x%08x\n", flag);

    if (mUIFlagsForcedMask & flag) {
        return (mUIFlagsForcedValue & flag);
    }
    else {
        return (mUIEnabled && (mUIFlags & flag));
    }
}

/* Audio Control */
void
UltraPlayer::SetVolume(int pVolume)
{
	if(pVolume > AUDIO_VOLUME_MAX){
	    pVolume = AUDIO_VOLUME_MAX;
	}

	if(pVolume < 0){
	    pVolume = 0;
	}
	if(mid_audio_volume_set(pVolume) != 0){
	    PLAYER_LOG_ERROR("Ultraplayer set volume error !\n");
	    return ;
	}
	if(appSettingSetInt("volume", pVolume) != 0){
	    PLAYER_LOG_ERROR("Ultraplayer save volume error !\n");
	    return ;
	}

#if defined(Gansu)
    if(0 == pVolume){
        SetMute(1);
    } else {
        if (1 == pVolume)
            SetMute(0);
        else
            appSettingSetInt("mute", 0);
    }
#else
    if(0 == pVolume){
        appSettingSetInt("mute", 1);
    } else {
        appSettingSetInt("mute", 0);
    }
#endif
	if (UIFlagIsEnabled(AudioVolume_Mask)
	    && NativeHandler::Config != defNativeHandler().getState()
        && NativeHandler::Local != defNativeHandler().getState()){
#if defined(GUANGDONG) || defined(Sichuan)
        if (mAudioMute) {
            if (pVolume) {
                int mute = 0;
                appSettingGetInt("mute", &mute, 0);
                if (!mute) {
                    mAudioMute->setState(AudioMuteWidget::StateUnmute);
                    mAudioMute->setVisibleP(true);
                    this->removeMessages(MessageType_ClearMuteIcon);
                    Message *message = this->obtainMessage(MessageType_ClearMuteIcon, 0, 0);
                    this->sendMessageDelayed(message, 3000);
                }
            }
        }
#else
        if(mAudioVolume) {
            mAudioVolume->setValue(pVolume/5);
        #if defined(Gansu)
            mAudioVolume->setVisibleP(false);
        #else
            mAudioVolume->setVisibleP(true);
        #endif
            this->removeMessages(MessageType_ClearVolumeIcon);
            Message *message = this->obtainMessage(MessageType_ClearVolumeIcon, 0, 0);
            this->sendMessageDelayed(message, 3000);
        }
        if(mAudioMute){
            if(0 == pVolume){
	            mAudioMute->setState(AudioMuteWidget::StateMute);
                mAudioMute->setVisibleP(true);
	        } else {
	            int mute = 0;
                appSettingGetInt("mute", &mute, 0);
                PLAYER_LOG(" mAudioMute=0x%08x   UIFlagIsEnabled=%d  \n", mAudioMute, UIFlagIsEnabled(AudioVolume_Mask));

                if(!mute) {
                    mAudioMute->setState(AudioMuteWidget::StateUnmute);
                #if defined(Gansu)
                    mAudioMute->setVisibleP(false);
                #else
                    mAudioMute->setVisibleP(true);
                #endif
                    this->removeMessages(MessageType_ClearMuteIcon);
                    Message *message = this->obtainMessage(MessageType_ClearMuteIcon, 0, 0);
                    this->sendMessageDelayed(message, 3000);
                }
	        }
    	}
#endif //defined(GUANGDONG)
    }
	this->removeMessages(MessageType_ConfigSave);
    Message *message = this->obtainMessage(MessageType_ConfigSave, 0, 0);
    this->sendMessageDelayed(message, 3000);
	return ;
}

int
UltraPlayer::GetVolume(void)
{
	int volume = 0;
	appSettingGetInt("volume", &volume, 0);

	if(volume < 0) {
		volume = 0;
	}
	else if(volume > AUDIO_VOLUME_MAX) {
		volume = AUDIO_VOLUME_MAX;
	}
    PLAYER_LOG("Get volume(%d)\n", volume);
	return volume;
}

void
UltraPlayer::SetMute(int pMute)
{
	appSettingSetInt("mute", pMute);
	if (UIFlagIsEnabled(AudioMute_Mask)
	    && NativeHandler::Config != defNativeHandler().getState()
        && NativeHandler::Local != defNativeHandler().getState()) {
        if(mAudioMute){
            mAudioMute->setState((AudioMuteWidget::State)pMute);
            mAudioMute->setVisibleP(true);
        }
        this->removeMessages(MessageType_ClearMuteIcon);
        if(!pMute){
		    Message *message = this->obtainMessage(MessageType_ClearMuteIcon, 0, 0);
            this->sendMessageDelayed(message, 3000);
        }
	}
	this->removeMessages(MessageType_ConfigSave);
	Message *message = this->obtainMessage(MessageType_ConfigSave, 0, 0);
    this->sendMessageDelayed(message, 3000);
	return;
}

int
UltraPlayer::GetMute(void)
{
	int mute= 0;
    appSettingGetInt("mute", &mute, 0);

	if(mute != 0){
	    return 1;
	}
	return 0;
}

void
UltraPlayer::SetChannel(int pChannel)
{
#ifdef ANHUI_HD
    int mute= 0;

    appSettingGetInt("mute", &mute, 0);
	if(mute){
		return;
	}
#endif
	if(pChannel < YX_AUDIO_CHANNEL_OUTPUT_STEREO || pChannel > YX_AUDIO_CHANNEL_OUTPUT_SWAP){
		pChannel = YX_AUDIO_CHANNEL_OUTPUT_STEREO;
	}
    yhw_aout_setOutputMode((YX_AUDIO_CHANNEL_MODE)pChannel);

    PLAYER_LOG("Audio Track UiFlag(%d)\n", UIFlagIsEnabled(AudioTrack_Mask));
	if (UIFlagIsEnabled(AudioTrack_Mask)
	    && NativeHandler::Config != defNativeHandler().getState()
        && NativeHandler::Local != defNativeHandler().getState()){
	    if(mAudioTrack){
            mAudioTrack->setState((AudioTrackWidget::State)pChannel, GetTrack());
            mAudioTrack->setVisibleP(true);
        }
        this->removeMessages(MessageType_ClearAudioTrackIcon);
        Message *message = this->obtainMessage(MessageType_ClearAudioTrackIcon, 0, 0);
        this->sendMessageDelayed(message, 3000);
	}
	return;
}

int
UltraPlayer::GetChannel(void)
{
	int chnl;

	if(yhw_aout_getOutputMode((YX_AUDIO_CHANNEL_MODE *)&chnl) == 0){
	    return chnl;
	}
	return YX_AUDIO_CHANNEL_OUTPUT_STEREO;
}

void
UltraPlayer::SetTrack(int pIndex)
{
	int i = 0, num = 0, track = 0;

#ifdef ANHUI_HD
    int mute= 0;
    appSettingGetInt("mute", &mute, 0);
	if(mute){
		return;
	}
#endif

    codec_audio_track_num(&num);
	if(num >= pIndex){
	    track = pIndex - 1;
	} else {
	    for(i = 0; i < num; i++) {
	        int CurAudioPid = 0;

            codec_audio_track_get_pid(i, &CurAudioPid);
            if(CurAudioPid == pIndex) {
                track = i;
                break;
            }
	    }
	}
	codec_audio_track_set(track);

	if(mDolbyIcon) {
	    int tAudioType = 0;
        Message *message = this->obtainMessage(MessageType_ClearDolbyIcon, 0, 0);

        this->removeMessages(MessageType_ClearDolbyIcon);
	    codec_audio_track_get_type(track, &tAudioType);
	    if(YX_AUDIO_TYPE_AC3 == tAudioType || YX_AUDIO_TYPE_AC3PLUS == tAudioType) {
    	    mDolbyIcon->setVisibleP(true);
            mDolbyDownmixIconShow();
            this->sendMessageDelayed(message, 5000);
        } else {
            this->sendMessage(message);
        }
    }
	return;
}

int
UltraPlayer::GetTrack(void)
{
    int track = 0;

    codec_audio_track_get(&track);
    return track;
}

int
UltraPlayer::AudioTrackPid(void)
{
    int track = 0;
    int CurAudioPid = 0;

    codec_audio_track_get(&track);
    codec_audio_track_get_pid(track, &CurAudioPid);
    return CurAudioPid;
}

int
UltraPlayer::GetTrackInfo(char *pTrackInfo)
{
    int track = 0;

	if(!pTrackInfo){
		return -1;
	}

	codec_audio_track_get(&track);
	return codec_audio_track_get_info(track, pTrackInfo);
}

int
UltraPlayer::GetAllTeletextInfo(std::string& strValue)
{
    int teletextNum = 0;
    int tIndex = 0;
    char languageCode[4] = {0};
    char languageName[32] = {0};
    json_object* jsonInfo = NULL;
    struct json_object *infoArray = NULL;
    struct json_object *eventList = NULL;

    jsonInfo = json_object_create_object();
    if (!jsonInfo)
        return 0;
    infoArray = json_object_create_array();
    if (!infoArray) {
        json_object_delete(jsonInfo);
        return 0;
    }

    codec_teletext_num(&teletextNum);
    if(teletextNum == 0){
        json_object_object_add(jsonInfo, "teletext_count", json_object_new_int(0));
        strValue = json_object_to_json_string(jsonInfo);
        json_object_delete(infoArray);
        json_object_delete(jsonInfo);
        PLAYER_LOG("strValue=%s\n",strValue.c_str());
        return 0;
    }

    json_object_object_add(jsonInfo, "teletext_count", json_object_new_int(teletextNum));
    for(tIndex = 0; tIndex < teletextNum; tIndex++){
        codec_teletext_lang(tIndex, languageCode);
        if (languageCode[0] != '\0')
            mid_get_language_full_name_from_iso639(languageCode, languageName, 32);
        json_object_object_add(eventList, "PID", json_object_new_int(tIndex));
        json_object_object_add(eventList, "language_code", json_object_new_string(languageCode));
        json_object_object_add(eventList, "language_eng_name", json_object_new_string(languageName));
        json_array_add_object(infoArray, eventList);
    }
    json_object_object_add(jsonInfo, "teletext_list", infoArray);
    strValue = json_object_to_json_string(jsonInfo);
    json_object_delete(jsonInfo);

    PLAYER_LOG("strValue=%s\n",strValue.c_str());
    return 0;
}

int
UltraPlayer::GetAllAudioTrackInfo(std::string& strValue)
{
    int tIndex = 0;
    int audioNum = 0;
    int audioPid = -1;
    char languageCode[4] = {0};
    char languageName[32] = {0};
    json_object* jsonInfo = NULL;
    struct json_object *infoArray = NULL;
    struct json_object *eventList = NULL;

    jsonInfo = json_object_create_object();
    if (!jsonInfo)
        return 0;
    infoArray = json_object_create_array();
    if (!infoArray) {
        json_object_delete(jsonInfo);
        return 0;
    }

    codec_audio_track_num(&audioNum);
    if (audioNum == 0) {
        json_object_object_add(jsonInfo, "audio_track_list_count", json_object_new_int(0));
        strValue = json_object_to_json_string(jsonInfo);
        json_object_delete(infoArray);
        json_object_delete(jsonInfo);
        PLAYER_LOG("strValue=%s\n",strValue.c_str());
        return 0;
    }

    json_object_object_add(jsonInfo, "audio_track_list_count", json_object_new_int(audioNum));
    for(tIndex = 0; tIndex < audioNum; tIndex++){
        codec_audio_track_get_pid(tIndex, &audioPid);
        codec_audio_track_get_info(tIndex, languageCode);
        if (languageCode[0] != '\0')
            mid_get_language_full_name_from_iso639(languageCode, languageName, 32);
        eventList = json_object_create_object();
        if (!eventList) {
            json_object_delete(infoArray);
            json_object_delete(jsonInfo);
            return 0;
        }
        json_object_object_add(eventList, "PID", json_object_new_int(audioPid));
        json_object_object_add(eventList, "language_code", json_object_new_string(languageCode));
        json_object_object_add(eventList, "language_eng_name", json_object_new_string(languageName));
        json_array_add_object(infoArray, eventList);
    }
    json_object_object_add(jsonInfo, "audio_track_list", infoArray);
    strValue = json_object_to_json_string(jsonInfo);
    json_object_delete(jsonInfo);

    PLAYER_LOG("strValue=%s\n",strValue.c_str());
    return 0;

}

int
UltraPlayer::GetCurrentAudioTrackInfo(HString& strValue)
{
    int tIndex = 0;
    int audioPid = -1;
    int audioNum = 0;
    char languageCode[4] = {0};
    char languageName[32] = {0};
    json_object* jsonInfo = NULL;

    jsonInfo = json_object_create_object();
    if (!jsonInfo)
        return 0;

    codec_audio_track_num(&audioNum);
    if (audioNum == 0) {
        json_object_object_add(jsonInfo, "PID", json_object_new_int(-1));
        strValue = json_object_to_json_string(jsonInfo);
        json_object_delete(jsonInfo);
        PLAYER_LOG("strValue=%s\n",strValue.c_str());
        return 0;
    }
	codec_audio_track_get(&tIndex);
	codec_audio_track_get_pid(tIndex, &audioPid);
	codec_audio_track_get_info(tIndex, languageCode);

    if (languageCode[0] != '\0')
        mid_get_language_full_name_from_iso639(languageCode, languageName, 32);
    json_object_object_add(jsonInfo, "PID", json_object_new_int(audioPid));
    json_object_object_add(jsonInfo, "language_code", json_object_new_string(languageCode));
    json_object_object_add(jsonInfo, "language_eng_name", json_object_new_string(languageName));
    strValue = json_object_to_json_string(jsonInfo);
    json_object_delete(jsonInfo);

    PLAYER_LOG("strValue=%s\n",strValue.c_str());
    return 0;
}

int
UltraPlayer::GetAllSubtitleInfo(std::string& strValue)
{
	int tIndex = 0;
	int subtitleNum = 0;
	char languageCode[4] = {0};
	char languageName[32] = {0};
	unsigned short subtitlePid = 0;
    json_object* jsonInfo = NULL;
    struct json_object *infoArray = NULL;
    struct json_object *eventList = NULL;

	jsonInfo = json_object_create_object();
    if (!jsonInfo)
        return 0;
    infoArray = json_object_create_array();
    if (!infoArray) {
        json_object_delete(jsonInfo);
        return 0;
    }
    codec_subtitle_num(&subtitleNum);

    if (subtitleNum == 0) {
        json_object_object_add(jsonInfo, "subtitle_list_count", json_object_new_int(0));
        strValue = json_object_to_json_string(jsonInfo);
        json_object_delete(infoArray);
        json_object_delete(jsonInfo);
        PLAYER_LOG("strValue=%s\n",strValue.c_str());
        return 0;
    }
    json_object_object_add(jsonInfo, "subtitle_list_count", json_object_new_int(subtitleNum));
    for (tIndex = 0; tIndex < subtitleNum; tIndex++) {
        codec_subtitle_pid(tIndex, &subtitlePid);
        codec_subtitle_lang(tIndex, languageCode);
        if(languageCode[0] !=  '\0')
            mid_get_language_full_name_from_iso639(languageCode, languageName, 32);
        eventList = json_object_create_object();
        if (!eventList) {
            json_object_delete(infoArray);
            json_object_delete(jsonInfo);
            return 0;
        }
        json_object_object_add(eventList, "PID", json_object_new_int(subtitlePid));
        json_object_object_add(eventList, "language_code", json_object_new_string(languageCode));
        json_object_object_add(eventList, "language_eng_name", json_object_new_string(languageName));
        json_array_add_object(infoArray, eventList);
    }
    json_object_object_add(jsonInfo, "subtitle_list", infoArray);
    strValue = json_object_to_json_string(jsonInfo);
    json_object_delete(jsonInfo);

    PLAYER_LOG("strValue=%s\n",strValue.c_str());
    return 0;

}

int
UltraPlayer::GetCurrentSubtitleInfo(HString& strValue)
{
    int tIndex = 0;
    unsigned short subtitlePid = 0;
    char languageCode[4] = {0};
    char languageName[32] = {0};
    json_object* jsonInfo = NULL;

    jsonInfo = json_object_create_object();
    if (!jsonInfo)
        return 0;

    if(!codec_subtitle_show_get(0)){
        json_object_object_add(jsonInfo, "PID", json_object_new_string("null"));
        strValue = json_object_to_json_string(jsonInfo);
        json_object_delete(jsonInfo);
        PLAYER_LOG("strValue=%s\n",strValue.c_str());
        return 0;
    }

    codec_subtitle_get(&tIndex);
    codec_subtitle_pid(tIndex, &subtitlePid);
    codec_subtitle_lang(tIndex,languageCode);
    if(languageCode[0]!= '\0')
        mid_get_language_full_name_from_iso639(languageCode, languageName, 32);
    json_object_object_add(jsonInfo, "PID", json_object_new_int(subtitlePid));
    json_object_object_add(jsonInfo, "language_code", json_object_new_string(languageCode));
    json_object_object_add(jsonInfo, "language_eng_name", json_object_new_string(languageName));
    strValue = json_object_to_json_string(jsonInfo);
    json_object_delete(jsonInfo);

    PLAYER_LOG("strValue=%s\n",strValue.c_str());
    return 0;
}

void
UltraPlayer::selectTeletext(int teletext)
{
	codec_teletext_set(teletext);
}

void
UltraPlayer::SelectSubtitle(int subtitlePid)
{
    int tIndex = 0;
    int subtitleNum = 0;
    unsigned short id = 0;

    codec_subtitle_num(&subtitleNum);

    if(subtitlePid <= subtitleNum) {
        tIndex = subtitlePid - 1;
    } else {
        for (tIndex = 0; tIndex < subtitleNum; tIndex++){
            codec_subtitle_pid(tIndex, &id);
            if (id == subtitlePid) {
                break;
            }
        }
    }
    codec_subtitle_set(tIndex);
    if (!codec_subtitle_show_get(0)) {
        codec_subtitle_show_set(1);
    }
    return;
}

void
UltraPlayer::SelectAudioTrack(int audioTrackPid)
{
    int tIndex = 0;
    int audioTrackNum = 0;
    int audioPid = -1;

    codec_audio_track_num(&audioTrackNum);
    for(tIndex = 0; tIndex < audioTrackNum; tIndex++) {
        codec_audio_track_get_pid(tIndex, &audioPid);
        if(audioPid == audioTrackPid) {
        	codec_audio_track_set(tIndex);
            break;
        }
    }

    if(mDolbyIcon) {
        int tAudioType = 0;
        Message *message = this->obtainMessage(MessageType_ClearDolbyIcon, 0, 0);

        this->removeMessages(MessageType_ClearDolbyIcon);
        codec_audio_track_get_type(tIndex, &tAudioType);
        if(YX_AUDIO_TYPE_AC3 == tAudioType || YX_AUDIO_TYPE_AC3PLUS == tAudioType) {
            mDolbyIcon->setVisibleP(true);
            mDolbyDownmixIconShow();
            this->sendMessageDelayed(message, 5000);
        } else {
            this->sendMessage(message);
        }
    }
    return;
}

void
UltraPlayer::SwitchAudioTrack(void)
{
	int tIndex = 0;
	int audioTrackNum = 0;

	codec_audio_track_num(&audioTrackNum);
	codec_audio_track_get(&tIndex);
	if(tIndex == audioTrackNum - 1){
		tIndex = 0;
	} else {
		tIndex++;
	}
	codec_audio_track_set(tIndex);
    if(mDolbyIcon) {
        int tAudioType = 0;
        Message *message = this->obtainMessage(MessageType_ClearDolbyIcon, 0, 0);

        this->removeMessages(MessageType_ClearDolbyIcon);
        codec_audio_track_get_type(tIndex, &tAudioType);
        if(YX_AUDIO_TYPE_AC3 == tAudioType || YX_AUDIO_TYPE_AC3PLUS == tAudioType) {
            mDolbyIcon->setVisibleP(true);
            mDolbyDownmixIconShow();
            this->sendMessageDelayed(message, 5000);
        } else {
            this->sendMessage(message);
        }
    }
    return;
}

void
UltraPlayer::SwitchSubtitle(void)
{
    int tIndex = 0;
    int subtitleNum = 0;

    codec_subtitle_num(&subtitleNum);
    codec_subtitle_get(&tIndex);
    if (tIndex == subtitleNum - 1) {
        tIndex = 0;
    } else {
        tIndex++;
    }

    codec_subtitle_set(tIndex);
    if (!codec_subtitle_show_get(0)) {
        codec_subtitle_show_set(1);
    }
}

void
UltraPlayer::SwitchAudioChannel(HString strValue)
{
	const char *strChannels[] = {"Stereo", "Left", "Right", "MONO"};
	/*	0：stereo -> 3: mix -> 1: left -> 2: right */
	const int channel[4] = {0, 3, 1, 2};
	int tIndex = 0;
	int i = 0;

	if(strValue.compare("undefined")){
		for(i = 0; i < 4; i++){
			if(!strValue.compare(strChannels[i])){
                yhw_aout_setOutputMode((YX_AUDIO_CHANNEL_MODE)i);
				break;
			}
		}
		return;
	}
	int chnl;

	if(yhw_aout_getOutputMode((YX_AUDIO_CHANNEL_MODE *)&chnl) == 0){
	    tIndex = chnl;
	} else {
	    tIndex = YX_AUDIO_CHANNEL_OUTPUT_STEREO;
	}

	for(i = 0; i < 4; i++){
		if(tIndex == channel[i]){
			break;
		}
	}
	if(i == 4 || i == 3){
		i = 0;
	} else {
		i++;
	}
    yhw_aout_setOutputMode((YX_AUDIO_CHANNEL_MODE)channel[i]);

	return;
}

int
UltraPlayer::GetCurrentAudioChannel(HString& strValue)
{
	int tIndex = 0;
#ifdef HUAWEI_C10
	const char *strChannels[] = {"Stereo", "Left", "Right", "JointStereo"};
#else
	const char *strChannels[] = {"Stereo", "Left", "Right", "MONO"};
#endif

	int chnl;

	if(yhw_aout_getOutputMode((YX_AUDIO_CHANNEL_MODE *)&chnl) == 0){
	    tIndex = chnl;
	} else {
	    tIndex = YX_AUDIO_CHANNEL_OUTPUT_STEREO;
	}

	if(tIndex >= 0 || tIndex <= 3){
		strValue = strChannels[tIndex];
	}
	return 0;
}

void
UltraPlayer::SetSubtitileFlag(int subtitleFlag)
{
	codec_subtitle_show_set(subtitleFlag);
}

int
UltraPlayer::GetSubtitileFlag(void)
{
	return codec_subtitle_show_get(0);
}

void
UltraPlayer::SetMacrovisionFlag(int macrovisionFlag)
{
	int m_Flag = 0;
	if(macrovisionFlag == -1){
		if(mProgram->getType() == Program::PT_CHANNEL){
			m_Flag = ((ProgramChannel *)mProgram)->GetMacrovEnable();
		} else {
			appSettingGetInt("macrovision", &m_Flag, 0);
		}
	} else {
		m_Flag = macrovisionFlag;
	}
	if(m_PlayerInstanceType != 1){
		mid_sys_Macrovision_set(m_Flag);
	} else {
		SystemManager &sysManager = systemManager();
		UltraPlayer *player = sysManager.obtainMainPlayer();
		if(!player){
			mid_sys_Macrovision_set(m_Flag);
		}
		sysManager.releaseMainPlayer(player);
	}
}

void
UltraPlayer::SetHDCPFlag(int HDCPFlag)
{
	int m_Flag = 0;
        PLAYER_LOG("HDCPFlag=%d\n",HDCPFlag);
	if(HDCPFlag == -1){
		if(mProgram->getType() == Program::PT_CHANNEL){
			m_Flag = ((ProgramChannel *)mProgram)->GetHDCP_Enable();
		} else {
			appSettingGetInt("HDCPEnableDefault", &m_Flag, 0);
		}
	} else {
		m_Flag = HDCPFlag;
	}
	if(m_PlayerInstanceType != 1){
		mid_sys_hdcp_enableflag_set(m_Flag);
	} else {
		SystemManager &sysManager = systemManager();
		UltraPlayer *player = sysManager.obtainMainPlayer();
		if(!player){
			mid_sys_hdcp_enableflag_set(m_Flag);
		}
		sysManager.releaseMainPlayer(player);
	}
}

void
UltraPlayer::SetCGMSAFlag(int CGMSAFlag)
{
	int m_Flag = 0;

    PLAYER_LOG("CGMSAFlag=%d\n",CGMSAFlag);
	if(CGMSAFlag == -1){
		if(mProgram->getType() == Program::PT_CHANNEL){
			m_Flag = ((ProgramChannel *)mProgram)->GetCGMSAEnable();
		} else {
			appSettingGetInt("CGMSAEnableDefault", &m_Flag, 0);
		}
	} else {
		m_Flag = CGMSAFlag;
	}

	if(m_PlayerInstanceType != 1){
		mid_sys_cgmsa_enableflag_set(m_Flag);
	} else {
		SystemManager &sysManager = systemManager();
		UltraPlayer *player = sysManager.obtainMainPlayer();
		if(!player){
		    mid_sys_cgmsa_enableflag_set(m_Flag);
		}
		sysManager.releaseMainPlayer(player);
	}
}

int
UltraPlayer::GetPlayBackMode(HString& aValue)
{
	switch(mCurrentStatus) {
		case STRM_STATE_CLOSE:
			aValue += "{\"PlayMode\":\"Stop\",\"Speed\":\"1x\"}";
			break;
		case STRM_STATE_PAUSE:
			aValue += "{\"PlayMode\":\"Pause\",\"Speed\":\"1x\"}";
			break;
		case STRM_STATE_PLAY:
        case STRM_STATE_IPTV:
			aValue += "{\"PlayMode\":\"Normal Play\",\"Speed\":\"1x\"}";
			break;
		case STRM_STATE_FAST:
			aValue += "{\"PlayMode\":\"Trickmode\",\"Speed\":\"";
			aValue += mCurrentSpeed;
			aValue += "x\"}";
			break;
	}
    return 0;
}

int
UltraPlayer::GetCurrentPlayUrl(std::string& strValue)
{
    if(mProgram) {
        if (mProgram->getType() == Program::PT_CHANNEL)
            strValue = ((ProgramChannel*)mProgram)->GetChanURL();
        else if (mProgram->getType() == Program::PT_VOD)
            strValue = mProgram->getVodSource(0)->GetUrl();
        else
            strValue = "";
    } else {
        strValue = "";
    }

   return 0;
}

int
UltraPlayer::setStreamVideoLocation(int pIndex, int x, int y, int w, int h, int mode )
{
    if((x < 0) || (y < 0) || (w < 0) || (h < 0))
        return -1;

	int s_width = 0;
	int s_height = 0;

    PLAYER_LOG("Index(%d)Mode(%d)x(%d)y(%d)w(%d)h(%d)\n", pIndex, mode, x, y, w, h);
	if(1 == mode ){
		mid_stream_rect(pIndex, 0, 0, 0, 0);
		mid_sys_aspectRatioMode_set(2);
    	mid_sys_video_show(1);
	} else if(2 == mode) {
		mid_stream_rect(pIndex, 0, 0, 0, 0);
		mid_sys_aspectRatioMode_set(1);
    	mid_sys_video_show(1);
	} else if(3 == mode){
		mid_stream_rect(pIndex, 0, 0, 0, 0);
		mid_sys_aspectRatioMode_set(0);
    	mid_sys_video_show(1);
	} else if (0 == mode){
		ygp_layer_getScreenSize(&s_width, &s_height);
		PLAYER_LOG("Screen width(%d) height(%d)\n", s_width, s_height);
		PLAYER_LOG("Screen max width(%d) height(%d)\n", SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT);
		if(w >= s_width || h >= s_height) {
		    mid_stream_rect(pIndex, 0, 0, 0, 0);
		} else {
    		mid_stream_rect(pIndex, ((x * s_width)/SCREEN_MAX_WIDTH), ((y * s_height)/SCREEN_MAX_HEIGHT), ((w * s_width)/SCREEN_MAX_WIDTH), ((h * s_height)/SCREEN_MAX_HEIGHT));
    	}
    	mid_sys_video_show(1);
	} else if(255 == mode){
	    mid_sys_video_show(0);
	} else {
		return -1;
	}

	return 0;
}

void
UltraPlayer::refreshVideoDisplay()
{
	setStreamVideoLocation(mIndex, m_VideoDisplaytop, m_VideoDisplayleft, m_VideoDisplaywidth, m_VideoDisplayheight, mDisplayMode);
}

void
UltraPlayer::handleMessage(Message *msg)
{
    if(msg){
        PLAYER_LOG("Ultraplayer receive msessage what(0x%x), info(0x%x)\n", msg->what, msg->arg1);
        switch(msg->what){
            case MessageType_ConfigSave:{
                settingManagerSave(); // 写入文件
                break;
            }
            case MessageType_ClearMuteIcon:{
                if(mAudioMute){
                    mAudioMute->setVisibleP(false);
                }
                break;
            }
            case MessageType_ClearVolumeIcon:{
                if(mAudioVolume){
                    mAudioVolume->setVisibleP(false);
                }
                break;
            }
            case MessageType_ClearAudioTrackIcon:{
                if(mAudioTrack){
                    mAudioTrack->setVisibleP(false);
                }
                break;
            }
            case MessageType_ClearPlayStateIcon:{
                if(mPlayState){
                    mPlayState->setVisibleP(false);
                }
                break;
            }
            case MessageType_ClearDolbyIcon: {
                if(mDolbyIcon){
                    mDolbyIcon->setVisibleP(false);
                    mDolbyDownmixIcon->setVisibleP(false);
                }
                break;
            }
            case MessageType_ClearAllIcon: {
                ClearAllIcon();
                break;
            }
            default:{
                break;
            }
        }
        if(STRM_MSG_STREAM_VIEW == msg->arg1) {
            if(mDolbyIcon) {
        	    int tAudioType = 0;
                Message *message = this->obtainMessage(MessageType_ClearDolbyIcon, 0, 0);

        	    this->removeMessages(MessageType_ClearDolbyIcon);
        	    codec_audio_track_get_type(GetTrack(), &tAudioType);
        	    if(YX_AUDIO_TYPE_AC3 == tAudioType || YX_AUDIO_TYPE_AC3PLUS == tAudioType) {
            	    mDolbyIcon->setVisibleP(true);
            	    mDolbyDownmixIconShow();
                    this->sendMessageDelayed(message, 5000);
                } else {
                    this->sendMessage(message);
                }
            }
        }

    }
    return ;
}

PlayStateWidget::State
UltraPlayer::ShowPlayStateIcon(PlayStateWidget::State pState)
{
    PlayStateWidget::State tOldState =  PlayStateWidget::StateMax;

    PLAYER_LOG("Play state(%d) mMediaType(%d) mDisplayMode(%d) \n", pState, mMediaType, mDisplayMode);
    PLAYER_LOG("PlayState_Mask(%d)\n", UIFlagIsEnabled(PlayState_Mask));
    if ((APP_TYPE_VOD == mMediaType || APP_TYPE_VODADV == mMediaType)
        && (PlayStateWidget::StateLive == pState || PlayStateWidget::StateTimeShift == pState))
        return tOldState;
#if (defined(GUANGDONG) || defined(Jiangsu) || defined(Chongqing)) // Guangdong bizarre requirements.Mandatory display timeshift and live icon in the timeshift playing.
    if((UIFlagIsEnabled(PlayState_Mask) || PlayStateWidget::StateLive == pState || PlayStateWidget::StateTimeShift == pState)
        // && mDisplayMode 广东有些页面设置的此值不规范。
        && APP_TYPE_ZEBRA != mMediaType
        && APP_TYPE_HTTP_MPA != mMediaType)
#else
    if(UIFlagIsEnabled(PlayState_Mask) && mDisplayMode && APP_TYPE_ZEBRA != mMediaType)
#endif
    {
        if(mPlayState){
            tOldState = mPlayState->setState(pState);
            mPlayState->setVisibleP(true);
        }
        this->removeMessages(MessageType_ClearPlayStateIcon);
        if(PlayStateWidget::StatePlay == pState
           || PlayStateWidget::StateLive == pState
           || PlayStateWidget::StatePrevious == pState
           || PlayStateWidget::StateNext == pState
           || PlayStateWidget::StateStop == pState){
            Message *message = this->obtainMessage(MessageType_ClearPlayStateIcon, 0, 0);
            this->sendMessageDelayed(message, 3000);
        }
	}
	return tOldState;
}

void
UltraPlayer::ClearAllIcon(void)
{
    if(mPlayState){
        mPlayState->setVisibleP(false);
    }
    if(mChannelNO){
        mChannelNO->setVisibleP(false);
    }
    if(mProgressBar){
        mProgressBar->setVisibleP(false);
    }
    if(mAudioMute){
        mAudioMute->setVisibleP(false);
    }
    if(mAudioTrack){
        mAudioTrack->setVisibleP(false);
    }
    if(mAudioVolume){
        mAudioVolume->setVisibleP(false);
    }
    if(mDolbyIcon){
        mDolbyIcon->setVisibleP(false);
        mDolbyDownmixIcon->setVisibleP(false);
    }
    return ;
}

void
UltraPlayer::mDolbyDownmixIconShow(void)
{
    if(mDolbyDownmixIcon) {
        int DownmixStatus = 0;

        ymm_decoder_getDolbyDownmixingStatus(&DownmixStatus);
        if(1 == DownmixStatus)
            mDolbyDownmixIcon->setVisibleP(true);

    }
}

} // namespace Hippo

// add for app/v1/app_tr069_port_b200.c (void tr069_diagnostics_task(int arg))
extern "C"
int GetCurrentPlayStatus()
{
    int PlayStatus = 0;
    Hippo::SystemManager &sysManager = Hippo::systemManager();
    Hippo::UltraPlayer *player = sysManager.obtainMainPlayer();

    if(player){
        PlayStatus = player->mCurrentStatus;
    }
    sysManager.releaseMainPlayer(player);
    return PlayStatus;
}

