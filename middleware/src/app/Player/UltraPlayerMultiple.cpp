
#include "UltraPlayerMultiple.h"


namespace Hippo {
UltraPlayerStatisticMultiple UltraPlayerMultiple::s_statistic;

UltraPlayerMultiple::UltraPlayerMultiple(UltraPlayerClient *client, BrowserPlayerReporter *pReporter, Program *pProgram)
	: UltraPlayer(client, pReporter, pProgram)
{
}

UltraPlayerMultiple::~UltraPlayerMultiple()
{
}

} // namespace Hippo
