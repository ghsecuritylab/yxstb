
#include "Assertions.h"

#include "IPTVMiddleware.h"
#include "customer.h"
#include "stbinfo/stbinfo.h"
#include "iptv_logging.h"
#include "concatenator_hw_logout.h"

#include "MainThread.h"
#include "KeyDispatcher.h"
#include "SystemManager.h"
#include "NativeHandler.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "BrowserAgent.h"
#include "StringData.h"
#include "NetworkFunctions.h"
#include "AndroidNetwork.h"
#include "MessageValueNetwork.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "app/app_heartbit.h"
#include "app/app_epg_para.h"

#include "cutils/properties.h"

#include "mid_stream.h"
#include "mid/mid_http.h"


#include "jpeg/jpeglib.h"
#include "libzebra.h"
#include "mid/mid_sem.h"
#include "webpageConfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


extern const char* gNetworkCardNames[2];
extern char* global_cookies;

IPTVMiddleware* IPTVMiddleware::mIPTV = 0;

extern "C" int ygp_layer_setDisplaySurface(int surfaceHandle);
extern "C" void Jvm_Main_Close();
extern "C" int iptv_start(int argc, char**argv);



static int s_run = 0;
static void* IptvStart(void* param)
{
    mid_sem_t sem = (mid_sem_t)param;
    char* argv[] = {"iptvstart", LOCAL_WEBPAGE_PATH_PREFIX"/boot.html"};
    iptv_start(2, argv);
    mid_sem_give(sem);
    mainThreadRun();
    return 0;
}

IPTVMiddleware::IPTVMiddleware()
    :mRectX(0),
    mRectY(0),
    mRectW(1280),
    mRectH(720),
    mSurfaceHandle(0)
{
    mIPTV = this;
    memset(&mNetworkParams, 0, sizeof(struct _NetworkParams));
}

IPTVMiddleware::~IPTVMiddleware()
{
    mIPTV = 0;
}

void
IPTVMiddleware::run()
{
    if (!s_run) {
        mid_sem_t sem = mid_sem_create();
        pthread_attr_t attributes;
        pthread_attr_init(&attributes);
        pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
        pthread_create(&mIPTVThreadID, &attributes, IptvStart, sem);
        pthread_attr_destroy(&attributes);
        mid_sem_take(sem, 0x0fffffff, 1);
        s_run = 1;
        onAuthAndOpenMenu();
    }
}

void
IPTVMiddleware::setSurface(int surface)
{
    setSurfaceHandle(surface);
    if (s_run)
        sendMessageToNativeHandler(MessageType_Android, eAndroidSetSurface, surface, 0);
}

void
IPTVMiddleware::setSurfaceHandle(int handle)
{
    mSurfaceHandle = handle;
}

int
IPTVMiddleware::getSurfaceHandle()
{
    return mSurfaceHandle;
}

//setDisplayRect 6106V8的这个已经不走这里，在SurfaceJNI.cpp里setNativeDisplayRect_JNI处理。
void
IPTVMiddleware::setDisplayRect(int x, int y, int w, int h)
{
    mRectX = x;
    mRectY = y;
    mRectW = w;
    mRectH = h;
}

void
IPTVMiddleware::enterStandby()
{
    Hippo::systemManager().destoryAllPlayer();
    httpHeartClr( );
    Hippo::ConcatenatorHwLogout    cl;
    std::string url = cl(Hippo::Customer().AuthInfo().AvailableEpgUrl());

#if defined(Sichuan)
    ctc_http_send_GETmessage(url.c_str(), NULL);
#else
    if(!mid_http_call(url.c_str(), (mid_http_f)httpDefaultFunc, 0, NULL, 0, global_cookies))
        sleep(2);
#endif
}

void
IPTVMiddleware::start()
{
    LogUserOperDebug("onStart");
    jNetworkInfoRefresh();
}

void
IPTVMiddleware::restart()
{
    LogUserOperDebug("onRestart");
#ifndef NEW_ANDROID_SETTING
    IPTVMiddleware_SettingParamSync();
#endif

    // 浼氬鑷?Unable to resume activity 閿欒
    // 鏆傛椂涓嶆槑鍘熷洜銆?
    /*
    appSettingSetInt("leftmagin", mRectX);
    appSettingSetInt("rightmagin", mRectX);
    appSettingSetInt("topmagin", mRectY);
    appSettingSetInt("bottommagin", mRectY);
    */
    sendMessageToNativeHandler(MessageType_Android, eAndroidRestart, 0, 0);
    onDoOpenMenu();
}

void
IPTVMiddleware::stop()
{
    LogUserOperDebug("onStop");
    sendMessageToNativeHandler(MessageType_Android, eAndroidStop, 0, 0);
}

void
IPTVMiddleware::resume()
{

}

void
IPTVMiddleware::pause()
{
    LogUserOperDebug("onPause");
    char value[64] = { 0 };

    property_get("sys.IptvExit", value, "0");
    if (!strcmp(value, "1")) {
        Hippo::systemManager().destoryAllPlayer();
        property_set("sys.IptvExit", "0");
    }
}

void
IPTVMiddleware::destroy()
{
    LogUserOperDebug("onDestroy");
    property_set("ctl.start", "sqm_control");
    Jvm_Main_Close();
}

void
IPTVMiddleware::sendKeyEvent(int type, int value)
{
    if (!s_run)
        return;
    LogUserOperDebug("sendKeyEvent type = %d, value = 0x%x\n", type, value);
    switch (type) {
    case MessageType_Network:
        if (1 == value)
            sendMessageToNativeHandler(MessageType_Network, MV_Network_PhysicalUp, 0, 0);
        if (2 == value)
            sendMessageToNativeHandler(MessageType_Network, MV_Network_PhysicalDown, 0, 0);
    case MessageType_Char:
        sendMessageToEPGBrowser(MessageType_Char, value, 0, 0);
        break;
    default:
        sendMessageToKeyDispatcher(MessageType_KeyDown, value, 0, 0);
    }
}

void
IPTVMiddleware::openUrl(const char* url)
{
    Hippo::StringData* data = new Hippo::StringData(url);
    if (data) {
        Hippo::Message *msg = Hippo::defNativeHandler().obtainMessage(MessageType_Android, eAndroidOpenUrl, 0, data);
        Hippo::defNativeHandler().sendMessage(msg);
        data->safeUnref();
    }
}

void
IPTVMiddleware::onAuthAndOpenMenu()
{
    sendMessageToNativeHandler(MessageType_System, MV_System_OpenMenuPage, 0, 4000);
}

void
IPTVMiddleware::onDoOpenMenu()
{
    LogUserOperError("Begin to Open Menu ...");
    sendMessageToKeyDispatcher(MessageType_KeyDown, EIS_IRKEY_MENU, 0, 0);
}

void
IPTVMiddleware::jGetParameter(int fileNameType, int fileValueType, const char* fieldName, char* fieldValue, int len)
{
    LogUserOperError("fileNameType = %d, fileValueType = %d, fileName = %s.\n", fileNameType, fileValueType, fieldName);
    return;
}

void
IPTVMiddleware::jSetParameter(int fileNameType, int fileValueType, const char* fieldName, const char* fieldValue)
{
    switch (fileNameType) {
    case eAppSetting: {
        if (fileValueType == eValueTypeString)
            appSettingSetString(fieldName, fieldValue);
        else if (fileValueType == eValueTypeInt)
            appSettingSetInt(fieldName, atoi(fieldValue));
        else
            LogUserOperError("jSetParameter:fileValueType[%d] is Error.", fileValueType);
        settingManagerSave();
        break;
    }
    case eSysSetting: {
        // Fixme: eds鍦板潃鍙樹簡鐨勬椂鍊欒涓嶈閲嶆柊鍥為椤碉紵
        if (fileValueType == eValueTypeString)
            sysSettingSetString(fieldName, fieldValue);
        else if (fileValueType == eValueTypeInt)
            sysSettingSetInt(fieldName, atoi(fieldValue));
        else
            LogUserOperError("jSetParameter:fileValueType[%d] is Error.", fileValueType);
        settingManagerSave();
        break;
    }
    case eTr069Setting:
        break;
    case eDvbSetting:
        break;
    case eMemSetting: {
        const char* pStr = fieldName;
        if (!strncmp("network.", pStr, strlen("network."))) {
            pStr += strlen("network.");
            if (!strncmp("connectType", pStr, strlen("connectType"))) {
                if (!strncmp("wired", fieldValue, 5))
                    mNetworkParams.nConnectType = NetworkCard::NT_ETHERNET;
                else
                    mNetworkParams.nConnectType = NetworkCard::NT_WIRELESS;
                mNetworkParams.nDirtyFlag = 1;
                return;
            }
            if (!strncmp("mainIfaceName", pStr, strlen("mainIfaceName"))) {
                strncpy(mNetworkParams.nIfaceName, fieldValue, sizeof(mNetworkParams.nIfaceName));
                mNetworkParams.nDirtyFlag = 1;
                return;
            }

            if (!strncmp(mNetworkParams.nIfaceName, pStr, strlen(mNetworkParams.nIfaceName))) {
                pStr += strlen(mNetworkParams.nIfaceName) + 1;
                if (!strncmp("protocolType", pStr, strlen("protocolType"))) {
                    if (!strncmp("pppoe", fieldValue, 5))
                        mNetworkParams.nProtocolType = NetworkInterface::PT_PPPOE;
                    else if (!strncmp("dhcp", fieldValue, 4)) {
                        mNetworkParams.nProtocolType = NetworkInterface::PT_DHCP;
                    } else
                        mNetworkParams.nProtocolType = NetworkInterface::PT_STATIC;
                    mNetworkParams.nDirtyFlag = 1;
                    return;
                }
                if (!strncmp("addressType", pStr, strlen("addressType"))) {
                    if (!strncmp("v4", fieldValue, 2))
                        mNetworkParams.nAddressType = NetworkInterface::AT_IPV4;
                    else
                        mNetworkParams.nAddressType = NetworkInterface::AT_IPV6;
                    mNetworkParams.nDirtyFlag = 1;
                    return;
                }
                if (!strcmp("address", pStr)) {
                    strncpy(mNetworkParams.nAddress, fieldValue, sizeof(mNetworkParams.nAddress));
                    mNetworkParams.nDirtyFlag = 1;
                    return;
                }
                if (!strcmp("netmask", pStr)) {
                    strncpy(mNetworkParams.nNetmask, fieldValue, sizeof(mNetworkParams.nNetmask));
                    mNetworkParams.nDirtyFlag = 1;
                    return;
                }
                if (!strcmp("gateway", pStr)) {
                    strncpy(mNetworkParams.nGateway, fieldValue, sizeof(mNetworkParams.nGateway));
                    mNetworkParams.nDirtyFlag = 1;
                    return;
                }
            }
        }
        break;
    }
    default:
        break;
    }
}

void
IPTVMiddleware::jNetworkInfoRefresh()
{
    if (!mNetworkParams.nDirtyFlag)
        return;

    //Debug, This can remove
    LogUserOperError("connect[%d] address[%d] protocol[%d] name[%s] address[%s] netmask[%s] gateway[%s]\n",
        mNetworkParams.nConnectType, mNetworkParams.nAddressType, mNetworkParams.nProtocolType,
        mNetworkParams.nIfaceName, mNetworkParams.nAddress, mNetworkParams.nNetmask, mNetworkParams.nGateway);

    NetworkInterface* iface = 0;
    if (NetworkCard::NT_ETHERNET == mNetworkParams.nConnectType)
        iface = new NetworkInterfaceAndroid(new WiredNetworkCard(gNetworkCardNames[0]), mNetworkParams.nIfaceName);
    else
        iface = new NetworkInterfaceAndroid(new WirelessNetworkCard(gNetworkCardNames[1]), mNetworkParams.nIfaceName);

    if (!iface)
        return;

    if (NetworkInterface::AT_IPV4 == mNetworkParams.nAddressType) {
        IPv4Setting ipv4Conf;
        ipv4Conf.setAddress(mNetworkParams.nAddress);
        ipv4Conf.setNetmask(mNetworkParams.nNetmask);
        ipv4Conf.setGateway(mNetworkParams.nGateway);
        iface->setIPv4Setting(ipv4Conf);
    } else {
        IPv6Setting ipv6Conf;
        ipv6Conf.setAddress(mNetworkParams.nAddress);
        ipv6Conf.setGateway(mNetworkParams.nGateway);
        iface->setIPv6Setting(ipv6Conf);
    }

    NetworkCard* device = networkManager().getActiveDevice();
    if (device)
        networkManager().delDevice(device->devname());

    networkManager().setActiveDevice(iface->device());
    networkManager().setActiveInterface(iface);
    networkManager().addDevice(iface->device());
    networkManager().addInterface(iface);

    mNetworkParams.nDirtyFlag = 0;
}

IPTVMiddleware& IPTVMiddlewareAgent()
{
    return *(IPTVMiddleware::mIPTV);
}

extern "C" void IPTVMiddleware_PostEvent(int msg, int ext1, int ext2, void* obj)
{
    IPTVMiddlewareAgent().postEvent(msg, ext1, ext2, (char*)obj);
}

#ifndef NEW_ANDROID_SETTING

extern "C" int IPTVMiddleware_SettingSetStr(const char* name, const char* value)
{
    IPTVMiddlewareAgent().setParameter(name, value);
    return 0;
}

extern "C" int IPTVMiddleware_SettingSetInt(const char* name, const int value)
{
    char str[32] = "";
    snprintf(str, 32, "%d", value);
    IPTVMiddlewareAgent().setParameter(name, str);
    return 0;
}

extern "C" char* IPTVMiddleware_SettingGetStr(const char* name, char* value, int len)
{
    IPTVMiddlewareAgent().getParameter(name, value, len);
    return value;
}

extern "C" int IPTVMiddleware_SettingGetInt(const char* name, int* value)
{
    char str[32] = {0};
    IPTVMiddlewareAgent().getParameter(name, str, 32);
    if (str[0])
        *value = atoi(str);
    return *value;
}

extern "C" void osex_ipaddr_set(const char*, char*);
extern "C" int IPTVMiddleware_SettingParamSync()
{
    const int size = 1024;
    char buffer[size];

    // IPTVMiddlewareAgent().setParameter("eds", "http://110.1.1.132:33200/EPG/jsp/AuthenticationURL");

    memset(buffer, 0, size);
    IPTVMiddlewareAgent().getParameter("eds", buffer, size);
    sysSettingSetString("eds", buffer);
    // sysSettingSetString("eds", "http://10.0.0.1/test/t.html");
    // RUN_HERE() << buffer;

    memset(buffer, 0, size);
    IPTVMiddlewareAgent().getParameter("eds1", buffer, size);
    sysSettingSetString("eds1", buffer);


    // memset(buffer, 0, size);
    // IPTVMiddlewareAgent().getParameter("volume", buffer, size);
    // appSettingSetInt("volume", atoi(buffer));

    memset(buffer, 0, size);
    IPTVMiddlewareAgent().getParameter("nettype", buffer, size);
    sysSettingSetInt("nettype", atoi(buffer));

    memset(buffer, 0, size);
    IPTVMiddlewareAgent().getParameter("connecttype", buffer, size);
    sysSettingSetInt("connecttype", atoi(buffer));

    settingManagerSave();

    return 0;
}
#endif

extern "C" int IPTVMiddleware_GetSurfaceHandle()
{
    return IPTVMiddlewareAgent().getSurfaceHandle();
}

void IPTVMiddleware_Notify(const char * msg)
{
    // RUN_HERE() << "msg = " << msg;
    if (strcmp(msg, "NETWORK_CONNECT") == 0) {
        if (!s_run)
            ;
           // IPTVMiddlewareAgent().run();
    } else if (strcmp(msg, "IPTV_STOP") == 0) {
        if (s_run)
            IPTVMiddlewareAgent().stop();
    } else if (strcmp(msg, "IPTV_RESTART") == 0) {
        if (s_run)
            IPTVMiddlewareAgent().restart();
    } else if (strncmp(msg, "NETWORK_CHANGED:", 16) == 0) {
        AndroidNetworkStateChanaged(msg + 16);
    }
}


//----------------------------------- SDK need add --------------------------------

#if 0
extern "C" int yhw_vout_setHDMIAudioSelfAdaption(int)
{
    return -1;
}
#endif

extern "C" cairo_status_t cairo_surface_write_to_png_stream(cairo_surface_t*, cairo_status_t (*)(void*, const unsigned char*, unsigned int), void*)
{
    return CAIRO_STATUS_INVALID_RESTORE;
}

extern "C" void jCalcDimensions(void *info)
{
    zebra_jCalcDimensions((j_decompress_ptr)info);
}

extern "C" int yhw_upgrade_checkSoftware(char *softwareBytes, int length)
{
    return -1;
}

extern "C" int yhw_upgrade_burnSoftware(char *softwareBytes, int length)
{
    return -1;
}

extern "C" void yhw_upgrade_getStatus(int *begin, int *cur, int *compress, int *real)
{
    return ;
}

extern "C" int yhw_board_getDolbySupport()
{
    return 0;
}

#if 0
//---------------------------------- Temp Log ------------------------------------
android_iptv_log(__func__, __LINE__, __FILE__, "Info:");
extern "C" void android_iptv_log(const char* func, int line, const char* file, const char* log)
{
    static FILE* fp = NULL;
    if (!fp)
        fp = fopen("/var/log.txt", "wb");
    char buff[1024] = "";
    snprintf(buff, 1023, "#####Android[%s:%d][%s]:%s\n", func, line, strrchr(file, '/') + 1, log);
    fwrite(buff, 1, strlen(buff), fp);
    fflush(fp);
}
#endif
