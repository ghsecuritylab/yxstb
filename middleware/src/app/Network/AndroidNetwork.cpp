#include "AndroidNetwork.h"
#include "NetworkAssertions.h"

#include "NetworkFunctions.h"
#include <stdio.h>

extern const char* gNetworkCardNames[2];

void AndroidNetworkStateChanaged(const char* ifname)
{
    NETWORK_LOG_INFO("ifname:%s\n", ifname);
    const int kLength = 64;
    char buff[kLength] = { 0 };
    NetworkInterface* iface = 0;

    getAndroidNetworkInfo(ifname, "connectType", buff, kLength);
    if (!strncmp("wired", buff, 5))
        iface = new NetworkInterfaceAndroid(new WiredNetworkCard(gNetworkCardNames[0]), ifname);
    else
        iface = new NetworkInterfaceAndroid(new WirelessNetworkCard(gNetworkCardNames[1]), ifname);

    getAndroidNetworkInfo(ifname, "protocolType", buff, kLength);
    if (!strncmp("pppoe", buff, 5))
        iface->setProtocolType(NetworkInterface::PT_PPPOE);
    else if (!strncmp("dhcp", buff, 4))
        iface->setProtocolType(NetworkInterface::PT_DHCP);
    else
        iface->setProtocolType(NetworkInterface::PT_STATIC);

    getAndroidNetworkInfo(ifname, "addressType", buff, kLength);
    if (!strncmp("v4", buff, 2))
        iface->setAddressType(NetworkInterface::AT_IPV4);
    else
        iface->setAddressType(NetworkInterface::AT_IPV6);

    char address[kLength] = { 0 };
    char netmask[kLength] = { 0 };
    char gateway[kLength] = { 0 };
    getAndroidNetworkInfo(ifname, "address", address, kLength);
    getAndroidNetworkInfo(ifname, "netmask", netmask, kLength);
    getAndroidNetworkInfo(ifname, "gateway", gateway, kLength);
    if (iface->getAddressType() == NetworkInterface::AT_IPV4) {
        IPv4Setting ipv4Conf;
        ipv4Conf.setAddress(address);
        ipv4Conf.setNetmask(netmask);
        ipv4Conf.setGateway(gateway);
        iface->setIPv4Setting(ipv4Conf);
    } else {
        IPv6Setting ipv6Conf;
        ipv6Conf.setAddress(address);
        ipv6Conf.setGateway(gateway);
        iface->setIPv6Setting(ipv6Conf);
    }

    NetworkCard* device = networkManager().getActiveDevice();
    if (device)
        networkManager().delDevice(device->devname());

    networkManager().setActiveDevice(iface->device());
    networkManager().setActiveInterface(iface);
    networkManager().addDevice(iface->device());
    networkManager().addInterface(iface);
    NETWORK_LOG_INFO("[%s] [%s] [%s] [%s]\n", 
        ifname,
        network_address_get(ifname, address, kLength),
        network_netmask_get(ifname, netmask, kLength),
        network_gateway_get(ifname, gateway, kLength)
        );
}
