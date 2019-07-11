
#include "JseStatic.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "NetworkFunctions.h"
#include "sys_basic_macro.h"

#include "SysSetting.h"
#include "app_sys.h"
#include "ind_mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HUAWEI_C10
static int JseStaticArpProbeRead(const char* param, char* value, int len)
{
#ifdef RFC5227
    char ipaddr[20] = {0};
    sysSettingGetString("ip", ipaddr, 20, 0);
    unsigned int ip = inet_addr(ipaddr);
    arp_probe_stop();
    int ret = arp_probe_start((const unsigned short *)&ip, sys_get_net_interface(), NULL);
    if(ret == 0) // IP_VALID
        IND_STRCPY(value, "1");
    else
        IND_STRCPY(value, "0");
#else
    IND_STRCPY(value, "1");
#endif
    return 0;
}
#endif

static int JseCurrentStbIpRead(const char* param, char* value, int len)
{
    char ifname[URL_LEN] = { 0 };
    char ifaddr[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    strcpy(value, network_address_get(ifname, ifaddr, URL_LEN));
    return 0;
}

static int JseCurrentStbNetmaskRead(const char* param, char* value, int len)
{
    char ifname[URL_LEN] = { 0 };
    char ifmask[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    strcpy(value, network_netmask_get(ifname, ifmask, URL_LEN));
    return 0;
}

static int JseCurrentStbGatewayRead(const char* param, char* value, int len)
{
    char ifname[URL_LEN] = { 0 };
    char ifgate[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    strcpy(value, network_gateway_get(ifname, ifgate, URL_LEN));
    return 0;
}

static int JseCurrentStbDnsRead(const char* param, char* value, int len)
{
    char ifname[URL_LEN] = { 0 };
    char ifdns0[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    strcpy(value, network_dns0_get(ifname, ifdns0, URL_LEN));
    return 0;
}

static int JseCurrentStbDns2Read(const char* param, char* value, int len)
{
    char ifname[URL_LEN] = { 0 };
    char ifdns1[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    strcpy(value, network_dns1_get(ifname, ifdns1, URL_LEN));
    return 0;
}

#ifdef ENABLE_IPV6
static int JseIpv6stbIPRead(const char* param, char* value, int len)
{
    sysSettingGetString("ipv6_ip", value, len, 0);
    return 0;
}

static int JseIpv6stbIPWrite(const char* param, char* value, int len)
{
    sysSettingSetString("ipv6_ip", value);
    return 0;
}

static int JseIpv6PrixRead(const char* param, char* value, int len)
{
    int ipv6_prix = 0;
    sysSettingGetInt("ipv6_prix", &ipv6_prix, 0);
    if(ipv6_prix >= 0)
        sprintf(value, "%d", ipv6_prix);
    return 0;
}

static int JseIpv6PrixWrite(const char* param, char* value, int len)
{
    sysSettingSetString("ipv6_prix", value);

    return 0;
}

static int JseIpv6GatewayRead(const char* param, char* value, int len)
{
    sysSettingGetString("ipv6_gateway", value, len, 0);
    return 0;
}

static int JseIpv6GatewayWrite(const char* param, char* value, int len)
{
    sysSettingSetString("ipv6_gateway", value);
    return 0;
}

static int JseIpv6DnsRead(const char* param, char* value, int len)
{
    sysSettingGetString("ipv6_dns", value, len, 0);
    return 0;
}

static int JseIpv6DnsWrite(const char* param, char* value, int len)
{
    sysSettingSetString("ipv6_dns", value);
    return 0;
}

static int JseIpv6Dns2Read(const char* param, char* value, int len)
{
    sysSettingGetString("ipv6_dns1", value, len, 0);
    return 0;
}

static int JseIpv6Dns2Write(const char* param, char* value, int len)
{
    sysSettingSetString("ipv6_dns1", value);
    return 0;
}
#endif

/*************************************************
Description: 初始化海博静态IP配置定义的接口,由JseNetwork.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseStaticInit()
{
    JseCall* call;

#ifdef HUAWEI_C10
    //C10 regist
    call = new JseFunctionCall("static_arp_probe", JseStaticArpProbeRead, 0);
    JseRootRegist(call->name(), call);
#endif

    //以下全为C10/C20 regist
    call = new JseFunctionCall("CurrentStbIp", JseCurrentStbIpRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("IPAddr", JseCurrentStbIpRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("CurrentStbNetmask", JseCurrentStbNetmaskRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("CurrentStbGateway", JseCurrentStbGatewayRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("GWAddr", JseCurrentStbGatewayRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("CurrentStbDns", JseCurrentStbDnsRead, 0);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("CurrentStbDns2", JseCurrentStbDns2Read, 0);
    JseRootRegist(call->name(), call);

#ifdef ENABLE_IPV6
    call = new JseFunctionCall("Ipv6stbIP", JseIpv6stbIPRead, JseIpv6stbIPWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("Ipv6Prix", JseIpv6PrixRead, JseIpv6PrixWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("Ipv6Gateway", JseIpv6GatewayRead, JseIpv6GatewayWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("Ipv6Dns", JseIpv6DnsRead, JseIpv6DnsWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("Ipv6Dns2", JseIpv6Dns2Read, JseIpv6Dns2Write);
    JseRootRegist(call->name(), call);
#endif
    return 0;
}
