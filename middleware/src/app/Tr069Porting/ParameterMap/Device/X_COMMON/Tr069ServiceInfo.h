#ifndef Tr069ServiceInfo_h
#define Tr069ServiceInfo_h

#include "Tr069GroupCall.h"

#ifdef __cplusplus

extern Tr069Call* g_tr69ServiceInfo;

/*------------------------------------------------------------------------------
 * CTC,CU,HUAWEI��ͳһ���õ�ServiceInfo
 ------------------------------------------------------------------------------*/
 
 
 // HW,CTCͳһ
class Tr069ServiceInfo : public Tr069GroupCall {
public:
    Tr069ServiceInfo();
    ~Tr069ServiceInfo();
};

#if 0
//CU �ַ������ֲ�ͬ����ʱû��Ū��һ�� 
class Tr069CUServiceInfo : public Tr069GroupCall {
public:
    Tr069CUServiceInfo();
    ~Tr069CUServiceInfo();
};
#endif


#endif // __cplusplus

#endif // Tr069ServiceInfo_h
