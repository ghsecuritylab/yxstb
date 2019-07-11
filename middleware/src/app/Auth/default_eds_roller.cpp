

#include "default_eds_roller.h"

#include "SysSetting.h"
#include "AppSetting.h"

namespace Hippo {

DefaultEdsRoller::DefaultEdsRoller()
    : Auth::EdsRoller()
    , index(0)
{
}

void DefaultEdsRoller::Next()
{
    index ++;
}

bool DefaultEdsRoller::IsEnd()
{
    if (index > 2)
        return true;
    return false;
}

void DefaultEdsRoller::Reset()
{
    index = 0;
}

std::string DefaultEdsRoller::Current()
{
    char    temp[4096] = {0};
    if (index == 0) {
        sysSettingGetString("eds", temp, sizeof(temp), 0);
    } else if (index == 1) {
        sysSettingGetString("eds1", temp, sizeof(temp), 0);
    } else if (index == 2) {
        appSettingGetString("epg", temp, sizeof(temp), 0);
    } else {
        temp[0] = '\0';
    }
    return std::string(temp);
} 
} // namespace Hippo


