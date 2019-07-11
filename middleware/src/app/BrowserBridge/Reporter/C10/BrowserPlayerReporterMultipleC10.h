#ifndef _BrowserPlayerReporterMultipleC10_H_
#define _BrowserPlayerReporterMultipleC10_H_

#include "mid_stream.h"
#include "BrowserPlayerReporterC10.h"

#ifdef __cplusplus

namespace Hippo {

class BrowserPlayerReporterMultipleC10: public BrowserPlayerReporterC10 {
public:
    BrowserPlayerReporterMultipleC10();
    ~BrowserPlayerReporterMultipleC10();

    virtual void reportState(STRM_STATE state, int rate);
    virtual void reportMessage(STRM_MSG message, int code);

protected:
    int getChannelKey();
    
};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserPlayerReporterMultipleC10_H_
