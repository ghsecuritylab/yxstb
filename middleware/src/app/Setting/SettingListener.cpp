
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


//���ڸ���ģ��Լ���setting�ֶμ���ע�ᣬ��׿�汾��������
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

//���ڸ���ģ��ע��������setting�ֶ�
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

