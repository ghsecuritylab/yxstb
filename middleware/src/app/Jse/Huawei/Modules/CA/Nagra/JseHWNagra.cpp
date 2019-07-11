
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
Description: 将JseHWNagra节点当作叶节点特殊处理。
Input: name:   调用名称
          param:  调用时传递的参数
          value:    返回值。
          length:   返回值长度。
          set:       读写标志。
Return: 被调用函数的返回值。
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
Description: 注册华为定义的接口 <Nagra.***>
接口相关说明见 《IPTV 海外版本STB与EPG接口文档 数字版权管理DRM(WebKit) V1.1》
Input: 无
Return: 无
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
Description: 初始化华为Nagra模块配置定义的接口，由JseHWCA.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWNagraInit()
{
    JseCall* call;

    //以下全为C20注册
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

