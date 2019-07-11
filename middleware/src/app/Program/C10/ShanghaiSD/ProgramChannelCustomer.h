#ifndef _ProgramChannelC10_H_
#define _ProgramChannelC10_H_

#include "ProgramChannel.h"

#ifdef __cplusplus

namespace Hippo {

class ProgramChannelCustomer : public ProgramChannel {
public:
    ProgramChannelCustomer();
    ~ProgramChannelCustomer();

    virtual void SetChanType(int iarg);
};

} // namespace Hippo

#endif // __cplusplus

#endif // _ProgramInfo_H_
