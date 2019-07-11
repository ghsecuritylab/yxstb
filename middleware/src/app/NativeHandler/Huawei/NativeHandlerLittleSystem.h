#ifndef _NativeHandlerLittleSystem_H_
#define _NativeHandlerLittleSystem_H_

#include "NativeHandlerPublic.h"

#ifdef __cplusplus

namespace Hippo {
class NativeHandlerLittleSystem : public NativeHandlerPublic {
public:
    
    NativeHandlerLittleSystem();
    ~NativeHandlerLittleSystem();
    
    virtual void onActive();
    virtual void onUnactive();
    virtual bool handleMessage(Message *msg);
    virtual NativeHandler::State state() { return NativeHandler::LittleSystem; }
protected:
    virtual bool doMenu() { return true; }
    virtual bool doOpenTimeoutPage() { return true; }
    virtual bool doConfig(Message *msg) { return true; }
    virtual bool doManualUpgrade() { return true; }
	virtual bool doNtpSyncOk() { return true; }
	virtual bool doNtpSyncError() { return true; }
	virtual bool doDNSServerNotfound() { return true; }
	virtual bool doDNSResolveError() { return true; }
	virtual bool doOpenStandbyPage() { return true; }
private:
    void startNetWork();
    void checkAppVersion();  
    void runSystem(int);
    void onEmergency();
    void InitApp();	
private:
    int mUpgradeMode;
    bool mIsHandleKey;
    int mPressOKCount;
                  
        
    
};    
    
}

#endif // __cplusplus

#endif //_NativeHandlerLittleSystem_H_
