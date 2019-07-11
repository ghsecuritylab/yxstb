
#include "TemplateIPUpgradeChecker.h"
#include "UpgradeAssertions.h"

#include "UpgradeManager.h"
#include "UpgradeIPSource.h"
#include "HttpDataSource.h"
#include "DataSource.h"
#include "RingBuffer.h"
#include "DataSink.h"
#include "SysSetting.h"
#include "customer.h"
#include <string.h>

namespace Hippo {

TemplateIPUpgradeChecker::TemplateIPUpgradeChecker(UpgradeManager* manager)
    : UpgradeChecker(manager)
    , m_httpDataSource(NULL)
{
}

TemplateIPUpgradeChecker::~TemplateIPUpgradeChecker()
{
    UpgradeLogDebug("~TemplateIPUpgradeChecker\n");
}


static int getTemplateVersion(char* templateUrl, int templateUrlLen, char* version)
{
    int j = 0;
    int tIndex = 0;

    // template url e.g. EPG/dvbs/EC2108.20121205.1122.tar.gz
    sysSettingGetString("templateUrl", templateUrl, templateUrlLen, 0);
    UpgradeLogDebug("templateUrl = %s\n", templateUrl);
    for (int i = 0; i < strlen(templateUrl); i++) {
         if (templateUrl[i] == '.')
             tIndex++;
         if (tIndex == 3)
             break;
         if (tIndex == 2 && templateUrl[i] != '.') {
             if (j >= 10)
                return -1;
             version[j++] = templateUrl[i];
             UpgradeLogDebug("version is %s\n", version);
         }
    }
    UpgradeLogDebug("version = %s\n", version);

    return 0;
}

bool
TemplateIPUpgradeChecker::start()
{
    char versionNum[10] = {0};
    char templateUrl[512] = {0};
    char epgUrl[1024] = {0};
    char* p1 = NULL;
    char* p2 = NULL;

	m_type = UpgradeManager::UMUT_IP_TEMPLATE;
	m_state = UCS_WORKING;

    if (!m_source)
        m_source = new UpgradeIPSource();

    if (!m_httpDataSource)
        m_httpDataSource = new HttpDataSource();
    m_source->m_dataSource = m_httpDataSource;
    UpgradeLogDebug("start check template info\n");
    m_httpDataSource->setBuffer(m_source->getRingBuffer());
    if (getTemplateVersion(templateUrl, 512, versionNum)) {
    	UpgradeLogDebug("get template version error\n");
        m_state = UCS_ERROR;
        setErrorCode(UpgradeManager::UMMI_UPGRADE_TEMPLATE_FAILED);
        m_manager->sendEmptyMessage(UpgradeManager::UMMC_CHECK_END);
        return false;
    }
    m_source->m_version = atoi(versionNum);
    m_source->m_prePrompt = true;
    UpgradeLogDebug("template version is [%d]\n", m_source->m_version);

    std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();
    if (url.empty()) {
        m_state = UCS_ERROR;
        setErrorCode(UpgradeManager::UMMI_UPGRADE_TEMPLATE_FAILED);
        m_manager->sendEmptyMessage(UpgradeManager::UMMC_CHECK_END);
        return false;
    }
    url += std::string(templateUrl);

    UpgradeLogDebug("template url is [%s]\n", url.c_str());

    //m_httpDataSource->SetRequestUrl(epgUrl);
    m_source->m_softwareSourceAddr = url;
    m_source->setSourceAddress(UpgradeManager::UMUC_SOFTWARE);
    m_state = UCS_OK;
    m_manager->sendEmptyMessage(UpgradeManager::UMMC_CHECK_END);
    return true;
}

bool
TemplateIPUpgradeChecker::stop()
{
    delete m_source;
    m_source = 0;
    if (m_httpDataSource) {
        delete m_httpDataSource;
        m_httpDataSource = 0;
    }
    m_state = UCS_IDLE;
    return true;
}


}









