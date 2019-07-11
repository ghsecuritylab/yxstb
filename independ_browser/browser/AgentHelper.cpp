#include "AgentHelper.h"
#include "cairo.h"

#include "libzebra.h" //TODO for InputEvent
#include "TAKIN_setting_type.h" //for TAKIN_OPEN_URL_BEGIN
#include "TAKIN_types.h" //for TAKIN_browser

#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <errno.h>

#include "IiPanelCaller.h"
#include "IiPanelPageWidgetCaller.h"
#include "IAuthenticationCaller.h"
#include "IDeviceCaller.h"
#include "IMediaPlayerCaller.h"
#include "IUBankDeviceCaller.h"
#include "IUtilityCaller.h"

static AgentHelper gAgent;

typedef struct _CairoMap_ {
    int _mapId;  /**< @brief the file descriptor of '/dev/mem' */
    unsigned int _mapSize;  /**< @brief the length of the mapping */
    unsigned char* _mapAddr; /**< @brief the address of the mapping */
    cairo_t* _cairo;  /**< @brief the the cairo's context handle , use it can draw something */
    cairo_surface_t* _surface; /**< @brief the surface address, provide the context for cairo */
}CairoMap_t;

static CairoMap_t gCairoMap = { -1, 0, 0, 0, 0 };

static cairo_t* _CreateCairo(int uDeviceAddress, int width, int height, int pitch)
{
#if defined(BROWSER_ROOT_RUN)
    if (!gCairoMap._cairo) {
        gCairoMap._mapId = open("/dev/mem", O_RDWR);
        if ( gCairoMap._mapId < 0 ) {
            perror("Unable to open /dev/mem for uncached memory");
            return NULL;
        }
        unsigned int uPageSize = sysconf(_SC_PAGE_SIZE);
        unsigned int uPageSizeMask = ~(uPageSize - 1);
        gCairoMap._mapSize = width * height * 4;
        gCairoMap._mapAddr =(unsigned char*)mmap(0, gCairoMap._mapSize, PROT_READ|PROT_WRITE, MAP_SHARED, gCairoMap._mapId, uDeviceAddress & uPageSizeMask);
        if (gCairoMap._mapAddr == (void*)-1) {
            perror("mmap:");
            return NULL;
        }
        LogIpcDebug("uDeviceAddress[0x%02x] width[%d] height[%d] pitch[%d] gCairoMap._mapAddr[0x%02x]\n", uDeviceAddress, width, height, pitch, gCairoMap._mapAddr);
        unsigned char* pMem = gCairoMap._mapAddr + (uDeviceAddress - (uDeviceAddress & uPageSizeMask));	
        gCairoMap._surface = cairo_image_surface_create_for_data(pMem, CAIRO_FORMAT_ARGB32, width,  height, pitch);
        gCairoMap._cairo = cairo_create(gCairoMap._surface);
    }
#else 
    if (gCairoMap._mapId < 0) {
        gCairoMap._mapSize = height * pitch;
        gCairoMap._mapId = shmget(LAYER_SHARE_KEY, gCairoMap._mapSize, IPC_CREAT | IPC_EXCL | 0666);
        if (-1 == gCairoMap._mapId) {
            if (errno == EEXIST)
                gCairoMap._mapId = shmget(LAYER_SHARE_KEY, 0, 0);
        } 
        if (gCairoMap._mapId > 0)
            gCairoMap._mapAddr = (unsigned char*)shmat(gCairoMap._mapId, NULL, SHM_RND);
        gCairoMap._surface = cairo_image_surface_create_for_data(gCairoMap._mapAddr, CAIRO_FORMAT_ARGB32, width,  height, pitch);
        gCairoMap._cairo = cairo_create(gCairoMap._surface);
    }
#endif
    return gCairoMap._cairo;
}

static void _DestroyCairo()
{
    if (-1 != gCairoMap._mapId)
        close(gCairoMap._mapId);
    if (gCairoMap._mapAddr)
        munmap(gCairoMap._mapAddr, gCairoMap._mapSize);
    if (gCairoMap._cairo) {
         cairo_destroy(gCairoMap._cairo);
         cairo_surface_destroy(gCairoMap._surface);
    }
    gCairoMap._cairo = NULL;
}

AgentHelper::AgentHelper():mNeedUpdateScreen(0)
{
    LogIpcTest("Static Agent Caller\n");
#if defined(DEBUG_BROWSER)
    system("ipcs -m | awk '$1 ~/0xabcdef[0-9][0-9]/ {print $2}' | while read s; do ipcrm -m $s; done");
    system("ipcs -s | awk '$1 ~/0xabcdef[0-9][0-9]/ {print $2}' | while read s; do ipcrm -s $s; done");
#endif

    setProgramName("BrowserRun");
    initialize(0); //0 for takin

    //add server global function
    addFunction("TakinBrowserGetVersion", &AgentHelper::TakinBrowserGetVersion);
    addFunction("TakinBrowserCreateWindow", &AgentHelper::TakinBrowserCreateWindow);
    addFunction("TakinBrowserCloseWindow", &AgentHelper::TakinBrowserCloseWindow);
    addFunction("TakinBrowserSocketCleanup", &AgentHelper::TakinBrowserSocketCleanup);
    addFunction("TakinBrowserFireEvent", &AgentHelper::TakinBrowserFireEvent);
    addFunction("TakinBrowserSetSetting", &AgentHelper::TakinBrowserSetSetting);
    addFunction("TakinBrowserGetSetting", &AgentHelper::TakinBrowserGetSetting);
    addFunction("TakinBrowserLoadURL", &AgentHelper::TakinBrowserLoadURL);
    addFunction("TakinBrowserGoback", &AgentHelper::TakinBrowserGoback);
    addFunction("TakinBrowserSetFocusObject", &AgentHelper::TakinBrowserSetFocusObject);
    addFunction("TakinBrowserPaint", &AgentHelper::TakinBrowserPaint);
}

AgentHelper::~AgentHelper()
{
}

//Caller Functions
int
AgentHelper::TakinPortingLoadNotification(int stateCode, char* buffer, int bufferLen)
{
    int ret = -1;

    initStack(SP_Call);
    pushArg("buffer", buffer);
    pushArg("stateCode", stateCode);
    call("TakinPortingLoadNotification");
    /*
     * Takin: 
     *  #1. bufferLen = strlen(buffer) when stateCode is TAKIN_OPEN_URL_BEGIN or TAKIN_OPEN_URL_END (popArg warning but nothing)
     *  #2. bufferLen = sizeof(buffer) when stateCode is TAKIN_LOADER_ERROR
     */
    popArg("return", ret);
    if (TAKIN_LOADER_ERROR == stateCode)
        popArg("buffer", buffer, bufferLen);
    freeStack();
    return ret;
}

int
AgentHelper::TakinPortingSetFrameSize(int type, char *buffer, int bufferLen)
{
    initStack(SP_Call);
    pushArg("bufferLen", bufferLen);
    pushArg("buffer", buffer);
    pushArg("type", type);
    call("TakinPortingSetFrameSize");
    freeStack();
    return 0;
}

int 
AgentHelper::TakinPortingInvalidateWindow(int* x, int* y, int* w, int* h)
{
    //TODO private now
    int ret = -1;
    int tx = 0, ty = 0, tw = 0, th = 0;
    initStack(SP_Call);
    pushArg("h", *h);
    pushArg("w", *w);
    pushArg("y", *y);
    pushArg("x", *x);
    call("TakinPortingInvalidateWindow");
    popArg("return", ret);
    popArg("x", tx);
    popArg("y", ty);
    popArg("w", tw);
    popArg("h", th);
    freeStack();
    *x = tx;
    *y = ty;
    *w = tw;
    *h = th;
    return ret;
}

int 
AgentHelper::TakinPortingUpdateScreen()
{
    //TODO Never come heare
    int ret = -1;
    initStack(SP_Call);
    call("TakinPortingUpdateScreen");
    popArg("return", ret);
    freeStack();
    return ret;
}

int
AgentHelper::TakinPortingExtensionStart(const char* url)
{
    initStack(SP_Call);
    pushArg("url", url);
    call("TakinPortingExtensionStart");
    freeStack();
    return 0;
}

int
AgentHelper::TakinPortingExtensionStop(const char* url)
{
    initStack(SP_Call);
    pushArg("url", url);
    call("TakinPortingExtensionStop");
    freeStack();
    return 0;
}

int 
AgentHelper::TakinPortingGetTimeZone()
{
    //TODO Never come heare
    int ret = -1;
    initStack(SP_Call);
    call("TakinPortingGetTimeZone");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
AgentHelper::TakinPortingInputGetEvent(char* event, int size, int timeout)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("timeout", timeout);
    call("TakinPortingInputGetEvent");
    popArg("return", ret);
    popArg("event", event, size); 
    freeStack();
    return ret;
}

int 
AgentHelper::TakinPortingInputSendEvent(char* event, int size)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("event", event, size);
    call("TakinPortingInputSendEvent");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
AgentHelper::TakinPortingWindowGetEvent(char* event, int size, int timeout)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("timeout", timeout);
    call("TakinPortingWindowGetEvent");
    popArg("return", ret);
    popArg("event", event, size);
    freeStack();
    return ret;
}

//JVM
void 
AgentHelper::JVMNotifyEvent(int eventType, int eventValue)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("eventType", eventType);
    pushArg("eventValue", eventValue);
    call("JVMNotifyEvent");
    freeStack();
    popArg("return", ret);
}

int 
AgentHelper::MediaPlayerPrivateInit(void)
{
    int ret = -1;
    initStack(SP_Call);
    call("MediaPlayerPrivateInit");
    popArg("return", ret);
    freeStack();
    return ret;
}

void 
AgentHelper::MediaPlayerPrivateFinal(int playerID)
{
    initStack(SP_Call);
    call("MediaPlayerPrivateFinal");
    freeStack();
}

void 
AgentHelper::MediaPlayerPrivateLoad(int playerID, char* url)
{
    initStack(SP_Call);
    if (!url)
        pushArg("url", 0, 0);
    else 
        pushArg("url", url);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateLoad");
    freeStack();
}

void 
AgentHelper::MediaPlayerPrivateSizeChanged(int playerID, int x, int y, int w, int h)
{
    initStack(SP_Call);
    pushArg("h", h);
    pushArg("w", w);
    pushArg("y", y);
    pushArg("x", x);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateSizeChanged");
    freeStack();
}

void 
AgentHelper::MediaPlayerPrivatePlay(int playerID)
{
    initStack(SP_Call);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivatePlay");
    freeStack();
}

void 
AgentHelper::MediaPlayerPrivatePause(int playerID)
{
    initStack(SP_Call);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivatePause");
    freeStack();
}

int 
AgentHelper::MediaPlayerPrivateWidth(int playerID)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateWidth");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
AgentHelper::MediaPlayerPrivateHeight(int playerID)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateHeight");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
AgentHelper::MediaPlayerPrivateHasVideo(int playerID)
{
    int ret = -1;
    pushArg("playerID", playerID);
    initStack(SP_Call);
    call("MediaPlayerPrivateHasVideo");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
AgentHelper::MediaPlayerPrivateHasAudio(int playerID)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateHasAudio");
    popArg("return", ret);
    freeStack();
    return ret;
}

float 
AgentHelper::MediaPlayerPrivateDuration(int playerID)
{
    float ret = -1;
    initStack(SP_Call);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateDuration");
    popArg("return", ret);
    freeStack();
    return ret;
}

float 
AgentHelper::MediaPlayerPrivateCurrentTime(int playerID)
{
    float ret = -1;
    initStack(SP_Call);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateCurrentTime");
    popArg("return", ret);
    freeStack();
    return ret;
}

unsigned int 
AgentHelper::MediaPlayerPrivateBytesLoaded(int playerID)
{
    int ret = 0;
    initStack(SP_Call);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateBytesLoaded");
    popArg("return", ret);
    freeStack();
    return ret;
}

unsigned int 
AgentHelper::MediaPlayerPrivateTotalBytes(int playerID)
{
    int ret = 0;
    initStack(SP_Call);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateTotalBytes");
    popArg("return", ret);
    freeStack();
    return ret;
}

void 
AgentHelper::MediaPlayerPrivateSeek(int playerID, float time)
{
    initStack(SP_Call);
    pushArg("time", time);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateSeek");
    freeStack();
}

int 
AgentHelper::MediaPlayerPrivateSeeking(int playerID)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateSeeking");
    popArg("return", ret);
    freeStack();
    return ret;
}

void 
AgentHelper::MediaPlayerPrivateSetVolume(int playerID, float volume)
{
    initStack(SP_Call);
    pushArg("volume", volume);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateSetVolume");
    freeStack();
}

void 
AgentHelper::MediaPlayerPrivateSetMute(int playerID, int mute)
{
    initStack(SP_Call);
    pushArg("mute", mute);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateSetMute");
    freeStack();
}

void 
AgentHelper::MediaPlayerPrivateSetRate(int playerID, float rate)
{
    initStack(SP_Call);
    pushArg("rate", rate);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateSetRate");
    freeStack();
}

int 
AgentHelper::MediaPlayerPrivateSupportsType(const char* type, const char* codecs)
{
    int ret = -1;
    initStack(SP_Call);
    if (codecs)
        pushArg("codecs", codecs);
    else 
        pushArg("codecs", 0, 0);
    pushArg("type", type);
    call("MediaPlayerPrivateSupportsType");
    freeStack();
    return 0;
}

void 
AgentHelper::MediaPlayerPrivateGetSupportedTypes(char* supportedTypes)
{
    initStack(SP_Call);
    call("MediaPlayerPrivateGetSupportedTypes");
    popArg("supportedTypes", supportedTypes, IJSEBUFFSIZE);
    freeStack();
}

int 
AgentHelper::MediaPlayerPrivateNetworkState(int playerID)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateNetworkState");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
AgentHelper::MediaPlayerPrivateReadyState(int playerID)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("playerID", playerID);
    call("MediaPlayerPrivateReadyState");
    popArg("return", ret);
    freeStack();
    return ret;
}

//Server Functions
extern "C" void TAKIN_browser_getVersion(int*, char** , char** );
extern "C" void TAKIN_browser_paint(int*, cairo_t*);
extern "C" void TAKIN_browser_setSetting(TAKIN_SETTING_TYPE, char*, int);
extern "C" void TAKIN_browser_getSetting(TAKIN_SETTING_TYPE, char*, int);
extern "C" int  TAKIN_browser_createWindow(TAKIN_browser**, char*, int);
extern "C" int  TAKIN_browser_closeWindow(TAKIN_browser*);
extern "C" int  TAKIN_socket_cleanup(TAKIN_browser*);
extern "C" int  TAKIN_browser_loadURL(TAKIN_browser*, const char*);                                                            
extern "C" int  TAKIN_browser_fireEvent(TAKIN_browser*, InputEvent*);
extern "C" int  TAKIN_browser_goBack(TAKIN_browser* );
extern "C" int  TAKIN_browser_setFocusObject(TAKIN_browser*, char*);

int 
AgentHelper::TakinBrowserGetVersion(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    int svn = 0;
    char* time = NULL;
    char* builder = NULL;
    self->freeStack();
    TAKIN_browser_getVersion(&svn, &time, &builder);

    self->initStack(SP_Result);
    self->pushArg("builder", builder);
    self->pushArg("time", time);
    self->pushArg("svn", svn);
    return 0;
}

int 
AgentHelper::TakinBrowserCreateWindow(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    unsigned int address = 0;
    int width = 0, height = 0, pitch = 0;
    self->popArg("address", address);
    self->popArg("width", width);
    self->popArg("height", height);
    self->popArg("pitch", pitch);
    self->freeStack();
    ::_CreateCairo(address, width, height, pitch);

    int* tHandle = NULL;
    int ret = TAKIN_browser_createWindow((TAKIN_browser**)&tHandle, 0, 0);

    self->initStack(SP_Result);
    self->pushArg("handle", (unsigned int)tHandle);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::TakinBrowserCloseWindow(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    int handle;
    self->popArg("handle", handle);
    self->freeStack();
    int ret = TAKIN_browser_closeWindow((TAKIN_browser*)handle);
    ::_DestroyCairo();

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::TakinBrowserSocketCleanup(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    int handle;
    self->popArg("handle", handle);
    self->freeStack();
    int ret = TAKIN_socket_cleanup((TAKIN_browser*)handle);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::TakinBrowserFireEvent(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    InputEvent dataEvent;
    int handle = 0;
    int ret = -1;
    self->popArg("handle", handle);
    ret = self->popArg("dataEvent", (char*)&dataEvent, sizeof(InputEvent));
    self->freeStack();
    if (IPC::eStackStateNullValue == ret)
        ret = TAKIN_browser_fireEvent((TAKIN_browser*)handle, NULL);
    else
        ret = TAKIN_browser_fireEvent((TAKIN_browser*)handle, &dataEvent);

    if (self->mNeedUpdateScreen) {
        static int x = 0, y = 0, w = 100, h = 100;
        self->TakinPortingInvalidateWindow(&x, &y, &w, &h);
        self->mNeedUpdateScreen = 0;
    }

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::TakinBrowserSetSetting(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    int type = 0, bufferLen = 0;
    char buffer[IJSEURLLENGTH + 1] = "";
    self->popArg("type", type);
    self->popArg("buffer", buffer, IJSEURLLENGTH);
    self->popArg("bufferLen", bufferLen);
    self->freeStack();
    TAKIN_browser_setSetting((TAKIN_SETTING_TYPE)type, buffer, bufferLen);

    self->initStack(SP_Result);
    return 0;
}

int 
AgentHelper::TakinBrowserGetSetting(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    int type = 0;
    char buffer[IJSEURLLENGTH + 1] = "";
    self->popArg("type", type);
    self->freeStack();
    TAKIN_browser_getSetting((TAKIN_SETTING_TYPE)type, buffer, IJSEURLLENGTH);

    self->initStack(SP_Result);
    self->pushArg("buffer", buffer);
    return 0;
}

int 
AgentHelper::TakinBrowserLoadURL(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    unsigned int handle = 0;
    char url[IJSEURLLENGTH + 1] = "";
    self->popArg("handle", handle);
    self->popArg("url", url, IJSEURLLENGTH);
    self->freeStack();
    int ret = TAKIN_browser_loadURL((TAKIN_browser*)handle, url);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::TakinBrowserGoback(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    int handle = 0;
    self->popArg("handle", handle);
    self->freeStack();
    int ret = TAKIN_browser_goBack((TAKIN_browser*)handle);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::TakinBrowserSetFocusObject(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    int handle = 0;
    int ret = -1;
    char objectCLSID[IJSEURLLENGTH + 1] = "";
    self->popArg("handle", handle);
    ret = self->popArg("objectCLSID", objectCLSID, IJSEURLLENGTH);
    self->freeStack();
    if (IPC::eStackStateNullValue == ret)
        ret = TAKIN_browser_setFocusObject((TAKIN_browser*)handle, 0);
    else 
        ret = TAKIN_browser_setFocusObject((TAKIN_browser*)handle, objectCLSID);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::TakinBrowserPaint(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    int handle = 0;
    self->popArg("handle", handle);
    self->freeStack();

    cairo_t* cr = _CreateCairo(0, 0, 0, 0);
    if (cr)
        TAKIN_browser_paint((int*)handle, cr);

    self->initStack(SP_Result);
    self->pushArg("return", 0);
    return 0;
}

IAuthentication* 
AgentHelper::createIAuthentication(const char* name)
{
    IAuthenticationCaller* pV = new IAuthenticationCaller(name, Agent());
    if (pV) {
        int ret = -1;
        initStack(SP_Call);
        pushArg("name", name);
        call("createIAuthentication");
        popArg("return", ret);
        freeStack();
        if (!ret) {
            delete pV;
            pV = NULL;
        }
    }
    return pV;
}

IDevice* 
AgentHelper::createIDevice(const char* name)
{
    IDeviceCaller* pV = new IDeviceCaller(name, Agent());
    if (pV) {
        int ret = 0;
        initStack(SP_Call);
        pushArg("name", name);
        call("createIDevice");
        popArg("return", ret);
        freeStack();
        if (!ret) {
            delete pV;
            pV = NULL;
        }
    }
    return pV;
}

IiPanel* 
AgentHelper::createIiPanel(const char* name)
{
    IiPanelCaller* pV = new IiPanelCaller(name, Agent());
    if (pV) {
        int ret = -1;
        initStack(SP_Call);
        pushArg("name", name);
        call("createIiPanel");
        popArg("return", ret);
        freeStack();
        if (!ret) {
            delete pV;
            pV = NULL;
        }
    }
    return pV;
}

IiPanelPageWidget* 
AgentHelper::createIiPanelPageWidget(const char* name)
{
    IiPanelPageWidgetCaller* pV = new IiPanelPageWidgetCaller(name, Agent());
    if (pV) {
        int ret = -1;
        initStack(SP_Call);
        pushArg("name", name);
        call("createIiPanelPageWidget");
        popArg("return", ret);
        freeStack();
        if (!ret) {
            delete pV;
            pV = NULL;
        }
    }
    return pV;
}

IMediaPlayer* 
AgentHelper::createIMediaPlayer(const char* name)
{
    IMediaPlayerCaller* pV = new IMediaPlayerCaller(name, Agent());

    if (pV) {
        int ret;
        initStack(SP_Call);
        pushArg("name", name);
        call("createIMediaPlayer");
        popArg("return", ret);
        freeStack();
        if (!ret) {
            delete pV;
            pV = NULL;
        }
    }
    return pV;
}

IUBankDevice* 
AgentHelper::createIUBankDevice(const char* name)
{
    IUBankDeviceCaller* pV = new IUBankDeviceCaller(name, Agent());
    if (pV) {
        int ret;
        initStack(SP_Call);
        pushArg("name", name);
        call("createIUBankDevice");
        popArg("return", ret);
        freeStack();
        if (!ret) {
            delete pV;
            pV = NULL;
        }
    }
    return pV;
}

IUtility* 
AgentHelper::createIUtility(const char* name)
{
    IUtilityCaller* pV = new IUtilityCaller(name, Agent());
    if (pV) {
        int ret;
        initStack(SP_Call);
        pushArg("name", name);
        call("createIUtility");
        popArg("return", ret);
        freeStack();
        if (!ret) {
            delete pV;
            pV = NULL;
        }
    }
    return pV;
}

extern "C" 
AgentHelper* Agent()
{
    return &gAgent;
}
