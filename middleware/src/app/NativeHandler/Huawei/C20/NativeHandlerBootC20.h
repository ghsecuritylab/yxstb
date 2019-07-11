#ifndef __TRUNK_EC2108_C27_IPSTB_SRC_APP_NATIVEHANDLER_HUAWEI_C20_NATIVEHANDLERBOOTC20_H_
#define __TRUNK_EC2108_C27_IPSTB_SRC_APP_NATIVEHANDLER_HUAWEI_C20_NATIVEHANDLERBOOTC20_H_

#include "NativeHandlerPublicC20.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerBootC20 : public NativeHandlerPublicC20 {
public:
    NativeHandlerBootC20();
    ~NativeHandlerBootC20();

    virtual NativeHandler::State state() { return NativeHandler::Boot; }
    virtual void onActive();
    virtual void onUnactive();
protected:
    virtual bool onPowerOff() { return true; }
    virtual bool doOpenDisconnectPage() { return true; }

    virtual bool doShortCut(Message *msg);
    
    virtual bool doConfig(Message *msg);

};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerBootC10_H_
