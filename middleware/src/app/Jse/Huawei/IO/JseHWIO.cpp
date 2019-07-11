
#include "JseHWIO.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "Analog/JseHWAnalog.h"
#include "Digital/JseHWDigital.h"

#if defined(BLUETOOTH)
#include "Bluetooth/JseHWBluetooth.h"
#endif //BLUETOOTH

#include "AppSetting.h"
#include "mid_sys.h"

#include <stdio.h>
#include <stdlib.h>

static int JseAspectRatioRead(const char *param, char *value, int len)
{
    int aspectRatioMode = 0;
    appSettingGetInt("hd_aspect_mode", &aspectRatioMode, 0);

    if (aspectRatioMode)
        aspectRatioMode = 0; //0: FULL for JS
    else
        aspectRatioMode = 1; //1: LETTERBOX  for JS
    snprintf(value, len, "%d", aspectRatioMode);
    return 0;
}

static int JseAspectRatioWrite(const char *param, char *value, int len)
{
    int aspectRatioMode = atoi(value);
    if (aspectRatioMode == 0) //0: FULL for JS
        aspectRatioMode = 2;
	else                     //1: LETTERBOX  for JS
		aspectRatioMode = 0;
	appSettingSetInt("hd_aspect_mode", aspectRatioMode);
    return 0;
}

/*************************************************
Description: 初始化华为IO配置定义的接口，由JseHuawei.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWIOInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("VideoOutput", JseAspectRatioRead, JseAspectRatioWrite);
    JseRootRegist(call->name(), call);

    JseHWAnalogInit();
    JseHWDigitalInit();

#if defined(BLUETOOTH)
    call = new JseHWBluetooth();
    JseRootRegist(call->name(), call);
#endif //BLUETOOTH

    return 0;
}

