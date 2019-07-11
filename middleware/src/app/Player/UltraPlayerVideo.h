#ifndef _UltraPlayerVideo_H_
#define _UltraPlayerVideo_H_

#include "UltraPlayer.h"
#include <string>

#ifdef __cplusplus

namespace Hippo {

class BrowserPlayerReporter;
class Program;

class UltraPlayerVideo : public UltraPlayer {
public:
    UltraPlayerVideo(UltraPlayerClient *client, BrowserPlayerReporter *reporter, Program *program);
    ~UltraPlayerVideo();

    virtual int getRequiredRrogNumber() { return 0; }
    virtual int getRequiredFrequency() { return 0; }
    virtual int getSpecialDevice(){ return 0; }
    virtual int onAcquireResource(){ return 0; }
    virtual int onLoseResource(){ return 0; }
    virtual int closeForResource(){ return 0; }

    virtual int open() { return 0; }
    virtual int play(unsigned int startTime);
    virtual int seekTo(unsigned int playTime);
    virtual int fastForward(int);
    virtual int fastRewind(int);
    virtual int pause();
    virtual int resume();
    virtual int stop() { return 0; }
    virtual int close(int mode);
    virtual int seekEnd() { return 0; }
    virtual unsigned int getTotalTime();
    virtual unsigned int getCurrentTime();
    virtual unsigned int getCurrentTimeString(char *TimeString) { return 0; }
    virtual int PlayFirst(void) { return -1; }
    virtual int PlayLast(void){ return -1; }
    virtual int PlayNext(void){ return -1; }
    virtual int PlayPrevious(void){ return -1; }
    virtual int PlayByEntryId(const char *){ return -1; }
    virtual int PlayByIndex(int){ return -1; }
    virtual int PlayByOffset(int){ return -1; }
    virtual int getVodListCurIndex(void){ return -1; }
	virtual std::string& getMediaCode(){ return m_mediaCode; }
	virtual std::string& getEntryID(){ return m_entryId; }

	void playerUrlSet(char* url) { m_playerUrl = url; }

private:
    std::string m_playerUrl;

};

}

#endif

#endif