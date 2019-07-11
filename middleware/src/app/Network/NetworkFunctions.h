#ifndef __NetworkFunctions__H_
#define __NetworkFunctions__H_

#include "NetworkInterface.h"
#include "NetworkManager.h"
#include "NetworkTypes.h"
#include "WiredNetworkCard.h"
#include "WirelessNetworkCard.h"
#include "NetworkErrorCode.h"
#include "NetworkInterfaceLinux.h"
#include "NetworkInterfaceAndroid.h"

#ifdef __cplusplus
extern "C" {
#endif

int network_init();
int network_connect(const char* ifname);
int network_disconnect(const char* ifname);
int network_connecttype_get(const char* ifname);
unsigned int network_interface_uptime(const char* ifname);
int network_device_link_up(const char* devname);
int network_device_link_down(const char* devname);
int network_device_link_state(const char* devname);

const char* network_wired_devname();
const char* network_wifi_devname();
const char* network_default_ifname(char* ifname, int size);
const char* network_default_devname(char* devname, int size);
const char* network_ifacemac_get(const char* ifname, char* mac, int size);
const char* network_tokenmac_get(char* mac, int size, int div);
const char* network_address_get(const char* ifname, char* address, int size);
const char* network_netmask_get(const char* ifname, char* netmask, int size);
const char* network_gateway_get(const char* ifname, char* gateway, int size);
const char* network_dns0_get(const char* ifname, char* dns0, int size);
const char* network_dns1_get(const char* ifname, char* dns1, int size);

unsigned int network_downloadrate_get(const char* ifname);
unsigned int network_linkspeed_get(const char* devname);
int network_wifi_signal_get(const char*, int*, int*, int*);

#ifdef __cplusplus
}
#endif

#endif
