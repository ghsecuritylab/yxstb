

#include "concatenator_hw_logout.h"
#include "SysSetting.h"

#include <sstream>


namespace Hippo {

std::string ConcatenatorHwLogout::operator()(std::string url) const
{
    if (url.empty())
        return url;

    // 某些华为EPG会写个这样的东西下来： 
    // http://serverip:port/EPG/jsp
    if (url.substr(url.length() - 8) == "/EPG/jsp") {
        url += std::string("/AuthenticationURL");
    } else if (url.substr(url.length() - 9) == "/EPG/jsp/") {
        url += std::string("AuthenticationURL");
    }

    if (url.find("?") == std::string::npos) {
        url += std::string("?");
    } else {
        url += std::string("&");
    }

    if (url.find("UserID=") == std::string::npos) {
        char    temp[4096];
        sysSettingGetString("ntvuser", temp, sizeof(temp), 0);
        url += std::string("UserID=");
        url += std::string(temp);
        url += std::string("&");
    }
    if (url.find("Action=") == std::string::npos) {
        url += std::string("Action=Logout");
    }
    if (url.substr(url.length() - 1) == "&") {
        url = url.substr(0, url.length() - 1);
    }
    return url;
}

} // namespace Hippo


