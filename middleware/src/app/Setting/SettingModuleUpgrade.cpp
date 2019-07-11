
#include "Assertions.h"
#include "SettingModuleUpgrade.h"

#include <stdio.h>

#include "Setting.h"
#include "AppSetting.h"
#include "SysSetting.h"

#include "configCustomer.h"

namespace Hippo {

static SettingModuleUpgrade g_SettingModuleUpgrade;

SettingModuleUpgrade::SettingModuleUpgrade()
    : SettingModule()
{
}

SettingModuleUpgrade::~SettingModuleUpgrade()
{
}

int
SettingModuleUpgrade::settingModuleRegister()
{
    sysSetting().add("[ UPGRADE ]", "");
    LogSafeOperDebug("SettingModuleUpgrade::settingModuleRegister [%d]\n", __LINE__);
    sysSetting().add("upgradeUrl", DEFAULT_UPGRADE_URL); //upgrade url
    sysSetting().add("upgradeBackupUrl", DEFAULT_UPGRADE_BACKUP_URL);
    sysSetting().add("upgradeForce", DEFAULT_UPGRADE_FORCE); //upgrade force flag
    sysSetting().add("upgradeMode", DEFAULT_UPGRADE_MODE);
    sysSetting().add("templateUrl", DEFAULT_TEMPLATE_URL); //EPG template url
#ifdef Jiangsu
    sysSetting().add("upgrade_state", 1); // 用于多路径升级，当值为1，就恢复为默认升级地址
#endif
    sysSetting().add("IPUpgradeOK", DEFAULT_IP_UPGRADEOK);
    sysSetting().add("isFirstUpgradeOK", DEFAULT_IS_FIRST_UPGRADEOK);
    sysSetting().add("upgradeNewVersion", DEFAULT_UPGRADE_NEW_VERSION);
    /************** customer ***********/
    appSetting().add("[ UPGRADE ]", DEFAULT_UP_CHECK_INTERVAL);

    appSetting().add("upcheckInterval", DEFAULT_UP_CHECK_INTERVAL); //hour
    return 0;
}

} // namespace Hippo
extern "C"
int settingUpgrade()
{
    return 0;
}

