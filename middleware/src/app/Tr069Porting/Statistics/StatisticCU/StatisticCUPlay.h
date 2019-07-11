#ifndef StatisticCUPlay_h
#define StatisticCUPlay_h 

#include "../StatisticHW/StatisticHWPlay.h"

#ifdef __cplusplus

class StatisticCUPlay : public StatisticHWPlay
{
public:
    StatisticCUPlay();
    ~StatisticCUPlay();
    
    // virtual void statisticPost(void); // CU有特别需求在来添加此函数
    virtual void statisticFile(char *url);

};

#endif // __cplusplus

#endif // StatisticCUPlay_h 
