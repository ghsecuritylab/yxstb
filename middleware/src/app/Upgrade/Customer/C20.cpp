#include "UpgradeCommon.h"
#include "NetworkFunctions.h"
#include "stbinfo/stbinfo.h"
#include "AppSetting.h"
#include "SysSetting.h"
#include "SettingEnum.h"

#include "customer.h"

#include "sys_basic_macro.h"

int constructRequestUpgradeAddrUrl(const char* ip, int port, char* upgradeUrl, char* requestUrl)
{
    int version = 0;
    char mac[64] = {0};
    char ntvUser[USER_LEN] = {0};

    version = upgrade_version_read(SOFTWARE_VERSION);
    network_tokenmac_get(mac, 64, ':');
    appSettingGetString("ntvuser", ntvUser, USER_LEN, 0);

    sprintf(url, "http://%s:%d/EDS/jsp/upgrade.jsp?TYPE=%s&MAC=%s&USER=%s&VER=%d&CHECKSUM=",
			    tIp, port, StbInfo::STB::UpgradeModel(), mac, ntvUser, version);

    urlCheckSum(requestUrl);

    return 0;

}

int constructRequestConfigUrl(const char* ip, int port, bool isTr069, int provider, char* upgradeUrl, char* requestUrl)
{
    if (isTr069 && upgradeUrl) {
       if(upgradeUrl[strlen(upgradeUrl) - 1] == '/')
            sprintf(requestUrl, "%sconfig.ini", upgradeUrl);
        else
            sprintf(requestUrl, "%s/config.ini", upgradeUrl);
    } else {
        int version = 0;
        char mac[64] = {0};
        char ntvUser[USER_LEN] = {0};

        network_tokenmac_get(mac, 64, ':');
        appSettingGetString("ntvuser", ntvUser, USER_LEN, 0);
        version = upgrade_version_read(SOFTWARE_VERSION);

        sprintf(url, "http://%s:%d/UPGRADE/jsp/upgrade.jsp?TYPE=%s&MAC=%s&USER=%s&VER=%d&CHECKSUM=",
                ip, port, StbInfo::STB::UpgradeModel(), mac, ntvUser, version);

        urlCheckSum(requestUrl);
    }

    return 0;
}


int GetUpgradeFileUrl(char* pConfigInfo, const char* ip, int port, char* upgradeUrl, bool isTr069, UpgradeSource* source)
{
    char softwareName[128] = {0};
    char logoName[128] = {0};
    char logomd5[64+1] = {0};
    char settingName[128] = {0};
    int softwareVersion = 0;
    int logoVersion = 0;
    int settingVersion = 0;

#ifdef INCLUDE_LITTLESYSTEM
    if (getVersionNumAndFileName(pConfigInfo, "[LITTLE_SYSTEM]", &softwareVersion, softwareName) < 0 )
        return -1;
#else
    if (getVersionNumAndFileName(pConfigInfo, "[UPGRADE_SECTION]", &softwareVersion, softwareName) < 0
        || getLogoVersionNumAndFileName(pConfigInfo, "[LOGO_B200]", &logoVersion, logoName, logomd5) < 0
        || getVersionNumAndFileName(pConfigInfo, "[CONFIG_B200]", &settingVersion, settingName) < 0)
        return -1;
#endif
    m_source->m_version = softwareVersion;
    m_source->m_logoVersion = logoVersion;
    m_source->m_logomd5 = logomd5;
    m_source->m_settingVersion = settingVersion;

    if (isTr069) {
        m_source->m_softwareSourceAddr += upgradeUrl;
        m_source->m_logoSourceAddr += upgradeUrl;
        m_source->m_settingSourceAddr += upgradeUrl;
        if(upgradeUrl[strlen(upgradeUrl) - 1] == '/') {
            m_source->m_softwareSourceAddr += softwareName;
            m_source->m_logoSourceAddr += logoName
            m_source->m_settingSourceAddr += settingName
        } else {
            m_source->m_softwareSourceAddr += "/";
            m_source->m_logoSourceAddr += "/"
            m_source->m_settingSourceAddr += "/"
            m_source->m_softwareSourceAddr += softwareName;
            m_source->m_logoSourceAddr += logoName
            m_source->m_settingSourceAddr += settingName
        }
    } else {
        char url[512] = {0};
        sprintf(url, "http://%s:%d/UPGRADE/jsp/upgrade.jsp?TYPE=%s&FILENAME=%s", ip, port, StbInfo::STB::UpgradeModel(), softwareName);
        m_source->m_softwareSourceAddr = url;
        sprintf(url, "http://%s:%d/UPGRADE/jsp/upgrade.jsp?TYPE=%s&FILENAME=%s", ip, port, StbInfo::STB::UpgradeModel(), logoName);
        m_source->m_logoSourceAddr = url;
        sprintf(url, "http://%s:%d/UPGRADE/jsp/upgrade.jsp?TYPE=%s&FILENAME=%s", ip, port, StbInfo::STB::UpgradeModel(), settingName);
        m_source->m_settingSourceAddr = url;
    }

}

void UpgradeSuccessed(int version)
{

}
