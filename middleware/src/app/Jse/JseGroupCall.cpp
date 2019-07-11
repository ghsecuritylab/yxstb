
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
Description: 根据接口名找到其父节点
Input: name:  目标节点的完整调用名
       fatherNode: 父节点指针,跟节点时为空
Return: 目标节点指针
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
Description: Jse 接口内部用于调用已注册的函数,当为JseGroupCall对象时调用该函数
Input: name:接口调用名
       param: 传递的参数
       value: 返回时的buffer
       length:buffer的长度
       set: 读写标志
Return: 0成功调用,其它非正常调用
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
Description: Jse 接口内部用于注册单个节点的函数
Input: name:接口调用名
       call:该节点的指针
Return: 0
 *************************************************/
int
JseGroupCall::regist(const char *name, JseCall *call)
{
    m_callMap[name] = call;
    return 0;
}

/*************************************************
Description: Jse 接口内部用于注销单个节点的函数
Input: name:接口调用名
Return: 该节点的指针
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

