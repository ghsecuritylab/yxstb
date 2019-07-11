
#if defined(INCLUDE_IMS)
#include "JseIMS.h"

#include "JseCall.h"
#include "JseFunctionCall.h"
#include "JseGroupCall.h"
#include "JseRoot.h"
#include "JseCamera.h"
#include <stdio.h>

//由于不知道下面函数中调用的接口具体包含哪些头文件，因此将可能包含的头文件全部列出
#include"IMSRegister.h"
#include"yx_ims_init.h"
#include"dev_flash.h"
#include"YX_IMS_porting.h"

static int JseModuelInitWrite(const char* param, char* value, int len)
{
    YX_IMS_module_init("teststring");

    return 0;
}

static int JseClearInfoWrite(const char* param, char* value, int len)
{
    remove(CONFIG_FILE_DIR"/YX_IMS_config.cfg");

    return 0;
}


/*************************************************
Description: 初始化并注册 hybroad 定义的Jse接口 <Camera.***> 
Input: 无
Return: 0
 *************************************************/
int JseIMSInit()
{    
    JseGroupCall* father = getNodeByName("IMS.Register"); // 使用华为接口的Camera.***已注册的任一接口找到父节点
    
    JseCall *module = new JseFunctionCall("ModuelInit", 0, JseModuelInitWrite);
    father->regist(module->name(), module);
    JseCall *clear = new JseFunctionCall("ClearInfo", 0, JseClearInfoWrite);
    father->regist(clear->name(), clear);
        
    JseCameraInit();
    return 0;
}

#endif
