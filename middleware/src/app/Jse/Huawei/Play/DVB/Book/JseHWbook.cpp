
#include "JseHWBook.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

static int JseBookCountRead(const char* param, char* value, int len)
{
	return 0;
}

static int JseBookListRead(const char* param, char* value, int len)
{
	return 0;
}

/*
 * var sValue = Utility.setValueByName(��dvb_chnl_book��,��{��chanKey��: chanKey,��programID��: �� programID��, ��startTime��: ��startTime ��, ��periodType��: periodType,��extendDescription��: ��extendDescription��}��);
 *   0:ԤԼ�ɹ�
 *  -1:ԤԼʧ��
 *  -2:ԤԼ����
 *  -3:ԤԼ��ͻ
 *  */
static int JseChnlAddBookWrite(const char* param, char* value, int len)
{
    return 0;
}

/*
 * var sValue = Utility.setValueByName (��dvb_chnl_del_book��, ��{��bookIDList��:bookIDList }��);
 * ԤԼ ID �б�,[bookID1,bookID2,...,bookIDn],���Դ��������ʾ���С�
 *  */
static int JseChnlDelBookWrite(const char* param, char* value, int len)
{
    return 0;
}

/*************************************************
Description: ��ʼ����Ϊ���������е�DVBSģ���е�book����Ľӿڣ���JseHWDVB.cpp����
Input: ��
Return: ��
 *************************************************/
int JseHWBookInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("dvb_get_book_count", JseBookCountRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_get_book_list", JseBookListRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_chnl_book", 0, JseChnlAddBookWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_chnl_del_book", 0, JseChnlDelBookWrite);
    JseRootRegist(call->name(), call);
    return 0;
}

