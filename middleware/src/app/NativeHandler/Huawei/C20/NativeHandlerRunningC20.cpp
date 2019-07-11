
#include "NativeHandlerRunningC20.h"
#include "NativeHandlerAssertions.h"

namespace Hippo {

NativeHandlerRunningC20::NativeHandlerRunningC20()
{
}

NativeHandlerRunningC20::~NativeHandlerRunningC20()
{
}

void
NativeHandlerRunningC20::onActive()
{
    DONT_SET(NMB_SHOW_ERROR_CODE);
    //DONT_SET(NMB_SHOW_BOTTOM_ICON);
    //DONT_SET(NMB_CONNECTOK_DOMENU);
    DONT_SET(NMB_REJOIN_WIRELESS);
}

void
NativeHandlerRunningC20::onUnactive()
{
    DONT_ZERO();
}


bool 
NativeHandlerRunningC20::handleMessage(Message *msg)
{
    return NativeHandlerPublicC20::handleMessage(msg);
}

} // namespace Hippo
