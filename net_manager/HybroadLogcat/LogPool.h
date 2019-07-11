#ifndef _LogPool_H_
#define _LogPool_H_

#include "DataSink.h"
#ifdef __cplusplus

namespace android {

class LogFilter;

class LogPool : public DataSink {
public:
    LogPool();
    virtual ~LogPool();

    virtual bool attachFilter(LogFilter*, int);
    virtual bool detachFilter(LogFilter*);

    void receiveData();

private:
    LogFilter* m_filterHead;
};

} // namespace android

#endif // __cplusplus

#endif // _LogPool_H_
