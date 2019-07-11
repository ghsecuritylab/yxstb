
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
Description: 以"."拆分调用名
Input: name:  接口调用名
       buffer: 第一个"."之后的调用名
       length: buffer长度
Return: 第一个"."之后的首字母指针
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
Description: Tr069 接口内部用于调用已注册的函数,当为Tr069GroupCall对象时调用该函数
Input: name:接口调用名
       str: 用于读时的返回值 buffer
       val: 读数值时返回值，写时buffer长度
       set: 读写标志
Return: 0成功调用,其它非正常调用
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
    } else {  // 将call中的所有注册函数添加到本节点
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

