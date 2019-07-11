
#ifndef __TR069_GLOBAL_H__
#define __TR069_GLOBAL_H__

typedef struct tagEventParam EventParam;
struct tagEventParam {
    EventParam *next;
    char *name;
};

typedef struct tagGlobalEvent GlobalEvent;
struct tagGlobalEvent {
    GlobalEvent *next;

    EventParam *paramQueue;

    int eventID;
    int eventNum;
    char *eventCode;
};

typedef struct tagObjectParam ObjectParam;
struct tagObjectParam {
    ObjectParam *next;
    int instance;
};

typedef struct tagGlobalObject GlobalObject;
struct tagGlobalObject {
    GlobalObject *next;

    ObjectParam *paramQueue;

    int sn;
    char *name;
};

/*
    tr069_api_message是tr069模块内部函数，不能放在tr069_api.h里
 */
void tr069_api_message(unsigned int id, int arg0, int arg1);

void tr069_global_setOpaque(char *opaque);
int tr069_global_getOpaque(char *opaque, int size);

int tr069_global_eventRegist(char *eventCode);
int tr069_global_eventParam(int eventID, char *paramName);
int tr069_global_eventPost(int eventID);

void tr069_global_eventSync(GlobalEvent *event);
void tr069_global_eventFinished(int eventID, int eventNum);
int tr069_global_eventPosting(int eventID);

int tr069_global_addObject(char *name);
int tr069_global_findObject(int sn, int instance, char *buf, int size);
int tr069_global_deleteObject(char *name, int instance);

#endif//__TR069_GLOBAL_H__

