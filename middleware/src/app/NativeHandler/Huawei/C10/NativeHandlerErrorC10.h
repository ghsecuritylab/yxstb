#ifndef _NativeHandlerErrorC10_H_
#define _NativeHandlerErrorC10_H_

#include "NativeHandlerPublicC10.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerErrorC10 : public NativeHandlerPublicC10 {
public:
    NativeHandlerErrorC10();
    ~NativeHandlerErrorC10();

    virtual bool handleMessage(Message *msg);

    virtual NativeHandler::State state() { return NativeHandler::Error; }
	virtual void onActive();
    virtual void onUnactive(void);

protected:
    virtual bool doShortCut(Message *);
    virtual bool doMenu();
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerErrorC10_H_
