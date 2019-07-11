
#include "UpgradeAssertions.h"
#include "UpgradeChecker.h"


namespace Hippo {

UpgradeChecker::UpgradeChecker(UpgradeManager* manager)
	: m_manager(manager)
	, m_source(0)
	, m_state(UCS_IDLE)
	, m_type(0)
	, m_errorCode(0)
{
}

UpgradeChecker::~UpgradeChecker()
{
}

bool 
UpgradeChecker::start()
{
    return false;
}

bool 
UpgradeChecker::stop()
{
    return false;
}

bool 
UpgradeChecker::reset()
{
    return false;
}

} // namespace Hippo

