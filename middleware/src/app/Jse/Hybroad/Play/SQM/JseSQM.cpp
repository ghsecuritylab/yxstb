
#include "JseSQM.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "sqm_port.h"

#include "sqm_types.h"

#include <stdlib.h>

#if (defined( SQM_VERSION_C21) || defined( SQM_VERSION_C22 ) || defined(SQM_VERSION_C23) || defined(SQM_VERSION_C26) || defined(SQM_VERSION_C28) || defined(SQM_VERSION_ANDROID))
static int JseSqmStandbyBootWrite(const char* param, char* value, int len)
{
    if(sqm_port_getStatus() == SQM_STOP_OK) {
        LogJseDebug("sqm standby boot!\n");
        //sqm_port_setStatus(4);
        sqm_port_msg_write(MSG_INIT);
        sqm_port_msg_write(MSG_START);
    } else {
        LogJseDebug("sqm is not at stop mode!\n");
    }
    return 0;
}

static int JseSqmServerPortWrite(const char* param, char* value, int len)
{
    sqm_set_server_port(atoi(value));
    return 0;
}

static int JseSqmServerIPWrite(const char* param, char* value, int len)
{
    LogJseDebug("C21 SQM sqm_server_ip = %s\n", value);
    sqm_port_mqmip_set(value);
    sqm_port_msg_write(MSG_INIT);
    sqm_port_msg_write(MSG_START);
    return 0;
}
#endif

/*******************************************************************************
Functional description:
    Initialize hybroad SQM module configuration defined interfaces, by JsePlays.CPP calls
Parameter:
Note:
Following specifications:
 *************************************************/
int JseSQMInit()
{
    JseCall* call;

#if (defined( SQM_VERSION_C21) || defined( SQM_VERSION_C22 ) || defined(SQM_VERSION_C23))
    //C10/C20 regist
    call = new JseFunctionCall("sqm_standby_boot", 0, JseSqmStandbyBootWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("sqm_server_port", 0, JseSqmServerPortWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("sqm_server_ip", 0, JseSqmServerIPWrite);
    JseRootRegist(call->name(), call);
#endif
    return 0;
}


