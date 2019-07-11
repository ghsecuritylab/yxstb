
#ifndef _CONCATENATOR_HW_LOGOUT_H_
#define _CONCATENATOR_HW_LOGOUT_H_
#pragma once

#include "auth.h"

namespace Hippo {

class ConcatenatorHwLogout : public Auth::Concatenator {
public:
    std::string operator()(std::string url) const;
};

} // namespace Hippo

#endif

