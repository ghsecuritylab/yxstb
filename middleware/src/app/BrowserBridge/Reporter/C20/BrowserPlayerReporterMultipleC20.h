#ifndef _TRUNK_EC2108_C27_IPSTB_SRC_APP_BROWSERBRIDGE_HUAWEI_C20_BROWSERPLAYERREPORTERMULTIPLEC20_H_
#define _TRUNK_EC2108_C27_IPSTB_SRC_APP_BROWSERBRIDGE_HUAWEI_C20_BROWSERPLAYERREPORTERMULTIPLEC20_H_

#include "mid_stream.h"
#include "BrowserPlayerReporterC20.h"

#ifdef __cplusplus

namespace Hippo {

class BrowserPlayerReporterMultipleC20: public BrowserPlayerReporterC20 {
public:	
    BrowserPlayerReporterMultipleC20();
    ~BrowserPlayerReporterMultipleC20();
	
	// virtual void reportState(STRM_STATE state, int rate);
	virtual void reportMessage(STRM_MSG message, int code);
};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserPlayerReporterMultipleC20_H_
