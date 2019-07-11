#ifndef _LogThread_H_
#define _LogThread_H_

#include "Thread.h"

#ifdef __cplusplus

namespace Hippo {

class LogThread : public Thread {
public:
    LogThread();
    ~LogThread();

    virtual void run();
};

} /* namespace Hippo */

#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif

void logThreadInit();

#ifdef __cplusplus
}
#endif

#endif // _LogThread_H_
