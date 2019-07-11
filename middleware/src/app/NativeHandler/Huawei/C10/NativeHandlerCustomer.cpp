
#include "NativeHandlerCustomer.h"
#include "NativeHandlerBootC10.h"
#include "NativeHandlerConfigC10.h"
#include "NativeHandlerRunningC10.h"
#include "NativeHandlerStandbyC10.h"
#include "NativeHandlerUpgradeC10.h"
#include "NativeHandlerUConfigC10.h"
#include "NativeHandlerErrorC10.h"
#include "NativeHandlerLocalC10.h"
#include "NativeHandlerNetworkDiagnoseC10.h"
#if defined(INCLUDE_LITTLESYSTEM)
#include "NativeHandlerLittleSystem.h"
#endif
#include "NativeHandlerRecovery.h"
#include "NativeHandlerRelinkC10.h"

#include "UltraPlayer.h"

#include "Message.h"
#include "Session.h"
#include "app_sys.h"


namespace Hippo {

static NativeHandlerBootC10 handleBoot;
static NativeHandlerConfigC10 handleConfig;
static NativeHandlerRunningC10 handleRunning;
static NativeHandlerStandbyC10 handleStandby;
static NativeHandlerUpgradeC10 handleUpgrade;
#if defined(U_CONFIG)
static NativeHandlerUConfigC10 handleUConfig;
#endif
static NativeHandlerErrorC10 handleError;
static NativeHandlerLocalC10 handleLocal;
static NativeHandlerNetworkDiagnoseC10 handleNetworkDiagnose;
#if defined(INCLUDE_LITTLESYSTEM)
static NativeHandlerLittleSystem handleLittleSystem;
#endif
static NativeHandlerRecovery handleRecovery;
static NativeHandlerRelinkC10 handleRelink;


} // namespace Hippo


extern "C" {

void NativeHandlerCustomerInit()
{
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Boot, &Hippo::handleBoot);
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Config, &Hippo::handleConfig);
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Running, &Hippo::handleRunning);
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Standby, &Hippo::handleStandby);
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Upgrade, &Hippo::handleUpgrade);
#if defined(U_CONFIG)
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::UConfig, &Hippo::handleUConfig);
#endif
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Error, &Hippo::handleError);
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Local, &Hippo::handleLocal);
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::NetworkDiagnose, &Hippo::handleNetworkDiagnose);
#if defined(INCLUDE_LITTLESYSTEM)
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::LittleSystem, &Hippo::handleLittleSystem);
#endif
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Recovery, &Hippo::handleRecovery);
    Hippo::NativeHandler::registerStateHandler(Hippo::NativeHandler::Relink, &Hippo::handleRelink);

}

static int init = 0;

int NativeHandlerSetState(int state)
{
    if (!init) {
    	init = 1;
    	NativeHandlerCustomerInit();

#if defined(HUBEI_HD)
        Hippo::UltraPlayer::setUIFlagsForcedMask(Hippo::UltraPlayer::AudioTrack_Mask); //湖北要求显示，但是EPG又没有下发所以在此初始化
        Hippo::UltraPlayer::setUIFlagsForcedValue(Hippo::UltraPlayer::AudioTrack_Mask);
        Hippo::UltraPlayer::setUIFlagsForcedMask(Hippo::UltraPlayer::getUIFlagsForcedMask() | Hippo::UltraPlayer::AudioVolume_Mask);
        Hippo::UltraPlayer::setUIFlagsForcedValue(Hippo::UltraPlayer::getUIFlagsForcedValue() | Hippo::UltraPlayer::AudioVolume_Mask);
        Hippo::UltraPlayer::setUIFlagsForcedMask(Hippo::UltraPlayer::getUIFlagsForcedMask() | Hippo::UltraPlayer::AudioMute_Mask);
        Hippo::UltraPlayer::setUIFlagsForcedValue(Hippo::UltraPlayer::getUIFlagsForcedValue() | Hippo::UltraPlayer::AudioMute_Mask);
        if(PLATFORM_ZTE == session().getPlatform()) {
            Hippo::UltraPlayer::setUIFlagsForcedMask(Hippo::UltraPlayer::getUIFlagsForcedMask() | Hippo::UltraPlayer::PlayState_Mask);
            Hippo::UltraPlayer::setUIFlagsForcedValue(Hippo::UltraPlayer::getUIFlagsForcedValue() | Hippo::UltraPlayer::PlayState_Mask);
        }
#endif

#if defined(Liaoning) || defined( HEILONGJIANG_SD )
        Hippo::UltraPlayer::setUIFlagsForcedMask(Hippo::UltraPlayer::getUIFlagsForcedMask() | Hippo::UltraPlayer::PlayState_Mask);
        Hippo::UltraPlayer::setUIFlagsForcedValue(Hippo::UltraPlayer::getUIFlagsForcedValue() | Hippo::UltraPlayer::PlayState_Mask);
#endif
       // return Hippo::defNativeHandler().setState(Hippo::NativeHandler::Boot);
    }
        return Hippo::defNativeHandler().setState((Hippo::NativeHandler::State)state);
}

int NativeHandlerGetState()
{
    return (int)Hippo::defNativeHandler().getState();
}

int NativeHandlerGetOldState() //TODO Can meld with C10 and C20 for code reuse
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
