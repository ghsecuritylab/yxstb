
#include "Assertions.h"
#include "Setting.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "VersionSetting.h"
#include "FrequenceSetting.h"
#include "SettingDigest.h"
#include "SettingModule.h"
#include "SettingModuleApp.h"
#include "SettingModuleNetwork.h"
#include "SettingModulePlayer.h"
#include "SettingModuleTr069.h"
#include "SettingModuleUpgrade.h"

#include "sys_basic_macro.h"
#include "cryptoFunc.h"
#include "charConvert.h"
#include "openssl/aes.h"
#include "ind_mem.h"
#include "tools.h"

#if defined(CACHE_CONFIG)
#include "CacheSetting.h"
#endif
#if defined(ENABLE_BAK_SETTING)
#include "BakSetting.h"
#endif
#ifdef INCLUDE_HMWMGMT
#include "Tr069Setting.h"
#endif
#include "Tr069.h"

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

static int aesEncrypt(const char* in, char* out);
static int aesDecrypt(const char* in, char* out);
static char LOCALKEY[]=  "Hybroad Vision..";

namespace Hippo {

Setting::Setting(std::string fileName)
    : m_fileName(fileName)
{
    m_mutex = mid_mutex_create();
}

Setting::~Setting()
{
}

#if 0
#include <fstream>
int
Setting::load()
{
    std::fstream in;
    std::string tag(""), value(""), line("");
    std::string::size_type position;
    char valueRead[1024] = {0};

    in.open(m_fileName.c_str(), std::ios::in);
    if(!in)
        return -1;

    //line eg. ip = 110.1.1.252
    while (getline(in, line)) {
        position = line.find('=');
        if (position == std::string::npos) {
            LogSafeOperDebug("undefined content[%s]\n", line.c_str());
            continue;
        }
        tag = line.substr(0, position);
        value = line.substr(position+1);
        //LogSafeOperDebug("##add [%s:%s] to map\n", tag.c_str(), value.c_str());

        if (!get(tag.c_str(), valueRead, 1024)) {
            LogSafeOperDebug("##set [%s:%s] to map\n", tag.c_str(), value.c_str());
            set(tag.c_str(), value.c_str());
        } else {
            LogSafeOperDebug("##[%d]add [%s:%s] to map\n", __LINE__, tag.c_str(), value.c_str());
            add(tag.c_str(), value.c_str());
        }
    }

    in.close();
    return -1;
}
#endif

int
Setting::load()
{
    FILE *fp = NULL;
    std::string tag(""), value(""), line("");
    std::string::size_type position;
    char ch;
    char valueRead[1024] = {0}; //the largest length ?

    checkFileDigest(m_fileName.c_str());
    if (( fp = fopen(m_fileName.c_str(), "r"))== NULL)
        return -1;

    //line eg. ip = 110.1.1.252
    while ((ch = fgetc(fp)) != (char)EOF || !line.empty()) { //incase of the last line without 0A end
        if(ch != '\n' && ch != (char)EOF) {
            line += ch;
            continue;
        }

        position = line.find('=');
        if (position == std::string::npos) {
            LogSafeOperDebug("undefined content[%s]\n", line.c_str());
            line.clear();
            continue;
        }
        tag = line.substr(0, position);
        value = line.substr(position+1);

        if (!get(tag.c_str(), valueRead, 1024)) {
            LogSafeOperDebug("##set [%s] to map\n", tag.c_str());
            std::map<std::string, SettingItem*>::iterator it = m_itemsMap.find(tag.c_str());
            if (it != m_itemsMap.end()) {
                if (it->second->m_isEncrypt) {
                    char tmp[512] = {0};
                    memset(valueRead, 0, sizeof(valueRead));
                    int ret = hex2Data((char*)value.c_str(), value.size(), tmp, sizeof(tmp));
                    aesEcbDecrypt(tmp, ret, LOCALKEY, valueRead, sizeof(valueRead));
                    it->second->m_encyptedValue = value.c_str();
                    set(tag.c_str(), valueRead);
                } else
                    set(tag.c_str(), value.c_str());
            }
        } else {
            LogSafeOperDebug("##add [%s] to map\n", tag.c_str());
            add(tag.c_str(), value.c_str());
        }
        line.clear();
    }

    fclose(fp);
    return 0;
}

int
Setting::restore(int flag)
{
    FILE *fp = NULL;
    std::string line(""), fielNameBak("");
    std::vector<SettingItem*>::iterator it;

    if(!dirty && !flag) {
        LogSafeOperDebug("%s no changed! dirty = %d, flag = %d no need to save.\n",
                m_fileName.c_str(), dirty, flag);
        return -1;
    }

    fielNameBak = m_fileName +"_bak";

    if (( fp = fopen(m_fileName.c_str(), "wb"))== NULL)
        return -1;

    mid_mutex_lock(m_mutex);
    for (it = m_itemsArray.begin(); it != m_itemsArray.end(); ++it) {
        SettingItem* item = *it;
        if (item->m_name.find('[') != std::string::npos && item->m_name.rfind(']') != std::string::npos)
            line = "\n" + item->m_name +'\n';
        else {
            if (item->m_isEncrypt)
                line = item->m_name + '=' + item->m_encyptedValue + '\n';
            else
                line = item->m_name + '=' + item->m_value + '\n';
        }

        uint len = fwrite(line.c_str(), 1, line.length(), fp);
        if(len != line.length()) {
            LogSafeOperError("write %s failed!\n", fielNameBak.c_str());
            return -1;
        }
        line.clear();
    }
    mid_mutex_unlock(m_mutex);

    fclose(fp);
    dirty = 0;
    updateFileDigest(m_fileName.c_str());
    return 0;
}

int
Setting::empty()
{
    std::vector<SettingItem*>::iterator it;

    mid_mutex_lock(m_mutex);
    for (it = m_itemsArray.begin(); it != m_itemsArray.end(); ++it)
        delete (*it);
    m_itemsArray.clear();
    m_itemsMap.clear();
    mid_mutex_unlock(m_mutex);

    return 0;
}

int
Setting::add(const char* name, const char* value, bool encrypt, SettingListener* listeners)
{
    //if (!name)
    //    return -1;

    SettingItem* item = new SettingItem();
    if (!item)
        return -1;
    item->m_name = name;
    item->m_isEncrypt = encrypt;
    item->m_listeners = listeners;
    if (encrypt) {
        char tmp[512] = {0};
        char plain[512] = {0};
        int ret = hex2Data((char*)value, strlen(value), tmp, sizeof(tmp));
        aesEcbDecrypt(tmp, ret, LOCALKEY, plain, sizeof(plain));
        item->m_value = plain;
        item->m_encyptedValue = value;
    } else
        item->m_value = value;

    mid_mutex_lock(m_mutex);
    m_itemsArray.push_back(item);
    m_itemsMap.insert(std::make_pair(item->m_name, item));
    mid_mutex_unlock(m_mutex);
    return 0;
}

int
Setting::add(const char* name, const int value, bool encrypt, SettingListener* listeners)
{
    char buffer[32];
    sprintf(buffer, "%d", value);

    add(name, buffer, encrypt, listeners);
    return 0;
}

int
Setting::del(const char* name)
{
    //if (!name)
    //    return -1;

    std::vector<SettingItem*>::iterator it;

    mid_mutex_lock(m_mutex);
    for (it = m_itemsArray.begin(); it != m_itemsArray.end(); ++it) {
        SettingItem* item = *it;
        if (name == item->m_name) {
            m_itemsMap.erase(item->m_name);
            m_itemsArray.erase(it);
            delete item;
            break;
        }
    }
    mid_mutex_unlock(m_mutex);
    return 0;
}

int
Setting::set(const char* name, const char* value)
{
    mid_mutex_lock(m_mutex);
    std::map<std::string, SettingItem*>::iterator it = m_itemsMap.find(name);
    if (it != m_itemsMap.end()) {
        if (it->second->m_isEncrypt) {
            char encrypted[1024] = {0};
            int ret = aesEcbEncrypt((char*)value, strlen(value), LOCALKEY, encrypted, sizeof(encrypted));
            data2Hex(encrypted, ret, encrypted, sizeof(encrypted));
            it->second->m_encyptedValue = encrypted;
        }

        std::string *stringValue = &(it->second->m_value);
        if ((*stringValue).compare(value)) {
            *stringValue = value;
            dirty = 1;

            //值改变时去执行监听者函数
            if (it->second->m_listeners)
                it->second->m_listeners->call(name, value);
        }
        mid_mutex_unlock(m_mutex);
        return 0;
    }
    mid_mutex_unlock(m_mutex);
    return -1;
}

int
Setting::get(const char* name, char* value, int valueLen)
{
    mid_mutex_lock(m_mutex);
    std::map<std::string, SettingItem*>::iterator it = m_itemsMap.find(name);
    if (it != m_itemsMap.end()) {
        std::string *stringValue = &(it->second->m_value);
        int stringLen = stringValue->size();
        if (stringLen < valueLen) {
            strcpy(value, stringValue->c_str());
            mid_mutex_unlock(m_mutex);
            return 0;
        }
    }
    mid_mutex_unlock(m_mutex);
    return -1;
}

int
Setting::set(const char* name, const int value)
{
    mid_mutex_lock(m_mutex);
    std::map<std::string, SettingItem*>::iterator it = m_itemsMap.find(name);
    if (it != m_itemsMap.end()) {
        char buffer[32];
        sprintf(buffer, "%d", value);
        if (it->second->m_isEncrypt) {
            char encrypted[128] = { 0 };
            int ret = aesEcbEncrypt(buffer, strlen(buffer), LOCALKEY, encrypted, sizeof(encrypted));
            ret = data2Hex(encrypted, ret, encrypted, sizeof(encrypted));
            it->second->m_encyptedValue = encrypted;
        }

        std::string *stringValue = &(it->second->m_value);
        if ((*stringValue).compare(buffer)) {
            *stringValue = buffer;
            dirty = 1;

            //值改变时去执行监听者函数
            if (it->second->m_listeners)
                it->second->m_listeners->call(name, (const char*)buffer);
        }
        mid_mutex_unlock(m_mutex);
        return 0;
    }
    mid_mutex_unlock(m_mutex);
    return -1;
}

int
Setting::get(const char* name, int* value)
{

    mid_mutex_lock(m_mutex);
    std::map<std::string, SettingItem*>::iterator it = m_itemsMap.find(name);
    if (it != m_itemsMap.end()) {
        std::string *stringValue = &(it->second->m_value);
        *value = atoi(stringValue->c_str());
        mid_mutex_unlock(m_mutex);
        return 0;
    }
    mid_mutex_unlock(m_mutex);
    return -1;
}

int
Setting::registListeners(const char* name, ListenerFunction func)
{
    std::map<std::string, SettingItem*>::iterator it = m_itemsMap.find(name);
    if (it != m_itemsMap.end()) {
        if (it->second->m_listeners)
            it->second->m_listeners->add(func);
        else {
            it->second->m_listeners = new SettingListener();
            it->second->m_listeners->add(func);
       }
    } else {
        LogUserOperDebug("The %s is not settingItem\n",name);
    }
    return 0;
}

int
Setting::unregistListeners(const char* name, ListenerFunction func)
{
    std::map<std::string, SettingItem*>::iterator it = m_itemsMap.find(name);
    if (it != m_itemsMap.end()) {
        if (it->second->m_listeners) {
            it->second->m_listeners->del(func);
            if (it->second->m_listeners->empty())
                it->second->m_listeners = 0;
        } else {
            LogUserOperDebug("The setting %s no listen functions\n", name);
        }
    } else
       LogUserOperDebug("The setting %s is not exist\n", name);
    return 0;
}

} // namespace Hippo

extern "C" {

void settingManagerInit(void)
{
    //just for link
    settingApp();

    settingNetwork();
    settingPlayer();

    settingUpgrade();

#ifdef INCLUDE_TR069
    settingTr069();
#endif

    settingModuleResgister();
    versionSettingInit();
    frequenceSettingInit();
}

void settingManagerLoad(int reset)
{
    if (reset) {
        Hippo::appSetting().empty();
        Hippo::sysSetting().empty();

#if defined(CACHE_CONFIG)
        Hippo::cacheSetting().empty();
#endif

#if defined(ENABLE_BAK_SETTING)
        Hippo::bakSetting().empty();
#endif
        settingModuleResgister();

        Hippo::appSetting().restore(1);
        Hippo::sysSetting().restore(1);

#if defined(CACHE_CONFIG)
        Hippo::cacheSetting().restore(1);
#endif

#if defined(ENABLE_BAK_SETTING)
        Hippo::bakSetting().recoverBakSeting();
        Hippo::bakSetting().restore(1);
#endif

#ifdef INCLUDE_HMWMGMT
        Hippo::tr069Setting().empty();
        Hippo::tr069Setting().restore(1);
#endif
    } else {
        int bakLoadFlag = -1;
        int appLoadFlag = -1;
        int sysLoadFlag = -1;

#if defined(ENABLE_BAK_SETTING)
        bakLoadFlag = Hippo::bakSetting().load();
        if (!bakLoadFlag)
            Hippo::bakSetting().restore(0);
#endif

        appLoadFlag = Hippo::appSetting().load();
        if (!appLoadFlag)
            Hippo::appSetting().restore(0);
        else
            Hippo::appSetting().restore(1);

        sysLoadFlag = Hippo::sysSetting().load();
        if (!sysLoadFlag)
            Hippo::sysSetting().restore(0);
        else
            Hippo::sysSetting().restore(1);

#if defined(ENABLE_BAK_SETTING)
        if (-1 == bakLoadFlag) {
            Hippo::bakSetting().recoverBakSeting();
            Hippo::bakSetting().backup();
        }
        if ((0 == bakLoadFlag) && (-1 == appLoadFlag))
            Hippo::appSetting().recoverBakSeting();
        if ((0 == bakLoadFlag) && (-1 == sysLoadFlag))
            Hippo::sysSetting().recoverBakSeting();
        bakListenRegist();
#endif

#if defined(CACHE_CONFIG)
        cacheSettingLoad();
#endif

#ifdef INCLUDE_HMWMGMT
        if (-1 == Hippo::tr069Setting().load())
            Hippo::tr069Setting().restore(1);
        else
            Hippo::tr069Setting().restore(0);
#endif
    }
}

void settingManagerSave()
{
    Hippo::appSetting().restore(0);
    Hippo::sysSetting().restore(0);

#ifdef INCLUDE_HMWMGMT
    Hippo::tr069Setting().restore(0);
#endif
}

}

