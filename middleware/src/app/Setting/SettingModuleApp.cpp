
#include "Assertions.h"
#include "SettingModuleApp.h"

#include <stdio.h>

#include "Setting.h"
#include "AppSetting.h"
#include "SysSetting.h"
#if defined(CACHE_CONFIG)
#include "CacheSetting.h"
#endif
#if defined(ENABLE_BAK_SETTING)
#include "BakSetting.h"
#endif
#include "configCustomer.h"

#include "build_info.h"

namespace Hippo {

static SettingModuleApp g_SettingModuleApp;

SettingModuleApp::SettingModuleApp()
    : SettingModule()
{
}

SettingModuleApp::~SettingModuleApp()
{
}

int
SettingModuleApp::settingModuleRegister()
{
    sysSetting().add("CFversion", g_make_build_CFversion);
#if defined(CACHE_CONFIG)
    cacheSetting().add("CacheSetting_CFversion", g_make_build_CFversion);
#endif
    sysSetting().add("[ APP ]", "");
    LogSafeOperDebug("SettingModuleApp::settingModuleRegister [%d]\n", __LINE__);

#if defined(ENABLE_BAK_SETTING) && defined(Sichuan) && defined(brcm7405)
    bakSetting().add("ntvuser", "");
    bakSetting().add("ntvAESpasswd", "", true);
#endif

    sysSetting().add("timezone", DEFAULT_TIME_ZONE);
    sysSetting().add("ntp", DEFAULT_NTP);
    sysSetting().add("ntp1", DEFAULT_NTP1);
    sysSetting().add("stb_softtype", DEFAULT_STB_SOFT_TYPE); //stb soft type ? A:_pub, B C
    sysSetting().add("eds", DEFAULT_EDS); // main page URL
    sysSetting().add("eds1", DEFAULT_EDS1); // backup main page URL
    sysSetting().add("lang", DEFAULT_LANG); // system language

    sysSetting().add("pagewidth", DEFAULT_PAGE_WIDTH); //yx_para_epgmat
    sysSetting().add("pageheight", DEFAULT_PAGE_HEIGHT);

#ifdef Chongqing
    sysSetting().add("reboot_time", DEFAULT_REBOOT_TIME); // auto reboot times, -1 is don't auto reboot
#endif

    sysSetting().add("gotomenu", DEFAULT_GOTO_MENU);
    sysSetting().add("AutoStandbyMode", DEFAULT_AUTO_STANDBY_MODE);
    sysSetting().add("AutoStandbyTime", DEFAULT_AUTO_STANDBY_TIME);


    //need move to custom
    sysSetting().add("bootlogo_md5", "28780808");
    sysSetting().add("authbg_md5", "huawei");
    sysSetting().add("logSend", 0);
    sysSetting().add("isUseBakAddr", 0);

    /*************************** customer ********************************/
    appSetting().add("CFversion", g_make_build_CFversion);
    appSetting().add("[ APP ]", "");

    appSetting().add("ntvuser", DEFAULT_NTV_USER);
    appSetting().add("ntvpasswd", DEFAULT_NTV_PASSWD);
    appSetting().add("ntvAESpasswd", DEFAULT_NTV_AES_PASSWD, true);
    appSetting().add("defContAcc", DEFAULT_DEF_CONT_ACC);
    appSetting().add("defContPwd", DEFAULT_DEF_CONT_PWD);
    appSetting().add("defAESContpwd", DEFAULT_DEF_AES_CONT_PWD, true);

    appSetting().add("epg_auth_flag", 0);         //?

    appSetting().add("epg", ""); //record the last EPG URL
    appSetting().add("areaid", "");
    appSetting().add("templateName", ""); //epg template
    appSetting().add("epg_PackageIDs", "");

    //daylight saving time
    appSetting().add("saveflag", 0);
    appSetting().add("jumpstep", 0);
    appSetting().add("lightstart", 0);
    appSetting().add("lightstop", 0);

    appSetting().add("startPicUpgradeURL", "");
    appSetting().add("PADBootLogPicURL", "");
    appSetting().add("PADAuthenBackgroundPicURL", "");
    appSetting().add("AuthShowPicFlag", DEFAULT_AUTH_SHOW_PIC_FLAG);
    appSetting().add("bootPicEnableFlag", DEFAULT_BOOT_PIC_ENABLE_FLAG); // whether to display the startPic, 1 means enable to display, disable on 0.
    appSetting().add("startPicEnableFlag", DEFAULT_START_PIC_ENABLE_FLAG); // when this value equal to 1, indicate that the bootPic will display when STB boot.

    appSetting().add("topmagin", DEFAULT_TOP_MAGIN);
    appSetting().add("bottommagin", DEFAULT_BOTTOM_MAGIN);
    appSetting().add("leftmagin", DEFAULT_LEFT_MAGIN);
    appSetting().add("rightmagin", DEFAULT_RIGHT_MAGIN);

    appSetting().add("pass_yuxing", "");
    appSetting().add("pass_huawei", "");
    appSetting().add("pass_huawei1", "");
    appSetting().add("pass_huaweiQtel", ""); // delete ?
    appSetting().add("pass_read", "");

    appSetting().add("userSTBName", DEFAULT_STB_NAME);

    appSetting().add("PADIRetryTime", PPPOE_RETRY_TIMES);
    appSetting().add("PADIRetryInterval", DEFAULT_PADIRETRYINTERVAL);
    appSetting().add("LCPRetryTime", DEFAULT_LCPRETRYTIME);
    appSetting().add("LCPRetryInterval", DEFAULT_LCPRETRYINTERVAL);
    appSetting().add("DHCPRetryTime", DHCP_RETRY_TIMES);
    appSetting().add("DHCPRetryInterval", DEFAULT_DHCPRETRYINTERVAL);
    appSetting().add("LinkProbeTime", DEFAULT_LINKPROBETIME);
    appSetting().add("StartupCaptured", DEFAULT_STARTUPCAPTURE);
    appSetting().add("StartupUploadAddr", "");
    appSetting().add("recoveryMode", -1);
    appSetting().add("HDMIConnect", 0);
    appSetting().add("UserField", DEFAULT_USERFIELD);

    appSetting().add("netCheckMulticastAdd", DEFAULT_NETCHECK_MULTICASTADD);
    appSetting().add("netCheckMulticastPort", DEFAULT_NETCHECK_MULTICASTPORT);
    appSetting().add("netCheckMulticastSourceAdd", DEFAULT_NETCHECK_MULTICASTSOURCEADD);
    appSetting().add("netCheckDomain", DEFAULT_NETCHECK_DOMAIN);
    appSetting().add("PingTestDomain", DEFAULT_NETCHECK_PINGTESTDOMAIN);
    appSetting().add("PingTestIp", DEFAULT_NETCHECK_PINGTESTIP);
    appSetting().add("PingTestDataSize", DEFAULT_NETCHECK_PINGTESTSIZE);
    appSetting().add("PingTestTimeout", DEFAULT_NETCHECK_PINGTESTTIMEOUT);
    appSetting().add("PingTestCount", DEFAULT_NETCHECK_PINGTESTCOUNT);
    appSetting().add("PingTestTTL", DEFAULT_NETCHECK_PINGTESTTTL);
    return 0;
}

} // namespace Hippo
extern "C"
int settingApp()
{
    return 0;
}


