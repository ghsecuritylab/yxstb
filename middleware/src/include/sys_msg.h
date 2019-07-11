#ifndef __SYS_MSG_H__
#define __SYS_MSG_H__

#include "config.h"

#include "Hippo_api.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*VideoMessageCallback)(int what, int arg1, int arg2);
void RegisterReceivedVideoMessageCallback(int owner, VideoMessageCallback callback);
typedef struct _VideoMessage_ {
    int nOwner;
    VideoMessageCallback nCallback;
}VideoMessage_t;

void sendMessageToKeyDispatcher(int what, int arg1, int arg2, unsigned int pDelayMillis);
void removeSpecifiedMessageFromKeyDispatcher(int what, int arg1, int arg2);
void sendMessageToNativeHandler(int what, int arg1, int arg2, unsigned int pDelayMillis);
void sendMessageToEPGBrowser(int what, int arg1, int arg2, unsigned int pDelayMillis);

void sys_msg_port_irkey(unsigned int msgno,int type, int stat);

void sys_msg_pressureenable_set(int enble);
unsigned int sys_msg_ir_cus_get(void);

#ifdef __cplusplus
}
#endif

#endif

