#include "SysSetting.h"
#include "AppSetting.h"

#include "NetworkAssertions.h"
#include "NetworkFunctions.h"
#include "NetworkErrorCode.h"

#include "osex_net.h"
#include "mid_sys.h"
#include "mid_fpanel.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <net/if.h>
#include <sys/sysinfo.h>
#include "IPTVMiddleware.h"

const char* gNetworkCardNames[2] = { "eth0", "rausb0" };

void* _NetworkStatusListen(void* param)
{
    NETWORK_LOG_INFO("_NetworkStatusListen run\n");
    NetworkManager* manager = (NetworkManager*)param;
    while (manager->listen())
        NETWORK_LOG_WARN("_NetworkStatusListen error\n");
    return (void*)0;
}

extern "C"

const char* network_wired_devname()
{
    return gNetworkCardNames[0];
}

const char* network_wifi_devname()
{
    return gNetworkCardNames[1];
}

int network_init() //main_net
{
    NETWORK_LOG_INFO("\n***********************NetworkTest************************\n");

    pthread_t pid;
    pthread_attr_t attributes;
    pthread_attr_init(&attributes);
    pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
    pthread_create(&pid, &attributes, _NetworkStatusListen, &networkManager());
    pthread_attr_destroy(&attributes);

    char username[64] = { 0 };
    char password[64] = { 0 };
    char clientid[64] = { 0 };
    char macaddr[18] = { 0 };
    char address[64] = { 0 };
    char netmask[64] = { 0 };
    char gateway[64] = { 0 };
    char dns0[64] = { 0 };
    char dns1[64] = { 0 };
    int prix = 0;
    int netType = 0, connectType = 0, vlanA = 0;

    IPv4Setting ipv4Conf;
    IPv6Setting ipv6Conf;
    DHCPSetting dhcpConf;
    PPPSetting pppConf;
    IPConflictSetting conflictConf;

    NetworkErrorCodeRegister(networkManager());

    sysSettingGetInt("nettype", &netType, 0);
    sysSettingGetInt("connecttype", &connectType, 0);
    NetworkCard* wire = new WiredNetworkCard(gNetworkCardNames[0]);
    NetworkCard* wifi = new WirelessNetworkCard(gNetworkCardNames[1]);
    NetworkInterface* iface = 0;
    if (!wire || !wifi)
        return -1;
    networkManager().addDevice(wire);
    networkManager().addDevice(wifi);
    if (netType == NetworkCard::NT_WIRELESS) {
        osex_netmac_get(wifi->devname(), macaddr);
        wifi->linkUp();
        networkManager().setActiveDevice(wifi);
#if defined(ANDROID)
        NETWORK_LOG_INFO("Debug\n");
        iface = new NetworkInterfaceAndroid(wifi);
#else
        iface = new NetworkInterfaceLinux(wifi);
#endif
    } else {
        osex_netmac_get(wire->devname(), macaddr);
        wire->linkUp();
        networkManager().setActiveDevice(wire);
#if defined(ANDROID)
        iface = new NetworkInterfaceAndroid(wire);
#else
        iface = new NetworkInterfaceLinux(wire);
#endif
    }

    iface->setMac(macaddr);

    conflictConf.setReplyTime(10);
    conflictConf.setConflictTime(1);
    conflictConf.setUnconflictTime(60);

    //networkManager().getActiveDevice()->linkUp();
    int encrpt = 0; //TODO need get the value from configure file.
    char data[1024] = { 0 };
    switch (connectType) {
    case 1: //PPPoE
        sysSettingGetString("netuser", username, 64, 0);
        //sysSettingGetString("netpasswd", password, 64, 0); //TODO des encrypt
        sysSettingGetString("netAESpasswd", password, sizeof(password), 0);
        pppConf.setUsername(username);
        pppConf.setPassword(password);
        pppConf.setRetryTimes(3);
        iface->setPPPSetting(pppConf);
        iface->setProtocolType(NetworkInterface::PT_PPPOE);
        break;
    case 2: //Dhcp
        sysSettingGetString("dhcpuser", username, 64, 0);
        //sysSettingGetString("dhcppasswd", password, 64, 0); //TODO des encrypt
        sysSettingGetString("ipoeAESpasswd", password, sizeof(password), 0);
        mid_sys_serial(clientid);
        dhcpConf.setUsername(username);
        dhcpConf.setPassword(password);
        dhcpConf.setClientID(clientid);
        dhcpConf.setRetryTimes(3);
        dhcpConf.setRetryInterval(30);
        dhcpConf.setReTransSeq("2,4,8");
        dhcpConf.setLeaseTimeSeq(".5,0.75,7/8,15/16");
        dhcpConf.setEnterpriseNumber(1);
        dhcpConf.setSuboptCode(1);
        //dhcpConf.setVerify("LNUNICOMIPTVDHCP");
        if (encrpt)
            dhcpConf.setVendorClass(EncryOption60(data, dhcpConf.getUsername(), dhcpConf.getPassword(), dhcpConf.getEnterpriseNumber()));
        else
            dhcpConf.setVendorClass("69:70:74:76"); //ascii: iptv
        iface->setDHCPSetting(dhcpConf);
        iface->setProtocolType(NetworkInterface::PT_DHCP);
        iface->setIPConflictSetting(conflictConf);
        break;
    case 3: //Static
        if (NetworkInterface::AT_IPV4 == iface->getAddressType()) {
            sysSettingGetString("ip", address, 16, 0);
            sysSettingGetString("netmask", netmask, 16, 0);
            sysSettingGetString("gateway", gateway, 16, 0);
            sysSettingGetString("dns", dns0, 16, 0);
            sysSettingGetString("dns1", dns1, 16, 0);
            ipv4Conf.setAddress(address);
            ipv4Conf.setNetmask(netmask);
            ipv4Conf.setGateway(gateway);
            ipv4Conf.setDns0(dns0);
            ipv4Conf.setDns1(dns1);
            iface->setIPv4Setting(ipv4Conf);
        } else {
            sysSettingGetString("ipv6_ip", address, 64, 0);
            sysSettingGetString("ipv6_gateway", gateway, 64, 0);
            sysSettingGetString("ipv6_dns", dns0, 64, 0);
            sysSettingGetString("ipv6_dns1", dns1, 64, 0);
            sysSettingGetInt("ipv6_prix", &prix, 0);
            ipv6Conf.setAddress(address);
            ipv6Conf.setGateway(gateway);
            ipv6Conf.setDns0(dns0);
            ipv6Conf.setDns1(dns1);
            ipv6Conf.setSubnetPrefix(prix);
            iface->setIPv6Setting(ipv6Conf);
        }
        iface->setIPConflictSetting(conflictConf);
        iface->setProtocolType(NetworkInterface::PT_STATIC);
        break;
    default:
        ;
    }
    networkManager().addInterface(iface);
    networkManager().setActiveInterface(iface);

#if defined(ANDROID)
    return 0;
#endif
    return networkManager().refresh();
}

extern "C"
const char* network_default_ifname(char* ifname, int size)
{
    NetworkInterface* iface = networkManager().getActiveInterface();
    if (!iface)
        return 0;
    snprintf(ifname, size, "%s", iface->ifname());
    return ifname;
}

extern "C"
int network_connect(const char* ifname)
{
    NETWORK_LOG_INFO("iface name: %s\n", ifname);
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return -1;
    iface->connect();
    return networkManager().refresh();
}

extern "C"
int network_disconnect(const char* ifname)
{
    NETWORK_LOG_INFO("iface name: %s\n", ifname);
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return -1;
    iface->disconnect();
    return networkManager().refresh();
}

extern "C"
const char* network_ifacemac_get(const char* ifname, char* mac, int size)
{
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return 0;
    snprintf(mac, size, "%s", iface->getMac());
    return mac;
}

extern "C"
const char* network_tokenmac_get(char* mac, int size, int div)
{
#ifdef ANDROID
    {
        static char macaddr[1024] = {0};
        if (macaddr[0] == 0) {
        #ifdef NEW_ANDROID_SETTING
            sysSettingGetString("TokenMACAddress", macaddr, sizeof(macaddr), 0);
        #else
            IPTVMiddleware_SettingGetStr("TokenMACAddress", macaddr, sizeof(macaddr));
        #endif
        }
        if (div) {
            snprintf(mac, size, "%s", macaddr);
            int i;
            for (i=3; i<=15; i+=3) {
                if (size > i)
                    mac[i - 1] = div;
            }
        } else {
            int i;
            for (i=3; i<=13; i+=2) {
                if (size > i) {
                    mac[i - 3] = macaddr[((i - 3) / 2) * 3];
                    mac[i - 2] = macaddr[((i - 3) / 2) * 3 + 1];
                    mac[i - 1] = '\0';
                }
            }
        }
        return mac;
    }
#endif
    static char s_macaddr[6];
    if (!s_macaddr[0])
        osex_netmac_get("eth0", s_macaddr); //TODO 3560E
    if (div)
        snprintf(mac, size, "%02x%c%02x%c%02x%c%02x%c%02x%c%02x",
            s_macaddr[0], div, s_macaddr[1], div,
            s_macaddr[2], div, s_macaddr[3], div,
            s_macaddr[4], div, s_macaddr[5]);
    else
        snprintf(mac, size, "%02x%02x%02x%02x%02x%02x",
            s_macaddr[0], s_macaddr[1],
            s_macaddr[2], s_macaddr[3],
            s_macaddr[4], s_macaddr[5]);
    return mac;
}

extern "C"
const char* network_address_get(const char* ifname, char* address, int size)
{
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return 0;
    return iface->getAddress(address, size);
}

extern "C"
const char* network_netmask_get(const char* ifname, char* netmask, int size)
{
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return 0;
    return iface->getNetmask(netmask, size);
}

extern "C"
const char* network_gateway_get(const char* ifname, char* gateway, int size)
{
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return 0;
    return iface->getGateway(gateway, size);
}

extern "C"
const char* network_dns0_get(const char* ifname, char* dns0, int size)
{
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return 0;
    return iface->getDns0(dns0, size);
}

extern "C"
const char* network_dns1_get(const char* ifname, char* dns1, int size)
{
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return 0;
    return iface->getDns1(dns1, size);
}

extern "C"
unsigned int network_downloadrate_get(const char* ifname)
{
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return 0;
    NetDevStats_t stats;
    if (iface->getStats(&stats) < 0)
        return 0;
    unsigned int rate = 0;
    unsigned int diff = 0;
    static time_t s_last = 0;
    static unsigned int s_rxBytes = 0;
    struct sysinfo si;
    sysinfo(&si);
    if (!s_last)
        diff = si.uptime;
    else
        diff = si.uptime - s_last;
    s_last = si.uptime;
    if (stats.rxBytes > s_rxBytes && diff > 0)
        rate = (stats.rxBytes - s_rxBytes) / diff;
    s_rxBytes = stats.rxBytes;
    return rate;
}

extern "C"
unsigned int network_linkspeed_get(const char* devname)
{
    NetworkCard* device= networkManager().getDevice(devname);
    if(!device)
        return -1;
    return device->getLinkSpeed();
}

extern "C"
unsigned int network_interface_uptime(const char* ifname)
{
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return 0;
    return iface->getConnectionUpTime();
}

extern "C"
int network_connecttype_get(const char* ifname)
{
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return -1;
    int type = -1;
    switch (iface->getProtocolType()) {
        case NetworkInterface::PT_STATIC:
            type = 3;
            break;
        case NetworkInterface::PT_DHCP:
            type = 2;
            break;
        case NetworkInterface::PT_PPPOE:
            type = 1;
            break;
        default:
            ;
    }
    return type;
}

extern "C"
const char* network_default_devname(char* devname, int size)
{
    snprintf(devname, size, "%s", networkManager().getActiveDevice()->devname());
    return devname;
}

extern "C"
int network_device_link_down(const char* devname)
{
    NetworkCard* device= networkManager().getDevice(devname);
    if(!device)
        return -1;
    device->linkDown();
    return 0;
}

extern "C"
int network_device_link_up(const char* devname)
{
    NetworkCard* device= networkManager().getDevice(devname);
    if(!device)
        return -1;
    device->linkUp();
    return 0;
}

extern "C"
int network_device_link_state(const char* devname)
{
    //TODO need think of. ?????
    NetworkCard* device = networkManager().getDevice(devname);
    if(!device)
        return -1;
    int status = device->linkStatus();
    if (status > 0)
        mid_fpanel_netled(1); //Use it templatelly.
    return status;
}

extern "C"
int network_wifi_signal_get(const char* devname, int* quality, int* level, int* noise)
{
    NetworkCard* device = networkManager().getDevice(devname);
    if(!device)
        return -1;
    return static_cast<WirelessNetworkCard*>(device)->getSignalQuality(quality, level, noise);
}
