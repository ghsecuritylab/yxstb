
#include "NativeHandlerErrorC20.h"
#include "NativeHandlerAssertions.h"

namespace Hippo {

NativeHandlerErrorC20::NativeHandlerErrorC20()
{
}

NativeHandlerErrorC20::~NativeHandlerErrorC20()
{
}

void
NativeHandlerErrorC20::onActive()
{
    DONT_SET(NMB_SHOW_ERROR_CODE);
    //DONT_SET(NMB_SHOW_BOTTOM_ICON);
    //DONT_SET(NMB_CONNECTOK_DOMENU);
    DONT_SET(NMB_REJOIN_WIRELESS);
}

void
NativeHandlerErrorC20::onUnactive()
{
    DONT_ZERO();
}


bool 
NativeHandlerErrorC20::handleMessage(Message *msg)
{
    NATIVEHANDLER_LOG("NativeHandlerErrorC20::handleMessage: what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);
    return NativeHandlerPublicC20::handleMessage(msg);
}

bool 
NativeHandlerErrorC20::doShortCut(Message *)
{
    return false;
}

} // namespace Hippo
