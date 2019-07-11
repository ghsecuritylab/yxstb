#include "NativeHandlerAssertions.h"
#include "NativeHandlerBootC10.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueNetwork.h"

#include "BrowserAgent.h"

#include "SystemManager.h"

#include <iostream>

#include "BootImagesShow.h"

#include "NetworkFunctions.h"

#include "sys_basic_macro.h"
#include "config.h"
#include "mid_fpanel.h"
#include "webpageConfig.h"

namespace Hippo
{

NativeHandlerBootC10::NativeHandlerBootC10()
{
}

NativeHandlerBootC10::~NativeHandlerBootC10()
{
}

void
NativeHandlerBootC10::onActive()
{
    Message *msg = NULL;

    BootImagesShowBootLogo(1);

    // Don't display network disconnect icon in the boot state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnect, 0);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnect, 0);
    epgBrowserAgent().sendMessage(msg);

    // Don't display network disconnect error code in the boot state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 0);
    epgBrowserAgent().sendMessage(msg);

    //Network mask set
    DONT_SET(NMB_SHOW_ERROR_CODE);
    DONT_SET(NMB_SEND_WEBPAGE_KEY);
}

void
NativeHandlerBootC10::onUnactive()
{
    Message *msg = NULL;
    /*由于认证图片的展示是在js调用中，此时清除bootlogo会有1S的黑屏*/
    //BootImagesShowBootLogo(0);

    // Display network disconnect icon when leave the boot state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnect, 1);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnect, 1);
    epgBrowserAgent().sendMessage(msg);

    // Display network disconnect error code when leave the boot state
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptDisconnectAlert, 1);
    epgBrowserAgent().sendMessage(msg);
    msg = epgBrowserAgent().obtainMessage(MessageType_Prompt, BrowserAgent::PromptWifiDisconnectAlert, 1);
    epgBrowserAgent().sendMessage(msg);

    //Stop reconnect
    // 【内部】【北京全播控】【有线PPPOE接入】不能按设置键中断PPPOE重连
    NetworkCard* device = networkManager().getActiveDevice();
    NetworkInterface* iface = 0;
    std::vector<std::string> ifnames = device->getIfnames();
    for (size_t i = 0; i < ifnames.size(); ++i) {
        iface = networkManager().getInterface(ifnames[i].c_str());
        if (iface && !iface->isActive()) {
            iface->disconnect();
            networkManager().refresh();
        }
    }
    //Network mask clear
    DONT_ZERO();
}

bool
NativeHandlerBootC10::doMenu()
{
    systemManager().destoryAllPlayer();
    return false;
}

bool
NativeHandlerBootC10::doShortCut(Message *msg)
{
    return false;
}

bool
NativeHandlerBootC10::onPowerOff()
{
    char devname[USER_LEN] = { 0 };
    network_default_devname(devname, USER_LEN);
    if(network_device_link_state(devname) == 0 || !m_isConnect)
        NativeHandlerPublicC10::onPowerOff();

    return true;
}

} // namespace Hippo
