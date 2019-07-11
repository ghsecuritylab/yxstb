
#if defined(INCLUDE_IMS)
#include "JseCamera.h"

#include "JseCall.h"
#include "JseFunctionCall.h"
#include "JseGroupCall.h"
#include "JseRoot.h"

//由于不知道下面函数中调用的接口具体包含哪些头文件，因此将可能包含的头文件全部列出
#include"TMW_Camera.h"
#include"TMW_Media.h"


static int JsetestStreamPlayWrite(const char* param, char* value, int len)
{
    //TMW_Camera_StreamStart("119.1.1.139", 5567);
    //TMW_Media_PlayData(0, 5567, 0);
    TMW_Call_CameraPlay(CAMERA_TYPE_REMOTE);

    return 0;
}

/*************************************************
Description: 初始化并注册 hybroad 定义的Jse接口 <Camera.***> 
Input: 无
Return: 无
 *************************************************/
int JseCameraInit()
{    
    JseGroupCall* father = getNodeByName("Camera.Open"); // 使用华为接口的Camera.***已注册的任一接口找到父节点
    
    JseCall *test  = new JseFunctionCall("testStreamPlay", 0, JsetestStreamPlayWrite);
    father->regist(test->name(), test);
    
    return 0;
}

#endif
