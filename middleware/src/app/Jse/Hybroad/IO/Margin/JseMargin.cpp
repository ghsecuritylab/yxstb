
#include "JseMargin.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "SysSetting.h"
#include "AppSetting.h"

#include "ind_mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" void LayerMixerDeviceSetLeftVertices(int leftmagin, int topmagin);

static int JseTopMarginRead(const char* param, char* value, int len)
{
    int topmagin = 0;
    appSettingGetInt("topmagin", &topmagin, 0);
    sprintf(value, "%d", topmagin);
    return 0;
}

static int JseTopMarginWrite(const char* param, char* value, int len)
{
    int topmagin = 0;
    appSettingGetInt("topmagin", &topmagin, 0);
    appSettingSetInt("topmagin", atoi(value));
    if(topmagin != atoi(value))
        LayerMixerDeviceSetLeftVertices(-1, atoi(value));
    return 0;
}

static int JseBottomMarginRead(const char* param, char* value, int len)
{
    int bottomMagin = 0;

    appSettingGetInt("bottommagin", &bottomMagin, 0);
    sprintf(value, "%d", bottomMagin);
    return 0;
}

static int JseBottomMarginWrite(const char* param, char* value, int len)
{
    appSettingSetInt("bottommagin", atoi(value));
    return 0;
}

static int JseLeftMarginRead(const char* param, char* value, int len)
{
    int leftMagin = 0;

    appSettingGetInt("leftmagin", &leftMagin, 0);
    sprintf(value, "%d", leftMagin);
    return 0;
}

static int JseLeftMarginWrite(const char* param, char* value, int len)
{
    int leftMagin = 0;

    appSettingGetInt("leftmagin", &leftMagin, 0);
    appSettingSetInt("leftmagin", atoi(value));
    if(leftMagin != atoi(value))
        LayerMixerDeviceSetLeftVertices(atoi(value), -1);
    return 0;
}

static int JseRightMarginRead(const char* param, char* value, int len)
{
    int rightMagin = 0;
    appSettingGetInt("rightmagin", &rightMagin, 0);
    sprintf(value, "%d", rightMagin);
    return 0;
}

static int JseRightMarginWrite(const char* param, char* value, int len)
{
    appSettingSetInt("rightmagin", atoi(value));
    return 0;
}

#ifdef HUAWEI_C10
static int JseEpgMatRead(const char* param, char* value, int len)
{
    int browserHeight = 0;
    LogJseDebug("Set page size as %s\n", value);
    // NTSC??0, PAL??1, 720P??5?? 1080I??7
    // ??tvmat???
    sysSettingGetInt("pageheight", &browserHeight, 0);
    switch(browserHeight) {
    case 480:
        IND_STRCPY(value, "0");
        break;
    case 576:
        IND_STRCPY(value, "1");
        break;
    case 720:
        IND_STRCPY(value, "5");
        break;
    case 1080:
        IND_STRCPY(value, "7");
        break;
    default:
        LogJseDebug("YOU PAST A WRONG PARAMETER.\n");
        IND_STRCPY(value, "-1");
        break;
    }
    return 0;
}
static int JseEpgMatWrite(const char* param, char* value, int len)
{
    LogJseDebug("Set page size as %s\n", value);
    // NTSC??0, PAL??1, 720P??5?? 1080I??7
    // ??tvmat???
    switch(atoi(value)) {
    case 0:
        sysSettingSetInt("pagewidth", 760);
        sysSettingSetInt("pageheight", 480);
        break;
    case 1:
        sysSettingSetInt("pagewidth", 720);
        sysSettingSetInt("pageheight", 576);
        break;
    case 5:
        sysSettingSetInt("pagewidth", 1280);
        sysSettingSetInt("pageheight", 820);
        break;
    case 7:
        sysSettingSetInt("pagewidth", 1920);
        sysSettingSetInt("pageheight", 1080);
        break;
    default:
        LogJseDebug("YOU PAST A WRONG PARAMETER.\n");
        break;
    }
    return 0;
}
static int JsePageWidthRead(const char* param, char* value, int len)
{
    int width = 0;
    sysSettingGetInt("pagewidth", &width, 0);
    sprintf(value, "%d", width);
    return 0;
}
static int JsePageWidthWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("pagewidth", atoi(value));
    return 0;
}
static int JsePageHightRead(const char* param, char* value, int len)
{
    int height = 0;
    sysSettingGetInt("pageheight", &height, 0);
    sprintf(value, "%d", height);
    return 0;
}
static int JsePageHightWrite(const char* param, char* value, int len)
{
    sysSettingSetInt("pageheight", atoi(value));
    return 0;
}
#endif

/*************************************************
Description: 初始化海博图形层边距配置定义的接口，由JseIO.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseMarginInit()
{
    JseCall* call;

    //C10/C20 regist
    call = new JseFunctionCall("yx_para_topmagin", JseTopMarginRead, JseTopMarginWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("yx_para_bottommagin", JseBottomMarginRead, JseBottomMarginWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("yx_para_leftmagin", JseLeftMarginRead, JseLeftMarginWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("yx_para_rightmagin", JseRightMarginRead, JseRightMarginWrite);
    JseRootRegist(call->name(), call);

#ifdef HUAWEI_C10
    call = new JseFunctionCall("yx_para_epgmat", JseEpgMatRead, JseEpgMatWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_page_width", JsePageWidthRead, JsePageWidthWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("yx_para_page_hight", JsePageHightRead, JsePageHightWrite);
    JseRootRegist(call->name(), call);
#endif
    return 0;
}

