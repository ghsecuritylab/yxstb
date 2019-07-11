
#include "Assertions.h"
#include "SysSetting.h"

#if defined(ENABLE_BAK_SETTING)
#include "BakSetting.h"
#endif
#if defined(CACHE_CONFIG)
#include "CacheSetting.h"
#endif
#ifdef NEW_ANDROID_SETTING
#include "AndroidSetting.h"
#endif

#include <stdio.h>
#include "config/pathConfig.h"
#include "IPTVMiddleware.h"

namespace Hippo {

static SysSetting g_sysSetting(CONFIG_FILE_DIR"/yx_config_system.ini");

SysSetting::SysSetting(std::string fileName)
    : Setting(fileName)
{
}

SysSetting::~SysSetting()
{
}

int
SysSetting::recoverBakSeting()
{
#if defined(ENABLE_BAK_SETTING)
    std::vector<SettingItem*>::iterator it;
    SettingItem* item;
    char valueRead[1024] = {0};

    for (it = m_itemsArray.begin(); it != m_itemsArray.end(); ++it) {
        item = *it;
        if (bakSettingGetString(item->m_name.c_str(), valueRead, 1024, 0) == 0)
            set(item->m_name.c_str(), valueRead);
    }
    restore(0);
#endif
    return 0;
}

SysSetting& sysSetting()
{
    return g_sysSetting;
}

} // namespace Hippo

extern "C" {

int sysSettingGetString(const char* name, char* value, int valueLen, int searchFlag)
{
    LogSafeOperDebug("sysSettingGetString:%s\n", name);
#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    int ret = androidSettingGetString(name, value, valueLen);
    if (!ret)
        return ret;
#else
    if (strcmp(name, "bootlogo_md5") == 0
        || strcmp(name, "authbg_md5") == 0
        || strcmp(name, "TransportProtocol") == 0
    ) {
        IPTVMiddleware_SettingGetStr(name, value, valueLen);
        return 0;
    }
#endif
#endif

    return Hippo::sysSetting().get(name, value, valueLen);
}

int sysSettingGetInt(const char* name, int* value, int searchFlag)
{
    LogSafeOperDebug("sysSettingGetInt:%s\n", name);
// Fixme:
#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    int ret = androidSettingGetInt(name, value);
    if (!ret)
        return ret;
#else
    if (strcmp(name, "changevideomode") == 0
        || strcmp(name, "lastChannelPlay") == 0
        || strcmp(name, "TransportProtocol") == 0
    ) {
        char    buffer[4096];
        IPTVMiddleware_SettingGetStr(name, buffer, sizeof(buffer));
        *value = atoi(buffer);
        return 0;
    }
#endif
#endif

    return Hippo::sysSetting().get(name, value);
}

int sysSettingSetString(const char* name, const char* value)
{
    LogSafeOperDebug("sysSettingSetString:%s\n", name);
    int ret = 0;

#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    ret = androidSettingSetString(name, value);
    if (!ret)
        return ret;
#else
    if (strcmp(name, "bootlogo_md5") == 0
        || strcmp(name, "authbg_md5") == 0
        || strcmp(name, "TransportProtocol") == 0
    ) {
        IPTVMiddleware_SettingSetStr(name, value);
    }
#endif
#endif

#if defined(CACHE_CONFIG)
    ret = sysSettingSetStringForce(name, value, 0);
#else
    ret = Hippo::sysSetting().set(name, value);
#if defined(C30) // Fixme: 后续页面修改后, 统一回去
    settingManagerSave();
#endif
#endif //end defined(CACHE_CONFIG)

    return ret;
}

int sysSettingSetInt(const char* name, const int value)
{
    LogSafeOperDebug("sysSettingSetInt:%s\n", name);
    int ret = 0;

#ifdef ANDROID
#ifdef NEW_ANDROID_SETTING
    ret = androidSettingSetInt(name, value);
    if (!ret)
        return ret;
#else
    if (strcmp(name, "changevideomode") == 0
        || strcmp(name, "lastChannelPlay") == 0
        || strcmp(name, "TransportProtocol") == 0
       )
    {
        char    buffer[4096];
        snprintf(buffer, sizeof(buffer), "%d", value);
        IPTVMiddleware_SettingSetStr(name, buffer);
        return 0;
    }
#endif
#endif

#if defined(CACHE_CONFIG)
    ret = sysSettingSetIntForce(name, value, 0);
#else
    ret = Hippo::sysSetting().set(name, value);
#if defined(C30) // Fixme:
    settingManagerSave();
#endif
#endif // end defined(CACHE_CONFIG)

    return ret;
}

#if defined(CACHE_CONFIG)
/*
force = 0: 通过cache文件过滤
force = 1: 不通过cache文件过滤，直接设置到配置文件中
*/
int sysSettingSetStringForce(const char* name, const char* value, const int force)
{
    int ret = 0;
    LogSafeOperDebug("sysSettingSetStringForce:%s\n", name);

    if (!force) {
        char buf[512 + 4] = {0};
        snprintf(buf, 512, "system.string.%s", name);
        ret = cacheSettingSetString((const char*)buf, value);
        if (!ret)
            return ret;
    }

    ret = Hippo::sysSetting().set(name, value);
#if defined(C30) // Fixme: 后续页面修改后, 统一回去
    settingManagerSave();
#endif
    return ret;
}

/*
force = 0: 通过cache文件过滤
force = 1: 不通过cache文件过滤，直接设置到配置文件中
*/
int sysSettingSetIntForce(const char* name, const int value, const int force)
{
    int ret = 0;
    LogSafeOperDebug("sysSettingSetIntForce:%s\n", name);

    if (!force) {
        char buf[512 + 4] = {0};
        snprintf(buf, 512, "system.int.%s", name);
        ret = cacheSettingSetInt((const char*)buf, value);
        if (!ret)
            return ret;
    }

    ret = Hippo::sysSetting().set(name, value);
#if defined(C30) // Fixme:
    settingManagerSave();
#endif
    return ret;
}
#endif //end #if defined(CACHE_CONFIG)

}


