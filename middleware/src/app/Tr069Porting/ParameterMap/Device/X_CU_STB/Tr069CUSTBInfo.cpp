#include "Tr069CUSTBInfo.h"

#include "Tr069FunctionCall.h"
#include "TR069Assertions.h"

#include "SysSetting.h"
#include "AppSetting.h"

#include "mid_sys.h"
#include "record_port.h"

#include "record_port.h"
#include "SysSetting.h"
#include "tr069_port1.h"
#include "cryptoFunc.h"
#include "openssl/evp.h"
#include <string.h>

#define IND_STRCPY(dest, src)		strcpy(dest, src)
#define IND_STRNCPY(dest, src, n)	strncpy(dest, src, n)
#define IND_MEMSET(s, ch, n)		memset(s, ch, n)

/*------------------------------------------------------------------------------
	STBID是由32位16进制的数字组成，是机顶盒的唯一标识码。
	格式为：AA BBBB CCCCCC DDD E FFFF GGGGGGGGGGGG
	AA		表示整个字符串的校验码，MD5校验
	BBBB	第1、2位运营商编号，后2位为终端类型
	CCCCCC	终端厂商认证编号，暂时由运营商分配
	DDD				终端型号
	E				终端的识别符类型
	FFFF			终端批次
	GGGGGGGGGGGG	终端的识别符
 ------------------------------------------------------------------------------*/
static int tr069_STBID_Read(char* value, unsigned int length)
{

    if(value != NULL)
        mid_sys_serial(value);

    return 0;
}


/*------------------------------------------------------------------------------
	总RAM大小，单位KB
 ------------------------------------------------------------------------------*/
static int tr069_PhyMemSize_Read(char* value, unsigned int length)
{
    if(value) {
        sprintf(value, "262144");
    }

    return 0;
}

/*------------------------------------------------------------------------------
	存储大小，单位KB
 ------------------------------------------------------------------------------*/
static int tr069_StorageSize_Read(char* value, unsigned int length)
{
    unsigned int totalblock = 0, freeblock = 0;
    int ret = -1;

    if(value) {
        //extern int record_port_disk_size(u_int* ptotalblock, u_int* pfreeblock);
        ret = record_port_disk_size(&totalblock, &freeblock);
        if(!ret)
            sprintf(value, "%u", totalblock * 1024);
        else
            sprintf(value, "0");
    }

    return 0;
}

static char gOperatorInfo[5] = "CU";
static int tr069_OperatorInfo_Read(char* value, unsigned int length)
{
    IND_STRCPY(value, gOperatorInfo);
    LogTr069Debug("value = [%s]\n", value);

    return 0;
}

static int tr069_OperatorInfo_Write(char* value, unsigned int length)
{
    if(!value || !strlen(value))
    	return -1;

    LogTr069Debug("value = [%s]\n", value);
    IND_STRCPY(gOperatorInfo, value);

    return 0;
}

static int tr069_IntegrityCheck_Read(char* value, unsigned int length)
{
    value[0] = 0;
    LogTr069Debug("value = [%s]\n", value);
    return 0;
}

static int tr069_UpgradeURL_Read(char* value, unsigned int length)
{
    sysSettingGetString("upgradeUrl", value, length, 0);
    return 0;
}

static int tr069_UpgradeURL_Write(char* value, unsigned int length)
{
    if (!value || !strlen(value))
        return -1;

    sysSettingSetString("upgradeUrl", value);
    return 0;
}

static int tr069_BrowserURL1_Read(char* value, unsigned int length)
{
    if(value)
        sysSettingGetString("eds", value, length, 0);
    return 0;
}

static int tr069_BrowserURL1_Write(char* value, unsigned int length)
{
    if(value && (strncmp(value, "http://", 7) == 0)) {
        sysSettingSetString("eds", value);
        LogTr069Debug("value = [%s]\n", value);
    }
    return 0;
}

static int tr069_BrowserURL2_Read(char* value, unsigned int length)
{
    if(value)
        sysSettingGetString("eds1", value, length, 0);
    return 0;
}

static int tr069_BrowserURL2_Write(char* value, unsigned int length)
{
    if(value && (strncmp(value, "http://", 7) == 0)) {
        sysSettingSetString("eds1", value);
        LogTr069Debug("value = [%s]\n", value);
    }
    return 0;
}

static int tr069_BrowserTagURL_Read(char* value, unsigned int length)
{
    value[0] = 0;
    LogTr069Debug("value = [%s]\n", value);
    return 0;
}

static int tr069_BrowserTagURL_Write(char* value, unsigned int length)
{
    LogTr069Debug("value = [%s]\n", value);
    return 0;
}

static int tr069_AdministratorPassword_Read(char* value, unsigned int length)
{
    value[0] = 0;
    LogTr069Debug("value = [%s]\n", value);
    return 0;
}

/*------------------------------------------------------------------------------
	业务帐号对应的密码，如果终端管理系统需要主动获取密码，则返回值默认为空
 ------------------------------------------------------------------------------*/
static int tr069_UserPassword_Read(char* value, unsigned int length)
{
    if(value) {
        IND_STRCPY(value, "");
    }
    return 0;
}

static int tr069_UserPassword_Write(char* value, unsigned int length)
{
    if(value) {
        LogUserOperDebug("Value(%s)\n", value);

        unsigned char output[33] = {'\0'};
        if(strlen(value) < 16 || strlen(value) % 4 != 0) {
            appSettingSetString("ntvAESpasswd", value);
            return -1;
        }
        unsigned char key[256] = {0};
        char temp[256] = {0};
        app_TMS_aes_keys_get(key);
        EVP_DecodeBlock((unsigned char*)temp, (unsigned char*)value, length);
        aesEcbDecrypt(temp, strlen(temp), (char*)key, (char*)output, sizeof(output));
        IND_MEMSET(value, 0, strlen(value));
        IND_STRNCPY(value, (char*)output, 33);
        value[33] = '\0';

        LogUserOperDebug("value(%s)\n", value);
        appSettingSetString("ntvAESpasswd", value);
    }
    return 0;
}

static char gUserProvince[4] = {0};
static int tr069_UserProvince_Read(char* value, unsigned int length)
{
    IND_STRCPY(value, gUserProvince);
    LogTr069Debug("value = [%s]\n", value);
    return 0;
}

static int tr069_UserProvince_Write(char* value, unsigned int length)
{
    if(!value || !strlen(value))
       return -1;

    IND_STRCPY(gUserProvince, value);
    gUserProvince[3] = 0;
    LogTr069Debug("value = [%s]\n", value);
    return 0;
}

/*************************************************
Description: 初始化tr069V1定义的接口, <Device.X_CU_STB.STBInfo.***>
Input: 无
Return: 无
 *************************************************/
Tr069CUSTBInfo::Tr069CUSTBInfo()
	: Tr069GroupCall("STBInfo")
{
    Tr069Call* id  = new Tr069FunctionCall("STBID",                       tr069_STBID_Read,                   NULL);
    Tr069Call* memSize  = new Tr069FunctionCall("PhyMemSize",             tr069_PhyMemSize_Read,              NULL);
    Tr069Call* storeSize  = new Tr069FunctionCall("StorageSize",          tr069_StorageSize_Read,             NULL);
    Tr069Call* opInfo  = new Tr069FunctionCall("OperatorInfo",            tr069_OperatorInfo_Read,            tr069_OperatorInfo_Write);
    Tr069Call* intCheck  = new Tr069FunctionCall("IntegrityCheck",        tr069_IntegrityCheck_Read,          NULL);
    Tr069Call* upURL  = new Tr069FunctionCall("UpgradeURL",               tr069_UpgradeURL_Read,              tr069_UpgradeURL_Write);
    Tr069Call* browsURL1  = new Tr069FunctionCall("BrowserURL1",          tr069_BrowserURL1_Read,             tr069_BrowserURL1_Write);
    Tr069Call* browsURL2  = new Tr069FunctionCall("BrowserURL2",          tr069_BrowserURL2_Read,             tr069_BrowserURL2_Write);
    Tr069Call* tagURL  = new Tr069FunctionCall("BrowserTagURL",           tr069_BrowserTagURL_Read,           tr069_BrowserTagURL_Write);
    Tr069Call* adminPwd  = new Tr069FunctionCall("AdministratorPassword", tr069_AdministratorPassword_Read,   NULL);
    Tr069Call* usrPwd  = new Tr069FunctionCall("UserPassword",            tr069_UserPassword_Read,            tr069_UserPassword_Write);
    Tr069Call* usrPrvn  = new Tr069FunctionCall("UserProvince",           tr069_UserProvince_Read,            tr069_UserProvince_Write);

    regist(id->name(), id);
    regist(memSize->name(), memSize);
    regist(storeSize->name(), storeSize);
    regist(opInfo->name(), opInfo);
    regist(intCheck->name(), intCheck);
    regist(upURL->name(), upURL);
    regist(browsURL1->name(), browsURL1);
    regist(browsURL2->name(), browsURL2);
    regist(tagURL->name(), tagURL);
    regist(adminPwd->name(), adminPwd);
    regist(usrPwd->name(), usrPwd);
    regist(usrPrvn->name(), usrPrvn);

}

Tr069CUSTBInfo::~Tr069CUSTBInfo()
{
}
