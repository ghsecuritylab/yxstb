#ifndef _NativeHandlerUpgradeC20_H_
#define _NativeHandlerUpgradeC20_H_

#include "NativeHandlerPublicC20.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerUpgradeC20 : public NativeHandlerPublicC20 {
public:
    NativeHandlerUpgradeC20();
    ~NativeHandlerUpgradeC20();

    virtual void onActive();
    virtual void onUnactive();
    virtual bool handleMessage(Message *msg);

    virtual NativeHandler::State state() { return NativeHandler::Upgrade; }

protected:
    virtual bool onUpgradeTr069Request();
    virtual bool doConfig(Message *msg) { return true; }
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerUpgradeC20_H_
