#ifndef _NativeHandlerRelinkC10_H_
#define _NativeHandlerRelinkC10_H_

#include "NativeHandlerPublicC10.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerRelinkC10 : public NativeHandlerPublicC10 {
public:
    NativeHandlerRelinkC10() {}
    ~NativeHandlerRelinkC10() {}

    virtual bool handleMessage(Message *msg);

    virtual NativeHandler::State state() { return NativeHandler::Relink; }
    virtual void onActive(void);
    virtual void onUnactive(void);
};

} // namespace Hippo

#endif // __cplusplus

#endif


