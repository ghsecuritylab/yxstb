#ifndef StatisticCTCPlay_h 
#define StatisticCTCPlay_h 

#include "../StatisticHW/StatisticHWPlay.h"

#ifdef __cplusplus

class StatisticCTCPlay : public StatisticHWPlay
{
public:
    StatisticCTCPlay();
    ~StatisticCTCPlay();
    
    // virtual void statisticPost(void); // CTC���ر�����������Ӵ˺���
    virtual void statisticFile(void);
    
};

#endif // __cplusplus

#endif // StatisticCTCPlay_h 
