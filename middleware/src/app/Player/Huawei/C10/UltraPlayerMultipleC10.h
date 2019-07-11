#ifndef _UltraPlayerMultipleC10_H_
#define _UltraPlayerMultipleC10_H_

#include "UltraPlayerMultipleHuawei.h"
//#include "BrowserPlayerReporterHuaweiMultiple.h"
#include "ChannelLogoWidget.h"

#ifdef __cplusplus

namespace Hippo {

class ProgramChannel;

class UltraPlayerMultipleC10 : public UltraPlayerMultipleHuawei {
public:
    UltraPlayerMultipleC10(UltraPlayerClient *, BrowserPlayerReporter *, ProgramChannel *);
    ~UltraPlayerMultipleC10();

    virtual int PlayFirst(void) { return -1; }
    virtual int PlayLast(void) { return -1; }
    virtual int PlayNext(void) { return -1; }
    virtual int PlayPrevious(void) { return -1; }
    virtual int PlayByEntryId(const char *) { return -1; }
    virtual int PlayByIndex(int) { return -1; }
    virtual int PlayByOffset(int) { return -1; }
    virtual int getVodListCurIndex(void) { return -1; }
	virtual int play(unsigned int startTime);
	virtual int close(int mode);
    virtual void SeekToStart(void);
    virtual void handleMessage(Message *msg);
    virtual unsigned int getCurrentTimeString(char *TimeString);
    
	int showChanLogo();
	int hideChanLogo();

    virtual bool onPause();
    virtual bool onPlay();
    virtual bool onStop();
    virtual bool onTrick(int rate);

private:
   // BrowserPlayerReporterHuaweiMultiple mReporterSpec;
    void onProgress(void);

    ChannelLogoWidget *mChannelLogo;
    bool m_isTr069Post;	
};

} // namespace Hippo

#endif // __cplusplus

#endif // _UltraPlayerMultipleC10_H_
