#include "IMediaPlayerCaller.h"

IMediaPlayerCaller::IMediaPlayerCaller(const char* name, IPCCaller* parent)
    : IIPCCaller(name, parent)
{
    LogIpcDebug("create %s\n", name);
}

int 
IMediaPlayerCaller::createPlayerInstance(const char* name)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("name", name);
    call("createPlayerInstance");
    popArg("return", ret);
    freeStack();
    return ret;
}

long 
IMediaPlayerCaller::getNativePlayerInstanceID()
{
    initStack(SP_Call);
    call("getNativePlayerInstanceID");
    long playID;
    popArg("playID", playID);
    freeStack();
    return playID;
}

long 
IMediaPlayerCaller::bindNativePlayerInstance(long playID)
{
    initStack(SP_Call);
    pushArg("palyID", playID);
    call("bindNativePlayerInstance");
    freeStack();
    return 0;
}

long IMediaPlayerCaller::HandleEvent()
{
    LogIpcError("TODO\n");
    return -1;
}

void 
IMediaPlayerCaller::setSingleOrPlaylistMode(long mode)
{
    initStack(SP_Call);
    pushArg("mode", mode);
    call("setSingleOrPlaylistMode");
    freeStack();
}

long 
IMediaPlayerCaller::getSingleOrPlaylistMode()
{
    initStack(SP_Call);
    call("getSingleOrPlaylistMode");
    long mode = 0;
    popArg("mode", mode);
    freeStack();
    return mode;
}

void 
IMediaPlayerCaller::setVideoDisplayMode(long mode)
{
    initStack(SP_Call);
    pushArg("mode", mode);
    call("setVideoDisplayMode");
    freeStack();
}

long 
IMediaPlayerCaller::getVideoDisplayMode()
{
    initStack(SP_Call);
    call("getVideoDisplayMode");
    long mode = 0;
    popArg("mode", mode);
    freeStack();
    return mode;
}

long 
IMediaPlayerCaller::setVideoDisplayArea(long left, long top, long width, long height)
{
    initStack(SP_Call);
    pushArg("height", height);
    pushArg("width", width);
    pushArg("top", top);
    pushArg("left", left);
    call("setVideoDisplayArea");
    freeStack();
    return 0;
}

long 
IMediaPlayerCaller::getVideoDisplayLeft()
{
    initStack(SP_Call);
    call("getVideoDisplayLeft");
    long left = 0;
    popArg("left", left);
    freeStack();
    return left;
}

long 
IMediaPlayerCaller::getVideoDisplayTop()
{
    initStack(SP_Call);
    call("getVideoDisplayTop");
    long top = 0;
    popArg("top", top);
    freeStack();
    return top;
}

long 
IMediaPlayerCaller::getVideoDisplayWidth()
{
    initStack(SP_Call);
    call("getVideoDisplayWidth");
    long width = 0;
    popArg("width", width);
    freeStack();
    return width;
}

long 
IMediaPlayerCaller::getVideoDisplayHeight()
{
    initStack(SP_Call);
    call("getVideoDisplayHeight");
    long height = 0;
    popArg("height", height);
    freeStack();
    return height;
}

void 
IMediaPlayerCaller::setMuteFlag(const bool muteFlag)
{
    initStack(SP_Call);
    pushArg("muteFlag", muteFlag);
    call("setMuteFlag");
    freeStack();
}

int 
IMediaPlayerCaller::getMuteFlag()
{
    initStack(SP_Call);
    call("getMuteFlag");
    int muteFlag = 0;
    popArg("muteFlag", muteFlag);
    freeStack();
    return muteFlag;
}

void 
IMediaPlayerCaller::setNativeUIFlag(bool nativeUIFlag)
{
    initStack(SP_Call);
    pushArg("nativeUIFlag", nativeUIFlag);
    call("setNativeUIFlag");
    freeStack();
}

int 
IMediaPlayerCaller::getNativeUIFlag()
{
    initStack(SP_Call);
    call("getNativeUIFlag");
    int nativeUIFlag = 0;
    popArg("nativeUIFlag", nativeUIFlag);
    freeStack();
    return nativeUIFlag;
}

void 
IMediaPlayerCaller::setMuteUIFlag(bool muteUIFlag)
{
    initStack(SP_Call);
    pushArg("muteUIFlag", muteUIFlag);
    call("setMuteUIFlag");
    freeStack();
}

int 
IMediaPlayerCaller::getMuteUIFlag()
{
    initStack(SP_Call);
    call("getMuteUIFlag");
    int muteUIFlag = 0;
    popArg("muteUIFlag", muteUIFlag);
    freeStack();
    return muteUIFlag;
}

void 
IMediaPlayerCaller::setAudioVolumeUIFlag(bool audioVolumeUIFlag)
{
    initStack(SP_Call);
    pushArg("audioVolumeUIFlag", audioVolumeUIFlag);
    call("setAudioVolumeUIFlag");
    freeStack();
}

int 
IMediaPlayerCaller::getAudioVolumeUIFlag()
{
    initStack(SP_Call);
    call("getAudioVolumeUIFlag");
    int audioVolumeUIFlag = 0;
    popArg("audioVolumeUIFlag", audioVolumeUIFlag);
    freeStack();
    return audioVolumeUIFlag;
}

void
IMediaPlayerCaller::setAudioTrackUIFlag(bool audioTrackUIFlag)
{
    initStack(SP_Call);
    pushArg("audioTrackUIFlag", audioTrackUIFlag);
    call("setAudioTrackUIFlag");
    freeStack();
}

int 
IMediaPlayerCaller::getAudioTrackUIFlag()
{
    initStack(SP_Call);
    call("getAudioTrackUIFlag");
    int audioTrackUIFlag = 0;
    popArg("audioTrackUIFlag", audioTrackUIFlag); 
    freeStack();
    return audioTrackUIFlag;
}

void 
IMediaPlayerCaller::setProgressBarUIFlag(bool progressBarUIFlag)
{
    initStack(SP_Call);
    popArg("progressBarUIFlag", progressBarUIFlag);
    call("setProgressBarUIFlag");
    freeStack();
}

int 
IMediaPlayerCaller::getProgressBarUIFlag()
{
    initStack(SP_Call);
    call("getProgressBarUIFlag");
    int progressBarUIFlag = 0;
    popArg("progressBarUIFlag", progressBarUIFlag);
    freeStack();
    return progressBarUIFlag;
}

void 
IMediaPlayerCaller::setChannelNoUIFlag(bool channelNoUIFlag)
{
    initStack(SP_Call);
    pushArg("channelNoUIFlag", channelNoUIFlag);
    call("setChannelNoUIFlag");
    freeStack();
}

int 
IMediaPlayerCaller::getChannelNoUIFlag()
{
    initStack(SP_Call);
    call("getChannelNoUIFlag");
    int channelNoUIFlag = 0;
    popArg("channelNoUIFlag", channelNoUIFlag);
    freeStack();
    return channelNoUIFlag;
}

int
IMediaPlayerCaller::getSubtitileFlag()
{
    initStack(SP_Call);
    call("getSubtitileFlag");
    int subtitileFlag = 0;
    popArg("subtitileFlag", subtitileFlag);
    freeStack();
    return subtitileFlag;
}

void 
IMediaPlayerCaller::setSubtitileFlag(bool subtitileFlag)
{
    initStack(SP_Call);
    pushArg("subtitileFlag", subtitileFlag);
    call("setSubtitileFlag");
    freeStack();
}

int 
IMediaPlayerCaller::getSubtitleFlag()
{
    initStack(SP_Call);
    call("getSubtitleFlag");
    int subtitleFlag = 0;
    popArg("subtitleFlag", subtitleFlag);
    freeStack();
    return subtitleFlag;
}

void 
IMediaPlayerCaller::setSubtitleFlag(bool subtitleFlag)
{
    initStack(SP_Call);
    pushArg("subtitleFlag", subtitleFlag);
    call("setSubtitleFlag");
    freeStack();
}

int 
IMediaPlayerCaller::getVideoAlpha()
{
    initStack(SP_Call);
    call("getVideoAlpha");
    int videoAlpha = 0;
    popArg("videoAlpha", videoAlpha);
    freeStack();
    return videoAlpha;
}

void 
IMediaPlayerCaller::setVideoAlpha(long videoAlpha)
{
    initStack(SP_Call);
    pushArg("videoAlpha", videoAlpha);
    call("setVideoAlpha");
    freeStack();
}

int 
IMediaPlayerCaller::getAllowTrickmodeFlag()
{
    initStack(SP_Call);
    call("getAllowTrickmodeFlag");
    int allowTrickmodeFlag = 0;
    popArg("allowTrickmodeFlag", allowTrickmodeFlag);
    freeStack();
    return allowTrickmodeFlag;
}

void IMediaPlayerCaller::setAllowTrickmodeFlag(bool allowTrickmodeFlag)
{
    initStack(SP_Call);
    pushArg("allowTrickmodeFlag", allowTrickmodeFlag);
    call("setAllowTrickmodeFlag");
    freeStack();
}

int
IMediaPlayerCaller::getCycleFlag()
{
    initStack(SP_Call);
    call("getCycleFlag");
    int cycleFlag = 0;
    popArg("cycleFlag", cycleFlag);
    freeStack();
    return cycleFlag;
}

void
IMediaPlayerCaller::setCycleFlag(bool cycleFlag)
{
    initStack(SP_Call);
    pushArg("cycleFlag", cycleFlag);
    call("setCycleFlag");
    freeStack();
}

int 
IMediaPlayerCaller::getRandomFlag()
{
    initStack(SP_Call);
    call("getRandomFlag");
    int randomFlag = 0; 
    popArg("randomFlag", randomFlag);
    freeStack();
    return randomFlag;
}

void 
IMediaPlayerCaller::setRandomFlag(bool randomFlag)
{
    initStack(SP_Call);
    pushArg("randomFlag", randomFlag);
    call("setRandomFlag");
    freeStack();
}

char* IMediaPlayerCaller::getVendorSpecificAttr(const char* pVSAttrKey)
{
    LogIpcError("TODO\n");
    return 0;
}

void IMediaPlayerCaller::setVendorSpecificAttr(const char* pVSAttrKey, const char* pVSAttrValue)
{
    LogIpcError("TODO\n");
}

int 
IMediaPlayerCaller::getMediaCode(char* mediaCode)
{
    int ret = -1;
    initStack(SP_Call);
    call("getMediaCode");
    popArg("return", ret);
    popArg("mediaCode", mediaCode, IJSEBUFFSIZE); 
    freeStack();
    return ret; 
}

long
IMediaPlayerCaller::getMediaDuration()
{
    initStack(SP_Call);
    call("getMediaDuration");
    long mediaDuration = 0;
    popArg("mediaDuration", mediaDuration);
    freeStack();
    return mediaDuration;
}

int 
IMediaPlayerCaller::getCurrentPlayTime(char* currentPlayTime)
{
    int ret = -1;
    initStack(SP_Call);
    call("getCurrentPlayTime");
    popArg("return", ret);
    popArg("currentPlayTime", currentPlayTime, IJSEBUFFSIZE);
    freeStack();
    return ret;
}

int  
IMediaPlayerCaller::getPlaybackMode(char* playbackMode)
{
    int ret = -1;
    initStack(SP_Call);
    call("getPlaybackMode");
    popArg("return", ret);
    popArg("playbackMode", playbackMode, IJSEBUFFSIZE);
    freeStack();
    return ret; 
}

long
IMediaPlayerCaller::getChannelNum()
{
    initStack(SP_Call);
    call("getChannelNum");
    long channelNum = 0;
    popArg("channelNum", channelNum);
    freeStack();
    return channelNum;
}

int 
IMediaPlayerCaller::getCurrentAudioChannel(char* currentAudioChannel)
{
    int ret = -1;
    initStack(SP_Call);
    call("getCurrentAudioChannel");
    popArg("return", ret);
    popArg("currentAudioChannel", currentAudioChannel, IJSEBUFFSIZE);
    freeStack();
    return ret; 
}

int 
IMediaPlayerCaller::getAudioTrack(char* audioTrack)
{
    int ret = -1;
    initStack(SP_Call);
    call("getAudioTrack");
    popArg("return", ret);
    popArg("audioTrack", audioTrack, IJSEBUFFSIZE);
    freeStack();
    return ret; 
}

int
IMediaPlayerCaller::getSubtitle(char* subtitle)
{
    int ret = -1;
    initStack(SP_Call);
    call("getSubtitle");
    popArg("return", ret);
    popArg("subtitle", subtitle, IJSEBUFFSIZE);
    freeStack();
    return ret; 
}

long
IMediaPlayerCaller::getMediaCount()
{
    initStack(SP_Call);
    call("getMediaCount");
    long mediaCount = 0;
    popArg("mediaCount", mediaCount);
    freeStack();
    return mediaCount;
}

long 
IMediaPlayerCaller::getCurrentIndex()
{
    initStack(SP_Call);
    call("getCurrentIndex");
    long currentIndex = 0;
    popArg("currentIndex", currentIndex);
    freeStack();
    return currentIndex;
}

int
IMediaPlayerCaller::getEntryID(char* entryID)
{
    int ret = -1;
    initStack(SP_Call);
    call("getEntryID");
    popArg("return", ret);
    popArg("entryID", entryID, IJSEBUFFSIZE);
    freeStack();
    return ret;
}

int
IMediaPlayerCaller::getPlaylist(char* playlist)
{
    int ret = -1;
    initStack(SP_Call);
    call("getPlaylist");
    popArg("return", ret);
    popArg("playlist", playlist, IJSEBUFFSIZE);
    freeStack();
    return ret;
}

long
IMediaPlayerCaller::releaseMediaPlayer(long nativePlayerInstanceID)
{
    initStack(SP_Call);
    pushArg("nativePlayerInstanceID", nativePlayerInstanceID);
    call("releaseMediaPlayer");
    freeStack();
    return 0;
}

long IMediaPlayerCaller::initMediaPlayer(long nativePlayerInstanceID, long playlistFlag, long videoDisplayMode, long height, long width, long left, long top, bool muteFlag, bool useNativeUIFlag, bool subtitleFlag, long videoAlpha, bool cycleFlag, bool randomFlag)
{
    initStack(SP_Call);
    pushArg("randomFlag", randomFlag);
    pushArg("cycleFlag", cycleFlag);
    pushArg("videoAlpha", videoAlpha);
    pushArg("subtitleFlag", subtitleFlag);
    pushArg("useNativeUIFlag", useNativeUIFlag);
    pushArg("muteFlag", muteFlag);
    pushArg("top", top);
    pushArg("left", left);
    pushArg("width", width);
    pushArg("height", height);
    pushArg("videoDisplayMode", videoDisplayMode);
    pushArg("playlistFlag", playlistFlag);
    pushArg("nativePlayerInstanceID", nativePlayerInstanceID);
    call("initMediaPlayer");
    freeStack();
    return 0;
}

long 
IMediaPlayerCaller::setSingleMedia(const char* mediaStr)
{
    initStack(SP_Call);
    pushArg("mediaStr", mediaStr);
    call("setSingleMedia");
    freeStack();
    return 0;
}

long 
IMediaPlayerCaller::joinChannel(long userChannelId)
{
    initStack(SP_Call);
    pushArg("userChannelId", userChannelId);
    call("joinChannel");
    int ret;
    popArg("return", ret);
    freeStack();
    return ret;
}

long 
IMediaPlayerCaller::leaveChannel()
{
    initStack(SP_Call);
    call("leaveChannel");
    int ret = 0;
    popArg("return", ret);
    freeStack();
    return ret;
}

void 
IMediaPlayerCaller::addSingleMedia(long pIndex, const char* mediaStr)
{
    initStack(SP_Call);
    pushArg("mediaStr", mediaStr);
    pushArg("index", pIndex);
    call("addSingleMedia");
    freeStack();
}

void 
IMediaPlayerCaller::addBatchMedia(const char* batchMediaStr)
{
    initStack(SP_Call);
    pushArg("batchMediaStr", batchMediaStr);
    call("addBatchMedia");
    freeStack();
}

void 
IMediaPlayerCaller::clearAllMedia()
{
    initStack(SP_Call);
    call("clearAllMedia");
    freeStack();
}

void 
IMediaPlayerCaller::removeMediaByIndex(long pIndex)
{
    initStack(SP_Call);
    pushArg("index", pIndex);
    call("removeMediaByIndex");
    freeStack();
}

void 
IMediaPlayerCaller::removeMediaByEntryID(const char* entryID)
{
    initStack(SP_Call);
    pushArg("entryID", entryID);
    call("removeMediaByEntryID");
    freeStack();
}

void 
IMediaPlayerCaller::moveMediaByIndex(const char* entryID, long toIndex)
{
    initStack(SP_Call);
    pushArg("toIndex", toIndex);
    pushArg("entryID", entryID);
    call("moveMediaByIndex");
    freeStack();
}

void 
IMediaPlayerCaller::moveMediaByOffset(const char* entryID, long offset)
{
    initStack(SP_Call);
    pushArg("offset", offset);
    pushArg("entryID", entryID);
    call("moveMediaByOffset");
    freeStack();
}

void 
IMediaPlayerCaller::moveMediaByIndex1(long pIndex, long toIndex)
{
    initStack(SP_Call);
    pushArg("toIndex", toIndex);
    pushArg("index", pIndex);
    call("moveMediaByIndex1");
    freeStack();
}

void 
IMediaPlayerCaller::moveMediaByOffset1(long pIndex, long offset)
{
    initStack(SP_Call);
    pushArg("offset", offset);
    pushArg("index", pIndex);
    call("moveMediaByOffset1");
    freeStack();
}

void 
IMediaPlayerCaller::moveMediaToNext(const char* entryID)
{
    initStack(SP_Call);
    pushArg("entryID", entryID);
    call("moveMediaToNext");
    freeStack();
}

void 
IMediaPlayerCaller::moveMediaToPrevious(const char* entryID)
{
    initStack(SP_Call);
    pushArg("entryID", entryID);
    call("moveMediaToPrevious");
    freeStack();
}

void 
IMediaPlayerCaller::moveMediaToFirst(const char* entryID)
{
    initStack(SP_Call);
    pushArg("entryID", entryID);
    call("moveMediaToFirst");
    freeStack();
}

void 
IMediaPlayerCaller::moveMediaToLast(const char* entryID)
{
    initStack(SP_Call);
    pushArg("entryID", entryID);
    call("moveMediaToLast");
    freeStack();
}

void 
IMediaPlayerCaller::moveMediaToNext1(long pIndex)
{
    initStack(SP_Call);
    pushArg("index", pIndex);
    call("moveMediaToNext1");
    freeStack();
}

void 
IMediaPlayerCaller::moveMediaToPrevious1(long pIndex)
{
    initStack(SP_Call);
    pushArg("index", pIndex);
    call("moveMediaToPrevious1");
    freeStack();
}

void
IMediaPlayerCaller::moveMediaToFirst1(long pIndex)
{
    initStack(SP_Call);
    pushArg("index", pIndex);
    call("moveMediaToFirst1");
    freeStack();
}

void
IMediaPlayerCaller::moveMediaToLast1(long pIndex)
{
    initStack(SP_Call);
    pushArg("index", pIndex);
    call("moveMediaToLast1");
    freeStack();
}

void 
IMediaPlayerCaller::selectMediaByIndex(long pIndex)
{
    initStack(SP_Call);
    pushArg("index", pIndex);
    call("selectMediaByIndex");
    freeStack();
}

void 
IMediaPlayerCaller::selectMediaByOffset(long offset)
{
    initStack(SP_Call);
    pushArg("offset", offset);
    call("selectMediaByOffset");
    freeStack();
}

void 
IMediaPlayerCaller::selectMediaByEntryID(const char* entryID)
{
    initStack(SP_Call);
    pushArg("entryID", entryID);
    call("selectMediaByEntryID");
    freeStack();
}

void
IMediaPlayerCaller::selectNext()
{
    initStack(SP_Call);
    call("selectNext");
    freeStack();
}

void 
IMediaPlayerCaller::selectPrevious()
{
    initStack(SP_Call);
    call("selectPrevious");
    freeStack();
}

void
IMediaPlayerCaller::selectFirst()
{
    initStack(SP_Call);
    call("selectFirst");
    freeStack();
}

void
IMediaPlayerCaller::selectLast()
{
    initStack(SP_Call);
    call("selectLast");
    freeStack();
}

void
IMediaPlayerCaller::play()
{
    initStack(SP_Call);
    call("play");
    freeStack();
}

void
IMediaPlayerCaller::playFromStart()
{
    initStack(SP_Call);
    call("playFromStart");
    freeStack();
}

void
IMediaPlayerCaller::playByTime(long type, const char* timestamp, float speed)
{
    initStack(SP_Call);
    pushArg("speed", speed);
    pushArg("timestamp", timestamp);
    pushArg("type", type);
    call("playByTime");
    freeStack();
}

void IMediaPlayerCaller::pause()
{
    initStack(SP_Call);
    call("pause");
    freeStack();
}

void 
IMediaPlayerCaller::fastForward(float speed)
{
    initStack(SP_Call);
    pushArg("speed", speed);
    call("fastForward");
    freeStack();
}

void
IMediaPlayerCaller::fastRewind(float speed)
{
    initStack(SP_Call);
    pushArg("speed", speed);
    call("fastRewind");
    freeStack();
}

void
IMediaPlayerCaller::resume()
{
    initStack(SP_Call);
    call("resume");
    freeStack();
}

void
IMediaPlayerCaller::gotoEnd()
{
    initStack(SP_Call);
    call("gotoEnd");
    freeStack();
}

void
IMediaPlayerCaller::gotoStart()
{
    initStack(SP_Call);
    call("gotoStart");
    freeStack();
}

void
IMediaPlayerCaller::stop()
{
    initStack(SP_Call);
    call("stop");
    freeStack();
}

void IMediaPlayerCaller::refreshVideoDisplay()
{
    initStack(SP_Call);
    call("refreshVideoDisplay");
    freeStack();
}

void
IMediaPlayerCaller::switchAudioChannel(const char* audioChnl)
{
    initStack(SP_Call);
    pushArg("audioChhl", audioChnl);
    call("switchAudioChannel");
    freeStack();
}

void
IMediaPlayerCaller::switchAudioTrack()
{
    initStack(SP_Call);
    call("switchAudioTrack");
    freeStack();
}

void
IMediaPlayerCaller::switchSubtitle()
{
    initStack(SP_Call);
    call("switchSubtitle");
    freeStack();
}

void
IMediaPlayerCaller::sendVendorSpecificCommand(const char* xml)
{
    initStack(SP_Call);
    pushArg("xml", xml);
    call("sendVendorSpecificCommand");
    freeStack();
}

long
IMediaPlayerCaller::getAudioPID()
{
    initStack(SP_Call);
    call("getAudioPID");
    long audioPID = 0;
    popArg("audioPID", audioPID);
    freeStack();
    return audioPID;
}

int
IMediaPlayerCaller::getAudioPIDs(char* audioPIDs)
{
    int ret = -1;
    initStack(SP_Call);
    call("getAudioPIDs");
    popArg("return", ret);
    popArg("audioPIDs", audioPIDs, IJSEBUFFSIZE);
    freeStack();
    return ret;
}

void
IMediaPlayerCaller::setAudioPID(long audioPID)
{
    initStack(SP_Call);
    pushArg("audioPID", audioPID);
    call("setAudioPID");
    freeStack();
}

long
IMediaPlayerCaller::getSubtitlePID()
{
    initStack(SP_Call);
    call("getSubtitlePID");
    long subtitlePID = 0;
    popArg("subtitle", subtitlePID);
    freeStack();
    return subtitlePID;
}

int
IMediaPlayerCaller::getSubtitlePIDs(char* subtitlePIDs)
{
    int ret = -1;
    initStack(SP_Call);
    call("getSubtitlePIDs");
    popArg("return", ret);
    popArg("subtitlePIDs", subtitlePIDs, IJSEBUFFSIZE);
    freeStack();
    return ret; 
}

void 
IMediaPlayerCaller::setSubtitlePID(long subtitlePID)
{
    initStack(SP_Call);
    pushArg("subtitlePID", subtitlePID);
    call("setSubtitlePID");
    freeStack();
}

void
IMediaPlayerCaller::setVolume(long val)
{
    initStack(SP_Call);
    pushArg("val", val);
    call("setVolume");
    freeStack();
}

long
IMediaPlayerCaller::getVolume()
{
    initStack(SP_Call);
    call("getVolume");
    long val = 0;
    popArg("val", val);
    freeStack();
    return val;
}

void
IMediaPlayerCaller::set(const char* ioStr, const char* wrStr)
{
    initStack(SP_Call);
    pushArg("wrStr", wrStr);
    pushArg("ioStr", ioStr);
    call("set");
    freeStack();
}

int
IMediaPlayerCaller::get(const char* ioStr, char* wrStr)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("ioStr", ioStr);
    call("get");
    popArg("return", ret);
    popArg("wrStr", wrStr, IJSEBUFFSIZE);
    freeStack();
    return ret; 
}

int 
IMediaPlayerCaller::getAllAudioTrackInfo(char* jsStr)
{
    int ret = -1;
    initStack(SP_Call);
    call("getAllAudioTrackInfo");
    popArg("return", ret);
    popArg("jsStr", jsStr, IJSEBUFFSIZE);
    freeStack();
    return ret;    
}

int 
IMediaPlayerCaller::getCurrentAudioTrackInfo(char* jsStr)
{
    int ret = -1;
    initStack(SP_Call);
    call("getCurrentAudioTrackInfo");
    popArg("return", ret);
    popArg("jsStr", jsStr, IJSEBUFFSIZE);
    freeStack();
    return ret;    
}

int
IMediaPlayerCaller::getAllSubtitleInfo(char* jsStr)
{
    int ret = -1;
    initStack(SP_Call);
    call("getAllSubtitleInfo");
    popArg("return", ret);
    popArg("jsStr", jsStr, IJSEBUFFSIZE);
    freeStack();
    return ret;    
}

int
IMediaPlayerCaller::getCurrentSubtitleInfo(char* jsStr)
{
    int ret = -1;
    initStack(SP_Call);
    call("getCurrentSubtitleInfo");
    popArg("return", ret);
    popArg("jsStr", jsStr, IJSEBUFFSIZE);
    freeStack();
    return ret;    
}

int
IMediaPlayerCaller::getAllTeletextInfo(char* jsStr)
{
    int ret = -1;
    initStack(SP_Call);
    call("getAllTeletextInfo");
    popArg("return", ret);
    popArg("jsStr", jsStr, IJSEBUFFSIZE);
    freeStack();
    return ret;    
}

void
IMediaPlayerCaller::selectAudio(long audioPID)
{
    initStack(SP_Call);
    pushArg("audioPID", audioPID);
    call("selectAudio");
    freeStack();
}

void IMediaPlayerCaller::selectSubtitle(long subtitlePID)
{
    initStack(SP_Call);
    pushArg("subtitlePID", subtitlePID);
    call("selectSubtitle");
    freeStack();
}

void 
IMediaPlayerCaller::selectTeletext(long teletextPID)
{
    initStack(SP_Call);
    pushArg("teletextPID", teletextPID);
    call("selectTeletext");
    freeStack();
}

long IMediaPlayerCaller::setMacrovisionFlag(long flag)
{
    initStack(SP_Call);
    pushArg("flag", flag);
    call("setMacrovisionFlag");
    freeStack();
    return 0;
}

long IMediaPlayerCaller::setHDCPFlag(long flag)
{
    initStack(SP_Call);
    pushArg("flag", flag);
    call("setHDCPFlag");
    freeStack();
    return 0;
}

long IMediaPlayerCaller::setCGMSAFlag(long flag)
{
    initStack(SP_Call);
    pushArg("flag", flag);
    call("setCGMSAFlag");
    freeStack();
    return 0;
}

//export
IMediaPlayer* createIMediaPlayer(const char* name)
{
    LogIpcDebug("name<%s>\n", name);
    return Agent()->createIMediaPlayer(name);
}
