#ifndef Tr069ServiceInfo_h
#define Tr069ServiceInfo_h

#include "Tr069GroupCall.h"

#ifdef __cplusplus

extern Tr069Call* g_tr69ServiceInfo;

/*------------------------------------------------------------------------------
 * CTC,CU,HUAWEI会统一调用的ServiceInfo
 ------------------------------------------------------------------------------*/
 
 
 // HW,CTC统一
class Tr069ServiceInfo : public Tr069GroupCall {
public:
    Tr069ServiceInfo();
    ~Tr069ServiceInfo();
};

#if 0
//CU 字符串名字不同，暂时没有弄到一起 
class Tr069CUServiceInfo : public Tr069GroupCall {
public:
    Tr069CUServiceInfo();
    ~Tr069CUServiceInfo();
};
#endif


#endif // __cplusplus

#endif // Tr069ServiceInfo_h
