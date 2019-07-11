#include "Assertions.h"
#include "BootImagesShow.h"
#include "BootImagesDownload.h"
#include "BootImagesCheck.h"
#include "OSOpenSSL.h"
#include "config/pathConfig.h"

#include "AppSetting.h"
#include "SysSetting.h"

#include "Tr069.h"
#include <openssl/md5.h>
#include "mid/mid_http.h"
#include "sys_basic_macro.h"
#include "gfx/img_decode.h"

#include "customer.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h> //for umask
#include <sys/stat.h>  //for umask

static char gBootLogoPicPath[URL_LEN + 1];
static char gAuthLogoPicPath[URL_LEN + 1];
static char gStartLogoPicPath[URL_LEN + 1];
static char logoMd5Buff[4*MD5_DIGEST_LENGTH + 1] = "0";
static char authMd5Buff[4*MD5_DIGEST_LENGTH + 1] = "0";
static int gPicUpdateResult[3] = {0};

extern char* global_cookies;
extern "C" int yhw_upgrade_bootlogo(char* buf, int len);

#if defined(hi3560e)
#define IMAGESIZE_MAX 131072 //128 * 1024
#define IMAGEWIDTH_MAX 720
#define IMAGEHEIGHT_MAX 576
#else
#define IMAGESIZE_MAX 524288 //512 * 1024
#define IMAGEWIDTH_MAX  1280
#define IMAGEHEIGHT_MAX 720
#endif

static int
_LogoDataSafeCheck(char* buf, int len)
{
    // 1. check image data size
    if (len < 32 || len > IMAGESIZE_MAX) {
        LogSysOperWarn("Picture data size error [%d]\n", len);
        return -1;
    }

    // 2. check whether the image date had beend destroyed already.
    unsigned char* imgBuf = (unsigned char*)buf;
    int imgLen = len;
    unsigned long imgWidth = 0, imgHeight = 0;
    const char* tFilePath = DEFAULT_TEMP_DATAPATH"/_bootimage.tmp";
    FILE* wTempFp = NULL;
    LogSysOperDebug("[%02x %02x %02x %02x %02x %02x %02x %02x]\n", imgBuf[0], imgBuf[1], imgBuf[2], imgBuf[3], imgBuf[19], imgBuf[20], imgBuf[21], imgBuf[22]);
    if (imgBuf[0] == 'S' && imgBuf[1] == 'T' && imgBuf[2] == 'B' && imgBuf[3] == 0) {
        imgLen = (int)(((uint32_t)imgBuf[4] << 24) | ((uint32_t)imgBuf[5] << 16) | ((uint32_t)imgBuf[6] << 8) | ((uint32_t)imgBuf[7] << 0));
        imgBuf += 16;
    }
    wTempFp = fopen(tFilePath, "w+");
    if (!wTempFp)
        return -1;
    if (imgLen != (int)fwrite(imgBuf, 1, imgLen, wTempFp)) {
        LogSysOperWarn("write error!\n");
        fclose(wTempFp);
        unlink(tFilePath);
        return -1;
    }
    fclose(wTempFp);

    int ret = -1;
    switch (imgBuf[0]) {
        case 'B': //BMP
            ret = Hippo::BootImagesCheckBMP(tFilePath, &imgWidth, &imgHeight);
            break;
        case 'G': //GIF
            ret = Hippo::BootImagesCheckGIF(tFilePath, &imgWidth, &imgHeight);
            if(int_gif_check(imgBuf,imgLen))
                ret = -1;
            break;
        case 0xFF: //JPG
            ret = Hippo::BootImagesCheckJPG(tFilePath, &imgWidth, &imgHeight);
            if(int_jpg_check(imgBuf, imgLen))
                ret = -1;
            break;
#if 0 // 规范要求不支持png格式
        case 0x89: //PNG
            ret = Hippo::BootImagesCheckPNG(tFilePath, &imgWidth, &imgHeight);
            break;
#endif
        default:
            LogSysOperWarn("not support the type!\n");
    }
    unlink(tFilePath);
    if (0 != ret) {
        LogSysOperError("Picture check error\n");
        return -1;
    }

    // 3. check whether the image resolution is legal
    if (imgWidth > IMAGEWIDTH_MAX || imgHeight > IMAGEHEIGHT_MAX) {
        LogSysOperWarn("%u x %u Picture resolution is not supported\n", imgWidth, imgHeight);
        return -1;
    }
    return 0;
}
/**
 * @brief _HttpCallbackBootLogoPic
 *
 * @param result status of request
 * @param buf download data pointer
 * @param len download data length
 * @param arg sign of registration
 */
static void
_HttpCallbackBootLogoPic(int result, char* buf, int len, int arg)
{
    int updateResult = 0;

    if (HTTP_OK_READDATA != result || len <= 0) {
        //relocation
        if (HTTP_OK_LOCATION == result) {
            LogSysOperWarn("302 relocation:%s\n", buf);
            mid_http_call(buf, (mid_http_f)_HttpCallbackBootLogoPic, 0, NULL, 0, global_cookies);
            return ;
        }
        LogSysOperError("request error result[%d]\n", result);
        updateResult = UPGRADE_LOGO_RESULT_FAILURE;
        return ;
    }

    if (_LogoDataSafeCheck(buf, len))
    	  return ;
    //create MD5 digest and save to flash
    OpenSSL::MD5Crypto picMD5;
    std::string digest = picMD5.CreateDigest(buf, len);
    if (digest.empty() || (strcmp(logoMd5Buff, "no md5") && strcasecmp(logoMd5Buff, digest.c_str()))) { // EPG MD5 obtained in letters to lowercase. STB obtained are capitalized, so here is to ignore case.
        LogSysOperError("MD5 calculate\n");
        updateResult = UPGRADE_LOGO_RESULT_FAILURE;
        return ;
    }

    //save pic named by macro BOOT_LOGO_PATH(/root/bootlogo.gif)
    mode_t oldMask = umask(0077);
    FILE* pFileW = fopen(BOOT_LOGO_PATH, "wb");
    umask(oldMask);
    if (!pFileW) {
        LogSysOperError("fopen %s error\n", BOOT_LOGO_PATH);
        updateResult = UPGRADE_LOGO_RESULT_FAILURE;
        return ;
    }
    if (len != (int)fwrite(buf, 1, len, pFileW))
        LogSysOperWarn("something is wrong for fwrite\n");
    fflush(pFileW);
    fclose(pFileW);

    LogSysOperDebug("BootLogoPic save %s md5:%s\n", gBootLogoPicPath, digest.c_str());
    updateResult = UPGRADE_LOGO_RESULT_SUCCESS;
    appSettingSetString("PADBootLogPicURL", gBootLogoPicPath);
    sysSettingSetString("bootlogo_md5", digest.c_str());

    Hippo::BootImageLogoUpdateResultSet(updateResult, BOOT_PICTURE);

}/*}}}*/

/**
 * @brief _HttpCallbackAuthLogoPic
 *
 * @param result status of request
 * @param buf download data pointer
 * @param len download data length
 * @param arg sign of registration
 */
static void
_HttpCallbackAuthLogoPic(int result, char* buf, int len, int arg)
{
    int updateResult = 0;

    if (HTTP_OK_READDATA != result || len <= 0) {
        //relocation
        if (HTTP_OK_LOCATION == result) {
            LogSysOperWarn("302 relocation:%s\n", buf);
            mid_http_call(buf, (mid_http_f)_HttpCallbackAuthLogoPic, 0, NULL, 0, global_cookies);
            return ;
        }
        LogSysOperError("request error result[%d]\n", result);
        updateResult = UPGRADE_LOGO_RESULT_FAILURE;
        return ;
    }

    if (_LogoDataSafeCheck(buf, len))
    	  return ;
    //create MD5 digest and save to flash
    OpenSSL::MD5Crypto picMD5;
    std::string digest = picMD5.CreateDigest(buf, len);
    if (digest.empty() || (strcmp(authMd5Buff, "no md5") && strcasecmp(authMd5Buff, digest.c_str()))) { // EPG MD5 obtained in letters to lowercase. STB obtained are capitalized, so here is to ignore case.
        LogSysOperError("MD5 calculate\n");
        updateResult = UPGRADE_LOGO_RESULT_FAILURE;
        return ;
    }

    //save pic named by macro AUTH_LOGO_PATH(/root/authlogo.gif)
    mode_t oldMask = umask(0077);
    FILE* pFileW = fopen(AUTH_LOGO_PATH, "wb");
    umask(oldMask);
    if (!pFileW) {
        LogSysOperError("fopen %s error\n", AUTH_LOGO_PATH);
        updateResult = UPGRADE_LOGO_RESULT_FAILURE;
        return ;
    }
    if (len != (int)fwrite(buf, 1, len, pFileW))
        LogSysOperWarn("something is wrong for fwrite\n");
    fflush(pFileW);
    fclose(pFileW);

    LogSysOperDebug("AuthenPic save %s md5:%s\n", gBootLogoPicPath, digest.c_str());
    updateResult = UPGRADE_LOGO_RESULT_SUCCESS;
    appSettingSetString("PADAuthenBackgroundPicURL", gAuthLogoPicPath);
    sysSettingSetString("authbg_md5", digest.c_str());
    Hippo::BootImageLogoUpdateResultSet(updateResult, AUTH_PICTURE);
}

static void
_HttpCallbackStartLogoPic(int result, char * buf, int len, int arg)
{
    int updateResult = 0;

    if (HTTP_OK_READDATA != result || len <= 0) {
        if (HTTP_OK_LOCATION == result) {
            LogSysOperWarn("302 relocation:%s\n", buf);
            mid_http_call(buf, (mid_http_f)_HttpCallbackStartLogoPic, 0, NULL, 0, global_cookies);
            return ;
        }
        LogSysOperError("request error result[%d]\n", result);
        updateResult = UPGRADE_LOGO_RESULT_FAILURE;
        return ;
    }

    if (_LogoDataSafeCheck(buf, len))
    	  return ;

    int ret = yhw_upgrade_bootlogo(buf, len);
	if (ret == 0)
	    updateResult = UPGRADE_LOGO_RESULT_SUCCESS;
	else
	    updateResult = UPGRADE_LOGO_RESULT_FAILURE;
    LogSysOperDebug("AuthenPic save %s\n", gStartLogoPicPath);
    appSettingSetString("startPicUpgradeURL", gBootLogoPicPath);
    Hippo::BootImageLogoUpdateResultSet(updateResult, START_PICTURE);
}

namespace Hippo {
std::map<std::string, std::string> gLogoInfo;
int
BootImagesUrlMd5(const char* url, char* buf)
{
    if(!url)
        return -1;

    char* pStr = NULL;
    char value[URL_LEN + 1] = {0};
    strncpy(value, url, URL_LEN);

    if(!(pStr = strstr(value, "MD5="))) {
        strcpy(buf, "no md5");
        LogSysOperWarn("Picture URL have not MD5\n");
        return 0;
    }
    memcpy(buf, pStr + 4, 4 * MD5_DIGEST_LENGTH);
    if(!strlen(buf))
        return -1;
    return 0;
}

/**
 * @brief BootImagesDownloadBootLogo
 *
 * @param path /EPG/jsp/ad/bootlog.gif
 */
int
BootImagesDownloadBootLogo(const char* path)
{/*{{{*/
    if (!path)
        return -1;

    char value[URL_LEN+1] = {0};
    char epgURL[URL_LEN + 1] = {0};
    char *pStr = NULL;

    strncpy(gBootLogoPicPath, path, URL_LEN - 1);
    strncpy(value, path, URL_LEN - 1);
    LogUserOperDebug("PADBootLogPicURL:%s\n", value);
    memset(logoMd5Buff, 0, 4 * MD5_DIGEST_LENGTH + 1);
    if(BootImagesUrlMd5(value, logoMd5Buff))
        return -1;

    // 虽然不支持https，但是也不要在这个地方过滤掉，先透下去吧。
    // 什么时候换成curl可能就支持了。
    if (strncasecmp(value, "http://", 7) != 0 && strncasecmp(value, "https://", 8) != 0) {
        std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();
        if (url.empty())
            return -1;
        url += std::string(path);
        snprintf(value, sizeof(value), "%s", url.c_str());
    }
    LogUserOperDebug("strcat url ok! value = %s\n", value);
    return mid_http_call(value, (mid_http_f)_HttpCallbackBootLogoPic, 0, NULL, 0, global_cookies);
}/*}}}*/

/**
 * @brief BootImagesDownloadAuthLogo
 *
 * @param path /EPG/jsp/ad/authlog.gif
 */
int
BootImagesDownloadAuthLogo(const char* path)
{/*{{{*/
    if (!path)
        return -1;

    char value[URL_LEN+1] = {0};
    char epgURL[URL_LEN + 1] = {0};
    char *pStr = NULL;

    strncpy(gAuthLogoPicPath, path, URL_LEN - 1);
    strncpy(value, path, URL_LEN - 1);
    LogUserOperDebug("Func:%s,PADAuthenBackgroundPicURL:%s\n", __FUNCTION__, value);
    memset(authMd5Buff, 0, 4*MD5_DIGEST_LENGTH+1);
    if(BootImagesUrlMd5(value, authMd5Buff))
        return -1;

    // 虽然不支持https，但是也不要在这个地方过滤掉，先透下去吧。
    // 什么时候换成curl可能就支持了。
    if (strncasecmp(value, "http://", 7) != 0 && strncasecmp(value, "https://", 8) != 0) {
        std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();
        if (url.empty())
            return -1;
        url += std::string(path);
        snprintf(value, sizeof(value), "%s", url.c_str());
    }
    LogUserOperDebug("strcat url ok! value = %s\n", value);
    return mid_http_call(value, (mid_http_f)_HttpCallbackAuthLogoPic, 0, NULL, 0, global_cookies);
}/*}}}*/



int
BootImagesDownloadStartLogo(const char* path)
{
    if (!path)
        return -1;

    char value[URL_LEN+1] = {0};
    char epgURL[URL_LEN + 1] = {0};
    char *pStr = NULL;

    strncpy(gStartLogoPicPath, path, URL_LEN - 1);
    strncpy(value, path, URL_LEN - 1);
    LogUserOperDebug("StartPicURL:%s\n",  value);

    if (strncasecmp(value, "http://", 7) != 0 && strncasecmp(value, "https://", 8) != 0) {
        std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();
        if (url.empty())
            return -1;
        url += std::string(path);
        snprintf(value, sizeof(value), "%s", url.c_str());
    }
    LogUserOperDebug("strcat url ok! value = %s\n", value);
    return mid_http_call(value, (mid_http_f)_HttpCallbackStartLogoPic, 0, NULL, 0, global_cookies);

}

void
BootImageLogoUpdateResultSet(int result, int upgradeType)
{
    gPicUpdateResult[upgradeType - 1] = result;

    // "Event.Parameter"在TR069_VERSION_1没有，会跳过；
    {
        int eventID = TR069_API_SETVALUE("Event.Regist", "LogoUpdateResult", 0);
        TR069_API_SETVALUE("Event.Parameter", "Device.UserInterface.Logo.X_CT-COM_StartPic_Result", eventID);
        TR069_API_SETVALUE("Event.Parameter", "Device.UserInterface.Logo.X_CT-COM_BootPic_Result", eventID);
        TR069_API_SETVALUE("Event.Parameter", "Device.UserInterface.Logo.X_CT-COM_AuthenticatePic_Result", eventID);
        TR069_API_SETVALUE("Event.Post", "", eventID);
    }
}

int
BootImageLogoUpdateResultGet(int upgradeType)
{
    if (upgradeType < START_PICTURE || upgradeType > AUTH_PICTURE)
        return -1;

    return gPicUpdateResult[upgradeType - 1];
}

} //End of namespace Hippo


