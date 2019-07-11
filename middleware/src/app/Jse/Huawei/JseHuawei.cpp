
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
Description: 初始化华为定义的接口，由JseRoot.cpp调用
Input: 无
Return: 无
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
