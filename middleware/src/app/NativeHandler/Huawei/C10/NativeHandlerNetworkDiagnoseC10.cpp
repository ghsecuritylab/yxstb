
#include <iostream>

#include "NativeHandlerNetworkDiagnoseC10.h"
#include "NativeHandlerAssertions.h"
#include "NetworkDiagnose.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "BrowserAgent.h"
#include "BrowserEventQueue.h"


namespace Hippo {

NativeHandlerNetworkDiagnoseC10::NativeHandlerNetworkDiagnoseC10()
{
}

NativeHandlerNetworkDiagnoseC10::~NativeHandlerNetworkDiagnoseC10()
{
}

bool
NativeHandlerNetworkDiagnoseC10::handleMessage(Message *msg)
{
    if(MessageType_Repaint != msg->what)
        NATIVEHANDLER_LOG("what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);

    return NativeHandlerPublicC10::handleMessage(msg);
}

void
NativeHandlerNetworkDiagnoseC10::onActive()
{
    Message *msg = NULL;

    // Don't display network disconnect error code in the NetworkDiagnose state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(msg);
}

void
NativeHandlerNetworkDiagnoseC10::onUnactive()
{
    Message *msg = NULL;

    // Display network disconnect error code when leave the NetworkDiagnose state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 1);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 1);
    epgBrowserAgent().sendMessage(msg);
}

bool
NativeHandlerNetworkDiagnoseC10::doNetworkProtocolConflict(int errcode, const char* ifname)
{
    NATIVEHANDLER_LOG("ErrorCode[%d], IfaceName[%s]\n", errcode, ifname);

    NetworkDiagnose::DiagnoseProcess* diagnose = networkDiagnose()->getDiagnose();
    if (!diagnose)
        return false;
    if (eTypeAddressDiagnose != diagnose->type())
        return false;
    if (!errcode)
        diagnose->doResult(0, 0);
    diagnose->doResult(-1, errcode);
    return true;
}

bool
NativeHandlerNetworkDiagnoseC10::doNetworkConnectOk(int errcode, const char* ifname)
{
    NATIVEHANDLER_LOG("ErrorCode[%d], IfaceName[%s]\n", errcode, ifname);

    Message* msg = 0;
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return false;
    NetworkDiagnose::DiagnoseProcess* diagnose = networkDiagnose()->getDiagnose();
    if (!diagnose)
        return false;
    if (eTypeGatewayDiagnose == diagnose->type()) {
        diagnose->doResult(0, 0); //no need ip check.
        return true;
    }
    if (eTypeAddressDiagnose != diagnose->type())
        return false;
    iface->startCheckIP();
    networkManager().refresh();
    return true;
}

bool
NativeHandlerNetworkDiagnoseC10::doNetworkDhcpError(int errcode, const char* ifname)
{
    NATIVEHANDLER_LOG("ErrorCode[%d], IfaceName[%s]\n", errcode, ifname);

    Message* msg = 0;
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return false;
    NetworkDiagnose::DiagnoseProcess* diagnose = networkDiagnose()->getDiagnose();
    if (!diagnose)
        return false;
    if (eTypeAddressDiagnose != diagnose->type())
        return false;
    diagnose->doResult(-1, errcode);
    return true;
}

bool
NativeHandlerNetworkDiagnoseC10::doNetworkPppoeError(int errcode, const char* ifname)
{
    NATIVEHANDLER_LOG("ErrorCode[%d], IfaceName[%s]\n", errcode, ifname);

    Message* msg = 0;
    NetworkInterface* iface = networkManager().getInterface(ifname);
    if (!iface)
        return false;
    NetworkDiagnose::DiagnoseProcess* diagnose = networkDiagnose()->getDiagnose();
    if (!diagnose)
        return false;
    if (eTypeAddressDiagnose != diagnose->type() 
        && eTypeGatewayDiagnose != diagnose->type())
        return false;
    diagnose->doResult(-1, errcode);
    return true;
}


} // namespace Hippo
