

#include "default_url_checker.h"

namespace Hippo {

bool DefaultUrlChecker::operator()(std::string url) const
{
    if (url.empty()) {
        return false;
    }

    if (url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://") {
        return false;
    }

    return true;
}

} // namespace Hippo


