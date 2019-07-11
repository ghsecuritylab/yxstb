
#if defined(INCLUDE_IMS)
#include "JseHWCamera.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include <stdio.h>
#include <string.h>

//由于不知道下面函数中调用的接口具体包含哪些头文件，因此将可能包含的头文件全部列出
#include"FriendBasic.h"
#include"Friends.h"
#include"FriendTV.h"
#include"Group.h"
#include"GroupMember.h"
#include"IMSRegister.h"
#include"InfoPush.h"
#include"platform_types.h"
#include"yx_ims_init.h"
#include"dev_flash.h"
#include"YX_IMS_porting.h"
#include"TMW_Camera.h"
#include"TMW_Media.h"

static int JseOpenWrite(const char* param, char* value, int len)
{
    TMW_Call_CameraInit();

    return 0;
}

static int JsePlayWrite(const char* param, char* value, int len)
{
    int ret = 0;
    ret = TMW_Call_CameraPlay(CAMERA_TYPE_LOCAL);
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

static int JseStopWrite(const char* param, char* value, int len)
{
    int ret = 0;
    ret = TMW_Call_CameraStop(CAMERA_TYPE_LOCAL);
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

static int JseCloseWrite(const char* param, char* value, int len)
{
    int ret = 0;
    ret = TMW_Call_CameraClose(CAMERA_TYPE_LOCAL);
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

static int JseSetLocationWrite(const char* param, char* value, int len)
{
    YX_IMS_JSON_PARSE jsonParse;
    YX_IMS_JSON_STR* pJsonNameValue = NULL;
    int i = 0;

    int x, y, w, h;
    int ret = 0;

    ims_json_parse_string(value, &jsonParse);
    pJsonNameValue = jsonParse.jsonStr;
    for(i = 0; i < jsonParse.count; i++) {
        if(NULL == pJsonNameValue) {
            YIMS_ERR();
            ret = -1;
            break;
        }
        if(!strcmp(pJsonNameValue->name, "x")) {
            x = strtol(pJsonNameValue->value, NULL, 10);
        } else if(!strcmp(pJsonNameValue->name, "y")) {
            y = strtol(pJsonNameValue->value, NULL, 10);
        } else if(!strcmp(pJsonNameValue->name, "w")) {
            w = strtol(pJsonNameValue->value, NULL, 10);
        } else if(!strcmp(pJsonNameValue->name, "h")) {
            h = strtol(pJsonNameValue->value, NULL, 10);
        } else {
            YIMS_ERR();
        }
        pJsonNameValue = pJsonNameValue->next;
    }
    ims_json_parse_free(&jsonParse);
    ret = TMW_Call_CameraSetLocation(CAMERA_TYPE_LOCAL, x, y, w, h);
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

static int JseGetStateRead(const char* param, char* value, int len)
{
    int state = 0;
    state = YX_IMS_camera_getStatus();
    sprintf(value, "%d", state);

    return 0;
}

static int JseQualityRead(const char* param, char* value, int len)
{
    int ret = 0;
    char buf[64] = {0};
    ret = STB_config_read("userInfo", "Camera.Quality", buf, 64);
    if(0 == ret) {
        sprintf(value, "%s", buf);
    } else {
        sprintf(value, "%s", "3");
        STB_config_write("userInfo", "Camera.Quality", "3");
    }

    return 0;
}

static int JseQualityWrite(const char* param, char* value, int len)
{
    STB_config_write("userInfo", "Camera.Quality", value);

    return 0;
}

static int JseIPRead(const char* param, char* value, int len)
{
    int ret = 0;
    char buf[64] = {0};
    ret = STB_config_read("userInfo", "Camera.IP", buf, 64);
    if(0 == ret) {
        sprintf(value, "%s", buf);
    } else {
        YIMS_ERR();
    }

    return 0;
}

/*************************************************
Description: 初始化并注册华为定义的接口 <Camera.***>  该接口是IMS
             系统的子功能接口。
接口相关说明见 《IPTV STB V100R002C10&C20 IMS业务STB与EPG接口说明书》
Input: 无
Return: 无
 *************************************************/
JseHWCamera::JseHWCamera()
	: JseGroupCall("Camera")
{
    JseCall* Call;
	
    Call = new JseFunctionCall("Open", 0, JseOpenWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Play", 0, JsePlayWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Stop", 0, JseStopWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Close", 0, JseCloseWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("SetLocation", 0, JseSetLocationWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("GetState", JseGetStateRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Quality", JseQualityRead, JseQualityWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("IP", JseIPRead, 0 );
    regist(Call->name(), Call);	

}

JseHWCamera::~JseHWCamera()
{
}

#endif
