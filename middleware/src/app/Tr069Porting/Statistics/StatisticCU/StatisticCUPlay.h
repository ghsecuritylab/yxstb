#ifndef StatisticCUPlay_h
#define StatisticCUPlay_h 

#include "../StatisticHW/StatisticHWPlay.h"

#ifdef __cplusplus

class StatisticCUPlay : public StatisticHWPlay
{
public:
    StatisticCUPlay();
    ~StatisticCUPlay();
    
    // virtual void statisticPost(void); // CU���ر�����������Ӵ˺���
    virtual void statisticFile(char *url);

};

#endif // __cplusplus

#endif // StatisticCUPlay_h 
