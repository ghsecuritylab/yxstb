#ifndef IMEDIAPLAYERCALLER_H
#define IMEDIAPLAYERCALLER_H

#include "IIPCCaller.h"
#include "IMediaPlayer.h"

class IMediaPlayerCaller : public IIPCCaller, public IMediaPlayer {
public:
    virtual int createPlayerInstance(const char* name);
    virtual long getNativePlayerInstanceID();
    virtual long bindNativePlayerInstance(long id);
    virtual long HandleEvent();

    virtual void setSingleOrPlaylistMode(long mode);
    virtual long getSingleOrPlaylistMode();

    virtual void setVideoDisplayMode(long mode);
    virtual long getVideoDisplayMode();

    virtual long setVideoDisplayArea(long left, long top, long width, long height);
    virtual long getVideoDisplayLeft();
    virtual long getVideoDisplayTop();
    virtual long getVideoDisplayWidth();
    virtual long getVideoDisplayHeight();

    virtual void setMuteFlag(const bool muteFlag);
    virtual int getMuteFlag();

    virtual void setNativeUIFlag(bool nativeUIFlag);
    virtual int getNativeUIFlag();

    virtual void setMuteUIFlag(bool muteUIFlag);
    virtual int getMuteUIFlag();

    virtual void setAudioVolumeUIFlag(bool audioVolumeUIFlag);
    virtual int getAudioVolumeUIFlag();

    virtual void setAudioTrackUIFlag(bool audioTrackUIFlag);
    virtual int getAudioTrackUIFlag();

    virtual void setProgressBarUIFlag(bool progressBarUIFlag);
    virtual int getProgressBarUIFlag();

    virtual void setChannelNoUIFlag(bool channelNoUIFlag);
    virtual int getChannelNoUIFlag();

    virtual int getSubtitileFlag();
    virtual void setSubtitileFlag(bool subtitileFlag);
    virtual int getSubtitleFlag();
    virtual void setSubtitleFlag(bool subtitileFlag);

    virtual int getVideoAlpha();
    virtual void setVideoAlpha(long videoAlpha);

    virtual int getAllowTrickmodeFlag();
    virtual void setAllowTrickmodeFlag(bool allowTrickmodeFlag);

    virtual int getCycleFlag();
    virtual void setCycleFlag(bool cycleFlag);

    virtual int getRandomFlag();
    virtual void setRandomFlag(bool randomFlag);

    virtual char* getVendorSpecificAttr(const char* pVSAttrKey);
    virtual void setVendorSpecificAttr(const char* pVSAttrKey, const char* pVSAttrValue);

    virtual int getMediaCode(char*);
    virtual long getMediaDuration();
    virtual int getCurrentPlayTime(char*);
    virtual int getPlaybackMode(char*);
    virtual long getChannelNum();
    virtual int getCurrentAudioChannel(char*);
    virtual int getAudioTrack(char*);
    virtual int getSubtitle(char*);
    virtual long getMediaCount();
    virtual long getCurrentIndex();
    virtual int  getEntryID(char*);
    virtual int  getPlaylist(char* playlist);
    virtual long releaseMediaPlayer(long nativePlayerInstanceID);
    virtual long initMediaPlayer(long nativePlayerInstanceID, long playlistFlag, long videoDisplayMode, long height, long width, long left, long top, bool muteFlag, bool useNativeUIFlag, bool subtitleFlag, long videoAlpha, bool cycleFlag, bool randomFlag);
    virtual long setSingleMedia(const char* mediaStr);
    virtual long joinChannel(long userChannelId);
    virtual long leaveChannel();
    virtual void addSingleMedia(long, const char* mediaStr);
    virtual void addBatchMedia(const char* batchMediaStr);
    virtual void clearAllMedia();
    virtual void removeMediaByIndex(long pIndex);
    virtual void removeMediaByEntryID(const char* entryID);
    virtual void moveMediaByIndex(const char* entryID, long toIndex);
    virtual void moveMediaByOffset(const char* entryID, long offset);
    virtual void moveMediaByIndex1(long pIndex, long toIndex);
    virtual void moveMediaByOffset1(long pIndex, long offset);
    virtual void moveMediaToNext(const char* entryID);
    virtual void moveMediaToPrevious(const char* entryID);
    virtual void moveMediaToFirst(const char* entryID);
    virtual void moveMediaToLast(const char* entryID);
    virtual void moveMediaToNext1(long pIndex);
    virtual void moveMediaToPrevious1(long pIndex);
    virtual void moveMediaToFirst1(long pIndex);
    virtual void moveMediaToLast1(long pIndex);
    virtual void selectMediaByIndex(long pIndex);
    virtual void selectMediaByOffset(long offset);
    virtual void selectMediaByEntryID(const char* entryID);
    virtual void selectNext();
    virtual void selectPrevious();
    virtual void selectFirst();
    virtual void selectLast();
    virtual void play();
    virtual void playFromStart();
    virtual void playByTime(long type, const char* timestamp, float speed);
    virtual void pause();
    virtual void fastForward(float speed);
    virtual void fastRewind(float speed);
    virtual void resume();
    virtual void gotoEnd();
    virtual void gotoStart();
    virtual void stop();
    virtual void refreshVideoDisplay();
    virtual void switchAudioChannel(const char* audioChnl);
    virtual void switchAudioTrack();
    virtual void switchSubtitle();
    virtual void sendVendorSpecificCommand(const char* xml);
    virtual long getAudioPID();
    virtual int getAudioPIDs(char*);
    virtual void setAudioPID(long audioPID);
    virtual long getSubtitlePID();
    virtual int getSubtitlePIDs(char*);
    virtual void setSubtitlePID(long subtitlePID);
    virtual void setVolume(long val);
    virtual long getVolume();
    virtual void set(const char* ioStr, const char* wrStr);
    virtual int get(const char* ioStr, char*);
    virtual int getAllAudioTrackInfo(char*);
    virtual int getCurrentAudioTrackInfo(char*);
    virtual int getAllSubtitleInfo(char*);
    virtual int getCurrentSubtitleInfo(char*);
    virtual int getAllTeletextInfo(char*);
    virtual void selectAudio(long audioPID);
    virtual void selectSubtitle(long subtitlePID);
    virtual void selectTeletext(long teletextPID);
    virtual long setMacrovisionFlag(long flag);
    virtual long setHDCPFlag(long flag);
    virtual long setCGMSAFlag(long flag);

private:
    IMediaPlayerCaller(const char*, IPCCaller*);
    IMediaPlayerCaller(const IMediaPlayerCaller&);
    IMediaPlayerCaller& operator = (const IMediaPlayerCaller&);

    friend class AgentHelper;
};

#endif //IMEDIAPLAYERCALLER_H

