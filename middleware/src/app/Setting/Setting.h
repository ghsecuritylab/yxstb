#ifndef _Setting_H_
#define _Setting_H_

#include <map>
#include <vector>
#include <string>

#include "mid/mid_mutex.h"

#include "SettingListener.h"

#ifdef __cplusplus

namespace Hippo {

class Setting {
public:
    Setting(std::string fileName);
    virtual ~Setting();

    virtual int load();
    virtual int restore(int flag);
    virtual int empty();
    virtual int recoverBakSeting() { return 0; }

    int add(const char* name, const char* value, bool encrypt = false, SettingListener* listeners = 0);
    int add(const char* name, const int value, bool encrypt = false, SettingListener* listeners = 0);
    int del(const char* name);

    int set(const char* name, const char* value);
    int get(const char* name, char* value, int valueLen);

    int set(const char* name, const int value);
    int get(const char* name, int* value);

    int registListeners(const char* name, ListenerFunction func);
    int unregistListeners(const char* name, ListenerFunction func);

protected:
    struct SettingItem {
            std::string m_name;
            std::string m_value;
            bool m_isEncrypt;
            std::string m_encyptedValue;
            SettingListener* m_listeners;
    };

    std::string m_fileName;
    std::vector<SettingItem*> m_itemsArray;
    std::map<std::string, SettingItem*> m_itemsMap;
    int dirty;
    mid_mutex_t m_mutex;
};

Setting* settingManager();

} // namespace Hippo

#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif
void settingManagerInit(void);
void settingManagerLoad(int reset);
void settingManagerSave();
#ifdef __cplusplus
}
#endif


#endif // _Setting_H_

