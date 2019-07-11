#include "UpgradeCommon.h"
#include "NetworkFunctions.h"
#include "UpgradeSource.h"
#include "stbinfo/stbinfo.h"
#include "AppSetting.h"
#include "SysSetting.h"
#include "SettingEnum.h"
#include "VersionSetting.h"

#include "customer.h"

#include "mid_sys.h"
#include "sys_basic_macro.h"
#include "app_sys.h"

#include <stdlib.h>
#include <string.h>

namespace Hippo {

int constructRequestUpgradeAddrUrl(const char* ip, int port, char* upgradeUrl, char* requestUrl)
{
    int version = 0;
    char mac[64] = {0};
    char stbID[34] = {0};
    char softwareVersion[32] = {0};
    char hardwareVersion[32] = {0};
    char ntvUser[USER_LEN] = {0};

    mid_sys_serial(stbID);
    version = upgrade_version_read(SOFTWARE_VERSION);
    get_upgrade_version(softwareVersion);
    HardwareVersion(hardwareVersion, 32);
    network_tokenmac_get(mac, 64, ':');
    appSettingGetString("ntvuser", ntvUser, USER_LEN, 0);

    sprintf(requestUrl, "http://%s:%d/EDS/jsp/upgrade.jsp?TYPE=%s&STBID=%s&MAC=%s&USER=%s&VER=%d&SoftwareVersion=%s&SoftwareHWVersion=%s&HardwareVersion=%s&CHECKSUM=",
            ip, port, StbInfo::STB::UpgradeModel(), stbID, mac, ntvUser, version, softwareVersion, StbInfo::STB::Version::HWVersion(), hardwareVersion);

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
        if (provider == 0) {
            int version = 0;
            char mac[64] = {0};
            char stbID[34] = {0};
            char softwareVersion[32] = {0};
            char hardwareVersion[32] = {0};
            char ntvUser[USER_LEN] = {0};

            mid_sys_serial(stbID);
            get_upgrade_version(softwareVersion);
            HardwareVersion(hardwareVersion, 32);
            network_tokenmac_get(mac, 64, ':');
            appSettingGetString("ntvuser", ntvUser, USER_LEN, 0);
            version = upgrade_version_read(SOFTWARE_VERSION);

            sprintf(requestUrl, "http://%s:%d/UPGRADE/jsp/upgrade.jsp?TYPE=%s&STBID=%s&MAC=%s&USER=%s&VER=%d&SoftwareVersion=%s&SoftwareHWVersion=%s&HardwareVersion=%s&CHECKSUM=",
                        ip, port, StbInfo::STB::UpgradeModel(), stbID, mac, ntvUser, version, softwareVersion, StbInfo::STB::Version::HWVersion(), hardwareVersion);
            urlCheckSum(requestUrl);
        } else {
            sprintf(requestUrl, "%s/000006/config.ini", upgradeUrl);
        }
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

    if (getVersionNumAndFileName(pConfigInfo, "[UPGRADE_SECTION]", &softwareVersion, softwareName) < 0
        || getLogoVersionNumAndFileName(pConfigInfo, "[LOGO_B200]", &logoVersion, logoName, logomd5) < 0
        || getVersionNumAndFileName(pConfigInfo, "[CONFIG_B200]", &settingVersion, settingName) < 0)
        return -1;

    source->m_version = softwareVersion;
    source->m_logoVersion = logoVersion;
    source->m_logomd5 = logomd5;
    source->m_settingVersion = settingVersion;

    if (isTr069) {
        source->m_softwareSourceAddr += upgradeUrl;
        source->m_logoSourceAddr += upgradeUrl;
        source->m_settingSourceAddr += upgradeUrl;
        if(upgradeUrl[strlen(upgradeUrl) - 1] == '/') {
            source->m_softwareSourceAddr += softwareName;
            source->m_logoSourceAddr += logoName;
            source->m_settingSourceAddr += settingName;
        } else {
            source->m_softwareSourceAddr += "/";
            source->m_logoSourceAddr += "/";
            source->m_settingSourceAddr += "/";
            source->m_softwareSourceAddr += softwareName;
            source->m_logoSourceAddr += logoName;
            source->m_settingSourceAddr += settingName;
        }
    } else {
        if (source->m_provider == 0) { //ctc
            int version = 0;
            char mac[64] = {0};
            char ntvUser[USER_LEN] = {0};
            char stbID[34] = {0};
            char strSoftwareVersion[32] = {0};
            char hardwareVersion[32] = {0};
            char url[512] = {0};

            mid_sys_serial(stbID);
            network_tokenmac_get(mac, 64, ':');
            appSettingGetString("ntvuser", ntvUser, USER_LEN, 0);
            get_upgrade_version(strSoftwareVersion);
            HardwareVersion(hardwareVersion, 32);
            version = upgrade_version_read(SOFTWARE_VERSION);

            sprintf(url, "http://%s:%d/UPGRADE/jsp/upgrade.jsp?TYPE=%s&STBID=%s&MAC=%s&USER=%s&VER=%d&SoftwareVersion=%s&SoftwareHWVersion=%s&HardwareVersion=%s&FILENAME=%s",
                        ip, port, StbInfo::STB::UpgradeModel(), stbID, mac, ntvUser, version, strSoftwareVersion, StbInfo::STB::Version::HWVersion(), hardwareVersion, softwareName);
            source->m_softwareSourceAddr = url;
            sprintf(url, "http://%s:%d/UPGRADE/jsp/upgrade.jsp?TYPE=%s&STBID=%s&MAC=%s&USER=%s&VER=%d&SoftwareVersion=%s&SoftwareHWVersion=%s&HardwareVersion=%s&FILENAME=%s",
                    ip, port, StbInfo::STB::UpgradeModel(), stbID, mac, ntvUser, version, strSoftwareVersion, StbInfo::STB::Version::HWVersion(), hardwareVersion, logoName);
            source->m_logoSourceAddr = url;
            sprintf(url, "http://%s:%d/UPGRADE/jsp/upgrade.jsp?TYPE=%s&STBID=%s&MAC=%s&USER=%s&VER=%d&SoftwareVersion=%s&SoftwareHWVersion=%s&HardwareVersion=%s&FILENAME=%s",
                    ip, port, StbInfo::STB::UpgradeModel(), stbID, mac, ntvUser, version, strSoftwareVersion, StbInfo::STB::Version::HWVersion(), hardwareVersion, settingName);
            source->m_settingSourceAddr = url;
        } else { // cu
            source->m_softwareSourceAddr += upgradeUrl;
            source->m_logoSourceAddr += upgradeUrl;
            source->m_settingSourceAddr += upgradeUrl;
            if(upgradeUrl[strlen(upgradeUrl) - 1] == '/') {
                source->m_softwareSourceAddr += "000006/";
                source->m_logoSourceAddr += "000006/";
                source->m_settingSourceAddr += "000006/";
            } else {
                source->m_softwareSourceAddr += "/000006/";
                source->m_logoSourceAddr += "/000006/";
                source->m_settingSourceAddr += "/000006/";
            }
            source->m_softwareSourceAddr += softwareName;
            source->m_logoSourceAddr += logoName;
            source->m_settingSourceAddr += settingName;
        }
    }

    return 0;
}

void UpgradeSuccessed(int version)
{

}

}