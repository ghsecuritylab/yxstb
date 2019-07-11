#ifndef _NativeHandlerRecovery_H_
#define _NativeHandlerRecovery_H_

#include "NativeHandlerPublic.h"

#ifdef __cplusplus

namespace Hippo {
class NativeHandlerRecovery : public NativeHandlerPublic {
public:
     NativeHandlerRecovery();
    ~NativeHandlerRecovery();
    
    virtual void onActive();
    virtual void onUnactive();
    virtual bool handleMessage(Message *msg);
    virtual NativeHandler::State state() { return NativeHandler::Recovery; }
protected:
    virtual bool doMenu() { return true; }
    virtual bool doConfig(Message *msg) { return true; }
	virtual bool doNtpSyncOk() { return true; }
	virtual bool doNtpSyncError() { return true; }
	virtual bool doDNSServerNotfound() { return true; }
	virtual bool doDNSResolveError() { return true; }
	virtual bool doOpenStandbyPage() { return true; }
	virtual bool doOpenTimeoutPage() {return true; }
private:
    void startNetWork();
    void onEmergency();
    int checkPressKey();
private:    
    bool mIsHandleKey;
    int mPressOKCount;
};

}

#endif // __cplusplus

#endif //_NativeHandlerLittleSystem_H_
