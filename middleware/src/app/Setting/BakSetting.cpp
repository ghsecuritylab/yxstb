
#include "BakSetting.h"

#include "Assertions.h"
#include "AppSetting.h"
#include "SysSetting.h"
#include "config/pathConfig.h"
#include "libzebra.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BAK_SETTING_FILE DEFAULT_TEMP_DATAPATH"/yx_config_bak.ini"

namespace Hippo {

static BakSetting g_bakSetting(BAK_SETTING_FILE);

BakSetting::BakSetting(std::string fileName)
    : Setting(fileName)
{
}

BakSetting::~BakSetting()
{
}

int
BakSetting::load()
{
    int ret = 0;
#if defined(hi3560e)
#else
    ret = yhw_env_checkfile(BAK_SETTING_FILE);
#endif
    if(!ret)
        ret = Setting::load();

    return ret;
}

int
BakSetting::backup()
{
    char *filename[1] = {NULL};
	int ret = 0;
	filename[0] = (char*)malloc(64);
	if(!filename[0]){
		 LogSafeOperError("malloc error\n");
		 return -1;
	}

    restore(0);
	memset(filename[0], 0, 64);
    memcpy(filename[0], m_fileName.c_str(), m_fileName.length());
#if defined(hi3560e)
#else
	ret = yhw_env_bakfile(1,(const char**)filename);
#endif
	free(filename[0]);

	if(ret != 0){
	    LogSafeOperError("bak file error\n");
	    return -1;
	}
	return 0;
}

int
BakSetting::remove()
{
	 int ret = 0;
#if defined(hi3560e)
#else
     ret = yhw_env_removefile(NULL);//delete all bakfile in kernel
#endif
	 if(ret != 0){
		 LogSafeOperError("remove bak file error!\n");
	 }
	 return 0;
}

int
BakSetting::recoverBakSeting()
{
    std::vector<SettingItem*>::iterator it;
    SettingItem* item;
    char valueRead[1024] = {0};

    for (it = m_itemsArray.begin(); it != m_itemsArray.end(); ++it) {
        item = *it;
        if (appSettingGetString(item->m_name.c_str(), valueRead, 1024, 0) == 0) {
            set(item->m_name.c_str(), valueRead);
            continue;
        }
        if (sysSettingGetString(item->m_name.c_str(), valueRead, 1024, 0) == 0) {
            set(item->m_name.c_str(), valueRead);
        }
    }
    restore(0);
    return 0;
}

BakSetting& bakSetting()
{
    return g_bakSetting;
}

} // namespace Hippo

int bakSettingGetString(const char* name, char* value, int valueLen, int searchFlag)
{
    LogSafeOperDebug("BakSettingGetString:%s\n", name);
    return Hippo::bakSetting().get(name, value, valueLen);
}


int bakSettingSetString(const char* name, const char* value)
{
    int ret = 0;
    LogSafeOperDebug("BakSettingSetString:%s\n", name);
    ret = Hippo::bakSetting().set(name, value);
    return ret;
}

static int bakSettingListenFunc(const char* name, const char* value)
{
    printf("bakSettingListenFunc*******************\n");
    int ret = 0;

    ret = bakSettingSetString(name, value);
    if (!ret)
        Hippo::bakSetting().backup();
    return 0;
}

//test func
void bakListenRegist()
{
    //printf("bakListenRegist*******************\n");
    SettingListenerRegist("ntvuser", 0, bakSettingListenFunc);
    SettingListenerRegist("ntvAESpasswd", 0, bakSettingListenFunc);
}

