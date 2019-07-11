#ifndef _UltraPlayerStatisticVod_H_
#define _UltraPlayerStatisticVod_H_

#include "UltraPlayerStatistic.h"
#ifdef __cplusplus

namespace Hippo {

class UltraPlayerStatisticVod {    /*for statistic */
public :
    UltraPlayerStatisticVod(){};
    ~UltraPlayerStatisticVod(){};

public:
    static int getRequestNum(){return s_vodRequestNum;}
    static int setRequestNum(int num) { return s_vodRequestNum = num;}
    static int getRequestReactTime(){return s_vodRequestReactTime;}
    static int setRequestReactTime(int num) { return s_vodRequestReactTime = num;}
    static int getFailedNum(){return s_vodFailedNum;}
    static int setFailedNum(int num) { return s_vodFailedNum = num;}
    static char* getFailedInfo(int index){return s_vodFailedInfo[index];}
    static int getUnderflowNum(){return s_vodUnderflowNum;}
    static int setUnderflowNum(int num) { return s_vodUnderflowNum = num;}
    static char* getAbendInfo(int index){return s_vodAbendInfo[index];}
    static int getOverflowNum(){return s_vodOverflowNum;}
    static int setOverflowNum(int num) { return s_vodOverflowNum = num;}
    static int getHDUnderflowNum(){return s_HD_vodUnderflowNum;}
    static int setHDUnderflowNum(int num) { return s_HD_vodUnderflowNum = num;}
    static int getHDOverflowNum(){return s_HD_vodOverflowNum;}
    static int setHDOverflowNum(int num) { return s_HD_vodOverflowNum = num;}
    static int getTotalRequestReactTime(){return s_vodTotalRequestReactTime;}
    static int getStopNum(){return s_vodStopNum;}
    static int getTotalChangeStopTime(){return s_vodTotalChangeStopTime;}
    static int getPauseNum(){return s_vodPauseNum;}
    static int getTotalChangePauseTime(){return s_vodTotalChangePauseTime;}
    static int getResumNum(){return s_vodResumNum;}
    static int setResumNum(int num){return s_vodResumNum = num;}
    static int getTotalChangeResumTime(){return s_vodTotalChangeResumTime;}
    static int getFastForwardNum(){return s_vodFastForwardNum;}
    static int getTotalChangeFastForwardTime(){return s_vodTotalChangeFastForwardTime;}
    static int getFastRewindNum(){return s_vodFastRewindNum;}
    static int getTotalChangeFastRewindTime(){return s_vodTotalChangeFastRewindTime;}
    static int getReqBestPerform(){return s_vodRequsetReactWorst;}
    static int getReqWorstPerform(){return s_vodRequsetReactBest;}

    static void streamVodstop(int ms);
    static void streamVodpause(int ms);
    static void streamVodreq(int ms);
    static void streamVodresume(int ms);
    static void streamVodforward(int ms);
    static void streamVodbackward(int ms);
protected:
    static int s_vodRequestNum;
    static int s_vodRequestReactTime;
    static int s_vodFailedNum;
    static char s_vodFailedInfo[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_FAID_SIZE];
    static char s_vodAbendInfo[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_FAID_SIZE];
    static int s_vodUnderflowNum;
    static int s_vodOverflowNum;
    static int s_HD_vodUnderflowNum;
    static int s_HD_vodOverflowNum;
    static int s_vodTotalRequestReactTime;
    static int s_vodRequsetReactWorst;
    static int s_vodRequsetReactBest;
    static int s_vodTotalChangeStopTime;
    static int s_vodStopNum;
    static int s_vodTotalChangePauseTime;
    static int s_vodPauseNum;
    static int s_vodTotalChangeResumTime;
    static int s_vodResumNum;
    static int s_vodTotalChangeFastForwardTime;
    static int s_vodFastForwardNum;
    static int s_vodTotalChangeFastRewindTime;
    static int s_vodFastRewindNum;
};

} //namespace Hippo


#endif // __cplusplus

#endif  // _UltraPlayerStatisticVod_H_