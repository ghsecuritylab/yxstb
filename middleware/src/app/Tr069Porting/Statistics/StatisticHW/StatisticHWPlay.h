#ifndef StatisticHWPlay_h
#define StatisticHWPlay_h
 
#include "StatisticBase.h" 

#ifdef __cplusplus

class StatisticHWPlay : public StatisticBase{
public:
    StatisticHWPlay();
    ~StatisticHWPlay();
    
    virtual void sysCfgInit(void);
    virtual void statisticCfgReset(void);
    virtual void statisticCfgSave(void);
    
    virtual void statisticStart(void);
    virtual void statisticPeriod(void);
    virtual void statisticPost(void);
    virtual void statisticFile(void);
    
    virtual void statisticBitrateRn(int mult, int width, int bitrate);
    virtual void statisticBitratePercentRn(int mult, int width, int bitrate);
    virtual void statisticPacketLostsRn(int mult, int width, int totals, int losts);
};

#endif // __cplusplus

#endif // StatisticHWPlay_h
