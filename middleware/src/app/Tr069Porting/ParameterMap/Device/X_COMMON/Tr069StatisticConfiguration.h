#ifndef Tr069StatisticConfiguration_h
#define Tr069StatisticConfiguration_h

#include "Tr069GroupCall.h"

#ifdef __cplusplus

extern Tr069Call* g_tr069StatisticConfiguration;

/*------------------------------------------------------------------------------
 * CTC,CU,HUAWEI会统一调用的StatisticConfiguration
 ------------------------------------------------------------------------------*/

class Tr069StatisticConfig : public Tr069GroupCall {
public:
    Tr069StatisticConfig();
    ~Tr069StatisticConfig();
};

#endif // __cplusplus

#endif // Tr069StatisticConfiguration_h
