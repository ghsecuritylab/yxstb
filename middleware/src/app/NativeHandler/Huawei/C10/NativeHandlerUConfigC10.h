#ifndef _NativeHandlerUConfigC10_H_
#define _NativeHandlerUConfigC10_H_

#include "NativeHandlerPublicC10.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerUConfigC10 : public NativeHandlerPublicC10 {
public:
    NativeHandlerUConfigC10();
    ~NativeHandlerUConfigC10();

    virtual bool handleMessage(Message *msg);

    virtual NativeHandler::State state() { return NativeHandler::UConfig; }
    virtual void onActive(void);
    virtual void onUnactive(void);
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerUConfigC10_H_
