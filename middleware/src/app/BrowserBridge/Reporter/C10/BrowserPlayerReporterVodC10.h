#ifndef _BrowserPlayerReporterVodC10_H_
#define _BrowserPlayerReporterVodC10_H_

#include "mid_stream.h"
#include "BrowserPlayerReporterC10.h"

#ifdef __cplusplus

namespace Hippo {

class BrowserPlayerReporterVodC10: public BrowserPlayerReporterC10 {
public:
    BrowserPlayerReporterVodC10();
    ~BrowserPlayerReporterVodC10();
    
    virtual void reportMessage(STRM_MSG message, int code);
};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserPlayerReporterVodC10_H_
