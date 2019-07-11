#ifndef _NativeHandlerPublicC10_H_
#define _NativeHandlerPublicC10_H_

#include "NativeHandlerPublic.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerPublicC10 : public NativeHandlerPublic {

public:
    NativeHandlerPublicC10();
    ~NativeHandlerPublicC10();

    virtual bool handleMessage(Message *msg);

    virtual bool doPowerOff();

protected:
    virtual bool onPowerOff();
    virtual bool onDLNADialog();

    virtual bool onUpgrade();
    virtual bool doMenu();
    virtual bool doOpenTimeoutPage() { return true; } // United after error code does not need to have this error page open.
    // open local webpage message
    virtual bool doOpenStandbyPage();
    // specific IRkey message
    virtual bool doConfig(Message *msg);
    // NTP sync message
    virtual bool doNtpSyncOk();
    virtual bool doNtpSyncError();
    virtual bool doDNSServerNotfound();
	virtual bool doDNSResolveError();

#if defined(INCLUDE_LocalPlayer)
    virtual bool doDeskTop();
#endif
    virtual bool doRequestReboot();
#if defined(Gansu)
    virtual bool doUdiskUnzipErrReboot();
#endif
#if defined (LIAONING_SD)
    virtual bool doSwitch();
#endif
#if defined(Jiangsu)
    virtual bool doOpenSerialNumberPaga();
#endif
    virtual bool doOpenMaintenancePage();
    virtual bool doCloseMaintenancePage();
    virtual bool doDealIMEKey();
protected:
    int mNtpDnsError;
    int mDHCPErrorCount;
    int mPppoeErrorCount;
    static int mNtpsyncerror;
    int mKeyTimes;
    int mLastKeyDownPressed;
#if defined(Jiangsu)
    static int networkPhysicalDown;
#endif
    unsigned int timeFirsePressed;

};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerPublicC10_H_
