#ifndef _UltraPlayerStatisticMultiple_H_
#define _UltraPlayerStatisticMultiple_H_

#include "UltraPlayerStatistic.h"
#ifdef __cplusplus

namespace Hippo {

class UltraPlayerStatisticMultiple {    /*for statistic */
public :
    UltraPlayerStatisticMultiple(){};
    ~UltraPlayerStatisticMultiple(){};

public:
    static int getRequestNum(){return s_multiRequestNum;}
    static int setRequestNum(int num) { return s_multiRequestNum = num;}
    static int getRequestReactTime(){return s_multiRequestReactTime;}
    static int setRequestReactTime(int num) { return s_multiRequestReactTime = num;}
    static int getFailedNum(){return s_multiFailedNum;}
    static int setFailedNum(int num) { return s_multiFailedNum = num;}
    static char* getFailedInfo(int index){return s_multiFailedInfo[index];}
    static char* getAbendInfo(int index){return s_mutiAbendInfo[index];}
    static int getUnderflowNum(){return s_multiUnderflowNum;}
    static int setUnderflowNum(int num) { return s_multiUnderflowNum = num;}
    static int getOverflowNum(){return s_multiOverflowNum;}
    static int setOverflowNum(int num) { return s_multiOverflowNum = num;}
    static int getHDUnderflowNum(){return s_HD_MultiUnderflowNum;}
    static int setHDUnderflowNum(int num) { return s_HD_MultiUnderflowNum = num;}
    static int getHDOverflowNum(){return s_HD_MultiOverflowNum;}
    static int setHDOverflowNum(int num) { return s_HD_MultiOverflowNum = num;}

private:
    static int s_multiRequestNum;
    static int s_multiRequestReactTime;
    static int s_multiFailedNum;
    static char s_multiFailedInfo[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_MULT_SIZE];
    static char s_mutiAbendInfo[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_MULT_SIZE];
    static int s_multiUnderflowNum;
    static int s_multiOverflowNum;
    static int s_HD_MultiUnderflowNum;
    static int s_HD_MultiOverflowNum;

};

} //namespace Hippo


#endif // __cplusplus

#endif  // _UltraPlayerStatisticMultiple_H_