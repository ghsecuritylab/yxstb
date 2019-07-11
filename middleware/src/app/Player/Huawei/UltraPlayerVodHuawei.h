#ifndef _UltraPlayerVodHuawei_H_
#define _UltraPlayerVodHuawei_H_

#include "UltraPlayerVod.h"

namespace Hippo {

class ProgramVOD;
class BrowserPlayerReporterHuawei;

class UltraPlayerVodHuawei : public UltraPlayerVod {
public:
    UltraPlayerVodHuawei(UltraPlayerClient *client, BrowserPlayerReporter *, ProgramVOD *);
    ~UltraPlayerVodHuawei();

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

    unsigned int getStartTime() { return mStartTime; }
protected:
    /**
     * Subclasses must implement this to receive messages.
     */
    virtual void handleMessage(Message *msg);

    ProgramVOD *mProgram;

    unsigned int mSighMagic;
private:
	std::string m_pathName;
    unsigned int mStartTime;
};

} // namespace Hippo

#endif // _UltraPlayerVodHuawei_H_
