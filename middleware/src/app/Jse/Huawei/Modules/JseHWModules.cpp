
#include "JseHWModules.h"

#ifdef TVMS_OPEN
#include "TVMS/JseHWTVMS.h"
#endif

#ifdef OPENSSL_SHA256
#include "SHA256/JseHWSHA256.h"
#endif

#if defined(USE_DISK)
#include "HDDmangment/JseHWHDDmangment.h"
#endif

#ifdef INCLUDE_PIP
#include "PIP/JseHWPIP.h"
#endif

#ifdef XMPP
#include "Xmpp/JseHWXmpp.h"
#endif

#ifdef PAY_SHELL
#include "Xmpp/JseHWPayShell.h"
#endif

#if defined(INCLUDE_IMS)
#include "IMS/JseHWIMS.h"
#endif

#include "CA/JseHWCA.h"

/*************************************************
Description: 初始化华为模块配置定义的接口，由JseHuawei.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWModulesInit()
{
#ifdef TVMS_OPEN
    JseHWTVMSInit();
#endif

#ifdef OPENSSL_SHA256
    JseHWSHA256Init();
#endif

#if defined(USE_DISK)
    JseHWHDDmangmentInit();
#endif

#ifdef INCLUDE_PIP
    JseHWPIPInit();
#endif

#ifdef XMPP
    JseHWXmppInit();
#endif

#ifdef PAY_SHELL
    JseHWPayShellInit();
#endif

#if defined(INCLUDE_IMS)
    JseHWIMSInit();	
#endif
    
    JseHWCAInit();	
	
    return 0;
}

