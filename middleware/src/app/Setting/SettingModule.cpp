
#include "Assertions.h"
#include "SettingModule.h"

#include <stdio.h>

namespace IBox {

static SettingModule* g_SettingModuleHead = 0;

SettingModule::SettingModule()
{
    m_next = g_SettingModuleHead;
    g_SettingModuleHead = this;
}

SettingModule::~SettingModule()
{
}

} // namespace IBox

extern "C"
void settingModuleResgister()
{
    Hippo::SettingModule *p_settingModule = Hippo::g_SettingModuleHead;
    while (p_settingModule) {
        p_settingModule->settingModuleRegister();
        p_settingModule = p_settingModule->m_next;
    }
}
