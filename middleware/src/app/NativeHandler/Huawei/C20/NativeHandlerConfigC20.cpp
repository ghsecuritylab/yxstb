
#include "NativeHandlerConfigC20.h"
#include "NativeHandlerAssertions.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "BrowserAgent.h"

#include <iostream>

#include <hippo_module_config.h>
#include <Hippo_OS.h>
#include "Hippo_Context.h"

#include "sys_key_deal.h"
#include "config/webpageConfig.h"


namespace Hippo {

NativeHandlerConfigC20::NativeHandlerConfigC20()
{
}

NativeHandlerConfigC20::~NativeHandlerConfigC20()
{
}

void
NativeHandlerConfigC20::onActive()
{
    DONT_SET(NMB_SHOW_ERROR_CODE);
    //DONT_SET(NMB_SHOW_BOTTOM_ICON);
    //DONT_SET(NMB_CONNECTOK_DOMENU);
    DONT_SET(NMB_REJOIN_WIRELESS);
}

void
NativeHandlerConfigC20::onUnactive()
{
    DONT_ZERO();
}

bool
NativeHandlerConfigC20::handleMessage(Message *msg)
{
#ifdef Huawei_v5
    switch (msg->arg1 ) {
    case EIS_IRKEY_MENU:
        NATIVEHANDLER_LOG("not deal with EIS_IRKEY_MENU in setting page\n");
        return true;
    default:
        break;
    }
#endif //Huawei_v5
    NATIVEHANDLER_LOG("NativeHandlerConfigC20::handleMessage what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);
    return NativeHandlerPublicC20::handleMessage(msg);
}

} // namespace Hippo
