
#include "BrowserAgent.h"

#include "Message.h"
#include "MessageTypes.h"

#include "browser_event.h"

#include "Hippo_api.h"

#include "SysSetting.h"
#include "SettingEnum.h"

namespace Hippo {

PromptWidget* BrowserAgent::mPrompt = NULL;
PicSetWidget* BrowserAgent::mWaitClock = NULL;
int BrowserAgent::NtpFailedShowed = 0;

BrowserAgent::BrowserAgent()
	: mView(NULL)
{
}

BrowserAgent::~BrowserAgent()
{
}

void
BrowserAgent::handleMessage(Message *msg)
{
    if (msg->what == MessageType_Prompt && mPrompt) {
        switch (msg->arg1) {
            case PromptDebug:{
                if(msg->arg2)
                    mPrompt->showDebug();
                else
                    mPrompt->clearDebug();
                break;
            }
            case PromptWifiDisconnect:{
#ifdef INCLUDE_WIFI
                int netType = 0;
                sysSettingGetInt("nettype", &netType, 0);
                if(netType == NET_WIRELESS){
                    if(msg->arg2)
                        mPrompt->showWifiDisconnect();
                    else
                        mPrompt->clearWifiDisconnect();
                }
#endif
                break;
            }
            case PromptWifiDisconnectAlert: {
#ifdef INCLUDE_WIFI
                int netType = 0;

                sysSettingGetInt("nettype", &netType, 0);
                if(NET_WIRELESS == netType){
                    if(msg->arg2)
                        mPrompt->showWifiDisconnectAlert();
                    else
                        mPrompt->clearWifiDisconnectAlert();
                }
#endif
                break;
            }
            case PromptWifiNoSignal: {
#ifdef INCLUDE_WIFI
                int netType = 0;
                sysSettingGetInt("nettype", &netType, 0);
                if(netType == NET_WIRELESS){
                    if(msg->arg2)
                        mPrompt->showWifiNoSignal();
                    else
                        mPrompt->clearWifiNoSignal();
                }
#endif
                break;
            }
            case PromptWifiLowQuality: {
#ifdef INCLUDE_WIFI
                int netType = 0;
                sysSettingGetInt("nettype", &netType, 0);
                if(netType == NET_WIRELESS){
                    switch(msg->arg2) {
                        case 0:
                            mPrompt->clearWifiWeakSignal1();
                            mPrompt->clearWifiWeakSignal2();
                            break;
                        case 1: // the low quality gif1
                            mPrompt->clearWifiWeakSignal2();
                            mPrompt->showWifiWeakSignal1();
                            break;
                        case 2: // the low quality gif2.
                            mPrompt->clearWifiWeakSignal1();
                            mPrompt->showWifiWeakSignal2();
                            break;
                        default:
                            mPrompt->clearWifiWeakSignal1();
                            mPrompt->clearWifiWeakSignal2();
                            break;
                    }
                }
#endif
                break;
            }
            case PromptDisconnect:{
				int netType = 0;
                sysSettingGetInt("nettype", &netType, 0);
                if(NET_WIRELESS != netType){
                    if(msg->arg2){
                        mPrompt->clearDuplicate();
                        mPrompt->showDisconnect();
                    }
                    else
                        mPrompt->clearDisconnect();
                }
                break;
            }
            case PromptDisconnectAlert: {
                int netType = 0;
                sysSettingGetInt("nettype", &netType, 0);
                if(NET_WIRELESS != netType){
                    if(msg->arg2)
                        mPrompt->showDisconnectAlert();
                    else
                        mPrompt->clearDisconnectAlert();
                }
                break;
            }
            case PromptPress:{
                if(msg->arg2)
                    mPrompt->showPress();
                else
                    mPrompt->clearPress();
                break;
            }
            case PromptDuplicate:{
                if(msg->arg2)
                    mPrompt->showDuplicate();
                else
                    mPrompt->clearDuplicate();
                break;
            }
            case PromptErrorCode10030:{
                if(msg->arg2)
                    mPrompt->showErrorCode10030();
                else
                    mPrompt->clearErrorCode10030();
                break;
            }
            case PromptErrorCode10031:{
                if(msg->arg2)
                    mPrompt->showErrorCode10031();
                else
                    mPrompt->clearErrorCode10031();
                break;
            }
            case PromptDhcpFailed:{
#ifdef VIETTEL_HD
                if(msg->arg2)
                    mPrompt->showDhcpFailedIcon();
                else
                    mPrompt->clearDhcpFailedIcon();
#endif
                break;
            }
            case PromptClearAll:
                mPrompt->clearDebug();
                mPrompt->clearPress();
            case PromptClearNetwork: {
#ifdef INCLUDE_WIFI
                mPrompt->clearWifiDisconnect();
                mPrompt->clearWifiDisconnectAlert();
                mPrompt->clearWifiNoSignal();
                mPrompt->clearWifiWeakSignal1();
                mPrompt->clearWifiWeakSignal2();
#endif
                mPrompt->clearDisconnect();
                mPrompt->clearDisconnectAlert();
                mPrompt->clearDuplicate();

                mPrompt->clearErrorCode10030();
                mPrompt->clearErrorCode10031();
                mPrompt->clearDhcpFailedIcon();
                break;
            }
            case PromptVisualizationDebug: {
                if(msg->arg2)
                    mPrompt->showVisualizationDebug();
                else
                    mPrompt->clearVisualizationDebug();
                break;
            }
            case PromptUsbInfo:
                if (msg->arg2)
                    mPrompt->showUsbInfo();
                else
                    mPrompt->clearUsbInfo();
                break;
            default:{
                break;
            }
        }
    }
    return ;
}

} // namespace Hippo


extern "C" void
sendMessageToEPGBrowser(int what, int arg1, int arg2, unsigned int pDelayMillis)
{
    Hippo::Message *msg = Hippo::epgBrowserAgent().obtainMessage(what, arg1, arg2);

    if(pDelayMillis){
        Hippo::epgBrowserAgent().sendMessageDelayed(msg, pDelayMillis);
    }
    else{
        Hippo::epgBrowserAgent().sendMessage(msg);
    }
}

extern "C" void
setUsbNoInsertMessage(char *msg)
{
    std::string message = msg;
    if(Hippo::BrowserAgent::mPrompt)
        Hippo::BrowserAgent::mPrompt->setUsbMessage(message);

    Hippo::epgBrowserAgent().removeMessages(MessageType_Prompt, Hippo::BrowserAgent::PromptUsbInfo, 0);
    sendMessageToEPGBrowser(MessageType_Prompt, Hippo::BrowserAgent::PromptUsbInfo, 1, 0);
    sendMessageToEPGBrowser(MessageType_Prompt, Hippo::BrowserAgent::PromptUsbInfo, 0, 3000);
}

