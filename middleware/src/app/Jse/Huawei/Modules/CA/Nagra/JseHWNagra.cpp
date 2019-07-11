
#include "JseHWNagra.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

//TODO
static int JseWorkModeRead(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseBaseInfoRead(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseConsumptionConfirmWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JsePinStatusRead(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseCurrentPinWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseChangePinWrite(const char* param, char* value, int len)
{
    return 0;
}

//TODO
static int JseDeliveryEmmWrite(const char* param, char* value, int len)
{
    return 0;
}

/*************************************************
Description: ��JseHWNagra�ڵ㵱��Ҷ�ڵ����⴦��
Input: name:   ��������
          param:  ����ʱ���ݵĲ���
          value:    ����ֵ��
          length:   ����ֵ���ȡ�
          set:       ��д��־��
Return: �����ú����ķ���ֵ��
 *************************************************/
int
JseHWNagra::call(const char *name, const char* param, char* value, int length, int set)
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
Description: ע�ỪΪ����Ľӿ� <Nagra.***>
�ӿ����˵���� ��IPTV ����汾STB��EPG�ӿ��ĵ� ���ְ�Ȩ����DRM(WebKit) V1.1��
Input: ��
Return: ��
 *************************************************/
JseHWNagra::JseHWNagra()
    : JseGroupCall("nagra")
{
    JseCall *call;

    //C20 regist
    call  = new JseFunctionCall("delivery.emm", 0, JseDeliveryEmmWrite);
    regist(call->name(), call);
}

JseHWNagra::~JseHWNagra()
{
}

/*************************************************
Description: ��ʼ����ΪNagraģ�����ö���Ľӿڣ���JseHWCA.cpp����
Input: ��
Return: ��
 *************************************************/
int JseHWNagraInit()
{
    JseCall* call;

    //����ȫΪC20ע��
    call = new JseFunctionCall("ca_get_work_mode", JseWorkModeRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("ca_get_base_info", JseBaseInfoRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("ca_consumption_confirm", JseConsumptionConfirmWrite, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("ca_get_pin_status", JsePinStatusRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("ca_set_current_pin", 0, JseCurrentPinWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("ca_change_pin", 0, JseChangePinWrite);
    JseRootRegist(call->name(), call);

    call = new JseHWNagra();
    JseRootRegist(call->name(), call);
    return 0;
}

