/**
 * @file LogUDiskInstall.h
 * @brief
 *      中国联通家庭宽带多媒体应用业务平台技术规范第五分册 -与盒端接口分册
 *      10.1 快速放装 U 盘的要求
 * @author Michael
 * @version 1.0
 * @date 2012-09-13
 */
#ifndef _UDISKQUICKINSTALL_H_
#define _UDISKQUICKINSTALL_H_

#include "ConfigFileParser.h"
#include "UDiskAssertions.h"
#include "MessageTypes.h"
#include "BrowserAgent.h"

#include "AppSetting.h"
#include "SysSetting.h"

#include "Tr069.h"
#include "config/webpageConfig.h"

#include <string>
#include <iostream>

namespace Hippo {

const std::string kQuickInstallDir("CUSTBConfig");

const long long kMD5Link[4] = { 0x67452301L, 0xefcdab89L, 0x98badcfeL, 0x10325476L };

namespace UDisk {

/**
 * @class AuthServiceInfo
 * @brief Used to store the parsed data about authorization from 'UDisk' file
 */
class AuthServiceInfo {
  public :
      AuthServiceInfo() :
          mPPPOEID(""),mPPPOEPassword(""),mPPPOEID2(""),mPPPOEPassword2(""),
          mUserID(""),mUserIDPassword(""),mUserID2(""),mUserIDPassword2(""),
          mPPPOEEnable("") { };
      ~AuthServiceInfo() { };
      std::string mPPPOEEnable;                        /**< @brief pppoe enable switch */
      std::string mPPPOEID;                            /**< @brief pppoe id */
      std::string mPPPOEPassword;                      /**< @brief pppoe password */
      std::string mPPPOEID2;                           /**< @brief pppoe id2 */
      std::string mPPPOEPassword2;                     /**< @brief pppoe password2 */
      std::string mUserID;                             /**< @brief user id */
      std::string mUserIDPassword;                     /**< @brief user password */
      std::string mUserID2;                            /**< @brief user id2 */
      std::string mUserIDPassword2;                    /**< @brief user password2 */
};

/**
 * @class StbInfo
 * @brief Used to store the parsed data about stb from 'UDisk' file
 */
class StbInfo {
  public:
      StbInfo() :
          mSTBID(""),mOperatorInfo(""),mUserPassword(""),mUpgradeURL(""),
          mBrowserURL1(""),mBrowserURL2(""),mUserProvince("") { };
      ~StbInfo() { };
      std::string mSTBID;                              /**< @brief stb serial id */
      std::string mOperatorInfo;                       /**< @brief operator infomation */
      std::string mUserPassword;                       /**< @brief usr ordinary password */
      std::string mUpgradeURL;                         /**< @brief upgrade url */
      std::string mBrowserURL1;                        /**< @brief eds browser url */
      std::string mBrowserURL2;                        /**< @brief eds browser url2 */
      std::string mUserProvince;                       /**< @brief user location province */
};

/**
 * @class ManagementServer
 * @brief Used to store the parsed data about tr069 from 'UDisk' file
 */
class ManagementServer {
  public:
      ManagementServer():
          mConnectionRequestPassword(""), mConnectionRequestUsername(""),
          mManagementServerURL(""),mSTUNServerAddress(""),mSTUNServerPort("") { };
      ~ManagementServer() { };
      std::string mConnectionRequestUsername;          /**< @brief tr069 user name */
      std::string mConnectionRequestPassword;          /**< @brief tr069 user password */
      std::string mManagementServerURL;                /**< @brief tr069 url */
      std::string mSTUNServerAddress;                  /**< @brief tr069 simple transmmit server address */
      std::string mSTUNServerPort;                     /**< @brief tr069 simple transmmit server port */
};

enum _NetworkConnectType_ {
    ePPPOE = 1,
    eDHCP  = 2,
    eStatic= 3
};

/**
 * @class LanLinkInfo
 * @brief Used to store the parsed data about net link from 'UDisk' file
 */
class LanLinkInfo {
  public:
      LanLinkInfo( ):
          mAddressingType(""),mIPAddress(""),mSubnetMask(""),
          mDefaultGateway(""),mDNSServers(""),mDNSServers2("") { };
      ~LanLinkInfo() { };
      std::string mAddressingType;                     /** @brief ip address type */
      std::string mIPAddress;                          /** @brief ip address */
      std::string mSubnetMask;                         /** @brief subnet mask */
      std::string mDefaultGateway;                     /** @brief default gateway */
      std::string mDNSServers;                         /** @brief dns server */
      std::string mDNSServers2;                        /** @brief dns server2 */
};

/**
 * @class QuickInstallData
 * @brief Used to store the parsed data from 'UDisk" file
 */
class QuickInstallData {
  public:
      QuickInstallData( ) :
          mNTPServer(""),mNTPServer2("") { };
      ~QuickInstallData( ) { };
      AuthServiceInfo  mAuthServiceInfo;          /**< @brief authorization service information */
      StbInfo          mStbInfo;                  /**< @brief stb information */
      ManagementServer mManagementServer;         /**< @brief tr069 information */
      LanLinkInfo      mLanLinkInfo;              /**< @brief network link information */
      std::string mNTPServer;                          /**< @brief ntp server */
      std::string mNTPServer2;                         /**< @brief ntp server2 */
};

inline void
DataPPPOEEnableSet(const char* value)
{
    int i_v = atoi(value);
    int ret = 0;
    if (i_v)
        ret = sysSettingSetInt("connecttype", UDisk::ePPPOE);
    if (ret < 0)
        LogUDiskError("set pppoe enable [%s]\n", value);
}

inline void
DataPPPOEIDSet(const char* value)
{
    int ret = sysSettingSetString("netuser", value);
    if (ret < 0)
        LogUDiskError("set pppoe id [%s]\n", value);
}

inline void
DataPPPOEPasswordSet(const char* value)
{
    int ret = sysSettingSetString("netAESpasswd", value);
    if (ret < 0)
        LogUDiskError("set pppoe password [%s]\n", value);
}

inline void
DataUseIDSet(const char* value)
{
    int ret = appSettingSetString("ntvuser", value);
    if (ret < 0)
        LogUDiskError("set user account [%s]\n", value);
}

inline void
DataUserIDPasswordSet(const char* value)
{
    int ret = appSettingSetString("ntvAESpasswd", value);
    if (ret < 0)
        LogUDiskError("set user password [%s]\n", value);
}

inline void
DataUpgradeURLSet(const char* value)
{
    int ret = sysSettingSetString("upgradeUrl", value);
    if (ret < 0)
        LogUDiskError("set upgrade url [%s]\n", value);
}

inline void
DataBrowserURLSet(const char* value)
{
    if(value == NULL || (strncmp(value, "http://", 7) != 0)) {
        LogUDiskError("set eds url [%s] is error\n", value);
        return;
    }

    sysSettingSetString("eds", value);
    return;
}

inline void
DataBrowserURL2Set(const char* value)
{
    if(value == NULL || (strncmp(value, "http://", 7) != 0)) {
        LogUDiskError("set eds url [%s] is error\n", value);
        return;
    }

    sysSettingSetString("eds1", value);
    return;
}

#ifdef INCLUDE_TR069
inline void
DataManagementServerURL(char* value)
{
    int ret = TR069_API_SETVALUE("Device.ManagementServer.URL", value, 0);

    if (ret < 0)
        LogUDiskError("tr069_api_set_URL\n");
}

inline void
DataConnectionRequestUsername(char* value)
{
    int ret = TR069_API_SETVALUE("Device.ManagementServer.ConnectionUsername", value, 0);

    if (ret < 0)
        LogUDiskError("tr069_api_set_ConnectionUsername\n");
}

inline void
DataConnectionRequestPassword(char* value)
{
    int ret = TR069_API_SETVALUE("Device.ManagementServer.ConnectionPassword", value, 0);

    if (ret < 0)
        LogUDiskError("tr069_api_set_ConnectionPassword\n");
}

inline void
DataSaveToFlash()
{
    settingManagerSave();

    TR069_API_SETVALUE("Config.Save", "", 0);

}
#endif // #ifdef INCLUDE_TR069


inline void
DataAddressingTypeSet(const char* value)
{
    int ret = -1;
	int connectType = -1;
    switch (value[0]) {
        case 'p': //PPPOEE
        case 'P':
            connectType = UDisk::ePPPOE;
            break;
        case 'd': //DHCP
        case 'D':
            connectType = UDisk::eDHCP;
            break;
        case 's': //Static
        case 'S':
            connectType = UDisk::eStatic;
            break;
        default:
            ret = -1;
    }
	if(-1 != connectType)
		ret = sysSettingSetInt("connecttype", connectType);
    if (ret<0)
        LogUDiskError("set address type [%s]\n", value);
}

inline void
DataIPAddressSet(const char* value)
{
    int ret = sysSettingSetString("ip", value);
    if (ret < 0 )
        LogUDiskError("set ip [%s]\n", value);
}

inline void
DataSubnetMaskSet(const char* value)
{
    int ret = sysSettingSetString("netmask", value);
    if (ret < 0 )
        LogUDiskError("set netmask [%s]\n", value);
}

inline void
DataDefaultGatewaySet(const char* value)
{
    int ret = sysSettingSetString("gateway", value);
    if (ret < 0 )
        LogUDiskError("set gateway [%s]\n", value);
}

inline void
DataDNSServersSet(const char* value)
{
    int ret = sysSettingSetString("dns", value);
    if (ret < 0 )
        LogUDiskError("set dns [%s]\n", value);
}

inline void
DataDNSServers2Set(const char* value)
{
    int ret =sysSettingSetString("dns1", value);
    if (ret < 0 )
        LogUDiskError("set dns2 [%s]\n", value);
}


inline void
DataNTPServerSet(const char* value)
{
    int ret = sysSettingSetString("ntp", value);
    if (ret < 0 )
        LogUDiskError("set ntp [%s]\n", value);
}

inline void
DataNTPServer2Set(const char* value)
{
    int ret = sysSettingSetString("ntp1", value);
    if (ret < 0 )
        LogUDiskError("set ntp2 [%s]\n", value);
}


}

int UDiskInstallStart(int);
int UDiskQuickInstallDetect(int);

}

#endif
