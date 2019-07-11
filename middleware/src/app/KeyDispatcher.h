#ifndef _KeyDispatcher_H_
#define _KeyDispatcher_H_

#include "MessageHandler.h"

#ifdef __cplusplus

#include <map>
#include <string>

namespace Hippo {

class KeyDispatcherPolicy {
public:
    enum Policy {
        JSFirst,
        NativeFirst,
        OpenUrl
    };
    short mEnable;
    short mKeyValue;
    std::string mKeyName;
    int mKeyType;
    int mKeyPolicy;
    std::string mKeyUrl;
    int mEditPower;
};

class KeyDispatcher : public MessageHandler {
public:
    KeyDispatcher();
    ~KeyDispatcher();

    bool isEnabled() { return mEnabled; }
    bool setEnabled(bool);

    int addPolicy(KeyDispatcherPolicy *policy);
    int deletePolicy(KeyDispatcherPolicy *policy);
    int deletePolicy(int keyValue);
    int deletePolicyByName(char *keyName);

    KeyDispatcherPolicy *getPolicy(int keyValue);
    KeyDispatcherPolicy *getPolicyByName(char *keyName);

    int setPolicy(int keyValue, int keyType, int keyPolicy, char *keyUrl, int editPowser);
    int setPolicyByName(char *keyName, int keyType, int keyPolicy, char *keyUrl, int editPowser);

    int enablePolicy(int keyValue);
    int enablePolicyByName(char *keyName);
    int disablePolicy(int keyValue);
    int disablePolicyByName(char *keyName);
    int totalPressKeyTimes() { return mKeyDownPressed; }

    virtual void handleMessage(Message *msg);

private:
    std::map<int, KeyDispatcherPolicy *> mKeyValueMap;
    std::map<std::string, KeyDispatcherPolicy *> mKeyNameMap;
    bool mEnabled;
    int mKeyDownPressed;
};


KeyDispatcher &keyDispatcher();

} // namespace Hippo

#endif // __cplusplus


#ifdef __cplusplus
extern "C" {
#endif

void keyDispatcherCreate();

void sendMessageToKeyDispatcher(int what, int arg1, int arg2, uint32_t pDelayMillis);
void removeSpecifiedMessageFromKeyDispatcher(int what, int arg1, int arg2);

#ifdef __cplusplus
}
#endif

#endif // _KeyDispatcher_H_
