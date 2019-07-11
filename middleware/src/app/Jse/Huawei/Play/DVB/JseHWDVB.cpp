
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
Description: ��ʼ����Ϊ���������е�DVBSģ�鶨��Ľӿڣ���JseHWPlay.cpp����
Input: ��
Return: ��
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

    //C20 regist,Ŀǰ��JseHWPlay.cpp regist
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

