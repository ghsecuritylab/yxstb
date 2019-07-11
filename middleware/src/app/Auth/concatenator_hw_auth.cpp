

#include "concatenator_hw_auth.h"
#include "SysSetting.h"
#include "AppSetting.h"

#include <sstream>


namespace Hippo {

std::string ConcatenatorHwAuth::operator()(std::string url) const
{
    if (url.empty())
        return url;

    // 不在这里做兼容了。在RESTART_AUTH的响应里去重新认证

    if (url.find("?") == std::string::npos) {
        url += std::string("?");
    } else {
        url += std::string("&");
    }

    if (url.find("UserID=") == std::string::npos) {
        char    temp[4096];
        appSettingGetString("ntvuser", temp, sizeof(temp), 0);
        url += std::string("UserID=");
        url += std::string(temp);
        url += std::string("&");
    }
    if (url.find("Action=") == std::string::npos) {
        url += std::string("Action=Login");
    }
    if (url.find("UserField=") == std::string::npos) {
        int temp = 0;
        char buf[15] = {0};
        appSettingGetInt("UserField", &temp, 0);
        snprintf(buf, 15, "%d", temp);
        url += std::string("&UserField=");
        url += std::string(buf);
    }
    if (url.substr(url.length() - 1) == "&") {
        url = url.substr(0, url.length() - 1);
    }
    return url;
}

} // namespace Hippo


