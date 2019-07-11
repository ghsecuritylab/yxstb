#ifndef _UltraPlayerMultipleHuawei_H_
#define _UltraPlayerMultipleHuawei_H_

#include "UltraPlayerMultiple.h"

namespace Hippo {

class ProgramChannel;
class BrowserPlayerReporterHuawei;

class UltraPlayerMultipleHuawei : public UltraPlayerMultiple {
public:
    UltraPlayerMultipleHuawei(UltraPlayerClient *client, BrowserPlayerReporter *, ProgramChannel *);
    ~UltraPlayerMultipleHuawei();

    /* for resource manager */
    virtual float getRequiredBandwidth();

    virtual int onAcquireResource();
    virtual int onLoseResource();

    virtual int closeForResource();

    virtual int open();
    virtual int play(unsigned int startTime);
    virtual int seekTo(unsigned int playTime);
    virtual int fastForward(int);
    virtual int fastRewind(int);
    virtual int pause();
    virtual int resume();
    virtual int stop();
    virtual int close(int mode);
    virtual int seekEnd();
    virtual unsigned int getTotalTime();
    virtual unsigned int getCurrentTime();
    virtual unsigned int getCurrentTimeString(char *TimeString);
    virtual std::string& getMediaCode();
    virtual std::string& getEntryID();

    ProgramChannel *getProgramChannel() {return mProgram; }

protected:
    /**
     * Subclasses must implement this to receive messages.
     */
    virtual void handleMessage(Message *msg);

    ProgramChannel *mProgram;

    unsigned int mSighMagic;
};

} // namespace Hippo

#endif // _UltraPlayerMultipleHuawei_H_
