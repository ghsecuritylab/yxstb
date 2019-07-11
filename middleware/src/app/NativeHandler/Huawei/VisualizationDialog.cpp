
#include "VisualizationDialog.h"
#include "MaintenancePageWidget.h"

#include "Message.h"
#include "MessageTypes.h"

#include "KeyDispatcher.h"
#include "NativeHandlerPublic.h"
#include "NativeHandlerAssertions.h"
#include "MessageValueMaintenancePage.h"
#include "BrowserAgent.h"

#include "SystemManager.h"
#include "LayerMixerDevice.h"
#include "browser_event.h"
#include "config.h"
#include "AppSetting.h"

#include "mid_sys.h"
#include "sys_msg.h"

#include "mgmtModule.h"
#include "mgmtModuleParam.h"
#ifndef ANDROID
extern int sendMsgToMonitorLinux(char *name, char *str, unsigned int str_len);
#endif


#if (SUPPORTE_HD)
#define VISUALIZATION_POS_X (((1280-878)/2)&0xfffe)
#define VISUALIZATION_POS_Y (((720-563)/2)&0xfffe)
#else
#define VISUALIZATION_POS_X (((720-641)/2)&0xfffe)
#define VISUALIZATION_POS_Y (((576-449)/2)&0xfffe)
#endif

namespace Hippo {

#if (SUPPORTE_HD)
static WidgetSource gVisualizationInfoSource = {Hippo::StandardScreen::S720, 0, 0, 1280, 720, 0, 0, 0};
static WidgetSource gMaintenancePageSource = {Hippo::StandardScreen::S720, VISUALIZATION_POS_X, VISUALIZATION_POS_Y, 878, 563, 0, 0, 0};
#else
static WidgetSource gVisualizationInfoSource = {Hippo::StandardScreen::S576, 0, 0, 720, 576, 0, 0, 0};
static WidgetSource gMaintenancePageSource = {Hippo::StandardScreen::S576, VISUALIZATION_POS_X, VISUALIZATION_POS_Y, 641, 449, 0, 0, 0};
#endif

VisualizationDialog::VisualizationDialog()
	: m_handle(NULL)
	, mCurrentPage(1)
	, mCurrentFocusPos(0)
	, isOpenMainPage(false)
	, isOpenChildPage(false)

{
    mVisualizationInfo = new VisualizationInfoWidget(&gVisualizationInfoSource);
    mMaintenancePage = new MaintenancePageWidget(&gMaintenancePageSource);

    LayerMixerDevice::Layer* layer = Hippo::systemManager().mixer().topLayer();
    layer->attachChildToBack(mMaintenancePage);
    layer->attachChildToBack(mVisualizationInfo);
}

VisualizationDialog::~VisualizationDialog()
{
    mVisualizationInfo->detachFromParent();
    mMaintenancePage->detachFromParent();
    if (mVisualizationInfo)
        delete mVisualizationInfo;
    if (mMaintenancePage)
        delete mMaintenancePage;
}

void
VisualizationDialog::setHandler(NativeHandlerPublic *handler)
{
    m_handle = handler;
}


bool
VisualizationDialog::handleMessage(Message *msg)
{
    if(MessageType_Repaint != msg->what)
        NATIVEHANDLER_LOG("what(0x%x),message(0x%x/0x%x)\n", msg->what, msg->arg1, msg->arg2);


    if (MessageType_KeyDown == msg->what) {
        switch(msg->arg1){
        case MV_Maintenance_refreshVisualizationInfo:
            if (StreamInfoPage == mCurrentPage || OTTInfoPage == mCurrentPage)
                doRefreshVisualizationInfo();
            return true;
        case MV_Maintenance_stopCollectDebugInfo:
            return doStopDebug();
        case EIS_IRKEY_UP:
            return doPressUP();
        case EIS_IRKEY_DOWN:
            return doPressDown();
        case EIS_IRKEY_BACK:
            return doPressGoBack();
        case EIS_IRKEY_SELECT:
            return doPressSelect();
        case EIS_IRKEY_POWER:
        case EIS_IRKEY_VK_F10:
            mVisualizationInfo->setVisibleP(false);
            mMaintenancePage->setVisibleP(false);
            return false;
        case EIS_IRKEY_LEFT:
        case EIS_IRKEY_RIGHT:
            return true;
        default:
            return false;
        }
    }

    return true;
}

bool
VisualizationDialog::doPressUP()
{
    switch(mCurrentPage) {
    case MainPage:
        switch(mCurrentFocusPos) {
        case MainPage_CollectSTBDebugInfo_Line:
            return true;
        case MainPage_AutoCollectSTBDebugInfo_Line:
            mCurrentFocusPos = MainPage_CollectSTBDebugInfo_Line;
            break;
        case MainPage_ShowStreamMediaInfo_Line:
            mCurrentFocusPos = MainPage_AutoCollectSTBDebugInfo_Line;
            break;
        case MainPage_ShowOTTDebugInfo_Line:
            mCurrentFocusPos = MainPage_ShowStreamMediaInfo_Line;
            break;
        default:
            break;
        }
        mMaintenancePage->setCurrentFocusPos(mCurrentFocusPos);
        mMaintenancePage->inval(NULL);
        break;
    case DebugPage:
        if (DebugPage_Start_line == mCurrentFocusPos)
            return true;
        else if (DebugPage_Stop_line == mCurrentFocusPos)
            mCurrentFocusPos = DebugPage_Start_line;
        mMaintenancePage->setCurrentFocusPos(mCurrentFocusPos);
        mMaintenancePage->inval(NULL);
        break;
    default:
        break;
    }

    return true;
}

bool
VisualizationDialog::doPressDown()
{
    switch(mCurrentPage) {
    case MainPage:
        switch(mCurrentFocusPos) {
        case MainPage_CollectSTBDebugInfo_Line:
            mCurrentFocusPos = MainPage_AutoCollectSTBDebugInfo_Line;
            break;
        case MainPage_AutoCollectSTBDebugInfo_Line:
            mCurrentFocusPos = MainPage_ShowStreamMediaInfo_Line;
            break;
        case MainPage_ShowStreamMediaInfo_Line:
            mCurrentFocusPos = MainPage_ShowOTTDebugInfo_Line;
            break;
        case MainPage_ShowOTTDebugInfo_Line:
            return true;
        default:
            break;
        }
        mMaintenancePage->setCurrentFocusPos(mCurrentFocusPos);
        mMaintenancePage->inval(NULL);
        break;
    case DebugPage:
        if (DebugPage_Start_line == mCurrentFocusPos)
            mCurrentFocusPos = DebugPage_Stop_line;
        else if (DebugPage_Stop_line == mCurrentFocusPos)
            return true;
        mMaintenancePage->setCurrentFocusPos(mCurrentFocusPos);
        mMaintenancePage->inval(NULL);
        break;
    default:
        break;
    }

    return true;
}

bool
VisualizationDialog::doPressSelect()
{
    switch(mCurrentPage) {
    case MainPage:
        switch(mCurrentFocusPos) {
        case MainPage_CollectSTBDebugInfo_Line:
            mCurrentPage = DebugPage;
            mCurrentFocusPos = DebugPage_Start_line;
            mMaintenancePage->setCurrentFocusPos(mCurrentFocusPos);
            mMaintenancePage->setNextShowPage(mCurrentPage);
            mMaintenancePage->inval(NULL);
            break;
        case MainPage_AutoCollectSTBDebugInfo_Line:
            mCurrentPage = AutoDebugPage;
            mMaintenancePage->setNextShowPage(mCurrentPage);
            mMaintenancePage->inval(NULL);
            break;
        case MainPage_ShowStreamMediaInfo_Line:
            mMaintenancePage->setVisibleP(false);
            mCurrentPage = StreamInfoPage;
            mid_sys_setCollectInfoType(InfoTYpe_stream_information);
            mVisualizationInfo->setNextShowPage(mCurrentPage);
            mVisualizationInfo->setVisibleP(true);
            doRefreshVisualizationInfo();
            break;
        case MainPage_ShowOTTDebugInfo_Line:
            mMaintenancePage->setVisibleP(false);
            mCurrentPage = OTTInfoPage;
            mid_sys_setCollectInfoType(InfoTYpe_OTT_information);
            mVisualizationInfo->setNextShowPage(mCurrentPage);
            mVisualizationInfo->setVisibleP(true);
            doRefreshVisualizationInfo();
            break;
        }
        isOpenMainPage = false;
        isOpenChildPage = true;
        break;
    case DebugPage:
        if (DebugPage_Start_line == mCurrentFocusPos) {
            if (!getMgmtDebugInfoStatus()) //getDebugInfoStatus: return. 0: start  -1:stop  1:pause  2:uploading
                return true;

            mid_sys_setStartCollectDebugInfo(1);
#ifdef ANDROID
            if (getMonitorUDiskStatus() != 1) {
                if (getMonitorUDiskStatus() == 2)
                    sendMessageToEPGBrowser(MessageType_Prompt, Hippo::BrowserAgent::PromptVisualizationDebug, 1, 0);
                else {
                    mid_sys_setStartCollectDebugInfo(0);
                    mCurrentFocusPos = DebugPage_Stop_line;
                    mMaintenancePage->setCurrentFocusPos(mCurrentFocusPos);
                }

            } else {
                iptvSendMsgToMonitor("DebugInfo", "start", 0);
                sendMessageToEPGBrowser(MessageType_Prompt, Hippo::BrowserAgent::PromptVisualizationDebug, 1, 0);
            }
#else
            // if (-1 == stb_startDebugInfo_ioctl(NULL, 0)) { // 不用老monitortool后删掉
            if (-1 == sendMsgToMonitorLinux("DebugInfo", "start", 0)) {
                mid_sys_setStartCollectDebugInfo(0);
                mCurrentFocusPos = DebugPage_Stop_line;
                mMaintenancePage->setCurrentFocusPos(mCurrentFocusPos);
            } else
                sendMessageToEPGBrowser(MessageType_Prompt, Hippo::BrowserAgent::PromptVisualizationDebug, 1, 0);
#endif
		} else if (DebugPage_Stop_line == mCurrentFocusPos) {
            mid_sys_setStartCollectDebugInfo(0);
#ifdef ANDROID
            iptvSendMsgToMonitor("DebugInfo", "stop", 0);
#else
            // stb_stopDebugInfo_ioctl(NULL, 0); // 不用老monitortool后删掉
            sendMsgToMonitorLinux("DebugInfo", "stop", 0);
#endif
            sendMessageToEPGBrowser(MessageType_Prompt, Hippo::BrowserAgent::PromptVisualizationDebug, 0, 0);
        }
        mMaintenancePage->inval(NULL);
        break;
    case AutoDebugPage: {
        isOpenMainPage = true;
        isOpenChildPage = false;
        int StartupCaptured = 0;
        #ifdef ANDROID
		iptvSendMsgToMonitor("StartupCaptured", "2", 0);
		#else
        appSettingGetInt("StartupCaptured", &StartupCaptured, 0);
        if ( 2 != StartupCaptured)
            appSettingSetInt("StartupCaptured", 2);
        #endif
        mCurrentPage = MainPage;
        mCurrentFocusPos = MainPage_CollectSTBDebugInfo_Line;
        mMaintenancePage->setCurrentFocusPos(mCurrentFocusPos);
        mMaintenancePage->setNextShowPage(mCurrentPage);
        mMaintenancePage->inval(NULL);
        break;
    }
    default:
        break;
    }
    return true;
}

bool
VisualizationDialog::doPressGoBack()
{
    switch(mCurrentPage) {
    case MainPage:
        mMaintenancePage->setVisibleP(false);
        mVisualizationInfo->setVisibleP(false);
        sendMessageToNativeHandler(MessageType_KeyDown, MV_Maintenance_openMaintenancePage_clear, 0, 0);
        return true;
    case DebugPage:
    case AutoDebugPage:
    case StreamInfoPage:
    case OTTInfoPage:
        mCurrentPage = MainPage;
        mCurrentFocusPos = MainPage_CollectSTBDebugInfo_Line;
        mMaintenancePage->setCurrentFocusPos(mCurrentFocusPos);
        mMaintenancePage->setNextShowPage(mCurrentPage);
        mVisualizationInfo->setNextShowPage(NULLBGPage);
        mVisualizationInfo->inval(NULL);
        mMaintenancePage->setVisibleP(true);
        break;
    default:
        break;
    }
    isOpenMainPage = true;
    isOpenChildPage = false;
    return true;
}

bool
VisualizationDialog::doStopDebug()
{
    mid_sys_setStartCollectDebugInfo(0);

    if (DebugPage == mCurrentPage && DebugPage_Start_line == mCurrentFocusPos) {
        mCurrentFocusPos = DebugPage_Stop_line;
        mMaintenancePage->setCurrentFocusPos(mCurrentFocusPos);
    }

    mMaintenancePage->inval(NULL);
    sendMessageToEPGBrowser(MessageType_Prompt, Hippo::BrowserAgent::PromptVisualizationDebug, 0, 0);
    return true;
}

bool
VisualizationDialog::doRefreshVisualizationInfo()
{
    if (mVisualizationInfo)
        mVisualizationInfo->inval(NULL);

    sendMessageToNativeHandler(MessageType_KeyDown, MV_Maintenance_refreshVisualizationInfo, 0, 2000);
    return true;
}

void
VisualizationDialog::draw()
{
    if (!isOpenMainPage || isOpenChildPage) {
        isOpenMainPage = true;
        isOpenChildPage = false;
        mCurrentPage = MainPage;
        mCurrentFocusPos = MainPage_CollectSTBDebugInfo_Line;
        mMaintenancePage->setCurrentFocusPos(mCurrentFocusPos);
        mMaintenancePage->setNextShowPage(MainPage);
        mVisualizationInfo->setNextShowPage(NULLBGPage);
        mVisualizationInfo->setVisibleP(true);
        mVisualizationInfo->inval(NULL);
        mMaintenancePage->setVisibleP(true);
        mMaintenancePage->inval(NULL);
    } else {
        mVisualizationInfo->setVisibleP(false);
        mMaintenancePage->setVisibleP(false);
        sendMessageToNativeHandler(MessageType_KeyDown, MV_Maintenance_openMaintenancePage_clear, 0, 0);
    }
}

} // namespace Hippo
