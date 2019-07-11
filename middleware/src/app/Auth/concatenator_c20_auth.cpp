

#include "concatenator_c20_auth.h"

#include <sstream>
#include "NetworkFunctions.h"
#include "SysSetting.h"
#include "AppSetting.h"
#include "mid_sys.h"
#include "mid/mid_tools.h"
#include "mid/mid_time.h"
#include "charConvert.h"
#include "cryptoFunc.h"
#include "sys_basic_macro.h"
#include "stbinfo/stbinfo.h"
#include "Business.h"

void app_md5_password(char *check, char *timestap)
{
    char *array[3] = {0};
    char buf[USER_LEN] = {0};
    char ntvPassword[USER_LEN] = { 0 };

    appSettingGetString("ntvAESpasswd", ntvPassword, USER_LEN, 0);
    array[0] = ntvPassword;
    md5Encypt(array, 1, check, 32, 1);
    data2Hex(check, 4, check, 32);
    mid_tool_time2string(mid_time(), timestap, 0);
    appSettingGetString("ntvuser", buf, USER_LEN, 0);
    array[0] = check;
    array[1] = buf;
    array[2] = timestap;
    md5Encypt(array, 3, check, 32, 1);
    data2Hex(check, 4, check, 32);
    return;
}

namespace Hippo {

std::string ConcatenatorC20Auth::operator()(std::string url) const
{
    std::stringstream    oss;
    oss << url;
    if (url.find("?") == std::string::npos) {
        oss << '?';
    } else {
        oss << '&';
    }

    if (url.find("User=") == std::string::npos) {
        char    temp[4096];
        appSettingGetString("ntvuser", temp, sizeof(temp), 0);
        oss << "User=" << temp << "&";
    }
    if (url.find("pwd=") == std::string::npos) {
        oss << "pwd=&";
    }
    if (url.find("ip=") == std::string::npos) {
        char ifname[4096] = {0};
        char ifaddr[4096] = {0};
        network_default_ifname(ifname, 4096);
        oss << "ip=" << network_address_get(ifname, ifaddr, 4096) << '&';
    }
    if (url.find("NTID=") == std::string::npos) {
        char NTID[32] = {0};
        oss << "NTID=" << network_tokenmac_get(NTID, 32, ':') << '&';
    }
    if (url.find("Version=") == std::string::npos) {
        oss << "Version=" << StbInfo::STB::Model() << '&';
    }
    if (url.find("lang=") == std::string::npos) {
        int lang;
        sysSettingGetInt("lang", &lang, 0);
        oss << "lang=" << lang << '&';
    }
    if (url.find("checkwords=") == std::string::npos) {
        char checkwords[16];
        char timestamp[16];
        app_md5_password(checkwords, timestamp);
        oss << "checkwords=" << checkwords << "&timestamp=" << timestamp << '&';
    }
    if (url.find("joinFlag=") == std::string::npos) {
        oss << "joinFlag=" << business().getEDSJoinFlag() << '&';
    }
    if (url.find("caid=") == std::string::npos) {
        oss << "caid=" << '&';
    }
    if (url.find("caid=") == std::string::npos) {
        oss << "caid=" << '&';
    }
    if (url.find("CASTBPlatFormID=") == std::string::npos) {
        oss << "CASTBPlatFormID=23";
    }

    if (url.substr(url.length() - 1) == "&") {
        url = url.substr(0, url.length() - 1);
    }
    return url;
}

} // namespace Hippo


