#ifndef __AgentHelper_H__
#define __AgentHelper_H__

#include "AgentShell.h"

class AgentHelper : public Hippo::AgentShell {
public:
    AgentHelper();
    ~AgentHelper();

    //Caller functions
    int TakinBrowserGetVersion(int* svn, char* time, char* builder);
    int TakinBrowserCreateWindow(int** handle, unsigned int addr, int width, int height, int pitch);
    int TakinBrowserCloseWindow(int* handle);
    int TakinBrowserSocketCleanup(int* handle);
    int TakinBrowserFireEvent(int* handle, const char* dataEvent, int dataSize);
    int TakinBrowserSetSetting(int type, char *buffer, int bufferLen);
    int TakinBrowserGetSetting(int type, char *buffer, int bufferLen);
    int TakinBrowserLoadURL(int* handle, char* url);
    int TakinBrowserGoback(int* handle);
    int TakinBrowserSetFocusObject(int* handle, char* objectCLSID);
    int TakinBrowserPaint(int* handle);

    int TakinBrowserHandleJsonMessage(const char* json);

    static int createIDevice(void*);
    static int createIAuthentication(void*);
    static int createIiPanel(void*);
    static int createIiPanelPageWidget(void*);
    static int createIMediaPlayer(void*);
    static int createIUBankDevice(void*);
    static int createIUtility(void*);
private:
    //Server Functions
    static int TakinPortingLoadNotification(void* obj);
    static int TakinPortingSetFrameSize(void* obj);
    static int TakinPortingInvalidateWindow(void* obj);
    static int TakinPortingUpdateScreen(void* obj);
    static int TakinPortingExtensionStart(void* obj);
    static int TakinPortingExtensionStop(void* obj);
    static int TakinPortingGetTimeZone(void* obj);
    static int TakinPortingInputGetEvent(void* obj);
    static int TakinPortingInputSendEvent(void* obj);
    static int TakinPortingWindowGetEvent(void* obj);

    /* JVM request */
    static int JVMNotifyEvent(void* obj);

    /* MediaPlayerPrivate server */
    static int MediaPlayerPrivateInit(void*);
    static int MediaPlayerPrivateFinal(void*);
    static int MediaPlayerPrivateLoad(void*);
    static int MediaPlayerPrivateSizeChanged(void*);
    static int MediaPlayerPrivatePlay(void*);
    static int MediaPlayerPrivatePause(void*);
    static int MediaPlayerPrivateWidth(void*);
    static int MediaPlayerPrivateHeight(void*);
    static int MediaPlayerPrivateHasVideo(void*);
    static int MediaPlayerPrivateHasAudio(void*);
    static int MediaPlayerPrivateDuration(void*);
    static int MediaPlayerPrivateCurrentTime(void*);
    static int MediaPlayerPrivateBytesLoaded(void*);
    static int MediaPlayerPrivateTotalBytes(void*);
    static int MediaPlayerPrivateSeek(void*);
    static int MediaPlayerPrivateSeeking(void*);
    static int MediaPlayerPrivateSetVolume(void*);
    static int MediaPlayerPrivateSetMute(void*);
    static int MediaPlayerPrivateSetRate(void*);
    static int MediaPlayerPrivateSupportsType(void*);
    static int MediaPlayerPrivateGetSupportedTypes(void*);
    static int MediaPlayerPrivateNetworkState(void*);
    static int MediaPlayerPrivateReadyState(void*);
};

extern "C" AgentHelper* Agent();
#endif

