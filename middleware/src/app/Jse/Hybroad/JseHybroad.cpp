
#include "JseHybroad.h"
#include "JseRoot.h"

#include "STB/JseSTB.h"
#include "Network/JseNetwork.h"
#include "Modules/JseModules.h"
#include "IO/JseIO.h"
#include "Business/JseBusiness.h"
#include "Maintenance/JseMaintenance.h"
#include "Maintenance/Upgrade/JseUpgrade.h"
#include "Play/JsePlay.h"
#include "Tools/JseTools.h"
#ifdef NW_DIAGNOSE_OPEN
    #include "Maintenance/NetDiagnoseTool/JseNetDiagnoseTool.h"
#endif
#include "Tools/JseTools.h"

JseHybroad::JseHybroad()
	: JseGroupCall("hybroad")
{
    JseCall *call;

    call = new JseSTB();
    regist(call->name(), call);

    call = new JseNetwork();
    regist(call->name(), call);

    call = new JseUpgrade();
    regist(call->name(), call);

#ifdef NW_DIAGNOSE_OPEN
    call = new JseNetDiagnoseTool();
    regist(call->name(), call);
#endif

    call = new JseKeyBoard();
    regist(call->name(), call);
}

JseHybroad::~JseHybroad()
{
}

/*************************************************
Description: 初始化hybroad定义的接口
Input: 无
Return: 0
 *************************************************/
int JseHybroadInit()
{
    JseCall* call;

    call = new JseHybroad();
    JseRootRegist(call->name(), call);

    JseSTBInit();
    JseModulesInit();
    JseNetworkInit();
    JseIOInit();
    JseBusinessInit();
    JseMaintenanceInit();
    JsePlayInit();
    JseToolsInit();

    return 0;
}
