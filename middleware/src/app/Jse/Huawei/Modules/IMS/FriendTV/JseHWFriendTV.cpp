
#if defined(INCLUDE_IMS)
#include "JseHWFriendTV.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include <string.h>

//���ڲ�֪�����溯���е��õĽӿھ��������Щͷ�ļ�����˽����ܰ�����ͷ�ļ�ȫ���г�
#include"FriendTV.h"
#include"IMSRegister.h"
#include"yx_ims_init.h"
#include"YX_IMS_porting.h"
#include"TMW_Camera.h"
#include"TMW_Media.h"

static int JseInviteWrite(const char* param, char* value, int len)
{
    YX_IMS_JSON_PARSE jsonParse;
    YX_IMS_JSON_STR* pJsonNameValue = NULL;
    char url[128] = { 0 };
    int position = 0;
    int ret = 0;
    char* ptr1 = NULL;
    char* ptr2 = NULL;
    char tempBuf[256] = { 0 };

    ptr1 = strchr(value, '{');
    ptr2 = strrchr(value, '}');
    if(NULL != ptr1 && NULL != ptr2) {
        memcpy(tempBuf, ptr1, ptr2 - ptr1 + 1);

        ims_json_parse_string(tempBuf, &jsonParse);
        pJsonNameValue = jsonParse.jsonStr;
        if(NULL != pJsonNameValue) {
            strncpy(url, pJsonNameValue->value, sizeof(pJsonNameValue->value));
            pJsonNameValue = pJsonNameValue->next;
            if(NULL != pJsonNameValue) {
                position = strtol(pJsonNameValue->value, NULL, 10);
            } else {
                YIMS_ERR();
                ret = -1;
            }
        } else {
            YIMS_ERR();
            ret = -1;
        }
        ims_json_parse_free(&jsonParse);

        if(0 == ret) {
            ret = TMW_friendTV_invite(url, position);
            if(ret) {
                YIMS_ERR();
            }
        }
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseSetInfoWrite(const char* param, char* value, int len)
{
    int ret = 0;
    ret = TMW_friendTV_accept(value);
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

/*************************************************
Description: ��ʼ����ע�ỪΪ����Ľӿ� <FriendTV.***>  �ýӿ���IMS
             ϵͳ���ӹ��ܽӿڡ�
�ӿ����˵���� ��IPTV STB V100R002C10&C20 IMSҵ��STB��EPG�ӿ�˵���顷
Input: ��
Return: ��
 *************************************************/
JseHWFriendTV::JseHWFriendTV()
	: JseGroupCall("FriendTV")
{
    JseCall* Call;
	
    Call = new JseFunctionCall("Invite", 0, JseInviteWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("SetInfo", 0, JseSetInfoWrite);
    regist(Call->name(), Call);	

}

JseHWFriendTV::~JseHWFriendTV()
{
}

#endif
