
#ifndef _CONCATENATOR_C20_AUTH_H_
#define _CONCATENATOR_C20_AUTH_H_
#pragma once

#include "auth.h"

namespace Hippo {

class ConcatenatorC20Auth : public Auth::Concatenator {
public:
    std::string operator()(std::string url) const;
};

} // namespace Hippo

#endif

