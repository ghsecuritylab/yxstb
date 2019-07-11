
#include "Assertions.h"
#include "AppSetting.h"
#include "config/pathConfig.h"
#if defined(CACHE_CONFIG)
#include "CacheSetting.h"
#endif
#if defined(ENABLE_BAK_SETTING)
#include "BakSetting.h"
#endif

#ifdef NEW_ANDROID_SETTING
#include "AndroidSetting.h"
#endif

#include "IPTVMiddleware.h"

namespace Hippo {

static AppSetting g_appSetting(CONFIG_FILE_DIR"/yx_config_customer.ini");

AppSetting::AppSetting(std::string fileName)
    : Setting(fileName)
{
}

AppSetting::~AppSetting()
{
}


int
AppSetting::recoverBakSeting()
{
#if defined(ENABLE_BAK_SETTING)
    std::vector<SettingItem*>::iterator it;
    SettingItem* item;
    char valueRead[1024] = {0};

    for (it = m_itemsArray.begin(); it != m_itemsArray.end(); ++it) {
        item = *it;
        if (bakSettingGetString(item->m_name.c_str(), valueRead, 1024, 0) == 0) {
            set(item->m_name.c_str(), valueRead);
        }
    }
    restore(0);
#endif

    return 0;
}

AppSetting& appSetting()
{
    return g_appSetting;
}

} // namespace Hippo

extern "C" {

int appSettingGetString(const char* name, char* value, int valueLen, int searchFlag)
{
    LogSafeOperDebug("appSettingGetString:%s\n", name);

#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    int ret = androidSettingGetString(name, value, valueLen);
    if (!ret)
        return ret;
#else
    if (strcmp(name, "epg") == 0
        || strcmp(name, "PADBootLogPicURL")  == 0
        || strcmp(name, "PADAuthenBackgroundPicURL") == 0
        || strcmp(name, "ntvAESpasswd") == 0
        || strcmp(name, "ntvuser") == 0
    ) {
        IPTVMiddleware_SettingGetStr(name, value, valueLen);
        return 0;
    }
#endif
#endif

    return Hippo::appSetting().get(name, value, valueLen);
}

int appSettingGetInt(const char* name, int* value, int searchFlag)
{
    LogSafeOperDebug("appSettingGetInt:%s\n", name);

#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    int ret = androidSettingGetInt(name, value);
    if (!ret)
        return ret;
#else
    if (strcmp(name, "volume") == 0
        || strcmp(name, "mute") == 0
        || strcmp(name, "PADIRetryTime") == 0
        || strcmp(name, "PADIRetryInterval") == 0
        || strcmp(name, "LCPRetryTime") == 0
        || strcmp(name, "LCPRetryInterval") == 0
        || strcmp(name, "DHCPRetryTime") == 0
        || strcmp(name, "DHCPRetryInterval") == 0
        || strcmp(name, "LinkProbeTime") == 0
    ) {
        char    temp[4096];
        IPTVMiddleware_SettingGetStr(name, temp, sizeof(temp));
        if (value)
            *value = atoi(temp);
        return 0;
    }
#endif
#endif

    return Hippo::appSetting().get(name, value);
}

int appSettingSetString(const char* name, const char* value)
{
    LogSafeOperDebug("appSettingSetString:%s\n", name);
    int ret = 0;

#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    ret = androidSettingSetString(name, value);
    if (!ret)
       return ret;
#else
    if (strcmp(name, "epg") == 0
        || strcmp(name, "ntvuser") == 0
        || strcmp(name, "ntvAESpasswd") == 0
        || strcmp(name, "PADBootLogPicURL")  == 0
        || strcmp(name, "PADAuthenBackgroundPicURL") == 0
       )
    {
        IPTVMiddleware_SettingSetStr(name, value);
    }
#endif
#endif

#if defined(CACHE_CONFIG)
    ret = appSettingSetStringForce(name, value, 0);
#else
    ret = Hippo::appSetting().set(name, value);
#if defined(C30) // Fixme.
    settingManagerSave();
#endif
#endif // defined(CACHE_CONFIG)

    return ret;
}

int appSettingSetInt(const char* name, const int value)
{
    LogSafeOperDebug("appSettingSetInt:%s\n", name);
    int ret = 0;

#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    ret = androidSettingSetInt(name, value);
    if (!ret)
        return ret;
#else
    if (strcmp(name, "volume") == 0
        || strcmp(name, "mute") == 0
        || strcmp(name, "PADIRetryTime") == 0
        || strcmp(name, "PADIRetryInterval") == 0
        || strcmp(name, "LCPRetryTime") == 0
        || strcmp(name, "LCPRetryInterval") == 0
        || strcmp(name, "DHCPRetryTime") == 0
        || strcmp(name, "DHCPRetryInterval") == 0
        || strcmp(name, "LinkProbeTime") == 0
    ) {
        char    temp[1024];
        snprintf(temp, sizeof(temp), "%d", value);
        IPTVMiddleware_SettingSetStr(name, temp);
        return 0;
    }
#endif
#endif

#if defined(CACHE_CONFIG)
    ret = appSettingSetIntForce(name, value, 0);
#else
    ret = Hippo::appSetting().set(name, value);
#if defined(C30) // Fixme.
    settingManagerSave();
#endif
#endif // defined(CACHE_CONFIG)

    return ret;
}

#if defined(CACHE_CONFIG)
/*
force = 0: 通过cache文件过滤
force = 1: 不通过cache文件过滤，直接设置到配置文件中
*/
int appSettingSetStringForce(const char* name, const char* value, const int force)
{
    int ret = 0;
    LogSafeOperDebug("appSettingSetString:%s\n", name);

    if (!force) {
        char buf[512 + 4] = {0};
        snprintf(buf, 512, "customer.string.%s", name);
        ret = cacheSettingSetString((const char*)buf, value);
        if (!ret)
            return ret;
    }

    ret = Hippo::appSetting().set(name, value);
#if defined(C30) // Fixme.
    settingManagerSave();
#endif
    return ret;
}

/*
force = 0: 通过cache文件过滤
force = 1: 不通过cache文件过滤，直接设置到配置文件中
*/
int appSettingSetIntForce(const char* name, const int value, const int force)
{
    int ret = 0;
    LogSafeOperDebug("appSettingSetInt:%s\n", name);

    if (!force) {
        char buf[512 + 4] = {0};
        snprintf(buf, 512, "customer.int.%s", name);
        ret = cacheSettingSetInt((const char*)buf, value);
        if (!ret)
            return ret;
    }

    ret = Hippo::appSetting().set(name, value);
#if defined(C30) // Fixme.
    settingManagerSave();
#endif
    return ret;
}
#endif // end #if defined(CACHE_CONFIG)
}


