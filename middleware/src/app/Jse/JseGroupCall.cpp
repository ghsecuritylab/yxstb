
#include "JseGroupCall.h"

#include "JseAssertions.h"

JseGroupCall::JseGroupCall(const char* name)
	: JseCall(name)
{
}

JseGroupCall::~JseGroupCall()
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
Description: ���ݽӿ����ҵ��丸�ڵ�
Input: name:  Ŀ��ڵ������������
       fatherNode: ���ڵ�ָ��,���ڵ�ʱΪ��
Return: Ŀ��ڵ�ָ��
 *************************************************/
JseCall*
JseGroupCall::getNode(const char* name, JseCall* fatherNode)
{
    char section[64] = {0};
    const char* nextSection = firstSection(name, section, sizeof(section));
    std::map<std::string, JseCall*>::iterator it = m_callMap.find(section);

    if (it != m_callMap.end())
        return (it->second)->getNode(nextSection, this);
    else
        return NULL;

}

/*************************************************
Description: Jse �ӿ��ڲ����ڵ�����ע��ĺ���,��ΪJseGroupCall����ʱ���øú���
Input: name:�ӿڵ�����
       param: ���ݵĲ���
       value: ����ʱ��buffer
       length:buffer�ĳ���
       set: ��д��־
Return: 0�ɹ�����,��������������
 *************************************************/
int
JseGroupCall::call(const char *name, const char *param, char *value, int length, int set)
{
    if (!name)
        return -1;

    char section[64];
    const char* nextSection = firstSection(name, section, sizeof(section));
    std::map<std::string, JseCall*>::iterator it = m_callMap.find(section);
    if (it != m_callMap.end())
        return (it->second)->call(nextSection, param, value, length, set);
    else
        return -1;
}

/*************************************************
Description: Jse �ӿ��ڲ�����ע�ᵥ���ڵ�ĺ���
Input: name:�ӿڵ�����
       call:�ýڵ��ָ��
Return: 0
 *************************************************/
int
JseGroupCall::regist(const char *name, JseCall *call)
{
    m_callMap[name] = call;
    return 0;
}

/*************************************************
Description: Jse �ӿ��ڲ�����ע�������ڵ�ĺ���
Input: name:�ӿڵ�����
Return: �ýڵ��ָ��
 *************************************************/
JseCall*
JseGroupCall::unregist(const char *name)
{
    if(!name)
        return NULL;

    char section[64];
    JseCall *call = NULL;

    const char* nextSection = firstSection(name, section, sizeof(section));
    std::map<std::string, JseCall*>::iterator it = m_callMap.find(section);
    if (nextSection) {
        if (it != m_callMap.end()){
            return ((JseGroupCall *)it->second)->unregist(nextSection);
        }else
            return NULL;
    } else {
        call = it->second;
        m_callMap.erase(it);
    }

    return call;
}

