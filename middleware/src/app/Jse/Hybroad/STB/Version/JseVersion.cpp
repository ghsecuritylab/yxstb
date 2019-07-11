
#include "JseVersion.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "build_info.h"

#include "ind_mem.h"
#include "customer.h"
#include "stbinfo/stbinfo.h"

#include <stdio.h>
#include <string.h>

static int JseVersionRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseCusVersionRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseBuildTimeRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseBuildAutherRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseBuildMachineRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseSourcePathRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseMarketPlaceRead(const char* param, char* value, int len)
{
    sprintf(value, "%s", g_make_customer_name);
    return 0;
}

static int JseSDKVersionRead(const char* param, char* value, int len)
{
    char *p;
    char buf[1024];
    IND_STRCPY(buf, g_make_sdk_revision);
    if (!strcmp(buf,"")) {
        IND_STRCPY(buf, "0");
        goto End;
    }
    p = strstr(buf,".tar");
    if (p) {
        *p = 0;
    }
End:
    IND_STRCPY(value, buf);
    return 0;
}

static int JseFastbootVersionRead(const char* param, char* value, int len)
{
    char *p;
    char buf[1024];
    IND_STRCPY(buf,g_make_fastboot_revision);
    if (!strcmp(buf,"")) {
        IND_STRCPY(buf,"U-Boot 2008.10");
        goto End;
    }
    p = strstr(buf,".yuf");
    if (p) {
        *p = 0;
    }
End:
    IND_STRCPY(value, buf);
    return 0;
}

static int JseSQAVersionRead(const char* param, char* value, int len)
{
    IND_STRCPY(value, (const char *)SQAVersion());
    return 0;
}

static int JseSQMVersionRead(const char* param, char* value, int len)
{
    IND_STRCPY(value, (const char *)SQMVersion());
    return 0;
}

static int JseProductClassVersionRead(const char* param, char* value, int len)
{
    IND_STRCPY(value, StbInfo::STB::UpgradeModel());
    return 0;
}

static int JseKernelVersionRead(const char* param, char* value, int len)
{
    char *p;
    char buf[1024];
    IND_STRCPY(buf,g_make_kernel_revision);
    if (!strcmp(buf,"")) {
        IND_STRCPY(buf,"");
        goto End;
    }
    p = strstr(buf,".yuf");
    if (p) {
        *p = 0;
    }
End:
    IND_STRCPY(value, buf);
    return 0;
}

JseVersion::JseVersion()
	: JseGroupCall("version")
{
    JseCall *call;

    call = new JseFunctionCall("version", JseVersionRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("customerVersion", JseCusVersionRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("buildTime", JseBuildTimeRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("buildAuther", JseBuildAutherRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("buildMachine", JseBuildMachineRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("sourcePath", JseSourcePathRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("marketplace", JseMarketPlaceRead, 0);
    regist(call->name(), call);
}

JseVersion::~JseVersion()
{
}

int JseVersionInit()
{
    JseCall* call;

    //以下全为/C10/C20 注册
    call = new JseFunctionCall("SDKVersion", JseSDKVersionRead,0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("FastbootVersion", JseFastbootVersionRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("SQAVersion", JseSQAVersionRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("SQMVersion", JseSQMVersionRead,0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("ProductClassVersion", JseProductClassVersionRead, 0);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("STBUpgradeVersion", JseProductClassVersionRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("KernelVersion", JseKernelVersionRead,0);
    JseRootRegist(call->name(), call);
    return 0;
}
