#include "IMediaPlayerServer.h"

IMediaPlayerServer::IMediaPlayerServer()
{
    LogJseGlueDebug("new\n");
}

IMediaPlayerServer::~IMediaPlayerServer()
{
    LogJseGlueDebug("delete\n");
}

int 
IMediaPlayerServer::attachServer()
{
    addFunction("createPlayerInstance", &IMediaPlayerServer::IcreatePlayerInstance);
    addFunction("getNativePlayerInstanceID", &IMediaPlayerServer::IgetNativePlayerInstanceID);
    addFunction("bindNativePlayerInstance", &IMediaPlayerServer::IbindNativePlayerInstance);
    addFunction("HandleEvent", &IMediaPlayerServer::IHandleEvent);

    addFunction("setSingleOrPlaylistMode", &IMediaPlayerServer::IsetSingleOrPlaylistMode);
    addFunction("getSingleOrPlaylistMode", &IMediaPlayerServer::IgetSingleOrPlaylistMode);

    addFunction("setVideoDisplayMode", &IMediaPlayerServer::IsetVideoDisplayMode);
    addFunction("getVideoDisplayMode", &IMediaPlayerServer::IgetVideoDisplayMode);

    addFunction("setVideoDisplayArea", &IMediaPlayerServer::IsetVideoDisplayArea);
    addFunction("getVideoDisplayLeft", &IMediaPlayerServer::IgetVideoDisplayLeft);
    addFunction("getVideoDisplayTop", &IMediaPlayerServer::IgetVideoDisplayTop);
    addFunction("getVideoDisplayWidth", &IMediaPlayerServer::IgetVideoDisplayWidth);
    addFunction("getVideoDisplayHeight", &IMediaPlayerServer::IgetVideoDisplayHeight);

    addFunction("setMuteFlag", &IMediaPlayerServer::IsetMuteFlag);
    addFunction("getMuteFlag", &IMediaPlayerServer::IgetMuteFlag);

    addFunction("setNativeUIFlag", &IMediaPlayerServer::IsetNativeUIFlag);
    addFunction("getNativeUIFlag", &IMediaPlayerServer::IgetNativeUIFlag);

    addFunction("setMuteUIFlag", &IMediaPlayerServer::IsetMuteUIFlag);
    addFunction("getMuteUIFlag", &IMediaPlayerServer::IgetMuteUIFlag);

    addFunction("setAudioVolumeUIFlag", &IMediaPlayerServer::IsetAudioVolumeUIFlag);
    addFunction("getAudioVolumeUIFlag", &IMediaPlayerServer::IgetAudioVolumeUIFlag);

    addFunction("setAudioTrackUIFlag", &IMediaPlayerServer::IsetAudioTrackUIFlag);
    addFunction("getAudioTrackUIFlag", &IMediaPlayerServer::IgetAudioTrackUIFlag);

    addFunction("setProgressBarUIFlag", &IMediaPlayerServer::IsetProgressBarUIFlag);
    addFunction("getProgressBarUIFlag", &IMediaPlayerServer::IgetProgressBarUIFlag);

    addFunction("setChannelNoUIFlag", &IMediaPlayerServer::IsetChannelNoUIFlag);
    addFunction("getChannelNoUIFlag", &IMediaPlayerServer::IgetChannelNoUIFlag);

    addFunction("getSubtitileFlag", &IMediaPlayerServer::IgetSubtitileFlag);
    addFunction("setSubtitileFlag", &IMediaPlayerServer::IsetSubtitileFlag);
    addFunction("getSubtitleFlag", &IMediaPlayerServer::IgetSubtitleFlag);
    addFunction("setSubtitleFlag", &IMediaPlayerServer::IsetSubtitleFlag);

    addFunction("getVideoAlpha", &IMediaPlayerServer::IgetVideoAlpha);
    addFunction("setVideoAlpha", &IMediaPlayerServer::IsetVideoAlpha);

    addFunction("getAllowTrickmodeFlag", &IMediaPlayerServer::IgetAllowTrickmodeFlag);
    addFunction("setAllowTrickmodeFlag", &IMediaPlayerServer::IsetAllowTrickmodeFlag);

    addFunction("getCycleFlag", &IMediaPlayerServer::IgetCycleFlag);
    addFunction("setCycleFlag", &IMediaPlayerServer::IsetCycleFlag);

    addFunction("getRandomFlag", &IMediaPlayerServer::IgetRandomFlag);
    addFunction("setRandomFlag", &IMediaPlayerServer::IsetRandomFlag);

    addFunction("getVendorSpecificAttr", &IMediaPlayerServer::IgetVendorSpecificAttr);
    addFunction("setVendorSpecificAttr", &IMediaPlayerServer::IsetVendorSpecificAttr);

    addFunction("getMediaCode", &IMediaPlayerServer::IgetMediaCode);
    addFunction("getMediaDuration", &IMediaPlayerServer::IgetMediaDuration);
    addFunction("getCurrentPlayTime", &IMediaPlayerServer::IgetCurrentPlayTime);
    addFunction("getPlaybackMode", &IMediaPlayerServer::IgetPlaybackMode);
    addFunction("getChannelNum", &IMediaPlayerServer::IgetChannelNum);
    addFunction("getCurrentAudioChannel", &IMediaPlayerServer::IgetCurrentAudioChannel);
    addFunction("getAudioTrack", &IMediaPlayerServer::IgetAudioTrack);
    addFunction("getSubtitle", &IMediaPlayerServer::IgetSubtitle);
    addFunction("getMediaCount", &IMediaPlayerServer::IgetMediaCount);
    addFunction("getCurrentIndex", &IMediaPlayerServer::IgetCurrentIndex);
    addFunction("getEntryID", &IMediaPlayerServer::IgetEntryID);
    addFunction("getPlaylist", &IMediaPlayerServer::IgetPlaylist);
    addFunction("releaseMediaPlayer", &IMediaPlayerServer::IreleaseMediaPlayer);
    addFunction("initMediaPlayer", &IMediaPlayerServer::IinitMediaPlayer);
    addFunction("setSingleMedia", &IMediaPlayerServer::IsetSingleMedia);
    addFunction("joinChannel", &IMediaPlayerServer::IjoinChannel);
    addFunction("leaveChannel", &IMediaPlayerServer::IleaveChannel);
    addFunction("addSingleMedia", &IMediaPlayerServer::IaddSingleMedia);
    addFunction("addBatchMedia", &IMediaPlayerServer::IaddBatchMedia);
    addFunction("clearAllMedia", &IMediaPlayerServer::IclearAllMedia);
    addFunction("removeMediaByIndex", &IMediaPlayerServer::IremoveMediaByIndex);
    addFunction("removeMediaByEntryID", &IMediaPlayerServer::IremoveMediaByEntryID);
    addFunction("moveMediaByIndex", &IMediaPlayerServer::ImoveMediaByIndex);
    addFunction("moveMediaByOffset", &IMediaPlayerServer::ImoveMediaByOffset);
    addFunction("moveMediaByIndex1", &IMediaPlayerServer::ImoveMediaByIndex1);
    addFunction("moveMediaByOffset1", &IMediaPlayerServer::ImoveMediaByOffset1);
    addFunction("moveMediaToNext", &IMediaPlayerServer::ImoveMediaToNext);
    addFunction("moveMediaToPrevious", &IMediaPlayerServer::ImoveMediaToPrevious);
    addFunction("moveMediaToFirst", &IMediaPlayerServer::ImoveMediaToFirst);
    addFunction("moveMediaToLast", &IMediaPlayerServer::ImoveMediaToLast);
    addFunction("moveMediaToNext1", &IMediaPlayerServer::ImoveMediaToNext1);
    addFunction("moveMediaToPrevious1", &IMediaPlayerServer::ImoveMediaToPrevious1);
    addFunction("moveMediaToFirst1", &IMediaPlayerServer::ImoveMediaToFirst1);
    addFunction("moveMediaToLast1", &IMediaPlayerServer::ImoveMediaToLast1);
    addFunction("selectMediaByIndex", &IMediaPlayerServer::IselectMediaByIndex);
    addFunction("selectMediaByOffset", &IMediaPlayerServer::IselectMediaByOffset);
    addFunction("selectMediaByEntryID", &IMediaPlayerServer::IselectMediaByEntryID);
    addFunction("selectNext", &IMediaPlayerServer::IselectNext);
    addFunction("selectPrevious", &IMediaPlayerServer::IselectPrevious);
    addFunction("selectFirst", &IMediaPlayerServer::IselectFirst);
    addFunction("selectLast", &IMediaPlayerServer::IselectLast);
    addFunction("play", &IMediaPlayerServer::Iplay);
    addFunction("playFromStart", &IMediaPlayerServer::IplayFromStart);
    addFunction("playByTime", &IMediaPlayerServer::IplayByTime);
    addFunction("pause", &IMediaPlayerServer::Ipause);
    addFunction("fastForward", &IMediaPlayerServer::IfastForward);
    addFunction("fastRewind", &IMediaPlayerServer::IfastRewind);
    addFunction("resume", &IMediaPlayerServer::Iresume);
    addFunction("gotoEnd", &IMediaPlayerServer::IgotoEnd);
    addFunction("gotoStart", &IMediaPlayerServer::IgotoStart);
    addFunction("stop", &IMediaPlayerServer::Istop);
    addFunction("refreshVideoDisplay", &IMediaPlayerServer::IrefreshVideoDisplay);
    addFunction("switchAudioChannel", &IMediaPlayerServer::IswitchAudioChannel);
    addFunction("switchAudioTrack", &IMediaPlayerServer::IswitchAudioTrack);
    addFunction("switchSubtitle", &IMediaPlayerServer::IswitchSubtitle);
    addFunction("sendVendorSpecificCommand", &IMediaPlayerServer::IsendVendorSpecificCommand);
    addFunction("getAudioPID", &IMediaPlayerServer::IgetAudioPID);
    addFunction("getAudioPIDs", &IMediaPlayerServer::IgetAudioPIDs);
    addFunction("setAudioPID", &IMediaPlayerServer::IsetAudioPID);
    addFunction("getSubtitlePID", &IMediaPlayerServer::IgetSubtitlePID);
    addFunction("getSubtitlePIDs", &IMediaPlayerServer::IgetSubtitlePIDs);
    addFunction("setSubtitlePID", &IMediaPlayerServer::IsetSubtitlePID);
    addFunction("setVolume", &IMediaPlayerServer::IsetVolume);
    addFunction("getVolume", &IMediaPlayerServer::IgetVolume);
    addFunction("set", &IMediaPlayerServer::Iset);
    addFunction("get", &IMediaPlayerServer::Iget);
    addFunction("getAllAudioTrackInfo", &IMediaPlayerServer::IgetAllAudioTrackInfo);
    addFunction("getCurrentAudioTrackInfo", &IMediaPlayerServer::IgetCurrentAudioTrackInfo);
    addFunction("getAllSubtitleInfo", &IMediaPlayerServer::IgetAllSubtitleInfo);
    addFunction("getCurrentSubtitleInfo", &IMediaPlayerServer::IgetCurrentSubtitleInfo);
    addFunction("getAllTeletextInfo", &IMediaPlayerServer::IgetAllTeletextInfo);
    addFunction("selectAudio", &IMediaPlayerServer::IselectAudio);
    addFunction("selectSubtitle", &IMediaPlayerServer::IselectSubtitle);
    addFunction("selectTeletext", &IMediaPlayerServer::IselectTeletext);
    addFunction("setMacrovisionFlag", &IMediaPlayerServer::IsetMacrovisionFlag);
    addFunction("setHDCPFlag", &IMediaPlayerServer::IsetHDCPFlag);
    addFunction("setCGMSAFlag", &IMediaPlayerServer::IsetCGMSAFlag);
}

int 
IMediaPlayerServer::IcreatePlayerInstance(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    std::string name;
    self->popArg("name", name);
    self->freeStack();
    int ret = self->createPlayerInstance(name.c_str());

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return 0;
}

int 
IMediaPlayerServer::IgetNativePlayerInstanceID(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    long playID = self->getNativePlayerInstanceID();

    self->initStack(SP_Result);
    self->pushArg("playID", playID);
    return 0;
}

int 
IMediaPlayerServer::IbindNativePlayerInstance(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long playID;
    self->popArg("playID", playID);
    self->freeStack();
    self->bindNativePlayerInstance(playID);

    self->initStack(SP_Result);
    return 0;
}

int IMediaPlayerServer::IHandleEvent(void* obj)
{
    //TODO
    return 0;
}

int 
IMediaPlayerServer::IsetSingleOrPlaylistMode(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long mode = 0;
    self->popArg("mode", mode);
    self->freeStack();
    self->setSingleOrPlaylistMode(mode);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgetSingleOrPlaylistMode(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->getSingleOrPlaylistMode();

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IsetVideoDisplayMode(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long mode = 0;
    self->popArg("mode", mode);
    self->freeStack();
    self->setVideoDisplayMode(mode);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgetVideoDisplayMode(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    long mode = self->getVideoDisplayMode();

    self->initStack(SP_Result);
    self->pushArg("mode", mode);
    return 0;
}

int 
IMediaPlayerServer::IsetVideoDisplayArea(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long left = 0, top = 0, width = 0, height = 0; 
    self->popArg("left", left);
    self->popArg("top", top);
    self->popArg("width", width);
    self->popArg("height", height);
    self->freeStack();
    self->setVideoDisplayArea(left, top, width, height);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgetVideoDisplayLeft(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    long left = self->getVideoDisplayLeft();

    self->initStack(SP_Result);
    self->pushArg("left", left);
    return 0;
}

int
IMediaPlayerServer::IgetVideoDisplayTop(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    long top = self->getVideoDisplayTop();

    self->initStack(SP_Result);
    self->pushArg("top", top);
    return 0;
}

int 
IMediaPlayerServer::IgetVideoDisplayWidth(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    long width = self->getVideoDisplayWidth();

    self->initStack(SP_Result);
    self->pushArg("width", width);
    return 0;
}

int 
IMediaPlayerServer::IgetVideoDisplayHeight(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    long height = self->getVideoDisplayHeight();

    self->initStack(SP_Result);
    self->pushArg("height", height);
    return 0;
}

int 
IMediaPlayerServer::IsetMuteFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    bool muteFlag = 0;
    self->popArg("muteFlag", muteFlag);
    self->freeStack();
    self->setMuteFlag(muteFlag);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgetMuteFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int muteFlag = self->getMuteFlag();

    self->initStack(SP_Result);
    self->pushArg("muteFlag", muteFlag);
    return muteFlag;
}

int 
IMediaPlayerServer::IsetNativeUIFlag(void* obj)
{ 
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    bool nativeUIFlag = 0;
    self->popArg("nativeUIFlag", nativeUIFlag);
    self->freeStack();
    self->setNativeUIFlag(nativeUIFlag);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgetNativeUIFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int nativeUIFlag = self->getNativeUIFlag();

    self->initStack(SP_Result);
    self->pushArg("nativeUIFlag", nativeUIFlag);
    return nativeUIFlag;
}

int 
IMediaPlayerServer::IsetMuteUIFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    bool muteUIFlag = 0;
    self->popArg("muteUIFlag", muteUIFlag);
    self->freeStack();
    self->setMuteUIFlag(muteUIFlag);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgetMuteUIFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int muteUIFlag = self->getMuteUIFlag();

    self->initStack(SP_Result);
    self->pushArg("muteUIFlag", muteUIFlag);
    return muteUIFlag;
}

int 
IMediaPlayerServer::IsetAudioVolumeUIFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    bool audioVolumeUIFlag = 0;
    self->popArg("audioVolumeUIFlag", audioVolumeUIFlag);
    self->freeStack();
    self->setAudioVolumeUIFlag(audioVolumeUIFlag);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgetAudioVolumeUIFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int audioVolumeUIFlag = self->getAudioVolumeUIFlag();

    self->initStack(SP_Result);
    self->pushArg("audioVolumeUIFlag", audioVolumeUIFlag);
    return 0;
}

int 
IMediaPlayerServer::IsetAudioTrackUIFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    bool audioTrackUIFlag = 0;
    self->popArg("audioTrackUIFlag", audioTrackUIFlag);
    self->freeStack();
    self->setAudioTrackUIFlag(audioTrackUIFlag);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgetAudioTrackUIFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int audioTrackUIFlag = self->getAudioVolumeUIFlag();

    self->initStack(SP_Result);
    self->pushArg("audioTrackUIFlag", audioTrackUIFlag);
    return 0;
}

int
IMediaPlayerServer::IsetProgressBarUIFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    bool progressBarUIFlag = 0;
    self->popArg("progressBarUIFlag", progressBarUIFlag);
    self->freeStack();
    self->setProgressBarUIFlag(progressBarUIFlag );
    
    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgetProgressBarUIFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int progressBarUIFlag = self->getProgressBarUIFlag();

    self->initStack(SP_Result);
    self->pushArg("progressBarUIFlag", progressBarUIFlag);
    return 0;
}

int 
IMediaPlayerServer::IsetChannelNoUIFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    bool channelNoUIFlag = 0;
    self->popArg("channelNoUIFlag", channelNoUIFlag);
    self->freeStack();
    self->setChannelNoUIFlag(channelNoUIFlag);

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IgetChannelNoUIFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int channelNoUIFlag = self->getChannelNoUIFlag();

    self->initStack(SP_Result);
    self->pushArg("channelNoUIFlag", channelNoUIFlag);
    return 0;
}

int 
IMediaPlayerServer::IgetSubtitileFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int subtitileFlag = self->getSubtitileFlag();

    self->initStack(SP_Result);
    self->pushArg("subtitileFlag", subtitileFlag);
    return 0;
}

int 
IMediaPlayerServer::IsetSubtitileFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    bool subtitileFlag = 0;
    self->popArg("subtitileFlag", subtitileFlag);
    self->freeStack();
    self->setSubtitileFlag(subtitileFlag);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgetSubtitleFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int subtitleFlag = self->getSubtitleFlag();

    self->initStack(SP_Result);
    self->pushArg("subtitleFlag", subtitleFlag);
    return 0;
}

int 
IMediaPlayerServer::IsetSubtitleFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    bool subtitleFlag = 0;
    self->popArg("subtitleFlag", subtitleFlag);
    self->freeStack();
    self->setSubtitleFlag(subtitleFlag );

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgetVideoAlpha(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int videoAlpha = self->getVideoAlpha();

    self->initStack(SP_Result);
    self->pushArg("videoAlpha", videoAlpha);
    return 0;
}

int 
IMediaPlayerServer::IsetVideoAlpha(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long videoAlpha = 0;
    self->popArg("videoAlpha", videoAlpha);
    self->freeStack();
    self->setVideoAlpha(videoAlpha);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgetAllowTrickmodeFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int allowTrickmodeFlag  = self->popArg("allowTrickmodeFlag", allowTrickmodeFlag);

    self->initStack(SP_Result);
    self->pushArg("allowTrickmodeFlag", allowTrickmodeFlag);
    return 0;
}

int
IMediaPlayerServer::IsetAllowTrickmodeFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    bool allowTrickmodeFlag = 0;
    self->popArg("allowTrickmodeFlag", allowTrickmodeFlag);
    self->freeStack();
    self->setAllowTrickmodeFlag(allowTrickmodeFlag);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgetCycleFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int cycleFlag = self->getCycleFlag();

    self->initStack(SP_Result);
    self->pushArg("cycleFlag", cycleFlag); 
    return 0;
}

int 
IMediaPlayerServer::IsetCycleFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    bool cycleFlag = 0;
    self->popArg("cycleFlag", cycleFlag);
    self->freeStack();
    self->setCycleFlag(cycleFlag);

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IgetRandomFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int randomFlag = self->getRandomFlag();

    self->initStack(SP_Result);
    self->pushArg("randomFlag", randomFlag);
    return 0;
}

int 
IMediaPlayerServer::IsetRandomFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    bool randomFlag = 0;
    self->popArg("randomFlag", randomFlag);
    self->freeStack();
    self->setRandomFlag(randomFlag );

    self->initStack(SP_Result);
    return 0;
}

int IMediaPlayerServer::IgetVendorSpecificAttr(void* obj)
{
    //TOOD
    return 0;
}
int IMediaPlayerServer::IsetVendorSpecificAttr(void* obj)
{
    //TODO
    return 0;
}

int IMediaPlayerServer::IgetMediaCode(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char mediaCode[IJSEBUFFSIZE + 1] = "";
    int ret = self->getMediaCode(mediaCode);

    self->initStack(SP_Result);
    self->pushArg("mediaCode", mediaCode);
    self->pushArg("return", ret);
    return ret;
}

int IMediaPlayerServer::IgetMediaDuration(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int mediaDuration = self->getMediaDuration();

    self->initStack(SP_Result);
    self->pushArg("mediaDuration", mediaDuration);
    return 0;
}

int IMediaPlayerServer::IgetCurrentPlayTime(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char currentPlayTime[IJSEBUFFSIZE + 1] = "";
    int ret = self->getCurrentPlayTime(currentPlayTime);

    self->initStack(SP_Result);
    self->pushArg("currentPlayTime", currentPlayTime);
    self->pushArg("return", ret);
    return ret;
}

int 
IMediaPlayerServer::IgetPlaybackMode(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char playbackMode[IJSEBUFFSIZE + 1] = "";
    int ret = self->getPlaybackMode(playbackMode);

    self->initStack(SP_Result);
    self->pushArg("playbackMode", playbackMode);
    self->pushArg("return", ret);
    return 0;
}

int 
IMediaPlayerServer::IgetChannelNum(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    long channelNum = self->getChannelNum();

    self->initStack(SP_Result);
    self->pushArg("channelNum", channelNum);
    return 0;
}

int 
IMediaPlayerServer::IgetCurrentAudioChannel(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char currentAudioChannel[IJSEBUFFSIZE + 1] = "";
    int ret = self->getCurrentAudioChannel(currentAudioChannel);

    self->initStack(SP_Result);
    self->pushArg("currentAudioChannel", currentAudioChannel);
    self->pushArg("return", ret);
    return ret;
}

int 
IMediaPlayerServer::IgetAudioTrack(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char audioTrack[IJSEBUFFSIZE + 1] = "";
    int ret = self->getAudioTrack(audioTrack);

    self->initStack(SP_Result);
    self->pushArg("audioTrack", audioTrack);
    self->pushArg("return", ret);
    return ret;
}

int 
IMediaPlayerServer::IgetSubtitle(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char subtitle[IJSEBUFFSIZE + 1] = "";
    int ret = self->getSubtitle(subtitle);

    self->initStack(SP_Result);
    self->pushArg("subtitle", subtitle);
    self->pushArg("return", ret);
    return ret;
}

int 
IMediaPlayerServer::IgetMediaCount(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    long mediaCount = self->getMediaCount();

    self->initStack(SP_Result);
    self->pushArg("mediaCount", mediaCount);
    return 0;
}

int IMediaPlayerServer::IgetCurrentIndex(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    long currentIndex = self->getCurrentIndex();

    self->initStack(SP_Result);
    self->pushArg("currentIndex", currentIndex);
    return 0;
}

int
IMediaPlayerServer::IgetEntryID(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char entryID[IJSEBUFFSIZE + 1] = "";
    int ret = self->getEntryID(entryID);

    self->initStack(SP_Result);
    self->pushArg("entryID", entryID);
    self->pushArg("return", ret);
    return ret;
}

int
IMediaPlayerServer::IgetPlaylist(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char playlist[IJSEBUFFSIZE + 1] = "";
    int ret = self->getPlaylist(playlist);

    self->initStack(SP_Result);
    self->pushArg("playlist", playlist);
    self->pushArg("return", ret); 
    return ret;
}

int 
IMediaPlayerServer::IreleaseMediaPlayer(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long nativePlayerInstanceID = 0;
    self->popArg("nativePlayerInstanceID", nativePlayerInstanceID);
    self->freeStack();
    self->releaseMediaPlayer(nativePlayerInstanceID);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IinitMediaPlayer(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long nativePlayerInstanceID = 0;
    long playlistFlag = 0;
    long videoDisplayMode = 0;
    long height = 0;
    long width = 0;
    long left = 0;
    long top = 0; 
    bool muteFlag = false;
    bool useNativeUIFlag = false;
    bool subtitleFlag = false;
    long videoAlpha = 0;
    bool cycleFlag = false;
    bool randomFlag = false;
    self->popArg("nativePlayerInstanceID", nativePlayerInstanceID);
    self->popArg("playlistFlag", playlistFlag);
    self->popArg("videoDisplayMode", videoDisplayMode);
    self->popArg("height", height);
    self->popArg("width", width);
    self->popArg("left", left);
    self->popArg("top", top);
    self->popArg("muteFlag", muteFlag);
    self->popArg("useNativeUIFlag", useNativeUIFlag);
    self->popArg("subtitleFlag", subtitleFlag);
    self->popArg("videoAlpha", videoAlpha);
    self->popArg("cycleFlag", cycleFlag);
    self->popArg("randomFlag", randomFlag);
    self->freeStack();
    self->initMediaPlayer(nativePlayerInstanceID, playlistFlag, videoDisplayMode, height, width, left, top, muteFlag, useNativeUIFlag, subtitleFlag, videoAlpha, cycleFlag, randomFlag);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IsetSingleMedia(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    char mediaStr[IJSEBUFFSIZE + 1] = "";
    self->popArg("mediaStr", mediaStr, IJSEBUFFSIZE);
    self->freeStack();
    self->setSingleMedia(mediaStr);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IjoinChannel(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long userChannelId = 0; 
    self->popArg("userChannelId", userChannelId);
    self->freeStack();
    int ret = self->joinChannel(userChannelId);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return 0;
}

int 
IMediaPlayerServer::IleaveChannel(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    int ret = self->leaveChannel();

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return 0;
}

int 
IMediaPlayerServer::IaddSingleMedia(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long tIndex = 0;
    char mediaStr[IJSEBUFFSIZE + 1] = "";
    self->popArg("index", tIndex);
    self->popArg("mediaStr", mediaStr, IJSEBUFFSIZE);
    self->freeStack();
    self->addSingleMedia(tIndex, mediaStr);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IaddBatchMedia(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    std::string batchMediaStr;
    self->popArg("batchMediaStr", batchMediaStr);
    self->freeStack();
    self->addBatchMedia(batchMediaStr.c_str());

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IclearAllMedia(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->clearAllMedia();

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IremoveMediaByIndex(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long tIndex = 0;
    self->popArg("index", tIndex);
    self->freeStack();
    self->removeMediaByIndex(tIndex);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IremoveMediaByEntryID(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    std::string entryID;
    self->popArg("entryID", entryID);
    self->freeStack();
    self->removeMediaByEntryID(entryID.c_str());

    self->initStack(SP_Result);
    return 0;
}

int IMediaPlayerServer::ImoveMediaByIndex(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    std::string entryID;
    long toIndex = 0;;
    self->popArg("entryID", entryID); 
    self->popArg("toIndex", toIndex);
    self->freeStack();
    self->moveMediaByIndex(entryID.c_str(), toIndex);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::ImoveMediaByOffset(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    std::string entryID;
    long offset;
    self->popArg("entryID", entryID);
    self->popArg("offset", offset);
    self->freeStack();
    self->moveMediaByOffset(entryID.c_str(), offset);

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::ImoveMediaByIndex1(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long tIndex = 0;
    long toIndex = 0;
    self->popArg("index", tIndex);
    self->popArg("toIndex", toIndex);
    self->freeStack();
    self->moveMediaByIndex1(tIndex, toIndex);

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::ImoveMediaByOffset1(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long tIndex = 0, offset = 0;
    self->popArg("index", tIndex);
    self->popArg("offset", offset);
    self->freeStack();
    self->moveMediaByOffset1(tIndex, offset);

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::ImoveMediaToNext(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    std::string entryID;
    self->popArg("entryID", entryID);
    self->freeStack();
    self->moveMediaToNext(entryID.c_str());

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::ImoveMediaToPrevious(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    std::string entryID;
    self->popArg("entryID", entryID);
    self->freeStack();
    self->moveMediaToPrevious(entryID.c_str());

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::ImoveMediaToFirst(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    std::string entryID;
    self->popArg("entryID", entryID);
    self->freeStack();
    self->moveMediaToFirst(entryID.c_str());

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::ImoveMediaToLast(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    std::string entryID;
    self->popArg("entryID", entryID);
    self->freeStack();
    self->moveMediaToLast(entryID.c_str());

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::ImoveMediaToNext1(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long tIndex;
    self->popArg("index", tIndex);
    self->freeStack();
    self->moveMediaToNext1(tIndex);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::ImoveMediaToPrevious1(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long tIndex;
    self->popArg("index", tIndex);
    self->freeStack();
    self->moveMediaToPrevious1(tIndex);

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::ImoveMediaToFirst1(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long tIndex;
    self->popArg("index", tIndex);
    self->freeStack();
    self->moveMediaToFirst1(tIndex);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::ImoveMediaToLast1(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long tIndex;
    self->popArg("index", tIndex);
    self->freeStack();
    self->moveMediaToLast1(tIndex);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IselectMediaByIndex(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long tIndex;
    self->popArg("index", tIndex);
    self->freeStack();
    self->selectMediaByIndex(tIndex);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IselectMediaByOffset(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long offset;
    self->popArg("offset", offset);
    self->freeStack();
    self->selectMediaByOffset(offset);

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IselectMediaByEntryID(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    std::string entryID;
    self->popArg("entryID", entryID);
    self->freeStack();
    self->selectMediaByEntryID(entryID.c_str());

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IselectNext(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->selectNext();

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IselectPrevious(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->selectPrevious();

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IselectFirst(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->selectFirst();

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IselectLast(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->selectLast();

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::Iplay(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->play();

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IplayFromStart(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->playFromStart();

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IplayByTime(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long type = 0;
    std::string timestamp;
    float speed = 0;
    self->popArg("type", type);
    self->popArg("timestamp", timestamp);
    self->popArg("speed", speed);
    self->freeStack();
    self->playByTime(type, timestamp.c_str(), speed);

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::Ipause(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->pause();

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IfastForward(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    float speed = 0;
    self->popArg("speed", speed);
    self->freeStack();
    self->fastForward(speed);

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IfastRewind(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    float speed = 0;
    self->popArg("speed", speed);
    self->freeStack();
    self->fastRewind(speed);

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::Iresume(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->resume();

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IgotoEnd(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->gotoEnd();

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgotoStart(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->gotoStart();

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::Istop(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->stop();

    self->initStack(SP_Result);
    return 0;
}

int IMediaPlayerServer::IrefreshVideoDisplay(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->refreshVideoDisplay();

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IswitchAudioChannel(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    std::string audioChnl;
    self->popArg("audioChnl", audioChnl);
    self->freeStack();
    self->switchAudioChannel(audioChnl.c_str());

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IswitchAudioTrack(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->switchAudioTrack();

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IswitchSubtitle(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    self->switchSubtitle();

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IsendVendorSpecificCommand(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    std::string xml;
    self->popArg("xml", xml);
    self->freeStack();
    self->sendVendorSpecificCommand(xml.c_str());

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IgetAudioPID(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    long audioPID = self->getAudioPID();

    self->initStack(SP_Result);
    self->pushArg("audioPID", audioPID);
    return 0;
}

int
IMediaPlayerServer::IgetAudioPIDs(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char audioPIDs[IJSEBUFFSIZE + 1] = "";
    int ret = self->getAudioPIDs(audioPIDs);

    self->initStack(SP_Result);
    self->pushArg("audioPIDs", audioPIDs);
    self->pushArg("return", ret);
    return ret;
}

int
IMediaPlayerServer::IsetAudioPID(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long audioPID = 0;
    self->popArg("audioPID", audioPID);
    self->freeStack();
    self->setAudioPID(audioPID);

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IgetSubtitlePID(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    long subtitlePID = self->getSubtitlePID();

    self->initStack(SP_Result);
    self->pushArg("subtitlePID", subtitlePID);
    return 0;
}

int
IMediaPlayerServer::IgetSubtitlePIDs(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char subtitlePIDs[IJSEBUFFSIZE + 1] = "";
    int ret = self->getSubtitlePIDs(subtitlePIDs);

    self->initStack(SP_Result);
    self->pushArg("subtitlePIDs", subtitlePIDs);
    self->pushArg("return", ret); 
    return ret;
}

int
IMediaPlayerServer::IsetSubtitlePID(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long subtitlePID = 0;
    self->popArg("subtitlePID", subtitlePID);
    self->freeStack();
    self->setSubtitlePID(subtitlePID);

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IsetVolume(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long val = 0;
    self->popArg("val", val);
    self->freeStack();
    self->setVolume(val);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IgetVolume(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    long val = self->getVolume();

    self->initStack(SP_Result);
    self->pushArg("val", val);
    return 0;
}

int
IMediaPlayerServer::Iset(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    std::string ioStr;
    std::string wrStr;
    self->popArg("ioStr", ioStr);
    self->popArg("wrStr", wrStr);
    self->freeStack();
    self->set(ioStr.c_str(), wrStr.c_str());

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::Iget(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    std::string ioStr;
    self->popArg("ioStr", ioStr);
    self->freeStack();
    char wrStr[IJSEBUFFSIZE + 1] = "";
    int ret = self->get(ioStr.c_str(), wrStr);

    self->initStack(SP_Result);
    self->pushArg("wrStr", wrStr);
    self->pushArg("return", ret);
    return ret;
}

int 
IMediaPlayerServer::IgetAllAudioTrackInfo(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char jsStr[IJSEBUFFSIZE + 1] = "";
    int ret = self->getAllAudioTrackInfo(jsStr);

    self->initStack(SP_Result);
    self->pushArg("jsStr", jsStr);
    self->pushArg("return", ret);
    return ret;
}

int
IMediaPlayerServer::IgetCurrentAudioTrackInfo(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char jsStr[IJSEBUFFSIZE + 1] = "";
    int ret = self->getCurrentAudioTrackInfo(jsStr);

    self->initStack(SP_Result);
    self->pushArg("jsStr", jsStr);
    self->pushArg("return", ret);
    return ret;
}

int
IMediaPlayerServer::IgetAllSubtitleInfo(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char jsStr[IJSEBUFFSIZE + 1] = "";
    int ret = self->getAllSubtitleInfo(jsStr);

    self->initStack(SP_Result);
    self->pushArg("jsStr", jsStr);
    self->pushArg("return", ret);
    return ret;
}

int 
IMediaPlayerServer::IgetCurrentSubtitleInfo(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char jsStr[IJSEBUFFSIZE + 1] = "";
    int ret = self->getCurrentSubtitleInfo(jsStr);

    self->initStack(SP_Result);
    self->pushArg("jsStr", jsStr);
    self->pushArg("return", ret);
    return ret;
}

int
IMediaPlayerServer::IgetAllTeletextInfo(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    self->freeStack();
    char jsStr[IJSEBUFFSIZE + 1] = "";
    int ret = self->getAllTeletextInfo(jsStr);

    self->initStack(SP_Result);
    self->pushArg("jsStr", jsStr);
    self->pushArg("return", ret);
    return ret;
}

int
IMediaPlayerServer::IselectAudio(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long audioPID = 0;
    self->popArg("audioPID", audioPID);
    self->freeStack();
    self->selectAudio(audioPID);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IselectSubtitle(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long subtitlePID = 0;
    self->popArg("subtitlePID", subtitlePID);
    self->freeStack();
    self->selectSubtitle(subtitlePID);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IselectTeletext(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long teletextPID = 0;
    self->popArg("teletextPID", teletextPID);
    self->freeStack();
    self->selectTeletext(teletextPID);

    self->initStack(SP_Result);
    return 0;
}

int
IMediaPlayerServer::IsetMacrovisionFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long flag = 0;
    self->popArg("flag", flag);
    self->freeStack();
    self->setMacrovisionFlag(flag);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IsetHDCPFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long flag = 0;
    self->popArg("flag", flag);
    self->freeStack();
    self->setHDCPFlag(flag);

    self->initStack(SP_Result);
    return 0;
}

int 
IMediaPlayerServer::IsetCGMSAFlag(void* obj)
{
    IMediaPlayerServer* self = (IMediaPlayerServer*)obj;

    long flag = 0;
    self->popArg("flag", flag);
    self->freeStack();
    self->setCGMSAFlag(flag);

    self->initStack(SP_Result);
    return 0;
}

//export
IIPCServer* IcreateIMediaPlayer(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);
    return new IMediaPlayerServer;
}
