
#include "NativeHandlerCustomer.h"

#include "UltraPlayer.h"
#include "Message.h"
#include "NativeHandlerBootC20.h"
#include "NativeHandlerConfigC20.h"
#include "NativeHandlerRunningC20.h"
#include "NativeHandlerStandbyC20.h"
#include "NativeHandlerUpgradeC20.h"
#include "NativeHandlerUConfigC20.h"
#include "NativeHandlerErrorC20.h"
#if defined(INCLUDE_LITTLESYSTEM)
#include "NativeHandlerLittleSystem.h"
#endif


namespace Hippo {

static NativeHandlerBootC20 handleBoot;
static NativeHandlerConfigC20 handleConfig;
static NativeHandlerRunningC20 handleRunning;
static NativeHandlerStandbyC20 handleStandby;
static NativeHandlerUpgradeC20 handleUpgrade;
static NativeHandlerUConfigC20 handleUConfig;
static NativeHandlerErrorC20 handleError;
#if defined(INCLUDE_LITTLESYSTEM)
static NativeHandlerLittleSystem handleLittleSystem;
#endif


} // namespace Hippo


extern "C" {

void NativeHandlerCustomerInit()
{
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Boot, &Hippo::handleBoot);
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Config, &Hippo::handleConfig);
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Running, &Hippo::handleRunning);
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Standby, &Hippo::handleStandby);
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Upgrade, &Hippo::handleUpgrade);
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::UConfig, &Hippo::handleUConfig);
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Error, &Hippo::handleError);
#if defined(INCLUDE_LITTLESYSTEM)
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::LittleSystem, &Hippo::handleLittleSystem);	
#endif
}

static int init = 0;

int NativeHandlerSetState(int state)
{    
    if (!init) {
    	init = 1;
    	NativeHandlerCustomerInit();
    	Hippo::defNativeHandler().setState(Hippo::NativeHandler::Boot);
    }
    return Hippo::defNativeHandler().setState((Hippo::NativeHandler::State)state);
}

int NativeHandlerGetState()
{
    return (int)Hippo::defNativeHandler().getState();
}

int NativeHandlerGetOldState()
{
    return (int)Hippo::defNativeHandler().getOldState();
}

int NativeHandlerInputMessage(int what, int arg1, int arg2, void *)
{
    if (!init) {
    	init = 1;
    	NativeHandlerCustomerInit();
    	Hippo::defNativeHandler().setState(Hippo::NativeHandler::Running);
    }

    Hippo::Message msg;
    msg.what = what;
    msg.arg1 = arg1;
    msg.arg2 = arg2;
    msg.obj = 0;
    msg.target = &Hippo::defNativeHandler();
    msg.callback = 0;

    Hippo::defNativeHandler().dispatchMessage(&msg);
    return 0;
}

}
