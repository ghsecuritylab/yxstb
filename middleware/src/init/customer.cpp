

#include "customer.h"
#include "Assertions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace Hippo {

static CCustomer* g_customer_instance = NULL;

CCustomer::CCustomer()
{
    if (g_customer_instance) {
        fprintf(stderr, "** ERROR ** You should call CustomerCreate() only once.\n");
        abort();
    }
    g_customer_instance = this;
}

CCustomer::~CCustomer()
{
    g_customer_instance = NULL;
}

int CCustomer::HardwareVersion(char * hard_type, int len)
{
    snprintf(hard_type, len, "EC2108");
    return 0;
}

void CCustomer::SN17toSN32(const char * SN17, char * SN32)
{
    if (!SN17 || !SN32) {
        ERR_OUT("change sn from 17 to 32 error\n");
    }
    memcpy(SN32, "00100199006000010000000000000000", 32);
#if defined(hi3716m)
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
#else
    memcpy(SN32 + 12, SN17, 3);
    memcpy(SN32 + 20, SN17, 3);
    memcpy(SN32 + 16, SN17 + 4, 4);
    memcpy(SN32 + 23, SN17 + 8, 9);
#endif
Err:
    return;
}


CCustomer& Customer()
{
    if (!g_customer_instance) {
        fprintf(stderr, "** ERROR ** You should call CustomerCreate() first.\n");
        abort();
    }
    return *g_customer_instance;
}

} // namespace Hippo


extern "C" void CustomerCreate()
{
    Hippo::CustomerCreate();
}

extern "C" int HardwareVersion(char * hard_type, int len)
{
    return Hippo::Customer().HardwareVersion(hard_type, len);
}

extern "C" void mid_SN17toSN32(const char * SN17, char * SN32)
{
    return Hippo::Customer().SN17toSN32(SN17, SN32);
}

extern "C" const char *SQAVersion(void)
{
    return "Not supported.";
}

extern "C" const char *SQMVersion(void)
{
    return "Not supported.";
}





