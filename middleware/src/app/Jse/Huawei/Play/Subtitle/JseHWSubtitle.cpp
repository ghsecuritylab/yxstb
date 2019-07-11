
#include "JseHWSubtitle.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "AppSetting.h"

#include "app_tool.h"
#include "codec.h"

#include <stdio.h>
#include <stdlib.h>

static int JseDefaultSubtitleLanguageRead(const char *param, char *value, int len)
{
    appSettingGetString("defaultSubtitleLanguage", value, len, 0);
    return 0;
}

static int JseDefaultSubtitleLanguageWrite(const char *param, char *value, int len)
{
    appSettingSetString("defaultSubtitleLanguage", value);
    codec_default_subtile(0, value);
    settingManagerSave();
    return 0;
}

static int JseSubtitleRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseSubtitleWrite(const char* param, char* value, int len)
{
    return 0;
}

static int JseSubtitileFlagRead(const char* param, char* value, int len)
{
    int tSubtitleIndex = 0;

    codec_subtitle_get(&tSubtitleIndex);
    tSubtitleIndex++;
    snprintf(value, len, "%d", tSubtitleIndex);
    return 0;
}

static int JseSubtitileFlagWrite(const char* param, char* value, int len)
{
    int tCount = 0, tIndex = 0;

    tIndex = atoi(value);
    if(tIndex == -1) {
        codec_subtitle_set(-1);
    } else {
        codec_subtitle_num(&tCount);
        if((tIndex - 1) < tCount) {
            codec_subtitle_set(tIndex - 1);
        } else {
            codec_subtitle_set(-1);
        }
    }
    return 0;
}

static int JseSubtitleEnableFlagWrite(const char* param, char* value, int len)
{
    codec_subtitle_show_set(atoi(value));
    return 0;
}

static int JseSubtitileNumRead(const char* param, char* value, int len)
{
    int tNum = 0;

    codec_subtitle_num(&tNum);
    snprintf(value, len, "%d", tNum);
    return 0;
}

static int JseSubtitileLanRead(const char* param, char* value, int len)
{
    int tIndex = 0;

    if(!value)
        return -1;
    codec_subtitle_get(&tIndex);
    codec_subtitle_lang(tIndex, value);
    return 0;
}

static int JseAllSubtitleInfoRead(const char* param, char* value, int len)
{
    int subnum = 0, tBufLen = 0;
    int tIndex = 0;

    char lan[4] = {0};
    char language[32] = {0};
    unsigned short subpid;

    if(!value) {
        return -1;
    }

    codec_subtitle_num(&subnum);
    if(subnum == 0) {
        LogJseDebug("NO subtitle this channel!\n");
        tBufLen += snprintf(value + tBufLen, len - tBufLen, "{\"subtitle_list_count\":\"%d\"}", subnum);
        return 0;
    }

    tBufLen += snprintf(value + tBufLen, len - tBufLen, "{\"subtitle_list_count\":\"%d\",\"subtitle_list\":[", subnum);

    for(tIndex = 0; tIndex < subnum; tIndex++) {
        codec_subtitle_pid(tIndex, &subpid);
        codec_subtitle_lang(tIndex, lan);

        if(lan[0] != '\0')
            mid_get_language_full_name_from_iso639(lan, language, 32);
        tBufLen += snprintf(value + tBufLen, len - tBufLen, "{\"PID\":\"%d\",\"language_code\":\"%s\",\"language_eng_name\":\"%s\"},", subpid, lan, language);
    }
    snprintf(value + tBufLen - 1, len - tBufLen + 1, "%s", "]}");
    return 0;
}

// lh 2010-3-14  查询当前字幕信息
static int JseCurrentSubtitleInfoRead(const char* param, char* value, int len)
{
    int tIndex = 0;
    int length;
    unsigned short subpid;
    char lang[4] = {0};
    char language[32] = {0};

    if(!value) {
        return -1;
    }

    length = 0;
    codec_subtitle_get(&tIndex);
    codec_subtitle_pid(tIndex, &subpid);
    codec_subtitle_lang(tIndex, lang);
    if(lang[0] != '\0')
        mid_get_language_full_name_from_iso639(lang, language, 32);

    length += sprintf(value + length, "{\"PID\":\"%d\",\"language_code\":\"%s\",\"language_eng_name\":\"%s\"}", subpid, lang, language);

    return 0;
}

static int JseHWOpSubtitleSelectWrite(const char* param, char* value, int len)
{
    int num = 0, i = 0;
    unsigned short id;

    codec_subtitle_num(&num);
    for(i = 0; i < num; i++) {
        codec_subtitle_pid(i, &id);
        if(id == atoi(value)) {
            codec_subtitle_set(i);
            return 0;
        }
    }
    return 0;
}

/*******************************************************************************
Functional description:
    Initialize huawei Subtitle module configuration defined interfaces, by JseHWPlays.CPP calls
Parameter:
Note:
Following specifications:
 *************************************************/
int JseHWSubtitleInit()
{
    JseCall* call;

    //C10/C20 regist
    call = new JseFunctionCall("defaultSubtitleLanguage", JseDefaultSubtitleLanguageRead, JseDefaultSubtitleLanguageWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("Subtitle", JseSubtitleRead, JseSubtitleWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("SubtitileFlag", JseSubtitileFlagRead, JseSubtitileFlagWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("SubtitleEnableFlag", 0, JseSubtitleEnableFlagWrite);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("SubtitileNum", JseSubtitileNumRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("SubtitileLan", JseSubtitileLanRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("AllSubtitleInfo", JseAllSubtitleInfoRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("CurrentSubtitleInfo", JseCurrentSubtitleInfoRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("hw_op_subtitle_select", 0, JseHWOpSubtitleSelectWrite);
    JseRootRegist(call->name(), call);
    return 0;
}


