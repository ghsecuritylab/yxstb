#ifndef _BrowserEventQueue_H_
#define _BrowserEventQueue_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef void (*browserEventCallback)(int id);

void browserEventQueueCreate();

int browserEventSend(const char* info, browserEventCallback callback);
int browserEventGet(int id, char *jsonchar, int length);
int browserEventCheck(int id);
int browserEventClear();

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus

#include <queue>
#include <string>
#include <pthread.h>

namespace Hippo {

class BrowserEventQueue {
public:
    BrowserEventQueue();
    ~BrowserEventQueue();

    int add(const char* value, browserEventCallback callback);
    int get(int id, char* value, int valueLen);
    int check(int id);
    int clear();

protected:
    struct BrowserEventItem {
        int m_id;
        std::string m_eventInfo;
        browserEventCallback m_eventCallback;
    };

    std::queue<BrowserEventItem*> m_itemsQueue;
    int m_idRadom;
    pthread_mutex_t m_mutex;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserEventQueue_H_
