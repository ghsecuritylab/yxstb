
#include "CTCServiceEntry.h"
#include "ProgramParser.h"

#include "KeyDispatcher.h"
#include "browser_event.h"
#include "BrowserAssertions.h"
#include "SysSetting.h"

#include "Hippo_api.h"

#ifdef INCLUDE_DMR
#include "../../../../../porting/Hybroad_dlna/DLNARegister.h"
#endif

#include "AppSetting.h"

#include "app_epg_para.h"
#include "app_sys.h"
#include <string.h>

#include "customer.h"

using namespace std;
//关键字:全局键值表 文档:.../Annotation/全局键值表.doc
namespace Hippo {

enum{
    INTERX_BUTTON = 269,
    PORTAL_BUTTON = 272,
    SHANGHAI_EPG_BUTTON = 274,
    BTV_BUTTON = 275,
    TVOD_BUTTON = 276,
    VOD_BUTTON = 277,
    NVOD_BUTTON = 278,
    LOCALPLAYER_BUTTON = 280,
    LOCALPLAYER_BUTTON2 = 287,
    DLNA_PUSH = 1024,
    URG_REG = 1040,
};

static int hotkeyTranslate(int key)
{
    int key_translate = 0;

    switch(key){
    case PORTAL_BUTTON:
        key_translate = EIS_IRKEY_MENU;
        break;
    case BTV_BUTTON:
#if defined(GUANGDONG)
        key_translate = EIS_IRKEY_RED;
#else
        key_translate = EIS_IRKEY_BTV;
#endif
        break;
    case TVOD_BUTTON:
#if defined(GUANGDONG)
        key_translate = EIS_IRKEY_GREEN;
#else
        key_translate = EIS_IRKEY_TVOD;
#endif
        break;
    case VOD_BUTTON:
#if defined(GUANGDONG)
        key_translate = EIS_IRKEY_YELLOW;
#else
        key_translate = EIS_IRKEY_VOD;
#endif
        break;
    case NVOD_BUTTON:
#ifdef Sichuan
        key_translate = EIS_IRKEY_BLUE;
#else
        key_translate = EIS_IRKEY_NVOD;
#endif
        break;
    case LOCALPLAYER_BUTTON:
    case LOCALPLAYER_BUTTON2:
        key_translate = EIS_IRKEY_DESKTOP;
        break;
#ifdef INCLUDE_DMR
    case DLNA_PUSH:
        key_translate = EIS_IRKEY_DLNA_PUSH;
        break;
    case URG_REG:
        key_translate = EIS_IRKEY_URG_REG;
        break;
#endif
    }
    return key_translate;
}

/**********************************************
*function : set shortcut url
*input fieldName : ServiceEntry
*                : URL="http://110.1.1.164:33200/EPG/jsp/getServicesEntryHWCTC.jsp?action=PortalEPGURL",
                   HotKey="272",Desc="null"
*
**********************************************/
int parseServiceEntry(const char* fieldName, const char* fieldParam, char* fieldValue, int result)
{
    std::string sInfo = fieldValue;
    vector<std::string> key, value;
    vector<std::string>::iterator keyIt, valueIt;
    std::string sURL(""), sDesc("");
    int hotkey = 0, i = 0;

    ProgramParser::tokenize(sInfo, key, value);
    keyIt = key.begin();
    valueIt = value.begin();
    for(i = 0; i < key.size() && i < value.size(); ++i){
        if(!strncasecmp(key[i].c_str(),"URL", 3)){
            sURL = value[i];
        }else if(!strncasecmp(key[i].c_str(), "HotKey", 6)){
            hotkey = atoi(value[i].c_str());
        }else if(!strncasecmp(key[i].c_str(), "Desc", 4)){
            sDesc = value[i];
        }
    }
    BROWSER_LOG("hotkey:%d, URL:%s\n", hotkey, sURL.c_str());
    hotkey = hotkeyTranslate(hotkey);
    if(0 == hotkey || sURL.empty()){
        ERR_OUT("invalid info:%s\n", fieldValue);
    }
    BROWSER_LOG("hotkey:%d, URL:%s\n", hotkey, sURL.c_str());

    keyDispatcher().setPolicy(hotkey, 0, 2, (char *)sURL.c_str(), 1);

    if (EIS_IRKEY_MENU == hotkey) {    // back epg url
        /* 按跑了这么多年的老代码拼装lastepg，
            用于主备eds不通时走lastepg，和待机时的Logout
        */
        std::string lastepg(sURL);
        if (!lastepg.empty()) {
            std::string::size_type position1;
            if ((position1 = lastepg.find("://")) != std::string::npos) {
                if ((position1 = lastepg.find('/', position1 + 3)) != std::string::npos) {
                    /* lastepg=http://110.1.1.164:33200/EPG/jsp/getServicesEntryHWCTC.jsp?action=PortalEPGURL */
                    lastepg.erase(position1, lastepg.length());
                    /* lastepg=http://110.1.1.164:33200*/
                    std::string urlStr = Hippo::Customer().AuthInfo().eds().Current();
                    /* urlStr=http://huawei.epg.net.cn:33200/EPG/jsp/AuthenticationURL?UserID=13131&Action=Login
                     *  or urlStr=http://zte.epg.net.cn:8080/iptvepg/platform/index.jsp?UserID=13131&Action=Login
                     *  or Current EPG
                    */
                    position1 = urlStr.find("://");
                    if (position1 != std::string::npos) {
                        position1 = urlStr.find('/', position1 + 3);
                        if (position1 != std::string::npos) {
                            std::string::size_type position2 = urlStr.find("?UserID=");
                            if (position2 != std::string::npos) {
                                lastepg += urlStr.substr(position1, (position2 > position1) ? (position2 - position1) : (position1 - position2));
                                /* urlStr=/EPG/jsp/AuthenticationURL
                                 *  or urlStr=/iptvepg/platform/index.jsp
                                 * then lastepg=http://110.1.1.164:33200 + urlStr
                                */
                                appSettingSetString("epg", lastepg.c_str());
                                settingManagerSave();
                                BROWSER_LOG("lastepg=[%s]\n", lastepg.c_str());
                            }
                        }
                    }
                }
            }
        }

        // 代码废弃原因：
        // 下发什么就存什么。有兼容问题的话在拼串的时候修正。
        // 中兴明显不是 /EPG/jsp/AuthenticationURL 的地址，但是又有ServiceEntry
        // 以前这样写肯定有问题。
        // std::string epg_url = sURL.c_str();
        // string::size_type pos = epg_url.find('/', 7);
        // if(pos != string::npos){
        //     std::string last_epg = epg_url.substr(0, pos);
        //     last_epg += "/EPG/jsp/AuthenticationURL";
        //     sys_setepgurl((char *)last_epg.c_str());
        // }
    }
#ifdef INCLUDE_DMR
    if (hotkey == EIS_IRKEY_URG_REG) {
	    DLNARegister *p_register = new DLNARegister();
	    p_register->registSTBToURG(1);
    }
#endif
Err:
    return 0;
}

} // namespace Hippo

