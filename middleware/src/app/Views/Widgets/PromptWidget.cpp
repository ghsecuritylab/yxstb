
#include "Canvas.h"

#include "NetworkFunctions.h"
#include "SystemManager.h"
#include "PromptWidget.h"
#include "sys_basic_macro.h"
#include "NetworkFunctions.h"

//wifi 
#ifdef INCLUDE_WIFI
#define PNG_WIFI_DISCONNECT_ICON_PATH            SYS_IMG_PATH_ROOT"/network/wireless_disconnect.png"
#if(SUPPORTE_HD)
#define PNG_WIFI_ERRORCODE_10060_ALERT_HD_PATH   SYS_IMG_PATH_ROOT"/errorcode/network/errorcode_10060_HD.png"
#else
#define PNG_WIFI_ERRORCODE_10060_ALERT_SD_PATH   SYS_IMG_PATH_ROOT"/errorcode/network/errorcode_10060_SD.png"
#endif
#define GIF_WIFI_NOSIGNAL_ICON_PATH              SYS_IMG_PATH_ROOT"/network/wireless_no_signal.gif"
#define GIF_WIFI_WEEKSIGNAL_1_ICON_PATH          SYS_IMG_PATH_ROOT"/network/wireless_weak_signal_1.gif"
#define GIF_WIFI_WEEKSIGNAL_2_ICON_PATH          SYS_IMG_PATH_ROOT"/network/wireless_weak_signal_2.gif"
#define GIF_WIFI_STRONGSIGNAL_ICON_PATH          SYS_IMG_PATH_ROOT"/network/wireless_strong_signal.gif"
#endif

//press
#define GIF_PRESS_ICON_PATH   SYS_IMG_PATH_ROOT"/press/press.gif"

//ntp errcode 
#if(SUPPORTE_HD)
#define PNG_ERRORCODE_10030_ICON_PATH    SYS_IMG_PATH_ROOT"/errorcode/ntp/errorcode10030_HD.png"
#define PNG_ERRORCODE_10031_ICON_PATH    SYS_IMG_PATH_ROOT"/errorcode/ntp/errorcode10031_HD.png"
#else
#define PNG_ERRORCODE_10030_ICON_PATH    SYS_IMG_PATH_ROOT"/errorcode/ntp/errorcode10030_SD.png"
#define PNG_ERRORCODE_10031_ICON_PATH    SYS_IMG_PATH_ROOT"/errorcode/ntp/errorcode10031_SD.png"
#endif

//network
#if defined(VIETTEL_HD) || defined(Cameroon_v5)
#define GIF_DISCONNECT_VIETTEL_ICON_PATH     SYS_IMG_PATH_ROOT"/network/network_disconnect_viettel.gif"
#define GIF_DUPLICATE_VIETTEL_ICON_PATH      SYS_IMG_PATH_ROOT"/network/network_duplicate_viettel.gif"
#define GIF_DHCP_VIETTEL_ICON_PATH           SYS_IMG_PATH_ROOT"/network/network_dhcp_viettel.gif"
#else
#if(SUPPORTE_HD)
#define PNG_NETWORK_ERRORCODE_10000_ALERT_HD_PATH     SYS_IMG_PATH_ROOT"/errorcode/network/errorcode_10000_HD.png"
#else
#define PNG_NETWORK_ERRORCODE_10000_ALERT_SD_PATH     SYS_IMG_PATH_ROOT"/errorcode/network/errorcode_10000_SD.png"
#endif
#define PNG_NETWORK_DISCONNECT_ICON_PATH           SYS_IMG_PATH_ROOT"/network/network_disconnect.png"
#define GIF_NETWORK_DUPLICATE_ICON_PATH            SYS_IMG_PATH_ROOT"/network/network_duplicate.gif"
#endif

//debug
#if(SUPPORTE_HD)
#define PNG_DEBUG_VISUALIZATION_ICON_HD_PATH       SYS_IMG_PATH_ROOT"/debug/VisualizationDebug_HD.png" 
#define PNG_DEBUG_ICON_HD_PATH                     SYS_IMG_PATH_ROOT"/debug/VisualizationDebug_HD.png" 
#else
#define PNG_DEBUG_VISUALIZATION_ICON_SD_PATH       SYS_IMG_PATH_ROOT"/debug/VisualizationDebug_SD.png" 
#define PNG_DEBUG_ICON_SD_PATH                     SYS_IMG_PATH_ROOT"/debug/VisualizationDebug_SD.png" 
#endif

namespace Hippo {
//wifi
#ifdef INCLUDE_WIFI
#if(SUPPORTE_HD)
static WidgetSource WifiDisconnectAlertSource = {StandardScreen::S720, 252, 121, 775, 478, 0, (void*)PNG_WIFI_ERRORCODE_10060_ALERT_HD_PATH, 0};
static WidgetSource WifiDisconnectSource = {StandardScreen::S720, 1070, 550, 160, 160, 0, (void*)PNG_WIFI_DISCONNECT_ICON_PATH, 0};
#else
static WidgetSource WifiDisconnectSource = {StandardScreen::S576, 540, 396, 160, 160, 0, (void*)PNG_WIFI_DISCONNECT_ICON_PATH, 0};
static WidgetSource WifiDisconnectAlertSource = {StandardScreen::S576, 82, 111, 555, 353, 0, (void*)PNG_WIFI_ERRORCODE_10060_ALERT_SD_PATH, 0};
#endif
static WidgetSource WifiNoSignalSource = { StandardScreen::S576, 590, 500, 72, 40, 0, (void*)GIF_WIFI_NOSIGNAL_ICON_PATH, 0};
static WidgetSource WifiWeakSignal1Source = { StandardScreen::S576, 590, 500, 72, 40, 0, (void*)GIF_WIFI_WEEKSIGNAL_1_ICON_PATH, 0 };
static WidgetSource WifiWeakSignal2Source = { StandardScreen::S576, 590, 500, 72, 40, 0, (void*)GIF_WIFI_WEEKSIGNAL_2_ICON_PATH, 0 };
static WidgetSource WifiStrongSignalSource = { StandardScreen::S576, 590, 500, 72, 40, 0, (void*)GIF_WIFI_STRONGSIGNAL_ICON_PATH, 0 };
#endif

//press
static WidgetSource PressSource = {StandardScreen::S576, 590, 450, 100, 50, 0, (void*)GIF_PRESS_ICON_PATH, 0};

//ntp
#if(SUPPORTE_HD)
static WidgetSource ErrorCode10030Source = {StandardScreen::S720, 0, 284, 1280, 436, 0, (void*)PNG_ERRORCODE_10030_ICON_PATH, 0};
static WidgetSource ErrorCode10031Source = {StandardScreen::S720, 0, 284, 1280, 436, 0, (void*)PNG_ERRORCODE_10031_ICON_PATH, 0};
#else
static WidgetSource ErrorCode10030Source = {StandardScreen::S576, 0, 269, 720, 307, 0, (void*)PNG_ERRORCODE_10030_ICON_PATH, 0};
static WidgetSource ErrorCode10031Source = {StandardScreen::S576, 0, 269, 720, 307, 0, (void*)PNG_ERRORCODE_10031_ICON_PATH, 0};
#endif

//network
#if defined(VIETTEL_HD) || defined(Cameroon_v5)
static WidgetSource DisconnectSource = {StandardScreen::S576, 585, 450, 100, 100, 0, (void*)GIF_DISCONNECT_VIETTEL_ICON_PATH, 0};
static WidgetSource DuplicateSource = {StandardScreen::S576, 585, 450, 100, 100, 0, (void*)GIF_DUPLICATE_VIETTEL_ICON_PATH, 0};
static WidgetSource DhcpfaildSource = {StandardScreen::S576, 585, 450, 100, 100, 0, (void*)GIF_DHCP_VIETTEL_ICON_PATH, 0};
#else
#if(SUPPORTE_HD)
static WidgetSource DisconnectAlertSource = {StandardScreen::S720, 252, 121, 775, 478, 0, (void*)PNG_NETWORK_ERRORCODE_10000_ALERT_HD_PATH, 0};
static WidgetSource DisconnectSource = {StandardScreen::S720, 1070, 550, 160, 160, 0, (void*)PNG_NETWORK_DISCONNECT_ICON_PATH, 0};
#else
static WidgetSource DisconnectSource = {StandardScreen::S576, 540, 396, 160, 160, 0, (void*)PNG_NETWORK_DISCONNECT_ICON_PATH, 0};
static WidgetSource DisconnectAlertSource = {StandardScreen::S576, 82, 111, 555, 353, 0, (void*)PNG_NETWORK_ERRORCODE_10000_ALERT_SD_PATH, 0};
#endif

static WidgetSource DuplicateSource = {StandardScreen::S576, 590, 500, 70, 30, 0, (void*)GIF_NETWORK_DUPLICATE_ICON_PATH, 0};
#endif

//debug
#if(SUPPORTE_HD)
static WidgetSource VisualizationDebugSource = {StandardScreen::S720, 76, 602, 121, 46, 0, (void*)PNG_DEBUG_VISUALIZATION_ICON_HD_PATH, 0};
static WidgetSource UsbInfoSource = {StandardScreen::S720, 425, 581, 430, 88, 0, 0, 0};
static WidgetSource DebugSource = {StandardScreen::S720, 76, 602, 121, 46, 0, (void*)PNG_DEBUG_ICON_HD_PATH, 0};
#else
static WidgetSource VisualizationDebugSource = {StandardScreen::S576, 56, 512, 92, 36, 0, (void*)PNG_DEBUG_VISUALIZATION_ICON_SD_PATH, 0};
static WidgetSource UsbInfoSource = {StandardScreen::S576, (((720 - 372 )/2)&0xfffe), 445, 372, 70, 0, 0, 0};
static WidgetSource DebugSource = {StandardScreen::S576, 56, 512, 92, 36, 0, (void*)PNG_DEBUG_ICON_SD_PATH, 0};
#endif


UsbInfoWidget *PromptWidget::mUsbInfo = 0;

PromptWidget::PromptWidget( )
	: DebugIcon(&DebugSource)
    , PressIcon(&PressSource)
    , DuplicateIcon(&DuplicateSource)
    , DisconnectIcon(&DisconnectSource)
    , mDisconnectAlertIcon(&DisconnectAlertSource)
#ifdef INCLUDE_WIFI
    , mWifiDisconnectIcon(&WifiDisconnectSource)
    , mWifiDisconnectAlertIcon(&WifiDisconnectAlertSource)
    , mWifiStrongSignalIcon(&WifiStrongSignalSource)
    , mWifiNoSignalIcon(&WifiNoSignalSource)
    , mWifiWeakSignal1Icon(&WifiWeakSignal1Source)
    , mWifiWeakSignal2Icon(&WifiWeakSignal2Source)
#endif
    , ErrorCode10030(&ErrorCode10030Source)
    , ErrorCode10031(&ErrorCode10031Source)
#ifdef VIETTEL_HD
    ,DhcpFailedIcon(&DhcpfaildSource)
#endif
    ,VisualizationDebugIcon(&VisualizationDebugSource)
{
    LayerMixerDevice::Layer* layer = Hippo::systemManager().mixer().topLayer();

    layer->attachChildToFront(&DebugIcon);
    layer->attachChildToFront(&PressIcon);
    layer->attachChildToFront(&DuplicateIcon);

#ifdef INCLUDE_WIFI
    layer->attachChildToFront(&mWifiDisconnectIcon);
    layer->attachChildToFront(&mWifiDisconnectAlertIcon);
    layer->attachChildToFront(&mWifiStrongSignalIcon);
    layer->attachChildToFront(&mWifiNoSignalIcon);
    layer->attachChildToFront(&mWifiWeakSignal1Icon);
    layer->attachChildToFront(&mWifiWeakSignal2Icon);
#endif

    layer->attachChildToFront(&DisconnectIcon);
    layer->attachChildToFront(&mDisconnectAlertIcon);

    layer->attachChildToFront(&ErrorCode10030);
    layer->attachChildToFront(&ErrorCode10031);

#ifdef VIETTEL_HD
	layer->attachChildToFront(&DhcpFailedIcon);
#endif
    layer->attachChildToFront(&VisualizationDebugIcon);

    mUsbInfo = new UsbInfoWidget(&UsbInfoSource);
    layer->attachChildToFront(mUsbInfo);
}

PromptWidget::~PromptWidget()
{
    DebugIcon.detachFromParent();
    PressIcon.detachFromParent();
    DuplicateIcon.detachFromParent();

#ifdef INCLUDE_WIFI
    mWifiDisconnectIcon.detachFromParent();
    mWifiDisconnectAlertIcon.detachFromParent();
    mWifiStrongSignalIcon.detachFromParent();
    mWifiNoSignalIcon.detachFromParent();
    mWifiWeakSignal1Icon.detachFromParent();
    mWifiWeakSignal2Icon.detachFromParent();
#endif

    DisconnectIcon.detachFromParent();
    mDisconnectAlertIcon.detachFromParent();

    ErrorCode10030.detachFromParent();
    ErrorCode10031.detachFromParent();

#ifdef VIETTEL_HD
    DhcpFailedIcon.detachFromParent();
#endif
    VisualizationDebugIcon.detachFromParent();
    mUsbInfo->detachFromParent();
}

void
PromptWidget::showDebug()
{
    DebugIcon.setVisibleP(true);
    return ;
}

void
PromptWidget::clearDebug()
{
    DebugIcon.setVisibleP(false);
    return ;
}

#ifdef INCLUDE_WIFI
void
PromptWidget::showWifiDisconnect()
{
    NetworkCard* device = networkManager().getDevice("rausb0");
    if (device->linkStatus() <= 0)
        mWifiDisconnectIcon.setVisibleP(true);
    return ;
}

void
PromptWidget::clearWifiDisconnect()
{
    mWifiDisconnectIcon.setVisibleP(false);
    return ;
}

void
PromptWidget::showWifiDisconnectAlert()
{
    NetworkCard* device = networkManager().getDevice("rausb0");
    if (device->linkStatus() <= 0)
        mWifiDisconnectAlertIcon.setVisibleP(true);
    return ;
}

void
PromptWidget::clearWifiDisconnectAlert()
{
    mWifiDisconnectAlertIcon.setVisibleP(false);
    return ;
}

void
PromptWidget::showWifiStrongSingal()
{
    mWifiStrongSignalIcon.setVisibleP(true);
    return ;
}

void
PromptWidget::clearWifiStrongSignal()
{
    mWifiStrongSignalIcon.setVisibleP(false);
    return ;
}

void
PromptWidget::showWifiNoSignal()
{
    mWifiNoSignalIcon.setVisibleP(true);
    return ;
}

void
PromptWidget::clearWifiNoSignal()
{
    mWifiNoSignalIcon.setVisibleP(false);
    return ;
}

void
PromptWidget::showWifiWeakSignal1()
{
    mWifiWeakSignal1Icon.setVisibleP(true);
    return ;
}

void
PromptWidget::clearWifiWeakSignal1()
{
    mWifiWeakSignal1Icon.setVisibleP(false);
    return ;
}

void
PromptWidget::showWifiWeakSignal2()
{
    mWifiWeakSignal2Icon.setVisibleP(true);
    return ;
}

void
PromptWidget::clearWifiWeakSignal2()
{
    mWifiWeakSignal2Icon.setVisibleP(false);
    return ;
}
#endif

void
PromptWidget::showDisconnect()
{
    char devname[USER_LEN] = { 0 };
    network_default_devname(devname, USER_LEN);
    if(!network_device_link_state(devname))
        DisconnectIcon.setVisibleP(true);
    else
        DisconnectIcon.setVisibleP(false);
    return ;
}

void
PromptWidget::clearDisconnect()
{
    DisconnectIcon.setVisibleP(false);
    return ;
}

void
PromptWidget::showDisconnectAlert()
{
    char devname[USER_LEN] = { 0 };
    network_default_devname(devname, USER_LEN);
    if(!network_device_link_state(devname))
        mDisconnectAlertIcon.setVisibleP(true);
    else
        mDisconnectAlertIcon.setVisibleP(false);
    return ;
}

void
PromptWidget::clearDisconnectAlert()
{
    mDisconnectAlertIcon.setVisibleP(false);
    return ;
}

void
PromptWidget::showPress()
{
    PressIcon.setVisibleP(true);
    return ;
}

void
PromptWidget::clearPress()
{
    PressIcon.setVisibleP(false);
    return ;
}

void
PromptWidget::showDuplicate()
{
    DuplicateIcon.setVisibleP(true);
    return ;
}

void
PromptWidget::clearDuplicate()
{
    DuplicateIcon.setVisibleP(false);
    return ;
}

void
PromptWidget::showErrorCode10030()
{
    ErrorCode10030.setVisibleP(true);
    return ;
}

void
PromptWidget::clearErrorCode10030()
{
    ErrorCode10030.setVisibleP(false);
    return ;
}

void
PromptWidget::showErrorCode10031()
{
    ErrorCode10031.setVisibleP(true);
    return ;
}

void
PromptWidget::clearErrorCode10031()
{
    ErrorCode10031.setVisibleP(false);
    return ;
}

void
PromptWidget::showDhcpFailedIcon()
{
#ifdef VIETTEL_HD
    DhcpFailedIcon.setVisibleP(true);
#endif
    return ;
}

void
PromptWidget::clearDhcpFailedIcon()
{
#ifdef VIETTEL_HD
    DhcpFailedIcon.setVisibleP(false);
#endif
    return ;
}

void
PromptWidget::showVisualizationDebug()
{
    VisualizationDebugIcon.setVisibleP(true);
    return ;
}

void
PromptWidget::clearVisualizationDebug()
{
    VisualizationDebugIcon.setVisibleP(false);
    return ;
}

void
PromptWidget::showUsbInfo()
{
    mUsbInfo->setVisibleP(true);
    mUsbInfo->inval(NULL);
    return;
}

void
PromptWidget::clearUsbInfo()
{
    mUsbInfo->setVisibleP(false);
    return;
}

void
PromptWidget::setUsbMessage(std::string msg)
{
    if (mUsbInfo && !msg.empty())
         mUsbInfo->setUsbInfo(msg);
}
} // namespace Hippo

