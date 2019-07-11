#ifndef _Tr069CUAlarm_H_
#define _Tr069CUAlarm_H_

#include "Tr069GroupCall.h"

#ifdef __cplusplus

class Tr069CUAlarm : public Tr069GroupCall {
public:
    Tr069CUAlarm();
    ~Tr069CUAlarm();
};

#endif // __cplusplus
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void tr069_cu_setFramesLost(unsigned int);
void tr069_cu_setPacketsLost(unsigned int);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _Tr069CUAlarm_H_
