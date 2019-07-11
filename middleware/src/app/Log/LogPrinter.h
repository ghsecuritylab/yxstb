#ifndef _LogPrinter_H_
#define _LogPrinter_H_

#include "LogFilter.h"

#ifdef __cplusplus

namespace Hippo {

class LogPrinter : public LogFilter {
public:
    virtual bool pushBlock(uint8_t* blockHead, uint32_t blockLength);
};

} // namespace Hippo

#endif // __cplusplus
#endif // _LogPrinter_H_
