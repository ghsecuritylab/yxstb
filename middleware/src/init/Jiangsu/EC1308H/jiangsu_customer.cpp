

#include "customer.h"
#include "auth.h"
#include "js_eds_roller.h"
#include "default_url_checker.h"
#include "concatenator_hw_auth.h"
#include "libzebra.h"
#include "Assertions.h"
#include "http/http_apple.h"

#include <stdio.h>


namespace Hippo {

class JsCustomer : public CCustomer {
public:
    virtual JsCustomer();
    virtual ~JsCustomer();
    virtual int HardwareVersion(char * output, int len);
    virtual void SN17toSN32(const char * SN17, char * SN32);
};


JsCustomer::JsCustomer()
{
    AutoUnrefT<JsEdsRoller>         r;
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

JsCustomer::~JsCustomer()
{
    Auth* a = &AuthInfo();
    if (a)
        delete a;
}

int JsCustomer::HardwareVersion(char * hard_type, int len)
{
    if (!hard_type)
        return -1;

    const char * hardtype = "M8048";
    snprintf(hard_type, len, "%s", hardtype);
    return 0;
}

void JsCustomer::SN17toSN32(const char * SN17, char * SN32)
{
    if (!SN17 || !SN32)
        ERR_OUT("change sn from 17 to 32 error\n");

    memcpy(SN32, "00100199006030810000000000000000", 32);
    memcpy(SN32 + 16, SN17 + 4, 4);
    memcpy(SN32 + 20, "308", 3);
    memcpy(SN32 + 23, SN17 + 8, 9);
Err:
    return;
}


void CustomerCreate ()
{
    JsCustomer * c = new JsCustomer();
}

} // namespace Hippo

