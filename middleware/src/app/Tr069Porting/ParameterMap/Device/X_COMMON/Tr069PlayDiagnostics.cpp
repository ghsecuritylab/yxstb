
#include "Tr069PlayDiagnostics.h"

#include "Tr069FunctionCall.h"

#include "Tr069.h"
#include "mid_stream.h"
#include "ind_mem.h"
#include "app_epg_para.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "NativeHandler.h"
#include "mid/mid_time.h"
#include "mid/mid_timer.h"
#include "Tr069PlayInfo.h"
#include "UltraPlayer.h"

#include <stdio.h>
#include <string.h>


struct Diagnostics {
    char PlayURL[PLAY_DIANOSTICS_URL_LEN];
    int  ReadyPlay;
    int  DiagnosticsState;
    int  PlayState;
    int  PlayingInPage;
};

static int g_tr069_play_url_mode = 0;
static struct Diagnostics g_diagnostics = {"", 1, 0, 0, 0};
static char g_diagnosticsStateString[][15] = {"None", "Requested", "Complete", "9822", "9823", "9824", "9825", "9826", "9827"};

int tr069_get_playurl_mode(void)
{
    return g_tr069_play_url_mode;
}

void tr069_set_playurl_mode(int mode)
{
    g_tr069_play_url_mode = mode;
    return;
}

void tr069_diagnostics_task(int arg)
{
    LogUserOperDebug("State ... g_diagnostics.DiagnosticsState(%d)\n", g_diagnostics.DiagnosticsState);
    if(DIAGNOSTATICS_STATE_REQUESTED == g_diagnostics.DiagnosticsState) {
        int atype = 0, play_state = 0;

        atype = mid_stream_get_apptype(0);
        play_state = GetCurrentPlayStatus();
        LogUserOperDebug("State is still REQUESTED then timeout atype = %d, Play state(%d)\n", atype, play_state);
        if(STRM_STATE_CLOSE == play_state || STRM_STATE_OPEN == play_state)
            tr069_diagnostics_set_state_by_enum(DIAGNOSTATICS_STATE_9822);
        else if(atype == APP_TYPE_IPTV)
            tr069_diagnostics_set_state_by_enum(DIAGNOSTATICS_STATE_9827);
        else
            tr069_diagnostics_set_state_by_enum(DIAGNOSTATICS_STATE_9826);
    }
    //if(DIAGNOSTATICS_STATE_COMPLETE == g_diagnostics.DiagnosticsState) {printf("***{%d}\n", __LINE__);
        ////柳建华的向上通知接口
    //TR069_EVENT_POST(TR069_EVENTCODE_EXTS_COMPLETE); //编译 不过
    //}
    LogUserOperDebug("Task end...\n");
    return;
}

void tr069_play_start(void)
{
    LogUserOperDebug("g_diagnostics.ReadyPlay[%d]\n", g_diagnostics.ReadyPlay);
    if(DIAGNOSTATICS_STATE_REQUESTED == g_diagnostics.DiagnosticsState && 1 == g_diagnostics.ReadyPlay) {
        g_diagnostics.ReadyPlay = 0;
        if(strlen(g_diagnostics.PlayURL) < 10) {
            LogUserOperDebug("Error happens, playURL len <10,  playURL[%s]\n", g_diagnostics.PlayURL);
            tr069_diagnostics_set_state_by_enum(DIAGNOSTATICS_STATE_9825);
            return;
        }
        LogUserOperDebug("DiagnosticsState[%s] playURL[%s]\n", g_diagnosticsStateString[g_diagnostics.DiagnosticsState], g_diagnostics.PlayURL);

        app_stbmonitor_tms_url_set(g_diagnostics.PlayURL);
        tr069_set_playurl_mode(1);
        sendMessageToNativeHandler(MessageType_System, MV_System_OpenTransparentPage, 0, 0);
        mid_timer_create(9, 1, (mid_timer_f)tr069_diagnostics_task, 0);
    }
    return;
}

void tr069_diagnostics_set_state_by_enum(int d_state)
{
    g_diagnostics.DiagnosticsState = d_state;
    LogUserOperDebug("Set g_diagnostics.DiagnosticsState [%d]\n", g_diagnostics.DiagnosticsState);
    return;
}


void tr069_set_PlayURL(char *value)
{
    if(!value) {
        LogUserOperDebug("Error happens! NULL == value\n");
        return;
    }
    memset(g_diagnostics.PlayURL, 0, sizeof(g_diagnostics.PlayURL));
    LogUserOperDebug("Befor strcpy \n");
    strcpy(g_diagnostics.PlayURL, value);
    LogUserOperDebug("Set g_diagnostics.PlayURL[ %s]\n", g_diagnostics.PlayURL);
    return;
}

void tr069_get_PlayURL(char *value, int size)
{
    if(value) {
        LogUserOperDebug("function come in,  value [%s], size[%d]\n", value, size);
        /**size 为柳建华存储play url 的buff空间大小，这里不需做判断**/
        //if(size <= 0 || size >= (PLAY_DIANOSTICS_URL_LEN - 20))
        //  return;
        if(DIAGNOSTATICS_STATE_REQUESTED == g_diagnostics.DiagnosticsState) {
            snprintf(value, size, "%s", g_diagnostics.PlayURL);
            LogUserOperDebug("Set value[%s], size[%d],  return \n", value, size);
            return ;
        }
        //strncpy(value, g_diagnostics.PlayURL, size);
        char url[PLAY_DIANOSTICS_URL_LEN] = {0};
        //app_get_streamURL(url);
        Tr069GetCurrentPlayURL(url, PLAY_DIANOSTICS_URL_LEN);
        if(!strcmp(url, "none") || strlen(url) >= size)
            value[0] = 0;
        else
            strcpy(value, url);
        LogUserOperDebug("function come out,  value [%s], size[%d]\n", value, size);
    }
    return;
}

void tr069_diagnostics_get_state(char *value, int size)
{
    if(value) {
        strcpy(value, g_diagnosticsStateString[g_diagnostics.DiagnosticsState]);
        LogUserOperDebug("function come out value [%s] size[%d]\n", value, size);
    }
    return;
}


void tr069_diagnostics_set_state(char *value)
{
    int i;

    if(value) {
        LogUserOperDebug("function come in, Set value [%s]\n", value);
        for(i = 0; i < 9; i++) {
            LogUserOperDebug("g_diagnosticsStateString[%d] = [%s]\n", i, g_diagnosticsStateString[i]);
            if(!strcmp(value, g_diagnosticsStateString[i])) {
                LogUserOperDebug("Error happens!\n");
                break;
            }
        }
        if(i >= 9) {
            LogUserOperDebug("Error happens !Can not fine value [%s]\n", value);
            return;
        }
        tr069_diagnostics_set_state_by_enum(i);
        g_diagnostics.ReadyPlay = 1;
    }
}

////////////////////////////////

Tr069Call* g_tr069PlayDiagnostics = new Tr069PlayDiagnostics();


/*------------------------------------------------------------------------------
 * 
 ------------------------------------------------------------------------------*/
static int getTr069PortDiagnosticsState(char* value, unsigned int size)
{
    tr069_diagnostics_get_state(value,size);
    
    return 0;
}

static int setTr069PortDiagnosticsState(char* value, unsigned int size)
{
    tr069_diagnostics_set_state(value);
    
    return 0;
}

/*------------------------------------------------------------------------------
 * 
 ------------------------------------------------------------------------------*/

static int getTr069PortPlayURL(char* value, unsigned int size)
{
    tr069_get_PlayURL(value, size);  
    
    return 0;
}

static int setTr069PortPlayURL(char* value, unsigned int size)
{
    tr069_set_PlayURL(value);
    
    return 0;
}


/*------------------------------------------------------------------------------
 * 
 ------------------------------------------------------------------------------*/
static int getTr069PortPlayState(char* value, unsigned int size)
{
    if (snprintf(value,size,"%d",Tr069GetCurrentPlayState()) >= size) {
        printf("Error.size is too short.");
        return -1;
    }

    return 0;
}

static int setTr069PortPlayState(char* value, unsigned int size)
{
    //tr069_set_PlayState(value);返回空

    return 0;
}
 

/*------------------------------------------------------------------------------
 * 以下对象的注册到表root.Device.XXX.PlayDiagnostics
 * XXX可以是X_CTC_IPTV，X_00E0FC
 ------------------------------------------------------------------------------*/

Tr069PlayDiagnostics::Tr069PlayDiagnostics()
	: Tr069GroupCall("PlayDiagnostics")
{
    // CTC,HW共有 fun1 - fun3
    Tr069Call* fun1  = new Tr069FunctionCall("DiagnosticsState", getTr069PortDiagnosticsState, setTr069PortDiagnosticsState);
    regist(fun1->name(), fun1);    

    Tr069Call* fun2  = new Tr069FunctionCall("PlayURL", getTr069PortPlayURL, setTr069PortPlayURL);
    regist(fun2->name(), fun2);    
    
    Tr069Call* fun3  = new Tr069FunctionCall("PlayState", getTr069PortPlayState, setTr069PortPlayState);
    regist(fun3->name(), fun3);   
    
     
}

Tr069PlayDiagnostics::~Tr069PlayDiagnostics()
{
}

