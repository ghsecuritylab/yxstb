
#include "JseUpgrade.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "UpgradeSource.h"
#include "UpgradeManager.h"
#include "BrowserAgent.h"
#include "UtilityTools.h"

#include "scbt_global.h"
#include "SettingEnum.h"
#include "SysSetting.h"

#include <stdio.h>
#include <stdlib.h>

extern "C" int yos_systemcall_runSystemCMD(char *buf, int *ret);

using namespace Hippo;

static int JseUpgradeServerEndWrite( const char* param, char* value, int len )
{
#if defined(VIETTEL_HD)&&defined(hi3716m)
    if (atoi(value) == 1) {
        epgBrowserAgentCleanTakinCache();
        yos_systemcall_runSystemCMD("echo 3 > /proc/sys/vm/drop_caches",NULL);
    }
#endif

    return 0;
}

#if defined (HUAWEI_C20)

static int JseUpgradeTypeRead( const char* param, char* value, int len )
{
    //upgrade type(software or template) are differentiate by this flag. used in html check is or not reboot STB.
    snprintf(value, len, "%d", upgradeManager()->upgradeTypeGet());

    return 0;
}

static int JseUpgradeTemplateInfoRead( const char* param, char* value, int len )
{
    int ret = -1;

    if (access(DEFAULT_TEMPLATE_DATAPTH"/template.tar.gz", R_OK | F_OK ) < 0) {
        sprintf(value, "%d", ret);
    } else {
#if defined (VERIFY_UPGRADE_FILE)
        ret = scbt_api_verify_and_generate_without_header_file(DEFAULT_TEMPLATE_DATAPTH"/template.tar.gz", "/var/temp.tar.gz");
#endif
		if (ret != 1) {
            upgrade_version_write(TEMPLATE_VERSION, 0);
            sysSettingSetString("templateUrl", "null");
            sprintf(value, "%d", -1);
        } else {
            ret = -1;
            yos_systemcall_runSystemCMD("tar zxvf /var/temp.tar.gz -C /var", &ret);
            removeFile("/var/temp.tar.gz");
            sprintf(value, "%d", ret);
        }
    }

    return 0;
}

#endif

static int JseStartUpgradeCheckVersionWrite( const char* param, char* value, int len )
{
    upgradeManager()->touchOffUpgradeCheck(UpgradeManager::UMUT_IP_SOFTWARE, false);

    return 0;
}

static int JseCurrentCheckedUpgradeVersionRead( const char* param, char* value, int len )
{
    int version = 0;
    //version = upgradeManager()->checkCurrentUpgradeVersion();
    if(0)//(UpgradeManager::UMUT_IP_SOFTWARE == upgradeManager()->getUpgradeType())
        sprintf(value, "%d", version);
    else
        sprintf(value, " ");

    return 0;
}

#if defined(Jiangsu)

static int JseCurrentUpgradeVersionRead( const char* param, char* value, int len )
{
    UpgradeSource* source = upgradeManager()->upgradeSource();
    if (!source)
        return;

    if (source->Type() == UpgradeManager::UMUT_IP_SOFTWARE)
        sprintf(aFieldValue, "%d", source->m_version);
    else
        sprintf(aFieldValue, "%s",  " ");

    return 0;
}
#endif

JseUpgrade::JseUpgrade()
	: JseGroupCall("upgrade")
{
    JseCall *call;

    call = new JseFunctionCall("start", 0, JseStartUpgradeCheckVersionWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("checkNewVersion", JseCurrentCheckedUpgradeVersionRead, 0);
    regist(call->name(), call);

}

JseUpgrade::~JseUpgrade()
{
}

/*************************************************
Description: 初始化华为Upgrade的接口，由JseHWMaintenance.cpp调用
Input: 无
Return: 无
*************************************************/
int JseUpgradeInit()
{
    JseCall* call;

    //C10 C20
    call = new JseFunctionCall("yx_para_upgrade_end", 0, JseUpgradeServerEndWrite);
    JseRootRegist(call->name(), call);


#if defined (HUAWEI_C20)

    //C20
    call = new JseFunctionCall("YX_UpgradeType", JseUpgradeTypeRead, 0);
    JseRootRegist(call->name(), call);

    //C20
    call = new JseFunctionCall("YX_ISTemplateExist", JseUpgradeTemplateInfoRead, 0);
    JseRootRegist(call->name(), call);
#endif
#if defined (HUAWEI_C10)
#if defined(Jiangsu)

    //C10
    call = new JseFunctionCall("newUpgradeVersionGet", JseCurrentUpgradeVersionRead, 0);
    JseRootRegist(call->name(), call);
#endif
#endif
    return 0;
}

