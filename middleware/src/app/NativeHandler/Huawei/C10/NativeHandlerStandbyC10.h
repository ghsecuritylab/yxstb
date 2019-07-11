#ifndef _NativeHandlerStandbyC10_H_
#define _NativeHandlerStandbyC10_H_

#include "NativeHandlerPublicC10.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerStandbyC10 : public NativeHandlerPublicC10 {
public:
    NativeHandlerStandbyC10();
    ~NativeHandlerStandbyC10();

    virtual void onActive(void);
    virtual void onUnactive(void);

    virtual bool handleMessage(Message *msg);

    virtual NativeHandler::State state() { return NativeHandler::Standby; }

protected:
    virtual bool doPowerOn();

};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerStandbyC10_H_
