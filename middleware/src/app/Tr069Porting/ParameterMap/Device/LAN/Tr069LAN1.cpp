#include "Tr069LAN1.h"

#include "Tr069GroupCall.h"
#include "Tr069FunctionCall.h"
#include "TR069Assertions.h"
#include "Tr069Stats.h"
#include "NetworkFunctions.h"
#include "SysSetting.h"
#include "SettingModuleNetwork.h"

#include "sys_basic_macro.h"

#include <string.h>

extern "C" int mid_net_dns_set(char* dns0, char* dns1);

static int tr069_MACAddressOverride_Read(char* value, unsigned int length)
{
    return 0;
}
static int tr069_MACAddressOverride_Write(char* value, unsigned int length)
{
    return 0;
}
static int tr069_IsSupportIPv6_Read(char* value, unsigned int length)
{
    strcpy(value, "1"); // 原先从return 返回，现从value 返回
	    return 0;
}

static int tr069_DNSServers2_Read(char* value, unsigned int length)
{
    char ifname[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    network_dns1_get(ifname, value, length);
    return 0;
}

static int tr069_DNSServers2_Write(char* value, unsigned int length)
{
    int len = 0;
    char dns0[16] = {0};
    char dns1[16] = {0};

    if (!value || !strlen(value))
        return -1;

    len = strlen(value);

    if (len >= 16)
        return -1;
    sysSettingSetString("dns1", value);
    sysSettingGetString("dns", dns0, NET_LEN, 0);
    sysSettingGetString("dns1", dns1, NET_LEN, 0);
    mid_net_dns_set(dns0, dns1);

    return 0;
}

/*------------------------------------------------------------------------------
	给网络接口分配地址的方法，枚举类型： “DHCP” “Static”
	当发生改变时，需立即上报终端管理系统。
 ------------------------------------------------------------------------------*/
static int tr069_AddressingType_Read(char* value, unsigned int length)
{
    static int type = -1;

    if(-1 == type) {
        sysSettingGetInt("connecttype", &type, 0);
        LogTr069Debug("AddressingType [%d].   {%d:NETTYPE_PPPOE, %d:NETTYPE_DHCP, %d:NETTYPE_STATIC}\n",
            type, NETTYPE_PPPOE, NETTYPE_DHCP,NETTYPE_STATIC);
    }
    switch(type) {
    case NETTYPE_PPPOE:
        strcpy(value, "PPPoE");
        break;
    case NETTYPE_DHCP:
        strcpy(value, "DHCP");
        break;
    case NETTYPE_STATIC:
		strcpy(value, "Static");
		break;
    case NETTYPE_DHCP_ENCRYPT:
		strcpy(value, "IPoE");
		break;
    default:
        strcpy(value, "Static");
        break;
    }
    return 0;
}
static int tr069_AddressingType_Write(char* value, unsigned int length)
{
    if (!value || !strlen(value))
        return -1;
    int connectType = NETTYPE_STATIC;
    if(0 == strcasecmp(value, "PPPoE"))
        connectType = NETTYPE_PPPOE;
    else if(0 == strcasecmp(value, "DHCP"))
        connectType = NETTYPE_DHCP;
    else
        connectType = NETTYPE_STATIC;

    sysSettingSetInt("connecttype", connectType);
    return 0;
}

/*------------------------------------------------------------------------------
	当前分配给网络接口的IP地址。本参数在AddressingType为”DHCP”时为R。
 ------------------------------------------------------------------------------*/
static int tr069_IPAddress_Read(char* value, unsigned int length)
{
    char ifname[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    network_address_get(ifname, value, length);
    return 0;
}

static int tr069_IPAddress_Write(char* value, unsigned int length)
{
    if (!value || !strlen(value))
        return -1;

    sysSettingSetString("ip", value);
    return 0;
}

static int tr069_SubnetMask_Read(char* value, unsigned int length)
{
    char ifname[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    network_netmask_get(ifname, value, length);
    return 0;
}

static int tr069_SubnetMask_Write(char* value, unsigned int length)
{
    if (!value || !strlen(value))
        return -1;
    sysSettingSetString("netmask", value);
    return 0;
}

/*------------------------------------------------------------------------------
	默认网关
 ------------------------------------------------------------------------------*/
static int tr069_DefaultGateway_Read(char* value, unsigned int length)
{
    char ifname[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    network_gateway_get(ifname, value, length);
    return 0;
}

/*------------------------------------------------------------------------------
	逗号分隔的DNS服务器地址。
 ------------------------------------------------------------------------------*/
static int tr069_DNSServers_Read(char* value, unsigned int length)
{
    char ifname[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    network_dns0_get(ifname, value, length);
    return 0;
}

/*------------------------------------------------------------------------------
	MAC地址
 ------------------------------------------------------------------------------*/
static int tr069_MACAddress_Read(char* value, unsigned int length)
{
#if defined(Sichuan) || (_HW_BASE_VER_ >= 58)
    int num;
    network_tokenmac_get(value, length, ':');
    for (num =0; num<17; num++) {
        if ((*(value+num) >= 97) && (*(value+num) <= 122))
            *(value+num) = *(value + num)-32;
    }
#else
    network_tokenmac_get(value, length, ':');
#endif
    return 0;
}

Tr069LAN1::Tr069LAN1()
	: Tr069GroupCall("LAN")
{

    Tr069Call* macover = new Tr069FunctionCall("MACAddressOverride",    tr069_MACAddressOverride_Read,        tr069_MACAddressOverride_Write);
    Tr069Call* ipv6 = new Tr069FunctionCall("IsSupportIPv6",            tr069_IsSupportIPv6_Read,             NULL);
    Tr069Call* addtype = new Tr069FunctionCall("AddressingType",        tr069_AddressingType_Read,            tr069_AddressingType_Write);
    Tr069Call* addr = new Tr069FunctionCall("IPAddress",                tr069_IPAddress_Read,                 tr069_IPAddress_Write);
    Tr069Call* mask = new Tr069FunctionCall("SubnetMask",               tr069_SubnetMask_Read,                tr069_SubnetMask_Write);
    Tr069Call* gateway = new Tr069FunctionCall("DefaultGateway",        tr069_DefaultGateway_Read,            NULL);
    Tr069Call* dns = new Tr069FunctionCall("DNSServers",                tr069_DNSServers_Read,                NULL);
    Tr069Call* mac = new Tr069FunctionCall("MACAddress",                tr069_MACAddress_Read,                NULL);
#ifdef NEIMENGGU_HD
    Tr069Call* dns2 = new Tr069FunctionCall("DNSServers2",              tr069_DNSServers2_Read,               tr069_DNSServers2_Write);
#endif

    regist(macover->name(), macover);
    regist(ipv6->name(), ipv6);
    regist(addtype->name(), addtype);
    regist(addr->name(), addr);
    regist(mask->name(), mask);
    regist(gateway->name(), gateway);
    regist(dns->name(), dns);
    regist(mac->name(), mac);
#ifdef NEIMENGGU_HD
    regist(dns2->name(), dns2);
#endif

#ifdef TR069_VERSION_2
    Tr069Call* stat = new Tr069Stats();
    regist(stat->name(), stat);
#endif

}

Tr069LAN1::~Tr069LAN1()
{
}
