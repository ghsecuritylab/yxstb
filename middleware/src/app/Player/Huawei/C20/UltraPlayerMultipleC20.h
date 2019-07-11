#ifndef _TRUNK_EC2108_C27_IPSTB_SRC_APP_PLAYER_HUAWEI_C20_UlTRAPLAYERMULTIPLEC20_H_
#define _TRUNK_EC2108_C27_IPSTB_SRC_APP_PLAYER_HUAWEI_C20_UlTRAPLAYERMULTIPLEC20_H_

#include "UltraPlayerMultipleHuawei.h"
#include "BrowserPlayerReporter.h"

#ifdef __cplusplus

namespace Hippo {

class ProgramChannel;

class UltraPlayerMultipleC20 : public UltraPlayerMultipleHuawei {
public:
    UltraPlayerMultipleC20(UltraPlayerClient *, BrowserPlayerReporter *, ProgramChannel *);
    ~UltraPlayerMultipleC20();
	virtual int play(unsigned int startTime);
	virtual int close(int mode);
	virtual int seekTo(unsigned int playTime);
	virtual unsigned int getTotalTime();
	virtual unsigned int getCurrentTimeString(char *TimeString);
    virtual int PlayFirst(void) { return -1; }
    virtual int PlayLast(void) { return -1; }
    virtual int PlayNext(void) { return -1; }
    virtual int PlayPrevious(void) { return -1; }
    virtual int PlayByEntryId(const char *) { return -1; }
    virtual int PlayByIndex(int) { return -1; }
    virtual int PlayByOffset(int) { return -1; }
    virtual int getVodListCurIndex(void) { return -1; }
    virtual float getRequiredBandwidth(void);
    virtual std::string& getMediaCode();
	static void SetMosaicKey(int pIndex, int key);
	static int GetMosaicIndexByKey(int key);
	void SetMosaicMute(int mode);
	int GetMosaicMute(void);

};

} // namespace Hippo

#endif // __cplusplus

#endif // _UltraPlayerMultipleC20_H_
