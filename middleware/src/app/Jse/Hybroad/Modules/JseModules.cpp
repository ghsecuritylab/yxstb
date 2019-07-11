
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
Description: ��ʼ��Modules����Ľӿ�
Input: ��
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
