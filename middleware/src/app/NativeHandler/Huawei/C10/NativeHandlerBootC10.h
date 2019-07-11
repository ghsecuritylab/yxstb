#ifndef _NativeHandlerBootC10_H_
#define _NativeHandlerBootC10_H_

#include "NativeHandlerPublicC10.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerBootC10 : public NativeHandlerPublicC10 {
public:
    NativeHandlerBootC10();
    ~NativeHandlerBootC10();

    virtual NativeHandler::State state() { return NativeHandler::Boot; }
    virtual void onActive();
    virtual void onUnactive();

protected:
    virtual bool onPowerOff();
    
    virtual bool doOpenDisconnectPage() { return true; }

    virtual bool doMenu();
    virtual bool doShortCut(Message *msg);

    virtual bool doNetworkReconnect(int force, const char* devname) { return true; }
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerBootC10_H_
