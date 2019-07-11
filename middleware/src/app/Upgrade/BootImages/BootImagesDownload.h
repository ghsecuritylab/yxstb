#ifndef _BootImagesDownload_H_
#define _BootImagesDownload_H_

#include "config/pathConfig.h"

#ifdef TR069_UPGRADE_LOGO  //used in jiangsu
#define START_PIC_PATH SYSTEM_LOGO_DIR"/startlogo.jpg" // the first picture
#define BOOT_PIC_PATH  SYSTEM_LOGO_DIR"/bootlogo.jpg" // the second picture
#define AUTH_PIC_PATH  SYSTEM_LOGO_DIR"/authlogo.jpg" // the third picture
#else
#define START_PIC_PATH SYSTEM_LOGO_DIR"/startlogo.gif" // the first picture
#define BOOT_PIC_PATH  SYSTEM_LOGO_DIR"/bootlogo.gif" // the second picture
#define AUTH_PIC_PATH  SYSTEM_LOGO_DIR"/authlogo.gif" // the third picture
#endif

enum{
    START_PICTURE = 1,
    BOOT_PICTURE,
    AUTH_PICTURE
};

enum{
    UPGRADE_LOGO_RESULT_NOT_UPDATED = 0,
    UPGRADE_LOGO_RESULT_SUCCESS,
    UPGRADE_LOGO_RESULT_FAILURE
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
namespace Hippo {

int BootImagesDownloadBootLogo(const char* path);
int BootImagesDownloadAuthLogo(const char* path);
int BootImagesDownloadStartLogo(const char* path);

int BootImageLogoUpdateResultGet(int upgradeType);
void BootImageLogoUpdateResultSet(int result, int upgradeType);

} // End of namespace Hippo
#endif

#ifdef __cplusplus
}
#endif

#endif //End _BootImagesDownload_H_

