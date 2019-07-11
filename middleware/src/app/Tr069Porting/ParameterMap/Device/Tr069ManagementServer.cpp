#include "Tr069ManagementServer.h"

#include "Tr069FunctionCall.h"

#include "SysSetting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*------------------------------------------------------------------------------
	指示终端管理系统是否控制机顶盒的升级。如果为真 (1)，机顶盒应只利用终端
	管理系统寻求可用的升级。如果为假 (0)，则机顶盒可以使用其他方法进行升级。
 ------------------------------------------------------------------------------*/
static int getUpgradesManaged(char* value, unsigned int size)
{
	int tr069Upgrade = 0;
    sysSettingGetInt((char*)"tr069_upgrades", &tr069Upgrade, 0);
    snprintf(value, size, "%d", tr069Upgrade);

    return 0;
}

/*------------------------------------------------------------------------------
 UpgradesManaged是是否由TR069接管升级的标志，UpgradesManaged 为 1 只能通过tr069方式升级
 否则不能用 tr069方式生级。
 ------------------------------------------------------------------------------*/
static int setUpgradesManaged(char* value, unsigned int size)
{
    int tr069Upgrade, upgradesManaged;

    upgradesManaged = atoi(value);
    sysSettingGetInt((char*)"tr069_upgrades", &tr069Upgrade, 0);
    sysSettingSetInt((char*)"tr069_upgrades", upgradesManaged);

#if defined(HUAWEI_C20)
    if (upgradesManaged != tr069Upgrade) {
        if (value) {
           upgradeManager().stopCheckTimer();
        } else {
            upgradeManager().startCheckTimer();
        }
    }
#endif

    return 0;
}

/*************************************************
Description: 初始化tr069V1定义的接口,这里是root.Device.STBService表
Input: 无
Return: 无
 *************************************************/

Tr069ManagementServer::Tr069ManagementServer()
	: Tr069GroupCall("ManagementServer")
{
	/* 以下对象的注册到表root.Device.STBService  */
    Tr069Call* fun  = new Tr069FunctionCall("UpgradesManaged", getUpgradesManaged, setUpgradesManaged);
    regist(fun->name(), fun);
}

Tr069ManagementServer::~Tr069ManagementServer()
{
}
