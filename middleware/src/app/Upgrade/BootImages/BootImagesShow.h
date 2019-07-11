#ifndef _BootImagesShow_H_
#define _BootImagesShow_H_

#include "config/pathConfig.h"

#if defined(Jiangsu)
#define BOOT_LOGO_PATH SYSTEM_LOGO_DIR"/bootlogo.jpg"
#define AUTH_LOGO_PATH SYSTEM_LOGO_DIR"/authlogo.jpg"
#else
#define BOOT_LOGO_PATH SYSTEM_LOGO_DIR"/bootlogo.gif"
#define AUTH_LOGO_PATH SYSTEM_LOGO_DIR"/authlogo.gif"
#endif

#define OLD_BOOT_LOGO_PATH SYSTEM_LOGO_DIR"/bootlogo"
#define OLD_AUTH_LOGO_PATH SYSTEM_LOGO_DIR"/authbg"

#ifdef __cplusplus
extern "C" {
#endif

int BootImagesShowLogoInit(void);
int BootImagesShowBootLogo(int show);
int BootImagesShowAuthLogo(int show);
int ImageZoomShowBackground(int show);

#ifdef __cplusplus
}
#endif

#endif //_BootImagesShow_H_
