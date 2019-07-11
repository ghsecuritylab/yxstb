
#ifndef _DEFAULT_URL_CHECKER_
#define _DEFAULT_URL_CHECKER_
#pragma once

#include "auth.h"

namespace Hippo {

class DefaultUrlChecker : public Auth::UrlChecker {
public:
    virtual bool operator()(std::string url) const;
};

} // namspace Hippo

#endif

