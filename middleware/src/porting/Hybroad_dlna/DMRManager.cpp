#include "DMRManager.h"

#include "Hippo_api.h"
#include "DMRPlayer.h"
#include "DMRPlayerCTC.h"
#include "DMRPlayerHuawei.h"
#include "DMRPlayerHuaweiC30.h"
#include "KeyDispatcher.h"
#include "BrowserAgent.h"
#include "NativeHandler.h"
#include "DlnaAssertions.h"

#include <stdio.h>
#include <string.h>

#include "json/json_public.h"
#include "include/dlna/dlna_type.h"
#include "include/dlna/dlna_api.h"
#include "ipanel_event.h"

namespace Hippo{
static DMRManager* g_dmrManager = NULL;
static std::string g_result;
static pthread_mutex_t gDmrMutex = PTHREAD_MUTEX_INITIALIZER;

static int dlna_push_result(const char *aFieldName, const char *param_json, char *aFieldValue, int aResult)
{
    printf("dlna_push_result aFieldValue = %s\n", aFieldValue);
    g_dmrManager->removeDmrMessage(DLNA_EVENT_DMR_PLAY);
    g_dmrManager->SetEPGResult(aFieldValue, DLNA_EVENT_DMR_PLAY);

    return 0;
}

static int dlna_product_info(const char *aFieldName, const char *param_json, char *aFieldValue, int aResult)
{
    g_result += aFieldValue;
    std::string::size_type pos;

    DLNA_LOG("dlna_product_info aFieldValue = %s\n", aFieldValue);
    if ((pos = g_result.find("&...")) != std::string::npos) {
        g_result = g_result.substr(0, pos);
        return 0;
    }
    if ((pos = g_result.find("&&")) != std::string::npos) {
        g_result = g_result.substr(0, pos - 1);
    }
    DLNA_LOG("dlna_product_info g_result = %s\n", g_result.c_str());

    g_dmrManager->removeDmrMessage(DLNA_EVENT_DMR_GETPRODUCTINFO);
    g_dmrManager->SetEPGResult(g_result, DLNA_EVENT_DMR_GETPRODUCTINFO);

    g_result.clear();

    return 0;
}

static int dlna_order_result(const char *aFieldName, const char *param_json, char *aFieldValue, int aResult)
{
    DLNA_LOG("dlna_order_result aFieldValue = %s\n", aFieldValue);
	std::string result;
	g_dmrManager->removeDmrMessage(DLNA_EVENT_DMR_ORDER);
	for (int i = 0; i < strlen(aFieldValue); i++) {
		if (aFieldValue[i] != '\r' && aFieldValue[i] != '\n')
			result += aFieldValue[i];
	}
	g_dmrManager->SetEPGResult(result, DLNA_EVENT_DMR_ORDER);

    return 0;
}

static int dlna_pic_addr(const char *aFieldName, const char *param_json, char *aFieldValue, int aResult)
{
    sprintf(aFieldValue, "%s", g_dmrManager->GetPlayUrl().c_str());
    return 0;
}
const char *RemoteKeytab[38] =
{
	"0x0008",
	"0x000D",
	"0x0021",
	"0x0022",
	"0x0025",
	"0x0026",
	"0x0027",
	"0x0028",
	"0x0030",
	"0x0031",
	"0x0032",
	"0x0033",
	"0x0034",
	"0x0035",
	"0x0036",
	"0x0037",
	"0x0038",
	"0x0039",
	"0x0069",
	"0x006A",
	"0x0081",
	"0x0100",
	"0x0101",
	"0x0102",
	"0x0103",
	"0x0104",
	"0x0105",
	"0x0107",
	"0x010F",
	"0x0110",
	"0x0113",
	"0x0114",
	"0x0115",
	"0x0116",
	"0x0118",
	"0x0119",
	"0x011C",
	"0x011E"
};
int RemoteKeytabEIS[38] =
{
	EIS_IRKEY_BACK,
	EIS_IRKEY_SELECT,
	EIS_IRKEY_PAGE_UP,
	EIS_IRKEY_PAGE_DOWN,
	EIS_IRKEY_LEFT,
	EIS_IRKEY_UP,
	EIS_IRKEY_RIGHT,
	EIS_IRKEY_DOWN,
	EIS_IRKEY_NUM0,
	EIS_IRKEY_NUM1,
	EIS_IRKEY_NUM2,
	EIS_IRKEY_NUM3,
	EIS_IRKEY_NUM4,
	EIS_IRKEY_NUM5,
	EIS_IRKEY_NUM6,
	EIS_IRKEY_NUM7,
	EIS_IRKEY_NUM8,
	EIS_IRKEY_NUM9,
	EIS_IRKEY_IME,
	EIS_IRKEY_STAR,
	EIS_IRKEY_UNKNOWN,
	EIS_IRKEY_POWER,
	EIS_IRKEY_CHANNEL_UP,
	EIS_IRKEY_CHANNEL_DOWN,
	EIS_IRKEY_VOLUME_UP,
	EIS_IRKEY_VOLUME_DOWN,
	EIS_IRKEY_VOLUME_MUTE,
	EIS_IRKEY_PLAY,
	EIS_IRKEY_UNKNOWN,
	EIS_IRKEY_MENU,
	EIS_IRKEY_RED,
	EIS_IRKEY_GREEN,
	EIS_IRKEY_YELLOW,
	EIS_IRKEY_BLUE,
	EIS_IRKEY_SWITCH,
	EIS_IRKEY_UNKNOWN,
	EIS_IRKEY_UNKNOWN,
	EIS_IRKEY_AUDIO_MODE
};

void send_dlna_RemoteKey(const char *mediaCode)
{
    int i;
    for(i = 0 ; i<38;i++) {
        if(0 == strcmp(mediaCode,RemoteKeytab[i])) {
		DLNA_LOG("RemoteKeytabEIS[i] = %d\n",RemoteKeytabEIS[i]);
		sendMessageToKeyDispatcher(MessageType_KeyDown, RemoteKeytabEIS[i], 0, 0);
		break;
        }

    }
}

static int DMREventCallback(int eventID, void *eventArg, void *arg)
{
    pthread_mutex_lock(&gDmrMutex);
    DLNA_LOG("DMREventCallback\n");
    DLNA_LOG("eventID = %d, eventArg = %s\n", eventID, eventArg);
    if(DLNA_EVENT_DMR_RemoteKey == eventID)
    {
        struct json_object *temp_Object;
	 const char *mediaCode;
        DLNA_LOG("eventArg = %s\n", eventArg);
        temp_Object = json_tokener_parse((char *)eventArg);
        if (!temp_Object) {
            json_object_put(temp_Object);
            pthread_mutex_unlock(&gDmrMutex);
            return -1;
        }
        //obj = json_object_object_get(temp_Object, "Keycode");
        mediaCode = json_object_get_string(json_object_get_object_bykey(temp_Object, "Keycode"));
        send_dlna_RemoteKey(mediaCode);
        pthread_mutex_unlock(&gDmrMutex);
        json_object_put(temp_Object);
	 return 0;
    }

    DMRPlayer *dmrPlayer = NULL;
    if (defNativeHandler().getState() == NativeHandler::Local)
        dmrPlayer = new DMRPlayerHuawei(eventID, g_dmrManager);
#if (defined(Guangdong))
    else
        dmrPlayer = new DMRPlayerCTC(eventID, g_dmrManager);
#else
    else
        dmrPlayer = new DMRPlayerHuaweiC30(eventID, g_dmrManager);
#endif
    int ret = dmrPlayer->setDmrEventArg((char*)eventArg);
    DLNA_LOG("DMREventCallback ret = %d\n", ret);
    if (ret != 0) {
        delete dmrPlayer;
        pthread_mutex_unlock(&gDmrMutex);

        return -1;
    }
    if (!g_dmrManager->addDmrPlayer(dmrPlayer)) {
        delete dmrPlayer;
        pthread_mutex_unlock(&gDmrMutex);
        return -1;
    }

    DLNA_LOG("DMREventCallback 1\n");
    dmrPlayer->handleDmrMessage();

    while(1) {
        if (dmrPlayer->GetfinishStatus()) {
            struct json_object *obj = dmrPlayer->GetResultDrm();
            if (!obj) {
                g_dmrManager->removeDmrPlayer(dmrPlayer);
                delete dmrPlayer;
                pthread_mutex_unlock(&gDmrMutex);
                return -1;
            }
            strcpy((char*)arg, (char*)json_object_to_json_string(obj));
            printf("DMREventCallback result1 = %s\n", arg);
            json_object_put(obj);
            break;
        }
        usleep(1000);
    }

    DLNA_LOG("DMREventCallback result = %s\n", arg);

    g_dmrManager->removeDmrPlayer(dmrPlayer);
    delete dmrPlayer;
    pthread_mutex_unlock(&gDmrMutex);

    return 0;
}


DMRManager::DMRManager()
{
    m_currentPlayState = StopState;
    m_currentPlayUrl.clear();
}

DMRManager::~DMRManager()
{

}


bool
DMRManager::addDmrPlayer(DMRPlayer *drmPlayer)
{
    for(int i = 0; i < m_playerArray.size(); i++) {
        if (m_playerArray[i] == drmPlayer)
            return true;
    }
    m_playerArray.push_back(drmPlayer);

    return true;
}

bool
DMRManager::removeDmrPlayer(DMRPlayer *drmPlayer)
{
    std::vector<DMRPlayer*>::iterator it;
    for(it = m_playerArray.begin(); it != m_playerArray.end(); ++it) {
        if(drmPlayer == *it) {
            m_playerArray.erase(it);
            return true;
        }
    }

    return false;
}

int
DMRManager::SetEPGResult(std::string result, int type)
{
    std::vector<DMRPlayer*>::iterator it;
    DMRPlayer *dmr = NULL;
    DLNA_LOG("SetEPGResult m_playerArray.size = %d\n", m_playerArray.size());
    for(it = m_playerArray.begin(); it != m_playerArray.end(); ++it) {
        if ((*it) != NULL) {
            printf("(*it)->Type() = %d\n", (*it)->Type());
            if ((*it)->Type() == type) {
                dmr = *it;
                break;
            }
        }
    }
    if (dmr) {
        dmr->parseString(result);
        dmr->SetFinishStatus(true);
    }
    return 0;
}

void
DMRManager::DMRStart()
{
    Dmr_Start((DMREVRNT_CALLBACK)DMREventCallback);
}

int
DMRManager::DMRJsResult()
{
   a_Hippo_API_JseRegister("DLNA PUSH RESULT", NULL, dlna_push_result, IoctlContextType_eHWBase);
   a_Hippo_API_JseRegister("DLNA_PUSH_RESULT", NULL, dlna_push_result, IoctlContextType_eHWBase);
   a_Hippo_API_JseRegister("DLNA_PRODUCTINFO_RESULT", NULL, dlna_product_info, IoctlContextType_eHWBase);
   a_Hippo_API_JseRegister("DLNA PRODUCTINFO RESULT", NULL, dlna_product_info, IoctlContextType_eHWBase);
   a_Hippo_API_JseRegister("DLNA_ORDER_RESULT", NULL, dlna_order_result, IoctlContextType_eHWBase);
   a_Hippo_API_JseRegister("DLNA ORDER RESULT", NULL, dlna_order_result, IoctlContextType_eHWBase);
   a_Hippo_API_JseRegister("DLNA_PIC_ADDR", dlna_pic_addr, NULL, IoctlContextType_eHWBase);
   //a_Hippo_API_JseRegister("EVENT_DLNA_PIC_RELEASED", dlna_pic_released, IoctlContextType_eHWBase);

   return 0;
}

void
DMRManager::sendDmrMessage(int what, unsigned int delayMillis)
{
    if (delayMillis)
        sendEmptyMessageDelayed(what, delayMillis);
    else
        sendEmptyMessage(what);
}

void
DMRManager::removeDmrMessage(int what)
{
    removeMessages(what);
}

void
DMRManager::handleMessage(Message *msg)
{
    KeyDispatcherPolicy *keyPolocy = NULL;
    keyPolocy = keyDispatcher().getPolicy(EIS_IRKEY_DLNA_PUSH);
    std::string dmrUrl;

    switch (msg->what) {
        case DLNA_EVENT_DMR_PLAY:
            dmrUrl = keyPolocy->mKeyUrl;
            dmrUrl += "?Action=dlna_push&key=0x400&";
            dmrUrl += m_currentPlayUrl;
            DLNA_LOG("play url %s \n", dmrUrl.c_str());
            epgBrowserAgent().openUrl(dmrUrl.c_str());
            break;
        case DLNA_EVENT_DMR_ORDER:
            dmrUrl = keyPolocy->mKeyUrl;
            dmrUrl += "?Action=dlna_order&key=0x400&";
            dmrUrl += m_orderUrl;
            DLNA_LOG("play url %s \n", dmrUrl.c_str());
            epgBrowserAgent().openUrl(dmrUrl.c_str());
            break;
        case DLNA_EVENT_DMR_GETPRODUCTINFO:
            dmrUrl = keyPolocy->mKeyUrl;
            dmrUrl += "?Action=dlna_getproductinfo&key=0x400&";
            dmrUrl += m_productUrl;
            DLNA_LOG("play url %s \n", dmrUrl.c_str());
            epgBrowserAgent().openUrl(dmrUrl.c_str());
            break;
        case DLNA_EVENT_DMR_PLAY + 0x100:
            epgBrowserAgent().openUrl("FILE:////home/hybroad/share/webpage/showpic.html");
            break;
	case DLNA_EVENT_DMR_PLAY + 0x200:
            epgBrowserAgent().openUrl("FILE:////home/hybroad/share/webpage/LocalPlayer/menu.html");
            break;
        default:
            break;
    }
}

}

extern "C"
void DMRManagerCreate()
{
    Hippo::g_dmrManager = new Hippo::DMRManager();
    Hippo::g_dmrManager->DMRJsResult();
}

extern "C"
void DMRStart()
{
    if (Hippo::g_dmrManager) {
        Hippo::g_dmrManager->DMRStart();
    }
}
