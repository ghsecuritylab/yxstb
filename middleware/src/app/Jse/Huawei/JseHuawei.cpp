
#include "JseHuawei.h"

#include "JseRoot.h"
#include "Tools/JseHWTools.h"
#include "STB/JseHWSTB.h"
#include "IO/JseHWIO.h"
#include "Network/JseHWNetwork.h"
#include "Modules/JseHWModules.h"
#include "Play/JseHWPlay.h"
#include "Maintenance/JseHWMaintenance.h"
#include "Business/JseHWBusiness.h"

/*************************************************
Description: ��ʼ����Ϊ����Ľӿڣ���JseRoot.cpp����
Input: ��
Return: ��
 *************************************************/
extern "C" int JseHuaweiInit()
{
    JseHWSTBInit();
    JseHWIOInit();
    JseHWNetworkInit();
    JseHWModulesInit();
    JseHWBusinessInit();
    JseHWPlayInit();
    JseHWMaintenanceInit();
    JseHWToolsInit();
	
    return 0;
}
