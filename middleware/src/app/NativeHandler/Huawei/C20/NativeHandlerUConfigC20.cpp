
#include <iostream>

#include "UDiskDetect.h"
#include "NativeHandlerUConfigC20.h"
#include "NativeHandlerAssertions.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "SystemManager.h"
#include "StandardScreen.h"

#include "config.h"

extern int gUnzipStatus;

namespace Hippo {

NativeHandlerUConfigC20::NativeHandlerUConfigC20()
{
}

NativeHandlerUConfigC20::~NativeHandlerUConfigC20()
{
}

void
NativeHandlerUConfigC20::onActive()
{
    DONT_SET(NMB_SHOW_ERROR_CODE);
    DONT_SET(NMB_SHOW_BOTTOM_ICON);
    //DONT_SET(NMB_CONNECTOK_DOMENU);
    DONT_SET(NMB_REJOIN_WIRELESS);
}

void
NativeHandlerC20::onUnactive()
{
    DONT_ZERO();
}


bool
NativeHandlerUConfigC20::handleMessage(Message *msg)
{
    NATIVEHANDLER_LOG("NativeHandlerBoot::handleMessage what(0x%x), info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);

    if (msg->what == MessageType_Repaint) {
        systemManager().mixer().refresh();
        return true;
    }
    /*else if (msg->what == MessageType_KeyDown)*/ {
        switch (msg->arg1) {
            case EIS_IRKEY_NUM0:
            case EIS_IRKEY_NUM1:
            case EIS_IRKEY_NUM2:
            case EIS_IRKEY_NUM3:
            case EIS_IRKEY_NUM4:
            case EIS_IRKEY_NUM5:
            case EIS_IRKEY_NUM6:
            case EIS_IRKEY_NUM7:
            case EIS_IRKEY_NUM8:
            case EIS_IRKEY_NUM9:
            case EIS_IRKEY_UP:
            case EIS_IRKEY_DOWN:
            case EIS_IRKEY_LEFT:
            case EIS_IRKEY_RIGHT:
            case EIS_IRKEY_SELECT:
            case EIS_IRKEY_BACK:
            case EIS_IRKEY_IME:
            case EIS_IRKEY_STAR:
            case EIS_IRKEY_PAGE_UP:
            case EIS_IRKEY_PAGE_DOWN:
            case EIS_IRKEY_POWER:
                return false;

            case USB_UPGRADE_OK:
            case USB_CONFIG_OK:
            case MV_System_OpenUnzipErrorPage:
                return NativeHandlerPublicC20::handleMessage(msg);

            case EIS_IRKEY_USB_UNINSERT:
                gUnzipStatus = 3;
                return true;

            default:
                return true;
        }
    }
}

bool
NativeHandlerUConfigC20::doUSBUninsert(Message *msg)
{
    NATIVEHANDLER_LOG("mount usb [%d] vs [%d]\n", UDiskGetMountNumber(), msg->arg2);
    return NativeHandlerPublic::doOpenUnzipErrorPage();
}

} // namespace Hippo
