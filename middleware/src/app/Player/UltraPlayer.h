#ifndef _UltraPlayer_H_
#define _UltraPlayer_H_

#include "RefCnt.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageHandler.h"
#include "mid_stream.h"

#include "app/Views/Widgets/PlayWidgets/PlayStateWidget.h" 
#include "app/Views/Widgets/PlayWidgets/ChannelNOWidget.h"
#include "app/Views/Widgets/PlayWidgets/ProgressBarWidget.h"
#include "app/Views/Widgets/PlayWidgets/AudioMuteWidget.h"
#include "app/Views/Widgets/PlayWidgets/AudioTrackWidget.h"
#include "app/Views/Widgets/PlayWidgets/AudioVolumeWidget.h"
#include "app/Views/Widgets/PlayWidgets/DolbyWidget.h"
#include "app/Views/Widgets/PlayWidgets/DolbyDownmixWidget.h"
#include "UltraPlayerStatistic/UltraPlayerStatistic.h"
//#include "AudioBGGraphWidget.h";




#ifdef __cplusplus
#include "Hippo_HString.h"
#include <map>
#include "ResourceUser.h"
namespace Hippo {

class Program;
class UltraPlayerClient;
class BrowserPlayerReporter;

class UltraPlayer : public RefCnt, public ResourceUser, public MessageHandler {
public:
    UltraPlayer(UltraPlayerClient *, BrowserPlayerReporter *, Program *);
    ~UltraPlayer();

    typedef enum {
        LastFrameMode = 0,
        BlackScreenMode,
        SlowlySyncMode,
    } PLAYERSTOPMODE;

    static int registerPlayer(int pIndex, UltraPlayer *player);
    static int unregisterPlayer(int pIndex, UltraPlayer *player);
    static UltraPlayer *lockPlayer(int pIndex);
    static void unlockPlayer(int pIndex, UltraPlayer *player);

    /* for resource manager */
    ResourceUser::Type type() { return mResourceUserType; }
    int getRequirement() { return mResourceRequirement; }

    void setIndex(int pIndex) { mIndex = pIndex; }
    int getIndex() { return mIndex; }

    int isFake() { return mIsFake; }

    uint32_t magicNumber() { return mMagicNumber; }

    Program *program() { return mProgram; }

    virtual int open() = 0;
    virtual int play(unsigned int startTime) = 0;
    virtual int seekTo(unsigned int playTime) = 0;
    virtual int fastForward(int) = 0;
    virtual int fastRewind(int) = 0;
    virtual int pause() = 0;
    virtual int resume() = 0;
    virtual int stop() = 0;
    virtual int close(int mode) = 0;
	virtual void SeekToStart(void) { return; }
    virtual int seekEnd() = 0;    virtual unsigned int getTotalTime() = 0;
    virtual unsigned int getCurrentTime() = 0;
    virtual unsigned int getCurrentTimeString(char *TimeString) = 0;
    virtual int PlayFirst(void) = 0;
    virtual int PlayLast(void) = 0;
    virtual int PlayNext(void) = 0;
    virtual int PlayPrevious(void) = 0;
    virtual int PlayByEntryId(const char *) = 0;
    virtual int PlayByIndex(int) = 0;
    virtual int PlayByOffset(int) = 0;
    virtual int getVodListCurIndex(void) = 0;
    virtual std::string& getMediaCode() = 0;
    virtual std::string& getEntryID() = 0;

    virtual bool onPlay() { return false; }
    virtual bool onOpen() { return false; }
    virtual bool onPause() { return false; }
    virtual bool onStop() { return false; }
    virtual bool onTrick(int rate) { return false; }
    /* Audio Control */
    void SetVolume(int);
    int GetVolume(void);
    void SetMute(int);
    int GetMute(void);
    void SetChannel(int);
    int GetChannel(void);
    void SetTrack(int);
    int GetTrack(void);
    int AudioTrackPid(void);

    int GetTrackInfo(char *pTrackInfo);
    int GetAllTeletextInfo(std::string& strValue);
    int GetAllAudioTrackInfo(std::string& strValue);
    int GetAllSubtitleInfo(std::string& strValue);
    int GetCurrentAudioTrackInfo(HString& strValue);
    int GetCurrentSubtitleInfo(HString& strValue);
    int GetCurrentAudioChannel(HString& strValue);
    void selectTeletext(int teltext);
    void SelectSubtitle(int subtitlePid);
    int SubtitleIndex();
    void SelectAudioTrack(int audioTrackPid);
    void SwitchAudioTrack(void);
    void SwitchSubtitle(void);
    void SwitchAudioChannel(HString strValue);
    int  GetSubtitileFlag(void);
    void SetSubtitileFlag(int subtitleFlag);
    void SetMacrovisionFlag(int macrovisionFlag);
    void SetHDCPFlag(int HDCPFlag);
    void SetCGMSAFlag(int CGMSAFlag);
    // int GetPlaybackMode(std::string& strValue);
    int GetPlayBackMode(HString& aValue);
    int GetCurrentPlayUrl(std::string& strValue);
    // int GetCurrentStatus() { return mCurrentStatus; }
    /* Native UI */
    static void enableUI(bool enable) { mUIEnabled = enable; }
    static bool UIIsEnabled() { return mUIEnabled; }
    enum UIFlag_Mask {
        PlayState_Mask    = 0x00000001,
        ChannelNO_Mask    = 0x00000010,
        ProgressBar_Mask  = 0x00000100,
        AudioMute_Mask    = 0x00001000,
        AudioTrack_Mask   = 0x00010000,
        AudioVolume_Mask  = 0x00100000,
        AudioBGGraph_Mask = 0x01000000
    };
    
    static void setVideoClearFlag(int);
    static int getVideoClearFlag();
    
    static void setUIFlags(uint32_t flags);
    static void clearUIFlags(uint32_t flags);

    /* 此函数仅查询可设置的状态，不包含强制标志的检查 */
    static bool getUIFlagState(uint32_t flag);

    /* 设置一些强制设定，对付个别局点的问题 */
    static void setUIFlagsForcedMask(uint32_t flags);
    static void setUIFlagsForcedValue(uint32_t flags);
    static uint32_t getUIFlagsForcedMask(void){return mUIFlagsForcedMask;}
    static uint32_t getUIFlagsForcedValue(void){return mUIFlagsForcedValue;}

    /* 用下面函数检查flag的确切状态，参考了本实例是不是主播放（mUIEnabled）和强制设定 */
    bool UIFlagIsEnabled(uint32_t flag);

    void SetCurrentChannelNum(int currentChannelNum) { mCurrentChannelNum = currentChannelNum;}
    int GetCurrentChannelNum(void) { return mCurrentChannelNum; }
    UltraPlayerClient *GetPlayerClient(){ return mClient; }
    int setStreamVideoLocation(int pIndex, int x, int y, int w, int h, int mode );
    void refreshVideoDisplay();
    int mPlaylistMode;
    int mPlayCycleFlag;                 //根据电信规范0为循环播放，1为播放一次.默认为1
    int mPlayRandomFlag;                //0为顺序播放，1为随机播放.默认为0
    int mDisplayMode;                   //根据规范0为小窗口播放，1为全屏播放，2、3、255
    int m_subtitleFlag;
    int m_muteFlag;
    int m_VideoDisplaytop;
    int m_VideoDisplayleft;
    int m_VideoDisplaywidth;
    int m_VideoDisplayheight;
    int m_PlayerInstanceType; /*0,Main; 1, PIP; 2, MOSAIC */
    BrowserPlayerReporter *reporter() { return mReporter; }
    int getMediaType(void) { return mMediaType; }

protected:
    UltraPlayerClient *mClient;
    BrowserPlayerReporter *mReporter;
    int mIndex;
    int mIsFake;
    uint32_t mMagicNumber;
    APP_TYPE mMediaType;
    int mCurrentChannelNum;
    std::string m_mediaCode;
    std::string m_entryId;

    /* for resource manager */
    ResourceUser::Type mResourceUserType;
    int mResourceRequirement;

    /* Subclasses must implement this to receive messages. */
    virtual void handleMessage(Message *msg);
    PlayStateWidget::State ShowPlayStateIcon(PlayStateWidget::State);
    void ClearAllIcon(void);
    int mPlayStartTime;

private:
    Program *mProgram;
    /* 不是所有播放实例都可以操作界面元素，应该由主的、全屏的控制。其它的播放实例mUIEnabled == false。*/
    static bool mUIEnabled;
    static uint32_t mUIFlags;
    static uint32_t mUIFlagsForcedMask;
    static uint32_t mUIFlagsForcedValue;
    static int mVideoClearFlag;

private:
    void mDolbyDownmixIconShow(void);

public:
    int mCurrentStatus;
    int mCurrentSpeed;
    static PlayStateWidget *mPlayState;
    static ChannelNOWidget *mChannelNO;
    static ProgressBarWidget *mProgressBar;
    static AudioMuteWidget *mAudioMute;
    static AudioTrackWidget *mAudioTrack;
    static AudioVolumeWidget *mAudioVolume;
    static DolbyWidget *mDolbyIcon;
    static DolbyDownmixWidget *mDolbyDownmixIcon;
    //static AudioBGGraphWidget *mAudioBGGraph;
   	int mInstanceId;
public:
    static UltraPlayerStatistic s_statistic;
};

} // namespace Hippo

extern "C" int GetCurrentPlayStatus();


#endif // __cplusplus

#endif // _UltraPlayer_H_
