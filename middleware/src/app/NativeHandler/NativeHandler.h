#ifndef _NativeHandler_H_
#define _NativeHandler_H_

#include "MessageHandler.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandler : public MessageHandler {
public:
    virtual void handleMessage(Message *msg);

    enum State {
        Boot = 0,
        Config,
        Running,
        Standby,
        Upgrade,
        UConfig,
        Local,
        Unified,
        NetworkDiagnose,
        LittleSystem,
    	 Error,
    	 Relink,
    	 Recovery
    };

    static int registerStateHandler(State, Callback *);

    int setState(State);
    State getState();
    State getOldState();  
private:
    State oldState;
};

NativeHandler &defNativeHandler();

} // namespace Hippo

#endif // __cplusplus


#ifdef __cplusplus
extern "C" {
#endif

void defNativeHandlerCreate();

int NativeHandlerSetState(int state);
int NativeHandlerGetState();
int NativeHandlerGetOldState();

int NativeHandlerInputMessage(int what, int arg1, int arg2, void *);

void sys_key_editfocus_set(int pFlage);
void sendMessageToNativeHandler(int what, int arg1, int arg2, unsigned int pDelayMillis);
void StatisticHTTPFailInfoGet(char* buf, int size);
int StatisticHTTPFailNumbersGet();

#ifdef __cplusplus
}
#endif

#endif // _NativeHandler_H_
