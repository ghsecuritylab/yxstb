#include "NetworkAssertions.h"
#include "PPPOEStateHandler.h"
#include "NetworkManager.h"
#include "NetworkInterface.h"
#include "NetworkTypes.h"
#include "ConfigFileParser.h"

#include "StringData.h"
#include "MessageTypes.h"
#include "NativeHandler.h"
#include "MessageValueNetwork.h"
#include "config.h"

#include <stdio.h>
#include <string.h>

PPPOEStateHandler::PPPOEStateHandler(NetworkInterface* iface) : StateHandler(iface)
{
}

PPPOEStateHandler::~PPPOEStateHandler()
{
}

int
PPPOEStateHandler::handleState(int state)
{
    NETWORK_LOG_INFO("State: %d\n", state);
    std::string dst;
    char fifoname[128] = { 0 };

    snprintf(fifoname, sizeof(fifoname), "%s/ppp-%s.info", PPPOE_ROOT_DIR, mIface->ifname());
    Hippo::ConfigFileParser cfg;
    if (cfg.fileOpen(fifoname) < 0) {
        NETWORK_LOG_WARN("fifoname[%s] open error!\n", fifoname);
        return -1;
    }
    if (state > 0) {
        EraseQuoteMark(cfg.GetVarStr(DEFAULT_FIELD, "ADDRTYPE"), dst);
        if (!strncmp(dst.c_str(), "v4", 2)) {
            IPv4Setting ipv4Conf;
            ipv4Conf.setAddress(EraseQuoteMark(cfg.GetVarStr(DEFAULT_FIELD, "IPADDR"), dst));
            ipv4Conf.setNetmask(EraseQuoteMark(cfg.GetVarStr(DEFAULT_FIELD, "NETMASK"), dst));
            ipv4Conf.setGateway(EraseQuoteMark(cfg.GetVarStr(DEFAULT_FIELD, "GATEWAYS"), dst));

            EraseQuoteMark(cfg.GetVarStr(DEFAULT_FIELD, "DNSSERVERS"), dst);
            std::size_t index = dst.find_first_of(' ');
            if (index != std::string::npos) {
                std::string dns0 = dst.substr(0, index);
                std::string dns1 = dst.substr(index + 1, 16);
                ipv4Conf.setDns0(dns0.c_str());
                ipv4Conf.setDns1(dns1.c_str());
            } else
                ipv4Conf.setDns0(dst.c_str());

            mIface->setIPv4Setting(ipv4Conf);
            mIface->setAddressType(NetworkInterface::AT_IPV4);
            NETWORK_LOG_INFO("%s %s %s dns0:%s dns1:%s\n", ipv4Conf.getAddress(), ipv4Conf.getNetmask(), ipv4Conf.getGateway(), ipv4Conf.getDns0(), ipv4Conf.getDns1());
        }
        Hippo::StringData* data = new Hippo::StringData(mIface->ifname());
        Hippo::Message* msg = Hippo::defNativeHandler().obtainMessage(MessageType_Network, MV_Network_ConnectOk, mIface->getProtocolType(), data);
        data->safeUnref();
        Hippo::defNativeHandler().sendMessage(msg);
        mIface->stateActiveChanaged(true);
        return 0;
    } else if (state < 0) {
        bool bFound = false;
        int errcode = 0;
        if (networkManager().hasCustomErrCode()) {
            if (EraseQuoteMark(cfg.GetVarStr(DEFAULT_FIELD, "ERRDETAILS"), dst)) {
                std::size_t index = dst.find_first_of(',');
                if (index != std::string::npos)
                    errcode = atoi(dst.substr(0, index).c_str());
            }
        }
        errcode = errcode << 16 | atoi(EraseQuoteMark(cfg.GetVarStr(DEFAULT_FIELD, "ERRCODE"), dst));
        Hippo::StringData* data = new Hippo::StringData(mIface->ifname());
        Hippo::Message* msg = Hippo::defNativeHandler().obtainMessage(MessageType_Network, MV_Network_PppoeError, errcode, data);
        data->safeUnref();
        Hippo::defNativeHandler().sendMessage(msg);
        mIface->stateActiveChanaged(false);
    }
    return 0;
}
