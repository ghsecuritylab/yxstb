
#include "JseHWChannel.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "BootImagesShow.h"
#include "app_sys.h"
#include "Session.h"

#include "AppSetting.h"

#include "Hippo_api.h"
#include "SystemManager.h"
#include "ProgramChannel.h"

#ifdef HUAWEI_C10
#include "ProgramChannelC10.h"
#include "ProgramParserC10.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#ifdef HUAWEI_C20
#include "PPVListInfo.h"
#include <string>
#include "app_heartbit.h"
#endif

#if defined(SQM_VERSION_C28)
#include "sqm_port.h"
#endif

using namespace Hippo;

#ifdef HUAWEI_C10

#define APP_CHANARRAY_FILE DEFAULT_TEMP_DATAPATH"/app_chanarray.cfg"

static char g_channel_array[1024] = {0};

//TODO
static int JseChannelVerRead(const char *param, char *value, int len)
{
    return 0;
}
//TODO
static int JseChannelVerWrite(const char *param, char *value, int len)
{
    return 0;
}

static int sChannelCount = 0;

static int JseChannelCountRead(const char *param, char *value, int len)
{
    sprintf(value, "%d", sChannelCount);
    return 0;
}

static int JseChannelCountWrite(const char *param, char *value, int len)
{
    sChannelCount = atoi(value);
#if defined(Jiangsu)
#else
    if((session().getPlatform() == PLATFORM_ZTE) && (sChannelCount != 0))
        BootImagesShowAuthLogo(0);
#endif
#if defined(Sichuan) && defined(SQM_VERSION_C28)
    if (atoi(value))
        sqm_port_msg_write(MSG_START);
#endif
    return 0;
}

static int JseChannelWrite(const char *param, char *value, int len)
{

    ProgramChannelC10 *pNode = (ProgramChannelC10 *)Hippo::programParser().parseSingleChannel(value);
    ProgramChannelC10 *pNode1 = NULL;

    if(!pNode){
        LogJseError("ChannelListAdd parseSingleChannel error !\n");
        return -1;
    }

    SystemManager &sysManager = systemManager();
    sysManager.channelList().removeProgramByNumberID(pNode->GetUserChanID());
    sysManager.channelList().addProgram(pNode);
    return 0;
}

static int JseChannelIndexRead(const char *param, char *value, int len)
{
    LogJseDebug("\n");
    if(value == NULL) {
        LogJseError("ERROR! the epg read the channel array ,the value is error\n");
        return -1;
    }
    if(strlen(g_channel_array) != 0) {
        strcpy(value, g_channel_array);
        return 0;
    }
    LogJseDebug("\n");
    FILE *fp = NULL;
    unsigned int num = 0;
    size_t readlen = 0;
    char tempbuff[1024 * 3] = {0};
    fp = fopen(APP_CHANARRAY_FILE, "r");
    if(fp == NULL) {
        LogJseError("ERROR! i can't open this file =%s=\n", APP_CHANARRAY_FILE);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    num = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    LogJseDebug("num =%d\n", num);
    if(num > sizeof(tempbuff))
        num = sizeof(tempbuff);
    readlen = fread(tempbuff, 1, num, fp);
    if (readlen != num) {
        fclose(fp);
        LogJseError("ERROR! I read the channel array is error\n");
        return -1;
    }
    fclose(fp);
    fp = NULL;
    strcpy(value, tempbuff);
    LogJseDebug("value =%s=\n", value);
    strcpy(g_channel_array, value);
    return 0;
}

static int JseChannelIndexWrite(const char *param, char *value, int len)
{
    if(value == NULL || strlen(value) == 0) {
        LogJseError("ERROR!The EPG write a wrong channel array num\n");
        return -1;
    }
    LogJseDebug("value =%s=\n", value);
    if(strcmp(value, g_channel_array) == 0) {
        LogJseDebug("OK,the epg write the same channle array\n");
        return 0;
    }
    FILE *fp = NULL;
    int num = 0;
    fp = fopen(APP_CHANARRAY_FILE, "w");
    if(fp == NULL) {
        LogJseError("ERROR! i can't open this file =%s=\n", APP_CHANARRAY_FILE);
        return -1;
    }
    num = fwrite(value, strlen(value), 1, fp);
    if(num != 1) {
        fclose(fp);
        LogJseError("ERROR! I write the channel array is error\n");
        return -1;
    }
    strcpy(g_channel_array, value);
    fclose(fp);
    fp = NULL;
    return 0;
}

#endif

#ifdef HUAWEI_C20
/***********************************************************************
EPG通过接口通知STB，及时向EPG发送获取频道列表请求，刷新STB中保存的频道列表数据。
Utility.setValueByName(‘hw_op_refreshchannellist?‘XXXX?）
 return 0 is successful, -1 is failed
************************************************************************/
static int JseHWOpRefreshchannellistWrite(const char *param, char *value, int len)
{
    channel_array_request(0);
    return 0;
}

static int JseHWOpRefreshchannelppvlistWrite(const char *param, char *value, int len)
{
    ppvListInfo()->refreshPPVList();
    return 0;
}

static int JseppvListVersionRead(const char *param, char *value, int len)
{
    std::string ppvVersion;
    ppvVersion = Hippo::ppvListInfo()->PPVVersion();
    if (len >= ppvVersion.length())
        strncpy(value, ppvVersion.c_str(), ppvVersion.length());

    LogJseDebug("ppv version = %s\n", value);
    return 0;
}

static int JsechannelListVersionRead(const char *param, char *value, int len)
{
    channel_array_get_version(value, len);
    LogJseDebug("channel version = %s\n", value);
    return 0;
}
#endif

/*************************************************
Description: 初始化华为业务频道配置定义的接口，由JseHWBusiness.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWChannelInit()
{
    JseCall* call;
#ifdef HUAWEI_C10
    //C10 regist
    call = new JseFunctionCall("ChannelVer", JseChannelVerRead, JseChannelVerWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("ChannelCount", JseChannelCountRead, JseChannelCountWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("Channel", 0, JseChannelWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("ChannelIndex", JseChannelIndexRead, JseChannelIndexWrite);
    JseRootRegist(call->name(), call);
#endif

#ifdef HUAWEI_C20
    //C20 regist
    call = new JseFunctionCall("hw_op_refreshchannellist", 0, JseHWOpRefreshchannellistWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("hw_op_refreshchannelppvlist", 0, JseHWOpRefreshchannelppvlistWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("ppvListVersion", JseppvListVersionRead, 0);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("channelListVersion", JsechannelListVersionRead, 0);
    JseRootRegist(call->name(), call);
#endif
    return 0;
}

