
#include "JseHWAppManager.h"
#include "JseFunctionCall.h"
#include "JseRoot.h"

#include "IPTVMiddleware.h"

static int JseIsAppInstalledRead(const char* param, char* value, int len)
{
    int ret = 1;//0 false, 1 true;
    sprintf(value, "%d", ret);
    return 0;
}

static int JseStartAppByNameWrite(const char* param, char* value, int len)
{
    IPTVMiddleware_PostEvent(100/*EVENT_RPC*/, 105/*RPC_START_APP*/, 0, value/*AppPackageName*/);
    return 0;
}

static int JseStartAppByIntentWrite(const char* param, char* value, int len)
{
    IPTVMiddleware_PostEvent(100/*EVENT_RPC*/, 106/*RPC_START_APP_BYINTENT*/, 0, value/*AppPackageInfo*/);
    return 0;
}


static int JseExitIptvAppWrite(const char* param, char* value, int len)
{
    IPTVMiddleware_PostEvent(100/*EVENT_RPC*/, 104/*RPC_EXIT_APP*/, 0, 0); //android
    return 0;
}

JseHWAppManager::JseHWAppManager()
	: JseGroupCall("STBAppManager")
{
    JseCall* call;

    call = new JseFunctionCall("isAppInstalled", JseIsAppInstalledRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("startAppByName", 0, JseStartAppByNameWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("startAppByIntent", 0, JseStartAppByIntentWrite);
    regist(call->name(), call);
}

JseHWAppManager::~JseHWAppManager()
{
}

//注册安卓版本的一些app管理jse接口
int JseHWAppManagerInit()
{
    JseCall* call;

    call = new JseFunctionCall("exitIptvApp", 0, JseExitIptvAppWrite);
    JseRootRegist(call->name(), call);

    call = new JseHWAppManager();
    JseRootRegist(call->name(), call);
    return 0;
}
