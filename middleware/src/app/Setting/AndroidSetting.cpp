
#include "AndroidSetting.h"
#include "SettingListener.h"
#include "Assertions.h"

#include <string>
#include <map>
#include <utility>
#include <vector>
#include <stdio.h>

int androidSettingGetString(const char* name, char* value, int valueLen)
{
    if (!getAndroidSetting(name, value, valueLen)) {
        LogSafeOperDebug("androidSettingGetString get name = %s, value = %s\n", name, value);
        return 0;
    } else {
        LogSafeOperDebug("%s is not AndroidSetting!\n", name);
        return -1;
    }
}

int androidSettingGetInt(const char* name, int* value)
{
    char buf[32] = { 0 };
    if (!getAndroidSetting(name, buf, sizeof(buf))) {
        LogSafeOperDebug("androidSettingGetInt get name = %s, value = %s\n", name, buf);
        *value = atoi(buf);
        return 0;
    } else {
        LogSafeOperDebug("%s is not AndroidSetting!\n", name);
        return -1;
    }
}

int androidSettingSetString(const char* name, const char* value)
{
    if (!setAndroidSetting(name, value)) {
        LogSafeOperDebug("androidSettingSetString set name = %s, value = %s\n", name, value);
        return 0;
    } else {
        LogSafeOperDebug("%s is not AndroidSetting!\n", name);
        return -1;
    }
}

int androidSettingSetInt(const char* name, const int value)
{
    char buf[32] = { 0 };
    snprintf(buf, sizeof(buf), "%d", value);

    if (!setAndroidSetting(name, buf)) {
        LogSafeOperDebug("androidSettingSetInt set name = %s, value = %d\n", name, value);
        return 0;
    } else {
        LogSafeOperDebug("%s is not AndroidSetting!\n", name);
        return -1;
    }
}

