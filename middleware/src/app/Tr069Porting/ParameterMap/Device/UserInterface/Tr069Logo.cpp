#include "Tr069Logo.h"

#include "Tr069GroupCall.h"
#include "Tr069FunctionCall.h"
#include "AppSetting.h"
#include "Assertions.h"
#include "BootImagesDownload.h"

#include <stdlib.h>
#include <stdio.h>

//#ifdef TR069_UPGRADE_LOGO

static
int tr069_StartPicURL_Read(char *str, unsigned int length)
{
    if(appSettingGetString("startPicUpgradeURL", str, length, 0) == -1)
        ERR_OUT("GET STARTPIC URL FAILED!");
    return 0;
Err:
    return -1;
}

static
int tr069_StartPicURL_Write(char *str, unsigned int length)
{
#if defined(Jiangsu)
    return 0;
#endif

#if 0
    // 逻辑有问题，应该是升级成功之后才能将URL的地址写入flash
    int ret = -1;
    ret = appSettingSetString("startPicUpgradeURL", str);
    if(ret == -1)
        ERR_OUT("SET STARTPIC URL FAILED!");
#endif

    Hippo::BootImagesDownloadStartLogo(str);
    return 0;
Err:
    return -1;
}

static
int tr069_StartPicEnable_Read(char *str, unsigned int length)
{
    int startPicEnable = 0;
    appSettingGetInt("startPicEnableFlag", &startPicEnable, 0);
    sprintf(str, "%u", startPicEnable);
    return 0;
}

static
int tr069_StartPicEnable_Write(char *str, unsigned int length)
{
    int ret = -1;
    ret = appSettingSetInt("startPicEnableFlag", atoi(str));
    settingManagerSave();
    if(ret == -1)
        ERR_OUT("SET STARTPIC ENABLE VALUE FAILED!");
    return 0;
Err:
    return -1;

}

static
int tr069_StartPicResult_Read(char *str, unsigned int length)
{
    unsigned int ret = (unsigned int)Hippo::BootImageLogoUpdateResultGet(START_PICTURE);
    sprintf(str, "%u", ret);
    return 0;
}

static
int tr069_StartPicResult_Write(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_BootPicURL_Read(char *str, unsigned int length)
{
    int ret = -1;
    ret = appSettingGetString("PADBootLogPicURL", str, length, 0);
    if(ret == -1)
        ERR_OUT("GET BOOTPIC UPGRADE URL FAILED!");
    return 0;
Err:
	return -1;

}

static
int tr069_BootPicURL_Write(char *str, unsigned int length)
{

    Hippo::BootImagesDownloadBootLogo(str);
    return 0;

}

static
int tr069_BootPicEnable_Read(char *str, unsigned int length)
{
    int bootPicEnable = 0;
	appSettingGetInt("bootPicEnableFlag", &bootPicEnable, 0);
    sprintf(str, "%u", bootPicEnable);
    return 0;
}

static
int tr069_BootPicEnable_Write(char *str, unsigned int length)
{
    int ret = -1;

    ret = appSettingSetInt("bootPicEnableFlag", atoi(str));
    settingManagerSave();
    if(ret == -1)
        ERR_OUT("SET BOOTPIC ENABLE FAILED!");
    return 0;
Err:
    return -1;

}

static
int tr069_BootPicResult_Read(char *str, unsigned int length)
{
    unsigned int ret = (unsigned int)Hippo::BootImageLogoUpdateResultGet(BOOT_PICTURE);
    sprintf(str, "%u", ret);
    return 0;
}

static
int tr069_BootPicResult_Write(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_AuthenticatePicURL_Read(char *str, unsigned int length)
{
    int ret = -1;

    ret = appSettingGetString("PADAuthenBackgroundPicURL", str, length, 0);
    if(ret == -1)
        ERR_OUT("GET AUTHPIC UPGRADE URL FAILED!");
    return 0;

Err:
    return -1;

}

static
int tr069_AuthenticatePicURL_Write(char *str, unsigned int length)
{

    Hippo::BootImagesDownloadAuthLogo(str);
    return 0;

}

static
int tr069_AuthenticatePicEnable_Read(char *str, unsigned int length)
{
    int authPicEnable = 0;
	appSettingGetInt("AuthShowPicFlag", &authPicEnable, 0);
    sprintf(str, "%u", authPicEnable);
    return 0;
}

static
int tr069_AuthenticatePicEnable_Write(char *str, unsigned int length)
{
    int ret = 0;

    ret = appSettingSetInt("AuthShowPicFlag", atoi(str));
    settingManagerSave();
    if(ret == -1)
        ERR_OUT("SET AUTH UPGRADE URL FAILED!");
    return 0;
Err:
    return -1;

}

static
int tr069_AuthenticatePicResult_Read(char *str, unsigned int length)
{
    unsigned int ret = (unsigned int)Hippo::BootImageLogoUpdateResultGet(AUTH_PICTURE);
    sprintf(str, "%u", ret);
    return 0;
}

static
int tr069_AuthenticatePicResult_Write(char *str, unsigned int length)
{
    return 0;
}
//#endif // TR069_UPGRADE_LOGO

Tr069Logo::Tr069Logo()
	: Tr069GroupCall("Logo")
{
//#ifdef TR069_UPGRADE_LOGO
    Tr069Call* startUrl  = new Tr069FunctionCall("X_CT-COM_StartPicURL",            tr069_StartPicURL_Read,            tr069_StartPicURL_Write);
    Tr069Call* pinEn  = new Tr069FunctionCall("X_CT-COM_StartPic_Enable",           tr069_StartPicEnable_Read,         tr069_StartPicEnable_Write);
    Tr069Call* startRes  = new Tr069FunctionCall("X_CT-COM_StartPic_Result",        tr069_StartPicResult_Read,         tr069_StartPicResult_Write);
    Tr069Call* bootUrl  = new Tr069FunctionCall("X_CT-COM_BootPicURL",              tr069_BootPicURL_Read,             tr069_BootPicURL_Write);
    Tr069Call* bootEn  = new Tr069FunctionCall("X_CT-COM_BootPic_Enable",           tr069_BootPicEnable_Read,          tr069_BootPicEnable_Write);
    Tr069Call* bootRes  = new Tr069FunctionCall("X_CT-COM_BootPic_Result",          tr069_BootPicResult_Read,          tr069_BootPicResult_Write);
    Tr069Call* authUrl  = new Tr069FunctionCall("X_CT-COM_AuthenticatePicURL",      tr069_AuthenticatePicURL_Read,     tr069_AuthenticatePicURL_Write);
    Tr069Call* authEn  = new Tr069FunctionCall("X_CT-COM_AuthenticatePic_Enable",   tr069_AuthenticatePicEnable_Read,  tr069_AuthenticatePicEnable_Write);
    Tr069Call* authRes  = new Tr069FunctionCall("X_CT-COM_AuthenticatePic_Result",  tr069_AuthenticatePicResult_Read,  tr069_AuthenticatePicResult_Write);

    regist(startUrl->name(), startUrl);
    regist(pinEn->name(), pinEn);
    regist(startRes->name(), startRes);
    regist(bootUrl->name(), bootUrl);
    regist(bootEn->name(), bootEn);
    regist(bootRes->name(), bootRes);
    regist(authUrl->name(), authUrl);
    regist(authEn->name(), authEn);
    regist(authRes->name(), authRes);
//#endif
}

Tr069Logo::~Tr069Logo()
{
}
