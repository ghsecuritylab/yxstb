#ifndef __TRUNK_EC2108_C27_IPSTB_SRC_APP_NATIVEHANDLER_HUAWEI_C20_NATIVEHANDLERSTANDBYC20_H_
#define __TRUNK_EC2108_C27_IPSTB_SRC_APP_NATIVEHANDLER_HUAWEI_C20_NATIVEHANDLERSTANDBYC20_H_

#include "NativeHandlerPublicC20.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerStandbyC20 : public NativeHandlerPublicC20 {
public:
    NativeHandlerStandbyC20();
    ~NativeHandlerStandbyC20();

    virtual void onActive();
    virtual void onUnactive();
    virtual bool handleMessage(Message *msg);

    virtual NativeHandler::State state() { return NativeHandler::Standby; }

protected:
    virtual bool doPowerOn();
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerStandbyC10_H_
