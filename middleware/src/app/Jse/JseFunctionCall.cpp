
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
Description: Jse �ӿ��ڲ����ڵ�����ע��ĺ���,��ΪJseFunctionCall����ʱ���øú���
Input: name:  �ӿڵ�����,�ú�����Ϊ��
       param: ���ݵĲ���
       value: ����ʱ��buffer
       length:buffer�ĳ���
       set: ��д��־
Return: 0�ɹ�����,��������������
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
Description: ���ݽӿ����ҵ��丸�ڵ�
Input: name: Ŀ��ڵ���������������ú�����Ϊ��
       fatherNode: ���ڵ�ָ��,���ڵ�ʱΪ��
Return: Ŀ��ڵ�ָ��
 *************************************************/
JseCall*
JseFunctionCall::getNode(const char* name, JseCall* fatherNode)
{
    return fatherNode;
}


