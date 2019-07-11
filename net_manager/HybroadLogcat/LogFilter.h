#ifndef _LogFilter_H_
#define _LogFilter_H_

#include <stdint.h>

#ifdef __cplusplus

namespace android {

class LogFilter {
public:
    LogFilter();
    virtual ~LogFilter();

    virtual bool pushBlock(uint8_t* blockHead, uint32_t blockLength) = 0;

    LogFilter* m_next; 
};

} // namespace android

#endif // __cplusplus

#endif // _LogFilter_H_
