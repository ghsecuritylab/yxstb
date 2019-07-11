#ifndef _JseRoot_H_
#define _JseRoot_H_

#ifdef __cplusplus

#include "JseCall.h"
#include "JseGroupCall.h"

int JseRootRegist(const char *name, JseCall *length);
JseCall* JseRootUnregist(const char *name);
JseGroupCall* getNodeByName(const char* name);
#endif // __cplusplus


#ifdef __cplusplus
extern "C" {
#endif

int JseRootInit();

int JseRootRead(const char *name, const char *param, char *value, int length);
int JseRootWrite(const char *name, const char *param, char *value, int length);

#ifdef __cplusplus
}
#endif

#endif // _JseRoot_H_
