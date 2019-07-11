#ifndef _CTCServiceEntry_H_
#define _CTCServiceEntry_H_

#include "JsGroupCall.h"

#ifdef __cplusplus

namespace Hippo {

class CTCServiceEntry : public JsGroupCall {

};

int parseServiceEntry(const char*, const char*, char*, int);


} // namespace Hippo

#endif // __cplusplus

#endif // _CTCServiceEntry_H_
