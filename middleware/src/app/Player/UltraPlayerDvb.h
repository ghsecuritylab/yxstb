#ifndef _UltraPlayerDvb_H_
#define _UltraPlayerDvb_H_

#include "UltraPlayer.h"

namespace Hippo {
class ProgramChannel;
class BrowserPlayerReporterHuawei;

class UltraPlayerDvb : public UltraPlayer {
public:
    UltraPlayerDvb(UltraPlayerClient *, BrowserPlayerReporter *, ProgramChannel *);
    ~UltraPlayerDvb();

    /* for resource manager */
    virtual int getRequiredProgNumber();
    virtual int getRequiredFrequency();
    virtual int getSpecialDevice();
    virtual int onAcquireResource();
    virtual int onLoseResource();

    virtual int closeForResource();

    virtual int play(unsigned int startTime);
    virtual int stop();
    virtual int close(int mode);
    virtual int open(){return 0;};
    virtual int seekTo(unsigned int playTime);
    virtual int fastForward(int);
    virtual int fastRewind(int);
    virtual int pause();
    virtual int resume();
    virtual int seekEnd();

    virtual unsigned int getTotalTime();
    virtual unsigned int getCurrentTime(){return 0;};
    virtual unsigned int getCurrentTimeString(char *TimeString);
    virtual int PlayFirst(void){return 0;};
    virtual int PlayLast(void){return 0;};
    virtual int PlayNext(void){return 0;};
    virtual int PlayPrevious(void){return 0;};
    virtual int PlayByEntryId(const char *){return 0;};
    virtual int PlayByIndex(int){return 0;}
    virtual int PlayByOffset(int){return 0;};
    virtual int getVodListCurIndex(void){return 0;};
    virtual std::string& getMediaCode();
    virtual std::string& getEntryID() {}
private:
    virtual void handleMessage(Message *msg);
    ProgramChannel *mDvbProgram;
    int m_tunerIndex;
};

} // namespace Hippo

#endif // _UltraPlayerDvb_H_
