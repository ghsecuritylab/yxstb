#include "UpgradeCommon.h"
#include "NetworkFunctions.h"
#include "UpgradeManager.h"
#include "stbinfo/stbinfo.h"
#include "AppSetting.h"
#include "SysSetting.h"
#include "VersionSetting.h"
#include "SettingEnum.h"

#include "customer.h"

#include "sys_basic_macro.h"

int constructRequestUpgradeAddrUrl(const char* ip, int port, char* upgradeUrl, char* requestUrl)
{
    int version = 0;
    char mac[64] = {0};
    char stbID[34] = {0};
    char softwareVersion[32] = {0};
    char hardwareVersion[32] = {0};
    char ntvUser[USER_LEN] = {0};
    int platform = 0;

#ifdef INCLUDE_LITTLESYSTEM
    bool status = Hippo::upgradeManager()->readUpgradeData();
    if (status) {
        Hippo::upgradeManager()->getUpgradeData("upgradeUrlflag", strValue);
        platform = atoi(strValue.c_str());
    } else
        platform = sys_platform_get();
#else
    platform = sys_platform_get();
#endif

    mid_sys_serial(stbID);
    version = upgrade_version_read(SOFTWARE_VERSION);
    get_upgrade_version(softwareVersion);
    HardwareVersion(hardwareVersion, 32);
    network_tokenmac_get(mac, 64, ':');
    appSettingGetString("ntvuser", ntvUser, USER_LEN, 0);
    if (platform == PLATFORM_ZTE) {
        sprintf(requestUrl, "%s?TYPE=%s&STBID=%s&MAC=%s&USER=%s&VER=%d&CHECKSUM=",
				    upgradeUrl, StbInfo::STB::UpgradeModel(), stbID, mac, ntvUser, version);
    } else {
        sprintf(requestUrl, "http://%s:%d/EDS/jsp/upgrade.jsp?TYPE=%s&STBID=%s&MAC=%s&USER=%s&VER=%d&SoftwareVersion=%s&SoftwareHWVersion=%s&HardwareVersion=%s&CHECKSUM=",
                    ip, port, StbInfo::STB::UpgradeModel(), stbID, mac, ntvUser, version, softwareVersion, StbInfo::STB::Version::HWVersion(),, hardwareVersion);
    }
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
                    ip, port, StbInfo::STB::UpgradeModel(), stbID, mac, ntv_user, version, softwareVersion, StbInfo::STB::Version::HWVersion(), hardwareVersion);
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
#ifdef hi3716m
    unsigned int hardwareVer = 0;
    if (yhw_board_getHWVersion(&hardwareVer) != 0)
        UpgradeLogDebug("yhw_board_getHWVersion ERROR!\n");
    if (hardwareVer >= 0x300) {
        if (getVersionNumAndFileName(pConfigInfo, "[UPGRADE_V3]", &softwareVersion, softwareName) < 0
            || getLogoVersionNumAndFileName(pConfigInfo, "[LOGO_B200]", &logoVersion, logoName, logomd5) < 0
            || getVersionNumAndFileName(pConfigInfo, "[CONFIG_B200]", &settingVersion, settingName) < 0)
            return -1;
    } else {
        if ((getVersionNumAndFileName(pConfigInfo, "[UPGRADE_V3]", &softwareVersion, softwareName) < 0
            && getVersionNumAndFileName(pConfigInfo, "[UPGRADE]", &softwareVersion, softwareName) < 0)
            || getLogoVersionNumAndFileName(pConfigInfo, "[LOGO_B200]", &logoVersion, logoName, logomd5) < 0
            || getVersionNumAndFileName(pConfigInfo, "[CONFIG_B200]", &settingVersion, settingName) < 0)
            return -1;
    }
#else
    if (getVersionNumAndFileName(pConfigInfo, "[UPGRADE_SECTION]", &softwareVersion, softwareName) < 0
        || getLogoVersionNumAndFileName(pConfigInfo, "[LOGO_B200]", &logoVersion, logoName, logomd5) < 0
        || getVersionNumAndFileName(pConfigInfo, "[CONFIG_B200]", &settingVersion, settingName) < 0)
        return -1;
#endif //hi3716m
#endif // INCLUDE_LITTLESYSTEM
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
            m_source->m_logoSourceAddr += logoName;
            m_source->m_settingSourceAddr += settingName;
        } else {
            m_source->m_softwareSourceAddr += "/";
            m_source->m_logoSourceAddr += "/";
            m_source->m_settingSourceAddr += "/";
            m_source->m_softwareSourceAddr += softwareName;
            m_source->m_logoSourceAddr += logoName;
            m_source->m_settingSourceAddr += settingName;
        }
    } else {
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
                    ip, port, StbInfo::STB::UpgradeModel(), stbID, mac, ntvUser, version, softwareVersion, StbInfo::STB::Version::HWVersion(), hardwareVersion, softwareName);
        m_source->m_softwareSourceAddr = url;
        sprintf(url, "http://%s:%d/UPGRADE/jsp/upgrade.jsp?TYPE=%s&STBID=%s&MAC=%s&USER=%s&VER=%d&SoftwareVersion=%s&SoftwareHWVersion=%s&HardwareVersion=%s&FILENAME=%s",
                ip, port, StbInfo::STB::UpgradeModel(), stbID, mac, ntvUser, version, softwareVersion, StbInfo::STB::Version::HWVersion(), hardwareVersion, logoName);
        m_source->m_logoSourceAddr = url;
        sprintf(url, "http://%s:%d/UPGRADE/jsp/upgrade.jsp?TYPE=%s&STBID=%s&MAC=%s&USER=%s&VER=%d&SoftwareVersion=%s&SoftwareHWVersion=%s&HardwareVersion=%s&FILENAME=%s",
                ip, port, StbInfo::STB::UpgradeModel(), stbID, mac, ntvUser, version, softwareVersion, StbInfo::STB::Version::HWVersion(), hardwareVersion, settingName);
        m_source->m_settingSourceAddr = url;
    }

    return 0;
}


void UpgradeSuccessed(int version)
{
    char userid[32] = {0};
    char password[64] = {0};

    if (version == 8845 || version == 7360 || version == 6370) {
    	int connectType = 0;
        sysSettingGetInt("connecttype", &connectType, 0);
        if(connectType == 2) {
            appSettingSetInt("epg_auth_flag", 0);
            sysSettingGetString("dhcpuser", userid, 32, 0);
            sysSettingSetString("netuser", userid);
            sysSettingGetString("ipoeAESpasswd", password, 64, 0);
            sysSettingSetString("netAESpasswd", password);
        }
    }
}