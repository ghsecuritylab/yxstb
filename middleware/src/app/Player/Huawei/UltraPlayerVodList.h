#ifndef _UltraPlayerVodPlayList_H_
#define _UltraPlayerVodPlayList_H_

#include "UltraPlayerVodHuawei.h"
//#include "BrowserPlayerReporterHuaweiVod.h"
#include "ProgramList.h"


#ifdef __cplusplus

namespace Hippo {

class UltraPlayerVodList : public UltraPlayerVodHuawei {
public:
    UltraPlayerVodList(UltraPlayerClient *client, BrowserPlayerReporter *, ProgramList *, unsigned int);
    ~UltraPlayerVodList();
    
    virtual int PlayFirst(void);
    virtual int PlayLast(void);
    virtual int PlayNext(void);
    virtual int PlayPrevious(void);
    virtual int PlayByEntryId(const char *);
    virtual int PlayByIndex(int);
    virtual int PlayByOffset(int);
    virtual int getVodListCurIndex(void);

protected:
    /**
     * Subclasses must implement this to receive messages.
     */
    virtual void handleMessage(Message *msg);

private:
    ProgramList *mProgramList;
    int mCurrentIndex;
};

}           // namespace Hippo

#endif      // __cplusplus

#endif      //_UltraPlayerVodPlayList_H_

