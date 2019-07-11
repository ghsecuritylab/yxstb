
#include "tr069_api.h"
#include "tr069_port.h"
#include "tr069_port1.h"
#include "TR069Assertions.h"

#include "SysSetting.h"
#include "Assertions.h"
#include "tr069Itms.h"
#include "StatisticRoot.h"
#include "Tr069BandwidthDiagnostics.h"
#include "Tr069LogMsg.h"
#include "Tr069X_CTC_IPTV_Monitor.h"
#include "Tr069Root.h"
#include "sys_basic_macro.h"
#include "SysSetting.h"
#include "AppSetting.h"
#include "tvms_setting.h"
#include "MessageTypes.h"
#include "config.h"
#include "NativeHandler.h"
#include "app_epg_para.h"
#include <string.h>

static int g_tms_flag = 0;

/*------------------------------------------------------------------------------
	恢复工厂默认设置
 ------------------------------------------------------------------------------*/
extern "C" void tr069_port_reset(void)
{
    int nettype = 0, qosenable = 0, tr069enable = 0;
    char ip[16] = {0}, tMask[16] = {0}, tGateway[16] = {0}, tDns[16] = {0}, tDnsBak[16] = {0};
    char dhcpuser[34] = {0}, dhcppwd[34] = {0};
    char ntvuser[34] = {0}, ntvpwd[128] = {0};
    char tPppuser[USER_LEN] = {0}, tPppPwd[USER_LEN] = {0};

    sysSettingGetInt("connecttype", &nettype, 0);
    if(nettype == 3) {
        sysSettingGetString("ip", ip, 16, 0);
        sysSettingGetString("netmask", tMask, 16, 0);
        sysSettingGetString("gateway", tGateway, 16, 0);
        sysSettingGetString("dns", tDns, NET_LEN, 0);
        sysSettingGetString("dns1", tDnsBak, NET_LEN, 0);
    }

    sysSettingGetString("netuser", tPppuser, USER_LEN, 0);
    sysSettingGetString("netAESpasswd", tPppPwd, USER_LEN, 0);

    sysSettingGetString("dhcpuser", dhcpuser, 34, 0);
    sysSettingGetString("ipoeAESpasswd", dhcppwd, sizeof(dhcppwd), 0);

    appSettingGetString("ntvuser", ntvuser, 34, 0);
    appSettingGetString("ntvAESpasswd", ntvpwd, sizeof(ntvpwd), 0);
#ifndef Sichuan
	sysSettingGetInt("logSend", &qosenable, 0);
	sysSettingGetInt("tr069_enable", &tr069enable, 0);
#endif
    int upgradeOK = 0;
    sysSettingGetInt("IPUpgradeOK", &upgradeOK, 0);
    settingManagerLoad(1);
    if(!upgradeOK){
        sysSettingSetInt("isFirstUpgradeOK", -1);
        sysSettingSetInt("IPUpgradeOK", 0);
    }
    settingManagerSave();
#ifdef TVMS_OPEN
    tvms_config_load(1);
#endif
    tr069StatisticConfigReset( );
    sysSettingSetInt("connecttype", nettype);
    if(nettype == 3) {
        sysSettingSetString("ip", ip);
        sysSettingSetString("netmask", tMask);
        sysSettingSetString("gateway", tGateway);
        sysSettingSetString("dns", tDns);
        sysSettingSetString("dns1", tDnsBak);
    }

    sysSettingSetString("netuser", tPppuser);
    sysSettingSetString("netAESpasswd", tPppPwd);

    sysSettingSetString("dhcpuser", dhcpuser);
    sysSettingGetString("ipoeAESpasswd", dhcppwd, sizeof(dhcppwd), 0);

    appSettingSetString("ntvuser", ntvuser);
    appSettingSetString("ntvAESpasswd", ntvpwd);
#ifndef Sichuan
     sysSettingSetInt("logSend", qosenable);
     sysSettingSetInt("tr069_enable", tr069enable);
#endif
    return;
}

extern "C" void tr069_upgradeUrl_init(int type)
{
    if (1 == type)
        sysSettingSetString("tr069UpgradeUrl", "");
    else
        return;
}

extern "C" int tr069_get_upgradeUrl(char *url, int len)
{
    char upgradeUrl[512] = {0};
    int length = 0;
    sysSettingGetString("tr069UpgradeUrl", upgradeUrl, sizeof(upgradeUrl), 0);
    length = strlen(upgradeUrl);
    if (!url) {
        LogUserOperError("url is null\n");
        return -1;
    }

    if (len <= length) {
        LogUserOperError("len is %d, tr069upgradeurl len is %d\n", len, length);
        return -1;
    }

    strcpy(url, upgradeUrl);
    return 0;
}

extern "C" int tr069_set_UpgradeUrl(char *url)
{
    if (!url) {
        LogUserOperError("url is null\n");
        return -1;
    }

    if (strlen(url) >= 512 || !strlen(url)) {
        LogUserOperError("tr069upgradeurl length = %d\n", strlen(url));
        return -1;
    }

    sysSettingSetString("tr069UpgradeUrl", url);
    return 0;
}

#ifdef TR069_ZERO_CONFIG
static int g_tr069BootStrap = -1; // 0 means first start, after this it is will be 1.
void tr069_set_bootStrap(int bootStrap)
{
    g_tr069BootStrap = bootStrap;
    return ;
}
int tr069_get_bootStrap(void)
{
    return g_tr069BootStrap;
}
#endif //TR069_ZERO_CONFIG


/*************************************************
Description: 本文件内部使用函数发送TR069状态
Input:  无
Return: 无
 *************************************************/
static void int_port_SetMessage(char *name, char *str, unsigned int val)
{
    if (!strcmp(name, "Reset")) {//恢复工厂默认设置
        tr069_port_reset( );
    } else if (!strcmp(name, "ReBoot")) {//TR069_POST_STB_REBOOT
        sendMessageToNativeHandler(MessageType_Tr069, TR069_REQUEST_REBOOT, 0, 0);
    } else if (!strcmp(name, "UpgradeIgnore")) {//TR069_POST_UPGRADE_IGNORE tr069链接成功
        sendMessageToNativeHandler(MessageType_Tr069, TR069_NET_CONNECT_OK, 0, 0);
    } else if (!strcmp(name, "ErrorConnect")) {//TR069_POST_ERROR_CONNECT
        sendMessageToNativeHandler(MessageType_Tr069, TR069_NET_CONNECT_ERROR, 0, 0);
    } else if (!strcmp(name, "UpgradeRequest")) {//TTR069_POST_UPGRADE_BOOT
        sendMessageToNativeHandler(MessageType_Tr069, TR069_UPGRADE_BOOT_REQUEST, 0, 0);
    } else if (!strcmp(name, "Download.1 Firmware Upgrade Image")) {//TR069_POST_UPGRADE_REQUEST str 为升级的URL
#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD)
        if (url && strlen(url))
            sysSettingSetString("upgradeUrl", str);
        g_tr069upgrade_flag = 1;
#endif
#ifndef Sichuan
        tr069_set_UpgradeUrl(str);
#endif
        sendMessageToNativeHandler(MessageType_Tr069, TR069_UPGRADE_REQUEST, 0, 0);

    } else if (!strcmp(name, "UpgradeFinished")) {//TR069_POST_UPGRADE_FINISHED
#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD)
        g_tr069upgrade_flag = 0;
#endif
#if defined(HUAWEI_C20) || (_HW_BASE_VER_ >= 58)
    } else if (!strcmp(name, "AuthOk")) {
        if (0 == g_tms_flag){
            int ret =0;
            ret = app_TMS_aes_keys_set ();
            if (0 != ret)
                LogTr069Debug("The tms keys is error,please checking\n");
            else
                g_tms_flag  = 1;
        }
#endif
    } else {
        ERR_PRN("Message. name = %s\n", name);
    }
}


/*************************************************
Description: Tr069模块参数设置与获取函数
Input:  无
Return: 无
 *************************************************/
extern "C" int tr069_port_getValue(char *name, char *str, unsigned int val)
{
     LogTr069Debug("tr069_port_getValue name = %s, str = %s, val = %d\n",name, str, val);
	 return Tr069RootRead(name, str, val);
}

extern "C" int tr069_port_setValue(char *name, char *str, unsigned int val)
{
    if (!strncmp(name, "Message.", 8)) {
        name += 8;
        int_port_SetMessage(name, str, val);
        return 0;
    }
    LogTr069Debug("tr069_port_setValue name = %s, str = %s\n",name, str);
	return Tr069RootWrite(name, str, (unsigned int)&val);

}

/*************************************************
Description: Tr069模块初始化
Input:  无
Return: 无
 *************************************************/
extern "C" void Tr069ApiInit()
{
#if !defined(ANDROID)
    tr069_api_setValue((char*)"Config.ParamPath", (char*)"/home/hybroad/share/tr069_param.xml", 0);
    tr069_api_setValue((char*)"Config.ConfigPath", (char*)"/root/yx_config_tr069.cfg", 0);
#endif
    tr069_api_init( );
    tr069_upgradeUrl_init(1);
    tr069StatisticConfigInit(); // 日志上传结构体初始化
#if defined(HUAWEI_C10)
    tr069LogMsgInit();
#endif

    tr069_port_setValue((char*)"Device.DeviceInfo.DeviceStatus", (char*)"Up", 0);
#ifdef INCLUDE_TR069_CTC
    tr069_api_setValue((char*)"Config.HTTPTimeout", (char*)"", 30);
#endif // NCLUDE_TR069_CTC
#ifdef NEIMENGGU_HD
    tr069_api_setValue((char*)"Config.Bootstrap", (char*)"", 0);
    tr069_api_setValue((char*)"Config.HoldCookie", (char*)"", 1);
#endif
    tr069_api_setValue((char*)"Config.ParamPedant", (char*)"", 0);
    tr069_port_bandwidthDiagnostics_init( );

#ifdef Jiangsu
    if (!strcmp(STBTYPE, (char*)"EC1308H")) {
        tr069_set_bootStrap(tr069_api_get_bootStrap());//括号内的函数被删除了，如果有用，这里需要修改。
    } else {
        tr069_api_tracert_extend();
    }
#endif

#ifdef TR069_MONITOR
    app_monitor_statistic_config_init();
#endif

    {//部分告警配置参数由TR069模块同步过来
        char buf[16];

        tr069_api_getValue((char*)"Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmSwitch", buf, 16);
        tr069_port_setValue((char*)"Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmSwitch", buf, 0);
        tr069_api_getValue((char*)"Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmConfig.PacketsLostAlarmValue", buf, 16);
        tr069_port_setValue((char*)"Device.DeviceInfo.X_CTC_IPTV_Alarm.AlarmConfig.PacketsLostAlarmValue", buf, 0);

        tr069_api_getValue((char*)"Device.X_00E0FC.SQMConfiguration.SQMLisenPort", buf, 16);
        tr069_port_setValue((char*)"Device.X_00E0FC.SQMConfiguration.SQMLisenPort", buf, 0);
        tr069_api_getValue((char*)"Device.X_00E0FC.SQMConfiguration.SQMServerPort", buf, 16);
        tr069_port_setValue((char*)"Device.X_00E0FC.SQMConfiguration.SQMServerPort", buf, 0);
    }
}
