
#if defined(INCLUDE_IMS)
#include "JseHWVideo.h"

#include "JseFunctionCall.h"
#include "JseAssertions.h"

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

static int JseInviteWrite(const char* param, char* value, int len)
{
    int ret = 0;
    ret = TMW_Call_AddSession(CALL_TYPE_VIDEO);
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

static int JseCancleWrite(const char* param, char* value, int len)
{
    int ret = 0;
    ret = TMW_Call_Cancel();
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

static int JseAcceptWrite(const char* param, char* value, int len)
{
    int ret = 0;
    ret = TMW_Call_Accept();
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

static int JsebyeWrite(const char* param, char* value, int len)
{
    int ret = 0;
    ret = TMW_Call_RemoveSession(CALL_TYPE_VIDEO);
    if(ret) {
        YIMS_ERR();
    }
    TMW_Call_CameraStop(CAMERA_TYPE_REMOTE);
    TMW_Call_CameraStop(CAMERA_TYPE_LOCAL);
    app_player_pip_stop();
    mid_stream_close(0, 1);
    TMW_Call_End();

    return 0;
}

static int JsePlayWrite(const char* param, char* value, int len)
{
    int playMode = 0;
    int ret = 0;

    playMode = strtol(value, NULL, 10);
    switch(playMode) {
    case 0:/*main window , local video*/
        TMW_Call_CameraStop(CAMERA_TYPE_LOCAL);
        YX_IMS_set_media_id(0);
        ret = TMW_Call_CameraPlay(CAMERA_TYPE_LOCAL);
        break;
    case 1:/*main window , remote video*/
        TMW_Call_CameraStop(CAMERA_TYPE_REMOTE);
        YX_IMS_set_media_id(0);
        ret = TMW_Call_CameraPlay(CAMERA_TYPE_REMOTE);
        break;
    case 2:/*sub window, local video*/
        TMW_Call_CameraStop(CAMERA_TYPE_LOCAL);
        YX_IMS_set_media_id(1);
        ret = TMW_Call_CameraPlay(CAMERA_TYPE_LOCAL);
        break;
    case 3:/*sub window . remote video*/
        TMW_Call_CameraStop(CAMERA_TYPE_REMOTE);
        YX_IMS_set_media_id(1);
        ret = TMW_Call_CameraPlay(CAMERA_TYPE_REMOTE);
        break;
    default:
        YIMS_ERR();
        break;
    }
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

static int JseStopWrite(const char* param, char* value, int len)
{
    int ret = 0;
    ret = TMW_Call_CameraStop(CAMERA_TYPE_LOCAL);
    mid_stream_close(1, 1);
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
    int x, y, w, h, midiaId;
    int ret = 0;
    ims_json_parse_string(value, &jsonParse);
    pJsonNameValue = jsonParse.jsonStr;
    for(i = 0; i < jsonParse.count; i++) {
        if(NULL == pJsonNameValue) {
            YIMS_ERR();
            ret = -1;
            break;
        }
        if(!strcmp(pJsonNameValue->name, "plane")) {
            midiaId = strtol(pJsonNameValue->value, NULL, 10);
        } else if(!strcmp(pJsonNameValue->name, "x")) {
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

    ret = TMW_Media_SetLocation(midiaId, x, y, w, h);
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

JseHWVideo::JseHWVideo()
	: JseGroupCall("Video")
{
    JseCall* Call;
	
    Call = new JseFunctionCall("Invite", 0, JseInviteWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Cancle", 0, JseCancleWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Accept", 0, JseAcceptWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("bye", 0, JsebyeWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Play", 0, JsePlayWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Stop", 0, JseStopWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("SetLocation", 0, JseSetLocationWrite);
    regist(Call->name(), Call);	
        
}

JseHWVideo::~JseHWVideo()
{
}

#endif
