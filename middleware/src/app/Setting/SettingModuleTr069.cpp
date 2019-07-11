
#include "Assertions.h"
#include "SettingModuleTr069.h"

#include <stdio.h>

#include "Setting.h"
#include "AppSetting.h"
#include "SysSetting.h"

#include "configCustomer.h"
#ifdef INCLUDE_HMWMGMT
#include "Tr069Setting.h"
#endif
namespace Hippo {

static SettingModuleTr069 g_SettingModuleTr069;

SettingModuleTr069::SettingModuleTr069()
    : SettingModule()
{
}

SettingModuleTr069::~SettingModuleTr069()
{
}

int
SettingModuleTr069::settingModuleRegister()
{
#if defined(INCLUDE_TR069) || defined(INCLUDE_HMWMGMT)

    sysSetting().add("[ TR069 ]", "");

    LogSafeOperDebug("SettingModuleTr069::settingModuleRegister [%d]\n", __LINE__);
    sysSetting().add("tr069_enable", DEFAULT_TR069_ENABLE);
    sysSetting().add("tr069_upgrades", DEFAULT_TR069_UPGRADES);
    sysSetting().add("tr069_type", DEFAULT_TR069_TYPE); //0:huawei;1:ctc;2:cu
    sysSetting().add("tr069UpgradeUrl", DEFAULT_TR069_TYPE);
#endif
#ifdef INCLUDE_HMWMGMT
    tr069Setting().add("Global.Bootstrap", 0);
    tr069Setting().add("Global.Reboot_state", 0);
    tr069Setting().add("Global.Reboot_commandkey", "");
    tr069Setting().add("Global.Schedule_time", 0);
	tr069Setting().add("Global.Schedule_commandkey", "");

	tr069Setting().add("Param.ProvisioningCode", "");
	tr069Setting().add("Param.FirstUseDate", DEFAULT_TR069_FIRSTUSEDATE);
    tr069Setting().add("Param.URL", DEFAULT_TR069_URL);
    tr069Setting().add("Param.URLBackup", DEFAULT_TR069_URLBACKUP);
    tr069Setting().add("Param.Username", DEFAULT_TR069_USERNAME);
	tr069Setting().add("Param.AESPassword", DEFAULT_TR069_PASSWORD);
	tr069Setting().add("Param.URLModifyFlag", DEFAULT_TR069_URLMODIFYFLAY);

	tr069Setting().add("Param.PeriodicInformEnable", DEFAULT_TR069_INFORMENABLE);
	tr069Setting().add("Param.PeriodicInformInterval", DEFAULT_TR069_INFORINTERVAL);
	tr069Setting().add("Param.PeriodicInformTime", "");
	tr069Setting().add("Param.ParameterKey", "");
	tr069Setting().add("Param.ConnectionRequestUsername", DEFAULT_TR069_CONNECTIONREQUESETUSERNAME);
	tr069Setting().add("Param.AESConnectionRequestPassword", DEFAULT_TR069_CONNECTIONREQUESETPASSWORD);
	tr069Setting().add("Param.STUNEnable", DEFAULT_TR069_STUNEABLE);
	tr069Setting().add("Param.STUNServerAddress", "");
	tr069Setting().add("Param.STUNServerPort", 0);
	tr069Setting().add("Param.STUNUsername", "");
	tr069Setting().add("Param.AESSTUNPassword", "");
	tr069Setting().add("Param.Ping_Host", "");
	tr069Setting().add("Param.Ping_NumberOfRepetitions", 0);
	tr069Setting().add("Param.Ping_Timeout", 0);
	tr069Setting().add("Param.Ping_DataBlockSize", 0);
	tr069Setting().add("Param.Ping_DSCP", 0);
	tr069Setting().add("Param.Trace_Host", "");
	tr069Setting().add("Param.Trace_Timeout", 0);
	tr069Setting().add("Param.Trace_DataBlockSize", 0);
	tr069Setting().add("Param.Trace_MaxHopCount", 0);
	tr069Setting().add("Param.Trace_DSCP", 0);
	tr069Setting().add("Param.ConnectionRequestPath", DEFAULT_TR069_CONNECTIONREQUESTPATH);
	tr069Setting().add("Param.STUNMaximumKeepAlivePeriod", DEFAULT_TR069_STUNMAXALIVEPERIOD);
	tr069Setting().add("Param.STUNMinimumKeepAlivePeriod", DEFAULT_TR069_STUNMINALIVEPERIOD);
#endif
    /************       customer   *****************/
    //appSetting().add("[ TR069 ]", "");
    
    //appSetting().add("syslog_server", "");
    //appSetting().add("syslog_server_port", 0); 
    return 0;
}

} // namespace Hippo
extern "C"
int settingTr069()
{
    return 0;
}

