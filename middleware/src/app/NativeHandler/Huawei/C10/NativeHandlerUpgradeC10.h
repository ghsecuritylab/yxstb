#ifndef _NativeHandlerUpgradeC10_H_
#define _NativeHandlerUpgradeC10_H_

#include "NativeHandlerPublicC10.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerUpgradeC10 : public NativeHandlerPublicC10 {
public:
    NativeHandlerUpgradeC10();
    ~NativeHandlerUpgradeC10();

    virtual bool handleMessage(Message *msg);

    virtual NativeHandler::State state() { return NativeHandler::Upgrade; }
    virtual void onActive(void);
    virtual void onUnactive(void);

protected:
#ifdef INCLUDE_TR069
    virtual bool onUpgradeTr069Request();
#endif
    virtual bool doConfig(Message *msg) { return true; }
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerUpgradeC10_H_
