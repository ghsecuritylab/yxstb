
#include "JseUDisk.h"
#include "JseFunctionCall.h"
#include "JseRoot.h"

#include "JseAssertions.h"
#include "UDiskConfig.h"
#include "UDiskDetect.h"
#include "UDiskQuickInstall.h"

#include "sys_msg.h"
#include "mid/mid_task.h"

#ifdef TVMS_OPEN
#include "tvms_setting.h"
#endif


#include <stdio.h>
#include <fnmatch.h>
#include <mntent.h>
#include <dirent.h>
#include <errno.h>

using namespace Hippo;
static int JseSysUDiskUserInfoWrite( const char* param, char* value, int len )
{
    UDiskSetCommonConfigData();
    UDiskSetUserConfigData();
    settingManagerSave();
    UDiskChanageUserStatus(value);
    mid_task_delay(1000);
    return 0;
}

/* Insterface in UDiskConfig.cpp */
static int JseSysUDiskCommonCfgRead( const char* param, char* value, int len )
{
    char wCommonCfg[64] = { 0 };
    sprintf(wCommonCfg,"/mnt/usb%d/usbconfig/common.cfg", UDiskGetMountNumber());
    if (access(wCommonCfg, R_OK | F_OK ) < 0)
        strcpy(value, "-1");
    else
        strcpy(value, "0");
    return 0;
}

static int JseSysUDiskCommonSetRead( const char* param, char* value, int len )
{
    int flag;

    flag = UDiskReadCommonConfigData();
    if (flag == -1) {
        LogJseDebug("parse err,please check the common.cfg file!!!\n");
        sprintf(value, "%d", -1);
        return -1;
    }
    UDiskSetCommonConfigData();
#ifdef TVMS_OPEN
    tvms_config_save();
#endif
    settingManagerSave();

    LogJseDebug("common set flag [%d]\n", flag);
    return sprintf(value, "%d", flag);
}

static int JseSysUDiskUserInfoCountRead( const char* param, char* value, int len )
{
    return snprintf(value, len, "%d", UDiskReadAccountConfigData());
}

static int JseSysUDiskUserInfoRead( const char* param, char* value, int len )
{
    AccountConfig_s* pAccountCfg = UDiskGetUserConfigByIndex(atoi(param));
    if (!pAccountCfg) {
        LogJseDebug( "wrong user account num.\n" );
        return -1;
    }
    LogJseDebug("account = %s, user name = %s\n",pAccountCfg->nAccount, pAccountCfg->nUser);
    return snprintf(value, len, "%s^%s^%d", pAccountCfg->nAccount, pAccountCfg->nUser, pAccountCfg->nIsInitialzed);
}

static int JseSysUDiskUserNumByUseridRead( const char* param, char* value, int len )
{
    return snprintf(value, len, "%d", UDiskGetUserConfigByUserID(param));
}

static int JseSysUDiskUserCfgflagRead( const char* param, char* value, int len )
{
    return snprintf(value, len, "%d", UDiskReadUserConfigData(param));
}

/* Insterface in UDiskQuickInstall */
static int JseLogUDiskInstallStartWrite( const char* param, char* value, int len )
{
    if (!param || !value)
        return -1;

    int   uDiskID = -1;
    FILE* pMnt = NULL;
    DIR*  pDir = NULL;
    struct mntent *pMntEntNext = NULL;
    struct dirent *pDirEntNext = NULL;

    if (!(pMnt = setmntent("/proc/mounts", "r"))) {
        LogJseDebug("setmntent return NULL\n");
        return -1;
    }
    while (uDiskID == -1 && (pMntEntNext = getmntent(pMnt))) {
        if (!strncmp( pMntEntNext->mnt_dir, "/mnt/usb", 8)) {
            LogJseDebug("pMntEntNext->mnt_dir is [%s]\n", pMntEntNext->mnt_dir);
            if (!(pDir = opendir(pMntEntNext->mnt_dir))) {
                LogJseWarning("error [%s]\n", strerror(errno));
                break;
            }
            while ((pDirEntNext = readdir(pDir))) {
                if (!strncasecmp(pDirEntNext->d_name, (const char*)"CUSTBConfig", 11)) {
                    uDiskID = (char)*(pMntEntNext->mnt_dir + 8) - 0x30;
                    LogJseDebug("find CUSTBConfig\n");
                    break;
                }
            }
            if (closedir(pDir) < 0) {
                LogJseWarning("close dir\n");
                break;
            }
        }
    }
    endmntent(pMnt);
    return UDiskInstallStart(uDiskID);
}

static int JseUDiskConfigCheckUpgradeRead( const char* param, char* value, int len )
{
    return sprintf(value, "%d", UDiskUpgradeExecute(0, 0));
}

static int JseUDiskConfigCheckConfigRead( const char* param, char* value, int len )
{
    return sprintf(value, "%d", UDiskConfigExecute(0));
}

/* Insterface in UDiskDetect.cpp */
static int JseUnzipinfoRead( const char* param, char* value, int len )
{
    return sprintf(value, "%d", UDiskUnzipStatusGet());
}

static int JseUsbUnzipConfigDetectRead( const char* param, char* value, int len )
{
    return sprintf(value, "%d", UDiskConfigPacketDetect(0));
}


int JseUDiskInit()
{
    JseCall* call;
    //UÅÌÃâÅä
    call = new JseFunctionCall("sys_UDiskUserInfo_set", 0, JseSysUDiskUserInfoWrite);
    JseRootRegist(call->name(), call);
    //UÅÌÃâÅä
    call = new JseFunctionCall("sys_UDisk_common_cfg", JseSysUDiskCommonCfgRead, 0);
    JseRootRegist(call->name(), call);
    //UÅÌÃâÅä
    call = new JseFunctionCall("sys_UDisk_common_ret", JseSysUDiskCommonSetRead, 0);
    JseRootRegist(call->name(), call);
    //UÅÌÃâÅä
    call = new JseFunctionCall("sys_UDiskUserInfo_count", JseSysUDiskUserInfoCountRead, 0);
    JseRootRegist(call->name(), call);
    //UÅÌÃâÅä
    call = new JseFunctionCall("sys_UDiskUserInfo", JseSysUDiskUserInfoRead, 0);
    JseRootRegist(call->name(), call);
    //UÅÌÃâÅä
    call = new JseFunctionCall("sys_UDiskUser_num_by_userid", JseSysUDiskUserNumByUseridRead, 0);
    JseRootRegist(call->name(), call);
    //UÅÌÃâÅä
    call = new JseFunctionCall("sys_UDiskUser_cfgflag", JseSysUDiskUserCfgflagRead, 0);
    JseRootRegist(call->name(), call);
    //UÅÌÃâÅä
    call = new JseFunctionCall("u_config_CUtr069_set", 0, JseLogUDiskInstallStartWrite);
    JseRootRegist(call->name(), call);

    //UÅÌÃâÅä UÅÌÉý¼¶
    call = new JseFunctionCall("u_config_check_upgrade", JseUDiskConfigCheckUpgradeRead, 0);
    JseRootRegist(call->name(), call);
    //UÅÌÃâÅä UÅÌÉý¼¶
    call = new JseFunctionCall("u_config_check_config", JseUDiskConfigCheckConfigRead, 0);
    JseRootRegist(call->name(), call);
    //UÅÌÉý¼¶
    call = new JseFunctionCall("unzipinfo", JseUnzipinfoRead, 0);
    JseRootRegist(call->name(), call);
    //UÅÌÉý¼¶
    call = new JseFunctionCall("usb_unzip_config_detect", JseUsbUnzipConfigDetectRead, 0);
    JseRootRegist(call->name(), call);

    return 0;
}

