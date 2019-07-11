
#include "NativeHandler.h"
#include "NativeHandlerPublic.h"
#include "NativeHandlerAssertions.h"

#include "Message.h"
#include "MessageTypes.h"
#include "BrowserAgent.h"


namespace Hippo {

/**
 * 将未处理消息转给浏览器
 */
void 
NativeHandler::handleMessage(Message *msg)
{
    if (msg->what == MessageType_KeyDown || msg->what == MessageType_Unknow || msg->what == MessageType_Upgrade)
        epgBrowserAgent().handleMessage(msg);
}


static NativeHandler::Callback *gCallbacks[NativeHandler::Recovery + 1] = {0};

int 
NativeHandler::registerStateHandler(State state, Callback *callback)
{
    if (state > Recovery)
        return -1;

    gCallbacks[state] = callback;
    return 0;
}

/**
 * 根据状态设置默认处理程序，如默认处理程序返回false，消息
 * 由下面handleMessage(Message *msg)处理
 */
int 
NativeHandler::setState(State state)
{
    if (state > Recovery)
        return -1;
    
    oldState = state;
    if (mCallback) {
        NativeHandlerPublic *handler = (NativeHandlerPublic *)mCallback;
        if (state == handler->state()) /*和当前状态一致就return*/
            return 0;
        handler->onUnactive();
        oldState = handler->state();
    }

    NATIVEHANDLER_LOG("Current stb state(%d)\n", state);
    mCallback = gCallbacks[state];

    if(mCallback) {
        NativeHandlerPublic *handler = (NativeHandlerPublic *)mCallback;
        handler->onActive();
    } else {
		return -1;
	}
    return 0;
}

NativeHandler::State 
NativeHandler::getState()
{
    NativeHandlerPublic *handler = (NativeHandlerPublic *)mCallback;
    if (handler)
        return handler->state();
    else
        return Boot;
}

NativeHandler::State 
NativeHandler::getOldState()
{
    return oldState;
}

static NativeHandler *gNativeHandler = NULL;

NativeHandler &defNativeHandler()
{
    return *gNativeHandler;
}

} // namespace Hippo


extern "C" void 
defNativeHandlerCreate()
{
    Hippo::gNativeHandler = new Hippo::NativeHandler();
}

extern "C" void 
sendMessageToNativeHandler(int what, int arg1, int arg2, unsigned int pDelayMillis)
{
    Hippo::Message *msg = Hippo::defNativeHandler().obtainMessage(what, arg1, arg2);

    if(pDelayMillis > 0)
        Hippo::defNativeHandler().sendMessageDelayed(msg, pDelayMillis);
    else
        Hippo::defNativeHandler().sendMessage(msg);
}

