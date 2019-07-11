
#include "JseModules.h"

#include "CA/JseCA.h"
#include "SafetyLine/JseSafetyLine.h"

#ifdef U_CONFIG
#include "UDisk/JseUDisk.h"
#endif

#if defined(INCLUDE_IMS)
#include "IMS/JseIMS.h"
#endif

/*************************************************
Description: 初始化Modules定义的接口
Input: 无
Return: 0
 *************************************************/
int JseModulesInit()
{
    JseCAInit();
    JseSafetyLineInit();
#ifdef U_CONFIG
    JseUDiskInit();
#endif

#if defined(INCLUDE_IMS)
    JseIMSInit();
#endif
    return 0;
}
