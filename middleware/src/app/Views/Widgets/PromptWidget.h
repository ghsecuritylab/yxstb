#ifndef _PromptWidget_H_
#define _PromptWidget_H_

#include "Icon.h"
#include "UsbInfoWidget.h"

#ifdef __cplusplus
#include <string>

namespace Hippo
{

class PromptWidget
{
public:
    PromptWidget();
    ~PromptWidget();


    void showDebug(void);
    void clearDebug(void);
#ifdef INCLUDE_WIFI
    void showWifiDisconnect(void);
    void clearWifiDisconnect(void);
    void showWifiDisconnectAlert(void);
    void clearWifiDisconnectAlert(void);
    void showWifiStrongSingal(void);
    void clearWifiStrongSignal(void);
    void showWifiNoSignal(void);
    void clearWifiNoSignal(void);
    void showWifiWeakSignal1(void);
    void clearWifiWeakSignal1(void);
    void showWifiWeakSignal2(void);
    void clearWifiWeakSignal2(void);
#endif

    void showDisconnect(void);
    void clearDisconnect(void);
    void showDisconnectAlert(void);
    void clearDisconnectAlert(void);

    void showPress(void);
    void clearPress(void);
    void showDuplicate();
    void clearDuplicate();

    void showErrorCode10030();
    void clearErrorCode10030();
    void showErrorCode10031();
    void clearErrorCode10031();

    void showDhcpFailedIcon();
    void clearDhcpFailedIcon();
    void showVisualizationDebug();
    void clearVisualizationDebug();
    void showUsbInfo();
    void clearUsbInfo();
    void setUsbMessage(std::string msg);
private:
    Icon DebugIcon;
#ifdef INCLUDE_WIFI
    Icon mWifiDisconnectIcon;
    Icon mWifiDisconnectAlertIcon;
    Icon mWifiStrongSignalIcon;
    Icon mWifiNoSignalIcon;
    Icon mWifiWeakSignal1Icon; // using mWifiWeakSignal1Icon and mWifiWeakSignal2Icon together.
    Icon mWifiWeakSignal2Icon;
#endif

    Icon DisconnectIcon;
    Icon mDisconnectAlertIcon;
    int mDisconnectStatus;

    Icon PressIcon;
    Icon DuplicateIcon;
    Icon ErrorCode10030;
    Icon ErrorCode10031;
#ifdef VIETTEL_HD
    Icon DhcpFailedIcon;
#endif
    Icon VisualizationDebugIcon;

    static UsbInfoWidget *mUsbInfo;

};

} // namespace Hippo

#endif // __cplusplus

#endif // _PromptWidget_H_
