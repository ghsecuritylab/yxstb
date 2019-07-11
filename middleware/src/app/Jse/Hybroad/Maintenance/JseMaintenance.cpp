
#include "JseMaintenance.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "Log/JseLog.h"
#include "Upgrade/JseUpgrade.h"
#if defined(INCLUDE_TR069) || defined(INCLUDE_HMWMGMT)
#include "Tr069/JseTr069.h"
#endif

#ifdef NW_DIAGNOSE_OPEN
    #include "NetDiagnoseTool/JseNetDiagnoseTool.h"
#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int TrriggerConnect = 0;

static int JseMonitorTriggerWrite(const char* param, char* value, int len)
{
    pthread_t Monitor_Trriggerpthread = 0;
    pthread_attr_t Monitor_Trriggertattr;
    pthread_attr_init(&Monitor_Trriggertattr);
    int ret = 0;
    char *ip = (char*)malloc(15);
    memset(ip, 0, 15);

    if(inet_addr(value) == INADDR_NONE){
        LogJseError("Input error!\n");
        return -1;
    }

    strcpy(ip, value);

    // ret = Monitor_STB_TrriggerConnect(ip);
    if(-1 == ret){
        TrriggerConnect = 0;
    }else{
        TrriggerConnect = 1;
    }
    return 0;
}

static int JseMonitorTriggerRead(const char* param, char* value, int len)
{
    sprintf(value, "%d", TrriggerConnect);
}

/*************************************************
Description: 初始化海博运营维护的接口，由JseHybroad.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseMaintenanceInit()
{
    JseCall* call;

    //C10/C20 regist
    call = new JseFunctionCall("MonitorTrigger", JseMonitorTriggerRead, JseMonitorTriggerWrite);
    JseRootRegist(call->name(), call);

    JseUpgradeInit();
    JseLogInit();

#if defined(INCLUDE_TR069) || defined(INCLUDE_HMWMGMT)
    JseTr069Init();
#endif

    return 0;
}

