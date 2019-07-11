#include "JseGlueAssertions.h"
#include "IMediaPlayerCell.h"

#include "Hippo_Context.h"

#include <stdio.h>

int
IMediaPlayerCell::createPlayerInstance(const char* name)
{
    int myId = -1;
    MediaPlayerMgr *playerMgr = HippoContext::getContextInstance()->getMediaPlayerMgr();
    if(!playerMgr)
        return -1;
    LogJseGlueDebug("getMediaPlayerMgr ok.\n");

    if(!m_player)
        m_player = playerMgr->createPlayerInstance(myId, name, NULL, NULL);
    if(!m_player)
        return -1;

    LogJseGlueDebug("createPlayerInstance ok.\n");
    return 0;
}

long
IMediaPlayerCell::getNativePlayerInstanceID()
{
    int id = -1;
    if(m_player)
        m_player->getInstanceId(id);

    LogJseGlueDebug("ret=%d\n", id);
    return id;
}

long
IMediaPlayerCell::bindNativePlayerInstance(long id)
{
    LogJseGlueDebug("val=%d\n", id);
    MediaPlayerMgr* playerMgr = HippoContext::getContextInstance()->getMediaPlayerMgr();

    if(m_player) {
        int cid = -1;
        m_player->getInstanceId(cid);
        if(playerMgr)
            playerMgr->releasePlayerInstanceById(cid);
        m_player = NULL;
    }
    m_player = playerMgr->getPlayerInstanceById(id);
    return 0;
}

long
IMediaPlayerCell::HandleEvent()
{
    LogJseGlueError("TODO\n");
    return -1; //TODO
}

void
IMediaPlayerCell::setSingleOrPlaylistMode(long mode)
{
    LogJseGlueTest("val=%ld\n", mode);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eSingleOrPlaylistMode;
    out.m_Value.m_intVal = mode;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eSingleOrPlaylistMode, out);
}

long
IMediaPlayerCell::getSingleOrPlaylistMode()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eSingleOrPlaylistMode;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eSingleOrPlaylistMode, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

void
IMediaPlayerCell::setVideoDisplayMode(long mode)
{
    LogJseGlueDebug("val=%ld\n", mode);
    Player::HPlayerProperty out;

    out.m_Value.m_intVal = mode;
    out.m_eType = Player::PlayerPropertyType_eVideoDisplayMode;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eVideoDisplayMode, out);
}

long
IMediaPlayerCell::getVideoDisplayMode()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eVideoDisplayMode;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eVideoDisplayMode, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

long
IMediaPlayerCell::setVideoDisplayArea(long left, long top, long width, long height)
{
    LogJseGlueDebug("area<%d %d %d %d>\n", left, top, width, height);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eVideoDisplayArea;
    out.m_Value.rect.m_x = left;
    out.m_Value.rect.m_y = top;
    out.m_Value.rect.m_w = width;
    out.m_Value.rect.m_h = height;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eVideoDisplayArea, out);

    return 0;
}

long
IMediaPlayerCell::getVideoDisplayLeft()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eVideoDisplayArea;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eVideoDisplayArea, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.rect.m_x);
    return out.m_Value.rect.m_x;
}

long
IMediaPlayerCell::getVideoDisplayTop()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eVideoDisplayArea;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eVideoDisplayArea, out);

    LogJseGlueDebug(" val=%d\n", out.m_Value.rect.m_y);
    return out.m_Value.rect.m_y;
}

long
IMediaPlayerCell::getVideoDisplayWidth()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eVideoDisplayArea;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eVideoDisplayArea, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.rect.m_w);
    return out.m_Value.rect.m_w;
}

long
IMediaPlayerCell::getVideoDisplayHeight()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eVideoDisplayArea;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eVideoDisplayArea, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.rect.m_h);
    return out.m_Value.rect.m_h;
}

void
IMediaPlayerCell::setMuteFlag(const bool muteFlag)
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eMuteFlag;
    out.m_Value.m_intVal = muteFlag;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eMuteFlag, out);

    LogJseGlueDebug("val=%d\n", muteFlag);
}

int
IMediaPlayerCell::getMuteFlag()
{
    Player::HPlayerProperty out;
    out.m_eType = Player::PlayerPropertyType_eMuteFlag;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eMuteFlag, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

void
IMediaPlayerCell::setNativeUIFlag(bool nativeUIFlag)
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eNativeUIFlag;
    out.m_Value.m_intVal = nativeUIFlag;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eNativeUIFlag, out);

    LogJseGlueDebug("val=%d\n", nativeUIFlag);
}

int
IMediaPlayerCell::getNativeUIFlag()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eNativeUIFlag;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eNativeUIFlag, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

void
IMediaPlayerCell::setMuteUIFlag(bool muteUIFlag)
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eMuteUIFlag;
    out.m_Value.m_intVal = muteUIFlag;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eMuteUIFlag, out);

    LogJseGlueDebug("val=%d\n", muteUIFlag);
}

int
IMediaPlayerCell::getMuteUIFlag()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eMuteUIFlag;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eMuteUIFlag, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

void
IMediaPlayerCell::setAudioVolumeUIFlag(bool audioVolumeUIFlag)
{
    LogJseGlueDebug("val=%d\n", audioVolumeUIFlag);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eAudioVolumeUIFlag;
    out.m_Value.m_intVal = audioVolumeUIFlag;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eAudioVolumeUIFlag, out);
}

int
IMediaPlayerCell::getAudioVolumeUIFlag()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eAudioVolumeUIFlag;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eAudioVolumeUIFlag, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

void
IMediaPlayerCell::setAudioTrackUIFlag(bool audioTrackUIFlag)
{
    LogJseGlueDebug("val=%d\n", audioTrackUIFlag);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eAudioTrackUIFlag;
    out.m_Value.m_intVal = audioTrackUIFlag;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eAudioTrackUIFlag, out);
}

int
IMediaPlayerCell::getAudioTrackUIFlag()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eAudioTrackUIFlag;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eAudioTrackUIFlag, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

void
IMediaPlayerCell::setProgressBarUIFlag(bool progressBarUIFlag)
{
    LogJseGlueDebug("val=%d\n", progressBarUIFlag);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eProgressBarUIFlag;
    out.m_Value.m_intVal = progressBarUIFlag;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eProgressBarUIFlag, out);
}

int
IMediaPlayerCell::getProgressBarUIFlag()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eProgressBarUIFlag;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eProgressBarUIFlag, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

void
IMediaPlayerCell::setChannelNoUIFlag(bool channelNoUIFlag)
{
    LogJseGlueDebug("val=%d\n", channelNoUIFlag);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eChnlNoUIFlag;
    out.m_Value.m_intVal = channelNoUIFlag;
    if(m_player) {
        m_player->setProperty(Player::PlayerPropertyType_eChnlNoUIFlag, out);
    }
}

int
IMediaPlayerCell::getChannelNoUIFlag()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eChnlNoUIFlag;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eChnlNoUIFlag, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

int
IMediaPlayerCell::getSubtitileFlag()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eSubtitleFlag;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eSubtitleFlag, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

void
IMediaPlayerCell::setSubtitileFlag(bool subtitileFlag)
{
    LogJseGlueDebug("val=%d\n", subtitileFlag);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eSubtitleFlag;
    out.m_Value.m_intVal = subtitileFlag;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eSubtitleFlag, out);
}

int
IMediaPlayerCell::getSubtitleFlag()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eSubtitleFlag;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eSubtitleFlag, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

void
IMediaPlayerCell::setSubtitleFlag(bool subtitileFlag)
{
    LogJseGlueDebug("val=%d\n", subtitileFlag);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eSubtitleFlag;
    out.m_Value.m_intVal = subtitileFlag;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eSubtitleFlag, out);
}

int
IMediaPlayerCell::getVideoAlpha()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eVideoAlpha;
    if(!m_player)
        return 0;
    m_player->getProperty(Player::PlayerPropertyType_eVideoAlpha, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

void
IMediaPlayerCell::setVideoAlpha(long videoAlpha)
{
    LogJseGlueDebug("val=%ld\n", videoAlpha);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eVideoAlpha;
    out.m_Value.m_intVal = videoAlpha;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eVideoAlpha, out);
}

int
IMediaPlayerCell::getAllowTrickmodeFlag()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eAllowTrickPlayFlag;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eAllowTrickPlayFlag, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

void
IMediaPlayerCell::setAllowTrickmodeFlag(bool allowTrickmodeFlag)
{
    LogJseGlueDebug("val=%d\n", allowTrickmodeFlag);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eAllowTrickPlayFlag;
    out.m_Value.m_intVal = allowTrickmodeFlag;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eAllowTrickPlayFlag, out);
}

int
IMediaPlayerCell::getCycleFlag()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eCycleFlag;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eCycleFlag, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

void
IMediaPlayerCell::setCycleFlag(bool cycleFlag)
{
    LogJseGlueDebug("val=%d\n", cycleFlag);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eCycleFlag;
    out.m_Value.m_intVal = cycleFlag;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eCycleFlag, out);
}

int
IMediaPlayerCell::getRandomFlag()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eRandomFlag;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eRandomFlag, out);

    LogJseGlueDebug("val=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

void
IMediaPlayerCell::setRandomFlag(bool randomFlag)
{
    LogJseGlueDebug("val=%d\n", randomFlag);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eRandomFlag;
    out.m_Value.m_intVal = randomFlag;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eRandomFlag, out);
}

int
IMediaPlayerCell::getMediaCode(char* mediaCode)
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eCurrentMediaCode;
    if(!m_player)
        return -1;
    m_player->getProperty(Player::PlayerPropertyType_eCurrentMediaCode, out);
    strncpy(mediaCode, out.m_strVal.c_str(), IJSEBUFFSIZE);
    mediaCode[IJSEBUFFSIZE] = '\0';

    LogJseGlueDebug("getMediaCode=%s\n", mediaCode);
    return 0;
}

long
IMediaPlayerCell::getMediaDuration()
{
    Player::HPlayerProperty out;
    std::string jsStr;

    out.m_eType = Player::PlayerPropertyType_eCurrentMediaDuration;
    if (!m_player)
        return -1;
    m_player->getProperty(Player::PlayerPropertyType_eCurrentMediaDuration, out);
    jsStr += out.m_PlayTime.m_strTimeStamp.c_str();
    if(jsStr.empty())
        jsStr += "0";

    LogJseGlueDebug("val=%s\n", jsStr.c_str());
    return atoi(jsStr.c_str());
}

int
IMediaPlayerCell::getCurrentPlayTime(char* currentPlayTime)
{
    Player::HPlayerProperty out;
    std::string jsStr;

    out.m_eType = Player::PlayerPropertyType_eCurrentPlayTime;
    if (!m_player)
        return -1;
    m_player->getProperty(Player::PlayerPropertyType_eCurrentPlayTime, out);
    jsStr += out.m_PlayTime.m_strTimeStamp.c_str();
    strncpy(currentPlayTime, jsStr.c_str(), IJSEBUFFSIZE);
    currentPlayTime[IJSEBUFFSIZE] = '\0';

    LogJseGlueDebug("val=%s\n", currentPlayTime);
    return 0;
}

int
IMediaPlayerCell::getPlaybackMode(char* playbackMode)
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eCurrentPlayBackMode;
    if (!m_player)
        return -1;
    m_player->getProperty(Player::PlayerPropertyType_eCurrentPlayBackMode, out);
    strncpy(playbackMode, out.m_strVal.c_str(), IJSEBUFFSIZE);
    playbackMode[IJSEBUFFSIZE] = '\0';

    LogJseGlueDebug("CurrentEntryID=%s\n", playbackMode);
    return 0;
}

long
IMediaPlayerCell::getChannelNum()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eCurrentPlayChnlNum;
    if (!m_player)
        return -1;

    m_player->getProperty(Player::PlayerPropertyType_eCurrentPlayChnlNum, out);
    LogJseGlueDebug("mediaCount=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

int
IMediaPlayerCell::getCurrentAudioChannel(char* currentAudioChannel)
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eCurrentAudioChannel;
    if(!m_player)
        return -1;
    m_player->getProperty(Player::PlayerPropertyType_eCurrentAudioChannel, out);
    strncpy(currentAudioChannel, out.m_strVal.c_str(), IJSEBUFFSIZE);
    currentAudioChannel[IJSEBUFFSIZE] = '\0';

    LogJseGlueDebug("Channel=%s\n", currentAudioChannel);
    return 0;
}

int
IMediaPlayerCell::getAudioTrack(char* audioTrack)
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eCurrentAudioTrack;
    if(!m_player)
        return -1;
    m_player->getProperty(Player::PlayerPropertyType_eCurrentAudioTrack, out);
    strncpy(audioTrack, out.m_strVal.c_str(), IJSEBUFFSIZE);
    audioTrack[IJSEBUFFSIZE] = '\0';

    LogJseGlueDebug("Channel=%s\n", audioTrack);
    return 0;
}

int
IMediaPlayerCell::getSubtitle(char* subtitle)
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eCurrentSubtitle;
    if(!m_player)
        return -1;
    m_player->getProperty(Player::PlayerPropertyType_eCurrentSubtitle, out);
    strncpy(subtitle, out.m_strVal.c_str(), IJSEBUFFSIZE);
    subtitle[IJSEBUFFSIZE] = '\0';

    LogJseGlueDebug("Channel=%s\n", subtitle);
    return 0;
}

long
IMediaPlayerCell::getMediaCount()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eMediaCount;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eMediaCount, out);

    LogJseGlueDebug("mediaCount=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

long
IMediaPlayerCell::getCurrentIndex()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eCurrentMediaIdx;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eCurrentMediaIdx, out);

    LogJseGlueDebug("CurrentIndex=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

int
IMediaPlayerCell::getEntryID(char* entryID)
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eCurrentMediaEntryId;
    if(!m_player)
        return -1;
    m_player->getProperty(Player::PlayerPropertyType_eCurrentMediaEntryId, out);
    strncpy(entryID, out.m_strVal.c_str(), IJSEBUFFSIZE);
    entryID[IJSEBUFFSIZE] = '\0';

    LogJseGlueDebug("CurrentEntryID=%s\n", entryID);
    return 0;
}

int
IMediaPlayerCell::getPlaylist(char* playlist)
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eMediaPlaylist;
    if(!m_player)
        return -1;
    m_player->getProperty(Player::PlayerPropertyType_eMediaPlaylist, out);
    strncpy(playlist, out.m_strVal.c_str(), IJSEBUFFSIZE);
    playlist[IJSEBUFFSIZE] = '\0';

    LogJseGlueDebug("Playlist=%s\n", playlist);
    return 0;
}

long
IMediaPlayerCell::releaseMediaPlayer(long nativePlayerInstanceID)
{
    LogJseGlueDebug("releaseMediaPlayer %d.\n", nativePlayerInstanceID);

    MediaPlayerMgr* playerMgr = HippoContext::getContextInstance()->getMediaPlayerMgr();
    if(!playerMgr)
        return -1;

    playerMgr->releasePlayerInstanceById(nativePlayerInstanceID);
    m_player = NULL;
    return 0;
}

long
IMediaPlayerCell::initMediaPlayer(long nativePlayerInstanceID, long playlistFlag, long videoDisplayMode, long height, long width, long left, long top, bool muteFlag, bool useNativeUIFlag, bool subtitleFlag, long videoAlpha, bool cycleFlag, bool randomFlag)
{
    if(!m_player)
        return -1;

    Player::HPlayerProperty out;
    Player* NativePlayer = m_player;
    //2. playlistflag.
    out.m_eType = Player::PlayerPropertyType_eSingleOrPlaylistMode;
    out.m_Value.m_intVal = playlistFlag;
    NativePlayer->setProperty(Player::PlayerPropertyType_eSingleOrPlaylistMode, out);
    //3.videoDisplayMode
    out.m_eType = Player::PlayerPropertyType_eVideoDisplayMode;
    out.m_Value.m_intVal = videoDisplayMode;
    NativePlayer->setProperty(Player::PlayerPropertyType_eVideoDisplayMode, out);
    //4.height,5.width,6.left,7.top
    out.m_Value.rect.m_h = height;
    out.m_Value.rect.m_w = width;
    out.m_Value.rect.m_x = left;
    out.m_Value.rect.m_y = top;
    out.m_eType = Player::PlayerPropertyType_eVideoDisplayArea;
    NativePlayer->setProperty(Player::PlayerPropertyType_eVideoDisplayArea, out);
    //8.muteflag.
    out.m_eType = Player::PlayerPropertyType_eMuteFlag;
    out.m_Value.m_intVal = muteFlag;
    //removed temporarily avoiding showing vol/mute ICON while start playing.
    //下面这句，导致进入小窗口播视频会自动静音。。所及注释掉先。
    //NativePlayer->setProperty( Player::PlayerPropertyType_eMuteFlag, out );
    //9.nativeUIFlag.
    out.m_eType = Player::PlayerPropertyType_eNativeUIFlag;
    out.m_Value.m_intVal = useNativeUIFlag;
    NativePlayer->setProperty(Player::PlayerPropertyType_eNativeUIFlag, out);
    //10. subtitleFlag.
    out.m_eType = Player::PlayerPropertyType_eSubtitleFlag;
    out.m_Value.m_intVal = subtitleFlag;
    NativePlayer->setProperty(Player::PlayerPropertyType_eSubtitleFlag, out);
    //11. videoAlpha.
    out.m_eType = Player::PlayerPropertyType_eVideoAlpha;
    out.m_Value.m_intVal = videoAlpha;
    NativePlayer->setProperty(Player::PlayerPropertyType_eVideoAlpha, out);
    //12. cycleFlag.
    out.m_eType = Player::PlayerPropertyType_eCycleFlag;
    out.m_Value.m_intVal = cycleFlag;
    NativePlayer->setProperty(Player::PlayerPropertyType_eCycleFlag, out);
    //13. randomflag.
    out.m_eType = Player::PlayerPropertyType_eRandomFlag;
    out.m_Value.m_intVal = randomFlag;
    NativePlayer->setProperty(Player::PlayerPropertyType_eRandomFlag, out);
    //14. auto del flag.. check document.
    out.m_eType = Player::PlayerPropertyType_eAutoDeleteFlag;
    out.m_Value.m_intVal = randomFlag;
    //NativePlayer->setProperty( Player::PlayerPropertyType_eAutoDeleteFlag, out );
    return 0;
}

long
IMediaPlayerCell::setSingleMedia(const char* mediaStr)
{
#if defined(DEBUG_BUILD)
    LogJseGlueDebug("val=%s,\n", mediaStr);
#endif
    if(m_player)
        m_player->setSingleMedia(Player::MediaInfoType_eJson, mediaStr);
    return 0;
}

long
IMediaPlayerCell::joinChannel(long userChannelId)
{
    LogJseGlueDebug("val=%ld\n", userChannelId);
    if(m_player)
        return m_player->joinChannel(userChannelId);
    return -1;
}

long
IMediaPlayerCell::leaveChannel()
{
    LogJseGlueDebug("leavel\n");
    if(m_player)
        return m_player->leaveChannel();
    return 0;
}

void
IMediaPlayerCell::addSingleMedia(long pIndex, const char* mediaStr)
{
    LogJseGlueDebug("val=%d %s\n", pIndex, mediaStr);
    if(m_player)
        m_player->addSingleMedia(Player::MediaInfoType_eJson, pIndex, mediaStr);
}

void
IMediaPlayerCell::addBatchMedia(const char* batchMediaStr)
{
    LogJseGlueDebug("val=%s\n", batchMediaStr);
    if(m_player)
        m_player->addSingleMedia(Player::MediaInfoType_eJson, 0, batchMediaStr);
}

void
IMediaPlayerCell::clearAllMedia()
{
    Player::HPlaylistProperty in;
    if(m_player)
        m_player->removePlayNode(Player::PlaylistOpType_eIdxRemoveAll, in);
}

void
IMediaPlayerCell::removeMediaByIndex(long pIndex)
{
    LogJseGlueDebug("val=%ld\n", pIndex);
	Player::HPlaylistProperty in;

	if(m_player) {
        in.m_oldValue.m_intVal = pIndex;
	    in.m_eType = Player::PlaylistOpType_eIdxRemoveByIndex;
	    m_player->removePlayNode(Player::PlaylistOpType_eIdxRemoveByIndex, in);
	}
}

void
IMediaPlayerCell::removeMediaByEntryID(const char* entryID)
{
    LogJseGlueDebug("val=%s\n", entryID);
	Player::HPlaylistProperty in;

	if(m_player ){
	    in.m_oldStrVal = entryID;
	    in.m_eType = Player::PlaylistOpType_eIdxRemoveByEntryId;
	    m_player->removePlayNode( Player::PlaylistOpType_eIdxRemoveByEntryId, in );
	}
}

void
IMediaPlayerCell::moveMediaByIndex(const char* entryID, long toIndex)
{
    LogJseGlueDebug("val=%s %ld\n", entryID, toIndex);
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_oldStrVal = entryID;
        in.m_newValue.m_intVal = toIndex;
        in.m_eType = Player::PlaylistOpType_eIdxMoveByIndex;
        m_player->movePlayNode(Player::PlaylistOpType_eIdxMoveByIndex, in);
    }
}

void
IMediaPlayerCell::moveMediaByOffset(const char* entryID, long offset)
{
    LogJseGlueDebug("val=%s %ld\n", entryID, offset);
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_oldStrVal = entryID;
        in.m_newValue.m_intVal = offset;
        in.m_eType = Player::PlaylistOpType_eIdxMoveByOffset;
        m_player->movePlayNode(Player::PlaylistOpType_eIdxMoveByOffset, in);
    }
}

void
IMediaPlayerCell::moveMediaByIndex1(long pIndex, long toIndex)
{
    LogJseGlueDebug("val=%ld %ld\n", pIndex, toIndex);
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_newValue.m_intVal = toIndex;
        in.m_oldValue.m_intVal = pIndex;
        in.m_eType = Player::PlaylistOpType_eIdxMoveByIndex1;
        m_player->movePlayNode(Player::PlaylistOpType_eIdxMoveByIndex1, in);
    }
}

void
IMediaPlayerCell::moveMediaByOffset1(long pIndex, long offset)
{
    LogJseGlueDebug("val=%ld %ld\n", pIndex, offset);
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_oldValue.m_intVal = pIndex;
        in.m_newValue.m_intVal = offset;
        in.m_eType = Player::PlaylistOpType_eIdxMoveByOffset1;
        m_player->movePlayNode(Player::PlaylistOpType_eIdxMoveByOffset1, in);
    }
}

void
IMediaPlayerCell::moveMediaToNext(const char* entryID)
{
    LogJseGlueDebug("val=%s\n", entryID);
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_oldStrVal = entryID;
        in.m_eType = Player::PlaylistOpType_eIdxMoveToNext;
        m_player->movePlayNode(Player::PlaylistOpType_eIdxMoveToNext, in);
    }
}

void
IMediaPlayerCell::moveMediaToPrevious(const char* entryID)
{
    LogJseGlueDebug("val=%s\n", entryID);
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_oldStrVal = entryID;
        in.m_eType = Player::PlaylistOpType_eIdxMoveToPrevious;
        m_player->movePlayNode(Player::PlaylistOpType_eIdxMoveToPrevious, in);
    }
}

void
IMediaPlayerCell::moveMediaToFirst(const char* entryID)
{
    LogJseGlueDebug("val=%s\n", entryID);
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_oldStrVal = entryID;
        in.m_eType = Player::PlaylistOpType_eIdxMoveToFirst;
        m_player->movePlayNode(Player::PlaylistOpType_eIdxMoveToFirst, in);
    }
}

void
IMediaPlayerCell::moveMediaToLast(const char* entryID)
{
    LogJseGlueDebug("val=%s\n", entryID);
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_oldStrVal = entryID;
        in.m_eType = Player::PlaylistOpType_eIdxMoveToLast;
        m_player->movePlayNode(Player::PlaylistOpType_eIdxMoveToLast, in);
    }
}

void
IMediaPlayerCell::moveMediaToNext1(long pIndex)
{
    LogJseGlueDebug("val=%ld\n", pIndex);
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_oldValue.m_intVal = pIndex;
        in.m_eType = Player::PlaylistOpType_eIdxMoveToNext1;
        m_player->movePlayNode(Player::PlaylistOpType_eIdxMoveToNext1, in);
    }
}

void
IMediaPlayerCell::moveMediaToPrevious1(long pIndex)
{
    LogJseGlueDebug("val=%ld\n", pIndex);
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_oldValue.m_intVal = pIndex;
        in.m_eType = Player::PlaylistOpType_eIdxMoveToPrevious1;
        m_player->movePlayNode(Player::PlaylistOpType_eIdxMoveToPrevious1, in);
    }
}

void
IMediaPlayerCell::moveMediaToFirst1(long pIndex)
{
    LogJseGlueDebug("val=%ld\n", pIndex);
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_oldValue.m_intVal = pIndex;
        in.m_eType = Player::PlaylistOpType_eIdxMoveToFirst1;
        m_player->movePlayNode(Player::PlaylistOpType_eIdxMoveToFirst1, in);
    }
}

void
IMediaPlayerCell::moveMediaToLast1(long pIndex)
{
    LogJseGlueDebug("val=%ld\n", pIndex);
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_oldValue.m_intVal = pIndex;
        in.m_eType = Player::PlaylistOpType_eIdxMoveToLast1;
        m_player->movePlayNode(Player::PlaylistOpType_eIdxMoveToLast1, in);
    }
}

void
IMediaPlayerCell::selectMediaByIndex(long pIndex)
{
    LogJseGlueDebug("val=%ld\n", pIndex);
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_oldValue.m_intVal = pIndex;
        in.m_eType = Player::PlaylistOpType_eIdxSelectByIndex;
        m_player->selectPlayNode(Player::PlaylistOpType_eIdxSelectByIndex, in);
    }
}

void
IMediaPlayerCell::selectMediaByOffset(long offset)
{
    LogJseGlueDebug("val=%ld\n", offset);
    Player::HPlaylistProperty in;
    if(m_player) {
        in.m_oldValue.m_intVal = offset;
        in.m_eType = Player::PlaylistOpType_eIdxSelectOffset;
        m_player->selectPlayNode(Player::PlaylistOpType_eIdxSelectOffset, in);
    }
}

void
IMediaPlayerCell::selectMediaByEntryID(const char* entryID)
{
    LogJseGlueDebug("val=%s\n", entryID);
    Player::HPlaylistProperty in;
    if(m_player) {
        in.m_oldStrVal = entryID;
        in.m_eType = Player::PlaylistOpType_eIdxSelectByEntryId;
        m_player->selectPlayNode(Player::PlaylistOpType_eIdxSelectByEntryId, in);
    }
}

void
IMediaPlayerCell::selectNext()
{
    LogJseGlueDebug("\n");
    Player::HPlaylistProperty in;
    if(m_player) {
        in.m_eType = Player::PlaylistOpType_eIdxSelectNext;
        m_player->selectPlayNode(Player::PlaylistOpType_eIdxSelectNext, in);
    }
}

void
IMediaPlayerCell::selectPrevious()
{
    LogJseGlueDebug("\n");
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_eType = Player::PlaylistOpType_eIdxSelectPrevious;
        m_player->selectPlayNode(Player::PlaylistOpType_eIdxSelectPrevious, in);
    }
}

void
IMediaPlayerCell::selectFirst()
{
    LogJseGlueDebug("\n");
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_eType = Player::PlaylistOpType_eIdxSelectFirst;
        m_player->selectPlayNode(Player::PlaylistOpType_eIdxSelectFirst, in);
    }
}

void
IMediaPlayerCell::selectLast()
{
    LogJseGlueDebug("\n");
    Player::HPlaylistProperty in;

    if(m_player) {
        in.m_eType = Player::PlaylistOpType_eIdxSelectLast;
        m_player->selectPlayNode(Player::PlaylistOpType_eIdxSelectLast, in);
    }
}

void
IMediaPlayerCell::play()
{
    LogJseGlueDebug("\n");
    if(m_player)
        m_player->play(0, Player::TimeType_eNPT);
}

void
IMediaPlayerCell::playFromStart()
{
    LogJseGlueDebug("\n");
    if(m_player)
        m_player->play(0, Player::TimeType_eNPT);
}

void
IMediaPlayerCell::playByTime(long type, const char* timestamp, float speed)
{
    LogJseGlueDebug("val=<%ld, %s, %f>\n", type, timestamp, speed);
    Player::time_type_e eType = Player::TimeType_eNPT;
    std::string ts = timestamp;

    if (m_player) {
        if(speed != 1) {
            eType = Player::TimeType_eNPT;
            m_player->fastForward(speed, atoi(ts.c_str()), eType);
            return;
        }
        switch (type) {
            case 1:
                eType = Player::TimeType_eNPT;
                break;
            case 2:
                eType = Player::TimeType_eUTC;
                break;
            default: // auto detective.
                eType = Player::TimeType_eNPT;
        }
        m_player->seekTo(ts.c_str(), eType);
    }
}

void
IMediaPlayerCell::pause()
{
    LogJseGlueDebug("\n");
    if(m_player)
        m_player->pause();
}

void
IMediaPlayerCell::fastForward(float speed)
{
    LogJseGlueDebug("val=%f\n", speed);
    if(m_player)
        m_player->fastForward(speed, 0, Player::TimeType_eIgnore);
}

void
IMediaPlayerCell::fastRewind(float speed)
{
    LogJseGlueDebug("val=%f\n", speed);
    if(m_player)
        m_player->fastRewind(speed, 0, Player::TimeType_eIgnore);
}

void
IMediaPlayerCell::resume()
{
    LogJseGlueDebug("\n");
    if(m_player)
        m_player->resume();
}

void
IMediaPlayerCell::gotoEnd()
{
    LogJseGlueDebug("\n");
    if(m_player)
        m_player->seekTo(0UL, Player::TimeType_eMediaEnd);
}

void
IMediaPlayerCell::gotoStart()
{
    LogJseGlueDebug("\n");
    if(m_player)
        m_player->seekTo(0UL, Player::TimeType_eMediaStart);
}

void
IMediaPlayerCell::stop()
{
    LogJseGlueDebug("\n");
    if(m_player)
        m_player->stop();
}

void
IMediaPlayerCell::refreshVideoDisplay()
{
    LogJseGlueDebug("\n");
    if(m_player)
        m_player->refreshVideoDisplay();
}

void
IMediaPlayerCell::switchAudioChannel(const char* audioChnl)
{
    LogJseGlueDebug("val=%s\n", audioChnl);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eAudioChannel;
    out.m_strVal = audioChnl;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eAudioChannel, out);
}

void
IMediaPlayerCell::switchAudioTrack()
{
    LogJseGlueDebug("\n");
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eAudioTrackPID;
    out.m_Value.m_intVal = -1;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eAudioTrackPID, out);
}

void
IMediaPlayerCell::switchSubtitle()
{
    LogJseGlueDebug("\n");
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eSubtitlePID;
    out.m_Value.m_intVal = -1;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eSubtitlePID, out);
}

void
IMediaPlayerCell::sendVendorSpecificCommand(const char* xml)
{
    LogJseGlueDebug("TODO\n");
}

long
IMediaPlayerCell::getAudioPID()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eAudioTrackPID;
    out.m_Value.m_intVal = -1;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eAudioTrackPID, out);

    LogJseGlueDebug("val=%ld\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

int
IMediaPlayerCell::getAudioPIDs(char* audioPIDs)
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eAudioTrackPIDs;
    if(!m_player)
        return -1;
    m_player->getProperty(Player::PlayerPropertyType_eAudioTrackPIDs, out);
    strncpy(audioPIDs, out.m_strVal.c_str(), IJSEBUFFSIZE);
    audioPIDs[IJSEBUFFSIZE] = '\0';

    LogJseGlueDebug("CurrentEntryID=%s\n",  audioPIDs);
    return 0;
}

void
IMediaPlayerCell::setAudioPID(long audioPID)
{
    LogJseGlueDebug("val=%ld\n", audioPID);
    selectAudio(audioPID);
}

long
IMediaPlayerCell::getSubtitlePID()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eSubtitlePID;
    out.m_Value.m_intVal = -1;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eSubtitlePID, out);

    LogJseGlueDebug("val=%ld\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

int
IMediaPlayerCell::getSubtitlePIDs(char* subtitlePIDs)
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eSubtitlePIDs;
    if(!m_player)
        return -1;
    m_player->getProperty(Player::PlayerPropertyType_eSubtitlePIDs, out);
    strncpy(subtitlePIDs, out.m_strVal.c_str(), IJSEBUFFSIZE);
    subtitlePIDs[IJSEBUFFSIZE] = '\0';

    LogJseGlueDebug("subtitlePIDs=%s\n", subtitlePIDs);
    return 0;
}

void
IMediaPlayerCell::setSubtitlePID(long subtitlePID)
{
    LogJseGlueDebug("val=%ld\n", subtitlePID);
    selectSubtitle(subtitlePID);
}

void
IMediaPlayerCell::setVolume(long val)
{
    LogJseGlueTest("val=%ld\n", val);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eAudioVolume;
    out.m_Value.m_intVal = val;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eAudioVolume, out);
}

long
IMediaPlayerCell::getVolume()
{
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eAudioVolume;
    if(m_player)
        m_player->getProperty(Player::PlayerPropertyType_eAudioVolume, out);

    LogJseGlueDebug("vol=%d\n", out.m_Value.m_intVal);
    return out.m_Value.m_intVal;
}

void
IMediaPlayerCell::set(const char* ioStr, const char* wrStr)
{
    LogJseGlueDebug("val=<%s, %s>\n", ioStr, wrStr);
    if(m_player)
        m_player->set(ioStr, wrStr);
}

int
IMediaPlayerCell::get(const char* ioStr, char* wrStr)
{
    int ret = -1;

    wrStr[0] = '\0';
    if (m_player) {
        const char* pstr = m_player->get(ioStr);
        if (pstr) {
            strncpy(wrStr, pstr, IJSEBUFFSIZE);
            wrStr[IJSEBUFFSIZE] = '\0';
            ret = 0;
        }
    }

    LogJseGlueDebug("vale=<%s, %s>\n", ioStr, wrStr);
    return ret;
}

int
IMediaPlayerCell::getAllAudioTrackInfo(char* jsStr)
{
    int ret = -1;

    jsStr[0] = '\0';
    if (m_player) {
        char ioctlStr[] = "getAllAudioTrackInfo";
        const char* pstr = m_player->get(ioctlStr);
        if (pstr) {
            strncpy(jsStr, pstr, IJSEBUFFSIZE);
            jsStr[IJSEBUFFSIZE] = '\0';
            ret = 0;
        }
    }

    LogJseGlueDebug("val=%s\n", jsStr);
    return ret;
}

int
IMediaPlayerCell::getCurrentAudioTrackInfo(char* jsStr)
{
    int ret = -1;
    Player::HPlayerProperty out;

    jsStr[0] = '\0';
    if(m_player) {
        out.m_eType = Player::PlayerPropertyType_eAudioTrackInfo;
        m_player->getProperty(Player::PlayerPropertyType_eAudioTrackInfo, out);
        strncpy(jsStr, out.m_strVal.c_str(), IJSEBUFFSIZE);
        jsStr[IJSEBUFFSIZE] = '\0';
        ret = 0;
    }

    LogJseGlueDebug("val=%s\n", jsStr);
    return ret;
}

int
IMediaPlayerCell::getAllSubtitleInfo(char* jsStr)
{
    int ret = -1;

    jsStr[0] = '\0';
    if(m_player) {
        char ioctlStr[] = "getAllSubtitleInfo";
        const char* pstr = m_player->get(ioctlStr);
        if (pstr) {
            strncpy(jsStr, pstr, IJSEBUFFSIZE);
            jsStr[IJSEBUFFSIZE] = '\0';
            ret = 0;
        }
    }

    LogJseGlueDebug("val=%s\n", jsStr);
    return -1;
}

int
IMediaPlayerCell::getCurrentSubtitleInfo(char* jsStr)
{
    int ret = -1;
    Player::HPlayerProperty out;

    jsStr[0] = '\0';
    if(m_player) {
        out.m_eType = Player::PlayerPropertyType_eSubtitleInfo;
        m_player->getProperty(Player::PlayerPropertyType_eSubtitleInfo, out);
        strncpy(jsStr, out.m_strVal.c_str(), IJSEBUFFSIZE);
        jsStr[IJSEBUFFSIZE] = '\0';
        ret = 0;
    }

    LogJseGlueDebug("val=%s\n", jsStr);
    return ret;
}

int
IMediaPlayerCell::getAllTeletextInfo(char* jsStr)
{
    int ret = -1;

    jsStr[0] = '\0';
    if(m_player) {
        char ioctlStr[] = "getAllTeletextInfo";
        const char* pstr = m_player->get(ioctlStr);
        if (pstr) {
            strncpy(jsStr, pstr, IJSEBUFFSIZE);
            jsStr[IJSEBUFFSIZE] = '\0';
            ret = 0;
        }
    }

    LogJseGlueDebug("val=%s\n", jsStr);
    return ret;
}

void
IMediaPlayerCell::selectAudio(long audioPID)
{
    LogJseGlueDebug("val=%ld\n", audioPID);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eAudioTrackPID;
    out.m_Value.m_intVal = audioPID;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eAudioTrackPID, out);
}

void
IMediaPlayerCell::selectSubtitle(long subtitlePID)
{
    LogJseGlueDebug("val=%ld\n", subtitlePID);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eSubtitlePID;
    out.m_Value.m_intVal = subtitlePID;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eSubtitlePID, out);
}

void
IMediaPlayerCell::selectTeletext(long teletextPID)
{
    LogJseGlueDebug("val=%ld\n", teletextPID);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eTeletext;
    out.m_Value.m_intVal = teletextPID;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eTeletext, out);
}

long
IMediaPlayerCell::setMacrovisionFlag(long flag)
{
    LogJseGlueDebug("val=%ld\n", flag);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eMacrovisionFlag;
    out.m_Value.m_intVal = flag;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eMacrovisionFlag, out);
    return 0;
}

long
IMediaPlayerCell::setHDCPFlag(long flag)
{
    LogJseGlueDebug("val=%ld\n", flag);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eHDCPFlag;
    out.m_Value.m_intVal = flag;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eHDCPFlag, out);
    return 0;
}

long
IMediaPlayerCell::setCGMSAFlag(long flag)
{
    LogJseGlueDebug("val=%ld\n", flag);
    Player::HPlayerProperty out;

    out.m_eType = Player::PlayerPropertyType_eCGMSAFlag;
    out.m_Value.m_intVal = flag;
    if(m_player)
        m_player->setProperty(Player::PlayerPropertyType_eCGMSAFlag, out);
    return 0;
}

IMediaPlayerCell::IMediaPlayerCell()
    : m_player(NULL)
{
}

//export
IMediaPlayer* createIMediaPlayer(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);
    return new IMediaPlayerCell;
}

