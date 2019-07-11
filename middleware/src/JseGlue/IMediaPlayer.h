#ifndef IMEDIAPLAYER_H
#define IMEDIAPLAYER_H

#include "JseGlueMacro.h"

class IMediaPlayer {
public:
    virtual int createPlayerInstance(const char* name) = 0;
    virtual long getNativePlayerInstanceID() = 0;
    virtual long bindNativePlayerInstance(long id) = 0;
    virtual long HandleEvent() = 0;

    virtual void setSingleOrPlaylistMode(long mode) = 0;
    virtual long getSingleOrPlaylistMode() = 0;

    virtual void setVideoDisplayMode(long mode) = 0;
    virtual long getVideoDisplayMode() = 0;

    virtual long setVideoDisplayArea(long left, long top, long width, long height) = 0;
    virtual long getVideoDisplayLeft() = 0;
    virtual long getVideoDisplayTop() = 0;
    virtual long getVideoDisplayWidth() = 0;
    virtual long getVideoDisplayHeight() = 0;

    virtual void setMuteFlag(const bool muteFlag) = 0;
    virtual int getMuteFlag() = 0;

    virtual void setNativeUIFlag(bool nativeUIFlag) = 0;
    virtual int getNativeUIFlag() = 0;

    virtual void setMuteUIFlag(bool muteUIFlag) = 0;
    virtual int getMuteUIFlag() = 0;

    virtual void setAudioVolumeUIFlag(bool audioVolumeUIFlag) = 0;
    virtual int getAudioVolumeUIFlag() = 0;

    virtual void setAudioTrackUIFlag(bool audioTrackUIFlag) = 0;
    virtual int getAudioTrackUIFlag() = 0;

    virtual void setProgressBarUIFlag(bool progressBarUIFlag) = 0;
    virtual int getProgressBarUIFlag() = 0;

    virtual void setChannelNoUIFlag(bool channelNoUIFlag) = 0;
    virtual int getChannelNoUIFlag() = 0;

    virtual int getSubtitileFlag() = 0;
    virtual void setSubtitileFlag(bool subtitileFlag) = 0;
    virtual int getSubtitleFlag() = 0;
    virtual void setSubtitleFlag(bool subtitileFlag) = 0;

    virtual int getVideoAlpha() = 0;
    virtual void setVideoAlpha(long videoAlpha) = 0;

    virtual int getAllowTrickmodeFlag() = 0;
    virtual void setAllowTrickmodeFlag(bool allowTrickmodeFlag) = 0;

    virtual int getCycleFlag() = 0;
    virtual void setCycleFlag(bool cycleFlag) = 0;

    virtual int getRandomFlag() = 0;
    virtual void setRandomFlag(bool randomFlag) = 0;

    virtual int getMediaCode(char*) = 0;
    virtual long getMediaDuration() = 0;
    virtual int getCurrentPlayTime(char*) = 0;
    virtual int getPlaybackMode(char*) = 0;
    virtual long getChannelNum() = 0;
    virtual int getCurrentAudioChannel(char*) = 0;
    virtual int getAudioTrack(char*) = 0;
    virtual int getSubtitle(char*) = 0;
    virtual long getMediaCount() = 0;
    virtual long getCurrentIndex() = 0;
    virtual int getEntryID(char*) = 0;
    virtual int getPlaylist(char*) = 0;
    virtual long releaseMediaPlayer(long nativePlayerInstanceID) = 0;
    virtual long initMediaPlayer(long nativePlayerInstanceID, long playlistFlag, long videoDisplayMode, long height, long width, long left, long top, bool muteFlag, bool useNativeUIFlag, bool subtitleFlag, long videoAlpha, bool cycleFlag, bool randomFlag) = 0;
    virtual long setSingleMedia(const char* mediaStr) = 0;
    virtual long joinChannel(long userChannelId) = 0;
    virtual long leaveChannel() = 0;
    virtual void addSingleMedia(long, const char* mediaStr) = 0;
    virtual void addBatchMedia(const char* batchMediaStr) = 0;
    virtual void clearAllMedia() = 0;
    virtual void removeMediaByIndex(long) = 0;
    virtual void removeMediaByEntryID(const char* entryID) = 0;
    virtual void moveMediaByIndex(const char* entryID, long toIndex) = 0;
    virtual void moveMediaByOffset(const char* entryID, long offset) = 0;
    virtual void moveMediaByIndex1(long, long toIndex) = 0;
    virtual void moveMediaByOffset1(long pIndex, long offset) = 0;
    virtual void moveMediaToNext(const char* entryID) = 0;
    virtual void moveMediaToPrevious(const char* entryID) = 0;
    virtual void moveMediaToFirst(const char* entryID) = 0;
    virtual void moveMediaToLast(const char* entryID) = 0;
    virtual void moveMediaToNext1(long pIndex) = 0;
    virtual void moveMediaToPrevious1(long pIndex) = 0;
    virtual void moveMediaToFirst1(long pIndex) = 0;
    virtual void moveMediaToLast1(long pIndex) = 0;
    virtual void selectMediaByIndex(long pIndex) = 0;
    virtual void selectMediaByOffset(long offset) = 0;
    virtual void selectMediaByEntryID(const char* entryID) = 0;
    virtual void selectNext() = 0;
    virtual void selectPrevious() = 0;
    virtual void selectFirst() = 0;
    virtual void selectLast() = 0;
    virtual void play() = 0;
    virtual void playFromStart() = 0;
    virtual void playByTime(long type, const char* timestamp, float speed) = 0;
    virtual void pause() = 0;
    virtual void fastForward(float speed) = 0;
    virtual void fastRewind(float speed) = 0;
    virtual void resume() = 0;
    virtual void gotoEnd() = 0;
    virtual void gotoStart() = 0;
    virtual void stop() = 0;
    virtual void refreshVideoDisplay() = 0;
    virtual void switchAudioChannel(const char* audioChnl) = 0;
    virtual void switchAudioTrack() = 0;
    virtual void switchSubtitle() = 0;
    virtual void sendVendorSpecificCommand(const char* xml) = 0;
    virtual long getAudioPID() = 0;
    virtual int getAudioPIDs(char*) = 0;
    virtual void setAudioPID(long audioPID) = 0;
    virtual long getSubtitlePID() = 0;
    virtual int getSubtitlePIDs(char*) = 0;
    virtual void setSubtitlePID(long subtitlePID) = 0;
    virtual void setVolume(long val) = 0;
    virtual long getVolume() = 0;
    virtual void set(const char* ioStr, const char* wrStr) = 0;
    virtual int get(const char* ioStr, char*) = 0;
    virtual int getAllAudioTrackInfo(char*) = 0;
    virtual int getCurrentAudioTrackInfo(char*) = 0;
    virtual int getAllSubtitleInfo(char*) = 0;
    virtual int getCurrentSubtitleInfo(char*) = 0;
    virtual int getAllTeletextInfo(char*) = 0;
    virtual void selectAudio(long audioPID) = 0;
    virtual void selectSubtitle(long subtitlePID) = 0;
    virtual void selectTeletext(long teletextPID) = 0;
    virtual long setMacrovisionFlag(long flag) = 0;
    virtual long setHDCPFlag(long flag) = 0;
    virtual long setCGMSAFlag(long flag) = 0;
    virtual ~IMediaPlayer() { }
};

extern IMediaPlayer* createIMediaPlayerCell(const char* name);

#endif //IMEDIAPLAYER_H

