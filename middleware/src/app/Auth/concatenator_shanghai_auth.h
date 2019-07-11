
#ifndef _CONCATENATOR_Sh_AUTH_H_
#define _CONCATENATOR_Sh_AUTH_H_
#pragma once

#include "auth.h"

namespace Hippo {

class ConcatenatorShAuth : public Auth::Concatenator {
public:
    std::string operator()(std::string url) const;
};

} // namespace Hippo

#endif

