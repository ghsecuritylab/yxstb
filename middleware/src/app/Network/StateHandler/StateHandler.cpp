#include "NetworkAssertions.h"
#include "StateHandler.h"
#include "NetworkInterface.h"
#include "NetworkManager.h"
#include "StringData.h"

#include "MessageValueNetwork.h"
#include "MessageTypes.h"
#include "NativeHandler.h"
#include "config.h"

#include <stdio.h>

StateHandler::StateHandler(NetworkInterface* iface) : mIface(iface)
{

}

StateHandler::StateHandler(NetworkCard* device) : mDevice(device)
{

}

int
StateHandler::handleState(int state)
{
    if (state > 0) {
        Hippo::StringData* data = new Hippo::StringData(mIface->ifname());
        Hippo::Message* msg = Hippo::defNativeHandler().obtainMessage(MessageType_Network, MV_Network_ConnectOk, mIface->getProtocolType(), data);
        data->safeUnref();
        Hippo::defNativeHandler().sendMessage(msg);
        mIface->stateActiveChanaged(true);
    }
    return 0;
}

int
StateHandler::handleIPConflict(int state)
{
    Hippo::StringData* data = new Hippo::StringData(mIface->ifname());
    Hippo::Message *msg = 0;
    if (state > 0) //ip conflict
        msg = Hippo::defNativeHandler().obtainMessage(MessageType_Network, MV_Network_ProtocolConflict, to_positive_sign(NET_IP_CONFILICT), data);
    else 
        msg = Hippo::defNativeHandler().obtainMessage(MessageType_Network, MV_Network_ProtocolConflict, 0, data);
    data->safeUnref();
    Hippo::defNativeHandler().sendMessage(msg);
    return 0;
}

const char*
EraseQuoteMark(std::string s, std::string& d)
{
    //eg. '192.168.1.1' --> 192.168.1.1
    std::size_t i = s.find_first_of('\'');
    std::size_t j = s.find_last_of('\'');
    if (j <= ++i)
        return 0;
    d = s.substr(i, j - i);
    return d.c_str();
}
