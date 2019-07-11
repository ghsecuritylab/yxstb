
#include "Tr069GroupCall.h"

#include "TR069Assertions.h"


Tr069GroupCall::Tr069GroupCall(const char* name)
	: Tr069Call(name)
{
}

Tr069GroupCall::~Tr069GroupCall()
{
}

/*************************************************
Description: ��"."��ֵ�����
Input: name:  �ӿڵ�����
       buffer: ��һ��"."֮��ĵ�����
       length: buffer����
Return: ��һ��"."֮�������ĸָ��
 *************************************************/
static const char* firstSection(const char* name, char* buffer, int length) // if buffer = null ?
{
    const char* result = 0;

    int i;
    for (i = 0; name[i] != '\0'; i++) {
        if (i == length)
            return result;

        if (name[i] == '.') {
            result = name + (i + 1);
            break;
        }
        else
            buffer[i] = name[i];
    }
    buffer[i] = 0;

    return result;
}

/*************************************************
Description: Tr069 �ӿ��ڲ����ڵ�����ע��ĺ���,��ΪTr069GroupCall����ʱ���øú���
Input: name:�ӿڵ�����
       str: ���ڶ�ʱ�ķ���ֵ buffer
       val: ����ֵʱ����ֵ��дʱbuffer����
       set: ��д��־
Return: 0�ɹ�����,��������������
 *************************************************/
int
Tr069GroupCall::call(const char *name, char *str, unsigned int val, int set)
{
    if (!name)
        return -1;

    char section[64];
    const char* nextSection = firstSection(name, section, sizeof(section));
    std::map<std::string, Tr069Call*>::iterator it = m_callMap.find(section);
    if (it != m_callMap.end())
        return (it->second)->call(nextSection, str, val, set);
    else
        return -1;
}

int
Tr069GroupCall::regist(const char *name, Tr069Call *call)
{    
    char section[64];
    std::map<std::string, Tr069Call*>::iterator it = m_callMap.find(name);
    std::map<std::string, Tr069Call*>::iterator it1;
    	
    if (it == m_callMap.end()) {
        m_callMap[name] = call;
    } else {  // ��call�е�����ע�ắ����ӵ����ڵ�
        for( it1 = ((Tr069GroupCall*)call)->m_callMap.begin(); it1 != ((Tr069GroupCall*)call)->m_callMap.end(); it1++) {
            ((Tr069GroupCall*)(it->second))->m_callMap.insert(std::map<std::string, Tr069Call*>::value_type (it1->first, it1->second));
        }
    }
    return 0;
}

Tr069Call*
Tr069GroupCall::unregist(const char *name)
{
    if(!name)
        return NULL;

    char section[64];
    Tr069Call *call = NULL;

    const char* nextSection = firstSection(name, section, sizeof(section));
    std::map<std::string, Tr069Call*>::iterator it = m_callMap.find(section);
    if (nextSection) {
        if (it != m_callMap.end()){
            return ((Tr069GroupCall *)it->second)->unregist(nextSection);
        }else
            return NULL;
    } else {
        call = it->second;
        m_callMap.erase(it);
    }

    return call;
}

