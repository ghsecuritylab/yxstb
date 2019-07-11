#ifndef _LogPostTerminal_H_
#define _LogPostTerminal_H_

#include "LogFilter.h"

#ifdef __cplusplus

namespace android {

class LogPostTerminal : public LogFilter {
public:
    virtual bool pushBlock(uint8_t* blockHead, uint32_t blockLength);
};

} // namespace android

#endif // __cplusplus
#endif // _LogPostTerminal_H_
