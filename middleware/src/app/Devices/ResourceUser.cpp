
#include "ResourceUser.h"


namespace Hippo {

ResourceUser::ResourceUser()
	: mPriority(0)
	, mCanBePreempted(false)
	, mOccupyMode(0)
{
}

ResourceUser::~ResourceUser()
{
}

} // namespace Hippo
