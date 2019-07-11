
#include "JseFunctionCall.h"

#include "JseCall.h"
#include "JseAssertions.h"

JseFunctionCall::JseFunctionCall(const char* name, JseFunction readFunc, JseFunction writeFunc)
	: JseCall(name)
	, m_readFunc(readFunc)
	, m_writeFunc(writeFunc)
{
}

JseFunctionCall::~JseFunctionCall()
{
}

/*************************************************
Description: Jse 接口内部用于调用已注册的函数,当为JseFunctionCall对象时调用该函数
Input: name:  接口调用名,该函数中为空
       param: 传递的参数
       value: 返回时的buffer
       length:buffer的长度
       set: 读写标志
Return: 0成功调用,其它非正常调用
 *************************************************/
int
JseFunctionCall::call(const char *name, const char *param, char *value, int length, int set)
{
    if (set) {
        if(m_writeFunc)
            m_writeFunc(param, value, length);
    } else {
        if(m_readFunc)
            m_readFunc(param, value, length);
    }
    return 0;
}

/*************************************************
Description: 根据接口名找到其父节点
Input: name: 目标节点的完整调用名，该函数中为空
       fatherNode: 父节点指针,根节点时为空
Return: 目标节点指针
 *************************************************/
JseCall*
JseFunctionCall::getNode(const char* name, JseCall* fatherNode)
{
    return fatherNode;
}


