#ifndef _BrowserPlayerReporterC20_H_
#define _BrowserPlayerReporterC20_H_

#include "BrowserPlayerReporterHuawei.h"

#ifdef __cplusplus

namespace Hippo {

class BrowserPlayerReporterC20 : public BrowserPlayerReporterHuawei {
public:
	BrowserPlayerReporterC20();
    ~BrowserPlayerReporterC20();

    virtual void reportState(STRM_STATE state, int rate);
    virtual void reportMessage(STRM_MSG message, int code);    
    
};

} // namespace Hippo
#endif

#endif
