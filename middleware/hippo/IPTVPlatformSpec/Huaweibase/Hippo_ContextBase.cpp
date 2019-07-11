#include <map>
#include <fstream>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Hippo_Debug.h"
#include "Hippo_ContextBase.h"
#include "Hippo_PlayerHWBase.h"

#include "app/app_heartbit.h"
#include "app/Setting/AppSetting.h"

#include "config/pathConfig.h"

using namespace std;

namespace Hippo {

HippoContextHWBase::HippoContextHWBase()
{
}

HippoContextHWBase::~HippoContextHWBase()
{
}

int
HippoContextHWBase::ioctlRead(HString &aField, HString &aValue /*out*/)
{
    HString param;
    HString newField;
    int ret = -1;
    char buf[4096] = { 0 };
    const char *start = NULL;
    const char *cmdend = NULL;
    const char *parastart = NULL;
    const char *str;
    char c;

    HIPPO_DEBUG("run here field=%s.\n", aField.c_str());
    //TODO: 准备运行参数
    str = aField.c_str();
    start = str;
    GET_CHAR(c, str);
    while(c != '\0') {
        switch(c) {
        case ',': {
            cmdend = str - 1;
            parastart = str;
            goto end;
        }
        case ':':
        case '^': {
            GET_CHAR(c, str);
            if(c == ':') {
                cmdend = str - 2;
                parastart = str;
            } else {
                cmdend = str - 2;
                parastart = str - 1;
            }
            goto end;
        }
        default: {
            break;
        }
        }
        GET_CHAR(c, str);
    }
end:
    if(cmdend != NULL && parastart != NULL) {
        char newFieldstr[1024] = {0};
        char parastr[1024] = {0};

        strncpy(newFieldstr, start, cmdend - start);
        newFieldstr[cmdend - start] = '\0';
        strncpy(parastr, parastart, 1024);
        newField = newFieldstr;
        param = parastr;
    } else {
        newField = aField;
    }
    //TODO: 准备运行参数
    do { //搜索本地列表
//        ioctlFunc pFunc = 0;
        ioctlMap::const_iterator it;

        it = m_ioctlMap.find(newField.c_str());
        if(it != m_ioctlMap.end()) {
            const ioctlMapNode<ioctlFunc>& ioctlNode = (it->second);
            if(ioctlNode.m_ioctlRead != 0) {
                ret = (this->*(ioctlNode.m_ioctlRead))(newField, param, aValue, 4096);
                return ret; //本地处理完成以后没有必要继续向下搜索.
            }
        }
    } while(0);
    aValue = buf;
    return ret;
}

int
HippoContextHWBase::ioctlWrite(HString &aField, HString &aValue /*in*/)
{
    HIPPO_DEBUG("run here field=%s,value=%s.\n", aField.c_str(), aValue.c_str());
    HString param;
    int ret = -1;

    do {
//        ioctlFunc pFunc = 0;
        ioctlMap::const_iterator it;
        it = m_ioctlMap.find(aField.c_str());

        if(it != m_ioctlMap.end()) {
            const ioctlMapNode<ioctlFunc>& ioctlNode = (it->second);
            if(ioctlNode.m_ioctlWrite != 0) {
                ret = (this->*(ioctlNode.m_ioctlWrite))(aField, param, aValue, 0);
                break;
            }
        }
    } while(0);
    return ret;
}

int
HippoContextHWBase::ioctl_printf(HString& aField, HString& aFieldParam, HString& aValue, int aResult)
{
    HIPPO_DEBUG("value=%s\n", aValue.c_str());
    //处理完毕,返回0, 中断处理.
    return 0;
}

}

