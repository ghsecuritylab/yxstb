
#include "config.h"
#include "Dialog.h"

#include "MessageTypes.h"
#include "NativeHandlerPublic.h"

namespace Hippo {

Dialog::Dialog()
{
}

Dialog::~Dialog()
{
    if (NativeHandlerPublic::mDialog == this)
        NativeHandlerPublic::mDialog = 0;
    if (NativeHandlerPublic::mProgressBarDialog == this)
        NativeHandlerPublic::mProgressBarDialog = 0;	
}

bool 
Dialog::handleMessage(Message *msg)
{
    return false;
}

void 
Dialog::draw()
{
}

void
Dialog::Close()
{
    sendMessageToNativeHandler(MessageType_System, HM_CLOSEDIALOG, 0, 0);
}

bool
Dialog::onClose()
{
    return true;
}


} // namespace Hippo
