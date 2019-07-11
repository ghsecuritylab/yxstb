
#include "JseHWAudio.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#if (SUPPORTE_HD)
#include "mid_stream.h"
#endif

#include "AppSetting.h"
#include "mid_task.h"

#include "codec.h"
#include "app_tool.h"

#include <stdio.h>
#include <stdlib.h>


static int JseDefaultAudioChannelRead(const char* param, char* value, int len)
{
    appSettingGetString("defaultAudioChannel", value, len, 0);
    return 0;
}

static int JseDefaultAudioChannelWrite(const char* param, char* value, int len)
{
    appSettingSetString("defaultAudioChannel", value);
    return 0;
}

static int JseDefaultAudioLanguageRead(const char *param, char *value, int len)
{
    appSettingGetString("defaultAudioLanguage", value, len, 0);
    return 0;
}

static int JseDefaultAudioLanguageWrite(const char *param, char *value, int len)
{
    appSettingSetString("defaultAudioLanguage", value);
    settingManagerSave();
    return 0;
}

static int JseChannelAudioSelectWrite(const char* param, char* value, int len)
{
    LogJseDebug("channel_audio_select:%s\n", value);
#if (SUPPORTE_HD)
    mid_stream_mosaic_set(atoi(value));
#endif
    return 0;
}

/*{"audio_track_list_count":"2","audio_track_list":[{"PID":"33", "language_code":"chi", "language_eng_name":"Chinese"},{"PID":"34", "language_code":"eng", "language_eng_name":"English"}]}*/
static int JseAllAudioTrackInfoRead(const char* param, char* value, int len)
{
    int track_num = 0;
    int pid = 0;
    char lan[4] = {0};
    char language[32] = {0};
    int tempLen = 0, i;

    codec_audio_track_num(&track_num);
    if (track_num <= 0) {
        mid_task_delay(1000);
       codec_audio_track_num(&track_num);
    }
    tempLen = snprintf(value, len, "{\"audio_track_list_count\":%d,\"audio_track_list\":[", track_num);

    for(i = 0; i < track_num; i++) {
        codec_audio_track_get_pid(i, &pid);
        codec_audio_track_get_info(i, lan);
        language[0] = '\0';
        if(lan[0]!= '\0')
            mid_get_language_full_name_from_iso639(lan, language, 32);
        tempLen += sprintf(value + tempLen,"{\"PID\":%d,\"language_code\":\"%s\",\"language_eng_name\":\"%s\"},", pid, lan, language);
    }
    if (track_num > 0) {
        sprintf(value + tempLen - 1,"%s", "]}");
        return 0;
    }
    else {
        sprintf(value + tempLen,"%s", "]}");
        return -1;
    }
}

static int JseAllAudioTrackInfoWrite(const char* param, char* value, int len)
{
    LogJseError("Can't support this function !\n");
    return 0;
}

//{"PID":"33","language_code":"chi","language_eng_name":"Chinese"}
static int JseCurrentAudioTrackInfoRead(const char* param, char* value, int len)
{
    int track = 0;
    int pid = 0;
    char lan[4] = {0};
    char language[32] = {0};

    if(value) {
        codec_audio_track_get(&track);
        codec_audio_track_get_pid(track, &pid);
        codec_audio_track_get_info(track, lan);
        mid_get_language_full_name_from_iso639(lan, language, 32);
        sprintf(value,"{\"PID\":%d,\"language_code\":\"%s\",\"language_eng_name\":\"%s\"}", pid, lan, language);
    }
    return 0;
}

static int JseHWOpAudiotrackSelectWrite(const char* param, char* value, int len)
{
    int track_num = 0, i = 0, id;

    codec_audio_track_num(&track_num);
    for(i = 0; i < track_num; i++){
        codec_audio_track_get_pid(i, &id);
        if(id == atoi(value)){
            codec_audio_track_set(i);
            return 0;
        }
    }
    LogJseError("No audio pid(%d)\n", atoi(value));
    return 0;
}

/*******************************************************************************
Functional description:
    Initialize huawei Audio module configuration defined interfaces, by JseHWPlays.CPP calls
Parameter:
Note:
Following specifications:
 *************************************************/
int JseHWAudioInit()
{
    JseCall* call;

    //C10/C20 regist
    call = new JseFunctionCall("defaultAudioChannel", JseDefaultAudioChannelRead, JseDefaultAudioChannelWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("defaultAudioLanguage", JseDefaultAudioLanguageRead, JseDefaultAudioLanguageWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("channel_audio_select", 0, JseChannelAudioSelectWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("AllAudioTrackInfo", JseAllAudioTrackInfoRead, JseAllAudioTrackInfoWrite);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("AllAudiotrackInfo", JseAllAudioTrackInfoRead, JseAllAudioTrackInfoWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("CurrentAudiotrackInfo", JseCurrentAudioTrackInfoRead, 0);
    JseRootRegist(call->name(), call);
    call = new JseFunctionCall("CurrentAudioTrackInfo", JseCurrentAudioTrackInfoRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("hw_op_audiotrack_select", 0, JseHWOpAudiotrackSelectWrite);
    JseRootRegist(call->name(), call);
    return 0;
}


