#ifndef _TRUNK_EC2108_C27_IPSTB_SRC_APP_PLAYER_HUAWEI_C20_UltraPlayerVodC20_H_
#define _TRUNK_EC2108_C27_IPSTB_SRC_APP_PLAYER_HUAWEI_C20_UltraPlayerVodC20_H_

#include "UltraPlayerVodHuawei.h"
#include "BrowserPlayerReporter.h"

#ifdef __cplusplus

namespace Hippo {

class UltraPlayerVodC20 : public UltraPlayerVodHuawei {
public:
    UltraPlayerVodC20(UltraPlayerClient *, BrowserPlayerReporter *, ProgramVOD *);
    ~UltraPlayerVodC20();    
    
    virtual int seekEnd();
    virtual int PlayFirst(void) { return -1; }
    virtual int PlayLast(void) { return -1; }
    virtual int PlayNext(void) { return -1; }
    virtual int PlayPrevious(void) { return -1; }
    virtual int PlayByEntryId(const char *) { return -1; }
    virtual int PlayByIndex(int) { return -1; }
    virtual int PlayByOffset(int) { return -1; }
    virtual int getVodListCurIndex(void) { return -1; }

	virtual void handleMessage(Message *msg);
	
};

} // namespace Hippo

#endif // __cplusplus

#endif // _UltraPlayerVodC20_H_
