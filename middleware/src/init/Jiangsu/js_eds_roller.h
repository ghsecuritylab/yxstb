
#pragma once

#include "auth.h"

namespace Hippo {


class JsEdsRoller : public Auth::EdsRoller {
public:
    JsEdsRoller();
    virtual void Next();
    virtual std::string Current();
    virtual void Reset();
    virtual bool IsEnd();
private:
    int index;
};

} // namespace Hippo

