#ifndef _Tr069TCPSYNDiagnostics_H_
#define _Tr069TCPSYNDiagnostics_H_

#include "Tr069GroupCall.h"

#ifdef __cplusplus

extern Tr069Call* g_cusLan;

class Tr069TCPSYNDiagnostics : public Tr069GroupCall {
public:
    Tr069TCPSYNDiagnostics();
    ~Tr069TCPSYNDiagnostics();
};

#endif // __cplusplus

#endif // _Tr069TCPSYNDiagnostics_H_ 