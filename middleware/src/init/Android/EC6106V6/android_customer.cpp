

#include "customer.h"
#include "auth.h"
#include "default_eds_roller.h"
#include "default_url_checker.h"
#include "concatenator_hw_auth.h"
#include "libzebra.h"
#include "Assertions.h"
#include "mid_stream.h"

#include "http/http_apple.h"

#include <stdio.h>


namespace Hippo {

class AndroidCustomer : public CCustomer {
public:
    AndroidCustomer();
    virtual ~AndroidCustomer();
    virtual int HardwareVersion(char * output, int len);
};


AndroidCustomer::AndroidCustomer()
{
    AutoUnrefT<DefaultEdsRoller>    r;
    AutoUnrefT<DefaultUrlChecker>   uc;
    AutoUnrefT<ConcatenatorHwAuth>  c;
    SetAuth(new Auth(r.Get(), uc.Get(), c.Get()));

    mid_stream_nat(1);
    mid_stream_rrs_timeout(2000);
    mid_stream_set_arq(1);
    mid_stream_set_burst(1);

    mid_stream_heartbit_period(30);
    mid_stream_standard(RTSP_STANDARD_CTC_GUANGDONG);
    mid_stream_set_size(4600 * 1316, 8192 * 1316);
}

AndroidCustomer::~AndroidCustomer()
{
    Auth* a = &AuthInfo();
    if (a)
        delete a;
}

int AndroidCustomer::HardwareVersion(char * hard_type, int len)
{
    snprintf(hard_type, len, "EC6106V6");
    return 0;
}


void CustomerCreate ()
{
    AndroidCustomer * c = new AndroidCustomer();
}

} // namespace Hippo

