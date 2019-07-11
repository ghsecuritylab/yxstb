
#include "JseRoot.h"

#include "JseGroupCall.h"
#include "Huawei/JseHuawei.h"
#include "Hybroad/JseHybroad.h"

static JseGroupCall g_root("root");

/*************************************************
Description: 向跟节点里添加一个子节点
Input:  name: 子节点名
        call: 子节点的结构指针
Return: 0 为成功注册
 *************************************************/
int JseRootRegist(const char *name, JseCall *call)
{
    return g_root.regist(name, call);
}

/*************************************************
Description: 按接口名获取其所在的父节点指针
Input:  name: 接口名
Return: 0为成功调用
 *************************************************/
JseGroupCall* getNodeByName(const char* name)
{
    return (JseGroupCall*)g_root.getNode(name);
}

/*************************************************
Description: Jse 注销接口，暂未实现
Input:  name: 要注销的接口名
Return: 0为成功调用
 *************************************************/
JseCall *JseRootUnregist(const char *name)
{
    return 0;
}

/*************************************************
Description: Jse 调用读接口的入口函数
Input:  name:   调用名称
        param:  传入的参数
        value:  返回的buffer
        length: buffer长度
Return: 0为成功调用
        其它值出错
 *************************************************/
extern "C" int JseRootRead(const char *name, const char *param, char *value, int length)
{
    return g_root.call(name, param, value, length, 0);
}

/*************************************************
Description: Jse 调用读接口的写口函数
Input:  name:   调用名称
        param:  传入的参数
        value:  要写入的buffer
        length: 无用，写0即可
Return: 0为成功调用
        其它值出错
 *************************************************/
extern "C" int JseRootWrite(const char *name, const char *param, char *value, int length)
{
    return g_root.call(name, param, value, length, 1);
}

/*************************************************
Description: Jse 初始化接口
Input:  无
Return: 0为成功调用
 *************************************************/
extern "C" int JseRootInit()
{
    JseHuaweiInit();
    JseHybroadInit();
    return 0;
}
