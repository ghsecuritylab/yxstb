#ifndef _Tr069LAN_H_
#define _Tr069LAN_H_

#include "Tr069GroupCall.h"

#ifdef __cplusplus

extern Tr069Call* g_cusLan;

class Tr069LAN : public Tr069GroupCall {
public:
    Tr069LAN();
    ~Tr069LAN();
};

#endif // __cplusplus

#endif // _Tr069LAN_H_ 