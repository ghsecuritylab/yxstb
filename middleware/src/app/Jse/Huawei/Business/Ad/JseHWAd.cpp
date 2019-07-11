
#include "JseHWAd.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "AdvertisementList.h"

#include <stdlib.h>

#ifdef HUAWEI_C10
static int JseAdPlaylistCountWrite(const char *param, char *value, int len)
{
    LogJseDebug("AdPlaylistCount = %d\n", atoi(value));
    advertisementList().init(atoi(value));
    return 0;
}

static int JseAdPlaylistWrite(const char *param, char *value, int len)
{
    LogJseDebug("download AdPlaylist,%s\n", value);
    advertisementList().add(value);
    return 0;
}

//TODO
static int JseADPlayListRequestUrlWrite(const char *param, char *value, int len)
{

    return 0;
}

//TODO
static int JseADLogServerUrlWrite(const char *param, char *value, int len)
{
	  return 0;
}
#endif

/*************************************************
Description: 初始化华为广告业务定义的接口，由JseHWBusiness.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWAdInit()
{
#ifdef HUAWEI_C10
    JseCall* call;

    call = new JseFunctionCall("AdPlaylistCount", 0, JseAdPlaylistCountWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("AdPlaylist", 0, JseAdPlaylistWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("ADPlayListRequestUrl", 0, JseADPlayListRequestUrlWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("ADLogServerUrl", 0, JseADLogServerUrlWrite);
    JseRootRegist(call->name(), call);
#endif
    return 0;
}

