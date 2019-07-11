
#include "Tr069CallInit.h"

#include "Tr069Call.h" 
#include "Tr069Root.h"

#include "Device/Tr069Device.h"


/*************************************************
Description: 初始化Tr069定义的接口,这里是root表
Input: 无
Return: 无
 *************************************************/
extern "C" int Tr069CallInit()
{

    /* 注册root.Device 及其下表 */
    Tr069Call* dev = new Tr069Device();
    Tr069RootRegist(dev->name(), dev);
	
	
    return 0;
}
