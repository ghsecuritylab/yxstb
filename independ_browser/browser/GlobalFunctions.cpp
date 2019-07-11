#include "AgentHelper.h"
#include <sys/mman.h>
#include <fcntl.h>

#include "libzebra.h"

extern "C" {

extern void TAKIN_browser_setSetting(int type, char *buffer, int bufferLen);

//Caller Global Functions
int TAKIN_porting_load_notification(int stateCode, char *buffer, int bufferLen, int handle) 
{ 
    int ret = Agent()->TakinPortingLoadNotification(stateCode, buffer, bufferLen);
    return ret;
}

void TAKIN_porting_set_frame_size(int type, char *buffer, int bufferLen)
{
    Agent()->TakinPortingSetFrameSize(type, buffer, bufferLen);
    TAKIN_browser_setSetting(type, buffer, bufferLen);
}

int TAKIN_porting_invalidateWindow(int* x, int* y, int* w, int* h)
{
    Agent()->setUpdateScreenFlag(1);
    return 0; //only record state
    //int ret = Agent()->TakinPortingInvalidateWindow(x, y, w, h);
    //return ret;
}

int TAKIN_browser_updateScreen(int handle)
{
    return 0; //not used
    //int ret = Agent()->TakinPortingUpdateScreen();
    //return ret;
}

void TAKIN_browser_Extension_start(const char* url)
{
    Agent()->TakinPortingExtensionStart(url);
}

void TAKIN_browser_Extension_stop(const char* url)
{
    Agent()->TakinPortingExtensionStop(url);
}

int TAKIN_browser_getTimezone(void)
{
    return 0; //not used
    //int ret = Agent()->TakinPortingGetTimeZone();
    //return ret;
}

int TAKIN_porting_inputGetEvent(InputEvent* event, int timeout)
{
    int ret = Agent()->TakinPortingInputGetEvent((char*)event, sizeof(InputEvent), timeout);
    return ret;
}

int TAKIN_porting_inputSendEvent(InputEvent event)
{
    int ret = Agent()->TakinPortingInputSendEvent((char*)&event, sizeof(InputEvent));
    return ret;
}

int TAKIN_browser_getEvent(InputEvent* event, int timeout) 
{ 
    int ret = Agent()->TakinPortingWindowGetEvent((char*)event, sizeof(InputEvent), timeout);
    return ret;
}

char* TAKIN_browser_checkFrameData(char* url, char* data, int* len)
{
    return NULL;
}

void TAKIN_browser_setFocusByJS(const char *name)
{
}

int TAKIN_jse_invoke(const char *name, const char *param, char *buffer, int bufferLen)
{
    return -1;
}

/* JVM */
void JVMNotifyEventBrowserThread(int eventType, int eventValue)
{
    Agent()->JVMNotifyEvent(eventType, eventValue);
}

}

/* CPP MediaPlayerPrivate */
int MediaPlayerPrivateInit(void)
{
    int ret = Agent()->MediaPlayerPrivateInit();
    return ret;
}

void MediaPlayerPrivateFinal(int playerID)
{
    Agent()->MediaPlayerPrivateFinal(playerID);
}

void MediaPlayerPrivateLoad(int playerID, char* url)
{
    Agent()->MediaPlayerPrivateLoad(playerID, url);
}

void MediaPlayerPrivateSizeChanged(int playerID, int x, int y, int w, int h)
{
    Agent()->MediaPlayerPrivateSizeChanged(playerID, x, y, w, h);
}

void MediaPlayerPrivatePlay(int playerID)
{
    Agent()->MediaPlayerPrivatePlay(playerID);
}

void MediaPlayerPrivatePause(int playerID)
{
    Agent()->MediaPlayerPrivatePause(playerID);
}

int MediaPlayerPrivateWidth(int playerID)
{
    int ret = Agent()->MediaPlayerPrivateWidth(playerID);
    return ret;
}

int MediaPlayerPrivateHeight(int playerID)
{
    int ret = Agent()->MediaPlayerPrivateHeight(playerID);
    return ret;
}

int MediaPlayerPrivateHasVideo(int playerID)
{
    int ret = Agent()->MediaPlayerPrivateHasVideo(playerID);
    return ret;
}

int MediaPlayerPrivateHasAudio(int playerID)
{
    int ret = Agent()->MediaPlayerPrivateHasAudio(playerID);
    return ret;
}

float MediaPlayerPrivateDuration(int playerID)
{
    float ret = Agent()->MediaPlayerPrivateDuration(playerID);
    return ret;
}

float MediaPlayerPrivateCurrentTime(int playerID)
{
    float ret = Agent()->MediaPlayerPrivateCurrentTime(playerID);
    return ret;
}

unsigned int MediaPlayerPrivateBytesLoaded(int playerID)
{
    unsigned int ret = Agent()->MediaPlayerPrivateBytesLoaded(playerID);
    return ret;
}

unsigned int MediaPlayerPrivateTotalBytes(int playerID)
{
    unsigned int ret = Agent()->MediaPlayerPrivateTotalBytes(playerID);
    return ret;
}

void MediaPlayerPrivateSeek(int playerID, float time)
{
    Agent()->MediaPlayerPrivateSeek(playerID, time);
}

int MediaPlayerPrivateSeeking(int playerID)
{
    int ret = Agent()->MediaPlayerPrivateSeeking(playerID);
    return ret;
}

void MediaPlayerPrivateSetVolume(int playerID, float volume)
{
    Agent()->MediaPlayerPrivateSetVolume(playerID, volume);
}

void MediaPlayerPrivateSetMute(int playerID, int mute)
{
    Agent()->MediaPlayerPrivateSetMute(playerID, mute);
}

void MediaPlayerPrivateSetRate(int playerID, float rate)
{
    Agent()->MediaPlayerPrivateSetRate(playerID, rate);
}

int MediaPlayerPrivateSupportsType(const char* type, const char* codecs)
{
    int ret = Agent()->MediaPlayerPrivateSupportsType(type, codecs);
    return ret;
}

void MediaPlayerPrivateGetSupportedTypes(char* supportedTypes)
{
    Agent()->MediaPlayerPrivateGetSupportedTypes(supportedTypes);
}

int MediaPlayerPrivateNetworkState(int playerID)
{
    int ret = Agent()->MediaPlayerPrivateNetworkState(playerID);
    return ret;
}

int MediaPlayerPrivateReadyState(int playerID)
{
    int ret = Agent()->MediaPlayerPrivateReadyState(playerID);
    return ret;
}
