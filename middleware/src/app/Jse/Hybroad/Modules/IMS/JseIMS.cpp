
#if defined(INCLUDE_IMS)
#include "JseIMS.h"

#include "JseCall.h"
#include "JseFunctionCall.h"
#include "JseGroupCall.h"
#include "JseRoot.h"
#include "JseCamera.h"
#include <stdio.h>

//���ڲ�֪�����溯���е��õĽӿھ��������Щͷ�ļ�����˽����ܰ�����ͷ�ļ�ȫ���г�
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
Description: ��ʼ����ע�� hybroad �����Jse�ӿ� <Camera.***> 
Input: ��
Return: 0
 *************************************************/
int JseIMSInit()
{    
    JseGroupCall* father = getNodeByName("IMS.Register"); // ʹ�û�Ϊ�ӿڵ�Camera.***��ע�����һ�ӿ��ҵ����ڵ�
    
    JseCall *module = new JseFunctionCall("ModuelInit", 0, JseModuelInitWrite);
    father->regist(module->name(), module);
    JseCall *clear = new JseFunctionCall("ClearInfo", 0, JseClearInfoWrite);
    father->regist(clear->name(), clear);
        
    JseCameraInit();
    return 0;
}

#endif
