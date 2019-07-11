
#include "KeyDispatcher.h"

#include "Message.h"
#include "MessageTypes.h"
#include "NativeHandler.h"
#include "BrowserAgent.h"
#include "SystemManager.h"
#include "Assertions.h"
#include "MessageValueMaintenancePage.h"

#include "browser_event.h"
#include "customer.h"
#include "auth.h"

namespace Hippo {

KeyDispatcherPolicy policyArray[] = {
{1, EIS_IRKEY_MENU, "KEY_PORTAL",   0, KeyDispatcherPolicy::NativeFirst, "", 0},
{1, EIS_IRKEY_BTV, "KEY_TV_MENU", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_VOD, "KEY_VOD_MENU", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_TVOD, "KEY_TVOD", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_NVOD, "KEY_NVOD", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_RED, "KEY_RED", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_GREEN, "KEY_GREEN", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_YELLOW, "KEY_YELLOW", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_BLUE, "KEY_BLUE", 0, KeyDispatcherPolicy::JSFirst, "", 0},
#ifdef HUAWEI_C10
{1, EIS_IRKEY_POWER, "KEY_POWER", 0, KeyDispatcherPolicy::NativeFirst, "", 0},
#else
{1, EIS_IRKEY_POWER, "KEY_POWER", 0, KeyDispatcherPolicy::JSFirst, "", 0},
#endif
#ifdef HUBEI_HD
{1, EIS_IRKEY_VOLUME_UP, "KEY_VOL_UP", 0, KeyDispatcherPolicy::NativeFirst, "", 0},
{1, EIS_IRKEY_VOLUME_DOWN, "KEY_VOL_DOWN", 0, KeyDispatcherPolicy::NativeFirst, "", 0},
{1, EIS_IRKEY_VOLUME_MUTE, "KEY_MUTE", 0, KeyDispatcherPolicy::NativeFirst, "", 0},
{1, EIS_IRKEY_AUDIO, "KEY_AUDIO", 0, KeyDispatcherPolicy::NativeFirst, "", 0},
{1, EIS_IRKEY_AUDIO_MODE, "KEY_AUDIO_MODE", 0, KeyDispatcherPolicy::NativeFirst, "", 0},
#else
#if defined(Gansu)
{1, EIS_IRKEY_VOLUME_UP, "KEY_VOL_UP", 0, KeyDispatcherPolicy::NativeFirst, "", 0},
{1, EIS_IRKEY_VOLUME_DOWN, "KEY_VOL_DOWN", 0, KeyDispatcherPolicy::NativeFirst, "", 0},
{1, EIS_IRKEY_AUDIO_MODE, "KEY_AUDIO_MODE", 0, KeyDispatcherPolicy::NativeFirst, "", 0},
#else
{1, EIS_IRKEY_VOLUME_UP, "KEY_VOL_UP", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_VOLUME_DOWN, "KEY_VOL_DOWN", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_AUDIO_MODE, "KEY_AUDIO_MODE", 0, KeyDispatcherPolicy::JSFirst, "", 0},
#endif
{1, EIS_IRKEY_VOLUME_MUTE, "KEY_MUTE", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_AUDIO, "KEY_AUDIO", 0, KeyDispatcherPolicy::JSFirst, "", 0},
#endif
{1, EIS_IRKEY_HELP, "KEY_HELP", 0, KeyDispatcherPolicy::JSFirst, "", 0},
#ifdef Liaoning
{1, EIS_IRKEY_BACK, "KEY_BACK", 0, KeyDispatcherPolicy::NativeFirst, "", 0},
#else
{1, EIS_IRKEY_BACK, "KEY_BACK", 0, KeyDispatcherPolicy::JSFirst, "", 0},
#endif
{1, EIS_IRKEY_UP, "KEY_UP", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_DOWN, "KEY_DOWN", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_LEFT, "KEY_LEFT", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_RIGHT,"KEY_RIGHT", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_NUM0, "KEY_0", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_NUM1, "KEY_1", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_NUM2, "KEY_2", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_NUM3, "KEY_3", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_NUM4, "KEY_4", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_NUM5, "KEY_5", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_NUM6, "KEY_6", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_NUM7, "KEY_7", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_NUM8, "KEY_8", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_NUM9, "KEY_9", 0, KeyDispatcherPolicy::JSFirst, "", 0},
#ifdef HUAWEI_C20
{1, EIS_IRKEY_FPANELPOWER, "KEY_FPANELPOWER", 0, KeyDispatcherPolicy::NativeFirst, "", 0},
#endif
{1, EIS_IRKEY_PLAY, "KEY_PAUSE_PLAY", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_STOP, "KEY_MEDIA_STOP", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_PAGE_UP, "KEY_PAGE_UP", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_PAGE_DOWN, "KEY_PAGE_DOWN", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_FASTFORWARD, "KEY_FAST_FORWARD", 0, KeyDispatcherPolicy::JSFirst, "", 0},
{1, EIS_IRKEY_REWIND, "KEY_FAST_BACK", 0, KeyDispatcherPolicy::JSFirst, "", 0},
#ifdef INCLUDE_DMR
{1, EIS_IRKEY_DLNA_PUSH, "EVENT_DLNA_PUSH", 0, KeyDispatcherPolicy::NativeFirst, "", 0},
{1, EIS_IRKEY_URG_REG, "EVNET_URG_REG", 0, KeyDispatcherPolicy::NativeFirst, "", 0},
#endif
};
#if 0
static int inHeap(KeyDispatcherPolicy *policy)
{
    if ((policy >= &policyArray[0]) && (policy < &policyArray[sizeof(policyArray)/sizeof(policyArray[0])]))
        return 0;
    else
        return 1;
}
#endif
KeyDispatcher::KeyDispatcher()
	: mEnabled(false)
	, mKeyDownPressed(0)
{
    int tCount, i;

    tCount = sizeof(policyArray) / sizeof(policyArray[0]);
    for(i = 0; i< tCount; i++)
        addPolicy(&policyArray[i]);
}

KeyDispatcher::~KeyDispatcher()
{
}

bool
KeyDispatcher::setEnabled(bool enabled)
{
    bool old = mEnabled;
    mEnabled = enabled;
    return old;
}

int
KeyDispatcher::addPolicy(KeyDispatcherPolicy *policy)
{
    if (policy == 0)
        return -1;

    KeyDispatcherPolicy *oldPolicy = getPolicy(policy->mKeyValue);
    if (oldPolicy) {
        if (oldPolicy->mEditPower > policy->mEditPower)
            return -1;
        mKeyValueMap.erase(oldPolicy->mKeyValue);
        mKeyNameMap.erase(oldPolicy->mKeyName);
    }

    mKeyValueMap.insert(std::make_pair(policy->mKeyValue, policy));
    mKeyNameMap.insert(std::make_pair(policy->mKeyName, policy));
    return 0;
}

int
KeyDispatcher::deletePolicy(KeyDispatcherPolicy *policy)
{
    if (policy == 0)
        return -1;

    mKeyValueMap.erase(policy->mKeyValue);
    mKeyNameMap.erase(policy->mKeyName);
    return 0;
}

int
KeyDispatcher::deletePolicy(int keyValue)
{
    std::map<int, KeyDispatcherPolicy *>::iterator it = mKeyValueMap.find(keyValue);

    if (it != mKeyValueMap.end()) {
        return deletePolicy(it->second);
    }
    else
        return -1;
}

int
KeyDispatcher::deletePolicyByName(char *keyName)
{
    std::map<std::string, KeyDispatcherPolicy *>::iterator it = mKeyNameMap.find(keyName);

    if (it != mKeyNameMap.end()) {
        return deletePolicy(it->second);
    }
    else
        return -1;
}

KeyDispatcherPolicy *
KeyDispatcher::getPolicy(int keyValue)
{
    std::map<int, KeyDispatcherPolicy *>::iterator it = mKeyValueMap.find(keyValue);

    if (it != mKeyValueMap.end())
        return it->second;
    else
        return 0;
}

KeyDispatcherPolicy *
KeyDispatcher::getPolicyByName(char *keyName)
{
    std::map<std::string, KeyDispatcherPolicy *>::iterator it = mKeyNameMap.find(keyName);

    if (it != mKeyNameMap.end())
        return it->second;
    else
        return 0;
}

int
KeyDispatcher::setPolicy(int keyValue, int keyType, int keyPolicy, char *keyUrl, int editPowser)
{
    std::map<int, KeyDispatcherPolicy *>::iterator it = mKeyValueMap.find(keyValue);

    LogUserOperDebug("KeyValue(0x%x), keyType(%d) keyPolicy(%d) keyUrl(%s)\n", keyValue, keyType, keyPolicy, keyUrl);
    if(keyUrl == NULL)
        return -1;

    if (it != mKeyValueMap.end()) {
        if (it->second->mEditPower > editPowser)
            return -1;

        it->second->mKeyType = keyType;
        it->second->mKeyPolicy = keyPolicy;
        if(keyPolicy == 2) {
            if(strstr(keyUrl, "http://"))
                it->second->mKeyUrl = keyUrl;
            else {
                std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();
                if (!url.empty()) {
                    if (url.substr(url.length() - 1, 1).compare(std::string("/")) == 0) {
                        if ('/' == keyUrl[0])
                            url.erase(url.length() -1, 1);
                    }
                    url += std::string(keyUrl);
                    it->second->mKeyUrl = url;
                }
            }
        }
        LogUserOperDebug("Set keyUrl(%s)\n", it->second->mKeyUrl.c_str());
        it->second->mEditPower = editPowser;
        return 0;
    }
    else
        return -1;
}

int
KeyDispatcher::setPolicyByName(char *keyName, int keyType, int keyPolicy, char *keyUrl, int editPowser)
{
    std::map<std::string, KeyDispatcherPolicy *>::iterator it = mKeyNameMap.find(keyName);

    LogUserOperDebug("keyName(%s), keyType(%d) keyPolicy(%d) keyUrl(%s)\n", keyName, keyType, keyPolicy, keyUrl);
    if(keyName == NULL || keyUrl == NULL)
        return -1;

    if (it != mKeyNameMap.end()) {
        if (it->second->mEditPower > editPowser)
            return -1;

        it->second->mKeyType = keyType;
        it->second->mKeyPolicy = keyPolicy;
        if((keyPolicy == 2) && (strlen(keyUrl) != 0)) {
            if(strstr(keyUrl, "http://"))
                it->second->mKeyUrl = keyUrl;
            else {
                std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();
                if (!url.empty()) {
                    if (url.substr(url.length() - 1, 1).compare(std::string("/")) == 0) {
                        if ('/' == keyUrl[0])
                            url.erase(url.length() -1, 1);
                    }
                    url += std::string(keyUrl);
                    it->second->mKeyUrl = url;
                }
            }
        }
        LogUserOperDebug("Set keyUrl(%s)\n", it->second->mKeyUrl.c_str());
        it->second->mEditPower = editPowser;
        return 0;
    }
    return -1;
}

int
KeyDispatcher::enablePolicy(int keyValue)
{
    std::map<int, KeyDispatcherPolicy *>::iterator it = mKeyValueMap.find(keyValue);

    if (it != mKeyValueMap.end()) {
        it->second->mEnable = 1;
        return 0;
    }
    else
        return -1;
}

int
KeyDispatcher::enablePolicyByName(char *keyName)
{
    std::map<std::string, KeyDispatcherPolicy *>::iterator it = mKeyNameMap.find(keyName);

    if (it != mKeyNameMap.end()) {
        it->second->mEnable = 1;
        return 0;
    }
    else
        return -1;
}

int
KeyDispatcher::disablePolicy(int keyValue)
{
    std::map<int, KeyDispatcherPolicy *>::iterator it = mKeyValueMap.find(keyValue);

    if (it != mKeyValueMap.end()) {
        it->second->mEnable = 0;
        return 0;
    }
    else
        return -1;
}

int
KeyDispatcher::disablePolicyByName(char *keyName)
{
    std::map<std::string, KeyDispatcherPolicy *>::iterator it = mKeyNameMap.find(keyName);

    if (it != mKeyNameMap.end()) {
        it->second->mEnable = 1;
        return 0;
    }
    else
        return -1;
}

void
KeyDispatcher::handleMessage(Message *msg)
{
    if (MessageType_KeyDown == msg->what)
        mKeyDownPressed++;

#if defined(Gansu)
    if (msg->arg1 == 0x237) {//EIS_IRKEY_INFO globalKey could not set
        setEnabled(1);
        msg->arg1 = 0x343;
    }
#endif


    if (mEnabled) {
        std::map<int, KeyDispatcherPolicy *>::iterator it = mKeyValueMap.find(msg->arg1);

        if (it != mKeyValueMap.end()) {
            LogUserOperDebug("KeyValue(0x%x) keyPolicy(%d)\n", msg->arg1, it->second->mKeyPolicy);
            if (it->second->mKeyPolicy == KeyDispatcherPolicy::NativeFirst) {
                defNativeHandler().dispatchMessage(msg);
            }
            else if (it->second->mKeyPolicy == KeyDispatcherPolicy::JSFirst) {
                epgBrowserAgent().dispatchMessage(msg);
            }
            else if (it->second->mKeyPolicy == KeyDispatcherPolicy::OpenUrl) {
                /* 打开新网页前，关闭所有播放 */
                LogUserOperDebug("OpenUrl(%s)\n", it->second->mKeyUrl.c_str());
				if (!strlen(it->second->mKeyUrl.c_str()))
                    return;
                systemManager().destoryAllPlayer();
                epgBrowserAgent().openUrl(it->second->mKeyUrl.c_str());
            }
            else {
                /* Surprise! */
            }
            return;
        }
    }

    defNativeHandler().dispatchMessage(msg);
    return;
}


static KeyDispatcher *gKeyDispatcher = NULL;

KeyDispatcher &keyDispatcher()
{
    return *gKeyDispatcher;
}

} // namespace Hippo


extern "C" void
keyDispatcherCreate()
{
    Hippo::gKeyDispatcher = new Hippo::KeyDispatcher();
}


extern "C" void
sendMessageToKeyDispatcher(int what, int arg1, int arg2, uint32_t pDelayMillis)
{
    Hippo::Message *msg = Hippo::keyDispatcher().obtainMessage(what, arg1, arg2);

    if(pDelayMillis)
        Hippo::keyDispatcher().sendMessageDelayed(msg, pDelayMillis);
    else
        Hippo::keyDispatcher().sendMessage(msg);
}

extern "C" void
removeSpecifiedMessageFromKeyDispatcher(int what, int arg1, int arg2)
{
    Hippo::keyDispatcher().removeMessages(what, arg1, arg2);
}
