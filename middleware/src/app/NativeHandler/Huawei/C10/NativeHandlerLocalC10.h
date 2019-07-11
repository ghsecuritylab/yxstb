#ifndef _NativeHandlerLocalC10_H_
#define _NativeHandlerLocalC10_H_

#include "NativeHandlerPublicC10.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerLocalC10 : public NativeHandlerPublicC10{
public:
    NativeHandlerLocalC10();
    ~NativeHandlerLocalC10();

    virtual bool handleMessage(Message *msg);

    virtual NativeHandler::State state() { return NativeHandler::Local; }
    virtual void onActive(void);
    virtual void onUnactive(void);

protected:
    virtual bool doOpenDisconnectPage() { return true; }

    virtual bool doOpenErrorPage() { return true; }
    virtual bool doMenu();
    virtual bool doShortCut(Message *msg);
    virtual bool doNtpSyncError() { return true; }
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerConfigC10_H_
