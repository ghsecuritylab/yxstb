#ifndef _UltraPlayerWebPage_H_
#define _UltraPlayerWebPage_H_

#include "UltraPlayerMultiple.h"

namespace Hippo {

class ProgramChannel;

class UltraPlayerWebPage : public UltraPlayerMultiple {
public:
    UltraPlayerWebPage(UltraPlayerClient *, BrowserPlayerReporter *, ProgramChannel *);
    ~UltraPlayerWebPage();

    /* for resource manager */
    virtual int onAcquireResource() { return 0; }
    virtual int onLoseResource() { return 0; }

    virtual int closeForResource() { return 0; }

    virtual int open() { return -1; }
    virtual int play(unsigned int);
    virtual int seekTo(unsigned int playTime) { return -1; }
    virtual int fastForward(int) { return -1; }
    virtual int fastRewind(int) { return -1; }
    virtual int pause() { return -1; }
    virtual int resume() { return -1; }
    virtual int stop() { return -1; }
    virtual int close(int mode) { return -1; }
    virtual int seekEnd() { return -1; }
    virtual unsigned int getTotalTime() { return 0; }
    virtual unsigned int getCurrentTime() { return 0; }
    virtual unsigned int getCurrentTimeString(char *TimeString) { return 0; }
    virtual int PlayFirst(void) { return -1; }
    virtual int PlayLast(void) { return -1; }
    virtual int PlayNext(void) { return -1; }
    virtual int PlayPrevious(void) { return -1; }
    virtual int PlayByEntryId(const char *) { return -1; }
    virtual int PlayByIndex(int) { return -1; }
    virtual int PlayByOffset(int) { return -1; }
    virtual int getVodListCurIndex(void) { return -1; }
    virtual std::string& getMediaCode() { return m_mediaCode; }
    virtual std::string& getEntryID() { return m_entryId; }

protected:
    /**
     * Subclasses must implement this to receive messages.
     */
    virtual void handleMessage(Message *msg) {}

    ProgramChannel *mProgram;

};

} // namespace Hippo

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int webchannelFlagSet(int flag);
int webchannelFlagGet(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _UltraPlayerWebPage_H_
