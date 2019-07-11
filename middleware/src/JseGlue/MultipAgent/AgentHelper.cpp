#include "AgentHelper.h"

#include "BCMediaPlayerPrivatePorting.h"

#include "IDeviceServer.h"
#include "IAuthenticationServer.h"
#include "IiPanelServer.h"
#include "IiPanelPageWidgetServer.h"
#include "IMediaPlayerServer.h"
#include "IUBankDeviceServer.h"
#include "IUtilityServer.h"

extern "C" {
#include "TAKIN_setting_type.h"
#include "TAKIN_mid_porting.h"
#include "TAKIN_browser.h"
int yos_systemcall_runSystemCMD(char *buf, int *ret);
}


static AgentHelper gAgent;

AgentHelper::AgentHelper()
{
    LogIpcTest("Static Agent Server\n");
#if !defined(DEBUG_BROWSER)
    int ret = -1;
    yos_systemcall_runSystemCMD((char*)"ipcs -m | awk '$1 ~/0xabcdef[0-9][0-9]/ {print $2}' | while read s; do ipcrm -m $s; done", &ret);
    yos_systemcall_runSystemCMD((char*)"ipcs -s | awk '$1 ~/0xabcdef[0-9][0-9]/ {print $2}' | while read s; do ipcrm -s $s; done", &ret);
    LogIpcDebug("remove ipc %d\n", ret);
#endif

    setProgramName("MidwareRun");
    initialize(1); //1 for midware 

    //add server global function
    addFunction("TakinPortingLoadNotification", &AgentHelper::TakinPortingLoadNotification);
    addFunction("TakinPortingSetFrameSize", &AgentHelper::TakinPortingSetFrameSize);
    addFunction("TakinPortingInvalidateWindow", &AgentHelper::TakinPortingInvalidateWindow);
    addFunction("TakinPortingUpdateScreen", &AgentHelper::TakinPortingUpdateScreen);
    addFunction("TakinPortingExtensionStart", &AgentHelper::TakinPortingExtensionStart);
    addFunction("TakinPortingExtensionStop", &AgentHelper::TakinPortingExtensionStop);
    addFunction("TakinPortingGetTimeZone", &AgentHelper::TakinPortingGetTimeZone);
    addFunction("TakinPortingInputGetEvent", &AgentHelper::TakinPortingInputGetEvent);
    addFunction("TakinPortingInputSendEvent", &AgentHelper::TakinPortingInputSendEvent);
    addFunction("TakinPortingWindowGetEvent", &AgentHelper::TakinPortingWindowGetEvent);

    //add jvm global function
    addFunction("JVMNotifyEvent", &AgentHelper::JVMNotifyEvent);

    //add create obj function
    addFunction("createIDevice", &AgentHelper::createIDevice);
    addFunction("createIAuthentication", &AgentHelper::createIAuthentication);
    addFunction("createIUtility", &AgentHelper::createIUtility);
    addFunction("createIMediaPlayer", &AgentHelper::createIMediaPlayer);
    addFunction("createIUBankDevice", &AgentHelper::createIUBankDevice);
    addFunction("createIiPanel", &AgentHelper::createIiPanel);
    addFunction("createIiPanelPageWidget", &AgentHelper::createIiPanelPageWidget);

    addFunction("removeServerObject", &AgentHelper::IPCServer::removeServerObject);

    addFunction("MediaPlayerPrivateInit", &AgentHelper::MediaPlayerPrivateInit);
    addFunction("MediaPlayerPrivateFinal", &AgentHelper::MediaPlayerPrivateFinal);
    addFunction("MediaPlayerPrivateLoad", &AgentHelper::MediaPlayerPrivateLoad);
    addFunction("MediaPlayerPrivateSizeChanged", &AgentHelper::MediaPlayerPrivateSizeChanged);
    addFunction("MediaPlayerPrivatePlay", &AgentHelper::MediaPlayerPrivatePlay);
    addFunction("MediaPlayerPrivatePause", &AgentHelper::MediaPlayerPrivatePause);
    addFunction("MediaPlayerPrivateWidth", &AgentHelper::MediaPlayerPrivateWidth);
    addFunction("MediaPlayerPrivateHeight", &AgentHelper::MediaPlayerPrivateHeight);
    addFunction("MediaPlayerPrivateHasVideo", &AgentHelper::MediaPlayerPrivateHasVideo);
    addFunction("MediaPlayerPrivateHasAudio", &AgentHelper::MediaPlayerPrivateHasAudio);
    addFunction("MediaPlayerPrivateDuration", &AgentHelper::MediaPlayerPrivateDuration);
    addFunction("MediaPlayerPrivateCurrentTime", &AgentHelper::MediaPlayerPrivateCurrentTime);
    addFunction("MediaPlayerPrivateBytesLoaded", &AgentHelper::MediaPlayerPrivateBytesLoaded);
    addFunction("MediaPlayerPrivateTotalBytes", &AgentHelper::MediaPlayerPrivateTotalBytes);
    addFunction("MediaPlayerPrivateSeek", &AgentHelper::MediaPlayerPrivateSeek);
    addFunction("MediaPlayerPrivateSeeking", &AgentHelper::MediaPlayerPrivateSeeking);
    addFunction("MediaPlayerPrivateSetVolume", &AgentHelper::MediaPlayerPrivateSetVolume);
    addFunction("MediaPlayerPrivateSetMute", &AgentHelper::MediaPlayerPrivateSetMute);
    addFunction("MediaPlayerPrivateSetRate", &AgentHelper::MediaPlayerPrivateSetRate);
    addFunction("MediaPlayerPrivateSupportsType", &AgentHelper::MediaPlayerPrivateSupportsType);
    addFunction("MediaPlayerPrivateReadyState", &AgentHelper::MediaPlayerPrivateReadyState);
}

AgentHelper::~AgentHelper()
{
}

//Caller Functions
int  
AgentHelper::TakinBrowserGetVersion(int* svn, char* time, char* builder)
{
    initStack(SP_Call);
    int tSvn = 0;
    call("TakinBrowserGetVersion");
    popArg("svn", tSvn);
    popArg("time", time, 64);
    popArg("builder", builder, 64);
    freeStack();
    *svn = tSvn; 
    return 0;
}

int 
AgentHelper::TakinBrowserCreateWindow(int** handle, unsigned int address, int width, int height, int pitch)
{
    int ret = 0;
    int tHandle = 0;
    initStack(SP_Call);
    pushArg("pitch", (int)pitch);
    pushArg("height", (int)height);
    pushArg("width", (int)width);
    pushArg("address", (int)address);
    call("TakinBrowserCreateWindow");
    popArg("return", ret);
    popArg("handle", tHandle);
    freeStack();
    *handle = (int*)tHandle;
    return ret;
}

int 
AgentHelper::TakinBrowserCloseWindow(int* handle)
{ 
    int ret;
    initStack(SP_Call);
    pushArg("handle", (int)handle);
    call("TakinBrowserCloseWindow");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
AgentHelper::TakinBrowserSocketCleanup(int* handle)
{ 
    int ret;
    initStack(SP_Call);
    pushArg("handle", (int)handle);
    call("TakinBrowserSocketCleanup");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
AgentHelper::TakinBrowserFireEvent(int* handle, const char* dataEvent, int dataSize)
{
    int ret = -1;
    initStack(SP_Call);
    if (dataEvent)
        pushArg("dataEvent", dataEvent, dataSize);
    else 
        pushArg("dataEvent", NULL, 0);
    pushArg("handle", (int)handle);
    call("TakinBrowserFireEvent");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
AgentHelper::TakinBrowserSetSetting(int type, char* buffer, int bufferLen)
{
    initStack(SP_Call);
    pushArg("bufferLen", bufferLen);
    if (buffer)
        pushArg("buffer", buffer);
    else 
        pushArg("buffer", 0, 0);
    pushArg("type", type);
    call("TakinBrowserSetSetting");
    freeStack();
    return 0;
}

int 
AgentHelper::TakinBrowserGetSetting(int type, char* buffer, int bufferLen)
{
    initStack(SP_Call);
    pushArg("type", type);
    call("TakinBrowserGetSetting");
    popArg("buffer", buffer, bufferLen);
    freeStack();
    return 0;
}

int 
AgentHelper::TakinBrowserLoadURL(int* handle, char* url)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("url", url);
    pushArg("handle", (int)handle);
    call("TakinBrowserLoadURL");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
AgentHelper::TakinBrowserGoback(int* handle)
{
    initStack(SP_Call);
    int ret = -1;
    pushArg("handle", (int)handle);
    call("TakinBrowserGoback");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
AgentHelper::TakinBrowserSetFocusObject(int* handle, char* objectCLSID)
{
    int ret = -1;
    initStack(SP_Call);
    if (objectCLSID)
        pushArg("objectCLSID", objectCLSID);
    else 
        pushArg("objectCLSID", NULL, 0);
    pushArg("handle", (int)handle);
    call("TakinBrowserSetFocusObject");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
AgentHelper::TakinBrowserPaint(int* handle)
{
    int ret = -1;
    initStack(SP_Call);
    pushArg("handle", (unsigned int)handle);
    call("TakinBrowserPaint");
    popArg("return", ret);
    freeStack();
    return ret;
}

int 
AgentHelper::TakinBrowserHandleJsonMessage(const char* jsonstr)
{
    if (!jsonstr)
        return -1;
    initStack(SP_Call);
    pushArg("jsonstr", jsonstr); 
    call("TakinBrowserHandleJsonMessage");
    freeStack();
    return 0;
}


//Server Functions
extern int TAKIN_porting_load_notification(TAKIN_SETTING_TYPE name, char *buffer, int buffer_len,int handler);

int 
AgentHelper::TakinPortingLoadNotification(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    int stateCode = 0, ret = -1; 
    char buffer[IJSEURLLENGTH + 1] = "";
    self->popArg("stateCode", stateCode);
    self->popArg("buffer", buffer, IJSEURLLENGTH);
    self->freeStack();
    ret = TAKIN_porting_load_notification((TAKIN_SETTING_TYPE)stateCode, buffer, IJSEURLLENGTH,0);

    self->initStack(SP_Result);
    if (TAKIN_LOADER_ERROR == stateCode)
        self->pushArg("buffer", buffer);
    self->pushArg("return", ret);
    return 0;
}

int 
AgentHelper::TakinPortingSetFrameSize(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    int type = 0, bufferLen = 1024, ret = 0; 
    char buffer[IJSEURLLENGTH + 1] = "";
    self->popArg("type", type);
    self->popArg("buffer", buffer, IJSEURLLENGTH);
    self->popArg("bufferLen", bufferLen);
    self->freeStack();
    TAKIN_porting_set_frame_size((TAKIN_SETTING_TYPE)type, buffer, bufferLen);

    self->initStack(SP_Result);
    return 0;
}

int 
AgentHelper::TakinPortingInvalidateWindow(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    int x = 0, y = 0, w = 0, h = 0;
    self->popArg("x", x);
    self->popArg("y", y);
    self->popArg("w", w);
    self->popArg("h", h);
    self->freeStack();

    int ret = TAKIN_porting_invalidateWindow(&x, &y, &w, &h);

    self->initStack(SP_Result);
    self->pushArg("h", h);
    self->pushArg("w", w);
    self->pushArg("y", y);
    self->pushArg("x", x);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::TakinPortingUpdateScreen(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    self->freeStack();
    int ret = TAKIN_browser_updateScreen(0);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::TakinPortingExtensionStart(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    std::string url;
    self->popArg("url", url);
    self->freeStack();
    TAKIN_browser_Extension_start(url.c_str());

    self->initStack(SP_Result);
    return 0;
}

int 
AgentHelper::TakinPortingExtensionStop(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;
    std::string url;
    self->popArg("url", url);
    self->freeStack();
    TAKIN_browser_Extension_stop(url.c_str());

    self->initStack(SP_Result);
    return 0;
}

int 
AgentHelper::TakinPortingGetTimeZone(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    self->freeStack();
    int ret = TAKIN_browser_getTimezone();

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::TakinPortingInputGetEvent(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    int timeout = 0;
    InputEvent event;
    self->popArg("timeout", timeout);
    self->freeStack();
    int ret = yhw_input_getEvent(&event, timeout);

    self->initStack(SP_Result);
    self->pushArg("event", (char*)&event, sizeof(InputEvent));
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::TakinPortingInputSendEvent(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    InputEvent event;
    self->popArg("event", (void*)&event, sizeof(InputEvent));
    self->freeStack();
    int ret = yhw_input_sendEvent(event);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::TakinPortingWindowGetEvent(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    int timeout = 0;
    InputEvent event;
    self->popArg("timeout", timeout);
    self->freeStack();
    int ret = TAKIN_browser_getEvent(&event, timeout);

    self->initStack(SP_Result);
    self->pushArg("event", (char*)&event, sizeof(InputEvent));
    self->pushArg("return", ret);
    return ret;                                
}

/* JVM request */

extern "C" void JVMNotifyEvent(int eventType, int eventValue);

int 
AgentHelper::JVMNotifyEvent(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;

    int eventType, eventValue;

    self->popArg("eventValue", eventValue);
    self->popArg("eventType", eventType);
    self->freeStack();

    int ret = 0;

    ::JVMNotifyEvent(eventType, eventValue);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}


int 
AgentHelper::MediaPlayerPrivateInit(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    self->freeStack();
    int ret = ::MediaPlayerPrivateInit();

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}
    
int 
AgentHelper::MediaPlayerPrivateFinal(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;
    
    int playerID = 0;
    self->popArg("playerID", playerID);
    self->freeStack();
    ::MediaPlayerPrivateFinal(playerID);

    self->initStack(SP_Result);
    return 0;
}

int  
AgentHelper::MediaPlayerPrivateLoad(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;
    
    int playerID = 0;
    char url[IJSEBUFFSIZE + 1] = "";
    self->popArg("playerID", playerID);
    self->popArg("url", url, IJSEBUFFSIZE);
    self->freeStack();
    ::MediaPlayerPrivateLoad(playerID, url);

    self->initStack(SP_Result);
    return 0;
}

int 
AgentHelper::MediaPlayerPrivateSizeChanged(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int x = 0, y = 0, w = 0, h = 0;
    int playerID = 0;
    self->popArg("playerID", playerID);
    self->popArg("x", x);
    self->popArg("y", y);
    self->popArg("w", w);
    self->popArg("h", h);
    self->freeStack();
    ::MediaPlayerPrivateSizeChanged(playerID, x, y, w, h);

    self->initStack(SP_Result);
    return 0;
}

int 
AgentHelper::MediaPlayerPrivatePlay(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    self->popArg("playerID", playerID);
    self->freeStack();
    ::MediaPlayerPrivatePlay(playerID);

    self->initStack(SP_Result);
    return 0;
}

int 
AgentHelper::MediaPlayerPrivatePause(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    self->popArg("playerID", playerID);
    self->freeStack();
    ::MediaPlayerPrivatePause(playerID);

    self->initStack(SP_Result);
    return  0;
}

int 
AgentHelper::MediaPlayerPrivateWidth(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    self->popArg("playerID", playerID);
    self->freeStack();
    int ret = ::MediaPlayerPrivateWidth(playerID);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::MediaPlayerPrivateHeight(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    self->popArg("playerID", playerID);
    self->freeStack();
    int ret = ::MediaPlayerPrivateHeight(playerID);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::MediaPlayerPrivateHasVideo(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    self->popArg("playerID", playerID);
    self->freeStack();
    int ret = 0; //TOOD ::MediaPlayerPrivateHasVideo(playerID);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::MediaPlayerPrivateHasAudio(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    self->popArg("playerID", playerID);
    self->freeStack();
    int ret = ::MediaPlayerPrivateHasAudio(playerID);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::MediaPlayerPrivateDuration(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    self->popArg("playerID", playerID);
    self->freeStack();
    float ret = ::MediaPlayerPrivateDuration(playerID);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int
AgentHelper::MediaPlayerPrivateCurrentTime(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    self->popArg("playerID", playerID);
    self->freeStack();
    float ret = ::MediaPlayerPrivateCurrentTime(playerID);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int
AgentHelper::MediaPlayerPrivateBytesLoaded(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    self->popArg("playerID", playerID);
    self->freeStack();
    int ret = ::MediaPlayerPrivateBytesLoaded(playerID);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::MediaPlayerPrivateTotalBytes(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    self->popArg("playerID", playerID);
    self->freeStack();
    int ret = ::MediaPlayerPrivateTotalBytes(playerID);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::MediaPlayerPrivateSeek(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    float time = 0.0;
    self->popArg("playerID", playerID);
    self->popArg("time", time);
    self->freeStack();
    ::MediaPlayerPrivateSeek(playerID, time);

    self->initStack(SP_Result);
    return 0;
}

int 
AgentHelper::MediaPlayerPrivateSeeking(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    self->popArg("playerID", playerID);
    self->freeStack();
    int ret = ::MediaPlayerPrivateSeeking(playerID);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int  
AgentHelper::MediaPlayerPrivateSetVolume(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    float volume = 0.0;
    self->popArg("playerID", playerID);
    self->popArg("volume", volume);
    self->freeStack();

    ::MediaPlayerPrivateSetVolume(playerID, volume);

    self->initStack(SP_Result);
    return 0;
}

int  
AgentHelper::MediaPlayerPrivateSetMute(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    int mute;
    self->popArg("playerID", playerID);
    self->popArg("mute", mute);
    self->freeStack();
    ::MediaPlayerPrivateSetMute(playerID, mute);

    self->initStack(SP_Result);
    return 0;
}

int  
AgentHelper::MediaPlayerPrivateSetRate(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    float rate = 0.0;
    self->popArg("playerID", playerID);
    self->popArg("rate", rate);
    self->freeStack();
    ::MediaPlayerPrivateSetRate(playerID, rate);

    self->initStack(SP_Result);
    return 0;
}

int 
AgentHelper::MediaPlayerPrivateSupportsType(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    std::string type;
    std::string codecs;
    self->popArg("type", type);
    self->popArg("codecs", codecs);
    self->freeStack();
    int ret = ::MediaPlayerPrivateSupportsType(type.c_str(), codecs.c_str());

    self->initStack(SP_Result);
    return ret;
}

int  
AgentHelper::MediaPlayerPrivateGetSupportedTypes(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    self->freeStack();
    char supportedTypes[IJSEBUFFSIZE + 1] = "";
    ::MediaPlayerPrivateGetSupportedTypes(supportedTypes);
    supportedTypes[IJSEBUFFSIZE] = '\0';

    self->initStack(SP_Result);
    self->pushArg("supportedTypes", supportedTypes);
}

int 
AgentHelper::MediaPlayerPrivateNetworkState(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    self->popArg("playerID", playerID);
    self->freeStack();
    int ret = ::MediaPlayerPrivateNetworkState(playerID);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::MediaPlayerPrivateReadyState(void* obj)
{
    AgentHelper* self = (AgentHelper*) obj;

    int playerID = 0;
    self->popArg("playerID", playerID);
    self->freeStack();
    int ret = ::MediaPlayerPrivateReadyState(playerID);

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

/* Create Obj */
int 
AgentHelper::createIAuthentication(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj; 
    IIPCServer* pServer = NULL;

    int ret = 0;
    std::string name;
    self->popArg("name", name);
    self->freeStack();
    pServer = (IIPCServer*)::IcreateIAuthentication(name.c_str());
    if (pServer) {
        if (!pServer->attachServer()) {
            if (!self->addObject(name.c_str(), pServer))
                ret = 1;
        }
        if (!ret)
            delete pServer;
    }

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::createIDevice(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;
    IIPCServer* pServer = NULL;

    int ret = 0;
    std::string name;
    self->popArg("name", name);
    self->freeStack();
    pServer = (IIPCServer*)::IcreateIDevice(name.c_str());
    if (pServer) {
        if (!pServer->attachServer()) {
            if (!self->addObject(name.c_str(), pServer))
                ret = 1;
        }
        if (!ret)
            delete pServer;
    }

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::createIiPanel(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;
    IIPCServer* pServer = NULL;

    int ret = 0;
    std::string name;
    self->popArg("name", name);
    self->freeStack();
    pServer = (IIPCServer*)::IcreateIiPanel(name.c_str());
    if (pServer) {
        if (!pServer->attachServer()) {
            if (!self->addObject(name.c_str(), pServer))
                ret = 1;
        }
        if (!ret)
            delete pServer;
    }

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::createIiPanelPageWidget(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj;
    IIPCServer* pServer = NULL;

    int ret = 0;
    std::string name;
    self->popArg("name", name);
    self->freeStack();
    pServer = (IIPCServer*)::IcreateIiPanelPageWidget(name.c_str());
    if (pServer) {
        if (!pServer->attachServer()) {
            if (!self->addObject(name.c_str(), pServer))
                ret = 1;
        }
        if (!ret)
            delete pServer;
    }

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::createIMediaPlayer(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj; 
    IIPCServer* pServer = NULL;

    int ret = 0;
    std::string name;
    self->popArg("name", name);
    self->freeStack();
    pServer = (IIPCServer*)::IcreateIMediaPlayer(name.c_str());
    if (pServer) {
        if (!pServer->attachServer()) {
            if (!self->addObject(name.c_str(), pServer))
                ret = 1;
        }
        if (!ret)
            delete pServer;
    }

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::createIUBankDevice(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj; 
    IIPCServer* pServer = NULL;

    int ret = 0;
    std::string name;
    self->popArg("name", name);
    self->freeStack();
    pServer = (IIPCServer*)::IcreateIUBankDevice(name.c_str());
    if (pServer) {
        if (!pServer->attachServer()) {
            if (!self->addObject(name.c_str(), pServer))
                ret = 1;
        }
        if (!ret)
            delete pServer;
    }

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}

int 
AgentHelper::createIUtility(void* obj)
{
    AgentHelper* self = (AgentHelper*)obj; 
    IIPCServer* pServer = NULL;

    int ret = 0;
    std::string name;
    self->popArg("name", name);
    self->freeStack();
    pServer = (IIPCServer*)::IcreateIUtility(name.c_str());
    if (pServer) {
        if (!pServer->attachServer()) {
            if (!self->addObject(name.c_str(), pServer))
                ret = 1;
        }
        if (!ret)
            delete pServer;
    }

    self->initStack(SP_Result);
    self->pushArg("return", ret);
    return ret;
}


extern "C" 
AgentHelper* Agent()
{
    return &gAgent;
}
