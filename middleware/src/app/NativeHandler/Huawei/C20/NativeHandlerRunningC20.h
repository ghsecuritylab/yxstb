#ifndef __TRUNK_EC2108_C27_IPSTB_SRC_APP_NATIVEHANDLER_HUAWEI_C20_NATIVEHANDLERRUNNINGC20_H_
#define __TRUNK_EC2108_C27_IPSTB_SRC_APP_NATIVEHANDLER_HUAWEI_C20_NATIVEHANDLERRUNNINGC20_H_

#include "NativeHandlerPublicC20.h"
#include "KeyDispatcher.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerRunningC20 : public NativeHandlerPublicC20 {
    
public:
    NativeHandlerRunningC20();
    ~NativeHandlerRunningC20();

    virtual void onActive();
    virtual void onUnactive();
    virtual bool handleMessage(Message *msg);

    virtual NativeHandler::State state() { return NativeHandler::Running; }
    /* 启用键值表功能 */
    virtual void onActive() { keyDispatcher().setEnabled(true); }
    /* 关闭键值表功能 */
    virtual void onUnactive() { keyDispatcher().setEnabled(false); }

};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerRunningC10_H_
