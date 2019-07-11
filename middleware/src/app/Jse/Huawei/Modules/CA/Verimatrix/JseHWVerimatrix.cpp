
#include "JseHWVerimatrix.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include <string.h>

#include "Verimatrix.h"

static int JseCompanyRead(const char* param, char* value, int len)
{
    LogJseDebug("### JseCompanyRead : value:", value);

    ca_vm_get_configfile(CONFIG_NAME, value);

    return 0;
}

static int JseCompanyWrite(const char* param, char* value, int len)
{
    LogJseDebug("### JseCompanyWrite : value:", value);
    char vm_company[32] = {0};

    ca_vm_get_configfile(CONFIG_NAME, vm_company);
    if (strcmp(vm_company, value))
        ca_vm_set_configfile(CONFIG_NAME, value);

    return 0;
}

static int JseServerAddressRead(const char* param, char* value, int len)
{
    LogJseDebug("### JseServerAddressRead : value:", value);

    ca_vm_get_configfile(CONFIG_SERV_IP, value);

    return 0;
}

static int JseServerAddressWrite(const char* param, char* value, int len)
{
    LogJseDebug("### JseServerAddressWrite : value:", value);
    char vm_serverip[32] = {0};

    ca_vm_get_configfile(CONFIG_SERV_IP, vm_serverip);
    if(strcmp(vm_serverip, value))
        ca_vm_set_configfile(CONFIG_SERV_IP, value);

    return 0;
}

static int JseServerPortRead(const char* param, char* value, int len)
{
    LogJseDebug("### JseServerPortRead : value:", value);

    ca_vm_get_configfile(CONFIG_SERV_PORT, value);

    return 0;
}

static int JseServerPortWrite(const char* param, char* value, int len)
{
    LogJseDebug("### JseServerPortWrite : value:", value);
    char vm_serverport[16] = {0};

    ca_vm_get_configfile(CONFIG_SERV_PORT, vm_serverport);
    if(strcmp(vm_serverport, value))
        ca_vm_set_configfile(CONFIG_SERV_PORT, value);

    return 0;
}

static int JsePreferredVKSRead(const char* param, char* value, int len)
{
    LogJseDebug("### JsePreferredVKSRead : value:", value);

    ca_vm_get_configfile(CONFIG_VKS, value);

    return 0;
}

static int JsePreferredVKSWrite(const char* param, char* value, int len)
{
    LogJseDebug("### JsePreferredVKSWrite : value:", value);
    char vm_prevks[32] = {0};

    ca_vm_get_configfile(CONFIG_VKS, vm_prevks);
    if(strcmp(vm_prevks, value))
        ca_vm_set_configfile(CONFIG_VKS, value);

    return 0;
}

static int JseOTTServerAddressRead(const char* param, char* value, int len)
{
    LogJseError("JseRead_Verimatrix_OTT_ServerAddress not support now\n");
    return 0;
}

static int JseOTTServerAddressWrite(const char* param, char* value, int len)
{
    LogJseError("JseWrite_Verimatrix_OTT_ServerAddress not support now\n");
    //ymm_vmdrm_getServerAddress(value);
    return 0;
}

static int JseVerimatrixErrorLevelRead(const char* param, char* value, int len)
{
    //ymm_vmdrm_getErrorLevel(value);
    return 0;
}

static int JseOTTServerPortWrite(const char* param, char* value, int len)
{
    LogJseError("JseWrite_Verimatrix_OTT_ServerPort not support now\n");
    return 0;
}

static int JseOTTServerPortRead(const char* param, char* value, int len)
{
    LogJseError("JseRead_Verimatrix_OTT_ServerPort not support now\n");
    return 0;
}


/*************************************************
Description: ��JseHWVerimatrix�ڵ㵱��Ҷ�ڵ����⴦��
Input: name:   ��������
          param:  ����ʱ���ݵĲ���
          value:    ����ֵ��
          length:   ����ֵ���ȡ�
          set:       ��д��־��
Return: �����ú����ķ���ֵ��
 *************************************************/
int
JseHWVerimatrix::call(const char *name, const char* param, char* value, int length, int set)
{
    std::map<std::string, JseCall*>::iterator it = m_callMap.find(name);
    LogJseDebug("### find call (it->second)name :%s\n", (it->second)->name());
    if ( it != m_callMap.end()) {
        return (it->second)->call(name, param, value, length, set);
    } else {
        LogJseDebug("### not find call :%s\n", name);
        return JseGroupCall::call(name, param, value, length, set);
    }
}

/*************************************************
Description: ע�ỪΪ����Ľӿ� <Verimatrix.***>
�ӿ����˵���� ��IPTV ����汾STB��EPG�ӿ��ĵ� ���ְ�Ȩ����DRM(WebKit) V1.1��
Input: ��
Return: ��
 *************************************************/
JseHWVerimatrix::JseHWVerimatrix()
    : JseGroupCall("Verimatrix")
{
    JseCall *call;

    //C20 regist
    call  = new JseFunctionCall("Company", JseCompanyRead, JseCompanyWrite);
    regist(call->name(), call);

    //C20 regist
    call  = new JseFunctionCall("ServerAddress", JseServerAddressRead, JseServerAddressWrite);
    regist(call->name(), call);

    //C20 regist
    call  = new JseFunctionCall("ServerPort", JseServerPortRead, JseServerPortWrite);
    regist(call->name(), call);

    //C20 regist
    call  = new JseFunctionCall("PreferredVKS", JsePreferredVKSRead, JsePreferredVKSWrite);
    regist(call->name(), call);

    //C20 regist
    call  = new JseFunctionCall("Verimatrix.ErrorLevel", 0, JseVerimatrixErrorLevelRead);
    regist(call->name(), call);

    //C20 regist
    call  = new JseFunctionCall("OTT.ServerPort", JseOTTServerPortRead, JseOTTServerPortWrite);
    regist(call->name(), call);

    //C20 regist
    call  = new JseFunctionCall("OTT.ServerAddress", JseOTTServerAddressRead, JseOTTServerAddressWrite);
    regist(call->name(), call);
}

JseHWVerimatrix::~JseHWVerimatrix()
{
}

