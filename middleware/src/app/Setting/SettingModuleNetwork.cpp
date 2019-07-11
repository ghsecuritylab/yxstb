
#include "Assertions.h"
#include "SettingModuleNetwork.h"

#include <stdio.h>

#include "Setting.h"
#include "AppSetting.h"
#include "SysSetting.h"
#if defined(CACHE_CONFIG)
#include "CacheSetting.h"
#endif

#include "configCustomer.h"

namespace Hippo {

static SettingModuleNetwork g_SettingModuleNetwork;

SettingModuleNetwork::SettingModuleNetwork()
    : SettingModule()
{
}

SettingModuleNetwork::~SettingModuleNetwork()
{
}

int
SettingModuleNetwork::settingModuleRegister()
{
    sysSetting().add("[ NETWORK ]", "");
    LogSafeOperDebug("SettingModuleNetwork::settingModuleRegister [%d]\n", __LINE__);

    sysSetting().add("netuser", DEFAULT_NET_USER); //pppoe user name
    sysSetting().add("netpasswd", DEFAULT_NET_PASSWORD);  //pppoe password
    sysSetting().add("netAESpasswd", DEFAULT_NET_AES_PASSWORD, true);

    sysSetting().add("dhcpuser", DEFAULT_DHCP_USER); //dhcp user name
    sysSetting().add("dhcppasswd", DEFAULT_DHCP_PASSWORD); //dhcp password
    sysSetting().add("ipoeAESpasswd", DEFAULT_IPOE_AES_PASSWORD, true);
    sysSetting().add("ipoe_flag", 0);            //?
    //sysSetting().add("ipoeneedpswd", "");         //app_sys_setting.c中没有写入flash

    sysSetting().add("connecttype", DEFAULT_CONNECT_TYPE); //1:pppoe 2:dhcp 3:static ip
    sysSetting().add("ipv4_enable", 1);
    sysSetting().add("enableDoubleStack", DEFAULT_ENABLE_DOUBLE_STACK);
    sysSetting().add("nettype", DEFAULT_NET_TYPE); // the network link type 0 is local, 1 is wifi
    sysSetting().add("ip", DEFAULT_IP);
    sysSetting().add("netmask", DEFAULT_NET_MASK);
    sysSetting().add("gateway", DEFAULT_GATEWAY);
    sysSetting().add("dns", DEFAULT_DNS);
    sysSetting().add("dns1", DEFAULT_DNS1);
#if defined(CACHE_CONFIG)
    cacheSetting().add("system.int.connecttype", DEFAULT_CONNECT_TYPE);
    cacheSetting().add("system.int.ipv4_enable", 1);
    cacheSetting().add("system.int.nettype", DEFAULT_NET_TYPE); // the network link type 0 is local, 1 is wifi
    cacheSetting().add("system.string.ip", DEFAULT_IP);
    cacheSetting().add("system.string.netmask", DEFAULT_NET_MASK);
    cacheSetting().add("system.string.gateway", DEFAULT_GATEWAY);
    cacheSetting().add("system.string.dns", DEFAULT_DNS);
    cacheSetting().add("system.string.dns1", DEFAULT_DNS1);
#endif

#ifdef ENABLE_IPV6
    sysSetting().add("ipv6_connectType", "");
    sysSetting().add("ipv6_enable", 0);
    sysSetting().add("ipv6_prix", "");
    sysSetting().add("ipv6_ip", "");
    sysSetting().add("ipv6_gateway", "");
    sysSetting().add("ipv6_dns", "");
    sysSetting().add("ipv6_dns1", "");
    sysSetting().add("ipv6_netuser", "");       //?need
    sysSetting().add("ipv6_netpasswd", "");     //?need

#if defined(CACHE_CONFIG)
    cacheSetting().add("system.string.ipv6_connectType", "");
    cacheSetting().add("system.int.ipv6_enable", 0);
    cacheSetting().add("system.string.ipv6_prix", "");
    cacheSetting().add("system.string.ipv6_ip", "");
    cacheSetting().add("system.string.ipv6_gateway", "");
    cacheSetting().add("system.string.ipv6_dns", "");
    cacheSetting().add("system.string.ipv6_dns1", "");
#endif // end defined(CACHE_CONFIG)
#endif

#ifdef INCLUDE_WIFI
    sysSetting().add("wifi_ssid", "");
    sysSetting().add("wifi_password", "", true);
    sysSetting().add("wifi_encryType", "");
    sysSetting().add("wifi_authType", "");
    sysSetting().add("Wifi_channel", "");
#endif

    //sysSetting().add("option_60", "");
    //sysSetting().add("option_61", "");
    //sysSetting().add("option_125", "");

#if defined(BLUETOOTH)
    sysSetting().add("BluetoothPIN", "");
    sysSetting().add("BluetoothDeviceName", "");
#endif // BLUETOOTH

    /**********************  customer  ******************************/
    //appSetting().add("[ NETWORK ]", "");
#ifdef ENABLE_DBLVLAN
    appSetting().add("vlanid_a", DEFAULT_VLANID_A);
    appSetting().add("vlanid_b", DEFAULT_VLANID_B);
#endif


    return 0;
}

} // namespace Hippo
extern "C"
int settingNetwork()
{
    return 0;
}

