
#include "BrowserPlayer.h"
#include "BrowserAssertions.h"
#include "BrowserPlayerReporter.h"
#include "BrowserPlayerReporterUtility.h"

#include "SystemManager.h"
#include "ProgramParser.h"
#include "ProgramVOD.h"
#include "UltraPlayerVod.h"
#ifdef PLAY_BGMUSIC
#include "UltraPlayerBGMusic.h"
#endif
#include "UltraPlayerUtility.h"

#include "mid/mid_time.h"
#include "mid/mid_tools.h"

#include "AppSetting.h"
#include "Hippo_Context.h"
#include "codec.h"

namespace Hippo {

BrowserPlayer::BrowserPlayer(int id, player_type_e playerInstanceType)
    : PlayerHWBase(id)
    , mPlayerInstanceType(playerInstanceType)
    , mActualPlayer(NULL)
    , mPlayerMagic(0)
    , mPlaylistMode(0)
    , mPlaylistCycleFlag(1) // 根据电信规范0为循环播放，1为播放一次.默认值为1
    , mPlaylistRandomFlag(0) // 0为顺序播放，1为随机播放。默认值为0
    , mProgramToPlay(0)
    , mMuteFlag(0)
    , mSubtitleFlag(1)
    , mMacrovisionFlag(-1)
    , mHDCPFlag(-1)
    , mCGSMAFlag(-1)
{
}

BrowserPlayer::~BrowserPlayer()
{
}

int
BrowserPlayer::open()
{
    return 0;
}

int
BrowserPlayer::play(int startTime, time_type_e timeType)
{
    SystemManager &sysManager = systemManager();

    BROWSER_LOG("timeType %d, startTime %d\n", (int)timeType, startTime);
    if(mProgramToPlay) {
        UltraPlayer *player = sysManager.obtainMainPlayer();
        if(player)
            player->stop();
        sysManager.releaseMainPlayer(player);
        sysManager.detachMainPlayer(player);

        BrowserPlayerReporter *reporter = BrowserPlayerReporterCreate(mProgramToPlay);

        player = UltraPlayerUtility::createPlayerByProgram(this, reporter, mProgramToPlay);
        reporter->safeUnref();
        mProgramToPlay->unref();
        mProgramToPlay = 0;
        if(player == 0)
            return -1;
        player->mPlaylistMode = mPlaylistMode;
        player->mPlayCycleFlag = mPlaylistCycleFlag;
        player->mPlayRandomFlag = mPlaylistRandomFlag;
        player->mDisplayMode = m_eVideoMode;
        mPlayerMagic = player->magicNumber();
        player->play(startTime);

        player->removeMessages(MessageType_ConfigSave);
        Message *message = player->obtainMessage(MessageType_ConfigSave, 0, 0);
        player->sendMessageDelayed(message, 3000);

        if(!player->isFake()) {
            sysManager.attachMainPlayer(player);
        }
        player->unref();
    } else {
        UltraPlayer *player = sysManager.obtainMainPlayer();
        if(player && player->magicNumber() == mPlayerMagic) {
            player->play(startTime);
        }
        sysManager.releaseMainPlayer(player);
    }

    return 0;
}

int
BrowserPlayer::fastForward(int speed, unsigned long playTime, time_type_e timeType)
{
    SystemManager &sysManager = systemManager();

    BROWSER_LOG("Player %dx fast forward !\n", speed);
    UltraPlayer *player = sysManager.obtainMainPlayer();
    if(player && player->magicNumber() == mPlayerMagic) {
        BROWSER_LOG("Have player instance and call fastForward !\n");
        player->fastForward(speed);
    }
    sysManager.releaseMainPlayer(player);
    BROWSER_LOG("Player fastForward release player instance!\n");
    return 0;
}

int
BrowserPlayer::fastRewind(int speed, unsigned long playTime, time_type_e timeType)
{
    SystemManager &sysManager = systemManager();

    BROWSER_LOG("Player %dx fast rewind !\n", speed);
    UltraPlayer *player = sysManager.obtainMainPlayer();
    if(player && player->magicNumber() == mPlayerMagic) {
        BROWSER_LOG("Have player instance and call fastRewind !\n");
        player->fastRewind(speed);
    }
    sysManager.releaseMainPlayer(player);
    BROWSER_LOG("Player fastRewind release player instance!\n");
    return 0;
}

int
BrowserPlayer::seekTo(unsigned long playTime, time_type_e timeType)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();
    int seekTotime = 0;

    BROWSER_LOG("Player seek to by play time(long %ld) and time type %d!\n", playTime, (int)timeType);
    if(mProgramToPlay) {
        if(player) {
            player->stop();
        }
        sysManager.releaseMainPlayer(player);
        sysManager.detachMainPlayer(player);
        BrowserPlayerReporter *reporter = BrowserPlayerReporterCreate(mProgramToPlay);

        player = UltraPlayerUtility::createPlayerByProgram(this, reporter, mProgramToPlay);
        reporter->safeUnref();
        mProgramToPlay->unref();
        mProgramToPlay = 0;
        if(player == 0)
            return -1;
        player->mPlaylistMode = mPlaylistMode;
        player->mPlayCycleFlag = mPlaylistCycleFlag;
        player->mPlayRandomFlag = mPlaylistRandomFlag;
        player->mDisplayMode = m_eVideoMode;
        mPlayerMagic = player->magicNumber();
        player->play(playTime);
        if(!player->isFake()) {
            sysManager.attachMainPlayer(player);
        }
        player->unref();
    } else {
        if(player && player->magicNumber() == mPlayerMagic) {
            switch(timeType) {
            case Player::TimeType_eMediaStart: {
                seekTotime = 0;
                break;
            }
            case Player::TimeType_eMediaEnd: {
#ifdef Liaoning
                player->seekEnd(); // 一键到尾立即结束
#else
                player->stop(); // 一键到尾播放3秒
#endif
                return 0;
            }
            default: {
                seekTotime = playTime;
                break;
            }
            }
            player->seekTo(seekTotime);
        }
        sysManager.releaseMainPlayer(player);
    }
    return 0;
}

int
BrowserPlayer::seekTo(const char *playTime, time_type_e timeType)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();
    int seekTotime = 0;

    BROWSER_LOG("Player seek to by play time(char %s) and time type %d!\n", playTime, (int)timeType);
    if(mProgramToPlay) {
        if(player) {
            player->stop();
        }
        sysManager.releaseMainPlayer(player);
        sysManager.detachMainPlayer(player);

        BrowserPlayerReporter *reporter = BrowserPlayerReporterCreate(mProgramToPlay);

        player = UltraPlayerUtility::createPlayerByProgram(this, reporter, mProgramToPlay);
        reporter->safeUnref();
        mProgramToPlay->unref();
        mProgramToPlay = 0;
        if(player == 0)
            return -1;
        player->mPlaylistMode = mPlaylistMode;
        player->mPlayCycleFlag = mPlaylistCycleFlag;
        player->mPlayRandomFlag = mPlaylistRandomFlag;
        player->mDisplayMode = m_eVideoMode;
        mPlayerMagic = player->magicNumber();
        if(timeType == Player::TimeType_eNPT) {
            player->play(atoi((char *)playTime));
        }
        if(!player->isFake()) {
            sysManager.attachMainPlayer(player);
        }
        player->unref();
    } else {
        if(player && player->magicNumber() == mPlayerMagic && playTime != NULL) {
            switch(timeType) {
            case Player::TimeType_eMediaStart: {
                seekTotime = mid_time() - player->getTotalTime();
                break;
            }
            case Player::TimeType_eMediaEnd: {
#ifdef Liaoning
                player->seekEnd(); // 一键到尾立即结束
#else
                player->stop(); // 一键到尾播放3秒
#endif
                return 0;
            }
            case Player::TimeType_eNPT: {
                BROWSER_LOG("Unsupported TimeType_eNPT time:%s \n", playTime);
                seekTotime = atoi((char *)playTime);
                break;
            }
            case Player::TimeType_eUTC: {
                unsigned int systime = 0;
                seekTotime =  mid_tool_string2time((char *)playTime);
#if defined(VIETTEL_HD)
                if (MID_UTC_SUPPORT == 0) {
                    int timezone = 0;
                    sysSettingGetInt("timezone", &timezone, 0);
                    seekTotime = seekTotime +  mid_tool_timezone2sec(timezone);
                }
#endif
                systime = mid_time();
                if (!seekTotime && (atoi(playTime) > 0))
                    seekTotime = systime - atoi(playTime);
                BROWSER_LOG("seekTotime %s<->%d systime %d\n", playTime, seekTotime, systime);
                if(seekTotime >= systime - 3) {
                    player->stop();
                    return 0;
                }
                /*else { // C15基线调试时发现这加上时区后无法时移，忘记为啥改这可能是上海集采用
                    seekTotime += mid_get_times_sec();
                }*/
                break;
            }
            default: {
                seekTotime = 0;
                break;
            }
            }
            player->seekTo(seekTotime);
        }
        sysManager.releaseMainPlayer(player);
    }
    return 0;
}

int
BrowserPlayer::pause()
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();

    BROWSER_LOG("Player pause !\n");
    if(player && player->magicNumber() == mPlayerMagic) {
        BROWSER_LOG("Have player instance and call pause !\n");
        player->pause();
    }
    sysManager.releaseMainPlayer(player);
    BROWSER_LOG("Player pause release player instance!\n");
    return 0;
}

int
BrowserPlayer::resume()
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();

    BROWSER_LOG("Player resume !\n");
    if(player && player->magicNumber() == mPlayerMagic) {
        player->resume();
    }
    sysManager.releaseMainPlayer(player);
    return 0;
}

int
BrowserPlayer::stop()
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();

    BROWSER_LOG("Player stop !\n");
    if(player && player->magicNumber() == mPlayerMagic) {
        if(mPlaylistMode) {
            player->close(UltraPlayer::LastFrameMode);
        } else {
            if(Program::PT_CHANNEL != player->program()->getType()) {
                player->close(UltraPlayer::BlackScreenMode);
            } else {
                player->stop();
                sysManager.releaseMainPlayer(player);
                return 0;
            }
        }
    }
    sysManager.releaseMainPlayer(player);

    if(mPlaylistMode)
        return 0;

#ifdef PLAY_BGMUSIC
    sysManager.detachMainPlayer(player);
    player = new UltraPlayerBGMusic(0, 0, 0, 5);
    if(!player) {
        return -1;
    }
    player->play(5);
    sysManager.attachMainPlayer(player);
    player->unref();
#endif
    return 0;
}

int
BrowserPlayer::close()
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();

    BROWSER_LOG("Player close !\n");
    if(player && player->magicNumber() == mPlayerMagic) {
        player->close(UltraPlayer::BlackScreenMode);
        sysManager.releaseMainPlayer(player);
        sysManager.detachMainPlayer(player);
#ifdef PLAY_BGMUSIC
        player = new UltraPlayerBGMusic(0, 0, 0, 5);
        if(!player) {
            return -1;
        }
		player->play(5);
        sysManager.attachMainPlayer(player);
        player->unref();
#endif
    } else {
        sysManager.releaseMainPlayer(player);
    }
    return 0;
}

int
BrowserPlayer::setProperty(player_property_type_e aType, HPlayerProperty& aValue)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();

    BROWSER_LOG("SetProperty aType(%s) aValue(%d)\n", getProertyString(aType), aValue.m_Value.m_intVal);
    if(aType != aValue.m_eType) {
        BROWSER_LOG_WARNING("Why not same.aType=%d,m_Type=%d\n", aType, aValue.m_eType);
    }
    switch(aType) {
    case PlayerPropertyType_eSingleOrPlaylistMode: {
        mPlaylistMode = aValue.m_Value.m_intVal;
        break;
    }
    case PlayerPropertyType_eCycleFlag: {
        BROWSER_LOG("Set CycleFlag to %d\n", aValue.m_Value.m_intVal);
        mPlaylistCycleFlag = aValue.m_Value.m_intVal;
        break;
    }
    case PlayerPropertyType_eRandomFlag: {
        mPlaylistRandomFlag = aValue.m_Value.m_intVal;
        break;
    }
    case PlayerPropertyType_eAllowTrickPlayFlag: {
        m_bAllowTrickPlay = aValue.m_Value.m_intVal;
        break;
    }
    case PlayerPropertyType_eVideoDisplayMode: {
        if( 0 == m_VideoArea.m_x && 0 == m_VideoArea.m_y
            && (0 == m_VideoArea.m_w && 0 == m_VideoArea.m_h) || (640 == m_VideoArea.m_w && 530 == m_VideoArea.m_h))
            m_eVideoMode = PlayerVideoMode_eFullScreen;
        else
            m_eVideoMode = (player_video_mode_e)aValue.m_Value.m_intVal;
        BROWSER_LOG("VideoDisplayMode(%d)\n", m_eVideoMode);
        break;
    }
    case PlayerPropertyType_eVideoDisplayArea: {
        BROWSER_LOG("SetProerty VideoDisplayArea x(%d)y(%d)w(%d)h(%d)\n",
                    aValue.m_Value.rect.m_x, aValue.m_Value.rect.m_y, aValue.m_Value.rect.m_w, aValue.m_Value.rect.m_h);
        m_VideoArea = aValue.m_Value.rect;
        if(0 == m_VideoArea.m_x && 0 == m_VideoArea.m_y
            && (0 == m_VideoArea.m_w && 0 == m_VideoArea.m_h)
                || (640 == m_VideoArea.m_w && 530 == m_VideoArea.m_h)) {
            m_eVideoMode = PlayerVideoMode_eFullScreen;
        } else {
            if(PlayerVideoMode_eFullScreen == m_eVideoMode)
                m_eVideoMode = PlayerVideoMode_eVideoByArea;
        }
        BROWSER_LOG("Video area display mode(%d)\n", m_eVideoMode);
        break;
    }
    case PlayerPropertyType_eVideoAlpha: {
        BROWSER_LOG_ERROR("SetProerty PlayerPropertyType_eVideoAlpha unknown !\n");
        break;
    }
    case PlayerPropertyType_eMuteFlag: {
        if(player) {
            player->SetMute(aValue.m_Value.m_intVal);
        } else {
            appSettingSetInt("mute", aValue.m_Value.m_intVal);
        }
        BROWSER_LOG("Mute flag %d\n", aValue.m_Value.m_intVal);
        break;
    }
    case PlayerPropertyType_eAudioVolume: {
        if(player) {
            player->SetVolume(aValue.m_Value.m_intVal);
        }
        break;
    }
    case PlayerPropertyType_eAudioChannel: {            /*mid_audio.c 0：stereo\1：left channel\2: right channel\3: mix*/
        if(player) {
            player->SetChannel(player->GetChannel() + 1);
        }
        break;
    }
    case PlayerPropertyType_eCurrentAudioChannel: {
        if(player) {
            player->SetChannel(aValue.m_Value.m_intVal);
        }
        break;
    }
    case PlayerPropertyType_eAudioTrackPID: {
        if(player) {
            if(aValue.m_Value.m_intVal == -1)
                player->SwitchAudioTrack();
            else
                player->SetTrack(aValue.m_Value.m_intVal);
        }
        break;
    }
    case PlayerPropertyType_eSubtitlePID: {
        if (player) {
            if (aValue.m_Value.m_intVal == -1)
                player->SwitchSubtitle();
            else
                player->SelectSubtitle(aValue.m_Value.m_intVal);
        }
        break;
    }
    case PlayerPropertyType_eNativeUIFlag: { // All guangdong EPG use this flag control ProgressBar and PlayState icon.
        if(aValue.m_Value.m_intVal){
            UltraPlayer::enableUI(true);
            UltraPlayer::setUIFlags(UltraPlayer::PlayState_Mask);
#ifdef GUANGDONG
            UltraPlayer::setUIFlags(UltraPlayer::ProgressBar_Mask);
#endif // GUANGDONG
        } else {
            UltraPlayer::enableUI(false);
            UltraPlayer::clearUIFlags(UltraPlayer::PlayState_Mask);
#ifdef GUANGDONG
            UltraPlayer::clearUIFlags(UltraPlayer::ProgressBar_Mask);
#endif // GUANGDONG
        }
        break;
    }
    case PlayerPropertyType_eChnlNoUIFlag: {
        if(aValue.m_Value.m_intVal)
            UltraPlayer::setUIFlags(UltraPlayer::ChannelNO_Mask);
        else
            UltraPlayer::clearUIFlags(UltraPlayer::ChannelNO_Mask);
        break;
    }
    case PlayerPropertyType_eProgressBarUIFlag: { // C57 specification in order to avoid the STB broadcast control icon in the top right corner and EPG conflicts may occur, STB broadcast control icon in the top right corner and the ProgressBar showed consistent, namely when the ProgressBar is set-top box drawing, will be painting broadcast control icon in the top right; If the ProgressBar is EPG, set-top box is not in the top right hand corner picture broadcast control ICONS.
        if(aValue.m_Value.m_intVal) {
            UltraPlayer::setUIFlags(UltraPlayer::ProgressBar_Mask);
            UltraPlayer::setUIFlags(UltraPlayer::PlayState_Mask);
        }
        else {
            UltraPlayer::clearUIFlags(UltraPlayer::ProgressBar_Mask);
            UltraPlayer::clearUIFlags(UltraPlayer::PlayState_Mask);
#ifdef HEILONGJIANG_SD //和现网高清保持一致
            UltraPlayer::setUIFlags(UltraPlayer::PlayState_Mask);
#endif
        }
        break;
    }
    case PlayerPropertyType_eMuteUIFlag: {
        if(aValue.m_Value.m_intVal)
            UltraPlayer::setUIFlags(UltraPlayer::AudioMute_Mask);
        else
            UltraPlayer::clearUIFlags(UltraPlayer::AudioMute_Mask);
        break;
    }
    case PlayerPropertyType_eAudioTrackUIFlag: {
        if(aValue.m_Value.m_intVal)
            UltraPlayer::setUIFlags(UltraPlayer::AudioTrack_Mask);
        else
            UltraPlayer::clearUIFlags(UltraPlayer::AudioTrack_Mask);
        break;
    }
    case PlayerPropertyType_eAudioVolumeUIFlag: {
        if(aValue.m_Value.m_intVal)
            UltraPlayer::setUIFlags(UltraPlayer::AudioVolume_Mask);
        else
            UltraPlayer::clearUIFlags(UltraPlayer::AudioVolume_Mask);
        break;
    }
    case PlayerPropertyType_eSubtitleFlag: {
        if(player) {
            player->SetSubtitileFlag(aValue.m_Value.m_intVal);
        } else {
            codec_subtitle_show_set(aValue.m_Value.m_intVal);
        }
        break;
    }
    default: {
        BROWSER_LOG_ERROR("SetProperty unknown Type.aType=%s\n", getProertyString(aType));
        break;
    }
    }
    sysManager.releaseMainPlayer(player);
    return 0;
}

int
BrowserPlayer::getProperty(player_property_type_e aType, HPlayerProperty& aResult)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();

    BROWSER_LOG("GetProperty aType=%s\n", getProertyString(aType));
    switch(aType) {
    case PlayerPropertyType_eSingleOrPlaylistMode: {
        aResult.m_Value.m_intVal = mPlaylistMode;
        break;
    }
    case PlayerPropertyType_eCycleFlag: {
        aResult.m_Value.m_intVal = mPlaylistCycleFlag;
        BROWSER_LOG("Get CycleFlag to %d\n", aResult.m_Value.m_intVal);
        break;
    }
    case PlayerPropertyType_eRandomFlag: {
        aResult.m_Value.m_intVal = mPlaylistRandomFlag;
        break;
    }
    case PlayerPropertyType_eAllowTrickPlayFlag: {
        aResult.m_Value.m_intVal = m_bAllowTrickPlay;
        break;
    }
    case PlayerPropertyType_eVideoDisplayMode: {
        aResult.m_Value.m_intVal = m_eVideoMode;
        BROWSER_LOG("PlayerPropertyType_eVideoDisplayMode %d\n", m_eVideoMode);
        break;
    }
    case PlayerPropertyType_eVideoDisplayArea: {
        aResult.m_Value.rect = m_VideoArea;
        BROWSER_LOG("Get VideoDisplayArea x(%d)y(%d)w(%d)h(%d)\n",
                    m_VideoArea.m_x, m_VideoArea.m_y, m_VideoArea.m_w, m_VideoArea.m_h);
        break;
    }
    case PlayerPropertyType_eVideoAlpha: {
        BROWSER_LOG("GetProerty PlayerPropertyType_eVideoAlpha unknown !\n");
        break;
    }
    case PlayerPropertyType_eCurrentPlayTime: {
        GetCurTime(aResult.m_PlayTime.m_strTimeStamp);
        aResult.m_PlayTime.m_TimeFormat = TimeType_eNPT;
        BROWSER_LOG("PlayerPropertyType_eCurrentPlayTime %s\n", aResult.m_PlayTime.m_strTimeStamp.c_str());
        break;
    }
    case PlayerPropertyType_eCurrentMediaDuration: {
        aResult.m_PlayTime.m_strTimeStamp = GetDuration();
        aResult.m_PlayTime.m_TimeFormat = TimeType_eNPT;
        break;
    }
    case PlayerPropertyType_eCurrentMediaCode: {
        if(player){
            aResult.m_strVal = player->getMediaCode();
        }
        break;
    }
    case PlayerPropertyType_eMuteFlag: {
        if(player) {
            aResult.m_Value.m_intVal = player->GetMute();
        } else {
            appSettingGetInt("mute", &aResult.m_Value.m_intVal, 0);
        }
        BROWSER_LOG("Mute flag(%d)\n", aResult.m_Value.m_intVal);
        break;
    }
    case PlayerPropertyType_eAudioVolume: {
        if(player) {
            aResult.m_Value.m_intVal = player->GetVolume();
        }
        break;
    }
    case PlayerPropertyType_eAudioChannel: {
        if(player) {
            aResult.m_Value.m_intVal = player->GetChannel();
        }
        break;
    }
    case PlayerPropertyType_eCurrentAudioChannel: {
        if(player){
            player->GetCurrentAudioChannel(aResult.m_strVal);
        }
        break;
    }
    case PlayerPropertyType_eAudioTrackPID: {
        if(player) {
            aResult.m_Value.m_intVal = player->AudioTrackPid();
        }
        break;
    }
    case PlayerPropertyType_eCurrentAudioTrack: {
        if(player) {
            char tAudioLanguage[8] = {0};

            player->GetTrackInfo(tAudioLanguage);
            aResult.m_strVal = tAudioLanguage;
        }
        break;
    }
    case PlayerPropertyType_eSubtitlePID:{
        if(player) {
            int CurSubtitleIndex = 0;
            unsigned short CurSubtitlePid = 0;

            codec_subtitle_get(&CurSubtitleIndex);
            codec_subtitle_pid(CurSubtitleIndex, &CurSubtitlePid);
            aResult.m_Value.m_intVal = CurSubtitlePid;
        }
        break;
    }
    case PlayerPropertyType_eCurrentSubtitle: {
        if(player) {
            char tSubLanguage[8] = {0};

            codec_subtitle_get(&(aResult.m_Value.m_intVal));
            codec_subtitle_lang(aResult.m_Value.m_intVal, tSubLanguage);
            aResult.m_strVal = tSubLanguage;
        }
        break;
    }
    case PlayerPropertyType_eCurrentPlayBackMode: {
        if(player) {
            player->GetPlayBackMode(aResult.m_strVal);
        }
        BROWSER_LOG("PlayerPropertyType_eCurrentPlayBackMode %s\n", aResult.m_strVal.c_str());
        break;
    }
    case PlayerPropertyType_eNativeUIFlag: {
        aResult.m_Value.m_intVal = UltraPlayer::UIIsEnabled();
        break;
    }
    case PlayerPropertyType_eChnlNoUIFlag: {
        aResult.m_Value.m_intVal = UltraPlayer::getUIFlagState(UltraPlayer::ChannelNO_Mask);
        break;
    }
    case PlayerPropertyType_eProgressBarUIFlag: {
        aResult.m_Value.m_intVal = UltraPlayer::getUIFlagState(UltraPlayer::ProgressBar_Mask);
        break;
    }
    case PlayerPropertyType_eMuteUIFlag: {
        aResult.m_Value.m_intVal = UltraPlayer::getUIFlagState(UltraPlayer::AudioMute_Mask);
        break;
    }
    case PlayerPropertyType_eAudioTrackUIFlag: {
        aResult.m_Value.m_intVal = UltraPlayer::getUIFlagState(UltraPlayer::AudioTrack_Mask);
        break;
    }
    case PlayerPropertyType_eAudioVolumeUIFlag: {
        aResult.m_Value.m_intVal = UltraPlayer::getUIFlagState(UltraPlayer::AudioVolume_Mask);
        break;
    }
    case PlayerPropertyType_eSubtitleFlag: {
        if(player) {
            aResult.m_Value.m_intVal = player->GetSubtitileFlag();
        } else {
            aResult.m_Value.m_intVal = codec_subtitle_show_get(0);
        }
        break;
    }
    case PlayerPropertyType_eAudioTrackInfo:{
        if(player){
            player->GetCurrentAudioTrackInfo(aResult.m_strVal);
            BROWSER_LOG("GetProperty CurrentAudioTrackInfo(%s)\n",aResult.m_strVal.c_str());
        }
        break;
    }
    case PlayerPropertyType_eSubtitleInfo:{
        if(player){
            player->GetCurrentSubtitleInfo(aResult.m_strVal);
        }
        break;
    }
    default: {
        BROWSER_LOG_ERROR("GetProperty unknown Type.aType=%s\n", getProertyString(aType));
        break;
    }
    }
    sysManager.releaseMainPlayer(player);
    return 0;
}

const char *
BrowserPlayer::get(const char * ioStr)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer * player = sysManager.obtainMainPlayer();


    mChannelInfo.clear();
    if(player){
		if(!strcmp(ioStr, "getAllTeletextInfo")){
			player->GetAllTeletextInfo(mChannelInfo);
		} else if(!strcmp(ioStr, "getAllAudioTrackInfo")) {
			player->GetAllAudioTrackInfo(mChannelInfo);
		} else if(!strcmp(ioStr, "getAllSubtitleInfo")){
			player->GetAllSubtitleInfo(mChannelInfo);
		} else {
			BROWSER_LOG("Unknown ioStr(%s)\n", ioStr);
		}
	}
    	sysManager.releaseMainPlayer(player);
	return mChannelInfo.c_str();
}

int
BrowserPlayer::setSingleMedia(media_info_type_e, const char *mediaString)
{
    Program *program = 0;

    if(mediaString == 0){
        BROWSER_LOG_VERBOSE("MediaString is NULL !\n");
        return -1;
    }

#if defined(DEBUG_BUILD)
    BROWSER_LOG("SetSingleMedia #%s# !\n", mediaString);
#endif
    program = programParser().parseSingleMedia(mediaString);
    if(program == 0) {
        return -1;
    }
    mProgramToPlay->safeUnref();
    mProgramToPlay = program;
    BROWSER_LOG("Lave !\n");
    return 0;
}

void
BrowserPlayer::clearForRecycle()
{
    mActualPlayer = NULL;
    mPlayerMagic = 0;
    mProgramToPlay->safeUnref();
    mProgramToPlay = 0;
    mMuteFlag = 0;
    mSubtitleFlag = 1;
    mMacrovisionFlag = -1;
    mHDCPFlag = -1;
    mCGSMAFlag = -1;
}

void
BrowserPlayer::onDestroy()
{
    mActualPlayer = NULL;
}

unsigned int
BrowserPlayer::GetDuration(void)
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = NULL;
    unsigned int Duration = 0;

    player = sysManager.obtainMainPlayer();
    if(player && player->magicNumber() == mPlayerMagic) {
        Duration = player->getTotalTime();
    }
    sysManager.releaseMainPlayer(player);
    return Duration;
}

unsigned int
BrowserPlayer::GetCurTime(HString& TimeString)
{
    char timeStamp[128] = {0};
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = NULL;

    player = sysManager.obtainMainPlayer();
    if(player && player->magicNumber() == mPlayerMagic) {
        player->getCurrentTimeString(timeStamp);
        TimeString = timeStamp;
    }
    sysManager.releaseMainPlayer(player);
    return 0;
}

} // namespace Hippo

