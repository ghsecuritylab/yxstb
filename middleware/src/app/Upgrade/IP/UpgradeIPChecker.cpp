#include "UpgradeIPChecker.h"
#include "UpgradeAssertions.h"
#include "UpgradeCommon.h"

#include "MessageTypes.h"
#include "Message.h"
#include "UpgradeManager.h"
#include "UpgradeSource.h"
#include "UpgradeIPSource.h"
#include "HttpDataSource.h"
#include "RingBuffer.h"
#include "DataSink.h"
#include "AppSetting.h"
#include "SysSetting.h"
#include "Tr069.h"

#include "mid/mid_http.h"
#include "sys_basic_macro.h"
#include "mid/mid_tools.h"
#include "dns/mid_dns.h"

#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern char* global_cookies;

namespace Hippo {

UpgradeIPChecker::UpgradeIPChecker(UpgradeManager* manager)
    : UpgradeChecker(manager)
    , m_realUpgradePort(0)
    , m_isUseBakAddr(false)
{

}

UpgradeIPChecker::~UpgradeIPChecker()
{

}

bool
UpgradeIPChecker::start()
{
    if(m_state > UCS_IDLE)
        return false;

    UpgradeLogDebug("Upgrade checker start\n");
    m_type = UpgradeManager::UMUT_IP_SOFTWARE;
    m_state = UCS_WORKING;
    if(!m_source)
        m_source = new UpgradeIPSource();
    m_source->m_dataSource = new HttpDataSource();
    m_source->m_dataSource->setBuffer(m_source->getRingBuffer());
    if (!m_manager->upgradeTr069Contral()) {
        m_source->m_prePrompt = false;
        m_isUseBakAddr = false;
        sendEmptyMessage(Request_Upgrade_Address);
    } else {
        m_source->m_prePrompt = true;
        char upgradeUrl[512] = {0};
        TR069_GET_UPGRADE_URL(upgradeUrl, 512);
        if (upgradeUrl[0] != 0 && strcasecmp(upgradeUrl, "undefined")) {
            m_isUseBakAddr = true;
            sendEmptyMessage(Get_Config_File);
        } else {
            m_isUseBakAddr = false;
            sendEmptyMessage(Request_Upgrade_Address);
        }
    }

    return true;
}

bool
UpgradeIPChecker::stop()
{
    delete m_source;
    m_source = 0;
    m_state = UCS_IDLE;
    m_isUseBakAddr = false;
    return true;
}

bool
UpgradeIPChecker::reset()
{
    return false;
}

static int upgradeResponseIP(int type, char* buf, int len, int arg)
{
    char tIp[64 + 4] = {0};
    unsigned int addr = INADDR_NONE;
    int port = 0;
    int ret = 0;

    Hippo::UpgradeIPChecker *checker = (Hippo::UpgradeIPChecker *)arg;
    if(!checker) {
        UpgradeLogDebug("checker is null\n");
        return -1;
    }

    UpgradeLogDebug("type = %d, Buf = %s\n", type, buf);

    if((type != 0 && type != HTTP_OK_LOCATION)
        || mid_tool_checkURL(buf, tIp, &port)
        || mid_tool_resolvehost(tIp, &addr)) {
        checker->setErrorCode(Hippo::UpgradeManager::UMMI_GET_SERVER_FAILED);
        checker->setState(Hippo::UpgradeChecker::UCS_ERROR);
        checker->sendEmptyMessage(Hippo::UpgradeIPChecker::Check_End);
        return -1;
    }

    mid_tool_addr2string(addr, tIp);
    UpgradeLogDebug("Upgrade Server IP = %s, Port = %d\n", tIp, port);

    checker->setRealUpgradeIP(tIp);
    checker->setRealUpgradePort(port);
    checker->sendEmptyMessage(Hippo::UpgradeIPChecker::Get_Config_File);

    return 0;
}


static void dnsCallBack(int arg, int dnsmsg, unsigned int hostip)
{
    char tIp[64 + 4] = {0};
    char url[URL_MAX_LEN + 4] = {0};
    char requestUrl[URL_MAX_LEN + 4] = {0};
    int port = 0;

    Hippo::UpgradeIPChecker *checker = (Hippo::UpgradeIPChecker *)arg;
    if(!checker) {
        UpgradeLogDebug("checker is null \n");
        return;
    }
    if(0 != dnsmsg) {
        checker->setErrorCode(Hippo::UpgradeManager::UMMI_GET_SERVER_FAILED);
        checker->setState(Hippo::UpgradeChecker::UCS_ERROR);
        checker->sendEmptyMessage(Hippo::UpgradeIPChecker::Check_End);
        return;
    }

    checker->upgradeAddressGet(url, URL_MAX_LEN + 4);
    mid_tool_checkURL(url, tIp, &port);
    mid_tool_addr2string(hostip, tIp);
    if (constructRequestUpgradeAddrUrl(tIp, port, url, requestUrl))
        return;

    mid_http_call(requestUrl, (mid_http_f)upgradeResponseIP, arg, NULL, 0, global_cookies);

}

static int upgradeResponseConfig(int type, char* buf, int len, int arg)
{
    Hippo::UpgradeIPChecker *checker = (Hippo::UpgradeIPChecker *)arg;

    if(!checker) {
        UpgradeLogDebug("upgrade checker is null\n");
        return -1;
    }
    UpgradeLogDebug("response config file info type = %d\n name = %s\n", type, buf);
    if(type != 0) {
        checker->setErrorCode(Hippo::UpgradeManager::UMMI_GET_CONFIG_FAILED);
        checker->setState(Hippo::UpgradeChecker::UCS_ERROR);
        checker->sendEmptyMessage(Hippo::UpgradeIPChecker::Check_End);
        return -1;
    }

    checker->setUpgradeConfigInfo(buf);
    checker->sendEmptyMessage(Hippo::UpgradeIPChecker::Parse_Config_File);

    return 0;
}

void
UpgradeIPChecker::requestRealUpgradeAddr(void)
{
    char tIp[64 + 4] = {0};
    char url[URL_MAX_LEN + 4] = {0};
    char requestUrl[URL_MAX_LEN + 4] = {0};
    unsigned int ipaddr = 0;
    int port = 0;

    upgradeAddressGet(url, URL_MAX_LEN + 4);
    if(memcmp(url, "http://", 7) || mid_tool_checkURL(url, tIp, &port)) {
        setErrorCode(Hippo::UpgradeManager::UMMI_GET_SERVER_FAILED);
        m_state = UCS_ERROR;
        sendEmptyMessage(Check_End);
        return;
    }

    m_source->m_provider = m_manager->upgradeProvider();
    m_manager->setUpgradeProvider(0);

    UpgradeLogDebug("ip = %s\n", tIp);
    ipaddr = inet_addr(tIp);
    if(ipaddr == INADDR_ANY || ipaddr == INADDR_NONE) {
        int ret = mid_dns_resolve(tIp, (mid_dns_f)dnsCallBack, (int)this, 28);
        if(ret == -1) {
            setErrorCode(Hippo::UpgradeManager::UMMI_GET_SERVER_FAILED);
            m_state = UCS_ERROR;
            sendEmptyMessage(Check_End);
        }
        UpgradeLogDebug("prase resolve success\n");
        return;
    }

    /* Don't attemper upgrade server on liao ning ZTE plateform */
    if (constructRequestUpgradeAddrUrl(tIp, port, url, requestUrl))
        return;

    UpgradeLogDebug("url = %s\n", url);
    mid_http_call(requestUrl, (mid_http_f)upgradeResponseIP, (int)this, NULL, 0, global_cookies);

    return;
}


void
UpgradeIPChecker::downLoadConfigFile()
{
    char requestUrl[URL_MAX_LEN + 4] = {0};
    char upgradeUrl[URL_MAX_LEN + 4] = {0};

    if (!m_manager->upgradeTr069Contral()) {
        upgradeAddressGet(upgradeUrl, URL_MAX_LEN + 4);
        constructRequestConfigUrl(m_realUpgradeIP.c_str(), m_realUpgradePort, false, m_source->m_provider, upgradeUrl, requestUrl);
    } else {
        TR069_GET_UPGRADE_URL(upgradeUrl, URL_MAX_LEN + 4);
        if (upgradeUrl[0] != 0 && strcasecmp(upgradeUrl, "undefined"))
            constructRequestConfigUrl(m_realUpgradeIP.c_str(), m_realUpgradePort, true, m_source->m_provider, upgradeUrl, requestUrl);
        else {
            memset(upgradeUrl, 0, sizeof(upgradeUrl));
            upgradeAddressGet(upgradeUrl, URL_MAX_LEN + 4);
            constructRequestConfigUrl(m_realUpgradeIP.c_str(), m_realUpgradePort, false, m_source->m_provider, upgradeUrl, requestUrl);
        }
    }
    UpgradeLogDebug("requestUrl = %s\n", requestUrl);
    mid_http_call(requestUrl, (mid_http_f)upgradeResponseConfig, (int)this, NULL, 0, global_cookies);

    return;
}

void
UpgradeIPChecker::parseConfigFile()
{
    int ret = 0;
    std::string softwareUrl;
    std::string logoUrl;
    std::string settingUrl;
    char upgradeUrl[URL_MAX_LEN + 4] = {0};

    char *pConfigInfo = (char *)malloc(m_fileInfo.length() + 1);
    if(!pConfigInfo) {
        UpgradeLogDebug("parseConfigFile NULL == p\n");
        return;
    }
    memset(pConfigInfo, 0, m_fileInfo.length() + 1);
    memcpy(pConfigInfo, m_fileInfo.data(), m_fileInfo.length());

    UpgradeLogDebug("start parse config file\n");
    if (!m_manager->upgradeTr069Contral()) {
        upgradeAddressGet(upgradeUrl, URL_MAX_LEN + 4);
        ret = GetUpgradeFileUrl(pConfigInfo, m_realUpgradeIP.c_str(), m_realUpgradePort, upgradeUrl, false, m_source);
    } else {
        TR069_GET_UPGRADE_URL(upgradeUrl, URL_MAX_LEN + 4);
        if (upgradeUrl[0] != 0 && strcasecmp(upgradeUrl, "undefined"))
            ret = GetUpgradeFileUrl(pConfigInfo, m_realUpgradeIP.c_str(), m_realUpgradePort, upgradeUrl, true, m_source);
        else {
            memset(upgradeUrl, 0, sizeof(upgradeUrl));
            upgradeAddressGet(upgradeUrl, URL_MAX_LEN + 4);
            ret = GetUpgradeFileUrl(pConfigInfo, m_realUpgradeIP.c_str(), m_realUpgradePort, upgradeUrl, false, m_source);
        }
    }
    free(pConfigInfo);
    if (ret) {
        setErrorCode(Hippo::UpgradeManager::UMMI_PARSE_CONFIG_FAILED);
        m_state = UCS_ERROR;
        sendEmptyMessage(Check_End);
        return;
    }

    sysSettingSetInt("isUseBakAddr", m_isUseBakAddr);
    m_state = UCS_OK;
    sendEmptyMessage(Check_End);

    return;
}

void
UpgradeIPChecker::onCheckEnd()
{
#ifdef HUAWEI_C10
    if (!m_isUseBakAddr
        && m_state == UCS_ERROR
        && errorCode() == UpgradeManager::UMMI_GET_SERVER_FAILED) {
        m_isUseBakAddr = true;
        m_state = UCS_WORKING;
        m_realUpgradeIP.clear();
        sendEmptyMessage(Request_Upgrade_Address);
    } else
#endif
    {
        m_manager->sendEmptyMessage(UpgradeManager::UMMC_CHECK_END);
    }
}

void
UpgradeIPChecker::upgradeAddressGet(char* addr, int length)
{
#ifdef HUAWEI_C10
    if (m_isUseBakAddr) {
        sysSettingGetString("upgradeBackupUrl", addr, length, 0);
    } else
#endif
    {
        sysSettingGetString("upgradeUrl", addr, length, 0);
    }
}

void
UpgradeIPChecker::handleMessage(Message *msg)
{
    switch(msg->what) {
    case Request_Upgrade_Address:
        requestRealUpgradeAddr();
        break;
    case Get_Config_File:
        downLoadConfigFile();
        break;
    case Parse_Config_File:
        parseConfigFile();
        break;
    case Check_End:
    	onCheckEnd();
        break;
    default:
        break;
    }
}

}