#ifndef __NtpDiagnose__H_
#define __NtpDiagnose__H_

#include "CentralAreaDiagnose.h"

#ifdef __cplusplus

class NtpDiagnose : public CentralAreaDiagnose {
public:
    NtpDiagnose(int mode = ND_MODE_SERVICE);
    ~NtpDiagnose(); 

    virtual int type() { return eTypeNtpDiagnose; }
    virtual int start(NetworkDiagnose* netdiag);
    virtual int stop();

private:
    int _AdoptSntp(int family);
};

#endif

#endif
