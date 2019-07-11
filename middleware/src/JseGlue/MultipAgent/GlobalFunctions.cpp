#include "AgentHelper.h"
#include <sys/mman.h>
#include <fcntl.h>

extern "C" {
#include "TAKIN_setting_type.h"
#include "libzebra.h"
#include "cairo/cairo.h"
}

extern "C" {
//Caller Global Functions
void TAKIN_browser_getVersion(int*svn, char** time, char** builder)
{
    LogIpcDebug("===TAKIN_browser_getVersion\n");
    static int tBrowserVersion = 0;
    static char tBrowserBuildTime[64] = { 0 };
    static char tBrowserBuilder[64] = { 0 };
    if (tBrowserVersion) {
        *svn = tBrowserVersion;
        *time = tBrowserBuildTime;
        *builder = tBrowserBuilder;
        return ;
    }
    Agent()->TakinBrowserGetVersion(&tBrowserVersion, tBrowserBuildTime, tBrowserBuilder);
}

int TAKIN_browser_createWindow(int** handle, char* url, int surface)
{
    int address = 0, width = 0, height = 0, pitch = 0;
    ygp_layer_getCreateSize(surface, &width, &height);
#if defined(BROWSER_ROOT_RUN)
    ygp_layer_getDeviceMemory(surface, &address, &pitch);
#else
    ygp_layer_getMemory(surface, &address, &pitch);
#endif

    int ret = Agent()->TakinBrowserCreateWindow(handle, (unsigned int)address, width, height, pitch);
    return ret;
}

int TAKIN_browser_closeWindow(int* handle)
{
    int ret = Agent()->TakinBrowserCloseWindow(handle);
    return ret;
}

int TAKIN_socket_cleanup(int* handle)
{
    int ret = Agent()->TakinBrowserSocketCleanup(handle);
    return ret;
}

int TAKIN_browser_fireEvent(int* handle, InputEvent *event)
{
    int ret = Agent()->TakinBrowserFireEvent(handle, (const char*)event, sizeof(InputEvent));
    return ret;
}

void TAKIN_browser_setSetting(TAKIN_SETTING_TYPE type, char* buffer, int bufferLen)
{
    Agent()->TakinBrowserSetSetting((int)type, buffer, bufferLen);
}

void TAKIN_browser_getSetting(TAKIN_SETTING_TYPE type, char* buffer, int bufferLen)
{
    Agent()->TakinBrowserGetSetting((int)type, buffer, bufferLen);
}

int TAKIN_browser_loadURL(int* handle, char* url)
{
    int ret = Agent()->TakinBrowserLoadURL(handle, url);
    return ret;
}

int TAKIN_browser_goBack(int* handle)
{
    int ret = Agent()->TakinBrowserGoback(handle);
    return ret;
}

int TAKIN_browser_setFocusObject(int* handle, char* objectCLSID)
{
    int ret = Agent()->TakinBrowserSetFocusObject(handle, objectCLSID);
    return ret;
}

void TAKIN_browser_paint(int* handle, cairo_t* cr)
{
    Agent()->TakinBrowserPaint(handle);
}

void TAKIN_browser_cleanCache()
{
    return;
}

void TAKIN_browser_handleJsonMessage(const char* msg)
{
    Agent()->TakinBrowserHandleJsonMessage(msg);
}

void TAKIN_browser_removeAllCookies(int* handle)
{
    return;
}

}

