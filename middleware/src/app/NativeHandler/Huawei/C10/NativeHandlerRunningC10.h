#ifndef _NativeHandlerRunningC10_H_
#define _NativeHandlerRunningC10_H_

#include "NativeHandlerPublicC10.h"
#include "KeyDispatcher.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerRunningC10 : public NativeHandlerPublicC10 {
public:
    NativeHandlerRunningC10();
    ~NativeHandlerRunningC10();

    virtual bool handleMessage(Message *msg);

    virtual NativeHandler::State state() { return NativeHandler::Running; }

    virtual void onActive();
    virtual void onUnactive();

protected:
    virtual bool onPlayPause();
    virtual bool onLeft(Message *msg);
    virtual bool onRight(Message *msg);
#if defined (LIAONING_SD)
    virtual bool doSwitch();
#endif
#ifdef Liaoning
    virtual bool doBack();
#endif
    virtual bool doMenu();
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerRunningC10_H_
