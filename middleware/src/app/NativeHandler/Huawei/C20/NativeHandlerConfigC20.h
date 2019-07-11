#ifndef __TRUNK_EC2108_C27_IPSTB_SRC_APP_NATIVEHANDLER_HUAWEI_C20_NATIVEHANDLERCONFIGC20_H_
#define __TRUNK_EC2108_C27_IPSTB_SRC_APP_NATIVEHANDLER_HUAWEI_C20_NATIVEHANDLERCONFIGC20_H_

#include "NativeHandlerPublicC20.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerConfigC20 : public NativeHandlerPublicC20 {
public:
    NativeHandlerConfigC20();
    ~NativeHandlerConfigC20();

    virtual void onActive();
    virtual void onUnactive();
    virtual bool handleMessage(Message *msg);

    virtual NativeHandler::State state() { return NativeHandler::Config; }

protected:
    virtual bool doOpenDisconnectPage() { return true; }

    virtual bool doOpenErrorPage() { return true; }
};

} // namespace Hippo

#endif // __cplusplus

#endif // __TRUNK_EC2108_C27_IPSTB_SRC_APP_NATIVEHANDLER_HUAWEI_C20_NATIVEHANDLERCONFIGC20_H
