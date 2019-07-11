
#include "BrowserAssertions.h"
#include "BrowserPlayerCTC.h"
#include "BrowserPlayerReporterUtility.h"

#include "SystemManager.h"
#include "ProgramParser.h"
#include "ProgramChannel.h"
#include "UltraPlayer.h"
#include "UltraPlayerVod.h"
#include "UltraPlayerVodList.h"
#include "UltraPlayerMultiple.h"
#ifdef PLAY_BGMUSIC
#include "UltraPlayerBGMusic.h"
#endif
#include "UltraPlayerUtility.h"

#include "config.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "mid/mid_time.h"
#include "mid/mid_tools.h"

#include "app_epg_para.h"
#include "mid_sys.h"
#include "codec.h"

#include "sdk/sdk.h"

namespace Hippo {

BrowserPlayerCTC::BrowserPlayerCTC(int id, player_type_e playerInstanceType)
    : BrowserPlayerC10(id, playerInstanceType)
    , mPlayrMode(PlayerMode_eIdle)
{
    mProgramList = new ProgramList();
    BROWSER_LOG("CTC constructed !\n");
}

BrowserPlayerCTC::~BrowserPlayerCTC()
{
    BROWSER_LOG("CTC destructor !\n");
    close();
    mProgramList->clearAndRelease();
    mProgramList->safeUnref();
}

int
BrowserPlayerCTC::joinChannel(int channelNumber)
{
    int id = instanceId();
    SystemManager &sysManager = systemManager();
    Program *program = sysManager.channelList().getProgramByNumberID(channelNumber);

    BROWSER_LOG("CTC joinChannel !\n");
    if(program) {
        UltraPlayer *player = sysManager.obtainFreePlayer(id);
        if(player && mPlayerMagic != player->magicNumber()) {
            player->stop();
        }
        sysManager.releaseFreePlayer(player);
        sysManager.detachFreePlayer(player);
        BrowserPlayerReporter *reporter = BrowserPlayerReporterCreate(program);

        player = UltraPlayerUtility::createPlayerByProgram(this, reporter, program);
        mPlayrMode = PlayerMode_eLive;
        reporter->safeUnref();
        if(!player)
            return -1;
        player->mPlaylistMode = mPlaylistMode;
        player->mPlayCycleFlag = mPlaylistCycleFlag;
        player->mPlayRandomFlag = mPlaylistRandomFlag;
        player->mDisplayMode = m_eVideoMode;
        mPlayerMagic = player->magicNumber();
        if(!player->isFake()) {
            BROWSER_LOG("BrowserPlayerCTC::joinChannel attachFreePlayer id(%d)\n", id);
            sysManager.attachFreePlayer(player, id);
        }
        player->play(0);
        player->unref();
        //UltraPlayerMultiple *multiplePlayer = static_cast<UltraPlayerMultiple *>(player);
        return 0;
    } else {
        return -1;
    }
}

int
BrowserPlayerCTC::leaveChannel()
{
    int id = instanceId();
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainFreePlayer(id);

	int changeVideoMode = 0;
    sysSettingGetInt("changevideomode", &changeVideoMode, 0);
    BROWSER_LOG("CTC leaveChannel mode %d VideoClearFlag %d!\n", changeVideoMode, UltraPlayer::getVideoClearFlag());
    if(player && player->magicNumber() == mPlayerMagic) {
        UltraPlayerMultiple *multiplePlayer = static_cast<UltraPlayerMultiple *>(player);
        mPlayrMode = PlayerMode_eIdle;

        if(!changeVideoMode) {           //0 is black screen 1 is last frame 2 is smooth change
            multiplePlayer->close(UltraPlayer::BlackScreenMode);
        } else {
            if(UltraPlayer::getVideoClearFlag()) {
                multiplePlayer->close(UltraPlayer::BlackScreenMode);
                UltraPlayer::setVideoClearFlag(0);
            } else {
                multiplePlayer->close(UltraPlayer::LastFrameMode);
            }
        }
        sysManager.releaseFreePlayer(player);
        sysManager.detachFreePlayer(player);
#ifdef PLAY_BGMUSIC
        player = new UltraPlayerBGMusic(0, 0, 0, 5);
        mPlayrMode = PlayerMode_eBGMusic;
        if(!player)
            return -1;
		BROWSER_LOG("BrowserPlayerCTC::leaveChannel attachFreePlayer id(%d)\n", id);
        sysManager.attachFreePlayer(player, id);
		player->play(5);
        player->unref();
#endif
    } else
        sysManager.releaseFreePlayer(player);
    return 0;
}

int
BrowserPlayerCTC::play(int startTime, time_type_e timeType)
{
    int id = instanceId();
    SystemManager &sysManager = systemManager();

    BROWSER_LOG("CTC play timeType %d, startTime %d\n", (int)timeType, startTime);
    if(mProgramToPlay) {
        BROWSER_LOG("CTC play instanceId %d\n", id);
        UltraPlayer *player = sysManager.obtainFreePlayer(id);
        if(player) {
            player->stop();
            sysManager.releaseFreePlayer(player);
            sysManager.detachFreePlayer(player);
        }
        BrowserPlayerReporter *reporter = BrowserPlayerReporterCreate(mProgramToPlay);

        player = UltraPlayerUtility::createPlayerByProgram(this, reporter, mProgramToPlay);
        mPlayrMode = PlayerMode_eVod;
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
        if(!player->isFake()) {
            BROWSER_LOG("BrowserPlayerCTC::play attachFreePlayer id(%d)\n", id);
            sysManager.attachFreePlayer(player, id);
        }
        player->play(startTime);
        player->unref();
    } else {
        UltraPlayer *player = sysManager.obtainFreePlayer(id);
        if(player && player->magicNumber() == mPlayerMagic) {
            player->play(startTime);
        }
        sysManager.releaseFreePlayer(player);
    }
    return 0;
}

int
BrowserPlayerCTC::fastForward(int speed, unsigned long playTime, time_type_e timeType)
{
    int id = instanceId();
    SystemManager &sysManager = systemManager();

    BROWSER_LOG("CTC %dx fast forward !\n", speed);
    UltraPlayer *player = sysManager.obtainFreePlayer(id);
    if(player && player->magicNumber() == mPlayerMagic) {
        BROWSER_LOG("Have player instance and call fastForward !\n");
        player->fastForward(speed);
    }
    sysManager.releaseFreePlayer(player);
    BROWSER_LOG("Player fastForward release player instance!\n");
    return 0;
}

int
BrowserPlayerCTC::fastRewind(int speed, unsigned long playTime, time_type_e timeType)
{
    int id = instanceId();
    SystemManager &sysManager = systemManager();

    BROWSER_LOG("CTC %dx fast rewind !\n", speed);
    UltraPlayer *player = sysManager.obtainFreePlayer(id);
    if(player && player->magicNumber() == mPlayerMagic) {
        BROWSER_LOG("Have player instance and call fastRewind !\n");
        player->fastRewind(speed);
    }
    sysManager.releaseFreePlayer(player);
    BROWSER_LOG("Player fastRewind release player instance!\n");
    return 0;
}

int
BrowserPlayerCTC::seekTo(unsigned long playTime, time_type_e timeType)
{
    int id = instanceId();
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainFreePlayer(id);

    BROWSER_LOG("CTC Player seek to by play time(long %ld) and time type %d!\n", playTime, (int)timeType);
    if(mProgramToPlay) {
        if(player) {
            player->stop();
        }
        sysManager.releaseFreePlayer(player);
        sysManager.detachFreePlayer(player);
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
        if(!player->isFake()) {
            BROWSER_LOG("BrowserPlayerCTC::seekTo attachFreePlayer id(%d)\n", id);
            sysManager.attachFreePlayer(player, id);
        }
        player->play(playTime);
        player->unref();
    } else {
        if(player && player->magicNumber() == mPlayerMagic) {
            switch(timeType) {
            case Player::TimeType_eMediaStart: {
                playTime = 0;
                break;
            }
            case Player::TimeType_eMediaEnd: {
                playTime = player->getTotalTime();
                break;
            }
            default: {
                break;
            }
            }
        }
        player->seekTo(playTime);
        sysManager.releaseFreePlayer(player);
    }
    return 0;
}

int
BrowserPlayerCTC::seekTo(const char *playTime, time_type_e timeType)
{
    int id = instanceId();
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainFreePlayer(id);
    int seekTotime = 0;

    BROWSER_LOG("Player seek to by play time(char %s) and time type %d!\n", playTime, (int)timeType);
    if(mProgramToPlay) {
        if(player) {
            player->stop();
        }
        sysManager.releaseFreePlayer(player);
        sysManager.detachFreePlayer(player);
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
        if(!player->isFake()) {
            BROWSER_LOG("BrowserPlayerCTC::seekto attachFreePlayer id(%d)\n", id);
            sysManager.attachFreePlayer(player, id);
        }
        if(timeType == Player::TimeType_eNPT) {
            player->play(atoi((char *)playTime));
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
                seekTotime = player->getTotalTime();
                break;
            }
            case Player::TimeType_eNPT: {
                BROWSER_LOG("Unsupported TimeType_eNPT time:%s \n", playTime);
                seekTotime = atoi((char *)playTime);
                break;
            }
            case Player::TimeType_eUTC: {
                seekTotime =  mid_tool_string2time((char *)playTime);
                break;
            }
            default: {
                seekTotime = 0;
                break;
            }
            }
            player->seekTo(seekTotime);
        }
        sysManager.releaseFreePlayer(player);
    }
    return 0;
}

int
BrowserPlayerCTC::pause()
{
    int id = instanceId();
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainFreePlayer(id);

    BROWSER_LOG("CTC pause !\n");
    if(player && player->magicNumber() == mPlayerMagic) {
        player->pause();
    }
    sysManager.releaseFreePlayer(player);
    return 0;
}

int
BrowserPlayerCTC::resume()
{
    int id = instanceId();
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainFreePlayer(id);

    BROWSER_LOG("CTC resume !\n");
    if(player && player->magicNumber() == mPlayerMagic) {
        player->resume();
    }
    sysManager.releaseFreePlayer(player);
    return 0;
}

int
BrowserPlayerCTC::stop()
{
    if(mPlayrMode == PlayerMode_eLive)
        return 0;
    int id = instanceId();
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainFreePlayer(id);

    BROWSER_LOG("CTC stop !\n");
    if(player && player->magicNumber() == mPlayerMagic) {
        player->stop();
    }
    sysManager.releaseFreePlayer(player);
    return 0;
}

int
BrowserPlayerCTC::close()
{
    int id = instanceId();
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainFreePlayer(id);

    BROWSER_LOG("CTC close !\n");
    if(player && player->magicNumber() == mPlayerMagic) {
        player->close(UltraPlayer::BlackScreenMode);
        sysManager.releaseFreePlayer(player);
        sysManager.detachFreePlayer(player);
#ifdef PLAY_BGMUSIC
        player = new UltraPlayerBGMusic(0, 0, 0, 5);
        mPlayrMode = PlayerMode_eBGMusic;
        if(!player) {
            return -1;
        }
		BROWSER_LOG("BrowserPlayerCTC::close attachFreePlayer id(%d)\n", id);
        sysManager.attachFreePlayer(player, id);
		player->play(5);
        player->unref();
#endif
    } else {
        sysManager.releaseFreePlayer(player);
    }
    return 0;
}

int
BrowserPlayerCTC::setProperty(player_property_type_e aType, HPlayerProperty& aValue)
{
    int id = instanceId();
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainFreePlayer(id);

    if(aType != aValue.m_eType) {
        BROWSER_LOG_WARNING("BrowserPlayerCTC::setProperty Why not same.aType=%d,m_Type=%d\n", aType, aValue.m_eType);
    }
    switch(aType) {
    case PlayerPropertyType_eVideoAlpha: { // 电信3.0规范，0是不透明，100是全透。
        unsigned char videoAlpha = 0xff * (100 - aValue.m_Value.m_intVal) / 100;
        HYW_vout_setVideoAlpha(0, videoAlpha);
        break;
    }
    case PlayerPropertyType_eMuteFlag: {
        if(player) {
            player->SetMute(aValue.m_Value.m_intVal);
        } else {
            appSettingSetInt("mute", aValue.m_Value.m_intVal);
        }
        BROWSER_LOG("CTC::setProperty mute %d\n", aValue.m_Value.m_intVal);
        break;
    }
    case PlayerPropertyType_eAudioVolume: {
        if(player) {
            player->SetVolume(aValue.m_Value.m_intVal);
        } else { //由于上海EPG会在播放等之前操作音量，此时没有player。
            appSettingSetInt("volume", aValue.m_Value.m_intVal);
        }
        break;
    }
    case PlayerPropertyType_eAudioChannel:{ /*mid_audio.c 0：stereo\1：left channel\2: right channel\3: mix*/
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
            if(aValue.m_Value.m_intVal == -1) {
                int tAudioNum = 0;

                codec_audio_track_num(&tAudioNum);
                codec_audio_track_get(&(aValue.m_Value.m_intVal));
                aValue.m_Value.m_intVal += 1;
                if(aValue.m_Value.m_intVal >= tAudioNum) {
                    aValue.m_Value.m_intVal = 0;
                }
            }
            codec_audio_track_set(aValue.m_Value.m_intVal);
//          player->SetTrack(aValue.m_Value.m_intVal);
        }
        break;
    }
    case PlayerPropertyType_eSubtitlePID:{
        if(player) {
            if(aValue.m_Value.m_intVal == -1) {
                int tSubtitleNum = 0;

                codec_subtitle_num(&tSubtitleNum);
                codec_subtitle_get(&(aValue.m_Value.m_intVal));
                aValue.m_Value.m_intVal += 1;
                if(aValue.m_Value.m_intVal >= tSubtitleNum) {
                    aValue.m_Value.m_intVal = 0;
                }
            }
            codec_subtitle_set(aValue.m_Value.m_intVal);
        }
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
        BrowserPlayerC10::setProperty(aType, aValue);
        break;
    }
    }
    sysManager.releaseFreePlayer(player);
    return 0;
}

int
BrowserPlayerCTC::getProperty(player_property_type_e aType, HPlayerProperty& aResult)
{
    int id = instanceId();
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainFreePlayer(id);

    BROWSER_LOG("BrowserPlayerCTC::getProperty aType=%s\n", getProertyString(aType));
    switch(aType) {
    case PlayerPropertyType_eVideoAlpha: {
        unsigned char videoAlpha = 0;
        HYW_vout_getVideoAlpha(0, &videoAlpha);
        aResult.m_Value.m_intVal = 100 - (videoAlpha / 0xff * 100);
        break;
    }
    case PlayerPropertyType_eMuteFlag: {
        if(player) {
            aResult.m_Value.m_intVal = player->GetMute();
        } else {
            appSettingGetInt("mute", &aResult.m_Value.m_intVal, 0);
        }
        BROWSER_LOG("CTC::getProperty mute %d\n", aResult.m_Value.m_intVal);
        break;
    }
    case PlayerPropertyType_eAudioVolume: {
        if(player) {
            aResult.m_Value.m_intVal = player->GetVolume();
        } else { //由于上海EPG会在播放等之前操作音量，此时没有player。
            appSettingGetInt("volume", &aResult.m_Value.m_intVal, 0);
        }
        break;
    }
    case PlayerPropertyType_eAudioChannel: /*mid_audio.c 0：stereo\1：left channel\2: right channel\3: mix*/
    case PlayerPropertyType_eCurrentAudioChannel: {
        if(player) {
            aResult.m_Value.m_intVal = player->GetChannel();
            switch(aResult.m_Value.m_intVal) {
            case 0: {
                aResult.m_strVal = "Stereo";
                break;
            }
            case 1: {
                aResult.m_strVal = "Left";
                break;
            }
            case 2: {
                aResult.m_strVal = "Right";
                break;
            }
            case 3: {
                aResult.m_strVal = "Mix";
                break;
            }
            }
        }
        break;
    }
    case PlayerPropertyType_eAudioTrackPID: {
        if(player) {
//            aResult.m_Value.m_intVal = player->GetTrack();
            codec_audio_track_get(&(aResult.m_Value.m_intVal));
        }
        break;
    }
    case PlayerPropertyType_eCurrentAudioTrack: {
        if(player) {
            char tAudioLanguage[8] = {0};

            codec_audio_track_get(&(aResult.m_Value.m_intVal));
            codec_audio_track_get_info(aResult.m_Value.m_intVal, tAudioLanguage);
            aResult.m_strVal = tAudioLanguage;
        }
        break;
    }
    case PlayerPropertyType_eAudioTrackPIDs: {
        if(player) {
            char *tReturnStr = 0;
            int tAudioNum = 0;
            int tIndex = 0;

            codec_audio_track_num(&tAudioNum);
            if(tAudioNum <= 0) {
                break;
            }
            tReturnStr = (char *)malloc(64 * tAudioNum);
            if(tReturnStr == 0)
                break;
            memset(tReturnStr, 0, 64 * tAudioNum);
            sprintf(tReturnStr,"{\"Ipids\":\"%d\",\"Ipidaddr\":\"[", tAudioNum);
            for(tIndex = 0; tIndex < tAudioNum; tIndex++) {
                int tPID = 0;
                char tLanguage[8] = {0};
                char tBuff[64] = {0};

                codec_audio_track_get_pid(tIndex, &tPID);
                codec_subtitle_lang(tIndex, tLanguage);
                sprintf(tBuff, "{\"AudioPID\":\"%d\",\"AudioLanguage\":\"%s\"}", tPID, tLanguage);
                strcat(tReturnStr, tBuff);
                if(tIndex < tAudioNum - 1) {
                    strcat(tReturnStr, ",");
                }
            }
            strcat(tReturnStr, "]\"}");
            aResult.m_strVal = tReturnStr;
            if(tReturnStr)
                free(tReturnStr);
        }
        break;
    }
    case PlayerPropertyType_eSubtitlePID:{
        if(player) {
            codec_subtitle_get(&(aResult.m_Value.m_intVal));
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
    case PlayerPropertyType_eSubtitlePIDs:{
        if(player) {
            char *tReturnStr = 0;
            int tSubtitleNum = 0;
            int tIndex = 0;

            codec_subtitle_num(&tSubtitleNum);
            if(tSubtitleNum <= 0) {
                break;
            }
            tReturnStr = (char *)malloc(64 * tSubtitleNum);
            if(tReturnStr == 0)
                break;
            memset(tReturnStr, 0, 64 * tSubtitleNum);
            sprintf(tReturnStr,"{\"Ipids\":\"%d\",\"Ipidaddr\":\"[", tSubtitleNum);
            for(tIndex = 0; tIndex < tSubtitleNum; tIndex++) {
                unsigned short tPID = 0;
                char tLanguage[8] = {0};
                char tBuff[64] = {0};

                codec_subtitle_pid(tIndex, &tPID);
                codec_subtitle_lang(tIndex, tLanguage);
                sprintf(tBuff, "{\"SubtitlePID\":\"%d\",\"SubtitleLanguage\":\"%s\"}", tPID, tLanguage);
                strcat(tReturnStr, tBuff);
                if(tIndex < tSubtitleNum - 1) {
                    strcat(tReturnStr, ",");
                }
            }
            strcat(tReturnStr, "]\"}");
            aResult.m_strVal = tReturnStr;
            if(tReturnStr)
                free(tReturnStr);
        }
        break;
    }
    case PlayerPropertyType_eCurrentPlayBackMode: {
        if(player) {
            player->GetPlayBackMode(aResult.m_strVal);
        }
        break;
    }
    case PlayerPropertyType_eCurrentPlayTime: {
        if(player && player->magicNumber() == mPlayerMagic) {
            char strTimeStamp[128] = {0};

            player->getCurrentTimeString(strTimeStamp);
            aResult.m_PlayTime.m_strTimeStamp = strTimeStamp;
            aResult.m_PlayTime.m_TimeFormat = TimeType_eNPT;
        }
        break;
    }
    case PlayerPropertyType_eCurrentMediaDuration: {
        if(player && player->magicNumber() == mPlayerMagic) {
            aResult.m_PlayTime.m_strTimeStamp = player->getTotalTime();
            aResult.m_PlayTime.m_TimeFormat = TimeType_eNPT;
        }
        break;
    }
    case PlayerPropertyType_eCurrentMediaCode: {
        if(player && player->magicNumber() == mPlayerMagic) {
            aResult.m_strVal = player->getMediaCode();
        }
        break;
    }
    case PlayerPropertyType_eMediaCount: {
        if(this) {
            aResult.m_Value.m_intVal = this->getMediaCount();
        }
        break;
    }
    case PlayerPropertyType_eCurrentMediaIdx: {
        if(this) {
            aResult.m_Value.m_intVal = this->getCurrentIndex();
        }
        break;
    }
    case PlayerPropertyType_eCurrentMediaEntryId: {
        if(this) {
            aResult.m_strVal = this->getCurrentEntryID();
        }
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
    case PlayerPropertyType_eMediaPlaylist:{
        int tIndex = 0;
        int tProgramCount = mProgramList->getProgramCount();
        char *tPlaylist = 0;
        char *mPlaylist = 0;

        mPlaylist = (char *)malloc(1024 * tProgramCount);
        if(!mPlaylist)
            return -1;
        strcpy(mPlaylist, "[");
        tPlaylist = (char *)malloc(1024);
        if(!tPlaylist){
            free(mPlaylist);
            mPlaylist = 0;
            return -1;
        }
        for(tIndex = 0; tIndex < tProgramCount; tIndex ++){
            Program *tProgram = mProgramList->getProgramByIndex(tIndex);
            VodSource *tVodsource = tProgram->getVodSource(0);

            memset(tPlaylist, 0, 1024);
            sprintf(tPlaylist,
                    "{mediaUrl:\"%s\",mediaCode: \"%s\",mediaType:%d,audioType:%d,videoType:%d,streamType:%d,drmType:%d,fingerPrint:%d,copyProtection:%d,allowTrickmode:%d,startTime:%s,endTime:%s,entryID:\"%s\"}",
                    tVodsource->GetUrl().c_str(),
                    tVodsource->GetMediaCode().c_str(),
                    tVodsource->GetMediaType(),
                    tVodsource->GetAudioType(),
                    tVodsource->GetVideoType(),
                    tVodsource->GetStreamType(),
                    tVodsource->GetDrmType(),
                    tVodsource->GetFingerPrint(),
                    tVodsource->GetCopyProtection(),
                    tVodsource->GetAllowTrickmode(),
                    tVodsource->GetStartTime().c_str(),
                    tVodsource->GetEndTime().c_str(),
                    tVodsource->GetEntryID().c_str());
            strcat(mPlaylist, tPlaylist);
            if(tIndex < tProgramCount - 1){
                strcat(mPlaylist, ",");
            }
        }
        strcat(mPlaylist, "]");
        if(tPlaylist)
            free(tPlaylist);
        BROWSER_LOG("\nThe PlayerPropertyType_eMediaPlaylist get :\n");
        BROWSER_LOG("%s\n\n", mPlaylist);
        aResult.m_strVal = mPlaylist;
        if(mPlaylist) {
            free(mPlaylist);
            mPlaylist = 0;
        }
        BROWSER_LOG("%s\n\n", aResult.m_strVal.c_str());
        break;
    }
    default: {
        BrowserPlayerC10::getProperty(aType, aResult);
        break;
    }
    }
    sysManager.releaseFreePlayer(player);
    return 0;
}

int
BrowserPlayerCTC::addSingleMedia(media_info_type_e pType, int pIndex, const char *mediaString)
{
    int id = instanceId();
    Program *tProgram = 0;

    if(pType != Player::MediaInfoType_eJson) {
        return -1;
    }
    BROWSER_LOG("CTC add singlemedia %s!\n", mediaString);
    if(pIndex == 0){ // Hippo glue 将add single media 和 add batch media 都放到了这里处理，因此这么改
        if(0 != programParser().parseMediaList(mProgramList, mediaString))
            return -1;
    }else{
        tProgram = programParser().parseSingleMedia(mediaString);

        if(tProgram == 0)
            return -1;
        mProgramList->addProgramToPosition(tProgram, pIndex);
    }
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainFreePlayer(id);
    if(player && mPlayerMagic != player->magicNumber()) {
        player->stop();
    }
    sysManager.releaseFreePlayer(player);
    sysManager.detachFreePlayer(player);
    Program *program = mProgramList->getProgramByIndex(pIndex);
    if(!program){
        return -1;
    }
    BrowserPlayerReporter *reporter = BrowserPlayerReporterCreate(program);

    player = new UltraPlayerVodList(this, reporter, mProgramList, pIndex);
    reporter->safeUnref();
    if(!player) {
        return -1;
    }
    player->mPlaylistMode = mPlaylistMode;
    player->mPlayCycleFlag = mPlaylistCycleFlag;
    player->mPlayRandomFlag = mPlaylistRandomFlag;
    player->mDisplayMode = m_eVideoMode;
    if(!player->isFake()) {
        BROWSER_LOG("BrowserPlayerCTC::addSingleMedia attachFreePlayer id(%d)\n", id);
        sysManager.attachFreePlayer(player, id);
    }
    mPlayerMagic = player->magicNumber();
    player->unref();
    return 0;
}

int
BrowserPlayerCTC::removePlayNode(playlist_op_type_e pType, HPlaylistProperty& aValue)
{
    BROWSER_LOG("CTC removePlayNode %p\n", mProgramList);
    switch(pType){
    case PlaylistOpType_eIdxRemoveAll:{
        BROWSER_LOG("CTC remove all media !\n");
        mProgramList->clearAndRelease();
        break;
    }
    case PlaylistOpType_eIdxRemoveByOffset:{
        break;
    }
    case PlaylistOpType_eIdxRemoveByEntryId:{
        BROWSER_LOG("CTC remove media %s !\n", aValue.m_oldStrVal.c_str());
        mProgramList->removeProgramByStringID(aValue.m_oldStrVal.c_str());
        break;
    }
    case PlaylistOpType_eIdxRemoveByIndex:{
        BROWSER_LOG("CTC remove media %d !\n", aValue.m_oldValue.m_intVal);
        mProgramList->removeProgramByIndex(aValue.m_oldValue.m_intVal);
        break;
    }
    case PlaylistOpType_eUnknown:
    default:{
        return -1;
    }
    }
    return 0;
}

int
BrowserPlayerCTC:: movePlayNode(playlist_op_type_e pType, HPlaylistProperty& aValue)
{
    Program *program = 0;

    switch(pType){
    case PlaylistOpType_eIdxMoveByIndex:{
        BROWSER_LOG("CTC move media(%s) to %d !\n", aValue.m_oldStrVal.c_str(), aValue.m_newValue.m_intVal);
	    program = mProgramList->getProgramByStringID(aValue.m_oldStrVal.c_str());

        if(program != 0) {
            mProgramList->moveToPosition(program, aValue.m_newValue.m_intVal);
        }
        break;
    }
	case PlaylistOpType_eIdxMoveByIndex1:{
	    BROWSER_LOG("CTC move media(%d) to %d !\n", aValue.m_oldValue.m_intVal, aValue.m_newValue.m_intVal);
	    program = mProgramList->getProgramByIndex(aValue.m_oldValue.m_intVal);

        if(program != 0) {
            mProgramList->moveToPosition(program, aValue.m_newValue.m_intVal);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveByOffset:{
        BROWSER_LOG("CTC move media(%s) offset %d !\n", aValue.m_oldStrVal.c_str(), aValue.m_newValue.m_intVal);
	    program = mProgramList->getProgramByStringID(aValue.m_oldStrVal.c_str());
        int toIndex = 0;

        if(program != 0) {
            toIndex = mProgramList->getProgramIndex(program);
            toIndex += aValue.m_newValue.m_intVal;
            mProgramList->moveToPosition(program, toIndex);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveByOffset1:{
        BROWSER_LOG("CTC move media(%d) offset %d !\n", aValue.m_oldValue.m_intVal, aValue.m_newValue.m_intVal);
        program = mProgramList->getProgramByIndex(aValue.m_oldValue.m_intVal);
        int toIndex = aValue.m_oldValue.m_intVal + aValue.m_newValue.m_intVal;

        if(program != 0) {
            mProgramList->moveToPosition(program, toIndex);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToNext:{
        BROWSER_LOG("CTC move media(%s) to next !\n", aValue.m_oldStrVal.c_str());
        program = mProgramList->getProgramByStringID(aValue.m_oldStrVal.c_str());

        if(program != 0) {
            mProgramList->moveToNext(program);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToNext1:{
        BROWSER_LOG("CTC move media(%d) to next !\n", aValue.m_oldValue.m_intVal);
        program = mProgramList->getProgramByIndex(aValue.m_oldValue.m_intVal);

        if(program != 0) {
            mProgramList->moveToNext(program);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToPrevious:{
        BROWSER_LOG("CTC move media(%s) to Previous !\n", aValue.m_oldStrVal.c_str());
        program = mProgramList->getProgramByStringID(aValue.m_oldStrVal.c_str());

        if(program != 0) {
            mProgramList->moveToPrevious(program);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToPrevious1:{
        BROWSER_LOG("CTC move media(%d) to Previous !\n", aValue.m_oldValue.m_intVal);
        program = mProgramList->getProgramByIndex(aValue.m_oldValue.m_intVal);

        if(program != 0) {
            mProgramList->moveToPrevious(program);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToFirst:{
        BROWSER_LOG("CTC move media(%s) to first !\n", aValue.m_oldStrVal.c_str());
        program = mProgramList->getProgramByStringID(aValue.m_oldStrVal.c_str());

        if(program != 0) {
            mProgramList->moveToFirst(program);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToFirst1:{
        BROWSER_LOG("CTC move media(%d) to first !\n", aValue.m_oldValue.m_intVal);
        program = mProgramList->getProgramByIndex(aValue.m_oldValue.m_intVal);

        if(program != 0) {
            mProgramList->moveToFirst(program);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToLast:{
        BROWSER_LOG("CTC move media(%s) to last !\n", aValue.m_oldStrVal.c_str());
        program = mProgramList->getProgramByStringID(aValue.m_oldStrVal.c_str());

        if(program != 0) {
            mProgramList->moveToPrevious(program);
        }
        break;
    }
    case PlaylistOpType_eIdxMoveToLast1:{
        BROWSER_LOG("CTC move media(%d) to last !\n", aValue.m_oldValue.m_intVal);
        program = mProgramList->getProgramByIndex(aValue.m_oldValue.m_intVal);

        if(program != 0) {
            mProgramList->moveToPrevious(program);
        }
        break;
    }
    case PlaylistOpType_eUnknown:
    default:{
        return -1;
    }
    }
    return 0;
}

int
BrowserPlayerCTC::selectPlayNode(playlist_op_type_e pType, HPlaylistProperty& aValue)
{
    int id = instanceId();
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainFreePlayer(id);

    if(player && player->magicNumber() == mPlayerMagic) {
        if(player->mPlaylistMode) {
            switch(pType){
        	case PlaylistOpType_eIdxSelectByIndex:{
                BROWSER_LOG("CTC select media by index %d !\n", aValue.m_oldValue.m_intVal);
                player->PlayByIndex(aValue.m_oldValue.m_intVal);
                break;
            }
        	case PlaylistOpType_eIdxSelectOffset:{
        	    BROWSER_LOG("CTC select media by offset %d !\n", aValue.m_oldValue.m_intVal);
                player->PlayByOffset(aValue.m_oldValue.m_intVal);
                break;
            }
        	case PlaylistOpType_eIdxSelectByEntryId:{
        	    BROWSER_LOG("CTC select media by entryID %s !\n", aValue.m_oldStrVal.c_str());
                player->PlayByEntryId(aValue.m_oldStrVal.c_str());
                break;
            }
        	case PlaylistOpType_eIdxSelectNext:{
        	    BROWSER_LOG("CTC select next media !\n");
                player->PlayNext();
                break;
            }
        	case PlaylistOpType_eIdxSelectPrevious:{
        	    BROWSER_LOG("CTC select previous media !\n");
                player->PlayPrevious();
                break;
            }
        	case PlaylistOpType_eIdxSelectFirst:{
        	    BROWSER_LOG("CTC select first media !\n");
                player->PlayFirst();
                break;
            }
        	case PlaylistOpType_eIdxSelectLast:{
        	    BROWSER_LOG("CTC select last media !\n");
                player->PlayLast();
                break;
            }
            case PlaylistOpType_eUnknown:
            default:{
                break;
            }
            }
        }
    }
    sysManager.releaseFreePlayer(player);
    return 0;
}

int
BrowserPlayerCTC::getMediaCount()
{
    return mProgramList->getProgramCount();
}

int
BrowserPlayerCTC::getCurrentIndex()
{
    int mCurrentIndex = 0;
    int id = instanceId();
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainFreePlayer(id);

    if(player && player->magicNumber() == mPlayerMagic) {
        if(player->mPlaylistMode) {
            mCurrentIndex = player->getVodListCurIndex();
        }
    }
    sysManager.releaseFreePlayer(player);
    return mCurrentIndex;
}

const char*
BrowserPlayerCTC::getCurrentEntryID()
{
    int tCurrentIndex = 0;
    Program *program = NULL;
    int id = instanceId();
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainFreePlayer(id);

    BROWSER_LOG("BrowserPlayerCTC::getCurrentEntryID\n");
    if(player && player->magicNumber() == mPlayerMagic) {
        if(player->mPlaylistMode) {
            tCurrentIndex = player->getVodListCurIndex();
        }
    }
    BROWSER_LOG("BrowserPlayerCTC::getCurrentEntryID current media index is %d\n", tCurrentIndex);
    sysManager.releaseFreePlayer(player);
    program = mProgramList->getProgramByIndex(tCurrentIndex);
    if(program == 0) {
        return 0;
    }
    return program->getStringID();
}

int
BrowserPlayerCTC::getPlaylist()
{
    return 0;
}

int
BrowserPlayerCTC::playFromStart()
{
    if(!mPlaylistMode)
        return -1;
    return 0;
}

void
BrowserPlayerCTC::clearForRecycle()
{
    close();
    mProgramList->clearAndRelease();
    BrowserPlayer::clearForRecycle();
}

extern "C" void mid_plane_browser_positionOffset(int x, int y, int w, int h, int *my_x, int *my_y, int *my_w, int *my_h);

int
BrowserPlayerCTC::refreshVideoDisplay()
{
    int tScreenW = 0, tScreenH = 0;
    HRect tVideoRect = m_VideoArea;

    ygp_layer_getScreenSize(&tScreenW, &tScreenH);
    BROWSER_LOG("CTC refreshVideoDisplay screen w(%d)h(%d)\n", tScreenW, tScreenH);
    BROWSER_LOG("CTC refreshVideoDisplay mode = %d, x = %d, y = %d, w = %d, h = %d\n",
                m_eVideoMode, m_VideoArea.m_x, m_VideoArea.m_y, m_VideoArea.m_w, m_VideoArea.m_h);

    if(PlayerVideoMode_eVideoByArea == m_eVideoMode){ // 按setVideoDisplayArea()中设定的Height, Width, Left, Top属性所指定的位置和大小来显示视频
        if(tScreenW <= m_VideoArea.m_w || tScreenH <= m_VideoArea.m_h){
			mid_stream_rect(0, 0, 0, 0, 0);
		} else {
            //systemManager().mixer().convertToDeviceCoordinates(tVideoRect.m_x, tVideoRect.m_y, tVideoRect.m_w, tVideoRect.m_h);
            mid_plane_browser_positionOffset(m_VideoArea.m_x,
                                             m_VideoArea.m_y,
                                             m_VideoArea.m_w,
                                             m_VideoArea.m_h,
                                             &tVideoRect.m_x,
                                             &tVideoRect.m_y,
                                             &tVideoRect.m_w,
                                             &tVideoRect.m_h);
    		mid_stream_rect(0, tVideoRect.m_x, tVideoRect.m_y, tVideoRect.m_w, tVideoRect.m_h);
    		mid_sys_video_show(1);
		}
	} else if(PlayerVideoMode_eVideoHide == m_eVideoMode) { // 视频显示窗口将被关闭
		mid_sys_video_show(0);
	} else {
	    mid_stream_rect(0, 0, 0, 0, 0);
	    if(PlayerVideoMode_eFullScreen == m_eVideoMode ) { // (FullScreen)全屏显示，按全屏高度和宽度显示(默认值)
            mid_sys_aspectRatioMode_set(2);
    	} else if(PlayerVideoMode_eByWidth == m_eVideoMode) { // (Pan Scan)按宽度显示，指在不改变原有图像纵横比的情况下按全屏宽度显示
    		mid_sys_aspectRatioMode_set(1);
    	} else if(PlayerVideoMode_eByHeight == m_eVideoMode) { // (Letterbox)按高度显示，指在不改变原有图像纵横比的情况下按全屏高度显示
    		mid_sys_aspectRatioMode_set(0);
    	}
    	mid_sys_video_show(1);
    }
	return 0;
}

void *createBrowserPlayerCTC(int id)
{
    return new BrowserPlayerCTC(id, BrowserPlayer::PlayerType_eMain);
}

} // namespace Hippo

