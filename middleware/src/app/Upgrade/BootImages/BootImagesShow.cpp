#include "Assertions.h"
#include "BootImagesShow.h"
#include "Customer/Customer.h"
#include "OSOpenSSL.h"
#include "MessageTypes.h"
#include "NativeHandler.h"

#include "SysSetting.h"
#include "AppSetting.h"

#include "config/webpageConfig.h"
#include "sys_basic_macro.h"
#include "UtilityTools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(HUAWEI_C10)
#define DEFAULT_BOOT_LOGO_PATH WEBPAGE_PATH_ROOT"/boot/image/boot_logo_bak.gif"
#define DEFAULT_AUTH_LOGO_PATH WEBPAGE_PATH_ROOT"/boot/image/boot_logo_bak.gif"
#define IMAGEZOOM_BACKGROUND_PATH WEBPAGE_PATH_ROOT"/piczoom/image/background.png"
#else
#define DEFAULT_BOOT_LOGO_PATH WEBPAGE_PATH_ROOT"/image/background.JPG"
#define DEFAULT_AUTH_LOGO_PATH WEBPAGE_PATH_ROOT"/image/background.JPG"
#define IMAGEZOOM_BACKGROUND_PATH NULL
#endif

unsigned char* gLogoBuff = NULL;
static int authlogoflags = 0;


namespace Hippo {

/**
 * @brief _InitDefaultLogo Init logo using default picture copying webpage dir
 *
 * @param srcLogo logo pic from webpage/image dir
 * @param dstLogo logo pic placed int /root/ dir
 * @param picMd5 the logo pic md5 digest
 *
 * @return -1 on error, 0 is ok
 */
static int _InitDefaultLogo(const char* srcLogo, const char* dstLogo, std::string& nMd5)
{/*{{{*/
    if (!srcLogo || !dstLogo)
        return -1;

    OpenSSL::MD5Crypto picMD5;
    mode_t oldMask = 0777;
    FILE* rFp = NULL;
    FILE* wFp = NULL;
    char* buf = NULL;
    struct stat filestat;
    if (stat(srcLogo, &filestat) < 0)
        return -1;

    LogSysOperDebug("filestat size [%d]\n", filestat.st_size);
    buf = (char*)calloc(1/*sizeof(char)*/, filestat.st_size + 1);
    if (!buf)
        return -1;

    rFp = fopen(srcLogo, "rb");
    if (!rFp)
        return -1;

    if (fread(buf, 1/*sizeof(char)*/, filestat.st_size, rFp) != filestat.st_size)
        goto Err;

    nMd5 = picMD5.CreateDigest(buf, filestat.st_size);
    if (nMd5.empty())
        goto Err;
    LogSysOperDebug("Digest [%s] \n", nMd5.c_str());

    oldMask = umask(0077);
    wFp = fopen(dstLogo, "wb");
    umask(oldMask);
    if (!wFp)
        goto Err;

    if (fwrite(buf, 1/*sizeof(char)*/, filestat.st_size, wFp) != filestat.st_size) {
        fclose(wFp);
        remove(dstLogo);
        goto Err;
    }

    free(buf);
    fclose(rFp);
    fflush(wFp);
    fclose(wFp);
    LogSysOperDebug("success\n");
    return 0;
Err:
    free(buf);
    fclose(rFp);
    LogSysOperError("init pic[%s]\n", srcLogo);
    return -1;
}/*}}}*/

/**
 * @brief _CheckLogoPicStat check logo picture which path pointed by picLogo integraty
 *
 * @param picLogo the logo picture full path needed check
 * @param picMd5 the corrected md5 digest
 *
 * @return -1 on error, positive number(>0) (the picture size) if ok
 */
static int _CheckLogoPicStat(const char* picLogo, const char* nMd5)
{/*{{{*/
    if (!picLogo)
        return -1;

    OpenSSL::MD5Crypto picMD5;
    FILE* rFp = NULL;
    struct stat filestat;
    char temp[USER_LEN + 1] = { 0 };

    if (gLogoBuff)
        free(gLogoBuff);
    gLogoBuff = NULL;

    //get picLogo file size
    if (stat(picLogo, &filestat) < 0)
        goto Err;

    gLogoBuff = (unsigned char*)calloc(1/*sizeof(char)*/, filestat.st_size + 1);
    if (!gLogoBuff)
        goto Err;

    //open file and read binary data to gLogoBufffer for MD5 using
    rFp = fopen(picLogo, "rb");
    if (!rFp)
        goto Err;

    LogSysOperDebug("file size: %ld\n", filestat.st_size);
    if (fread(gLogoBuff, 1/*sizeof(char)*/, filestat.st_size, rFp) == filestat.st_size) {
        //calculate new md5 value and compare with original md5 digest
        std::string digest = picMD5.CreateDigest((const char*)gLogoBuff, filestat.st_size);
        if(!nMd5) {
            fclose(rFp);
            return filestat.st_size;
        } else {
            if (!strcmp(nMd5, digest.c_str())) {
                fclose(rFp);
                return filestat.st_size;
            }
        }
        LogSysOperDebug("diff md5: [%s] vs [%s]\n", nMd5, digest.c_str());
    }
Err:
    LogSysOperError("show[%s]\n", picLogo);
    return -1;
}/*}}}*/

} // end of namespace Hippo

/**
 * @brief _TransitOldVerion
 */
static void _TransitOldVerion(void)
{/*{{{*/
    struct _DOWNFILE_HEADER_ {
        int integrality;
        int file_size;
        char url[128];
        char reserved[120];
    };

    OpenSSL::MD5Crypto picMD5;
    FILE* rFp = NULL;
    FILE* wFp = NULL;
    char* buf = NULL;
    int len = 0;
    struct stat filestat;

    //Transit BootLogo Picture
    if (!stat(OLD_BOOT_LOGO_PATH, &filestat)) {
        buf = (char*)calloc(1, filestat.st_size + 1);
        rFp = fopen(OLD_BOOT_LOGO_PATH, "rb");
        if (fread(buf, 1, filestat.st_size, rFp) == filestat.st_size) {
            len = filestat.st_size - sizeof(struct _DOWNFILE_HEADER_);
            std::string digest = picMD5.CreateDigest(buf + sizeof(struct _DOWNFILE_HEADER_), len);
            if (!digest.empty()) {
                LogSysOperDebug("Digest [%s] \n", digest.c_str());
                sysSettingSetString("bootlogo_md5", digest.c_str());
                mode_t oldMask = umask(0077);
                wFp = fopen(BOOT_LOGO_PATH, "wb");
                umask(oldMask);
                if (wFp) {
                    if (fwrite(buf + sizeof(struct _DOWNFILE_HEADER_), 1, len, wFp) == len)
                        fflush(wFp);
                    fclose(wFp);
                }
            }
        }
        removeFile(OLD_BOOT_LOGO_PATH);
    }

    //Transit AuthLogo Picture
    if (!stat(OLD_AUTH_LOGO_PATH, &filestat)) {
        buf = (char*)calloc(1, filestat.st_size + 1);
        rFp = fopen(OLD_AUTH_LOGO_PATH, "rb");
        if (fread(buf, 1, filestat.st_size, rFp) == filestat.st_size) {
            len = filestat.st_size - sizeof(struct _DOWNFILE_HEADER_);
            std::string digest = picMD5.CreateDigest(buf + sizeof(struct _DOWNFILE_HEADER_), len);
            if (!digest.empty()) {
                LogSysOperDebug("Digest [%s] \n", digest.c_str());
                sysSettingSetString("authbg_md5", digest.c_str());
                mode_t oldMask = umask(0077);
                wFp = fopen(AUTH_LOGO_PATH, "wb");
                umask(oldMask);
                if (wFp) {
                    if (fwrite(buf + sizeof(struct _DOWNFILE_HEADER_), 1, len, wFp) == len)
                        fflush(wFp);
                    fclose(wFp);
                }
            }
        }
        removeFile(OLD_AUTH_LOGO_PATH);
    }
}/*}}}*/

extern "C"  {
/**
 * @brief SystemLogoPicInit
 *
 * @return
 */
int
BootImagesShowLogoInit(void)
{/*{{{*/
#if !defined(Huawei_v5)
    _TransitOldVerion(); // 20130403 svn r3430
#endif

    std::string nMd5("");
    char lMd5Buff[4*MD5_DIGEST_LENGTH + 1] = "";

    //check whether exists BOOT_LOGO_PATH(bootlogo.gif)
    sysSettingGetString("bootlogo_md5", lMd5Buff, 4*MD5_DIGEST_LENGTH, 0);
    if (access(BOOT_LOGO_PATH, F_OK) < 0 || Hippo::_CheckLogoPicStat(BOOT_LOGO_PATH, lMd5Buff) < 0) {
        if (!Hippo::_InitDefaultLogo(DEFAULT_BOOT_LOGO_PATH, BOOT_LOGO_PATH, nMd5))
            sysSettingSetString("bootlogo_md5", nMd5.c_str());
        appSettingSetString("PADBootLogPicURL", "0"); //restore default
    }

    //check whether exists AUTH_LOGO_PATH(authlogo.gif)
    sysSettingGetString("authbg_md5", lMd5Buff, 4*MD5_DIGEST_LENGTH, 0);
    if (access(AUTH_LOGO_PATH, F_OK) < 0 || Hippo::_CheckLogoPicStat(AUTH_LOGO_PATH, lMd5Buff) < 0) {
        if (!Hippo::_InitDefaultLogo(BOOT_LOGO_PATH, AUTH_LOGO_PATH, nMd5))
            sysSettingSetString("authbg_md5", nMd5.c_str());
        appSettingSetString("PADAuthenBackgroundPicURL", "0"); //restore default
    }

    BootImagesShowBootLogo(1);
    return 0;
}/*}}}*/

/**
 * @brief BootImagesShowBootLogo
 *
 * @param show flag 0: hide 1:show
 *
 * @return
 */
int
BootImagesShowBootLogo(int show) // called eg: Utility.setValueByName("JseSysControl", "logoshow:1");
{/*{{{*/
    LogSysOperDebug("show: %d\n", show);
    int imageLen = 0;

    if (show) {
        // check logo picture integraty
        char lMd5Buff[4*MD5_DIGEST_LENGTH + 1] = "";
        sysSettingGetString("bootlogo_md5", lMd5Buff, 4*MD5_DIGEST_LENGTH, 0);
        imageLen = Hippo::_CheckLogoPicStat(BOOT_LOGO_PATH, lMd5Buff);
        int showFlag = 1;
#if defined(Jiangsu)
        appSettingGetInt("bootPicEnableFlag", &showFlag, 0);
#endif
        if (imageLen > 0 && showFlag)
            LogoShow(show, gLogoBuff, imageLen);
    } else {
        //clear the last logo picture
        LogoShow(0, NULL, 0);
        // free buffer of logo picture
        if (gLogoBuff) {
            free(gLogoBuff);
            gLogoBuff = NULL;
        }
    }
    return imageLen;
}/*}}}*/

/**
 * @brief BootImagesShowAuthLogo
 *
 * @param show flag 0: hide 1:show
 *
 * @return
 */
int
BootImagesShowAuthLogo(int show)
{/*{{{*/
    LogSysOperDebug("show: %d\n", show);
    int imageLen = 0;

	if(authlogoflags == show) {
        LogSysOperError("Authlogo has been opened or closed\n");
        if(!show)
            BootImagesShowBootLogo(0);
        return -1;
	}

    authlogoflags = show;
    if (show) {
        // check logo picture integraty
        char lMd5Buff[4 * MD5_DIGEST_LENGTH + 1] = "";
        sysSettingGetString("authbg_md5", lMd5Buff, 4 * MD5_DIGEST_LENGTH, 0);
        imageLen = Hippo::_CheckLogoPicStat(AUTH_LOGO_PATH, lMd5Buff);
        int showFlag = 1;
#if defined(Jiangsu)
        appSettingGetInt("AuthShowPicFlag", &showFlag, 0);
#endif
        if (imageLen > 0 && showFlag)
            LogoShow(show, gLogoBuff, imageLen);
#if defined(Jiangsu)
        sendMessageToNativeHandler(MessageType_ClearAuthLogo, 0, 0, 5000);
#endif
    } else {
        //clear the last logo picture
        LogoShow(0, NULL, 0);
        // free buffer of logo picture
        if (gLogoBuff) {
            free(gLogoBuff);
            gLogoBuff = NULL;
        }
    }
    return imageLen;
}/*}}}*/

int ImageZoomShowBackground(int show)
{
    int imageLen = 0;

    if (show) {
        imageLen = Hippo::_CheckLogoPicStat(IMAGEZOOM_BACKGROUND_PATH, NULL); // check logo picture integraty
        if (imageLen > 0)
            LogoShow(show, gLogoBuff, imageLen);
    } else { // clear the last logo picture
        LogoShow(0, NULL, 0);

        if (gLogoBuff) { // free buffer of logo picture
            free(gLogoBuff);
            gLogoBuff = NULL;
        }
    }
    return imageLen;
}
} // End of extern "C"
