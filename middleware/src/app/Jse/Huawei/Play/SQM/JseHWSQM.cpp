
#include "JseHWSQM.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"
#include "AppSetting.h"
#include "SysSetting.h"

#include "sqm_port.h"
#include "../../Maintenance/Tr069/JseHWTr069.h"
#include <stdio.h>
#include <stdlib.h>

#if (defined( SQM_VERSION_C21) || defined( SQM_VERSION_C22 ) || defined(SQM_VERSION_C23))
static int JseMQMCUrlWrite(const char* param, char* value, int len)
{
    LogJseDebug("[C21 SQM PORT]  MQMCURL = %s\n", value);
    sqm_port_mqmip_set(value);
    sqm_port_msg_write(MSG_INIT);
    sqm_port_msg_write(MSG_START);
    return 0;
}
#endif

#if (defined(SQM_VERSION_C26) ||defined(SQM_VERSION_C28))
static int JseMQMCUrlWrite(const char* param, char* value, int len)
{
    LogJseDebug("[C26 SQM PORT]  MQMCURL = %s\n", value);
    sqm_port_mqmip_set(value);
#ifndef Sichuan
    sqm_port_sqmloader_start();
#endif
#if  defined(Huawei_v5)&&defined(INCLUDE_TR069)
    char url[512] = {0};
    snprintf(url, 512, "http://%s:37021/acs", value);
    JseManagementDomainSet(url);
#endif
    return 0;
}
#endif

#if defined(SQM_VERSION_ANDROID)
static int JseMQMCUrlWrite(const char *param, char *value, int len)
{
    LogUserOperDebug("JseMQMCUrlWrite  mqmcIp[%s]\n", value);
    sysSettingSetString("mqmcIp", value);
    settingManagerSave();
    sqm_port_mqmip_set(value);
    sqm_port_sqmloader_start();
    return 0;
}

static int JseWrite_MQMCPort(const char* param, char* value, int len)
{
    LogUserOperDebug("JseWrite_MQMCPort[%s]\n", value);
    return 0;
}

#endif

#if (defined(SQM_VERSION_C26) ||defined(SQM_VERSION_C28)||defined( SQM_VERSION_C21) || defined( SQM_VERSION_C22 ) || defined(SQM_VERSION_C23))
static int JseWrite_MQMCPort(const char* param, char* value, int len)
{
    sqm_set_server_port(atoi(value));
    return 0;
}
#endif

/*******************************************************************************
Functional description:
    Initialize huawei SQM module configuration defined interfaces, by JseHWPlays.CPP calls
Parameter:
Note:
Following specifications:
 *************************************************/
int JseHWSQMInit()
{
    JseCall* call;

#if (defined( SQM_VERSION_C21) || defined( SQM_VERSION_C22 ) || defined(SQM_VERSION_C23))
    //C10/C20 regist
    call = new JseFunctionCall("MQMCURL", 0, JseMQMCUrlWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("MQMCUrl", 0, JseMQMCUrlWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("MQMCPort", NULL, JseWrite_MQMCPort);
    JseRootRegist(call->name(), call);
#endif

#if (defined(SQM_VERSION_C26) || defined(SQM_VERSION_C28) || defined(SQM_VERSION_ANDROID) )
    //C10/C20 regist
    call = new JseFunctionCall("MQMCURL", 0, JseMQMCUrlWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("MQMCUrl", 0, JseMQMCUrlWrite);
    JseRootRegist(call->name(), call);
#endif
    return 0;
}


