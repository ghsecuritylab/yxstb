#ifndef __TRUNK_EC2108_C27_IPSTB_SRC_APP_NATIVEHANDLER_HUAWEI_C20_NATIVEHANDLERERRORC20_H_
#define __TRUNK_EC2108_C27_IPSTB_SRC_APP_NATIVEHANDLER_HUAWEI_C20_NATIVEHANDLERERRORC20_H_

#include "NativeHandlerPublicC20.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerErrorC20 : public NativeHandlerPublicC20 {
public:
    NativeHandlerErrorC20();
    ~NativeHandlerErrorC20();

    virtual void onActive();
    virtual void onUnactive();
    virtual bool handleMessage(Message *msg);

    virtual NativeHandler::State state() { return NativeHandler::Error; }

protected:
    virtual bool doShortCut(Message *);
    
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerErrorC20_H_
