#ifndef _Program_H_
#define _Program_H_

#include "RefCnt.h"
#include "VodSource.h"

#ifdef __cplusplus

namespace Hippo {

class Program : public RefCnt {
public:
    Program();
    ~Program();

    enum ProgramType {
    	PT_UNDEFINED = -1,
    	PT_CHANNEL,
    	PT_CHANNEL_NUMBER,
    	PT_VOD,
    	PT_ADV,
    	PT_CHANNEL_DVB,
    	PT_MUSIC,
    };
    virtual ProgramType getType();

    virtual int getNumberID() { return 0; }
    virtual const char *getStringID() { return 0; }
    virtual int GetChanKey() { return 0; }
    virtual VodSource *getVodSource(int index) { return 0; }
};

} // namespace Hippo

#endif // __cplusplus

#endif // _Program_H_
