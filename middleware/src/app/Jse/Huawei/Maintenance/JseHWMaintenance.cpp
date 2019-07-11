
#include "JseHWMaintenance.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "Log/JseHWLog.h"
#include "Upgrade/JseHWUpgrade.h"
#include "NetDiagnoseTool/JseHWNetDiagnoseTool.h"
#include "Jse/Huawei/Maintenance/Tr069/JseHWTr069.h"

#include "Tr069.h"

#include "ind_mem.h"
#include "mid_sys.h"

#include <string.h>

static int JseDebugTypeWrite(const char* param, char* value, int len)
{
    LogJseDebug("do nothing \n");
    return 0;
}

static int JseStreamParaListypeRead(const char* param, char* value, int len)
{
#ifdef ENABLE_VISUALINFO
    char temp[2048] = {0};

    if (strncmp(value, "StreamParaListMain", len))
        mid_sys_getVisualizationStreamInfo(temp, sizeof(temp), InfoTYpe_stream_information);
    else if (strncmp(value, "StreamParaListPip", len))
        mid_sys_getVisualizationStreamInfo(temp, sizeof(temp), InfoTYpe_PIP_stream_information);
    else
        LogJseDebug("do nothing \n");

    IND_STRNCPY(value, temp, len);
#endif
    return 0;
}

JseHWDebug::JseHWDebug()
	: JseGroupCall("Debug")
{
    JseCall* call;

    call = new JseFunctionCall("type", JseDebugTypeWrite, 0);
    regist(call->name(), call);
}

JseHWDebug::~JseHWDebug()
{
}

/*************************************************
Description: 初始化华为运营维护的接口，由JseHuawei.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWMaintenanceInit()
{
    JseCall* call;

    //C10/C20 regist 《HMT V100R003C20 本地运维页面基线接口.doc》
    call = new JseHWDebug();
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("StreamParaListype", JseStreamParaListypeRead, 0);
    JseRootRegist(call->name(), call);

    JseHWLogInit();
#if defined(NW_DIAGNOSE_OPEN) && !defined(ANDROID)
    JseHWNetDiagnoseToolInit();
#endif
    JseHWUpgradeInit();

#if INCLUDE_TR069
    JseHWTr069Init();
#endif
    return 0;
}

