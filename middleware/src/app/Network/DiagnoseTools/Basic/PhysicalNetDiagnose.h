#ifndef __PhysicalNetDiagnose__H_
#define __PhysicalNetDiagnose__H_

#include "NetworkDiagnose.h"

#ifdef __cplusplus

class NetworkCard;

class PhysicalNetDiagnose : public NetworkDiagnose::DiagnoseProcess {
public:
    PhysicalNetDiagnose();
    ~PhysicalNetDiagnose(); 
    virtual int type() { return eTypePhysicalNetDiagnose; }
    virtual int start(NetworkDiagnose* netdiag);
    virtual int stop();

    void attach(NetworkCard* card) { mDevice = card; }
    NetworkCard* device() { return mDevice; }
private:
    NetworkCard* mDevice;
};
#endif

#endif
