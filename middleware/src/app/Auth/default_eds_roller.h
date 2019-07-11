
#pragma once

#include "auth.h"

namespace Hippo {

class DefaultEdsRoller : public Auth::EdsRoller {
public:
    DefaultEdsRoller();
    virtual void Next();
    virtual std::string Current();
    virtual void Reset();
    virtual bool IsEnd();
private:
    int index;
};

} // namespace Hippo

