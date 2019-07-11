
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
 * var sValue = Utility.setValueByName(‘dvb_chnl_book’,’{“chanKey”: chanKey,“programID”: “ programID”, “startTime”: “startTime “, “periodType“: periodType,“extendDescription“: “extendDescription”}’);
 *   0:预约成功
 *  -1:预约失败
 *  -2:预约已满
 *  -3:预约冲突
 *  */
static int JseChnlAddBookWrite(const char* param, char* value, int len)
{
    return 0;
}

/*
 * var sValue = Utility.setValueByName (‘dvb_chnl_del_book’, ‘{“bookIDList”:bookIDList }’);
 * 预约 ID 列表,[bookID1,bookID2,...,bookIDn],可以传空数组表示所有。
 *  */
static int JseChnlDelBookWrite(const char* param, char* value, int len)
{
    return 0;
}

/*************************************************
Description: 初始化华为播放流控中的DVBS模块中的book定义的接口，由JseHWDVB.cpp调用
Input: 无
Return: 无
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

