
#include "tr069_global.h"
#include "tr069_header.h"

#include <pthread.h>

static char g_opaque[DIGEST_PARAM_LEN] = {0};

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static GlobalEvent *g_eventQueue = NULL;
static int g_eventID = 0;

static GlobalObject *g_objectQueue = NULL;
static int g_objectSN = 0;

void tr069_global_setOpaque(char *opaque)
{
    pthread_mutex_lock(&g_mutex);
    strcpy(g_opaque, opaque);
    pthread_mutex_unlock(&g_mutex);
}

int tr069_global_getOpaque(char *opaque, int size)
{
    int len, ret = -1;

    pthread_mutex_lock(&g_mutex);

    len = strlen(g_opaque);
    if (size <= len)
        TR069ErrorOut("size = %d / %d\n", size, len);

    strcpy(opaque, g_opaque);
    ret = 0;

Err:
    pthread_mutex_unlock(&g_mutex);
    return ret;
}

int tr069_global_eventRegist(char *eventCode)
{
    GlobalEvent *event;
    int eventID;

    pthread_mutex_lock(&g_mutex);

    event = g_eventQueue;
    while (event) {
        if (0 == strcmp(eventCode, event->eventCode))
            break;
        event = event->next;
    }

    if (event) {
        eventID = event->eventID;
    } else {
        eventID = g_eventID;
        g_eventID++;

        event = (GlobalEvent *)IND_CALLOC(sizeof(GlobalEvent), 1);
        event->next = g_eventQueue;
        g_eventQueue = event;

        event->eventCode = IND_STRDUP(eventCode);
        event->eventID = eventID;
    }

    pthread_mutex_unlock(&g_mutex);
    TR069Printf("eventID = %d, eventCode = %s\n", eventID, eventCode);

    return eventID;
}

int tr069_global_eventParam(int eventID, char *paramName)
{
    int ret = -1;
    GlobalEvent *event;
    EventParam *param, *prev;

    pthread_mutex_lock(&g_mutex);

    if (!paramName)
        TR069ErrorOut("paramName is NULL!\n");

    event = g_eventQueue;
    while (event) {
        if (eventID == event->eventID)
            break;
        event = event->next;
    }
    if (!event)
        TR069ErrorOut("eventID = %d not regist!\n", eventID);

    prev = NULL;
    param = event->paramQueue;
    while (param) {
        if (!strcmp(paramName, param->name))
            break;
        prev = param;
        param = param->next;
    }

    if (!param) {
        param = (EventParam *)IND_CALLOC(sizeof(EventParam), 1);
        param->name = IND_STRDUP(paramName);
        if (prev)
            prev->next = param;
        else
            event->paramQueue = param;
    }

    ret = 0;
Err:
    pthread_mutex_unlock(&g_mutex);

    return ret;
}

int tr069_global_eventPost(int eventID)
{
    GlobalEvent *event;

    TR069Printf("eventID = %d\n", eventID);

    pthread_mutex_lock(&g_mutex);

    event = g_eventQueue;
    while (event) {
        if (eventID == event->eventID)
            break;
        event = event->next;
    }

    if (event)
        event->eventNum++;

    pthread_mutex_unlock(&g_mutex);

    if (event) {
        tr069_api_message(EVENTCODE_X_EXTEND, eventID, 0);
        return 0;
    }

    TR069Error("eventID = %d, invalid\n", eventID);
    return -1;
}

void tr069_global_eventSync(GlobalEvent *event)
{
    GlobalEvent *e;

    pthread_mutex_lock(&g_mutex);

    e = g_eventQueue;
    while (e) {
        if (e->eventID == event->eventID)
            break;
        e = e->next;
    }

    if (e) {
        EventParam *p, *param;

        while (e->paramQueue) {
            p = e->paramQueue;
            e->paramQueue = p->next;

            param = event->paramQueue;
            while (param) {
                if (!strcmp(p->name, param->name))
                    break;
                param = param->next;
            }
            if (param) {
                IND_FREE(p->name);
                IND_FREE(p);
            } else {
                p->next = event->paramQueue;
                event->paramQueue = p;
            }
        }

        if (!event->eventCode)
            event->eventCode = IND_STRDUP(e->eventCode);
        event->eventNum = e->eventNum;
    } else {
        TR069Error("eventID = %d invalid!\n", event->eventID);
    }

    pthread_mutex_unlock(&g_mutex);
}

void tr069_global_eventFinished(int eventID, int eventNum)
{
    GlobalEvent *event;

    pthread_mutex_lock(&g_mutex);

    event = g_eventQueue;
    while (event) {
        if (eventID == event->eventID)
            break;
        event = event->next;
    }
    if (event) {
        if (eventNum >= event->eventNum)
            event->eventNum = 0;
    }

    pthread_mutex_unlock(&g_mutex);
}

int tr069_global_eventPosting(int eventID)
{
    int posting = 0;
    GlobalEvent *event;

    pthread_mutex_lock(&g_mutex);

    event = g_eventQueue;
    while (event) {
        if (eventID == event->eventID)
            break;
        event = event->next;
    }
    if (event) {
        if (event->eventNum)
            posting = 1;
    }

    pthread_mutex_unlock(&g_mutex);

    return posting;
}


int tr069_global_addObject(char *name)
{
    int instance, sn;
    ObjectParam *param, *prev;
    GlobalObject *object;

    TR069Printf("AddObject %s\n", name);
    pthread_mutex_lock(&g_mutex);

    object = g_objectQueue;
    while (object) {
        if (!strcmp(name, object->name))
            break;
        object = object->next;
    }

    if (!object) {
        object = (GlobalObject *)IND_CALLOC(sizeof(GlobalObject), 1);
        object->name = IND_STRDUP(name);
        g_objectSN++;
        object->sn = g_objectSN;

        object->next = g_objectQueue;
        g_objectQueue = object;
    }
    sn = object->sn;

    prev = NULL;
    param = object->paramQueue;
    while (param) {
        prev = param;
        param = param->next;
    }

    param = (ObjectParam *)IND_CALLOC(sizeof(ObjectParam), 1);
    if (prev) {
        prev->next = param;
        instance = prev->instance + 1;
    } else {
        object->paramQueue = param;
        instance = 1;
    }
    param->instance = instance;

    pthread_mutex_unlock(&g_mutex);
    TR069Printf("sn = %d, instance = %d\n", sn, instance);

    tr069_api_message(EVENTCODE_Y_OBJECT_ADD, sn, instance);

    return instance;
}

int tr069_global_findObject(int sn, int instance, char *buf, int size)
{
    int find = 0;

    ObjectParam *param;
    GlobalObject *object;

    pthread_mutex_lock(&g_mutex);

    object = g_objectQueue;
    while (object) {
        if (sn == object->sn)
            break;
        object = object->next;
    }
    if (object) {
        snprintf(buf, size, "%s", object->name);

        param = object->paramQueue;
        while (param) {
            if (instance == param->instance)
                break;
            param = param->next;
        }
        if (param)
            find = 1;
    } else {
        TR069Warn("sn = %d invalid\n", sn);
    }

    pthread_mutex_unlock(&g_mutex);

    return find;
}

int tr069_global_deleteObject(char *name, int instance)
{
    int sn;
    GlobalObject *object;

    TR069Printf("DeleteObject %s, instance = %d\n", name, instance);

    sn = 0;

    pthread_mutex_lock(&g_mutex);

    object = g_objectQueue;
    while (object) {
        if (!strcmp(name, object->name))
            break;
        object = object->next;
    }
    if (object) {
        ObjectParam *param, *prev, *next;

        sn = object->sn;

        prev = NULL;
        param = object->paramQueue;
        while (param) {
            if (0 == instance || instance == param->instance) {
                next = param->next;
                if (prev)
                    prev->next = next;
                else
                    object->paramQueue = next;

                IND_FREE(param);
                param = next;
            } else {
                prev = param;
                param = param->next;
            }
        }
    } else {
        TR069Warn("sn = %d invalid\n", sn);
    }

    pthread_mutex_unlock(&g_mutex);

    TR069Printf("sn = %d\n", sn);
    if (sn > 0)
        tr069_api_message(EVENTCODE_Y_OBJECT_DELETE, sn, instance);

    return 0;
}

