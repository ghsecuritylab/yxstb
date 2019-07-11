#ifndef _UltraPlayerBGMusic_H_
#define _UltraPlayerBGMusic_H_

#include "UltraPlayer.h"

#ifdef __cplusplus

namespace Hippo {

class UltraPlayerBGMusic : public UltraPlayer {
public:
    UltraPlayerBGMusic(UltraPlayerClient *, BrowserPlayerReporter *, Program *, int delay);
    ~UltraPlayerBGMusic();

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
    virtual int stop();
    virtual int close(int mode);
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

    static int setUrl(const char *);
    static int getUrl(char *);
    static int enable(int);
    static int isEnable();

protected:
    /**
     * Subclasses must implement this to receive messages.
     */
    virtual void handleMessage(Message *msg);

    int mDelay;
private:
    std::string m_mediaCode;
    std::string m_entryId;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _UltraPlayerBGMusic_H_
