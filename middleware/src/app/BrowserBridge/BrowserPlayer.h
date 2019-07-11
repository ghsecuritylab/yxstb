#ifndef _BrowserPlayer_H_
#define _BrowserPlayer_H_

#include "Hippo_PlayerHWBase.h"

#include "Program.h"
#include "UltraPlayerClient.h"

#ifdef __cplusplus

namespace Hippo {

class UltraPlayer;

class BrowserPlayer : public PlayerHWBase, public UltraPlayerClient {
public:
	typedef enum{
		PlayerType_eMain = 0,
		PlayerType_ePIP,
		PlayerType_eMosaic,
		PlayerType_eUnknown
	}player_type_e;

	typedef enum{
	    PlayerMode_eIdle = 0,
        PlayerMode_eLive,
        PlayerMode_eVod,
        PlayerMode_eBGMusic,
    }player_mode_e;
public:
    BrowserPlayer(int id, player_type_e playerInstanceType);
    ~BrowserPlayer();

    virtual int open();
    virtual int play(int startTime, time_type_e timeType);
    virtual int fastForward(int speed, unsigned long playTime, time_type_e timeType);
    virtual int fastRewind(int speed, unsigned long playTime, time_type_e timeType);
    virtual int seekTo(unsigned long playTime, time_type_e timeType);
    virtual int seekTo(const char*, time_type_e);
    virtual int pause();
    virtual int resume();
    virtual int stop();
    virtual int close();

    virtual int setProperty(player_property_type_e aType, HPlayerProperty& aValue);
    virtual int getProperty(player_property_type_e aType, HPlayerProperty& aResult);

    virtual int setSingleMedia(media_info_type_e, const char* mediaString);

    virtual int addSingleMedia(media_info_type_e eType, int aIdx, const char* str) {return 0;}
    virtual int removePlayNode(playlist_op_type_e, HPlaylistProperty& aValue) {return 0;}
    virtual int movePlayNode(playlist_op_type_e, HPlaylistProperty& aValue) {return 0;}
    virtual int selectPlayNode(playlist_op_type_e, HPlaylistProperty& aValue) {return 0;}
    virtual const char * get(const char * ioStr);
    virtual void clearForRecycle();

    virtual Type type() { return Browser; }
    virtual int instanceId() { int id; getInstanceId(id); return id; }

    virtual void onDestroy();
	void SetPlayInstanceType(player_type_e playerInstanceType) { mPlayerInstanceType = playerInstanceType; }
protected:
	UltraPlayer *mActualPlayer;
	int mPlayerMagic;
	int mPlaylistMode;
	int mPlaylistCycleFlag;
	int mPlaylistRandomFlag;
	int mSubtitleFlag;
	int mMuteFlag;
	int mMacrovisionFlag;
	int mHDCPFlag;
	int mCGSMAFlag;
	Program *mProgramToPlay;
	player_type_e mPlayerInstanceType;
protected:
    unsigned int GetDuration(void);
    unsigned int GetCurTime(HString& TimeString);
private:
	std::string mChannelInfo;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserPlayer_H_
