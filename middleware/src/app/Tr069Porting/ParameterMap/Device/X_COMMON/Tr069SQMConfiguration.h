#ifndef Tr069SQMConfiguration_h
#define Tr069SQMConfiguration_h

#include "Tr069GroupCall.h"

#ifdef __cplusplus

extern Tr069Call* g_tr69SQMConfiguration;

/*------------------------------------------------------------------------------
 * HUAWEI,CTC统一调用的SQMConfiguration
 ------------------------------------------------------------------------------*/
class Tr069SQMConfiguration : public Tr069GroupCall {
public:
    Tr069SQMConfiguration();
    ~Tr069SQMConfiguration();
};

#endif // __cplusplus

#endif // Tr069SQMConfiguration_h
