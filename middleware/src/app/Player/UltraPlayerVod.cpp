
#include "UltraPlayerVod.h"


namespace Hippo {

UltraPlayerStatisticVod UltraPlayerVod::s_statistic;

UltraPlayerVod::UltraPlayerVod(UltraPlayerClient *client, BrowserPlayerReporter *pReporter, Program *pProgram)
	: UltraPlayer(client, pReporter, pProgram)
{
}

UltraPlayerVod::~UltraPlayerVod()
{
}

} // namespace Hippo
