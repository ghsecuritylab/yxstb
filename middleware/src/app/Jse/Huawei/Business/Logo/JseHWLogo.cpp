
#include "JseHWLogo.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "BootImagesShow.h"
#include "BootImagesDownload.h"

#include "AppSetting.h"
#include "sys_basic_macro.h"

#include <stdlib.h>
#include <string.h>

//TODO
static int JsePADLoaderLogoPicURLWrite(const char *param, char *value, int len)
{
    return 0;
}

static int JsePADBootLogPicURLRead(const char *param, char *value, int len)
{
    appSettingGetString("PADBootLogPicURL", value, len - 1, 0);
    return 0;
}

/**
 * @brief _JseW_PADBootLogPicURL set boot logo picture download url
 *          # If download success, save to STB flash
 *          # If URL is NULL, show default boot logo when STB reboot
 *          # Pic URL value is a relative path, eg: /EPG/jsp/ad/bootlog.gif
 *          # Use the full path EPG-URL for requesting new picture, eg: http://192.168.3.48:8082/EPG/jsp/ad/bootlog.gif
 *
 * @param func "PADBootLogPicURL"
 * @param param NULL
 * @param value URL of bootlogo Picture or NULL
 * @param len 4096
 *      eg:Utility.setValueByName('PADBootLogPicURL','/EPG/jsp/ad/bootlog.gif');
 * @return -1 on error, 0 on success
 */
static int JsePADBootLogPicURLWrite(const char *param, char *value, int len)
{
    LogJseDebug("value[%s]\n", value);
    char picURL[URL_LEN + 1] = { 0 };
	appSettingGetString("PADBootLogPicURL", picURL, URL_LEN, 0);
    //If the new URL(picURL != value) then update
    if (strcmp(picURL, value))
        Hippo::BootImagesDownloadBootLogo(value);
    return 0;
}

static int JsePADAuthenBackgroundPicURLRead(const char *param, char *value, int len)
{
    appSettingGetString("PADAuthenBackgroundPicURL", value, len - 1, 0);
    return 0;
}

/**
 * @brief _JseW_PADAuthenBackgroundPicURL set authen background picture download url
 *          # If download success, save to STB flash
 *          # If URL is NULL, show default authen logo when STB reboot
 *          # Pic URL value is a relative path, eg: /EPG/jsp/ad/authlog.gif
 *          # Use the full path EPG-URL for requesting new picture, eg: http://192.168.3.48:8082/EPG/jsp/ad/authlog.gif
 *
 * @param func "PADAuthenBackgroundPicURL"
 * @param param NULL
 * @param value URL of Authen background picture or NULL
 * @param len 4096
 *      eg: Utility.setValueByName('PDAuthenBackgroundPicURL','/EPG/jsp/ad/authlog.gif);
 * @return
 */
static int JsePADAuthenBackgroundPicURLWrite(const char *param, char *value, int len)
{
    LogJseDebug("value[%s]\n", value);
    char picURL[URL_LEN + 1] = { 0 };
    appSettingGetString("PADAuthenBackgroundPicURL", picURL, URL_LEN, 0);
    //If the new URL(picURL != value) then update
    if (strcmp(picURL, value)) // Domestic platform issued boot logo url will not change. But he didn't update specifications different urls.
        Hippo::BootImagesDownloadAuthLogo(value);
    return 0;
}

/**
 * @brief _JseW_ShowPic  set authen picture show switch
 *          # 1 - STB show Authen Pic
 *          # 2 - STB don't show Pic (default)
 * @param func "ShowPic"
 * @param param NULL
 * @param value "1" or "2"
 * @param len 4096
 *      eg: Utility.setValueByName('ShowPic', '1');
 * @return -1 on error, 0 on success
 */
static int JseShowPicWrite(const char *param, char *value, int len)
{
    LogJseDebug("value[%s]\n", value);
    switch(atoi(value)) {
    case 1:
        BootImagesShowAuthLogo(1);
        break;
    case 3:
        ImageZoomShowBackground(1);
        break;
    case 4:
        ImageZoomShowBackground(0);
        break;
    case 2:
    default:
        BootImagesShowAuthLogo(0);
        break;
    }
    return 0;
}

static int JseShowPicRead(const char *param, char *value, int len)
{
    int authShowPicFlag = 0;
    appSettingGetInt("AuthShowPicFlag", &authShowPicFlag, 0);

    sprintf(value, "%d", authShowPicFlag);

    return 0;
}

/*************************************************
Description: 初始化华为Logo定义的接口，由JseHWBusiness.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWLogoInit()
{
    JseCall* call;

    //C10 regist
    call = new JseFunctionCall("PADLoaderLogoPicURL", 0, JsePADLoaderLogoPicURLWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("PADBootLogPicURL", JsePADBootLogPicURLRead, JsePADBootLogPicURLWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("PADAuthenBackgroundPicURL", JsePADAuthenBackgroundPicURLRead, JsePADAuthenBackgroundPicURLWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("ShowPic", JseShowPicRead, JseShowPicWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

