#include "NetworkAssertions.h"
#include "NetlinkStateHandler.h"
#include "NetworkErrorCode.h"
#include "NetworkCard.h"

#include "StringData.h"
#include "MessageValueNetwork.h"
#include "MessageTypes.h"
#include "NativeHandler.h"

NetlinkStateHandler::NetlinkStateHandler(NetworkCard* device) : StateHandler(device)
{

}

NetlinkStateHandler::~NetlinkStateHandler()
{

}

int
NetlinkStateHandler::handleState(int state)
{
    NETWORK_LOG_INFO("NetlinkState: %s\n", NetlinkStatStr(state));
    Hippo::StringData* data = new Hippo::StringData(mDevice->devname());
    Hippo::Message* msg = 0;
    int delay = 0;
    switch (state) {
    case NetworkCard::LS_ETHERNET_UP:
    case NetworkCard::LS_WIRELESS_UP:
        Hippo::defNativeHandler().removeMessages(MessageType_Network, MV_Network_ProtocolUp, 0);
        msg = Hippo::defNativeHandler().obtainMessage(MessageType_Network, MV_Network_ProtocolUp, 0, data);
        break;
    case NetworkCard::LS_ETHERNET_DOWN:
        Hippo::defNativeHandler().removeMessages(MessageType_Network, MV_Network_ProtocolDown, NET_NETWORK_DISCONNECT);
        msg = Hippo::defNativeHandler().obtainMessage(MessageType_Network, MV_Network_ProtocolDown, NET_NETWORK_DISCONNECT, data);
        break;
    case NetworkCard::LS_WIRELESS_DOWN:
        Hippo::defNativeHandler().removeMessages(MessageType_Network, MV_Network_ProtocolDown, WIFI_LOADING_FAILED);
        msg = Hippo::defNativeHandler().obtainMessage(MessageType_Network, MV_Network_ProtocolDown, WIFI_LOADING_FAILED, data);
        break;
    case NetworkCard::LS_WIRELESS_JOIN_FAIL:
        Hippo::defNativeHandler().removeMessages(MessageType_Network, MV_Network_WifiJoinFail, WIFI_CONNECT_FAILED);
        msg = Hippo::defNativeHandler().obtainMessage(MessageType_Network, MV_Network_WifiJoinFail, WIFI_CONNECT_FAILED, data);
        break;
    case NetworkCard::LS_WIRELESS_JOIN_SECCESS:
        Hippo::defNativeHandler().removeMessages(MessageType_Network, MV_Network_WifiJoinSuccess, 0);
        msg = Hippo::defNativeHandler().obtainMessage(MessageType_Network, MV_Network_WifiJoinSuccess, 0, data);
        break;
    case NetworkCard::LS_WIRELESS_CHECK_SIGNAL:
        delay = 500; //I have something to say, but... the wireless is too painfull for me.
        Hippo::defNativeHandler().removeMessages(MessageType_Network, MV_Network_WifiCheckSignal, 0);
        msg = Hippo::defNativeHandler().obtainMessage(MessageType_Network, MV_Network_WifiCheckSignal, 0, data);
        break;
    default:
        return 0;
    }
    data->safeUnref();
    if (msg)
        Hippo::defNativeHandler().sendMessageDelayed(msg, delay);
    return 0;
}
