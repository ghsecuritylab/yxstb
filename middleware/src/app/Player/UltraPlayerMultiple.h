#ifndef _UltraPlayerMultiple_H_
#define _UltraPlayerMultiple_H_

#include "UltraPlayer.h"
#include "UltraPlayerStatistic/UltraPlayerStatisticMultiple.h"
#ifdef __cplusplus

namespace Hippo {

class UltraPlayerMultiple : public UltraPlayer {
public:
    UltraPlayerMultiple(UltraPlayerClient *, BrowserPlayerReporter *, Program *);
    ~UltraPlayerMultiple();

public:
    static UltraPlayerStatisticMultiple s_statistic;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _UltraPlayerMultiple_H_
