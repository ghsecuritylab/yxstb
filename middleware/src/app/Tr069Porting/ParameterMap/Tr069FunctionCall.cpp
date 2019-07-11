
#include "Tr069FunctionCall.h"

#include "TR069Assertions.h"

Tr069FunctionCall::Tr069FunctionCall(const char* name, Tr069Function readFunc, Tr069Function writeFunc)
	: Tr069Call(name)
	, m_readFunc(readFunc)
	, m_writeFunc(writeFunc)
{
}

Tr069FunctionCall::~Tr069FunctionCall()
{
}

/*************************************************
Description: Tr069 接口内部用于调用已注册的函数,当为Tr069FunctionCall对象时调用该函数
Input: name:  接口调用名,该函数中为空
       param: 用于读时返回值buffer
       val:   读数值时为数值的返回值，写时为buffer的长度
       set:   读写标志
Return: 0成功调用,其它非正常调用
 *************************************************/
int
Tr069FunctionCall::call(const char *name, char *str, unsigned int val, int set)
{
    if (set) {
        if(m_writeFunc)
            return m_writeFunc(str, val);
    } else {
        if(m_readFunc)
            return m_readFunc(str, val);
    }
}
