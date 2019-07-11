#include "Tr069UserInterface.h"

#include "Tr069GroupCall.h"
#include "Tr069FunctionCall.h"
#include "Tr069Logo.h"
#include "SysSetting.h"
#include "SettingEnum.h"
#include <string.h>
#include <stdio.h>


/*------------------------------------------------------------------------------
	RGB表示的GUI屏幕背景颜色
 ------------------------------------------------------------------------------*/
static
int tr069_BackgroundColor_Read(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_BackgroundColor_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	只有当终端提供密码保护的 LAN 侧用户接口时，此参数
	才适用。指示本地用户接口是否必须要求密码，以便
	供用户选择。如果此参数为假，则由用户来选择是否
	使用密码。
 ------------------------------------------------------------------------------*/
static
int tr069_PasswordRequired_Read(char *str, unsigned int length)
{
    return 0;
}
static
int tr069_PasswordRequired_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	只有当终端提供密码保护的 LAN 侧用户接口并支持 LAN 侧
	自动配置时，此参数才适用。指示用户是否可以直接
	选择密码以保护终端的本地用户接口，或者必须使用
	与 LAN 侧自动配置协议相同的密码。
 ------------------------------------------------------------------------------*/
static
int tr069_PasswordUserSelectable_Read(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_PasswordUserSelectable_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	指示某个终端升级是可用的，允许终端将此信息显示
	给用户。
 ------------------------------------------------------------------------------*/
static
int tr069_UpgradeAvailable_Read(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_UpgradeAvailable_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	指示与终端相关的保修期过期的日期和时间。
 ------------------------------------------------------------------------------*/
static
int tr069_WarrantyDate_Read(char *str, unsigned int length)
{
    snprintf(str, length, "%d", 20121221);
    return 0;
}

static
int tr069_WarrantyDate_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	客户的 ISP 名称。
 ------------------------------------------------------------------------------*/
static
int tr069_ISPName_Read(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_ISPName_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	ISP 的咨询电话号码。
 ------------------------------------------------------------------------------*/
static
int tr069_ISPHelpDesk_Read(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_ISPHelpDesk_Write(char *str, unsigned int length)
{
    return 0;
}


/*------------------------------------------------------------------------------
	ISP 主页的 URL。
 ------------------------------------------------------------------------------*/
static
 int tr069_ISPHomePage_Read(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_ISPHomePage_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	ISP 在线支持页面的 URL。
 ------------------------------------------------------------------------------*/
static
int tr069_ISPHelpPage_Read(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_ISPHelpPage_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	Base64编码的Gif或JPEG图像。二进制图像大小限制为4095k以下
 ------------------------------------------------------------------------------*/
static
int tr069_ISPLogo_Read(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_ISPLogo_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	未编码的二进制图像大小(K)。如果ISPLogoSize输入值为0，
	则ISPLogo被清除。ISPLogoSize也能够用于图片的检测图片的
	正确传输与Base64字符串到图片大小的转换。
 ------------------------------------------------------------------------------*/
static
int tr069_ISPLogoSize_Read(char *str, unsigned int length)
{
    snprintf(str, length, "%d", 2048);
    return 0;
}

static
int tr069_ISPLogoSize_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	ISP 邮件服务器的 URL。
 ------------------------------------------------------------------------------*/
static
int tr069_ISPMailServer_Read(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_ISPMailServer_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	ISP 新闻服务器的 URL。
 ------------------------------------------------------------------------------*/
static
int tr069_ISPNewsServer_Read(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_ISPNewsServer_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	RGB表示的GUI屏幕文字颜色
 ------------------------------------------------------------------------------*/
static
int tr069_TextColor_Read(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_TextColor_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	RGB表示的GUI屏幕按钮颜色
 ------------------------------------------------------------------------------*/
static
int tr069_ButtonColor_Read(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_ButtonColor_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	RGB表示的GUI屏幕按钮文字颜色
 ------------------------------------------------------------------------------*/
static
int tr069_ButtonTextColor_Read(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_ButtonTextColor_Write(char *str, unsigned int length)
{
    return 0;
}


/*------------------------------------------------------------------------------
	用户通过浏览器检查是否有最新的更新并下载到PC上的
	服务器地址。当Device.ManagementServer.UpgradesManaged为true时，
	不能使用本参数。
 ------------------------------------------------------------------------------*/
static
int tr069_UserUpdateServer_Read(char *str, unsigned int length)
{
    return 0;
}

static
int tr069_UserUpdateServer_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	必须贯穿机顶盒重启的任意用户数据。
 ------------------------------------------------------------------------------*/
static
int tr069_PersistentData_Read(char *str, unsigned int length)
{
    strncpy(str, "xiangyang", length);
    return 0;
}

static
int tr069_PersistentData_Write(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	读取当前频道描述
 ------------------------------------------------------------------------------*/
static
int tr069_ServiceName_Read(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	读取当前流服务器IP
 ------------------------------------------------------------------------------*/
static
int tr069_StreamServerIP_Read(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	读取节点信令调度服务器IP
 ------------------------------------------------------------------------------*/
static
int tr069_CDNAgentIP_Read(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	读取升级URL
 ------------------------------------------------------------------------------*/
static int getTr069PortAutoUpdateServer(char* str, unsigned int val)
{
    sysSettingGetString("upgradeUrl", str, val, 0);

    return 0;
}

/*------------------------------------------------------------------------------
	更新升级URL
 ------------------------------------------------------------------------------*/
static int setTr069PortAutoUpdateServer(char* str, unsigned int val)
{
    sysSettingSetString("upgradeUrl", str);

    return 0;
}

/*------------------------------------------------------------------------------
	可选语种
 ------------------------------------------------------------------------------*/
static int getTr069PortAvailableLanguages(char* str, unsigned int val)
{
#if defined(STBTYPE_QTEL)
    strncpy(str, "ar,en", val);
#else
    strncpy(str, "zh,en", val);
#endif

    return 0;
}

/*------------------------------------------------------------------------------
	当前语言
 ------------------------------------------------------------------------------*/
static int getTr069PortCurrentLanguage(char* str, unsigned int val)
{
    int lang;
    sysSettingGetInt("lang", &lang, 0);
    if (LANGUAGE_ENGLISH == lang) {
      strncpy(str, "en", val);
    } else {
#if defined(STBTYPE_QTEL)
        strncpy(str, "ar", val);
#else
        strncpy(str, "zh", val);
#endif
    }

    return 0;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
static int setTr069PortCurrentLanguage(char* str, unsigned int val)
{
    int lang;
    if (!str || !strlen(str)) {
        return 0;
    }
    if (strcasecmp(str, "en") == 0) {
        lang = LANGUAGE_ENGLISH;
    } else
        lang = LANGUAGE_LOCAL;
    sysSettingSetInt("lang", lang);
    return 0;
}

Tr069UserInterface::Tr069UserInterface()
	: Tr069GroupCall("UserInterface")
{
    // regist nodes
//#ifdef TR069_UPGRADE_LOGO
    Tr069Call* logo1  = new Tr069Logo();

    regist(logo1->name(), logo1);
//#endif
    /// regist functions
    Tr069Call* bgc  = new Tr069FunctionCall("BackgroundColor",                 tr069_BackgroundColor_Read,            tr069_BackgroundColor_Write);
    Tr069Call* passr  = new Tr069FunctionCall("PasswordRequired",              tr069_PasswordRequired_Read,           tr069_PasswordRequired_Write);
    Tr069Call* passsel  = new Tr069FunctionCall("PasswordUserSelectable",      tr069_PasswordUserSelectable_Read,     tr069_PasswordUserSelectable_Write);
    Tr069Call* upavla  = new Tr069FunctionCall("UpgradeAvailable",             tr069_UpgradeAvailable_Read,           tr069_UpgradeAvailable_Write);
    Tr069Call* warrant  = new Tr069FunctionCall("WarrantyDate",                tr069_WarrantyDate_Read,               tr069_WarrantyDate_Write);
    Tr069Call* name  = new Tr069FunctionCall("ISPName",                        tr069_ISPName_Read,                    tr069_ISPName_Write);
    Tr069Call* desk  = new Tr069FunctionCall("ISPHelpDesk",                    tr069_ISPHelpDesk_Read,                tr069_ISPHelpDesk_Write);
    Tr069Call* home  = new Tr069FunctionCall("ISPHomePage",                    tr069_ISPHomePage_Read,                tr069_ISPHomePage_Write);
    Tr069Call* help  = new Tr069FunctionCall("ISPHelpPage",                    tr069_ISPHelpPage_Read,                tr069_ISPHelpPage_Write);
    Tr069Call* logo  = new Tr069FunctionCall("ISPLogo",                        tr069_ISPLogo_Read,                    tr069_ISPLogo_Write);
    Tr069Call* size  = new Tr069FunctionCall("ISPLogoSize",                    tr069_ISPLogoSize_Read,                tr069_ISPLogoSize_Write);
    Tr069Call* mail  = new Tr069FunctionCall("ISPMailServer",                  tr069_ISPMailServer_Read,              tr069_ISPMailServer_Write);
    Tr069Call* news  = new Tr069FunctionCall("ISPNewsServer",                  tr069_ISPNewsServer_Read,              tr069_ISPNewsServer_Write);
    Tr069Call* textclor  = new Tr069FunctionCall("TextColor",                  tr069_TextColor_Read,                  tr069_TextColor_Write);
    Tr069Call* buttclor  = new Tr069FunctionCall("ButtonColor",                tr069_ButtonColor_Read,                tr069_ButtonColor_Write);
    Tr069Call* butttxt  = new Tr069FunctionCall("ButtonTextColor",             tr069_ButtonTextColor_Read,            tr069_ButtonTextColor_Write);
    Tr069Call* server  = new Tr069FunctionCall("UserUpdateServer",             tr069_UserUpdateServer_Read,           tr069_UserUpdateServer_Write);
    Tr069Call* persis  = new Tr069FunctionCall("PersistentData",               tr069_PersistentData_Read,             tr069_PersistentData_Write);
    Tr069Call* cdn  = new Tr069FunctionCall("CDNAgentIP",                      tr069_CDNAgentIP_Read,                 NULL);
    Tr069Call* servip  = new Tr069FunctionCall("StreamServerIP",               tr069_StreamServerIP_Read,             NULL);
    Tr069Call* service  = new Tr069FunctionCall("ServiceName",                 tr069_ServiceName_Read,                NULL);

    regist(cdn->name(), cdn);
    regist(servip->name(), servip);
    regist(service->name(), service);
    regist(bgc->name(), bgc);
    regist(passr->name(), passr);
    regist(passsel->name(), passsel);
    regist(upavla->name(), upavla);
    regist(warrant->name(), warrant);
    regist(name->name(), name);
    regist(desk->name(), desk);
    regist(home->name(), home);
    regist(help->name(), help);
    regist(logo->name(), logo);
    regist(size->name(), size);
    regist(mail->name(), mail);
    regist(news->name(), news);
    regist(textclor->name(), textclor);
    regist(buttclor->name(), buttclor);
    regist(butttxt->name(), butttxt);
    regist(server->name(), server);
    regist(persis->name(), persis);


    //tr106_UserInterface_init
    Tr069Call* AutoUpdateServer  = new Tr069FunctionCall("AutoUpdateServer",       getTr069PortAutoUpdateServer,       setTr069PortAutoUpdateServer);
    Tr069Call* AvailableLanguages  = new Tr069FunctionCall("AvailableLanguages",   getTr069PortAvailableLanguages,     NULL);
    Tr069Call* CurrentLanguage  = new Tr069FunctionCall("CurrentLanguage",         getTr069PortCurrentLanguage,        setTr069PortCurrentLanguage);

    regist(AutoUpdateServer->name(), AutoUpdateServer);
    regist(AvailableLanguages->name(), AvailableLanguages);
    regist(CurrentLanguage->name(), CurrentLanguage);

}

Tr069UserInterface::~Tr069UserInterface()
{
}
