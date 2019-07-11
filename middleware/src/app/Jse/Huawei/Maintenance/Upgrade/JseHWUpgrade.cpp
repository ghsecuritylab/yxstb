
#include "JseHWUpgrade.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "JseAssertions.h"
#include "UpgradeManager.h"
#include "json/json_public.h"
#include "MessageTypes.h"
#include "BrowserAgent.h"
#include "Session.h"

#include "SysSetting.h"
#include "AppSetting.h"
#include "VersionSetting.h"
#include "SettingEnum.h"


#include "Tr069.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef NEW_ANDROID_SETTING
extern "C" int IPTVMiddleware_SettingSetStr(const char* name, const char* value);
#endif
using namespace Hippo;

static int JseUpgradeCheckIntervalWrite( const char* param, char* value, int len )
{
    appSettingSetInt("upcheckInterval", atoi(value));

    return 0;
}

/*******************************************************
    EPG可以调用此接口通知STB立即进入指定Path的升级流程。
    对于STB版本有IP和DVB两种升级途径的情况，当EPG收到STB上报的有新版本可以升级的事件EVENT_NEW_VERSION时，
    如果需要立即进行升级时，EPG调用此接口，并根据上报事件中的新版本来源指定本次要升级的路径。
    var sValue=Utility.setValueByName('hw_op_upgradeStart', '{"PATH": "path"}');
        PATH == DVB：表示DVB path的升级; PATH == IP：表示是IP path的升级
        sValue 0 is successful, others is failed
*******************************************************/
static int JseUpgradeStartWrite( const char* param, char* value, int len )
{
    struct json_object* object  = NULL;
    struct json_object* obj = NULL;
    std::string sValue;
    sValue.clear();
    object = json_tokener_parse_string(value);
    if (object == NULL) {
        return -1;
    }
    obj = json_object_get_object_bykey(object,"path");
    if (obj) {
        sValue = json_object_get_string(obj);
        if (!sValue.compare("DVB")) {
            upgradeManager()->responseEvent(UpgradeManager::UMUT_DVB_SOFTWARE, true);
        } else {
            upgradeManager()->responseEvent(UpgradeManager::UMUT_IP_SOFTWARE, true);
        }
    }
    json_object_delete(object);

    return 0;
}

/*******************************************************
	  EPG可以调用此接口通知STB取消升级等待。
        当机顶盒检查到有新版本需要升级时，会发送消息通知EPG，消息发送后机顶盒处于超时等待中，
    超过等待时间后，机顶盒将自动升级。此接口会取消机顶盒的超时等待状态，机顶盒在接受到该调用后，
    仅仅取消超时等待状态，EPG仍然可以调用触发升级接口通知STB升级。
        当EPG接收到STB上报的有新版本可以升级的事件EVENT_NEW_VERSION时，弹出提示框让用户选择，
    并立即调用此接口取消STB的升级等待，避免STB超时后自动升级；如果不需要让用户选择，EPG不需要调用此接口，
    STB上报事件后过超时时间后会自动升级。
        在hybrid方案中，会同时存在IP和DVB通道的升级，需要EPG指定取消升级等待的路径。
    var sValue= Utility.setValueByName('upgradeStart_wait', 'cancel,path')
        path 升级路径。DVB：表示DVB path的升级; IP：表示是IP path的升级.path的值由EPG根据获得的新版本通知事件中的新版本来源来判断。如果不下发，STB理解为IP通道。
        sValue 0 is successful, others is failed
*******************************************************/
static int JseUpgradeStartWaitWrite( const char* param, char* value, int len )
{
    if (!strncmp(value, "cancel", 6)) {
    }

    return 0;
}

static int JseUpgradeCancelWrite( const char* param, char* value, int len )
{
    char* p = NULL;
    if (!strncmp(value, "cancel", 6)) {
#ifdef INCLUDE_TR069
      int tr069Upgrade = 0;
        sysSettingGetInt("tr069_upgrades", &tr069Upgrade, 0);
	 	  if ( 1 == tr069Upgrade) {
            TR069_API_SETVALUE((char*)"Event.Upgraded", NULL, -1);
		}
#endif
        p = strstr(value, ",");
        if (!p || !strcmp(p, "IP"))
            upgradeManager()->responseEvent(UpgradeManager::UMUT_IP_SOFTWARE, false);
        else
        	upgradeManager()->responseEvent(UpgradeManager::UMUT_DVB_SOFTWARE, false);
    }

    return 0;
}

#if defined (HUAWEI_C20)
/*******************************************************
        EPG可以调用此接口下发升级方式，升级方式支持下发后实时生效，
    下发后STB不中断当前进行中的升级检测过程，但是终止当前升级方式后续的处理，
    当正在进行中的升级检测处理结束后，按照新的升级方式处理。
        当STB内保存的升级方式不符合局点要求时，EPG可以认证通过后调用此接口下发局点要求的升级方式。
    var sValue= Utility.setValueByName('upgradeMode', 'upgradeMode')
        upgradeMode 升级方式，保存在Flash：IP 仅从IP通道升级。DVBS 仅从DVBS通道升级。IP&DVBS 可以从IP和DVBS通道升级。
        sValue 0 is successful, others is failed
*******************************************************/
static int JseUpgradeModeWrite( const char* param, char* value, int len )
{
    if (!strncmp(value, "IP", 2)) {
        sysSettingSetInt("upgradeMode", IP_UPGRADE_MODE);
    } else if (!strncmp(value, "DVB", 3)) {
        sysSettingSetInt("upgradeMode", DVB_UPGRADE_MODE);
    } else if (!strncmp(value, "IP&DVB", 6)) {
        sysSettingSetInt("upgradeMode", MIX_UPGRADE_MODE);
    } else {
        sysSettingSetInt("upgradeMode", IP_UPGRADE_MODE);
    }

    return 0;
}

/*******************************************************
        EPG可以调用此接口下发DVB通道升级频点，如果需要开机检测DVBS通道是否有新版本，
    STB会开机锁定下发的升级频点进行检测，EPG下发后在STB后续第一次使用此频点信息时生效。
        当STB支持DVBS通道升级时，STB内保存的升级方式不符合局点要求时，
    EPG可以认证通过后调用调用此接口下发局点要求的升级方式。
    var sValue = Utility.setValueByName('DVBSUpgradeTP', '{"frequency":frequency,"symbolRate":symbolRate,"polarization":polarization,"satLongitude":"satLongitude"}')
        frequency(int 2)        升级频点的频率，保存在Flash。
        symbolRate(int 2)       升级频点的符号率，保存在Flash。
        polarization(int 1)     升级频点的极化方式，保存在Flash.0表示V，垂直极化;1表示H，水平极化;2表示左旋极化;3表示右旋极化.
        satLongitude(String 6)  升级频点所属的卫星经度，保存在Flash，为真实经度(整数部分最多3位，精确到小数点后一位，可以无小数部分)，如果是西经则为负数，比如“115.5”表示东经115.5°，”-15”表示西经15°。
        sValue 0 is successful, others is failed
*******************************************************/
static int JseUpgradeTPInfoWrite( const char* param, char* value, int len )
{
    struct json_object* object  = NULL;
    struct json_object* obj = NULL;
    std::string sValue;
    int frequency = 0;
    int symbolRate = 0;
    int polarization = 0;

    sValue.clear();
    object = json_tokener_parse_string(value);
    if (object == NULL) {
    	goto err;
    }

    obj = json_object_get_object_bykey(object,"frequency");
    if (obj == NULL) {
    	goto err;
    } else {
    	sValue = json_object_get_string(obj);
    	frequency = atoi(sValue.c_str());
    }

    obj = json_object_get_object_bykey(object,"symbolRate");
    if (obj == NULL) {
    	goto err;
    } else {
    	sValue = json_object_get_string(obj);
    	symbolRate = atoi(sValue.c_str());
    }

    obj = json_object_get_object_bykey(object,"polarization");
    if (obj == NULL) {
    	goto err;
    } else {
    	sValue = json_object_get_string(obj);
    	polarization = atoi(sValue.c_str());
    }

    obj = json_object_get_object_bykey(object,"satLongitude");
    if (obj == NULL) {
    	goto err;
    } else {
    	sValue = json_object_get_string(obj);
    }
#ifdef INCLUDE_DVBS
    sysSettingSetInt("MainFrequency", frequency);
    sysSettingSetInt("MainSymbolRate", symbolRate);
    sysSettingSetInt("MainPolarization", polarization);
    sysSettingSetString("SatelliteLongitude", sValue.c_str());
#endif
    json_object_delete(object);
    return 0;
err:
    json_object_delete(object);
    return -1;

}

static int JseUpgradeTemplateUrlRead( const char* param, char* value, int len )
{
    sysSettingGetString("templateUrl", value, URL_LEN, 0);

    return 0;
}

static int JseUpgradeTemplateUrlWrite( const char* param, char* value, int len )
{
    struct json_object* object  = NULL;
    struct json_object* obj = NULL;
    std::string templateUrl;
    std::string templateFlag;
    char localTemplateUrl[512] = {0};
    object = json_tokener_parse_string(value);
    if (object == NULL) {
    	LogJseError("json is invaild\n");
        json_object_delete(object);
        return -1;
    }
    obj = json_object_get_object_bykey(object,"EPGTemplateUrl");
    if (!obj) {
        LogJseError("json is invaild\n");
        json_object_delete(object);
        return -1;
    }
    templateUrl = json_object_get_string(obj);

    obj = json_object_get_object_bykey(object,"flag");
    if (!obj) {
    	LogJseError("json is invaild\n");
        json_object_delete(object);
        return -1;
    }
    templateFlag = json_object_get_string(obj);

    json_object_delete(object);

    if (templateFlag.compare("full")) {
        return -1;
    }

    sysSettingGetString("templateUrl", localTemplateUrl, 512, 0);
    if (!templateUrl.compare(localTemplateUrl)) {
        return -1;
    }

    sysSettingSetString("templateUrl", templateUrl.c_str());

    if (upgradeManager())
        upgradeManager()->touchOffUpgradeCheck(UpgradeManager::UMUT_IP_TEMPLATE, false);

    return 0;
}

static int JseUpgradeTemplateUpdateWrite( const char* param, char* value, int len )
{
    if (!strcmp(value, "start"))
    	upgradeManager()->responseEvent(UpgradeManager::UMUT_IP_TEMPLATE, true);
    else if (!strcmp(value, "cancel"))
        upgradeManager()->responseEvent(UpgradeManager::UMUT_IP_TEMPLATE, false);
    else
        return -1;

    return 0;
}

#endif

static int JseUpgradeServiceRead( const char* param, char* value, int len )
{
    if (value != NULL) {
		sysSettingGetString("upgradeUrl", value, len, 0);

    }

    return 0;
}

static int JseUpgradeServiceWrite( const char* param, char* value, int len )
{
#if defined(Liaoning) && defined(hi3716M)
    extern int NativeHandlerGetState();
    if (session().getPlatform() == PLATFORM_ZTE && defNativeHandler().getState() != NativeHandler::Config)
    	return 0;
#endif
    if (value != NULL) {
        if (0 != strlen(value) && 0 != strcmp(value, "null")) {
            sysSettingSetString("upgradeUrl", value);
#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
            sysSettingSetString("upgradeUrl", value);
#else
            IPTVMiddleware_SettingSetStr("upgradeUrl", value);
#endif
#endif
	    }
    }

    return 0;
}

static int JseUpgradeBackupServiceRead( const char* param, char* value, int len )
{
    if (value != NULL) {
		sysSettingGetString("upgradeBackupUrl", value, len, 0);
    }

    return 0;
}

static int JseUpgradeBackupServiceWrite( const char* param, char* value, int len )
{
#if defined(Liaoning) && defined(hi3716M)
    if (session().getPlatform() == PLATFORM_ZTE)
    	return 0;
#endif
    if (value != NULL) {
        if(0 != strlen(value) && 0 != strcmp(value, "null")) {
			sysSettingSetString("upgradeBackupUrl", value);
#if defined(ANDROID)
#ifdef NEW_ANDROID_SETTING
            sysSettingSetString("upgradeBackupUrl", value);
#else
	        IPTVMiddleware_SettingSetStr("upgradeBackupUrl", value);
#endif
#endif
        }
    }

    return 0;
}

static int JseUpgradeTr069ManageRead( const char* param, char* value, int len)
{
    int tr069Upgrade = 0;
    sysSettingGetInt("tr069_upgrades", &tr069Upgrade, 0);
    sprintf(value, "%d", tr069Upgrade);

    return 0;
}

static int JseUpgradeTr069ManageWrite( const char* param, char* value, int len )
{
    int upgrades = atoi(value);
    int tr069Upgrade;

    sysSettingGetInt("tr069_upgrades", &tr069Upgrade, 0);
    sysSettingSetInt("tr069_upgrades", upgrades);

#if defined(HUAWEI_C20)
    if (upgrades != tr069Upgrade) {
        if (upgrades) {
            upgradeManager().stopCheckTimer();
        } else {
            upgradeManager().startCheckTimer();
        }
    }
#endif

    return 0;
}

static int JseUpgradeForcedFlagRead( const char* param, char* value, int len )
{
    int upgradeForce = 0;
    sysSettingGetInt("upgradeForce", &upgradeForce, 0);

    snprintf(value, len, "%d", upgradeForce);

    return 0;
}

static int JseUpgradeForcedFlagWrite( const char* param, char* value, int len )
{
    sysSettingSetInt("upgradeForce", atoi(value));

    return 0;
}

static int JseCTCStartUpdateWrite( const char* param, char* value, int len )
{
    int tr069Upgrade = 0;

    sysSettingGetInt("tr069_upgrades", &tr069Upgrade, 0);
    if(tr069Upgrade){ // Indicates whether the ACS control CPE upgrade. If is true (1), the CPE should only use the ACS to seek available upgrades. If it is false (0), the CPE can use other methods to upgrade. IPTV SQM V100R001C29 STB与终端网管子系统接口数据模型（E7）.xls
        sendMessageToEPGBrowser(MessageType_Unknow, UpgradeManager::UMMI_UPGRADE_END, 0, 0);
        return -1;
    }

    if (upgradeManager())
        upgradeManager()->touchOffUpgradeCheck(UpgradeManager::UMUT_IP_SOFTWARE, false);

    return 0;
}

static int JseCUStartUpdateWrite( const char* param, char* value, int len )
{
    int tr069Upgrade = 0;

    sysSettingGetInt("tr069_upgrades", &tr069Upgrade, 0);
    if(tr069Upgrade){ // Indicates whether the ACS control CPE upgrade. If is true (1), the CPE should only use the ACS to seek available upgrades. If it is false (0), the CPE can use other methods to upgrade. IPTV SQM V100R001C29 STB与终端网管子系统接口数据模型（E7）.xls
        sendMessageToEPGBrowser(MessageType_Unknow, UpgradeManager::UMMI_UPGRADE_END, 0, 0);
        return -1;
    }

    if (upgradeManager()) {
        bool result = upgradeManager()->touchOffUpgradeCheck(UpgradeManager::UMUT_IP_SOFTWARE, false);
        if (result)
            upgradeManager()->setUpgradeProvider(1);
    }

    return 0;
}


/*************************************************
Description: 初始化华为Upgrade的接口，由JseHWMaintenance.cpp调用
Input: 无
Return: 无
*************************************************/
int JseHWUpgradeInit()
{
    JseCall* call;
    //C10 C20
    call = new JseFunctionCall("Update_check_interval", 0, JseUpgradeCheckIntervalWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("upgradeStart_wait", 0, JseUpgradeStartWaitWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("currentUpgrade", 0, JseUpgradeCancelWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("UpgradeDomain", JseUpgradeServiceRead, JseUpgradeServiceWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("UpgradeDomainBackup", JseUpgradeBackupServiceRead, JseUpgradeBackupServiceWrite);
    JseRootRegist(call->name(), call);

    //C10 C20 升级服务器地址(String 1024)，写入STB Flash。
    call = new JseFunctionCall("UpgradeServer", JseUpgradeServiceRead, JseUpgradeServiceWrite);
    JseRootRegist(call->name(), call);

    //C10 C20 UpgradesManaged是指升级是否受控，无论设为0还是1，都是允许ACS（终端网管）控制升级的，只是为1时升级是受控的，只允许ACS控制升级。
    call = new JseFunctionCall("UpgradesManaged", JseUpgradeTr069ManageRead, JseUpgradeTr069ManageWrite);
    JseRootRegist(call->name(), call);

    //C10 C20 备用升级服务器地址(String 1024)，写入STB Flash。国内C15基线增加
    call = new JseFunctionCall("UpgradeServerBackup", JseUpgradeBackupServiceRead, JseUpgradeBackupServiceWrite);
    JseRootRegist(call->name(), call);
    //C10 C20 强制升级标志。1 is 强制升级
    call = new JseFunctionCall("UpgradeForcedFlag", JseUpgradeForcedFlagRead, JseUpgradeForcedFlagWrite);
    JseRootRegist(call->name(), call);
    //C10 C20
    call = new JseFunctionCall("CTCStartUpdate", 0, JseCTCStartUpdateWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("CUStartUpdate", 0, JseCUStartUpdateWrite);
    JseRootRegist(call->name(), call);
#if defined (HUAWEI_C20)

    //C20
    call = new JseFunctionCall("hw_op_upgradeStart", 0, JseUpgradeStartWrite);
    JseRootRegist(call->name(), call);

    //C20
    call = new JseFunctionCall("upgradeMode", 0, JseUpgradeModeWrite);
    JseRootRegist(call->name(), call);

    //C20
    call = new JseFunctionCall("DVBSUpgradeTP", 0, JseUpgradeTPInfoWrite);
    JseRootRegist(call->name(), call);

    //C20 国内用于EPG下发升级服务器地址
    call = new JseFunctionCall("EPGTemplateUrl", JseUpgradeTemplateUrlRead, JseUpgradeTemplateUrlWrite);
    JseRootRegist(call->name(), call);

    //C20
    call = new JseFunctionCall("EPGTemplateUpgrade", 0, JseUpgradeTemplateUpdateWrite);
    JseRootRegist(call->name(), call);

#endif

#if defined (HUAWEI_C10)

    //C10
    call = new JseFunctionCall("upgradeStart", 0, JseUpgradeStartWrite);
    JseRootRegist(call->name(), call);

#endif
    return 0;
}

