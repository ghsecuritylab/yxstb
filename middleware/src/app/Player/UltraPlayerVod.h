#ifndef _UltraPlayerVod_H_
#define _UltraPlayerVod_H_

#include "UltraPlayer.h"

#include "UltraPlayerStatistic/UltraPlayerStatisticVod.h"

#ifdef __cplusplus

namespace Hippo {

class UltraPlayerVod : public UltraPlayer {
public:
    UltraPlayerVod(UltraPlayerClient *, BrowserPlayerReporter *, Program *);
    ~UltraPlayerVod();

public:
    static UltraPlayerStatisticVod s_statistic;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _UltraPlayerVod_H_
