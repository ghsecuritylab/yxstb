
#if defined(INCLUDE_IMS)
#include "JseCamera.h"

#include "JseCall.h"
#include "JseFunctionCall.h"
#include "JseGroupCall.h"
#include "JseRoot.h"

//���ڲ�֪�����溯���е��õĽӿھ��������Щͷ�ļ�����˽����ܰ�����ͷ�ļ�ȫ���г�
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
Description: ��ʼ����ע�� hybroad �����Jse�ӿ� <Camera.***> 
Input: ��
Return: ��
 *************************************************/
int JseCameraInit()
{    
    JseGroupCall* father = getNodeByName("Camera.Open"); // ʹ�û�Ϊ�ӿڵ�Camera.***��ע�����һ�ӿ��ҵ����ڵ�
    
    JseCall *test  = new JseFunctionCall("testStreamPlay", 0, JsetestStreamPlayWrite);
    father->regist(test->name(), test);
    
    return 0;
}

#endif
