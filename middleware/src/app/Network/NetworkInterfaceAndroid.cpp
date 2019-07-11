#include "NetworkAssertions.h"
#include "NetworkInterfaceAndroid.h"
#include "IPTVMiddleware.h"
#include "NetworkCard.h"

NetworkInterfaceAndroid::NetworkInterfaceAndroid(NetworkCard* device, const char* ifname) 
    : NetworkInterface(device, ifname)
{
}

NetworkInterfaceAndroid::NetworkInterfaceAndroid(NetworkCard* device, int vlanId) 
    : NetworkInterface(device, vlanId)
{
}

NetworkInterfaceAndroid::~NetworkInterfaceAndroid()
{
}

int NetworkInterfaceAndroid::connect(int mode)
{
    NETWORK_LOG_INFO("Debug\n");
    return 0;
}

int NetworkInterfaceAndroid::disconnect()
{
    NETWORK_LOG_INFO("Debug\n");
    return 0;
}

int NetworkInterfaceAndroid::preSelect(fd_set* rset, fd_set* wset, fd_set* eset, struct timeval& time)
{
    NETWORK_LOG_INFO("Debug\n");
    return 0;
}

int NetworkInterfaceAndroid::postSelect(fd_set* rset, fd_set* wset, fd_set* eset)
{
    NETWORK_LOG_INFO("Debug\n");
    return 0;
}

int NetworkInterfaceAndroid::startCheckIP()
{
    return 0;
}

int NetworkInterfaceAndroid::stopCheckIP()
{
    return 0;
}
