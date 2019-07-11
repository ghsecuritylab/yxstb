#include "UltraPlayerStatisticMultiple.h"

#include "UltraPlayerMultiple.h"

namespace Hippo {
    int UltraPlayerStatisticMultiple::s_multiRequestNum;
    int UltraPlayerStatisticMultiple::s_multiRequestReactTime;
    int UltraPlayerStatisticMultiple::s_multiFailedNum;
    char UltraPlayerStatisticMultiple::s_multiFailedInfo[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_MULT_SIZE];
    char UltraPlayerStatisticMultiple::s_mutiAbendInfo[STATISTIC_INFO_FAID_NUM][STATISTIC_INFO_MULT_SIZE];
    int UltraPlayerStatisticMultiple::s_multiOverflowNum;
    int UltraPlayerStatisticMultiple::s_multiUnderflowNum;
    int UltraPlayerStatisticMultiple::s_HD_MultiUnderflowNum;
    int UltraPlayerStatisticMultiple::s_HD_MultiOverflowNum;


} // namespace Hippo


