
#include "EmergencyDialog.h"

#include "Message.h"
#include "MessageTypes.h"
#include "SystemManager.h"
#include "browser_event.h"
#if defined(U_CONFIG)
#include "UDiskDetect.h"
#endif
#include "NativeHandler.h"
#include "BootImagesShow.h"
#include "AppSetting.h"
#include "SysSetting.h"

#include "UtilityTools.h"
#include "libzebra.h"
#include "mid_fpanel.h"
#include "BootImagesDownload.h"

#include "VersionSetting.h"
#include "SettingEnum.h"
#include "config.h"

#ifdef HUAWEI_C10
#define UPGRADE_POS_X (((720-640)/2)&0xfffe)
#define UPGRADE_POS_Y (((576 - 530)/2)&0xfffe)
#else
#define UPGRADE_POS_X (((720-450)/2)&0xfffe)
#define UPGRADE_POS_Y ((((576 - 160)/2)&0xfffe) -80)
#endif

extern "C" int iptv_appInit();

#ifdef HUAWEI_C10
#if defined(hi3560e)
#define GIF_EMERGENCY_SD_PATH     SYS_IMG_PATH_ROOT"/upgrade/emergency_SD.gif"
#else
#define GIF_EMERGENCY_HD_PATH     SYS_IMG_PATH_ROOT"/upgrade/emergency_HD.gif"
#endif
#else //defined c20
#define JPG_EMERGENCY_PATH        SYS_IMG_PATH_ROOT"/upgrade/emergency.jpg"
#endif //end defined c10

namespace Hippo {
#ifdef HUAWEI_C10
#if defined(hi3560e)
static WidgetSource EmergencyBackgroundSource = {StandardScreen::S576, UPGRADE_POS_X, UPGRADE_POS_Y, 640, 530, 0, (void*)GIF_EMERGENCY_SD_PATH, 0};
#else
static WidgetSource EmergencyBackgroundSource = {StandardScreen::S720, 0, 0, 1280, 720, 0, (void*)GIF_EMERGENCY_HD_PATH, 0};
#endif
#else
static WidgetSource EmergencyBackgroundSource = {StandardScreen::S576, UPGRADE_POS_X, UPGRADE_POS_Y, 450, 350, 0, (void *)JPG_EMERGENCY_PATH, 0};
#endif


static int clearBootLogo()
{
#if 0
    int ret = 0;
    unsigned char buf[4] = {0xff, 0xd8, 0xff,0xd9};
    ret = yhw_upgrade_bootlogo(buf, 4);
    if (ret) {
        printf("upgrade logo fail\n");
        return -1;
    }
    printf("upgrade logo ok\n");
#endif
    removeFile(BOOT_PIC_PATH);
    removeFile(AUTH_PIC_PATH);
    return 0;

}

static int setOtherPartitionStart()
{
    char *envflag = NULL;
    int bootflag = -1;

    if (yhw_env_readString( "SYSTEMUPGRADEOK", &envflag ) != 0) {
        printf("SYSTEMUPGRADEOK err!!");
        return -1;
    }
    if (strcmp(envflag, "y")){
        printf("envflag y err!!");
        return -1;
    }
    if (yhw_env_readString((char *)"ROOTFS", &envflag) == 0) {
        bootflag = atoi(envflag);
        if (bootflag == 0) {
            if (yhw_env_writeString((char *)"ROOTFS", (char *)"1") == 0) {
                if (yhw_env_writeString((char *)"BAKROOTFS", (char *)"0") == 0) {
                    printf("Set Rootfs to 1 ok\n");
                }
            }
        } else if (bootflag == 1) {
            if (yhw_env_writeString((char *)"ROOTFS", (char *)"0") == 0) {
                if (yhw_env_writeString((char *)"BAKROOTFS", (char *)"1") == 0) {
                    printf("Set Rootfs to 0 ok\n");
                }
            }
         }
    }

    if (yhw_env_readString((char *)"KERNEL", &envflag) == 0) {
        bootflag = atoi(envflag);
        if (bootflag == 0) {
            if (yhw_env_writeString((char *)"KERNEL", (char *)"1") == 0) {
                if (yhw_env_writeString((char *)"BAKKERNEL", (char *)"0") == 0) {
                    printf("Set Rootfs to 1 ok\n");
                }
            }
        } else if (bootflag == 1) {
            if (yhw_env_writeString((char *)"KERNEL", (char *)"0") == 0) {
                if (yhw_env_writeString((char *)"BAKKERNEL", (char *)"1") == 0) {
                    printf("Set Rootfs to 0 ok\n");
                }
            }
         }
    }
#ifdef INCLUDE_LITTLESYSTEM
    sysSettingSetInt("upgradeForce", 1);
#endif

    return 0;
}

EmergencyDialog::EmergencyDialog(int type)
	: mEmergencyBackground(&EmergencyBackgroundSource)
       , mType(type)
{
    StandardScreen *layer = (StandardScreen *)Hippo::systemManager().mixer().topLayer();

    layer->attachChildToFront(&mEmergencyBackground);

}

EmergencyDialog::~EmergencyDialog()
{
    mEmergencyBackground.detachFromParent();
}

bool
EmergencyDialog::handleMessage(Message *msg)
{
    int localVersion = 0;
    int localVersionBak = 0;
    switch (msg->arg1) {
    case EIS_IRKEY_NUM0:
        if (mType == MiniSystem) {
            sendMessageToNativeHandler(MessageType_Unknow, LITTLE_SYSTEM_RUN, 1, 0);
            delete this;
            return true;
        }
        else {
            //sendMessageToNativeHandler(MessageType_Upgrade, 0, 0, 0);
            //appSettingSetInt("recoveryMode", 1);
            delete this;
            BootImagesShowBootLogo(1);//显示第二张logo
            iptv_appInit();
            break;
        }

    case EIS_IRKEY_NUM1:
        if (setOtherPartitionStart() == 0) {
            localVersion = upgrade_version_read(SOFTWARE_VERSION);
            localVersionBak = upgrade_version_read(SOFTWARE_VERSION_BAK);
            upgrade_version_write(SOFTWARE_VERSION, localVersionBak);
            upgrade_version_write(SOFTWARE_VERSION_BAK, localVersion);
            yhw_board_setRunlevel(1);
            appSettingSetInt("recoveryMode", 2);
            //settingManagerSave();
            mid_fpanel_reboot();
            return true;
        }
        printf("setOtherPartitionStart err!!");
        delete this;
        if (mType == MiniSystem)
            exit(0);
        else
            iptv_appInit();
        break;
    case EIS_IRKEY_NUM2:
        if (clearBootLogo() == 0) {
            yhw_board_setRunlevel(1);
            appSettingSetInt("recoveryMode", 3);
            //settingManagerSave();
            mid_fpanel_reboot();
        }
        return true;
    case EIS_IRKEY_NUM3:
#if defined(U_CONFIG)
        UDiskUpgradeInEmergency(mType);
        appSettingSetInt("recoveryMode", 4);
        //settingManagerSave();
#endif
        mEmergencyBackground.setVisibleP(false);
        delete this;
        break;
    case EIS_IRKEY_SELECT:
        break;
    default:
        delete this;
        if (mType == MiniSystem)
            exit(0);
        else
        {
            BootImagesShowBootLogo(1);//显示第二张logo
            iptv_appInit();
        }
        break;
    }

    return true;
}

void
EmergencyDialog::draw()
{
    mEmergencyBackground.setVisibleP(true);
}

} // namespace Hippo

