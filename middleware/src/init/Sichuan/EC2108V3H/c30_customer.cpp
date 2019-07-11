

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

class C30Customer : public CCustomer {
public:
    C30Customer();
    virtual ~C30Customer();
    virtual int HardwareVersion(char * output, int len);
    virtual void SN17toSN32(const char * SN17, char * SN32);
};


C30Customer::C30Customer()
    : CCustomer()
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

C30Customer::~C30Customer()
{
    Auth* a = &AuthInfo();
    if (a)
        delete a;
}

int C30Customer::HardwareVersion(char * hard_type, int len)
{
    unsigned int hardwareVer = 0;
    if (!hard_type)
        return -1;
    yhw_board_getHWVersion(&hardwareVer);
    if (hardwareVer == 0x300)
        snprintf(hard_type, len, "M8043V02");
    else
        snprintf(hard_type, len, "EC2108");
    return 0;
}

void C30Customer::SN17toSN32(const char * SN17, char * SN32)
{
    if( (SN17 == NULL) || (SN32 == NULL) )
        ERR_OUT("change sn from 17 to 32 error\n");
    memcpy(SN32, "00100199006000010000000000000000", 32);
    memcpy(SN32 + 16, SN17 + 4, 4);
    if (!strncasecmp(SN17 + 8, "HWC", 3)) { // HWC:EC2108(Hi3716M方案)统一生产版本标志
        memcpy(SN32 + 4,  "02",  2);
        memcpy(SN32 + 12, "201", 3);
        memcpy(SN32 + 20, "201", 3);
    } else if (!strncasecmp(SN17 + 8, "H2E", 3)) { // H2E:EC2108(Hi3716M方案)对接四川电信局点标志
        memcpy(SN32 + 12, "208", 3);
        memcpy(SN32 + 20, "208", 3);
    } else if (!strncasecmp(SN17 + 8, "HB7", 3)) { // HB7:EC2108(Brcm7405方案)对接四川电信局点标志
        memcpy(SN32 + 12, "210", 3);
        memcpy(SN32 + 20, "210", 3);
    } else if (!strncasecmp(SN17 + 8, "H5D", 3)) { // H5D:EC5108(Brcm7405方案)对接四川电信局点标志
        memcpy(SN32 + 4,  "02",  2);
        memcpy(SN32 + 12, "510", 3);
        memcpy(SN32 + 20, "510", 3);
    } else {
        memcpy(SN32 + 12, SN17, 1);
        memcpy(SN32 + 13, SN17 + 2, 2);
        memcpy(SN32 + 20, SN17, 1);
        memcpy(SN32 + 21, SN17 + 2, 2);
    }
    memcpy(SN32 + 23, SN17 + 8, 9);
Err:
    return;
}


void CustomerCreate ()
{
    C30Customer * c = new C30Customer();
}

} // namespace Hippo

