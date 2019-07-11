#ifndef _TRUNK_EC2108_C27_IPSTB_SRC_APP_BROWSERBRIDGE_HUAWEI_C20_BROWSERPLAYERREPORTERVODC20_H_
#define _TRUNK_EC2108_C27_IPSTB_SRC_APP_BROWSERBRIDGE_HUAWEI_C20_BROWSERPLAYERREPORTERVODC20_H_

#include "mid_stream.h"
#include "BrowserPlayerReporterC20.h"

#ifdef __cplusplus

namespace Hippo {

class BrowserPlayerReporterVodC20: public BrowserPlayerReporterC20 {
public:
    BrowserPlayerReporterVodC20();
    ~BrowserPlayerReporterVodC20();
    
    virtual void reportMessage(STRM_MSG message, int code);
};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserPlayerReporterVodC20_H_
