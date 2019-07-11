#ifndef _configDefaultPayShell_H_
#define _configDefaultPayShell_H_

#include "solution.h"

#ifndef DEFAULT_PAY_CLIENT_DOMAIN
#define DEFAULT_PAY_CLIENT_DOMAIN "http://192.168.0.116:8080/payclient/huawei/EC5108"
#endif

// 只支持 sha1 sha256 sha512 这三种类型
#ifndef DEFAULT_PAY_CLIENT_SIGN_TYPE
#define DEFAULT_PAY_CLIENT_SIGN_TYPE "-sha1"
#endif

#ifndef DEFAULT_PAY_CLIENT_SIGN_FILE
#define DEFAULT_PAY_CLIENT_SIGN_FILE "iptvsig"DEFAULT_PAY_CLIENT_SIGN_TYPE".bin"
#endif

#endif // _configDefaultPayShell_H_
