
#include "KeyTableParser.h"
#include "BrowserAssertions.h"

#include "MessageTypes.h"
#include "KeyDispatcher.h"
#include "browser_event.h"

#include "UltraPlayer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "sys_msg.h"
#include "customer.h"
#include "mid/mid_http.h"

extern char* global_cookies;

namespace Hippo {
#if defined(GUANGDONG) ||defined(Gansu)
static bool s_PausePlaySet = false;
#endif

int
KeyTableParser(const char *table)
{
    char *tableBegin = NULL, *tableEnd = NULL;
    char tagValue[512] = {0};
    char *ptr = NULL;

    if(table == NULL) {
        BROWSER_LOG_VERBOSE(table);
        return -1;
    }
    tableBegin = strstr((char *)table, "<global_keytable>");
    tableEnd = strstr((char *)table, "</global_keytable>");
    if(!tableBegin || !tableEnd) {
        BROWSER_LOG_VERBOSE(tableBegin);
        BROWSER_LOG_VERBOSE(tableEnd);
        return -1;
    }
    KeyDispatcherPolicy::Policy PolicyVar;
    ptr = tableBegin + (sizeof("<global_keytable>") - 1);
    BROWSER_LOG_VERBOSE(">>>keytable:[\n%s\n]\n", table);
    while(ptr < tableEnd) {
        char *tagBegin = NULL, *tagEnd = NULL, *p = NULL;
        char keyname[32] = {0};
        int restype = 0;
        char url[128] = {0};
        int len = 0;

        KeyDispatcherPolicy* KeyPolicy = new KeyDispatcherPolicy;
        memset(url, 0, sizeof(url));
        memset(tagValue, 0, sizeof(tagValue));
        KeyPolicy->mEnable = 1;
        if(*ptr == '\n') {
            ptr++;
        }
        tagBegin = strstr(ptr, "<response_define>");
        tagEnd = strstr(ptr, "</response_define>");
        if(!tagBegin || !tagEnd) {
            delete KeyPolicy;
            break;
        }
        tagBegin += strlen("<response_define>");
        if((tagEnd - tagBegin) >= 512) {
            delete KeyPolicy;
            BROWSER_LOG_ERROR("KeyTableParser error (tagEnd-tagEnd)>=512 !\n");
            break;
        }
        strncpy(tagValue, tagBegin, tagEnd - tagBegin);
        ptr = tagEnd + (sizeof("</response_define>") - 1);
        tagBegin = strstr(tagValue, "<key_name>"); // Parser key_name
        tagEnd = strstr(tagValue, "</key_name>");
        if(!tagBegin || !tagEnd) { // Parser key_code
            tagBegin = strstr(tagValue, "<key_code>");
            tagEnd = strstr(tagValue, "</key_code>");
        }
        if(!tagBegin || !tagEnd) {
            delete KeyPolicy;
            continue;
        }
        p = tagBegin + (sizeof("<key_name>") - 1);
        len = tagEnd - p;
        strncpy(keyname, p, len);
        keyname[len] = '\0';
        //tagBegin = strstr(tagValue, "<event_type>"); // Parser event_type
        //tagEnd = strstr(tagValue, "</event_type>");
        tagBegin = strstr(tagValue, "<response_type>"); // Parser response_type
        tagEnd = strstr(tagValue, "</response_type>");
        if(tagBegin && tagEnd) {
            p = tagBegin + (sizeof("<response_type>") - 1);
            restype = atoi(p);
        }
        tagBegin = strstr(tagValue, "<service_url>"); // Parser URL
        tagEnd = strstr(tagValue, "</service_url>");
        if(tagBegin && tagEnd) {
            p = tagBegin + (sizeof("<service_url>") - 1);
            len = tagEnd - p;
            strncpy(url, p, len);
            url[len] = '\0';
        }
        switch(restype) {
        case 0: {
            PolicyVar = KeyDispatcherPolicy::NativeFirst;
            break;
        }
        case 1: {
            PolicyVar = KeyDispatcherPolicy::JSFirst;
            break;
        }
        case 2: {
            PolicyVar = KeyDispatcherPolicy::OpenUrl;
            break;
        }
        default: {
            PolicyVar = KeyDispatcherPolicy::NativeFirst;
            break;
        }
        }
        if(0 == strcmp(keyname, "KEY_VOL_DOWN") || 0 == strcmp(keyname, "KEY_VOL_UP")) {
            if(KeyDispatcherPolicy::NativeFirst == PolicyVar) {
                LogUserOperError("Set volume mask!\n");
                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() | UltraPlayer::AudioVolume_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() | UltraPlayer::AudioVolume_Mask);
            } else {
                LogUserOperError("Clear volume mask!\n");
                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() & ~UltraPlayer::AudioVolume_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() & ~UltraPlayer::AudioVolume_Mask);
            }
        }
        // Half play control adjust UIFlag according to the global key value table.
        if (0 == strcmp(keyname, "KEY_PAUSE_PLAY")) {
            if(KeyDispatcherPolicy::NativeFirst == PolicyVar) {
                LogUserOperError("Set ProgressBar_Mask.\n");
                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() | UltraPlayer::ProgressBar_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() | UltraPlayer::ProgressBar_Mask);

                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() | UltraPlayer::PlayState_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() | UltraPlayer::PlayState_Mask);

                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() | UltraPlayer::AudioVolume_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() | UltraPlayer::AudioVolume_Mask);

                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() | UltraPlayer::AudioMute_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() | UltraPlayer::AudioMute_Mask);

                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() | UltraPlayer::AudioTrack_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() | UltraPlayer::AudioTrack_Mask);
            } else {
                LogUserOperError("Clear ProgressBar_Mask.\n");
                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() & ~UltraPlayer::ProgressBar_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() & ~UltraPlayer::ProgressBar_Mask);
#ifndef HUBEI_HD
                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() & ~UltraPlayer::PlayState_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() & ~UltraPlayer::PlayState_Mask);

                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() & ~UltraPlayer::AudioVolume_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() & ~UltraPlayer::AudioVolume_Mask);

                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() & ~UltraPlayer::AudioMute_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() & ~UltraPlayer::AudioMute_Mask);

                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() & ~UltraPlayer::AudioTrack_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() & ~UltraPlayer::AudioTrack_Mask);
#endif
            }

#if defined(GUANGDONG) // Compatible with domestic individual CTC point
            /*In the domestic half broadcast, issued by EPG pageup, pagedown for js priority processing.
            But half of broadcast control and to deal with the key, so the key of kePolicy value with
            paly_pause key change.*/
            keyDispatcher().setPolicyByName("KEY_VOL_UP", 0, PolicyVar, url, 2);
            keyDispatcher().setPolicyByName("KEY_VOL_DOWN", 0, PolicyVar, url, 2);
            if(KeyDispatcherPolicy::NativeFirst == PolicyVar) {
                LogUserOperError("Set volume mask!\n");
                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() | UltraPlayer::AudioVolume_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() | UltraPlayer::AudioVolume_Mask);
            } else {
                LogUserOperError("Clear volume mask!\n");
                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() & ~UltraPlayer::AudioVolume_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() & ~UltraPlayer::AudioVolume_Mask);
            }
            keyDispatcher().setPolicyByName("KEY_PAGE_DOWN", 0, PolicyVar, url, 2);
            keyDispatcher().setPolicyByName("KEY_PAGE_UP", 0, PolicyVar, url, 2);
            s_PausePlaySet = true;

#elif defined(Gansu)
            keyDispatcher().setPolicyByName("KEY_VOL_UP", 0, PolicyVar, url, 2);
            keyDispatcher().setPolicyByName("KEY_VOL_DOWN", 0, PolicyVar, url, 2);
           // keyDispatcher().setPolicyByName("KEY_AUDIO_MODE", 0, PolicyVar, url, 2);

            if(KeyDispatcherPolicy::NativeFirst == PolicyVar) {
                LogUserOperError("Set volume mask!\n");
                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() | UltraPlayer::AudioVolume_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() | UltraPlayer::AudioVolume_Mask);

                //UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() | UltraPlayer::AudioTrack_Mask);
                //UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() | UltraPlayer::AudioTrack_Mask);

            } else {
                LogUserOperError("Clear volume mask!\n");
                UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() & ~UltraPlayer::AudioVolume_Mask);
                UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() & ~UltraPlayer::AudioVolume_Mask);

                //UltraPlayer::setUIFlagsForcedMask(UltraPlayer::getUIFlagsForcedMask() & ~UltraPlayer::AudioTrack_Mask);
                //UltraPlayer::setUIFlagsForcedValue(UltraPlayer::getUIFlagsForcedValue() & ~UltraPlayer::AudioTrack_Mask);

            }
            s_PausePlaySet = true;
#endif
        }

#if defined(GUANGDONG)
        if((!strcmp(keyname, "KEY_VOL_UP")
                || !strcmp(keyname, "KEY_VOL_DOWN")
                || !strcmp(keyname, "KEY_PAGE_DOWN")
                || !strcmp(keyname, "KEY_PAGE_UP"))
            && s_PausePlaySet) {
                delete KeyPolicy;
                continue;
            }
#elif  defined(Gansu)
        if((!strcmp(keyname, "KEY_VOL_UP")
                || !strcmp(keyname, "KEY_VOL_DOWN")
                //|| !strcmp(keyname, "KEY_AUDIO")
                //|| !strcmp(keyname, "KEY_AUDIO_MODE")
                )
            && s_PausePlaySet) {
                delete KeyPolicy;
                continue;
            }
#endif
#if defined(HUBEI_HD)
        if(!strcmp(keyname, "KEY_VOL_UP")
            || !strcmp(keyname, "KEY_VOL_DOWN")
            || !strcmp(keyname, "KEY_MUTE")
            || !strcmp(keyname, "KEY_AUDIO")
            || !strcmp(keyname, "KEY_AUDIO_MODE")) {
                delete KeyPolicy;
                continue;
            }
#endif
        keyDispatcher().setPolicyByName(keyname, 0, PolicyVar, url, 2);
        delete KeyPolicy;
    }
    return 0;
}

static int
KeyTableParser_CallBack(int err, char* xml, int len, int arg)
{
    KeyTableParser(xml);
    sendMessageToNativeHandler(MessageType_System, KEYTABLE_UPDATE, 0, 0);
    return 0;
}


static int
KeyTableUrlParse_CallBack(int err, char* keyTableUrl, int len, int arg)
{
    char url[1028] = {0};
    char *p1 = NULL, *p2 = NULL;

    if(NULL == keyTableUrl) {
        BROWSER_LOG_VERBOSE("The epgkeytable_url is NULL!\n");
        return -1;
    }
    p1 = strstr(keyTableUrl, "href = \"");
    if(NULL == p1) {
        BROWSER_LOG_VERBOSE("href not found!\n");
        return -1;
    }
    p1 += (sizeof("href = \"") - 1);
    if(NULL != p1) {
        p2 = strchr(p1, '\"');
        if(NULL != p2) {
            strncpy(url, p1, (p2 - p1));
            url[p2 - p1] = '\0';
            KeyTableGet(url);
            return 0;
        }
    }
    return -1;
}

} // namespace Hippo

extern "C" int
KeyTableGet(const char* keyTableUrl)
{
    if (!keyTableUrl) {
        BROWSER_LOG_VERBOSE("The epgkeytable_url is NULL!\n");
        return -1;
    }
    std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();
    if (url.empty())
        return -1;

    url += keyTableUrl;
    if(mid_http_call(url.c_str(), (mid_http_f)Hippo::KeyTableParser_CallBack, 0, NULL, 0, global_cookies) != 0) {
        BROWSER_LOG_ERROR("request epg keytable error !\n");
        return -1;
    }
    return 0;
}

extern "C" int
KeyTableUrlGet()        //ªÒ»°EPG Server IP
{
    std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();
    if (url.empty())
        return -1;
    url += "/EPG/jsp/EPGServlet?action_type=GET_KEY_TABLE";

    if(mid_http_call(url.c_str(), (mid_http_f)Hippo::KeyTableParser_CallBack, 0, NULL, 0, global_cookies) != 0) {
        BROWSER_LOG_ERROR("request epg keytable_URL error.\n");
        return -1;
    }
    return 0;
}

extern "C" int
KeyTableUpdate()
{
    return 0;
}

extern "C" int
KeyTableParserTest()
{
    const char* keystr = "\
<?xml version='1.0'>\n\
<global_keytable>\n\
   <response_define>\n\
      <key_name>KEY_INTERX</key_name>\n\
      <response_type>1</response_type>\n\
   </response_define>\n\
   <response_define>\n\
      <key_code>KEY_PORTAL</key_code>\n\
      <response_type>2</response_type>\n\
      <service_url>/EPG/jsp/index.jsp?action=portal </service_url>\n\
   </response_define>\n\
  <response_define>\n\
      <key_code>KEY_RED</key_code>\n\
      <response_type>2</response_type>\n\
      <service_url>/EPG/jsp/index.jsp?action=red </service_url>\n\
   </response_define>\n\
      <response_define>\n\
      <key_code>KEY_GREEN</key_code>\n\
      <response_type>2</response_type>\n\
      <service_url>/EPG/jsp/index.jsp?action=green </service_url>\n\
   </response_define>\n\
         <response_define>\n\
      <key_code>KEY_YELLOW</key_code>\n\
      <response_type>2</response_type>\n\
      <service_url>/EPG/jsp/index.jsp?action=yellow </service_url>\n\
   </response_define>\n\
         <response_define>\n\
      <key_code>KEY_BLUE</key_code>\n\
      <response_type>2</response_type>\n\
      <service_url>/EPG/jsp/index.jsp?action=blue </service_url>\n\
   </response_define>\n\
         <response_define>\n\
      <key_code>KEY_HELP</key_code>\n\
      <response_type>2</response_type>\n\
      <service_url>/EPG/jsp/index.jsp?action=help </service_url>\n\
   </response_define>\n\
  </global_keytable>";
    return Hippo::KeyTableParser(keystr);
}


