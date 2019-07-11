#ifndef __TRUNK_EC2108_C27_IPSTB_SRC_APP_NATIVEHANDLER_HUAWEI_C20_NATIVEHANDLERPUBLICC20_H_
#define __TRUNK_EC2108_C27_IPSTB_SRC_APP_NATIVEHANDLER_HUAWEI_C20_NATIVEHANDLERPUBLICC20_H_

#include "NativeHandlerPublic.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerPublicC20 : public NativeHandlerPublic {
public:
    NativeHandlerPublicC20();
    ~NativeHandlerPublicC20();

    virtual bool handleMessage(Message *msg);
    
protected:
    virtual bool doPowerOff();
    virtual bool doPowerOffDeep();
    virtual bool doOpenStandbyPage();
    virtual bool doMenu();
    // open local webpage message
    //virtual bool doLocalPage(Message *msg) { return false; }
    virtual bool doOpenTimeoutPage() { return false; } // ����������ص�UserValid�ֶ�Ϊfalse���ϱ����ⰴ��0x9901��EPG�������������⴦��
    // specific IRkey message
    virtual bool doConfig(Message *msg) ;

    // NTP sync message
    virtual bool doNtpSyncOk();
    virtual bool doNtpSyncError();
    virtual bool doDNSServerNotfound(); 
    virtual bool doDNSResolveError();
    virtual bool doLocalTS(Message *msg);

private:
    static int mIpConflictFlag;
    static int mErrorCodeOfNTP;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerPublicC10_H_
