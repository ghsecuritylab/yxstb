
#include "Concurrent.h"

#include "ResourceUser.h"


namespace Hippo {

Concurrent::Concurrent(int num)
	: mConcurrentNumber(num)
{
}

Concurrent::~Concurrent()
{
}

int
Concurrent::setConcurrentNumber(int num)
{
   	if (num < 0) return -1;

   	mConcurrentNumber = num;
   	return 0;
}

int
Concurrent::getFreeConcurrentNumber()
{
    int used = 0;

    std::vector<ResourceUser *>::iterator it;
    for (it = m_resourceUserArray.begin(); it != m_resourceUserArray.end(); ++it) {
		//if ((*it)->occupyMode() == ResourceUser::Alone)
        	used += 1;
    }

    return mConcurrentNumber - used;
}

int
Concurrent::getConcurrentNumber(void)
{
   return mConcurrentNumber;
}

ResourceUser *
Concurrent::resourceCanBeShared(ResourceUser *user)
{
#if 0
    std::vector<ResourceUser *>::iterator it;
    for (it = m_resourceUserArray.begin(); it != m_resourceUserArray.end(); ++it) {
        if (user != (*it)
			&& user->getRequiredProgNumber() == (*it)->getRequiredProgNumber())
			return (*it);
    }
#endif
    return NULL;
}

bool
Concurrent::resourceIsEnough(ResourceUser *user)
{
	if (resourceCanBeShared(user))
		return true;
    return getFreeConcurrentNumber() == 0;
}

bool
Concurrent::attachUser(ResourceUser *user)
{
#if 0
	if (resourceCanBeShared(user)) {
		user->setOccupyMode(ResourceUser::Share);
	} else {
		user->setOccupyMode(ResourceUser::Alone);
	}
#endif	
    return Resource::attachUser(user);
}

bool
Concurrent::detachUser(ResourceUser *user)
{
#if 0
	if (user->occupyMode() == ResourceUser::Alone) {
		std::vector<ResourceUser *>::iterator it;
	    for (it = m_resourceUserArray.begin(); it != m_resourceUserArray.end(); ++it) {
	        if (user != (*it)
				&& user->getRequiredProgNumber() == (*it)->getRequiredProgNumber()){
				(*it)->setOccupyMode(ResourceUser::Alone);
				break;
	        }	
	    }
	}
#endif	
    return Resource::detachUser(user);
}

} // namespace Hippo
