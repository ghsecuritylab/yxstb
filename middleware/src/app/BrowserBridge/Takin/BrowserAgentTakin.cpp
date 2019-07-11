
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

#include "BrowserAgentTakin.h"
#include "BrowserAssertions.h"

#include "BrowserView.h"
#include "Message.h"
#include "MessageTypes.h"
#include "SystemManager.h"

#include "BrowserBridge/Huawei/BrowserEventQueue.h"

#include "config/webpageConfig.h"
#include "sys_basic_macro.h"
#include "AppSetting.h"
//#include "TAKIN_Media.h"
#include "TAKIN_setting_type.h"
#include "TAKIN_event_type.h"
#include "NativeHandler.h"
#include "BootImagesShow.h"

#include "Tr069.h"
#include "TAKIN_browser.h"

#include "browser_event.h"
#include "UtilityTools.h"
#include "mid_sys.h"

#include "NetworkFunctions.h"

#if defined(BROWSER_INDEPENDENCE)
extern "C" void TAKIN_browser_getVersion(int*svn, char** time, char** builder);
extern "C" void TAKIN_browser_paint(int* handle);
extern "C" void TAKIN_browser_setSetting(TAKIN_SETTING_TYPE type, char* buffer, int bufferLen);
extern "C" void TAKIN_browser_getSetting(TAKIN_SETTING_TYPE type, char* buffer, int bufferLen);
extern "C" int TAKIN_browser_createWindow(int **handle, char *url, int surface);
extern "C" int TAKIN_browser_closeWindow(int* handle);
extern "C" int TAKIN_socket_cleanup(int* handle);
extern "C" int TAKIN_browser_fireEvent(int* handle, InputEvent *event);
extern "C" int TAKIN_browser_setFocusObject(int *handle, char *objectCLSID);
extern "C" int TAKIN_browser_goBack(int *handle);
extern "C" int TAKIN_browser_loadURL(int* handle, char* url);
#endif

extern "C" void TAKIN_Proc_Key(void *handle, int msg, unsigned int p1, unsigned int p2);
extern "C" int JVM_audio_open();
extern "C" int JVM_audio_close();
extern "C" int TAKIN_jvm_getVersion(void);
extern "C" int HardwareVersion(char *hard_type, int len);
#if defined(Jiangsu)// || defined(Chongqing) // Fixme: 这是中兴EPG的通用处理逻辑。不一定需要浏览器配置设置。
extern "C" int getMenuHandleFlag(void);
extern "C" void TAKIN_porting_setMenuHandleFlag(int flag);
extern "C" void keyUnprocessedMenu(void);
#endif
namespace Hippo {

BrowserAgentTakin::BrowserAgentTakin()
    : mBrowserHandle(0)
    , mOpeningUrl(0)
    , mJvmStatus(0)
{
    memset(mCurrentUrl, 0, MAX_URL_LEN);

#ifdef NONE_BROWSER
#else
    // 所有局点的cookie目录都扔到var去。否则断电重启后请求eds会带上cookie。// 陕西先报的这个问题，后来哈尔滨版本也报了。
    TAKIN_browser_setSetting(TAKIN_COOKIE_PATH, "/var/takincookies", 32);

    TAKIN_browser_setSetting(TAKIN_BROWSER_OFFSET_RECT, "0 0 0 0", 32);
    TAKIN_browser_setSetting(TAKIN_2D_THRESHOLD,"-1", 3);

    TAKIN_browser_setSetting(TAKIN_LOG_LEVEL, "0", 2); /*level : 0 no info output,  9 output all info*/
    //add error page set,because the url is unfixed in the different stb.
    TAKIN_browser_setSetting(TAKIN_LOADER_ERROR, LOCAL_WEBPAGE_PATH_ERROR, strlen(LOCAL_WEBPAGE_PATH_ERROR) + 1);

#if defined(HUAWEI_C10)
    char tempUserAgent[256] = {0};
    char tempHardwareVersion[16] = {0};

    HardwareVersion(tempHardwareVersion, 16);
    if(0 == strcmp(tempHardwareVersion, "M8043V02")) {
        memset(tempHardwareVersion, 0, 16);
        strcpy(tempHardwareVersion, "EC2108V3");
    }
#if(SUPPORTE_HD)
    sprintf(tempUserAgent, "Mozilla/5.0 (compatible; EIS iPanel 2.0; Linux2.4.26/mips; win32; HI3110) AppleWebKit/2.0 (KHTML, like Gecko) %s Hybroad;Resolution(PAL,720P,1080i)", tempHardwareVersion);
#else
    sprintf(tempUserAgent, "Mozilla/5.0 (compatible; EIS iPanel 2.0; Linux2.4.26/mips; win32; HI3110) AppleWebKit/2.0 (KHTML, like Gecko) %s Hybroad;Resolution(PAL)", tempHardwareVersion);
#endif
    TAKIN_browser_setSetting(TAKIN_USERAGENT, tempUserAgent, strlen(tempUserAgent) + 1);
#else
    // TAKIN_browser_setSetting(TAKIN_CAN_CROSS_ORIGIN, "1", 2);
    #define USER_AGENT "Mozilla/5.0 (X11; Linux i686; en-US) AppleWebKit/534.0 (KHTML, like Gecko)"
    TAKIN_browser_setSetting(TAKIN_USERAGENT, USER_AGENT, strlen(USER_AGENT) + 1);
#endif

#if defined(VIETTEL_HD)
    TAKIN_browser_setSetting(TAKIN_FRAME_SIZE, "1280*720", 9);
#endif
#if defined(Cameroon_v5)
    char navigaterName[] = "webkit-2.2.1";
    TAKIN_browser_setSetting(TAKIN_NAVIGATOR_BROWSERVERSION, navigaterName, strlen(navigaterName));
#endif
#if defined(Huawei_v5)
    char tzone[32] = "";
    int timeZone = 0;
    sysSettingGetInt("timezone", &timeZone, 0);

    snprintf(tzone, 31, "%d", timeZone);
    TAKIN_browser_setSetting(TAKIN_MID_TIMEZONE, tzone, strlen(tzone) + 1);
    TAKIN_browser_setSetting(TAKIN_SCREEN_SIZE, "1280*720", 9);
#endif
#if defined(SHANGHAI_HD)|| defined(SHANGHAI_SD)
    TAKIN_browser_setSetting(TAKIN_DEFAULT_CHARSET, "gbk", 32);
    TAKIN_browser_setSetting(TAKIN_CAN_SCROLL, "1", 2);
    TAKIN_browser_setSetting(TAKIN_NAVIGATOR_PLATFORM, "Z112", 32);
#endif
#if defined(ANDROID)
    TAKIN_browser_setSetting(TAKIN_SETTING_DATAPATH, CONFIG_FILE_DIR"/takin", 32);
    char ca[] = "/system/etc/security";
    TAKIN_browser_setSetting(TAKIN_CA_PATH, ca, strlen(ca) + 1);
    TAKIN_browser_setSetting(TAKIN_ENABLE_PAGEVIEWSIZE, "1", 2);
    TAKIN_browser_setSetting(TAKIN_IE_MODE, "1", 2); // 1为开启IE兼容模式，0为关闭兼容模式。
    TAKIN_browser_setSetting(TAKIN_START_SHOT, "100", 4);
    TAKIN_browser_setSetting(TAKIN_FRAME_SIZE, "640*530", 8);
#endif
#endif
}

BrowserAgentTakin::~BrowserAgentTakin()
{
}

int
BrowserAgentTakin::ReadJvmVersion(void)
{
    typedef int (*LPFNJVMGETVERSION)(void);
    const char * jvmso = "/home/"PLATFORM_HOME"/bin/jvm.so";
    void * handle = dlopen(jvmso, RTLD_LAZY);

    if (!handle) {
        BROWSER_LOG("dlopen(jvm.so) failed: %s\n", dlerror());
        return -1;
    }
    LPFNJVMGETVERSION fnjvm_getVersion = (LPFNJVMGETVERSION)dlsym(handle, "TAKIN_jvm_getVersion");
    if (!fnjvm_getVersion) {
        BROWSER_LOG("dlsym(jvm_getVersion) failed: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    int ret = fnjvm_getVersion();
    dlclose(handle);
    return ret;
}

int
BrowserAgentTakin::openUrl(const char *url)
{
#ifdef NONE_BROWSER
#else
    BROWSER_LOG("Browser open url(%s)\n", url);
    if(strlen(url) == 0 || NULL == url || strlen(url) >= MAX_URL_LEN) {
        strcpy(mCurrentUrl, LOCAL_WEBPAGE_PATH_ERROR);
        TR069_API_SETVALUE("ErrorCode", "", 10071);
        BROWSER_LOG_ERROR("BrowserAgentTakin::openUrl input url error!\n");
    } else {
        strcpy(mCurrentUrl, url);
    }
    //如果在JVM中在需先让JVM退出
    if(mJvmStatus) {
        mOpeningUrl = (char *)malloc(MAX_URL_LEN);
        strcpy(mOpeningUrl, mCurrentUrl);
        YX_INPUTEVENT jvm_event;
        memset(&jvm_event, 0, sizeof(YX_INPUTEVENT));
        jvm_event.eventkind = YX_EVENT_KEYDOWN;
        jvm_event.vkey = USEREVENT_JVM + JVM_FORCE_EXIT;
        TAKIN_browser_fireEvent(mBrowserHandle, &jvm_event);
    } else {
        BROWSER_LOG("BrowserAgentTakin::openUrl %s\n", mCurrentUrl);
        TAKIN_browser_loadURL(mBrowserHandle, mCurrentUrl);
    }
#endif
    return 0;
}

void
BrowserAgentTakin::handleMessage(Message *msg)
{
#ifdef NONE_BROWSER
#else
    if(msg == NULL) {
        BROWSER_LOG_VERBOSE("msg is null\n");
        return ;
    }
    if(msg->what == MessageType_Timer) {
        TAKIN_browser_fireEvent(mBrowserHandle, NULL);
        sendEmptyMessageDelayed(MessageType_Timer, 10);
    } else {
        BROWSER_LOG("BrowserAgentTakin::handleMessage: what 0x%x, arg1 0x%x ,arg2 0x%x!\n", msg->what, msg->arg1,msg->arg2);
        if(msg->what == MessageType_KeyDown) {
#if (defined(Jiangsu) && defined(EC1308H))// || defined(Chongqing) // Fixme: 这是中兴EPG的通用处理逻辑。不一定需要浏览器配置设置。
            if (msg->arg1 == EIS_IRKEY_MENU )
                TAKIN_porting_setMenuHandleFlag(0);

            TAKIN_Proc_Key(mBrowserHandle, 7, msg->arg1, 0);
            if (msg->arg1 == EIS_IRKEY_MENU ) {
                if (!getMenuHandleFlag())
                    keyUnprocessedMenu();
            }
#else
            TAKIN_Proc_Key(mBrowserHandle, 7, msg->arg1, 0);
#endif
        } else if(msg->what == MessageType_KeyUp) {
            TAKIN_Proc_Key(mBrowserHandle, -100, msg->arg1, 0);
#if defined(Huawei_v5) || defined(ANDROID)  //TODO temporary do it for USB keyboard
        } else if (msg->what == MessageType_Char) {
            YX_INPUTEVENT event;
            event.eventkind = YX_EVENT_KEYDOWN;
            event.keyvalue = 0;
            event.vkey = msg->arg1;
            event.unicode = msg->arg1;
            TAKIN_browser_fireEvent(mBrowserHandle, &event);
#endif
        } else if(msg->what == MessageType_JVM) {
            switch(msg->arg1 - USEREVENT_JVM) {
            case JVM_ENTERING: {
#if defined(hi3560e) || (defined(VIETTEL_HD) && defined(hi3716m))
                FILE *fp = NULL;
                char JvmVer[16] = {0};
                int JvmCurVer = 0, JvmFileVer = 0;
                DIR *Fd = NULL;

                /*jvm used osd graphic, destroy now*/
#if defined(hi3560e)
                if(0 != ygp_layer_destoryLayer())
                    BROWSER_LOG_ERROR("The layer destroy error !!!\n");
#endif
                JvmCurVer = ReadJvmVersion();   //TAKIN_jvm_getVersion();
#if defined(hi3560e)
                Fd = opendir("/root/jvm/");
#else
                Fd = opendir("/var/jvm/");
#endif
                if(Fd == NULL) {
#if defined(hi3560e)
                    yos_systemcall_runSystemCMD("tar xzvf /home/hybroad/bin/jvm.tar.gz -C /root/", NULL);
#else
                    yos_systemcall_runSystemCMD("tar xzvf /home/hybroad/bin/jvm.tar.gz -C /var/", NULL);
#endif
                    memset(JvmVer, 0, 16);
                    sprintf(JvmVer, "%d", JvmCurVer);
#if defined(hi3560e)
                    fp = fopen("/root/jvm/version", "w+");
#else
                    fp = fopen("/var/jvm/version", "w+");
#endif
                    if(fp != NULL) {
                        fwrite(JvmVer, 16, 1, fp);
                        fclose(fp);
                        sync();
                    }
                } else {
                    closedir(Fd);
#if defined(hi3560e)
                    fp = fopen("/root/jvm/version", "r");
#else
                    fp = fopen("/var/jvm/version", "r");
#endif
                    if(fp != NULL) {
                        memset(JvmVer, 0, 16);
                        fread(JvmVer, 16, 1, fp);
                        JvmFileVer = atoi(JvmVer);
                        fclose(fp);
                    }
                    if(JvmFileVer != JvmCurVer) {
#if defined(hi3560e)
                        yos_systemcall_runSystemCMD("tar xzvf /home/hybroad/bin/jvm.tar.gz -C /root/", NULL);
#else
                        yos_systemcall_runSystemCMD("tar xzvf /home/hybroad/bin/jvm.tar.gz -C /var/", NULL);
#endif
                        memset(JvmVer, 0, 16);
                        sprintf(JvmVer, "%d", JvmCurVer);
#if defined(hi3560e)
                        fp = fopen("/root/jvm/version", "w+");
#else
                        fp = fopen("/var/jvm/version", "w+");
#endif
                        if(fp != NULL) {
                            fwrite(JvmVer, 16, 1, fp);
                            fclose(fp);
                            sync();
                        }
                    }
                }
#endif

                enteringJvm();
                break;
            }
            case JVM_EXITING: {
                exitingJvm();
#if defined(hi3560e)
                int tScreenWidth = 0, tScreenHeight = 0;

                ygp_layer_getScreenSize(&tScreenWidth, &tScreenHeight);
                ygp_layer_createLayer(tScreenWidth, tScreenHeight, 0, 0, 9); //YX_COLOR_AYCbCr8888
#endif
#if (defined(VIETTEL_HD)&&defined(hi3716m))
                yos_systemcall_runSystemCMD("rm -rf /var/jvm",NULL);
#endif
                break;
            }
            default: {
                BROWSER_LOG_WARNING("Unknow JVM Event 0x%x\n", msg->arg1);
                break;
            }
        }
        }
        else if(msg->what == MessageType_Unknow) {
            TAKIN_Proc_Key(mBrowserHandle, 7, msg->arg1, 0);
            if(0x300 == msg->arg1)
                browserEventCheck(msg->arg2);
        } else if(msg->what == MessageType_Prompt) {
            BrowserAgent::handleMessage(msg);
        } else if(msg->what == MessageType_WaitClock) {
            if(1 == msg->arg1 || 2 == msg->arg1) {  /* TAKIN_OPEN_URL_END or TAKIN_LOADER_ERROR - see takin.c */
                BROWSER_LOG("open url end[1] or timeout[2] - arg1[%d]\n", msg->arg1);
                mWaitClock->hide(); /* hide clock gif */
                removeMessages(MessageType_WaitClock); /* remove the message from the message enqueue */
            } else { /* TAKIN_OPEN_URL_BEGIN - see takin.c */
                mWaitClock->show();/* show clock gif alternately */
                Message *tMsg = obtainMessage(MessageType_WaitClock, 0, 0);
                sendMessageDelayed(tMsg, 250); /* add delay message to the message enqueue */
            }
        } else {
            TAKIN_Proc_Key(mBrowserHandle, msg->what, msg->arg1, 0);
        }
    }
#endif
}

void
BrowserAgentTakin::closeAllConnection()
{
#ifdef NONE_BROWSER
#else
    TAKIN_socket_cleanup(mBrowserHandle);
#endif
}

void 
BrowserAgentTakin::setView(BrowserView *view)
{
    BrowserAgent::setView(view);

#ifdef NONE_BROWSER
#else
    if (!mBrowserHandle) {
        TAKIN_browser_createWindow(&mBrowserHandle, NULL, mView->mPlatformLayer);
        if (mBrowserHandle == NULL)
            BROWSER_LOG_ERROR("Takin browser create failed!\n");
    }
#endif
}

int
BrowserAgentTakin::getHandle()
{
    return (int)mBrowserHandle;
}

int
BrowserAgentTakin::testRestore(char* url) //For Test
{
#ifdef NONE_BROWSER
#else
    TAKIN_browser_createWindow(&mBrowserHandle, NULL, mView->mPlatformLayer);
    if (url)
        TAKIN_browser_loadURL(mBrowserHandle, url);
    else
        TAKIN_browser_loadURL(mBrowserHandle, mCurrentUrl);
#endif
    return 0;
}

void
BrowserAgentTakin::setCurrentUrl(char *pUrl)
{
    if(pUrl == NULL) {
        return;
    }
    if(strstr(pUrl, "config.html") != NULL) {
        return;
    }
    if(strlen(pUrl) < MAX_URL_LEN) {
        strcpy(mCurrentUrl, pUrl);
    }
    return ;
}

char *
BrowserAgentTakin::getCurrentUrl()
{
    return mCurrentUrl;
}

void
BrowserAgentTakin::getTakinVersion(int *svn, char **pTime, char **builer)
{
#ifdef NONE_BROWSER
#else
    TAKIN_browser_getVersion(svn, pTime, builer);
#endif
}

void
BrowserAgentTakin::setTakinSettings(int type, char *buffer, unsigned int bufferLen)
{
#ifdef NONE_BROWSER
#else
    TAKIN_browser_setSetting((TAKIN_SETTING_TYPE)type, buffer, bufferLen);
#endif
}

void
BrowserAgentTakin::getTakinSettings(int type, char *buffer, unsigned int bufferLen)
{
#ifdef NONE_BROWSER
#else
    TAKIN_browser_getSetting((TAKIN_SETTING_TYPE)type, buffer, bufferLen);
#endif
}
void
BrowserAgentTakin::cleanTakinCache()
{
#ifdef NONE_BROWSER
#else
    TAKIN_browser_cleanCache();
#endif
}

void
BrowserAgentTakin::setTakinEDSFlag(int flag)
{
#ifdef NONE_BROWSER
#else
    Takin_Set_EdsFlag(flag);
#endif
}

void
BrowserAgentTakin::closeBrowser()
{
#ifdef NONE_BROWSER
#else
    if(mBrowserHandle) {
        TAKIN_browser_closeWindow(mBrowserHandle);
        mBrowserHandle = 0;
    }
#endif
}

int
BrowserAgentTakin::getJvmStatus()
{
    return mJvmStatus;
}

void
BrowserAgentTakin::enteringJvm()
{
#ifdef NONE_BROWSER
#else
    char temp[1024] = {0};
    char buf[1024] = {0};
    FILE *fp = NULL;
    int len = 0;
    char ifname[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);

    fp = fopen("/var/jvm_stbinfo", "w");
    if(fp == NULL) {
        BROWSER_LOG_ERROR("BrowserAgentTakin::enteringJvm open /var/jvm_stbinfo error!\n");
        return;
    }
    appSettingGetString("ntvuser", temp, 1024, 0);
    len += snprintf(buf + len, 1024 - len, "ServiceUser=%s\n", temp);

    memset(temp, 0, 1024);
    mid_sys_serial(temp);

    len += snprintf(buf + len, 1024 - len, "ProductID=%s\n", temp);
    len += snprintf(buf + len, 1024 - len, "IpAddr=%s\n", network_address_get(ifname, temp, 1024));
    fwrite(buf, strlen(buf) + 1, 1, fp);
    fclose(fp);
    JVM_audio_open();
    mJvmStatus = 1;
    TAKIN_browser_setFocusObject(mBrowserHandle, "CAFEEFAC-0014-0002-FFFF-ABCDEFFEDCBA");
#endif
}

void
BrowserAgentTakin::exitingJvm()
{
#ifdef NONE_BROWSER
#else
    JVM_audio_close();
    mJvmStatus = 0;
#if defined(SHANGHAI_HD) || defined(SHANGHAI_SD)
    char tEvent[] = {"{type:\"EVENT_JVM_CLIENT\",event_code:0,event_result:0}"};

    browserEventSend(tEvent, NULL);
    strcpy(tEvent, "{type:\"EVENT_JVM_CLIENT\",event_code:1,event_result:0}");
    browserEventSend(tEvent, NULL);
    strcpy(tEvent, "{type:\"EVENT_JVM_CLIENT\",event_code:2,event_result:0}");
    browserEventSend(tEvent, NULL);
    strcpy(tEvent, "{type:\"EVENT_JVM_CLIENT\",event_code:3,event_result:0}");
    browserEventSend(tEvent, NULL);
    TAKIN_browser_setFocusObject(mBrowserHandle, 0); // jvm 结束时置焦点
#else
    if(mOpeningUrl) {
        TAKIN_browser_loadURL(mBrowserHandle, mOpeningUrl);
        free(mOpeningUrl);
        mOpeningUrl = NULL;
    } else {
        TAKIN_browser_goBack(mBrowserHandle);
    }
#endif
#endif
}

static BrowserAgentTakin *gBrowserAgent = NULL;

BrowserAgent &epgBrowserAgent()
{
    return *gBrowserAgent;
}

} // namespace Hippo

extern "C" void
epgBrowserAgentCreate()
{
    Hippo::gBrowserAgent = new Hippo::BrowserAgentTakin();
}

extern "C" int
epgBrowserAgentOpenUrl(const char* url)
{
    return Hippo::gBrowserAgent->openUrl(url);
}

extern "C" int
epgBrowserAgentTestRestore(char* url)
{
    return Hippo::gBrowserAgent->testRestore(url);
}

extern "C" int
epgBrowserAgentGetHandle()
{
    return (int)Hippo::gBrowserAgent->getHandle();
}

extern "C" void
epgBrowserAgentSetCurrentUrl(char *pUrl)
{
    return Hippo::gBrowserAgent->setCurrentUrl(pUrl);
}

extern "C" char *
epgBrowserAgentGetCurrentUrl()
{
    return Hippo::gBrowserAgent->getCurrentUrl();
}

extern "C" int
epgBrowserAgentGetJvmStatus()
{
    return Hippo::gBrowserAgent->getJvmStatus();
}

extern "C" void
epgBrowserAgentCloseBrowser()
{
    Hippo::gBrowserAgent->closeBrowser();
}

extern "C" void
epgBrowserAgentGetTakinVersion(int *svn, char **pTime, char **builer)
{
    Hippo::gBrowserAgent->getTakinVersion(svn, pTime, builer);
}

extern "C" void
epgBrowserAgentSetTakinSettings(int type, char *buffer, int bufferLen)
{
    Hippo::gBrowserAgent->setTakinSettings(type, buffer, bufferLen);
}

extern "C" void
epgBrowserAgentgetTakinSettings(int type, char *buffer, int bufferLen)
{
    Hippo::gBrowserAgent->getTakinSettings(type, buffer, bufferLen);
}

#if defined(Huawei_v5)
extern "C" void
epgBrowserAgentSetTimezone(int tzone)
{
    char stz[32] = "";
    snprintf(stz, 31, "%d", tzone);
    Hippo::gBrowserAgent->setTakinSettings(TAKIN_MID_TIMEZONE, stz, strlen(stz) + 1);
}
#endif

extern "C" void
epgBrowserAgentCleanTakinCache()
{
    Hippo::gBrowserAgent->cleanTakinCache();
}
#if defined(Gansu) || defined(C30)  //to finish off ld error
static int  MenuHandleFlag = 0; //用于给浏览器标记是否EPG有处理首页键.
extern "C" void
TAKIN_porting_setMenuHandleFlag(int flag)
{
    MenuHandleFlag = flag;
}
#endif

#if (defined(Jiangsu) && defined(EC1308H))// || defined(Chongqing) // Fixme: 这是中兴EPG的通用处理逻辑，不一定需要浏览器配置设置。
static int  MenuHandleFlag = 0; //用于给浏览器标记是否EPG有处理首页键.
extern "C" int
getMenuHandleFlag(void)
{
    return MenuHandleFlag;
}

extern "C" void
TAKIN_porting_setMenuHandleFlag(int flag)
{
    MenuHandleFlag = flag;
}

extern "C" void
keyUnprocessedMenu(void)
{
    char mid_url[URL_MAX_LEN + 4] = { 0 };
    strncpy(mid_url, app_EPGDomian_get( ), URL_MAX_LEN);
    if (!strlen(mid_url))
        app_mainUrl_get(mid_url);

    BROWSER_LOG_ERROR("BrowserAgentTakin::jiangSuOpenMenu open url(%s)\n", mid_url);
    Hippo::defNativeHandler().setState(Hippo::NativeHandler::Running);;
    Hippo::epgBrowserAgent().openUrl(mid_url);
    BootImagesShowAuthLogo(0);
}
#elif defined(Chongqing)
extern "C" void
TAKIN_porting_setMenuHandleFlag(int flag)
{
}
#endif
