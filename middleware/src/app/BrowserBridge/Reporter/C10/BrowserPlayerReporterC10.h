#ifndef _BrowserPlayerReporterC10_H_
#define _BrowserPlayerReporterC10_H_

#include "BrowserPlayerReporterHuawei.h"


#ifdef __cplusplus

namespace Hippo {

class BrowserPlayerReporterC10 : public BrowserPlayerReporterHuawei {
public:
	BrowserPlayerReporterC10();
    ~BrowserPlayerReporterC10();

    virtual void reportState(STRM_STATE state, int rate);
    virtual void reportMessage(STRM_MSG message, int code);
};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserPlayerReporterC10_H_
