#ifndef _BrowserPlayerC10_H_
#define _BrowserPlayerC10_H_

#include "BrowserPlayer.h"
#include "ProgramList.h"

#ifdef __cplusplus

namespace Hippo {

class BrowserPlayerC10 : public BrowserPlayer {
public:
    BrowserPlayerC10(int id, player_type_e playerInstanceType);
    ~BrowserPlayerC10();

    virtual int joinChannel(int channelNumber);
    virtual int leaveChannel();

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
    
	virtual int addSingleMedia(media_info_type_e, int, const char *);
	virtual int removePlayNode(playlist_op_type_e, HPlaylistProperty& aValue);
	virtual int movePlayNode(playlist_op_type_e, HPlaylistProperty& aValue);
	virtual int selectPlayNode(playlist_op_type_e, HPlaylistProperty& aValue);

    int addBatchMedia(const char*);
/*
    int removeMediaByEntryID(const char*);
    int removeMediaByIndex(int);

    int selectFirst();
    int selectLast();
    int selectNext();
    int selectPrevious();
    int selectMediaByEntryId(const char*);
    int selectMediaByIndex(int);
    int selectMediaByOffset(int);
    int moveMediaByEntryID(const char*, int);
    int moveMediaByEntryIDOffset(const char*, int);
    int moveMediaByIndex(int, int);
    int moveMediaByIndexOffset(int, int);
    int moveMediaToNextByEntryID(const char*);
    int moveMediaToPreviousByEntryID(const char*);
    int moveMediaToFirstByEntryID(const char*);
    int moveMediaToLastByEntryID(const char*);
    int moveMediaToNextByIndex(int);
    int moveMediaToPreviousByIndex(int);
    int moveMediaToFirstByIndex(int);
    int moveMediaToLastByIndex(int);
*/
    //统计和回显
    int getMediaCount();
    int getCurrentIndex();
    const char* getCurrentEntryID();
    int getPlaylist();
    //播放
    int playFromStart();

    virtual void clearForRecycle();
    virtual int refreshVideoDisplay();

private:
    ProgramList *mProgramList;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserPlayerC10_H_
