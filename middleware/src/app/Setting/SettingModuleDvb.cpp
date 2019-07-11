
#include "Assertions.h"
#include "SettingModuleDvb.h"

#include <stdio.h>

#include "Setting.h"
#include "AppSetting.h"
#include "SysSetting.h"

#include "configCustomer.h"

namespace Hippo {

static SettingModuleDvb g_SettingModuleDvb;

SettingModuleDvb::SettingModuleDvb()
    : SettingModule()
{
}

SettingModuleDvb::~SettingModuleDvb()
{
}

int
SettingModuleDvb::settingModuleRegister()
{
    sysSetting().add("[ DVB ]", "");
    LogSafeOperDebug("SettingModuleDvb::settingModuleRegister [%d]\n", __LINE__);
    sysSetting().add("DVBChannelManageFlag", DEFAULT_DVB_CHANNEL_MANAGE_FLAG);
    sysSetting().add("DvbServiceCheck", DEFAULT_DVB_SERVICE_CHECK);
    sysSetting().add("LnbConnectType", DEFAULT_LNB_CONNECT_TYPE);
    sysSetting().add("LocalLatitude", DEFAULT_LOCAL_LONGITUDE);
    sysSetting().add("LocalLongitude", DEFAULT_LOCAL_LATITUDE);
    sysSetting().add("MainFrequency", DEFAULT_MAIN_FREQUENCY);
    sysSetting().add("MainSymbolRate", DEFAULT_MAIN_SYMBOLRATE);
    sysSetting().add("MainPolarization", DEFAULT_MAIN_POLARIZATION);
    sysSetting().add("SatelliteLongitude", DEFAULT_SATELLITE_LONGITUDE);
    sysSetting().add("ReviseSwitch", DEFAULT_REVISE_SWITCH);	
    sysSetting().add("LastChannelType", DEFAULT_LAST_CHANNELTYPE);
    sysSetting().add("LastVideoNumber", DEFAULT_LAST_VIDEO_NUMBER);
    sysSetting().add("LastAudioNumber", DEFAULT_LAST_AUDIO_NUMBER);
    return 0;
}

} // namespace Hippo
extern "C"
int settingDvb()
{
    return 0;
}

