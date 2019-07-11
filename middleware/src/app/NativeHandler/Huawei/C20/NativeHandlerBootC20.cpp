
#include "NativeHandlerBootC20.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueNetwork.h"

#include "BrowserAgent.h"
#include "SystemManager.h"

#include <iostream>

#include "config.h"
#include "config/webpageConfig.h"
#include "BootImagesShow.h"

namespace Hippo {

NativeHandlerBootC20::NativeHandlerBootC20()
{
}

NativeHandlerBootC20::~NativeHandlerBootC20()
{
}

void
NativeHandlerBootC20::onActive()
{
    DONT_SET(NMB_SHOW_ERROR_CODE);
    //DONT_SET(NMB_SHOW_BOTTOM_ICON);
    //DONT_SET(NMB_CONNECTOK_DOMENU);
    DONT_SET(NMB_REJOIN_WIRELESS);
    BootImagesShowBootLogo(1);
}

void
NativeHandlerBootC20::onUnactive()
{
    BootImagesShowBootLogo(0);
    DONT_ZERO();
}


bool
NativeHandlerBootC20::doShortCut(Message *msg)
{
    return false;
}

bool
NativeHandlerBootC20::doConfig(Message * msg)
{
    std::string mid_url = "";

    mid_url = LOCAL_WEBPAGE_PATH_PREFIX LOCAL_WEBPAGE_PATH_CONFIG;

    systemManager().destoryAllPlayer();

    epgBrowserAgent().openUrl(mid_url.c_str());
    return true;
}

} // namespace Hippo
