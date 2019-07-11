
#include "JseHWDVB.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "Book/JseHWBook.h"
#include "Channel/JseHWDVBChannel.h"
#include "Motor/JseHWMotor.h"
#include "Program/JseHWProgram.h"
#include "Satellite/JseHWSatellite.h"
#include "Search/JseHWSearch.h"
#include "TP/JseHWTP.h"
#include "Tuner/JseHWTuner.h"

static int JseLocalPositionRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseLocalPositionWrite( const char* param, char* value, int len)
{
    return 0;
}

static int JseHeadEndStatusCheckIntervalRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseHeadEndStatusCheckIntervalWrite(const char* param, char* value, int len)
{
    return 0;
}

static int JseBatCategoryListRead(const char* param, char* value, int len)
{
    return 0;
}

/*************************************************
Description: 初始化华为播放流控中的DVBS模块定义的接口，由JseHWPlay.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWDVBInit()
{
#ifdef HUAWEI_C20
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("dvb_get_local_position", JseLocalPositionRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_set_local_position", 0, JseLocalPositionWrite);
    JseRootRegist(call->name(), call);

    //C20 regist,目前在JseHWPlay.cpp regist
    call = new JseFunctionCall("headEndStatusCheckInterval", JseHeadEndStatusCheckIntervalRead, JseHeadEndStatusCheckIntervalWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_get_bat_category_list", JseBatCategoryListRead, 0);
    JseRootRegist(call->name(), call);

    JseHWBookInit();
    JseHWDVBChannelInit();
    JseHWMotorInit();
    JseHWProgramInit();
    JseHWSatelliteInit();
    JseHWSearchInit();
    JseHWTPInit();
    JseHWTunerInit();
#endif
    return 0;
}

