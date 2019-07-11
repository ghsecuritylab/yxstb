

#include "customer.h"
#include "auth.h"
#include "js_eds_roller.h"
#include "default_url_checker.h"
#include "concatenator_hw_auth.h"
#include "libzebra.h"
#include "Assertions.h"

#include <stdio.h>

#include "http/http_apple.h"

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
#if defined(brcm7405)
    YX_HW_INFO info;
    yhw_env_getHardwareInfo(&info);
    if(strlen(info.product_id) >= len) {
        strncpy(hard_type, info.product_id, len - 1);
        hard_type[len - 1] = '\0';
    } else
        strcpy(hard_type, info.product_id);
#elif defined(hi3716m)
    unsigned int hardwareVer = 0;
    if(!hard_type)
        return -1;
    yhw_board_getHWVersion(&hardwareVer);
    if (hardwareVer == 0x300)
        strcpy(hard_type, "EC2108V3");
    else
        strcpy(hard_type, "EC2108");
#else
#error "Can't defined platform."
#endif
    return 0;
}

void JsCustomer::SN17toSN32(const char * SN17, char * SN32)
{
    if (!SN17 || !SN32)
        ERR_OUT("change sn from 17 to 32 error\n");

    memcpy(SN32, "00100299006020110000000000000000", 32);
    memcpy(SN32 + 16, SN17 + 4, 4);
    memcpy(SN32 + 20, "201", 3);
    memcpy(SN32 + 23, SN17 + 8, 9);
Err:
    return;
}


void CustomerCreate ()
{
    JsCustomer * c = new JsCustomer();
}

} // namespace Hippo

