
#include "JseVerimatrix.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"
#include "JseRoot.h"

#ifdef ANDROID
#include "IPTVMiddleware.h"
#endif

#include "SysSetting.h"
#include "ntp/mid_ntp.h"
#include "sys_basic_macro.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int JseNetworkTypeRead(const char *param, char *value, int len)
{
    int netType = 0;

    sysSettingGetInt("nettype", &netType, 0);
    snprintf(value, len, "%d", netType);
    return 0;
}

static int JseNetworkTypeWrite(const char *param, char *value, int len)
{
    sysSettingSetInt("nettype", atoi(value));
    return 0;
}

static int JseNetworkConnectTypeRead(const char *param, char *value, int len)
{
    int connectType = 0;

    sysSettingGetInt("connecttype", &connectType, 0);
    snprintf(value, len, "%d", connectType);
    return 0;
}

static int JseNetworkConnectTypeWrite(const char *param, char *value, int len)
{
    sysSettingSetInt("connecttype", atoi(value));
    return 0;
}

static int JseSTBIPRead(const char *param, char *value, int len)
{
    sysSettingGetString("ip", value, len, 0);
    return 0;
}

static int JseSTBIPReadWrite(const char *param, char *value, int len)
{
    sysSettingSetString("ip", value);
    return 0;
}

static int JseNetmaskRead(const char *param, char *value, int len)
{
    sysSettingGetString("netmask", value, len, 0);
    return 0;
}

static int JseNetmaskWrite(const char *param, char *value, int len)
{
    sysSettingSetString("netmask", value);
    return 0;
}

static int JseGatewayRead(const char *param, char *value, int len)
{
    sysSettingGetString("gateway", value, len, 0);
    return 0;
}

static int JseGatewayWrite(const char *param, char *value, int len)
{
    sysSettingSetString("gateway", value);
    return 0;
}

static int JseDNSRead(const char *param, char *value, int len)
{
    sysSettingGetString("dns", value, len, 0);
    return 0;
}

static int JseDNSWrite(const char *param, char *value, int len)
{
    sysSettingSetString("dns", value);
    return 0;
}

static int JseDNS2Read(const char *param, char *value, int len)
{
    sysSettingGetString("dns1", value, len, 0);
    return 0;
}

static int JseDNS2Write(const char *param, char *value, int len)
{
    sysSettingSetString("dns1", value);
    return 0;
}

static int JseNTPDomainRead(const char *param, char *value, int len)
{
    sysSettingGetString("ntp", value, len, 0);
    return 0;
}

static int JseNTPDomainWrite(const char *param, char *value, int len)
{
    if ((!value) || (strlen(value) == 0) || (!strcmp(value, "null")) || (!strcmp(value, "NULL")))
        return 0;
    char inflash_ntp[128] = {0};
    sysSettingGetString("ntp", inflash_ntp, 128, 0);
	if (strcmp(inflash_ntp , value)){
	    sysSettingSetString("ntp", value);
		mid_ntp_time_sync();
	}
#if defined(ANDROID)
#ifndef NEW_ANDROID_SETTING
	IPTVMiddleware_SettingSetStr("ntp", value);
#endif
#endif
    return 0;
}

static int JseNTPDomainBackupRead(const char *param, char *value, int len)
{
    sysSettingGetString("ntp1", value, len, 0);
    return 0;
}

static int JseNTPDomainBackupWrite(const char *param, char *value, int len)
{
    if ((!value) || (strlen(value) == 0) || (!strcmp(value, "null")) || (!strcmp(value, "NULL"))){
        return 0;
    } else {
        char inflash_ntp[128] = {0};
        sysSettingGetString("ntp1", inflash_ntp, 128, 0);
	    if (strcmp(inflash_ntp , value)) {
	        sysSettingSetString("ntp1",value);
		    mid_ntp_time_sync();
	    }
	}
#if defined(ANDROID)
#ifndef NEW_ANDROID_SETTING
	IPTVMiddleware_SettingSetStr("ntp1", value);
#endif
#endif
    return 0;
}

static int JseEDSRead(const char *param, char *value, int len)
{
    sysSettingGetString("eds", value, len, 0);
    return 0;
}

static int JseEDSWrite(const char *param, char *value, int len)
{
   if (!value || (strncmp(value, "http://", 7) != 0)) {
        LogJseError("JseEDSWrite value is error\n");
        return -1;
    }
    sysSettingSetString("eds", value);
    return 0;
}

static int JseEDS1Read(const char *param, char *value, int len)
{
    sysSettingGetString("eds1", value, len, 0);
    return 0;
}

static int JseEDS1Write(const char *param, char *value, int len)
{
    if (!value || (strncmp(value, "http://", 7) != 0)) {
        LogJseError("JseEDS1Write value is error\n");
        return -1;
    }
    sysSettingSetString("eds1", value);
    return 0;
}

/*************************************************
Description: 初始化并注册Verimatrix定义的接口 <Verimatrix.***>
Input: 无
Return: 无
 *************************************************/
int JseVerimatrixInit()
{
    JseGroupCall* father = getNodeByName("Verimatrix.Company"); // 使用华为接口的Verimatrix.***已注册的任一接口找到父节点
    LogJseDebug("\n---name[%s]------\n", father->name());

    JseCall* call;

    //以下全为C20注册
    call = new JseFunctionCall("network_type", JseNetworkTypeRead, JseNetworkTypeWrite);
    father->regist(call->name(), call);

    call = new JseFunctionCall("connect_type", JseNetworkConnectTypeRead, JseNetworkConnectTypeWrite);
    father->regist(call->name(), call);

    call = new JseFunctionCall("ip", JseSTBIPRead, JseSTBIPReadWrite);
    father->regist(call->name(), call);

    call = new JseFunctionCall("mask", JseNetmaskRead, JseNetmaskWrite);
    father->regist(call->name(), call);

    call = new JseFunctionCall("gateway", JseGatewayRead, JseGatewayWrite);
    father->regist(call->name(), call);

    call = new JseFunctionCall("dns", JseDNSRead, JseDNSWrite);
    father->regist(call->name(), call);
    call = new JseFunctionCall("dns1", JseDNS2Read, JseDNS2Write);
    father->regist(call->name(), call);

    call = new JseFunctionCall("ntp", JseNTPDomainRead, JseNTPDomainWrite);
    father->regist(call->name(), call);
    call = new JseFunctionCall("ntp1", JseNTPDomainBackupRead, JseNTPDomainBackupWrite);
    father->regist(call->name(), call);

    call = new JseFunctionCall("eds", JseEDSRead, JseEDSWrite);
    father->regist(call->name(), call);
    call = new JseFunctionCall("eds1", JseEDS1Read, JseEDS1Write);
    father->regist(call->name(), call);
    return 0;
}

