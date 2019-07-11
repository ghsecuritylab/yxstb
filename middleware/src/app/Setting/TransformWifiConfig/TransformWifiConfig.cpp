
#include "CConfig.h"
#include "Assertions.h"
#include "config/pathConfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SysSetting.h"

#include "sys_basic_macro.h"
#include "app_epg_para.h"

#define WIFICONFIG CONFIG_FILE_DIR"/copywirelessconf.ini"

namespace Hippo {
static bool enum_proc(const char* section, const char* name, const char* value, void* param)
{
    // printf("[%s] %s=>%s\n", section, name, value);
    return true;
}


typedef struct tagNewWifiConfig {
    std::string     ssid;
    std::string     AuthMode;
    std::string     EncryptionType;
    std::string     password;
}NewWifiConfig;

static void Old2New(NewWifiConfig& wifi)
{
    CConfig   cfg;
    cfg.Load(WIFICONFIG);
    cfg.Enum(enum_proc, NULL);

    std::string ssid = cfg.Read("default", "ESSID0");
    if (ssid.empty()) {
        return;
    }
    wifi.ssid = ssid;
    wifi.AuthMode = cfg.Read(ssid, "AuthMode");
    wifi.EncryptionType = cfg.Read(ssid, "EncryptionType");

    std::string index = cfg.Read(ssid, "Enc_PassWordKeyIndex");
    std::string PassWordKeyIndex_1 = cfg.Read(ssid, std::string("PassWordKeyIndex_") + index);
    std::string Enc_PassWordKey = cfg.Read(ssid, "Enc_PassWordKey");

    wifi.password = PassWordKeyIndex_1;
    if (wifi.password.empty())
        wifi.password = Enc_PassWordKey;
}

static void New2Old(const char* ssid, const char* password, const char* authmode, const char* encryptiontype)
{
    if (!ssid || !password || !authmode || !encryptiontype)
        return;

    CConfig   cfg;
    cfg.Load(WIFICONFIG);
    std::string numbers = cfg.Read("default", "SSID_NUMBERS");
    if (numbers.empty()) {
        cfg.Write("default", "SSID_NUMBERS", "1");
    }

    cfg.Write("default", "ESSID0", ssid);
    cfg.Remove(ssid, "Enc_PassWordKeyIndex");
    cfg.Remove(ssid, "Enc_PassWordKeyIndex_1");
    cfg.Remove(ssid, "Enc_PassWordKeyIndex_2");
    cfg.Remove(ssid, "Enc_PassWordKeyIndex_3");
    cfg.Remove(ssid, "Enc_PassWordKeyIndex_4");
    cfg.Remove(ssid, "EncryDataType");
    cfg.Remove(ssid, "Enc_PassWordKey");
    cfg.Write(ssid, "AuthMode", authmode);
    cfg.Write(ssid, "EncryptionType", encryptiontype);
    std::string qua = cfg.Read(ssid, "QualityLevel");
    if (qua.empty())
        cfg.Write(ssid, "QualityLevel", "100");

    if (strstr(encryptiontype, "WEP")) {
        cfg.Write(ssid, "Enc_PassWordKeyIndex", "1");
        int len = strlen(password);
        switch (len) {
        case 5:
            cfg.Write(ssid, "EncryDataType", "64A");
            break;
        case 10:
            cfg.Write(ssid, "EncryDataType", "64H");
            break;
        case 13:
            cfg.Write(ssid, "EncryDataType", "128A");
            break;
        case 26:
            cfg.Write(ssid, "EncryDataType", "128H");
            break;
        default:
            cfg.Write(ssid, "EncryDataType", "64A");
            break;
        }
        cfg.Write(ssid, "PassWordKeyIndex_1", password);
    } else {
        cfg.Write(ssid, "Enc_PassWordKey", password);
    }
    cfg.Save(WIFICONFIG);
}
} // end of namespace Hippo

extern "C"  {
int app_transWifiConfig_toNew()
{
    Hippo::NewWifiConfig   wifi;
    // 从老配置文件过渡过来：
    Hippo::Old2New(wifi);

    LogUserOperDebug("[%s], [%s], [%s], [%s]\n", wifi.ssid.c_str(), wifi.password.c_str(), wifi.AuthMode.c_str(), wifi.EncryptionType.c_str());

    if (!wifi.AuthMode.empty()) {
        sysSettingSetString("wifi_ssid", (char*)wifi.ssid.c_str());
        sysSettingSetString("wifi_password", wifi.password.c_str());
        sysSettingSetString("wifi_authType", (char*)wifi.AuthMode.c_str());
        sysSettingSetString("wifi_encryType", (char*)wifi.EncryptionType.c_str());
        Hippo::New2Old(wifi.ssid.c_str(), "", "", "");
    }

    return 0;
}

int app_transWifiConfig_toOld()
{
    char ssid[USER_LEN] = { 0 };
    char passwd[USER_LEN] = { 0 };
    char encry[USER_LEN] = { 0 };
    char auth[USER_LEN] = { 0 };

    sysSettingGetString("wifi_ssid", ssid, USER_LEN, 0);
    //sys_wifi_config_get("password", passwd, USER_LEN);
    sysSettingGetString("wifi_password", passwd, ENCRYPT_PASSWORD_LEN, 0);
    sysSettingGetString("wifi_encryType", encry, USER_LEN, 0);
    sysSettingGetString("wifi_authType", auth, USER_LEN, 0);

    LogUserOperDebug("[%s], [%s], [%s], [%s]\n", ssid, passwd, auth, encry);

    // 存为老的配置文件。
    Hippo::New2Old(ssid, passwd, auth, encry);
    return 0;
}
} // End of extern "C"









