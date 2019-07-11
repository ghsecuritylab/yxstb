#include "Tr069UserInterface.h"

#include "Tr069GroupCall.h"
#include "Tr069FunctionCall.h"
#include "Tr069Logo.h"
#include "SysSetting.h"
#include "SettingEnum.h"
#include <string.h>
#include <stdio.h>


/*------------------------------------------------------------------------------
	RGB��ʾ��GUI��Ļ������ɫ
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
	ֻ�е��ն��ṩ���뱣���� LAN ���û��ӿ�ʱ���˲���
	�����á�ָʾ�����û��ӿ��Ƿ����Ҫ�����룬�Ա�
	���û�ѡ������˲���Ϊ�٣������û���ѡ���Ƿ�
	ʹ�����롣
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
	ֻ�е��ն��ṩ���뱣���� LAN ���û��ӿڲ�֧�� LAN ��
	�Զ�����ʱ���˲��������á�ָʾ�û��Ƿ����ֱ��
	ѡ�������Ա����ն˵ı����û��ӿڣ����߱���ʹ��
	�� LAN ���Զ�����Э����ͬ�����롣
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
	ָʾĳ���ն������ǿ��õģ������ն˽�����Ϣ��ʾ
	���û���
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
	ָʾ���ն���صı����ڹ��ڵ����ں�ʱ�䡣
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
	�ͻ��� ISP ���ơ�
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
	ISP ����ѯ�绰���롣
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
	ISP ��ҳ�� URL��
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
	ISP ����֧��ҳ��� URL��
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
	Base64�����Gif��JPEGͼ�񡣶�����ͼ���С����Ϊ4095k����
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
	δ����Ķ�����ͼ���С(K)�����ISPLogoSize����ֵΪ0��
	��ISPLogo�������ISPLogoSizeҲ�ܹ�����ͼƬ�ļ��ͼƬ��
	��ȷ������Base64�ַ�����ͼƬ��С��ת����
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
	ISP �ʼ��������� URL��
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
	ISP ���ŷ������� URL��
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
	RGB��ʾ��GUI��Ļ������ɫ
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
	RGB��ʾ��GUI��Ļ��ť��ɫ
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
	RGB��ʾ��GUI��Ļ��ť������ɫ
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
	�û�ͨ�����������Ƿ������µĸ��²����ص�PC�ϵ�
	��������ַ����Device.ManagementServer.UpgradesManagedΪtrueʱ��
	����ʹ�ñ�������
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
	����ᴩ�����������������û����ݡ�
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
	��ȡ��ǰƵ������
 ------------------------------------------------------------------------------*/
static
int tr069_ServiceName_Read(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	��ȡ��ǰ��������IP
 ------------------------------------------------------------------------------*/
static
int tr069_StreamServerIP_Read(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	��ȡ�ڵ�������ȷ�����IP
 ------------------------------------------------------------------------------*/
static
int tr069_CDNAgentIP_Read(char *str, unsigned int length)
{
    return 0;
}

/*------------------------------------------------------------------------------
	��ȡ����URL
 ------------------------------------------------------------------------------*/
static int getTr069PortAutoUpdateServer(char* str, unsigned int val)
{
    sysSettingGetString("upgradeUrl", str, val, 0);

    return 0;
}

/*------------------------------------------------------------------------------
	��������URL
 ------------------------------------------------------------------------------*/
static int setTr069PortAutoUpdateServer(char* str, unsigned int val)
{
    sysSettingSetString("upgradeUrl", str);

    return 0;
}

/*------------------------------------------------------------------------------
	��ѡ����
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
	��ǰ����
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
