#ifndef _MessageValueSystem_H_
#define _MessageValueSystem_H_

#include "browser_event.h"


#define MV_System_OpenBootPage            EIS_IRKEY_PAGE_BOOT
#define MV_System_OpenConfigPage          EIS_IRKEY_PAGE_CONFIG
#define MV_System_OpenStandbyPage         EIS_IRKEY_PAGE_STANDBY
#define MV_System_OpenUpgradePage         0x9215 /*EIS_IRKEY_PAGE_UPGRADE*/
#define MV_System_OpenTimeoutPage         EIS_IRKEY_PAGE_TIMEOUT
#define MV_System_OpenErrorPage           EIS_IRKEY_PAGE_ERROR
#define MV_System_OpenMenuPage            EIS_IRKEY_URL_MENU
#define MV_System_OpenTransparentPage     EIS_IRKEY_PAGE_TRANSPARENT
// #define MV_System_OpenDisconnectPage      EIS_IRKEY_NET_UNCONNECT // 0x1392
#define MV_System_OpenUnzippingPage       0x924C
#define MV_System_OpenUnzipErrorPage      0x924D

#if defined(Gansu)
#define MV_System_UnzipErr2Restart        0x2000
#endif

#define MV_System_OpenDVBPage             EIS_IRKEY_PAGE_DVBS
#define MV_System_ModifyPPPoEAccount      EIS_IRKEY_MODIFY_PPPOEACCOUNT
#define MV_System_ModifyPPPoEPwd          EIS_IRKEY_MODIFY_PPPOEPWD
#define MV_System_OpenCheckPPPoEAccountPage     EIS_IRKEY_PAGE_CHECK_PPPOEACCOUNT
#define MV_System_Propmt    0x924E

#ifdef __cplusplus

namespace Hippo {


} // namespace Hippo

#endif // __cplusplus

#endif // _MessageValueSystem_H_
