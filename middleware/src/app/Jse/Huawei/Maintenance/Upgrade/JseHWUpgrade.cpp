
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
    EPG���Ե��ô˽ӿ�֪ͨSTB��������ָ��Path���������̡�
    ����STB�汾��IP��DVB��������;�����������EPG�յ�STB�ϱ������°汾�����������¼�EVENT_NEW_VERSIONʱ��
    �����Ҫ������������ʱ��EPG���ô˽ӿڣ��������ϱ��¼��е��°汾��Դָ������Ҫ������·����
    var sValue=Utility.setValueByName('hw_op_upgradeStart', '{"PATH": "path"}');
        PATH == DVB����ʾDVB path������; PATH == IP����ʾ��IP path������
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
	  EPG���Ե��ô˽ӿ�֪ͨSTBȡ�������ȴ���
        �������м�鵽���°汾��Ҫ����ʱ���ᷢ����Ϣ֪ͨEPG����Ϣ���ͺ�����д��ڳ�ʱ�ȴ��У�
    �����ȴ�ʱ��󣬻����н��Զ��������˽ӿڻ�ȡ�������еĳ�ʱ�ȴ�״̬���������ڽ��ܵ��õ��ú�
    ����ȡ����ʱ�ȴ�״̬��EPG��Ȼ���Ե��ô��������ӿ�֪ͨSTB������
        ��EPG���յ�STB�ϱ������°汾�����������¼�EVENT_NEW_VERSIONʱ��������ʾ�����û�ѡ��
    ���������ô˽ӿ�ȡ��STB�������ȴ�������STB��ʱ���Զ��������������Ҫ���û�ѡ��EPG����Ҫ���ô˽ӿڣ�
    STB�ϱ��¼������ʱʱ�����Զ�������
        ��hybrid�����У���ͬʱ����IP��DVBͨ������������ҪEPGָ��ȡ�������ȴ���·����
    var sValue= Utility.setValueByName('upgradeStart_wait', 'cancel,path')
        path ����·����DVB����ʾDVB path������; IP����ʾ��IP path������.path��ֵ��EPG���ݻ�õ��°汾֪ͨ�¼��е��°汾��Դ���жϡ�������·���STB���ΪIPͨ����
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
        EPG���Ե��ô˽ӿ��·�������ʽ��������ʽ֧���·���ʵʱ��Ч��
    �·���STB���жϵ�ǰ�����е����������̣�������ֹ��ǰ������ʽ�����Ĵ���
    �����ڽ����е�������⴦������󣬰����µ�������ʽ����
        ��STB�ڱ����������ʽ�����Ͼֵ�Ҫ��ʱ��EPG������֤ͨ������ô˽ӿ��·��ֵ�Ҫ���������ʽ��
    var sValue= Utility.setValueByName('upgradeMode', 'upgradeMode')
        upgradeMode ������ʽ��������Flash��IP ����IPͨ��������DVBS ����DVBSͨ��������IP&DVBS ���Դ�IP��DVBSͨ��������
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
        EPG���Ե��ô˽ӿ��·�DVBͨ������Ƶ�㣬�����Ҫ�������DVBSͨ���Ƿ����°汾��
    STB�Ὺ�������·�������Ƶ����м�⣬EPG�·�����STB������һ��ʹ�ô�Ƶ����Ϣʱ��Ч��
        ��STB֧��DVBSͨ������ʱ��STB�ڱ����������ʽ�����Ͼֵ�Ҫ��ʱ��
    EPG������֤ͨ������õ��ô˽ӿ��·��ֵ�Ҫ���������ʽ��
    var sValue = Utility.setValueByName('DVBSUpgradeTP', '{"frequency":frequency,"symbolRate":symbolRate,"polarization":polarization,"satLongitude":"satLongitude"}')
        frequency(int 2)        ����Ƶ���Ƶ�ʣ�������Flash��
        symbolRate(int 2)       ����Ƶ��ķ����ʣ�������Flash��
        polarization(int 1)     ����Ƶ��ļ�����ʽ��������Flash.0��ʾV����ֱ����;1��ʾH��ˮƽ����;2��ʾ��������;3��ʾ��������.
        satLongitude(String 6)  ����Ƶ�����������Ǿ��ȣ�������Flash��Ϊ��ʵ����(�����������3λ����ȷ��С�����һλ��������С������)�������������Ϊ���������硰115.5����ʾ����115.5�㣬��-15����ʾ����15�㡣
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
    if(tr069Upgrade){ // Indicates whether the ACS control CPE upgrade. If is true (1), the CPE should only use the ACS to seek available upgrades. If it is false (0), the CPE can use other methods to upgrade. IPTV SQM V100R001C29 STB���ն�������ϵͳ�ӿ�����ģ�ͣ�E7��.xls
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
    if(tr069Upgrade){ // Indicates whether the ACS control CPE upgrade. If is true (1), the CPE should only use the ACS to seek available upgrades. If it is false (0), the CPE can use other methods to upgrade. IPTV SQM V100R001C29 STB���ն�������ϵͳ�ӿ�����ģ�ͣ�E7��.xls
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
Description: ��ʼ����ΪUpgrade�Ľӿڣ���JseHWMaintenance.cpp����
Input: ��
Return: ��
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

    //C10 C20 ������������ַ(String 1024)��д��STB Flash��
    call = new JseFunctionCall("UpgradeServer", JseUpgradeServiceRead, JseUpgradeServiceWrite);
    JseRootRegist(call->name(), call);

    //C10 C20 UpgradesManaged��ָ�����Ƿ��ܿأ�������Ϊ0����1����������ACS���ն����ܣ����������ģ�ֻ��Ϊ1ʱ�������ܿصģ�ֻ����ACS����������
    call = new JseFunctionCall("UpgradesManaged", JseUpgradeTr069ManageRead, JseUpgradeTr069ManageWrite);
    JseRootRegist(call->name(), call);

    //C10 C20 ����������������ַ(String 1024)��д��STB Flash������C15��������
    call = new JseFunctionCall("UpgradeServerBackup", JseUpgradeBackupServiceRead, JseUpgradeBackupServiceWrite);
    JseRootRegist(call->name(), call);
    //C10 C20 ǿ��������־��1 is ǿ������
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

    //C20 ��������EPG�·�������������ַ
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

