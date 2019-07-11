

#include "stbinfo.h"
#include "SysSetting.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "IPTVMiddleware.h"

// TODO
// 还没用到，后续补齐。
class StbResources {
public:
    StbResources() {}
    void Init() {
        static int inited = false;
        if (inited)
            return;
        std::string um(DEFAULT_UPGRADE_TYPE);
        size_t pos = um.find_first_of("_");

        if (pos == std::string::npos) {
            mUpgradeModelA = um;
            mUpgradeModelB = um + std::string("_dj");
            mUpgradeModelC = um + std::string("_hotel");
        } else {
            std::string prefix(um.substr(0, pos));
            mUpgradeModelA = prefix + std::string("_pub");
            mUpgradeModelB = prefix + std::string("_dj");
            mUpgradeModelC = prefix + std::string("_hotel");
        }
        inited = true;
    }
    ~StbResources() {}

    const char * UpgradeModel(char index) {
        Init();
        switch (index) {
        case 'B': case 'b': case 1:
            return mUpgradeModelB.c_str();
        case 'C': case 'c': case 2:
            return mUpgradeModelC.c_str();
        default:
            return mUpgradeModelA.c_str();
        }
    }

private:
    std::string mUpgradeModelA;
    std::string mUpgradeModelB;
    std::string mUpgradeModelC;
};


static StbResources g_resources;

namespace StbInfo {
namespace STB {

namespace Version {
    const char * Version(void)
    {
#ifdef ANDROID
        static char version[4096];
#ifdef NEW_ANDROID_SETTING
        sysSettingGetString("SoftwareVersion", version, sizeof(version), 0);
#else
        IPTVMiddleware_SettingGetStr("SoftwareVersion", version, sizeof(version));
#endif
        return version;
#else
        return g_make_build_version;
#endif
    }
    const char * HWVersion(void)
    {
#ifdef ANDROID
        const char * p = strrchr(HWVersionWithPrefix(), ' ');
        if (!p)
            return HWVersionWithPrefix();
        return (p + 1);
#else
        return HWVERSION;
#endif
    }
    const char * HWVersionWithPrefix(void)
    {
#ifdef ANDROID
        static std::string g_hwversion;
        if (g_hwversion.empty()) {
            char    version[4096];
#ifdef NEW_ANDROID_SETTING
            sysSettingGetString("SoftwareHWversion", version, sizeof(version), 0);
#else
            IPTVMiddleware_SettingGetStr("SoftwareHWversion", version, sizeof(version));
#endif
            g_hwversion = std::string(version);
        }
        return g_hwversion.c_str();
#else
        return "IPTV STB "HWVERSION;
#endif
    }
}

const char * UpgradeModel(char index) {
    return g_resources.UpgradeModel(index);
}

const char * UpgradeModel(void) {
    char softType[2] = {0};
    sysSettingGetString("stb_softtype", softType, 2, 0);
    return UpgradeModel(softType[0]);
}
}
}



extern "C" const char * HwSoftwareVersion(int flag)
{
    if (flag)
        return StbInfo::STB::Version::HWVersion();
    else
        return StbInfo::STB::Version::HWVersionWithPrefix();
}

extern "C" const char * StbModel(void)
{
    return StbInfo::STB::Model();
}

extern "C" const char * StbUpgradeModel(void)
{
    return StbInfo::STB::UpgradeModel();
}

extern "C" int StbUpgradeVersion(char * out)
{
    sprintf(out, "%s", StbInfo::STB::Version::Version());
    return 0;
}

extern "C" const char* StbVersionBuildtime(void)
{
    return StbInfo::STB::Version::BuildTime();
}


