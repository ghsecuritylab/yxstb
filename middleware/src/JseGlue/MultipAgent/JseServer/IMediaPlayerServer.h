#ifndef IMEDIAPLAYERAGENTSERVER_H
#define IMEDIAPLAYERAGENTSERVER_H

#include "IIPCServer.h"
#include "IMediaPlayerCell.h"

class IMediaPlayerServer : public IIPCServer, public IMediaPlayerCell {
public:
    virtual int attachServer();

    ~IMediaPlayerServer();
private:
    IMediaPlayerServer();
    IMediaPlayerServer(const IMediaPlayerServer&);
    IMediaPlayerServer& operator = (const IMediaPlayerServer&);

    static int IcreatePlayerInstance(void*);
    static int IgetNativePlayerInstanceID(void*);
    static int IbindNativePlayerInstance(void*);
    static int IHandleEvent(void*);

    static int IsetSingleOrPlaylistMode(void*);
    static int IgetSingleOrPlaylistMode(void*);

    static int IsetVideoDisplayMode(void*);
    static int IgetVideoDisplayMode(void*);

    static int IsetVideoDisplayArea(void*);
    static int IgetVideoDisplayLeft(void*);
    static int IgetVideoDisplayTop(void*);
    static int IgetVideoDisplayWidth(void*);
    static int IgetVideoDisplayHeight(void*);

    static int IsetMuteFlag(void*);
    static int IgetMuteFlag(void*);

    static int IsetNativeUIFlag(void*);
    static int IgetNativeUIFlag(void*);

    static int IsetMuteUIFlag(void*);
    static int IgetMuteUIFlag(void*);

    static int IsetAudioVolumeUIFlag(void*);
    static int IgetAudioVolumeUIFlag(void*);

    static int IsetAudioTrackUIFlag(void*);
    static int IgetAudioTrackUIFlag(void*);

    static int IsetProgressBarUIFlag(void*);
    static int IgetProgressBarUIFlag(void*);

    static int IsetChannelNoUIFlag(void*);
    static int IgetChannelNoUIFlag(void*);

    static int IgetSubtitileFlag(void*);
    static int IsetSubtitileFlag(void*);
    static int IgetSubtitleFlag(void*);
    static int IsetSubtitleFlag(void*);

    static int IgetVideoAlpha(void*);
    static int IsetVideoAlpha(void*);

    static int IgetAllowTrickmodeFlag(void*);
    static int IsetAllowTrickmodeFlag(void*);

    static int IgetCycleFlag(void*);
    static int IsetCycleFlag(void*);

    static int IgetRandomFlag(void*);
    static int IsetRandomFlag(void*);

    static int IgetVendorSpecificAttr(void*);
    static int IsetVendorSpecificAttr(void*);

    static int IgetMediaCode(void*);
    static int IgetMediaDuration(void*);
    static int IgetCurrentPlayTime(void*);
    static int IgetPlaybackMode(void*);
    static int IgetChannelNum(void*);
    static int IgetCurrentAudioChannel(void*);
    static int IgetAudioTrack(void*);
    static int IgetSubtitle(void*);
    static int IgetMediaCount(void*);
    static int IgetCurrentIndex(void*);
    static int IgetEntryID(void*);
    static int IgetPlaylist(void*);
    static int IreleaseMediaPlayer(void*);
    static int IinitMediaPlayer(void*);
    static int IsetSingleMedia(void*);
    static int IjoinChannel(void*);
    static int IleaveChannel(void*);
    static int IaddSingleMedia(void*);
    static int IaddBatchMedia(void*);
    static int IclearAllMedia(void*);
    static int IremoveMediaByIndex(void*);
    static int IremoveMediaByEntryID(void*);
    static int ImoveMediaByIndex(void*);
    static int ImoveMediaByOffset(void*);
    static int ImoveMediaByIndex1(void*);
    static int ImoveMediaByOffset1(void*);
    static int ImoveMediaToNext(void*);
    static int ImoveMediaToPrevious(void*);
    static int ImoveMediaToFirst(void*);
    static int ImoveMediaToLast(void*);
    static int ImoveMediaToNext1(void*);
    static int ImoveMediaToPrevious1(void*);
    static int ImoveMediaToFirst1(void*);
    static int ImoveMediaToLast1(void*);
    static int IselectMediaByIndex(void*);
    static int IselectMediaByOffset(void*);
    static int IselectMediaByEntryID(void*);
    static int IselectNext(void*);
    static int IselectPrevious(void*);
    static int IselectFirst(void*);
    static int IselectLast(void*);
    static int Iplay(void*);
    static int IplayFromStart(void*);
    static int IplayByTime(void*);
    static int Ipause(void*);
    static int IfastForward(void*);
    static int IfastRewind(void*);
    static int Iresume(void*);
    static int IgotoEnd(void*);
    static int IgotoStart(void*);
    static int Istop(void*);
    static int IrefreshVideoDisplay(void*);
    static int IswitchAudioChannel(void*);
    static int IswitchAudioTrack(void*);
    static int IswitchSubtitle(void*);
    static int IsendVendorSpecificCommand(void*);
    static int IgetAudioPID(void*);
    static int IgetAudioPIDs(void*);
    static int IsetAudioPID(void*);
    static int IgetSubtitlePID(void*);
    static int IgetSubtitlePIDs(void*);
    static int IsetSubtitlePID(void*);
    static int IsetVolume(void*);
    static int IgetVolume(void*);
    static int Iset(void*);
    static int Iget(void*);
    static int IgetAllAudioTrackInfo(void*);
    static int IgetCurrentAudioTrackInfo(void*);
    static int IgetAllSubtitleInfo(void*);
    static int IgetCurrentSubtitleInfo(void*);
    static int IgetAllTeletextInfo(void*);
    static int IselectAudio(void*);
    static int IselectSubtitle(void*);
    static int IselectTeletext(void*);
    static int IsetMacrovisionFlag(void*);
    static int IsetHDCPFlag(void*);
    static int IsetCGMSAFlag(void*);

    friend IIPCServer* IcreateIMediaPlayer(const char*);
};

extern IIPCServer* IcreateIMediaPlayer(const char*);

#endif //IMEDIAPLAYERAGENTSERVER_H

