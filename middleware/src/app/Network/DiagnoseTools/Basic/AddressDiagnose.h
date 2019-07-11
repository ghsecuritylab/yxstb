#ifndef __AddressDiagnose__H_
#define __AddressDiagnose__H_

#include "NetworkDiagnose.h"
#include "DHCPSetting.h"
#include "PPPSetting.h"
#include "IPConflictSetting.h"

#ifdef __cplusplus

class NetworkInterface;

class AddressDiagnose : public NetworkDiagnose::DiagnoseProcess {
public:
    AddressDiagnose();
    ~AddressDiagnose(); 

    virtual int type() { return eTypeAddressDiagnose; }
    virtual int start(NetworkDiagnose* netdiag);
    virtual int stop();
    virtual int doResult(int result, int arg1, void* arg2 = 0);

    void attach(NetworkInterface* iface) { mIface = iface; }
    NetworkInterface* iface() { return mIface; }

private:
    void _InitSet();
    void _Restore();
    void _Backup();

private:
    NetworkInterface* mIface; 
    DHCPSetting mDHCPConfBak;
    PPPSetting mPPPConfBak;
    IPConflictSetting mConflictConfBak;
    int mErrorCode;
};
#endif

#endif
