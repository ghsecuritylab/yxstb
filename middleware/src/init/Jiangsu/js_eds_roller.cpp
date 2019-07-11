

#include "js_eds_roller.h"

#include "SysSetting.h"
#include "customer.h"

namespace Hippo {

JsEdsRoller::JsEdsRoller ()
    : Auth::EdsRoller()
    , index(0)
{
}

void JsEdsRoller::Next()
{
    index++;
}

void JsEdsRoller::Reset()
{
    index = 0;
}

bool JsEdsRoller::IsEnd()
{
    if (index > 3)
        return true;
    return false;
}

std::string JsEdsRoller::Current()
{
    std::string url;
    if (index == 0) {
        char    temp[4096] = {0};
        sysSettingGetString("eds", temp, sizeof(temp), 0);
        url = temp;
    } else if (index == 1) {
        url = Customer().AuthInfo().Get(Auth::eAuth_epgDomain);
    } else if (index == 2) {
        char    temp[4096] = {0};
        sysSettingGetString("eds1", temp, sizeof(temp), 0);
        url = temp;
    } else if (index == 3) {
        url = Customer().AuthInfo().Get(Auth::eAuth_epgDomainBackup);
    } else {
        return std::string("");
    }
    if (url.empty()) {
        // 递归
        Next();
        return Current();
    }
    return url;
}


} // namespace Hippo


