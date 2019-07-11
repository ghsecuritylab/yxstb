
#if defined(INCLUDE_IMS)
#include "JseHWVoice.h"

#include "JseFunctionCall.h"
#include "JseAssertions.h"

//���ڲ�֪�����溯���е��õĽӿھ��������Щͷ�ļ�����˽����ܰ�����ͷ�ļ�ȫ���г�
#include"IMSRegister.h"
#include"yx_ims_init.h"
#include"YX_IMS_porting.h"
#include"TMW_Camera.h"
#include"TMW_Media.h"

static int JseTransferWrite(const char* param, char* value, int len)
{
    TMW_Call_Transfer(value);

    return 0;
}

static int JseRejectWrite(const char* param, char* value, int len)
{
    int ret = 0;
    ret = TMW_Call_Reject();
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

JseHWVoice::JseHWVoice()
	: JseGroupCall("Voice")
{
    JseCall* Call;
	
    Call  = new JseFunctionCall("Transfer", 0, JseTransferWrite);
    regist(Call->name(), Call);	
    Call  = new JseFunctionCall("Reject", 0, JseRejectWrite);
    regist(Call->name(), Call);	

}

JseHWVoice::~JseHWVoice()
{
}

#endif

