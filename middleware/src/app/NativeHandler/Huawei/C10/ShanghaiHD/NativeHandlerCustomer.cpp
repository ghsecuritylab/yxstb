
#include "app_graph.h"

#include "NativeHandlerCustomer.h"

#include "UltraPlayer.h"
#include "Message.h"
#include "NativeHandlerBootC10.h"
#include "NativeHandlerConfigC10.h"
#include "NativeHandlerRunningC10.h"
#include "NativeHandlerStandbyC10.h"
#include "NativeHandlerUpgradeC10.h"
#include "NativeHandlerUConfigC10.h"
#include "NativeHandlerErrorC10.h"


namespace Hippo {

static NativeHandlerBootC10 handleBoot;
static NativeHandlerConfigC10 handleConfig;
static NativeHandlerRunningC10 handleRunning;
static NativeHandlerStandbyC10 handleStandby;
static NativeHandlerUpgrade handleUpgrade;
static NativeHandlerUConfig handleUConfig;
static NativeHandlerErrorC10 handleError;

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
