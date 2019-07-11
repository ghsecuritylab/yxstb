#ifndef _Tr069Root_H_
#define _Tr069Root_H_

#ifdef __cplusplus

#include "Tr069Call.h"

int Tr069RootRegist(const char *name, Tr069Call *call);
Tr069Call *Tr069RootUnregist(const char *name);

#endif // __cplusplus


#ifdef __cplusplus
extern "C" {
#endif
  
int Tr069RootInit();
int Tr069RootRead(const char *name, char *str, unsigned int val);
int Tr069RootWrite(const char* name, char* str, unsigned int val);

#ifdef __cplusplus
}
#endif

#endif // _Tr069Root_H_
