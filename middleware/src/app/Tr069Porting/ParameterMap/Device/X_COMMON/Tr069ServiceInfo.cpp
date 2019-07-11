#include "Tr069ServiceInfo.h"

#include "Tr069FunctionCall.h"

#include "SysSetting.h"
#include "AppSetting.h"
#include "SettingModuleNetwork.h"

#include "tr069_port.h"
#include "tr069_port1.h"
#include "cryptoFunc.h"
#include "openssl/evp.h"
#include "Tr069.h"
#include "telecom_config.h"
#include "ind_mem.h"
#include "sys_basic_macro.h"
#include "TR069Assertions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Tr069Call* g_tr69ServiceInfo = new Tr069ServiceInfo();


/*------------------------------------------------------------------------------
	给网络接口分配地址的方法，枚举类型： “DHCP” “Static”
	当发生改变时，需立即上报终端管理系统。
 ------------------------------------------------------------------------------*/
static void getAddressingType(char *value, int size) //tr069_port_get_AddressingType
{
    static int type = -1;

    if(-1 == type) {
        sysSettingGetInt("connecttype", &type, 0);
        LogTr069Debug("AddressingType [%d].   {%d:NETTYPE_PPPOE, %d:NETTYPE_DHCP, %d:NETTYPE_STATIC}\n",
            type, NETTYPE_PPPOE, NETTYPE_DHCP,NETTYPE_STATIC);
    }
    switch(type) {
    case NETTYPE_PPPOE:
        IND_STRCPY(value, "PPPoE");
        break;
    case NETTYPE_DHCP:
        IND_STRCPY(value, "DHCP");
        break;
    case NETTYPE_STATIC:
		IND_STRCPY(value, "Static");
		break;
    case NETTYPE_DHCP_ENCRYPT:
		IND_STRCPY(value, "IPoE");
		break;
    default:
        IND_STRCPY(value, "Static");
        break;
    }
}



/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
static void setAddressingType(char *value) // tr069_port_set_AddressingType
{
    if (!value || !strlen(value))
        return;
    int connectType = NETTYPE_STATIC;
    if(0 == strcasecmp(value, "PPPoE"))
        connectType = NETTYPE_PPPOE;
    else if(0 == strcasecmp(value, "DHCP"))
        connectType = NETTYPE_DHCP;
    else
        connectType = NETTYPE_STATIC;

	sysSettingSetInt("connecttype", connectType);
}

/////////////////////////////////
/*------------------------------------------------------------------------------
 用户PPPoE拨号用户名
 ------------------------------------------------------------------------------*/
static int getTr069PortPPPoEID(char* value, unsigned int size)
{
    sysSettingGetString("netuser", value, size, 0);

    return 0;
}

static int setTr069PortPPPoEID(char* value, unsigned int size)
{
    if(value)
        sysSettingSetString("netuser", value);

    return 0;
}

static int getTr069PortPPPoEID2(char* value, unsigned int size)
{
    sysSettingGetString("netuser2", value, size, 0);

    return 0;
}

static int setTr069PortPPPoEID2(char* value, unsigned int size)
{
    if(value)
        sysSettingSetString("netuser2", value);

    return 0;
}

/*------------------------------------------------------------------------------
 用户PPPoE拨号密码，如果终端管理系统需要主动获取密码，则返回值默认为空
 ------------------------------------------------------------------------------*/
static int getTr069PortPPPoEPassword(char* value, unsigned int size)
{
    if(value) {
        IND_STRCPY(value, "");
    }

    return 0;
}

static int setTr069PortPPPoEPassword(char* value, unsigned int size)
{
    if(value) {
        LogUserOperDebug("app_tr069_port_set_PPPoEPassword value=%s\n", value);

        if(strlen(value) < 16 || strlen(value) % 4 != 0) {
            sysSettingSetString("netAESpasswd", value);
            return 0;
        }

        char output[33] = {'\0'};
        unsigned char key[256] = {0};
        char temp[256] = {0};
        app_TMS_aes_keys_get(key);
        EVP_DecodeBlock((unsigned char*)temp, (unsigned char*)value, strlen(value));
        aesEcbDecrypt(temp, strlen(temp), (char*)key, output, sizeof(output));
        IND_MEMSET(value, 0, strlen(value));
        IND_STRNCPY(value, output, 33);
        value[33] = '\0';
        sysSettingSetString("netAESpasswd", value);
    }

    return 0;
}

static int getTr069PortPPPoEPassword2(char* value, unsigned int size)
{
    if(value) {
        IND_STRCPY(value, "");
    }

    return 0;
}

static int setTr069PortPPPoEPassword2(char* value, unsigned int size)
{
    //新的设置配置文件里面没有ipoeAESpasswd2字段，放空
    return 0;
}

/*------------------------------------------------------------------------------
 除基本音视频播放能力外，终端支持的业务功能列表，
 采用半角竖线"|"进行分割。
 ------------------------------------------------------------------------------*/
static int getTr069PortServiceList(char* value, unsigned int size)
{
    if (size <= strlen("VideoComm|FEC|BurstControl|ResidentMedia|3DGame|AppStore|HomeMedia|MassStorage"))
        return -1;
    strcpy(value, "VideoComm|FEC|BurstControl|ResidentMedia|3DGame|AppStore|HomeMedia|MassStorage");

    return 0;
}

/*------------------------------------------------------------------------------
	是否启用DHCP鉴权
 ------------------------------------------------------------------------------*/
static int getTr069PortDHCPEnable(char* value, unsigned int size)
{
    if (snprintf(value,size,"1") >= (int)size) {
        printf("Error.size is too short.");
        return -1;
    }

    return 0;
}

 static int setTr069PortDHCPEnable(char* value, unsigned int size)
{
    return 0;
}


/*------------------------------------------------------------------------------
	用户DHCP接入用户名
 ------------------------------------------------------------------------------*/
static int getTr069PortDHCPID(char* value, unsigned int size)
{
    if(value)
        sysSettingGetString("dhcpuser", value, size, 0);

    return 0;
}

static int setTr069PortDHCPID(char* value, unsigned int size)
{
    if(value)
        sysSettingSetString("dhcpuser", value);

    return 0;
}

/*------------------------------------------------------------------------------
 用户DHCP接入密码，如果终端管理系统需要主动获取密码，则返回值默认为空
 ------------------------------------------------------------------------------*/
static int getTr069PortDHCPPassword(char* value, unsigned int size)
{
    if(value) {
        IND_STRCPY(value, "");
    }

    return 0;
}

static int setTr069PortDHCPPassword(char* value, unsigned int size)
{
    if(value)
        sysSettingSetString("ipoeAESpasswd", value);
    return 0;
}


/*------------------------------------------------------------------------------
 上海业务帐号对应的密码，如果终端管理系统需要主动获取密码，则返回值默认为空
 ------------------------------------------------------------------------------*/
static int getTr069PortSHUserPassword(char* value, unsigned int size)
{
    if(value) {
        IND_STRCPY(value, "");
    }

    return 0;
}

static int setTr069PortSHUserPassword(char* value, unsigned int size)
{
    if(value) {
        LogUserOperDebug("Value(%s)\n", value);

        char output[33] = {'\0'};
        if(strlen(value) < 16 || strlen(value) % 4 != 0) {
            appSettingSetString("ntvAESpasswd", value);
            return 0;
        }
        unsigned char key[256] = {0};
        char temp[256] = {0};
        app_TMS_aes_keys_get(key);
        EVP_DecodeBlock((unsigned char*)temp, (unsigned char*)value, strlen(value));
        aesEcbDecrypt(temp, strlen(temp), (char*)key, output, sizeof(output));
        IND_MEMSET(value, 0, strlen(value));
        IND_STRNCPY(value, output, 33);
        value[33] = '\0';
        LogUserOperDebug("Real value(%s)\n", value);

        appSettingSetString("ntvAESpasswd", value);
        settingManagerSave();
    }
    return 0;
}


/*------------------------------------------------------------------------------
 最后一次业务认证的服务器备份URL，形式如
 http://IptvAuthDomain/AuthenticationURLBackup。当发生改变时，需立即上报终端管理系统。
 ------------------------------------------------------------------------------*/
static int getTr069PortAuthURLBackup(char* value, unsigned int size)
{
    char buf[URL_LEN * 2 + 1] = {0};

    if(value) {
        sysSettingGetString("eds1", buf, URL_LEN, 0);
        if(strlen(buf) >= size)
            value[0] = 0;
        else
            IND_STRCPY(value, buf);
    }

    return 0;
}

static int setTr069PortAuthURLBackup(char* value, unsigned int size)
{
    if(value && (strncmp(value, "http://", 7) == 0))
         sysSettingSetString("eds1", value);

    return 0;
}



/*-----------------;-------------------------------------------------------------
	用户帐号，参考《机顶盒与IPTV业务运营平台接口技术规范V2.2》附录C的说明。
 当发生改变时，需立即上报终端管理系统。
 ------------------------------------------------------------------------------*/
static int getTr069PortUserID(char* value, unsigned int size)
{
    if(value)
        appSettingGetString("ntvuser", value, size, 0);

    return 0;
}

static int setTr069PortUserID(char* value, unsigned int size)
{
    if(value)
        appSettingSetString("ntvuser", value);

    return 0;
}

static int getTr069PortUserID2(char* value, unsigned int size)
{
    if(value)
        appSettingGetString("ntvuser2", value, size, 0);

    return 0;
}

static int setTr069PortUserID2(char* value, unsigned int size)
{
    appSettingSetString("ntvuser2", value);

    return 0;
}


/*------------------------------------------------------------------------------
 业务帐号对应的密码，如果终端管理系统需要主动获取密码，则返回值默认为空
 ------------------------------------------------------------------------------*/
static int getTr069PortUserPassword(char* value, unsigned int size)
{
    if(value) {
        IND_STRCPY(value, "");
    }

    return 0;
}

static int setTr069PortUserPassword(char* value, unsigned int size)
{
    if(value) {
        LogUserOperDebug("Value(%s)\n", value);

        char output[33] = {'\0'};
        if(strlen(value) < 16 || strlen(value) % 4 != 0) {
            appSettingSetString("ntvAESpasswd", value);
            return 0;
        }
        unsigned char key[256] = {0};
        char temp[256] = {0};
        app_TMS_aes_keys_get(key);
        EVP_DecodeBlock((unsigned char*)temp, (unsigned char*)value, strlen(value));
        aesEcbDecrypt(temp, strlen(temp), (char*)key, output, sizeof(output));
        IND_MEMSET(value, 0, strlen(value));
        IND_STRNCPY(value, output, 33);
        value[33] = '\0';

        LogUserOperDebug("value(%s)\n", value);
        appSettingSetString("ntvAESpasswd", value);
    }

    return 0;
}

static int getTr069PortUserPassword2(char* value, unsigned int size)
{
    if(value) {
        IND_STRCPY(value, "");
    }
    return 0;
}

static int setTr069PortUserPassword2(char* value, unsigned int size)
{
    //新的配置文件中没有ntvAESpasswd2，所以放空
    return 0;
}

/*------------------------------------------------------------------------------
  最后一次业务认证的服务器URL，形式如
  http://IptvAuthDomain/AuthenticationURL。当发生改变时，需立即上报终端管理系统。
 ------------------------------------------------------------------------------*/
static int getTr069PortAuthURL(char* value, unsigned int size)
{
    char buf[URL_LEN * 2 + 1] = {0};

    if(value) {
        sysSettingGetString("eds", buf, URL_LEN, 0);
        strcat(buf, ",");
        sysSettingGetString("eds1", buf + strlen(buf), sizeof(buf) - strlen(buf), 0);
        if(strlen(buf) >= size)
            value[0] = 0;
        else
            IND_STRCPY(value, buf);
    }

    return 0;
}

static int setTr069PortAuthURL(char* value, unsigned int size)
{
    char buf[URL_LEN * 2 + 1] = {0};
    char *p = NULL;

    if(value == NULL) {
        LogUserOperDebug("[DBG ITMS ERROR]\n");
        return -1;
    }

    IND_STRCPY(buf, value);

    LogUserOperDebug("Url(%s)\n", value);
    if (strncmp(buf, "http://", 7) == 0) {
        if(!(p = strstr(buf, ","))) {
            sysSettingSetString("eds", buf);
        } else {
            *p = '\0';
            sysSettingSetString("eds", buf);
            sysSettingSetString("eds1", p + 1);
        }
    }
    return 0;
}






/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
static int getTr069PortPPPOEEnable(char* value, unsigned int size)
{
    int connectType = 0;
    sysSettingGetInt("connecttype", &connectType, 0);
    if(NETTYPE_PPPOE == connectType)
        IND_STRCPY(value, "1");
    else
        IND_STRCPY(value, "0");
    LogTr069Debug("value = [%s]\n", value);

    return 0;
}

static int setTr069PortPPPOEEnable(char* value, unsigned int size)
{
    if(!value)
        return -1;

    LogTr069Debug("value = [%s]\n", value);
    if(atoi(value))
        sysSettingSetInt("connecttype", NETTYPE_PPPOE);
    else {
        getAddressingType(value,size);
        setAddressingType(value);
    }

    return 0;
}


/*------------------------------------------------------------------------------
 *  以下对象的注册到表root.Device.XXX.ServiceInfo
 * XXX可以是X_CTC_IPTV，X_00E0FC
 ------------------------------------------------------------------------------*/
Tr069ServiceInfo::Tr069ServiceInfo()
	: Tr069GroupCall("ServiceInfo")
{
    // CTC,HW,CU共有 fun1 - fun4
    Tr069Call* fun1  = new Tr069FunctionCall("PPPoEID", getTr069PortPPPoEID, setTr069PortPPPoEID);
    regist(fun1->name(), fun1);


    Tr069Call* fun2  = new Tr069FunctionCall("PPPoEPassword", getTr069PortPPPoEPassword, setTr069PortPPPoEPassword);
    regist(fun2->name(), fun2);

    Tr069Call* fun3  = new Tr069FunctionCall("UserID", getTr069PortUserID, setTr069PortUserID);
    regist(fun3->name(), fun3);

    Tr069Call* fun4  = new Tr069FunctionCall("UserIDPassword", getTr069PortUserPassword, setTr069PortUserPassword);
    regist(fun4->name(), fun4);

    // CTC,HW共有 fun5
    Tr069Call* fun5  = new Tr069FunctionCall("AuthURL", getTr069PortAuthURL, setTr069PortAuthURL);
    regist(fun5->name(), fun5);


    // CTC
#if defined(HUAWEI_C10)

    Tr069Call* fun6  = new Tr069FunctionCall("ServiceList", getTr069PortServiceList, NULL);
    regist(fun6->name(), fun6);

    Tr069Call* fun7  = new Tr069FunctionCall("IPTV_DHCP_Enable", getTr069PortDHCPEnable, setTr069PortDHCPEnable);
    regist(fun7->name(), fun7);


    Tr069Call* fun8  = new Tr069FunctionCall("DHCPID", getTr069PortDHCPID, setTr069PortDHCPID);
    regist(fun8->name(), fun8);

    Tr069Call* fun9  = new Tr069FunctionCall("DHCPPassword", getTr069PortDHCPPassword, setTr069PortDHCPPassword);
    regist(fun9->name(), fun9);

    Tr069Call* fun10  = new Tr069FunctionCall("IPTV_DHCP_Username", getTr069PortDHCPID, setTr069PortDHCPID);
    regist(fun10->name(), fun10);

    Tr069Call* fun11  = new Tr069FunctionCall("IPTV_DHCP_Password", getTr069PortDHCPPassword, setTr069PortDHCPPassword);
    regist(fun11->name(), fun11);

    Tr069Call* fun12  = new Tr069FunctionCall("Password", getTr069PortSHUserPassword, setTr069PortSHUserPassword);
    regist(fun12->name(), fun12);

    Tr069Call* fun13  = new Tr069FunctionCall("AuthURLBackup", getTr069PortAuthURLBackup, setTr069PortAuthURLBackup);
    regist(fun13->name(), fun13);

#endif



    /* CU 独有 fun14 - fun20
       CU的查找字符串命名跟CTC/HW有差异，如AuthServiceInfo，在CTC,HW都是ServiceInfo，因此在Tr069X_CU_STB.cpp注册的时候只能直接填"AuthServiceInfo".
    */
    Tr069Call* fun14  = new Tr069FunctionCall("PPPOEEnable", getTr069PortPPPOEEnable, setTr069PortPPPOEEnable);
    regist(fun14->name(), fun14);

    Tr069Call* fun17  = new Tr069FunctionCall("PPPOEID2", getTr069PortPPPoEID2, setTr069PortPPPoEID2);
    regist(fun17->name(), fun17);

    Tr069Call* fun18  = new Tr069FunctionCall("PPPOEPassword2", getTr069PortPPPoEPassword2, setTr069PortPPPoEPassword2);
    regist(fun18->name(), fun18);

    Tr069Call* fun19  = new Tr069FunctionCall("UserID2", getTr069PortUserID2, setTr069PortUserID2);
    regist(fun19->name(), fun19);

    Tr069Call* fun20  = new Tr069FunctionCall("UserIDPassword2", getTr069PortUserPassword2, setTr069PortUserPassword2);
    regist(fun20->name(), fun20);

}

Tr069ServiceInfo::~Tr069ServiceInfo()
{
}



