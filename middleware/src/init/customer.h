
#ifndef __CUSTOMER_H__
#define __CUSTOMER_H__

#pragma once

#ifdef __cplusplus

#include "auth.h"

namespace Hippo {

class Auth;
class CCustomer {
public:
    CCustomer();
    virtual ~CCustomer();

    virtual int HardwareVersion(char * output, int len);
    virtual void SN17toSN32(const char * sn17, char * sn32);

    Auth& AuthInfo() { return *m_auth; }

protected:
    void SetAuth(Auth* auth) { m_auth = auth; }

private:
    Auth* m_auth;
};


void CustomerCreate();
CCustomer& Customer();

} // namespace Hippo
#endif

#ifdef __cplusplus
extern "C" {
#endif

void CustomerCreate();
int HardwareVersion(char * hard_type, int len);
void mid_SN17toSN32(const char * sn17, char * sn32);

const char *SQAVersion(void);
const char *SQMVersion(void);

#ifdef __cplusplus
}
#endif

#endif


