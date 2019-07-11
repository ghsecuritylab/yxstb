#ifndef _NativeHandlerConfigC10_H_
#define _NativeHandlerConfigC10_H_

#include "NativeHandlerPublicC10.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerConfigC10 : public NativeHandlerPublicC10{
public:
    NativeHandlerConfigC10();
    ~NativeHandlerConfigC10();

    virtual bool handleMessage(Message *msg);

    virtual NativeHandler::State state() { return NativeHandler::Config; }
    virtual void onActive(void);
    virtual void onUnactive(void);

protected:
    virtual bool doOpenDisconnectPage() { return true; }

    virtual bool doOpenErrorPage() { return true; }
    virtual bool doMenu();
    virtual bool doNetworkReconnect(int, const char*) { return false; }
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerConfigC10_H_
