#include "Tr069Root.h"

#include "Tr069Call.h"
#include "Tr069GroupCall.h"
#include "Device/Tr069Device.h"

#include "Tr069CallInit.h"
#include "TR069Assertions.h"

static Tr069GroupCall g_root("root"); /*Tr069所有接口的根节点*/

/*************************************************
Description: Tr069向根节点注册子节点的接口
Input:  无
Return: 0成功，其它失败
 *************************************************/
int Tr069RootRegist(const char *name, Tr069Call *call)
{
    return g_root.regist(name, call);
}

/*************************************************
Description: Tr069节点注册接口
Input:  无
Return: 无
 *************************************************/
extern "C" int Tr069RootInit()  // 在此添加节点初始化函数
{
    Tr069CallInit();  //tanf
   
    return 0;     
}

/*************************************************
Description: Tr069 注销接口，暂未实现
Input:  name: 要注销的接口名
Return: 0为成功调用
 *************************************************/
Tr069Call* Tr069RootUnregist(const char *name)
{
    return 0;
}

/*************************************************
Description: Tr069调用读接口有两种返回方式，
             1、返回一段buf
             2、返回数值，只用到pval，经过转换可以使用JseCall的结构
Input:  name:   调用名称
        str: 返回buffer的首地址
        pval:返回buffer的长度的指针
Return: 无
 *************************************************/
extern "C" int Tr069RootRead(const char *name, char *str, unsigned int val) 
{ 
    int ret = -1;
    if (str) { 
        str[0] = '\0'; // 先赋上默认值
        
         ret =  g_root.call(name, str, val, 0);
	  LogTr069Debug("tr069 read iptv[%s] ------[%s], ret = %d\n",name, str, ret);
	  return ret;
    }
    LogTr069Error("name ------[%s] str is null,will  ret = -1\n",name);

	return -1;
} 
  
/*************************************************
Description: Tr069调用写接口
Input:  name: 调用名称
        str:  要写入buffer的首地址
        pval: buffer的长度的指针
Return: 无
 *************************************************/
extern "C" int Tr069RootWrite(const char *name, char *str, unsigned int val) 
{ 
    char buf[10] = "0";
    int ret = -1;	
    if (str) {
        ret = g_root.call(name, str, val, 1);
	 LogTr069Debug("tr069 write iptv[%s] ------[%s], ret = %d\n",name, str, ret);
        return ret;
    }
    LogTr069Error("name ------[%s] str is null,will  ret = -1\n",name);

    return -1;
} 
