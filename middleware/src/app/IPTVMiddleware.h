#ifndef __IPTVMIDDLEWARE_H_
#define __IPTVMIDDLEWARE_H_

#include <pthread.h>

typedef enum SettingFileType_ {
    eAppSetting   = 0,
    eSysSetting   = 1,
    eTr069Setting = 2,
    eDvbSetting   = 3,
    eMemSetting   = 4,
}SettingFileType_e;

typedef enum SettingValueType_ {
    eValueTypeString = 0,
    eValueTypeInt = 1,
}SettingValueType_e;

typedef enum AndroidMessage_ {
    eAndroidOpenUrl = 0,
    eAndroidSetSurface = 1,
    eAndroidStop = 2,
    eAndroidRestart = 3,
}AndroidMessage_t;


// TODO
// Fixme: 这个东西在Java里定义一个数值在C里又定义了一遍又没办法统一怎么可以这样用！
#define IPTV_MIDDLEWARE_IME_KEYBOARD    15


#ifdef __cplusplus

class NetworkDevice_;
class NetworkInterface;

class IPTVMiddleware {
public:
    IPTVMiddleware();
    virtual ~IPTVMiddleware();
    virtual void postEvent(int msg, int ext1, int ext2, void *obj = 0) = 0;
#ifdef EC6106V8_TEST
    virtual void authSucessedToAndroid() = 0;
    virtual void authFailedToAndroid() = 0;
#endif
#ifndef NEW_ANDROID_SETTING
    virtual void getParameter(const char* fieldName, char* fieldValue, int len) = 0;
    virtual void setParameter(const char* fieldName, const char* fieldValue) = 0;
#endif
    void run();
    void start();
    void restart();
    void stop();
    void resume();
    void pause();
    void destroy();
    void sendKeyEvent(int type, int value);
    void openUrl(const char* url);
    void onAuthAndOpenMenu();
    void onDoOpenMenu();
    void setSurface(int surface);
    void jGetParameter(int fileNameType, int fileValueType, const char* fieldName, char* fieldValue, int len);
    void jSetParameter(int fileNameType, int fileValueType, const char* fieldName, const char* fieldValue);
    static IPTVMiddleware* mIPTV;
    int getSurfaceHandle();
    void setSurfaceHandle(int handle);
    void setDisplayRect(int x, int y, int w, int h);
    void enterStandby();

    void jNetworkInfoRefresh();
private:
    int mRectX;
    int mRectY;
    int mRectW;
    int mRectH;
    int mSurfaceHandle;

    struct _NetworkParams {
        char nDirtyFlag;
        char nConnectType;
        char nAddressType;
        char nProtocolType;
        char nIfaceName[32];
        char nAddress[64];
        char nNetmask[64];
        char nGateway[64];
    } mNetworkParams;

    pthread_t mIPTVThreadID;
};

IPTVMiddleware& IPTVMiddlewareAgent();

void IPTVMiddleware_Notify(const char * msg);

extern "C" {
#endif

#ifndef NEW_ANDROID_SETTING
int IPTVMiddleware_SettingParamSync();
int IPTVMiddleware_SettingSetStr(const char* name, const char* value);
int IPTVMiddleware_SettingSetInt(const char* name, const int value);
char* IPTVMiddleware_SettingGetStr(const char* name, char* value, int len);
int IPTVMiddleware_SettingGetInt(const char* name, int* value);
#endif

void IPTVMiddleware_PostEvent(int msg, int ext1, int ext2, void* obj);
int IPTVMiddleware_GetSurfaceHandle();

#ifdef __cplusplus
}
#endif

#endif //__IPTVMIDDLEWARE_H_
