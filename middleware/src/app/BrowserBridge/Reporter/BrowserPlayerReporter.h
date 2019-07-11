#ifndef _BrowserPlayerReporter_H_
#define _BrowserPlayerReporter_H_

#include "RefCnt.h"

#include "mid_stream.h"

#ifdef __cplusplus

namespace Hippo {    
    
class UltraPlayer;

class BrowserPlayerReporter : public RefCnt {
public:        
    BrowserPlayerReporter();
    ~BrowserPlayerReporter();

    void setPlayer(UltraPlayer *player) { mPlayer = player; }

    virtual void reportState(STRM_STATE state, int rate) = 0;
    virtual void reportMessage(STRM_MSG message, int code) = 0;

protected:
    UltraPlayer *mPlayer;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserPlayerReporter_H_
