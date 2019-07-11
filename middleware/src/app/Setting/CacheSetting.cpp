
#include "Assertions.h"
#include "CacheSetting.h"
#include "config/pathConfig.h"
#include "SysSetting.h"
#include "AppSetting.h"
#include "SettingModule.h"

#include "build_info.h"

#include <stdio.h>
#include "stdlib.h"
#include "string.h"

namespace Hippo {

static CacheSetting g_cacheSetting(CONFIG_FILE_DIR"/yx_config_cache.ini");

CacheSetting::CacheSetting(std::string fileName)
    : Setting(fileName)
{
}

CacheSetting::~CacheSetting()
{
}

CacheSetting& cacheSetting()
{
    return g_cacheSetting;
}

} // namespace Hippo

#ifdef __cplusplus
extern "C" {
#endif

static int exchangeSetting(const int flag);

int cacheSettingGetString(const char* name, char* value, int valueLen, int searchFlag)
{
    LogSafeOperDebug("CacheSettingGetString:%s\n", name);
    return Hippo::cacheSetting().get(name, value, valueLen);
}

int cacheSettingGetInt(const char* name, int* value, int searchFlag)
{
    LogSafeOperDebug("CacheSettingGetInt:%s\n", name);
    return Hippo::cacheSetting().get(name, value);
}

int cacheSettingSetString(const char* name, const char* value)
{
    int ret = 0;
    LogSafeOperDebug("CacheSettingSetString:%s\n", name);
    ret = Hippo::cacheSetting().set(name, value);
    Hippo::cacheSetting().restore(0);
    return ret;
}

int cacheSettingSetInt(const char* name, const int value)
{
    int ret = 0;
    LogSafeOperDebug("CacheSettingSetInt:%s\n", name);
    ret = Hippo::cacheSetting().set(name, value);
    Hippo::cacheSetting().restore(0);
    return ret;
}

int cacheSettingLoad()
{
    FILE* fp = NULL;
    if ((fp = fopen(CONFIG_FILE_DIR"/yx_config_cache.ini", "rb")) == NULL) {
        Hippo::cacheSetting().restore(1);
        exchangeSetting(1);
    } else {
        char cacheSettingCFversion[512 + 4] = {0};
        int isFindVersionFlag = 0;
        while(fgets(cacheSettingCFversion, 512, fp) != NULL) {
            if (strstr(cacheSettingCFversion, "CacheSetting_CFversion") != NULL) {
                if ('\n' == cacheSettingCFversion[strlen(cacheSettingCFversion) - 1]
                || '\r' == cacheSettingCFversion[strlen(cacheSettingCFversion) - 1])
                    cacheSettingCFversion[strlen(cacheSettingCFversion) - 1] = 0;
                isFindVersionFlag = 1;
                char* totkenVersionPr = NULL;

                totkenVersionPr = strtok(cacheSettingCFversion, "=");
                if (totkenVersionPr) {
                    totkenVersionPr = strtok(NULL, "=");
                    if (totkenVersionPr)
                        if (strcmp(totkenVersionPr, g_make_build_CFversion))
                            isFindVersionFlag = 0; // version is not same. then rm old config file.
                }
                break;
            }
            memset(cacheSettingCFversion, 0, sizeof(cacheSettingCFversion));
        }
        if (0 == isFindVersionFlag) {
            std::string rmCacheFile("rm "CONFIG_FILE_DIR"/yx_config_cache.ini");
            system(rmCacheFile.c_str());
            Hippo::cacheSetting().restore(1);
            exchangeSetting(1);
        }
        fclose(fp);
    }
    Hippo::cacheSetting().load();
    Hippo::cacheSetting().restore(0);
    exchangeSetting(0);

    return 0;
}

/*
flag=0; CacheSetting to FlashSetting.
flag=1; FlashSetting to CacheSetting.
*/
static int exchangeSetting(const int flag)
{
#define NAME_LEN 512
#define VALUE_LEN 1024
#define CACHEDATA_LEN (NAME_LEN + VALUE_LEN)
#define CACHENAME_LEN (NAME_LEN + 50)

    std::string cacheFile(CONFIG_FILE_DIR"/yx_config_cache.ini");
    FILE* fp = NULL;

    if (flag) {
        std::string cacheFileBack("cp "CONFIG_FILE_DIR"/yx_config_cache.ini "CONFIG_FILE_DIR"/yx_config_cache.ini.back");
        system(cacheFileBack.c_str()); // for setting data.
        cacheFile = CONFIG_FILE_DIR"/yx_config_cache.ini.back";
    }

    char cacheData[CACHEDATA_LEN + 4] = {0};

    if ((fp = fopen(cacheFile.c_str(), "rb")) == NULL) {
        LogSafeOperWarn("CacheSetting file is not save to FlashSetting file!!\n");
        return -1;
    }

    while(fgets(cacheData, CACHEDATA_LEN, fp) != NULL) {
        if (strstr(cacheData, "=") != NULL) {
            if ('\n' == cacheData[strlen(cacheData) - 1]
                || '\r' == cacheData[strlen(cacheData) - 1])
                cacheData[strlen(cacheData) - 1] = 0;

            char cacheName[CACHENAME_LEN + 4] = {0};
            char name[NAME_LEN + 4] = {0};
            char value[VALUE_LEN + 4] = {0};
            char totkenName[NAME_LEN + 4] = {0};
            char* tokenNamePr = NULL;
            char* tokenValuePr = NULL;

            //printf("\n\n cacheData=[%s] \n\n ", cacheData);
            tokenValuePr = strtok(cacheData, "=");
            if (tokenValuePr) {
                strncpy(cacheName, tokenValuePr, CACHENAME_LEN);
                tokenValuePr = strtok(NULL, "=");
                if (tokenValuePr)
                    strncpy(value, tokenValuePr, VALUE_LEN);
            }

            strncpy(totkenName, cacheName, NAME_LEN);
            tokenNamePr = strtok(totkenName, ".");
            while(tokenNamePr) {
                memset(name, 0, sizeof(name));
                strncpy(name, tokenNamePr, NAME_LEN);
                tokenNamePr = strtok(NULL, ".");
            }
            //printf("cacheName=[%s],,,,,,,name=[%s],,,,,,,,,,,value=[%s] \n\n",
                //cacheName, name, value);

            if (strstr(cacheName, "system.string.") != NULL) {
                if (flag) {
                    char cacheValue[VALUE_LEN + 4] = {0};
                    sysSettingGetString((const char*)name, cacheValue, VALUE_LEN, 0);
                    cacheSettingSetString((const char*)cacheName, (const char*)cacheValue);
                } else {
                    sysSettingSetStringForce((const char*)name, (const char*)value, 1);
                }
            } else if (strstr(cacheName, "system.int.") != NULL) {
                if (flag) {
                    int cacheValue = 0;
                    sysSettingGetInt((const char*)name, &cacheValue, 0);
                    cacheSettingSetInt((const char*)cacheName, (const int)cacheValue);
                } else {
                    sysSettingSetIntForce((const char*)name, (const int)atoi(value), 1);
                }
            } else if (strstr(cacheName, "customer.string.") != NULL) {
                if (flag) {
                    char cacheValue[VALUE_LEN + 4] = {0};
                    appSettingGetString((const char*)name, cacheValue, VALUE_LEN, 0);
                    cacheSettingSetString((const char*)cacheName, (const char*)cacheValue);
                } else {
                    appSettingSetStringForce((const char*)name, (const char*)value, 1);
                }
            } else if (strstr(cacheName, "customer.int.") != NULL) {
                if (flag) {
                    int cacheValue = 0;
                    appSettingGetInt((const char*)name, &cacheValue, 0);
                    cacheSettingSetInt((const char*)cacheName, (const int)cacheValue);
                } else {
                    appSettingSetIntForce((const char*)name, (const int)atoi(value), 1);
                }
            } else {
                LogSafeOperWarn("[%s] Not found!!\n", cacheName);
            }
        }
        memset(cacheData, 0, sizeof(cacheData));
    }
    fclose(fp);
    cacheFile = "rm "CONFIG_FILE_DIR"/yx_config_cache.ini.back";
    system(cacheFile.c_str());

    return 0;
}

}

