#ifndef SettingListener_h
#define SettingListener_h

typedef int (*ListenerFunction)(const char*, const char*);

#ifdef __cplusplus

#include <vector>

namespace Hippo {
class SettingListener {
public:
    SettingListener();
    ~SettingListener();

    int call(const char* name, const char* value);
    int add(ListenerFunction func);
    int del(ListenerFunction func);
    bool empty() { return m_functions.empty(); };

private:
    std::vector<ListenerFunction> m_functions;
};
}
#endif //__cplusplus

#ifdef __cplusplus
extern "C" {
#endif

int SettingListenerRegist(const char* name, int type, ListenerFunction func);
int SettingListenerUnregist(const char* name, int type, ListenerFunction func);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //SettingListener_h
