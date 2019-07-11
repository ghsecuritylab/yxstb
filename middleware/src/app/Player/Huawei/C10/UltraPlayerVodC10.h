#ifndef _UltraPlayerVodC10_H_
#define _UltraPlayerVodC10_H_

#include "UltraPlayerVodHuawei.h"
//#include "BrowserPlayerReporterHuaweiVod.h"

#ifdef __cplusplus

namespace Hippo {

class UltraPlayerVodC10 : public UltraPlayerVodHuawei {
public:
    UltraPlayerVodC10(UltraPlayerClient *, BrowserPlayerReporter *, ProgramVOD *);
    ~UltraPlayerVodC10();
    
    virtual int PlayFirst(void) { return -1; }
    virtual int PlayLast(void) { return -1; }
    virtual int PlayNext(void) { return -1; }
    virtual int PlayPrevious(void) { return -1; }
    virtual int PlayByEntryId(const char *) { return -1; }
    virtual int PlayByIndex(int) { return -1; }
    virtual int PlayByOffset(int) { return -1; }
    virtual int getVodListCurIndex(void) { return -1; }
   
    virtual void SeekToStart(void);
    virtual int close(int);
    virtual bool onPause();
    virtual bool onPlay();
    virtual bool onStop();
    virtual bool onTrick(int rate);

protected:
    virtual void handleMessage(Message * msg);

private:
    //BrowserPlayerReporterHuaweiVod mReporterSpec;
    void onProgress(void);
    bool m_isTr069Post;	
};

} // namespace Hippo

#endif // __cplusplus

#endif // _UltraPlayerVodC10_H_
