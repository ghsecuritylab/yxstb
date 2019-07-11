#ifndef _TAKIN_browser_H
#define _TAKIN_browser_H


#include "libzebra.h"

#define YX_INPUTEVENT InputEvent

#define TAKIN_ZORDER_MIN	100
#define TAKIN_ZORDER_BOTTOM	105
#define TAKIN_ZORDER_TOP	115
#define TAKIN_ZORDER_MAX	120
#define TAKIN_ZORDER_IME	249
#define TAKIN_ZORDER_CURSOR	250

enum {
	TAKIN_IME_CLOSE = -1,
	TAKIN_IME_ENGLISH = 0,
	TAKIN_IME_QUANPIN,
	TAKIN_IME_T9ABC,
	TAKIN_IME_T9QUANPIN,
	TAKIN_IME_LAST
};

enum TAKIN_ObjectType {
    TAKIN_ObjectBufferLow = -1,
    TAKIN_ObjectTypeNone = 0,
    TAKIN_ObjectTypeInt,
    TAKIN_ObjectTypeString,
    TAKIN_ObjectTypeObjectJSON,
    TAKIN_ObjectTypeObjectCustom,
    TAKIN_ObjectTypeFunctionAG,
    TAKIN_ObjectTypeFunctionCA,
    TAKIN_ObjectTypeFunctionSG,
};

typedef struct ObjectValue{
    int number;
    const char *string;
} ObjectValue;

typedef int(*TAKIN_FunctionAG)(void*,const char*,const char *,char*,const int);
typedef struct FunctionAG{
    TAKIN_FunctionAG func;
    const char *fmt;
} FunctionAG;

typedef int(*TAKIN_FunctionCA)(void*,const char**,int*,char*,const int);
typedef struct FunctionCA{
    TAKIN_FunctionCA func;
    int none;
} FunctionCA;

typedef int(*TAKIN_FunctionSGER)(void*,char*,const int);
typedef struct FunctionSG{
    TAKIN_FunctionSGER setter;
    TAKIN_FunctionSGER getter;
} FunctionSG;

typedef struct TAKIN_CustomObject
{
    const char* key; // property name
    unsigned char funType;
    unsigned char argc;
    union{
        ObjectValue value;
        FunctionAG ag;
        FunctionCA ca;
        FunctionSG sg;
    }func;
} TAKIN_CustomObject;

typedef struct TAKIN_StaticGlobal
{
    unsigned char type;
    const char *identifier;
    union {
        int number;
        const char *string;
        TAKIN_CustomObject *object;
        TAKIN_FunctionCA function;
    }value;
} TAKIN_StaticGlobal;

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#include "TAKIN_setting_type.h"
#if !defined(BROWSER_INDEPENDENCE)
extern int TAKIN_browser_createWindow(int **handle, char *url, int surface);
extern int TAKIN_browser_closeWindow(int *handle);
extern int TAKIN_socket_cleanup(int *handle);
extern int TAKIN_browser_loadURL(int *handle, char* url);
extern int TAKIN_browser_fireEvent(int *handle, YX_INPUTEVENT *event);
extern int TAKIN_browser_goBack(int *handle);
extern int TAKIN_browser_setFocusObject(int *handle, char *objectCLSID);
extern int *TAKIN_browser_getInlineHandle();
extern void TAKIN_browser_setSetting(TAKIN_SETTING_TYPE name, char *buffer, unsigned int buffer_len);
extern void TAKIN_browser_getSetting(TAKIN_SETTING_TYPE name, char *buffer, unsigned int buffer_len);
extern void TAKIN_browser_getVersion(int *svn, char **time, char **builder);
#else
#endif

extern int TAKIN_browser_createKeyboard(int *handle);
extern int TAKIN_browser_hitKeyboard(int handle, YX_INPUTEVENT *event);
extern int TAKIN_browser_closeKeyboard(int handle);
extern int TAKIN_browser_editingEnabled(int *handle, int *x, int *y, int *w, int *h);
extern int TAKIN_porting_invalidateWindow(int* x, int* y, int* w, int* h);
extern int TAKIN_browser_getSurface(int *surface);
extern int TAKIN_browser_loadHTMLString(int *handle, char* htmlstring, char* baseurl);
extern int TAKIN_browser_dumpCacheState(int *handle);
extern int TAKIN_browser_reLoad(int *handle);
extern int TAKIN_browser_goForward(int *handle);
extern int TAKIN_jse_invoke(const char *name, const char *param, char *buffer, int buffer_len);
extern int TAKIN_browser_getKeyboardMode(int handle, int *mode, int *widht, int *height);
extern int TAKIN_browser_setKeyboardMode(int handle, int mode);
extern int TAKIN_browser_showKeyboard(int handle, int x, int y, int zorder );
extern int TAKIN_browser_deleteMode(int handle, int mode);
extern int TAKIN_browser_addMode(int handle, int mode);
extern int* TAKIN_browser_getKeyboardModeStatus(int handle, int *length);

extern void TAKIN_browser_setFocusByJS(const char *name);
extern int TAKIN_browser_getEvent(YX_INPUTEVENT *event, int time_out);

extern int TAKIN_browser_setStaticGlobals(int *handle, TAKIN_StaticGlobal* globals);
extern int TAKIN_browser_executeScript(int *handle,const char* script);

extern int TAKIN_browser_createWidgetFrame(int *handle, const char *name, const char* url, int x, int y, int w, int h);
extern int TAKIN_browser_setFocusWidgetByName(int *handle, const char *name);
extern int TAKIN_browser_sendEventToFocusWidget(int *handle, YX_INPUTEVENT *zebraEvent);
extern int TAKIN_browser_killAllWidgetsFocus(int *handle);
extern int TAKIN_browser_sendKeyEventToWidgetByName(int *handle, const char *name, YX_INPUTEVENT *zebraEvent);
extern int TAKIN_browser_destoryWidgetFrame(int *handle, const char* name);

extern void TAKIN_browser_cleanCache();
  /* Ends C function definitions when using C++ */
void TAKIN_Proc_Key(void *handle, int msg, unsigned int p1, unsigned int p2);
void Takin_Set_EdsFlag(int flag);

#ifdef __cplusplus
}
#endif



#endif /* _TAKIN_browser_H */
