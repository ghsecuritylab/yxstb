#ifndef __GatewayDiagnose__H_
#define __GatewayDiagnose__H_

#include "NetworkDiagnose.h"

#ifdef __cplusplus

class NetworkInterface;

class GatewayDiagnose : public NetworkDiagnose::DiagnoseProcess {
public:
    GatewayDiagnose();
    ~GatewayDiagnose(); 

    virtual int type() { return eTypeGatewayDiagnose; }
    virtual int start(NetworkDiagnose* netdiag);
    virtual int stop();
    virtual int doResult(int result, int arg1, void* arg2 = 0);

    void attach(NetworkInterface* iface) { mIface = iface; }
    NetworkInterface* iface() { return mIface; }

private:
    int _SendARP();
    int _SendLCP();

private:
    NetworkInterface* mIface;
    int mErrorCode;
};
#endif

#endif
