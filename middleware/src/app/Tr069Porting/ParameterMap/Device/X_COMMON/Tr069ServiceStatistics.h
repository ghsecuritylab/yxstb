#ifndef Tr069ServiceStatistics_h
#define Tr069ServiceStatistics_h

#include "Tr069GroupCall.h"

#ifdef __cplusplus

extern Tr069Call* g_tr069ServiceStatistics;


class Tr069ServiceStatistics : public Tr069GroupCall {
public:
    Tr069ServiceStatistics();
    ~Tr069ServiceStatistics();
};

#endif // __cplusplus

#endif // Tr069ServiceStatistics_h
