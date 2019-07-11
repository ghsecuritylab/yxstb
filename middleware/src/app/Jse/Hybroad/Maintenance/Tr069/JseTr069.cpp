
#include "JseTr069.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "Tr069.h"

#if defined(INCLUDE_HMWMGMT)
#include "Tr069Setting.h"

#endif//if defined(INCLUDE_HMWMGMT)

#include "sys_basic_macro.h"
#include "NetworkFunctions.h"

#include "jse.h"

#include "SysSetting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(INCLUDE_TR069)

static int JseYxParaTr069PcapStateRead( const char* param, char* value, int len )
{
    sprintf(value, "%d", getTr069OpenPacketCaptureFlag());
    return 0;
}

static int JseYxParaTr069PcapStateWrite( const char* param, char* value, int len )
{
    setTr069OpenPacketCaptureFlag(atoi(value));

    return 0;
}

#ifdef TR069_ZERO_CONFIG
static int JseZeroConfigSerialNumWrite( const char* param, char* value, int len )
{

    TR069_API_SETVALUE("Device.X_CTC_IPTV.UserAccount.AccountNumber", value, 0);

    TR069_API_SETVALUE("Event.Parameter", "Device.X_CTC_IPTV.UserAccount.AccountNumber", TR069_EXTERN_EVENT_ACCOUNT_REPORT);
    TR069_API_SETVALUE("Event.Post", "", TR069_EXTERN_EVENT_ACCOUNT_REPORT);
    return 0;
}

static int JseNeedZeroConfigRead( const char* param, char* value, int len )
{
    if(0 == tr069_get_bootStrap()) // when first use STB, tr069 use the value to judge whether need zero config.
        snprintf(value, len, "%d", 1);
    else
        snprintf(value, len, "%d", 0);
    return 0;
}

#endif//#ifdef TR069_ZERO_CONFIG

static int JseTR069ControlWrite( const char* param, char* value, int len )
{
    int i1 = 0;
    char s1[URL_MAX_LEN + 4] = {0};

    LogJseDebug("Set(%s)\n", value);
    if(jse_parse_param(value, "Save", ':', "") == 0) {
        TR069_API_SETVALUE("Config.Save", "", 1);
    } else if(jse_parse_param(value, "ACE_URL", ':', "e", s1) == 1) {
#if defined(HUAWEI_C10)
        uint32_t flag = 0;
        TR069_API_GETVALUE("Device.ManagementServer.URLModifyFlag", s1, URL_MAX_LEN);
        flag = atoi(s1);
        if(!(flag & 0x04)) {
            LogJseError("not allow to change tms url.\n");
            return -1;
        }
#endif
        TR069_API_SETVALUE("Device.ManagementServer.URL", s1, 0);
    } else if(jse_parse_param(value, "ACE_Username", ':', "s", s1) == 1) {
        TR069_API_SETVALUE("Device.ManagementServer.Username", s1, 0);
    } else if(jse_parse_param(value, "ACE_Password", ':', "s", s1) == 1) {
        TR069_API_SETVALUE("Device.ManagementServer.Password", s1, 0);
    } else if(jse_parse_param(value, "tr069Enable", ':', "d", &i1) == 1) {
		sysSettingSetInt("tr069_enable", i1);
    } else if(jse_parse_param(value, "PeriodicEnable", ':', "s", &s1) == 1) {
        TR069_API_SETVALUE("Device.ManagementServer.PeriodicInformEnable", s1, 0);
    } else if(jse_parse_param(value, "PeriodicInterval", ':', "s", &s1) == 1) {
        TR069_API_SETVALUE("Device.ManagementServer.PeriodicInformInterval", s1, 0);
    } else if(jse_parse_param(value, "CPE_Username", ':', "s", s1) == 1) {
        TR069_API_SETVALUE("Device.ManagementServer.ConnectionRequestUsername", s1, 0);
    } else if(jse_parse_param(value, "CPE_Password", ':', "s", s1) == 1) {
        TR069_API_SETVALUE("Device.ManagementServer.ConnectionRequestPassword", s1, 0);
    } else {
        LogJseError("Can't find this key(%s)\n", value);
        return -1;
    }
    return 0;
}

static int JseTR069Read( const char* param, char* value, int len )
{
    unsigned int i1 = 0;

    if(!strcmp(param, "ACE_URL")) {
        TR069_API_GETVALUE("Device.ManagementServer.URL", value, len);
    } else if(!strcmp(param, "ACE_Username")) {
        TR069_API_GETVALUE("Device.ManagementServer.Username", value, len);
    } else if(!strcmp(param, "ACE_Password")) {
        TR069_API_GETVALUE("Device.ManagementServer.Password", value, len);
    } else if(!strcmp(param, "PeriodicEnable")) {
        TR069_API_GETVALUE("Device.ManagementServer.PeriodicInformEnable", value, len);
    } else if(!strcmp(param, "tr069Enable")) {
        int tr069Enable = 0;
        sysSettingGetInt("tr069_enable", &tr069Enable, 0);
        sprintf(value, "%ud", tr069Enable);
    } else if(!strcmp(param, "tr069Active")) {
        sprintf(value, "%ud", itms_isstarted());
    } else if(!strcmp(param, "PeriodicInterval")) {
        TR069_API_GETVALUE("Device.ManagementServer.PeriodicInformInterval", value, len);
    } else if(!strcmp(param, "CPE_Username")) {
        TR069_API_GETVALUE("Device.ManagementServer.ConnectionRequestUsername", value, len);
    } else if(!strcmp(param, "CPE_Password")) {
        TR069_API_GETVALUE("Device.ManagementServer.ConnectionRequestPassword", value, len);
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)
    } else if(!strcmp(param, "ConnectionRequestURL")) {
        char ifname[URL_LEN] = { 0 };
        char ifaddr[URL_LEN] = { 0 };
        network_default_ifname(ifname, URL_LEN);
        sprintf(value, "http://%s:7547/digest", network_address_get(ifname, ifaddr, URL_LEN));
#endif
    } else {
        LogJseError("Can't find this key(%s)\n", param);
        return -1;
    }

    return 0;
}


#elif defined(INCLUDE_HMWMGMT)
static int JseTR069ControlWrite( const char* param, char* value, int len )
{
    int i1 = 0;
    char s1[URL_MAX_LEN + 4] = {0};

    LogJseDebug("Set(%s)\n", value);
    if(jse_parse_param(value, "Save", ':', "") == 0) {
        tr069SettingSave();
    } else if(jse_parse_param(value, "ACE_URL", ':', "e", s1) == 1) {
#if defined(HUAWEI_C10)&& 0
        uint32_t flag = 0;
        tr069SettingGetInt("Param.URLModify", i1, 0);
        if(!(i1 & 0x04)) {
            LogJseError("not allow to change tms url.\n");
            return -1;
        }
#endif
        tr069SettingSetString("Param.URL", s1);
    } else if(jse_parse_param(value, "ACE_Username", ':', "s", s1) == 1) {
        tr069SettingSetString("Param.Username", s1);
    } else if(jse_parse_param(value, "ACE_Password", ':', "s", s1) == 1) {
        tr069SettingSetString("Param.AESPassword", s1);
    } else if(jse_parse_param(value, "tr069Enable", ':', "d", &i1) == 1) {
		tr069SettingSetInt("tr069_enable", i1);
    } else if(jse_parse_param(value, "PeriodicEnable", ':', "d", &i1) == 1) {
        tr069SettingSetInt("Param.PeriodicInformEnable", i1);
    } else if(jse_parse_param(value, "PeriodicInterval", ':', "d", &i1) == 1) {
        tr069SettingSetInt("Param.PeriodicInformInterval", i1);
    } else if(jse_parse_param(value, "CPE_Username", ':', "s", s1) == 1) {
        tr069SettingSetString("Param.ConnectionRequestUsername", s1);
    } else if(jse_parse_param(value, "CPE_Password", ':', "s", s1) == 1) {
        tr069SettingSetString("Param.AESConnectionRequestPassword", s1);
    } else {
        LogJseError("Can't find this key(%s)\n", value);
        return -1;
    }
    return 0;
}

static int JseTR069Read( const char* param, char* value, int len )
{
    unsigned int i1 = 0;

    if(!strcmp(param, "ACE_URL")) {
        tr069SettingGetString("Param.URL", value, len, 0);
    } else if(!strcmp(param, "ACE_Username")) {
        tr069SettingGetString("Param.Username", value, len, 0);
    } else if(!strcmp(param, "ACE_Password")) {
        tr069SettingGetString("Param.AESPassword", value, len, 0);
    } else if(!strcmp(param, "PeriodicEnable")) {
        tr069SettingGetInt("Param.PeriodicInformEnable", &i1, 0);
        sprintf(value, "%d", i1);
    } else if(!strcmp(param, "tr069Enable")) {
        int tr069Enable = 0;
        sysSettingGetInt("tr069_enable", &tr069Enable, 0);
        sprintf(value, "%ud", tr069Enable);
    } else if(!strcmp(param, "PeriodicInterval")) {
        tr069SettingGetInt("Param.PeriodicInformInterval", &i1, 0);
        sprintf(value, "%d", i1);
    } else if(!strcmp(param, "CPE_Username")) {
        tr069SettingGetString("Param.ConnectionRequestUsername", value, len, 0);
    } else if(!strcmp(param, "CPE_Password")) {
        tr069SettingGetString("Param.AESConnectionRequestPassword", value, len, 0);
#if defined(SHANGHAI_HD)||defined(SHANGHAI_SD)
    } else if(!strcmp(param, "ConnectionRequestURL")) {
        char ifname[URL_LEN] = { 0 };
        char ifaddr[URL_LEN] = { 0 };
        network_default_ifname(ifname, URL_LEN);
        sprintf(value, "http://%s:7547/digest", network_address_get(ifname, ifaddr, URL_LEN));
#endif
    } else {
        LogJseError("Can't find this key(%s)\n", param);
        return -1;
    }
    LogSafeOperDebug("jse read %s = %s\n",param, value);
}

#endif //#if defined(INCLUDE_TR069) or INCLUDE_HMWMGMT


/*************************************************
Description: 初始化华为Tr069的接口，由JseHWMaintenance.cpp调用
Input: 无
Return: 无
*************************************************/
int JseTr069Init()
{

    JseCall* call;
#if defined(INCLUDE_TR069)
    //C10 C20
    call = new JseFunctionCall("yx_para_tr069_pcap_state", JseYxParaTr069PcapStateRead, JseYxParaTr069PcapStateWrite);
    JseRootRegist(call->name(), call);

#ifdef TR069_ZERO_CONFIG
    //C10 C20
    call = new JseFunctionCall("zeroConfigSerialNum", 0, JseZeroConfigSerialNumWrite);
    JseRootRegist(call->name(), call);

    //C10 C20
    call = new JseFunctionCall("needZeroConfig", JseNeedZeroConfigRead, 0);
    JseRootRegist(call->name(), call);

#endif //#ifdef TR069_ZERO_CONFIG
#endif //#if defined(INCLUDE_TR069)

#if defined(INCLUDE_TR069) || defined(INCLUDE_HMWMGMT)
    //C10 C20
    call = new JseFunctionCall("JseTR069Control", 0, JseTR069ControlWrite);
    JseRootRegist(call->name(), call);
    //C10 C20
    call = new JseFunctionCall("JseTR069Read", JseTR069Read, 0);
    JseRootRegist(call->name(), call);

#endif
    return 0;
}
