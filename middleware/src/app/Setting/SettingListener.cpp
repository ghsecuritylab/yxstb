
#include "SettingListener.h"
#include "AppSetting.h"
#include "SysSetting.h"
#include "Assertions.h"

#include <string>
#include <map>
#include <utility>

namespace Hippo {

SettingListener::SettingListener()
{
}

SettingListener::~SettingListener()
{
}

int
SettingListener::add(ListenerFunction func)
{
    if (!func)
        return -1;

    m_functions.push_back(func);
    return 0;
}

int
SettingListener::del(ListenerFunction func)
{
    if (!func)
        return -1;

    std::vector<ListenerFunction>::iterator it;
    for (it = m_functions.begin(); it != m_functions.end(); ++it) {
        if ((*it) == func) {
             m_functions.erase(it);
             break;
        }
    }
    if (it == m_functions.end())
        return -1;
    return 0;
}

int
SettingListener::call(const char*name, const char* value)
{
    std::vector<ListenerFunction>::iterator it;
    for (it = m_functions.begin(); it != m_functions.end(); ++it) {
        (*it)(name, value);
    }
    return 0;
}

} // namespace Hippo


//用于各个模块对监听setting字段监听注册，安卓版本不监听。
extern "C" int SettingListenerRegist(const char* name, int type, ListenerFunction func)
{
#ifdef ANDROID
    return 0;
#endif

    if (type) //sysSetting
        Hippo::sysSetting().registListeners(name, func);
    else //AppSetting
        Hippo::appSetting().registListeners(name, func);
    return 0;
}

//用于各个模块注销监听的setting字段
extern "C" int SettingListenerUnregist(const char* name, int type, ListenerFunction func)
{
#ifdef  ANDROID
    return 0;
#endif

    if (type) //sysSetting
        Hippo::sysSetting().unregistListeners(name, func);
    else //AppSetting
        Hippo::appSetting().unregistListeners(name, func);
    return 0;
}

