
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
Description: Tr069 �ӿ��ڲ����ڵ�����ע��ĺ���,��ΪTr069FunctionCall����ʱ���øú���
Input: name:  �ӿڵ�����,�ú�����Ϊ��
       param: ���ڶ�ʱ����ֵbuffer
       val:   ����ֵʱΪ��ֵ�ķ���ֵ��дʱΪbuffer�ĳ���
       set:   ��д��־
Return: 0�ɹ�����,��������������
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
