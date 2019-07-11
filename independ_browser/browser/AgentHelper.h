#ifndef __AgentHelper_H__
#define __AgentHelper_H__

#include "AgentShell.h"
#include <pthread.h>
#include <iostream>
#include <map>

class IiPanel;
class IiPanelPageWidget;
class IAuthentication;
class IDevice;
class IMediaPlayer;
class IUBankDevice;
class IUtility;

typedef void (*HandleMessageCallback)(int what, int arg1, int arg2);

class AgentHelper : public Hippo::AgentShell {
public:
    AgentHelper();
    ~AgentHelper();

    //Caller Funtions
    int TakinPortingLoadNotification(int stateCode, char *buffer, int bufferLen);
    int TakinPortingSetFrameSize(int type, char *buffer, int bufferLen);
    int TakinPortingInvalidateWindow(int* x, int* y, int* w, int* h);
    int TakinPortingUpdateScreen();
    int TakinPortingExtensionStart(const char* url);
    int TakinPortingExtensionStop(const char* url);
    int TakinPortingGetTimeZone();
    int TakinPortingInputGetEvent(char* event, int size, int timeout);
    int TakinPortingInputSendEvent(char* event, int size);
    int TakinPortingWindowGetEvent(char* event, int size, int timeout);

    //JVM
    void JVMNotifyEvent(int eventType, int eventValue);

    //MediaPlayerPrivate
    int MediaPlayerPrivateInit(void);
    void MediaPlayerPrivateFinal(int playerID);
    void MediaPlayerPrivateLoad(int playerID, char* url);
    void MediaPlayerPrivateSizeChanged(int playerID, int x, int y, int w, int h);
    void MediaPlayerPrivatePlay(int playerID);
    void MediaPlayerPrivatePause(int playerID);
    int MediaPlayerPrivateWidth(int playerID);
    int MediaPlayerPrivateHeight(int playerID);
    int MediaPlayerPrivateHasVideo(int playerID);
    int MediaPlayerPrivateHasAudio(int playerID);
    float MediaPlayerPrivateDuration(int playerID);
    float MediaPlayerPrivateCurrentTime(int playerID);
    unsigned int MediaPlayerPrivateBytesLoaded(int playerID);
    unsigned int MediaPlayerPrivateTotalBytes(int playerID);
    void MediaPlayerPrivateSeek(int playerID, float time);
    int MediaPlayerPrivateSeeking(int playerID);
    void MediaPlayerPrivateSetVolume(int playerID, float volume);
    void MediaPlayerPrivateSetMute(int playerID, int mute);
    void MediaPlayerPrivateSetRate(int playerID, float rate);
    int MediaPlayerPrivateSupportsType(const char* type, const char* codecs);
    void MediaPlayerPrivateGetSupportedTypes(char* supportedTypes);
    int MediaPlayerPrivateNetworkState(int playerID);
    int MediaPlayerPrivateReadyState(int playerID);

    //Create Caller Object
    IiPanel* createIiPanel(const char* name);
    IiPanelPageWidget* createIiPanelPageWidget(const char* name);
    IAuthentication* createIAuthentication(const char*);
    IDevice* createIDevice(const char*);
    IMediaPlayer* createIMediaPlayer(const char*);
    IUBankDevice* createIUBankDevice(const char*);
    IUtility* createIUtility(const char*);

    void setUpdateScreenFlag(int flg) { mNeedUpdateScreen = flg; }
    int mNeedUpdateScreen;
private:
    //Server Functions
    static int TakinBrowserGetVersion(void* obj);
    static int TakinBrowserCreateWindow(void* obj);
    static int TakinBrowserCloseWindow(void* obj);
    static int TakinBrowserSocketCleanup(void* obj);
    static int TakinBrowserFireEvent(void* obj);
    static int TakinBrowserSetSetting(void* obj);
    static int TakinBrowserGetSetting(void* obj);
    static int TakinBrowserLoadURL(void* obj);
    static int TakinBrowserGoback(void* obj);
    static int TakinBrowserSetFocusObject(void* obj);
    static int TakinBrowserPaint(void* obj);
};

extern "C" AgentHelper* Agent();
#endif
