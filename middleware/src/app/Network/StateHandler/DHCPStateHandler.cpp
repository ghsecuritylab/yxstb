#include "NetworkAssertions.h"
#include "DHCPStateHandler.h"
#include "NetworkManager.h"
#include "NetworkInterface.h"
#include "NetworkTypes.h"
#include "ConfigFileParser.h"

#include "StringData.h"
#include "NativeHandler.h"
#include "MessageTypes.h"
#include "MessageValueNetwork.h"

#include "config.h"
#include <stdio.h>

DHCPStateHandler::DHCPStateHandler(NetworkInterface* iface) : StateHandler(iface)
{
}

DHCPStateHandler::~DHCPStateHandler()
{
}

#define DECLINE_STATE 2
int
DHCPStateHandler::handleState(int state)
{
    NETWORK_LOG_INFO("State: %d\n", state);
    std::string dst;
    char infoname[128] = { 0 };

    if (0 == state)
        return 0; //do nothing. 

    snprintf(infoname, sizeof(infoname), "%s/dhclient-%s.info", DHCP_ROOT_DIR, mIface->ifname());
    Hippo::ConfigFileParser cfg;
    if (cfg.fileOpen(infoname) < 0) {
        NETWORK_LOG_WARN("fifoname[%s] open error!\n", infoname);
        return -1;
    }
    if (DECLINE_STATE == cfg.GetVarInt(DEFAULT_FIELD, "extend_state"))
        return 0;

    if (state > 0) {
         if (NetworkInterface::AT_IPV4 == mIface->getAddressType()) {
            IPv4Setting ipv4Conf;
            ipv4Conf.setAddress(cfg.GetVarStr(DEFAULT_FIELD, "ip_address").c_str());
            ipv4Conf.setNetmask(cfg.GetVarStr(DEFAULT_FIELD, "subnet_mask").c_str());
            ipv4Conf.setGateway(cfg.GetVarStr(DEFAULT_FIELD, "gateway").c_str());
            ipv4Conf.setDns0(cfg.GetVarStr(DEFAULT_FIELD, "dns0").c_str());
            ipv4Conf.setDns1(cfg.GetVarStr(DEFAULT_FIELD, "dns1").c_str());
            mIface->setIPv4Setting(ipv4Conf);
            NETWORK_LOG_INFO("%s %s %s dns0:%s dns1:%s\n", ipv4Conf.getAddress(), ipv4Conf.getNetmask(), ipv4Conf.getGateway(), ipv4Conf.getDns0(), ipv4Conf.getDns1());
        } else {
            IPv6Setting ipv6Conf;
            ipv6Conf.setAddress(cfg.GetVarStr(DEFAULT_FIELD, "ip6_address").c_str());
            ipv6Conf.setGateway(cfg.GetVarStr(DEFAULT_FIELD, "gateway").c_str());
            ipv6Conf.setSubnetPrefix(cfg.GetVarInt(DEFAULT_FIELD, "ip6_prefixlen"));
            ipv6Conf.setDns0(cfg.GetVarStr(DEFAULT_FIELD, "dns0").c_str());
            ipv6Conf.setDns1(cfg.GetVarStr(DEFAULT_FIELD, "dns1").c_str());
            mIface->setIPv6Setting(ipv6Conf);
            NETWORK_LOG_INFO("%s %d %s dns0:%s dns1:%s\n", ipv6Conf.getAddress(), ipv6Conf.getSubnetPrefix(), ipv6Conf.getGateway(), ipv6Conf.getDns0(), ipv6Conf.getDns1());
        }
        Hippo::StringData* data = new Hippo::StringData(mIface->ifname());
        Hippo::Message* msg = Hippo::defNativeHandler().obtainMessage(MessageType_Network, MV_Network_ConnectOk, mIface->getProtocolType(), data);
        data->safeUnref();
        Hippo::defNativeHandler().sendMessage(msg);
        mIface->stateActiveChanaged(true);
    } else if (state < 0) {
        bool bFound = false;
        int errcode = 0;
        if (networkManager().hasCustomErrCode()) {
            if (cfg.GetVarStr(DEFAULT_FIELD, "errdetails").c_str()) {
                std::size_t index = dst.find_first_of(',');
                if (index != std::string::npos)
                    errcode = atoi(dst.substr(0, index).c_str());
            }
        }
        errcode = (errcode << 16) | cfg.GetVarInt(DEFAULT_FIELD, "errcode");
        Hippo::StringData* data = new Hippo::StringData(mIface->ifname());
        Hippo::Message* msg = Hippo::defNativeHandler().obtainMessage(MessageType_Network, MV_Network_DhcpError, errcode, data);
        data->safeUnref();
        Hippo::defNativeHandler().sendMessage(msg);
        mIface->stateActiveChanaged(false);
    }
    return 0;
}
