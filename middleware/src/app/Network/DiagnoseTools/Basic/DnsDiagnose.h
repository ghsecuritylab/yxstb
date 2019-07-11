#ifndef __DnsDiagnose__H_
#define __DnsDiagnose__H_

#include "CentralAreaDiagnose.h"

#ifdef __cplusplus

class DnsDiagnose : public CentralAreaDiagnose {
public:
    DnsDiagnose();
    ~DnsDiagnose(); 

    virtual int type() { return eTypeDnsDiagnose; }
    virtual int start(NetworkDiagnose* netdiag);
    virtual int stop();

};

#endif

#endif
