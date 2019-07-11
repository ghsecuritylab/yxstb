#ifndef _NativeHandlerUConfigC20_H_
#define _NativeHandlerUConfigC20_H_

#include "NativeHandlerPublicC20.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerUConfigC20 : public NativeHandlerPublicC20 {
public:
    NativeHandlerUConfigC20();
    ~NativeHandlerUConfigC20();

    virtual void onActive();
    virtual void onUnactive();
    virtual bool handleMessage(Message *msg);

	virtual bool doUSBUninsert(Message *msg);
    virtual NativeHandler::State state() { return NativeHandler::UConfig; }
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerUConfigC20_H_
