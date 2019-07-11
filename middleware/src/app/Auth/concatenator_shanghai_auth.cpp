

#include "concatenator_shanghai_auth.h"
#include "SysSetting.h"

#include <sstream>


namespace Hippo {

std::string ConcatenatorShAuth::operator()(std::string url) const
{
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
        url += std::string("Action=Login");
        url += std::string("&");
    }
    if (url.find("Mode=") == std::string::npos) {
        url += std::string("Mode=Menu");
        url += std::string("&");
    }
    if (url.find("STBAdminStatus=") == std::string::npos) {
        url += std::string("STBAdminStatus=1");
        url += std::string("&");
    }

    if (url.substr(url.length() - 1) == "&") {
        url = url.substr(0, url.length() - 1);
    }
    return url;
}

} // namespace Hippo


