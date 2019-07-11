
#include "BrowserEventQueue.h"

#include "BrowserAssertions.h"

#include "MessageTypes.h"
#include "KeyDispatcher.h"

#include <stdio.h>


namespace Hippo {

BrowserEventQueue::BrowserEventQueue()
    :m_idRadom(0)
{
    pthread_mutex_init(&m_mutex, NULL);
}

BrowserEventQueue::~BrowserEventQueue()
{
    clear();
    pthread_mutex_destroy(&m_mutex);
}

int
BrowserEventQueue::add(const char* value, browserEventCallback callback)
{
    BrowserEventItem* item = new BrowserEventItem();
    if (!item)
        return -1;
    item->m_id = m_idRadom++;
    item->m_eventInfo = value;
    item->m_eventCallback = callback;
    
    if(m_idRadom >= 1024)
        m_idRadom = 0;

    pthread_mutex_lock(&m_mutex);
    m_itemsQueue.push(item);
    pthread_mutex_unlock(&m_mutex);
    return item->m_id;
}

int
BrowserEventQueue::get(int id, char* value, int valueLen)
{
    pthread_mutex_lock(&m_mutex);

    if (!m_itemsQueue.empty()) {
        BrowserEventItem* item = m_itemsQueue.front();
        snprintf(value, valueLen, item->m_eventInfo.c_str());
        m_itemsQueue.pop();
        delete item;
        pthread_mutex_unlock(&m_mutex);
        return 0;
    }
    pthread_mutex_unlock(&m_mutex);
    return -1;
}

int
BrowserEventQueue::check(int id)
{
    pthread_mutex_lock(&m_mutex);

    if (!m_itemsQueue.empty()) {
        BrowserEventItem* item = m_itemsQueue.front();
        if (id == item->m_id) {
            if(item->m_eventCallback)
                item->m_eventCallback(item->m_id);
            m_itemsQueue.pop();
            delete item;
            pthread_mutex_unlock(&m_mutex);
            return -1;
        }
    }
    pthread_mutex_unlock(&m_mutex);
    return 0;
}

int
BrowserEventQueue::clear()
{
    pthread_mutex_lock(&m_mutex);

    while (!m_itemsQueue.empty())
        m_itemsQueue.pop(); 

    pthread_mutex_unlock(&m_mutex);
    return 0;
}

static BrowserEventQueue* gBrowserEventQueue = NULL;

BrowserEventQueue &browserEventQueue()
{
    return *gBrowserEventQueue;
}

} // namespace Hippo

extern "C" {

void browserEventQueueCreate()
{
    Hippo::gBrowserEventQueue = new Hippo::BrowserEventQueue();
}

int browserEventSend(const char* info, browserEventCallback callback)
{
    int ret = Hippo::browserEventQueue().add(info, callback);

    BROWSER_LOG("browserEventSend info[%s], callback:%p, id=%d\n", info, callback, ret);
    sendMessageToKeyDispatcher(MessageType_Unknow, 0x300, ret, 0);
    return ret;
}

int browserEventGet(int id, char *jsonchar, int length)
{
    return Hippo::browserEventQueue().get(id, jsonchar, length);
}

int browserEventCheck(int id)
{
    printf("browserEventCheck id[%d]\n", id);
    BROWSER_LOG("browserEventCheck id[%d]\n", id);
    return Hippo::browserEventQueue().check(id);
}

int browserEventClear()
{
    BROWSER_LOG("browserEventClear\n");
    return Hippo::browserEventQueue().clear();
}

}
