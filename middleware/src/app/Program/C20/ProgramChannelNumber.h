#ifndef _ProgramChannelNumber_H_
#define _ProgramChannelNumber_H_

#include "Program.h"

#ifdef __cplusplus

namespace Hippo {

class ProgramChannelNumber {
public:
    ProgramChannelNumber();
    ~ProgramChannelNumber();

    virtual ProgramType getType();

protected:
    int mChannelNumber;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _ProgramChannelNumber_H_
