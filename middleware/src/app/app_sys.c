
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include "app_include.h"
#include "ind_mem.h"
#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "MessageValueDebug.h"
#include "config/pathConfig.h"
#include "preconfig.h"
#include "SettingEnum.h"
#include "SettingModuleNetwork.h"

static int g_bootflag = 1;
static const char NET_INTERFACE[2][7] = {"eth0", "rausb0"};
#if defined(Jiangsu)
static char NET_VLAN_INTERFACE[] = "rausb0.4095";
#endif


void mid_sys_boot_set(int flag)
{
	g_bootflag = flag;
}

int mid_sys_boot_get(void)
{
	return g_bootflag;
}

/***************************** need to be moved******************************/
#define CONFIG_LEN (32*1024)

#if defined(Jiangsu)
const char *sys_get_net_interface_base(void)
{
    int netType = 0;
    sysSettingGetInt("nettype", &netType, 0);
    if(netType != NET_ETH && netType != NET_WIRELESS) {
        netType = NET_ETH;
    }
    return NET_INTERFACE[netType];
}

int sys_get_net_vlanid_a()
{
    int id = 0;
    if (appSettingGetInt("vlanid_a", &id, 0)
        || id <= 0 || id > 4095)
        id = 0;
    return id;
}
#endif

const char *sys_get_net_interface(void)
{
    int netType = 0;
    sysSettingGetInt("nettype", &netType, 0);
    if(netType != NET_ETH && netType != NET_WIRELESS) {
        netType = NET_ETH;
    }
#if defined(Jiangsu)
    if (sys_get_net_vlanid_a() > 0) {
        snprintf(NET_VLAN_INTERFACE, sizeof(NET_VLAN_INTERFACE), "%s.%d", NET_INTERFACE[netType], sys_get_net_vlanid_a());
        return NET_VLAN_INTERFACE;
    }
#endif
    return NET_INTERFACE[netType];
}

int sys_get_net_ifname(char *ifname, int len)
{
    if(ifname == NULL || len <= 0) {
        LogSafeOperError("ifname or len err\n");
        return -1;
    }

    int connectType = 0;
    sysSettingGetInt("connecttype", &connectType, 0);
    if(connectType == NETTYPE_PPPOE) {
        IND_MEMCPY(ifname, "ppp0", len);
    } else {
        IND_MEMCPY(ifname, sys_get_net_interface(), len);
    }
    return 0;
}

