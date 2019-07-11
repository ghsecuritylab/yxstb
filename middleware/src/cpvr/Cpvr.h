#ifndef _CPVR_H_
#define _CPVR_H_

#include "MessageHandler.h"
#include "UserManager.h"

#ifdef __cplusplus

#include <string>
#include <vector>

namespace Hippo {

class CpvrRes;
class UserManager;
class Cpvr : public MessageHandler {
public:

    Cpvr();
    ~Cpvr();

    int recordStart(char *scheduleID, int needReqRes);
    int recordStop(char *scheduleID);
    int recordClose(char *scheduleID);
    int recordEnd(char *scheduleID);
    int recordDelete(char *scheduleID);
    int recordReleaseResource(char *scheduleID);

    Message *timerCreate(int what, int arg, time_t tm);

protected:
    virtual void handleMessage(Message *msg);
    UserManager cpvrUserMng;
};

void *cpvr_timer_create(int what, int arg, time_t tm);

} // namespace Hippo

#endif // __cplusplus

#endif // _CPVR_H_

