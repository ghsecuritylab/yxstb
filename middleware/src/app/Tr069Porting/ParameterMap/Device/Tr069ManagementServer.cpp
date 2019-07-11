#include "Tr069ManagementServer.h"

#include "Tr069FunctionCall.h"

#include "SysSetting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*------------------------------------------------------------------------------
	ָʾ�ն˹���ϵͳ�Ƿ���ƻ����е����������Ϊ�� (1)��������Ӧֻ�����ն�
	����ϵͳѰ����õ����������Ϊ�� (0)��������п���ʹ��������������������
 ------------------------------------------------------------------------------*/
static int getUpgradesManaged(char* value, unsigned int size)
{
	int tr069Upgrade = 0;
    sysSettingGetInt((char*)"tr069_upgrades", &tr069Upgrade, 0);
    snprintf(value, size, "%d", tr069Upgrade);

    return 0;
}

/*------------------------------------------------------------------------------
 UpgradesManaged���Ƿ���TR069�ӹ������ı�־��UpgradesManaged Ϊ 1 ֻ��ͨ��tr069��ʽ����
 �������� tr069��ʽ������
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
Description: ��ʼ��tr069V1����Ľӿ�,������root.Device.STBService��
Input: ��
Return: ��
 *************************************************/

Tr069ManagementServer::Tr069ManagementServer()
	: Tr069GroupCall("ManagementServer")
{
	/* ���¶����ע�ᵽ��root.Device.STBService  */
    Tr069Call* fun  = new Tr069FunctionCall("UpgradesManaged", getUpgradesManaged, setUpgradesManaged);
    regist(fun->name(), fun);
}

Tr069ManagementServer::~Tr069ManagementServer()
{
}
