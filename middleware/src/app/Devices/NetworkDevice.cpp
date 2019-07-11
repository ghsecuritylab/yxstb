
#include "NetworkDevice.h"

#include "ResourceUser.h"


namespace Hippo {

NetworkDevice::NetworkDevice(float bandwidth)
	: mBandwidth(bandwidth)
{
}

NetworkDevice::~NetworkDevice()
{
}

float
NetworkDevice::getFreeBandwidth()
{
    float used = 0.0;

    std::vector<ResourceUser *>::iterator it;
    for (it = m_resourceUserArray.begin(); it != m_resourceUserArray.end(); ++it) {
		//if ((*it)->occupyMode() == ResourceUser::Alone)
        	used += (*it)->getRequiredBandwidth();
    }

    return mBandwidth - used;
}

ResourceUser *
NetworkDevice::resourceCanBeShared(ResourceUser *user)
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


int
NetworkDevice::setNetworkBandwidth(float bandwidth)
{
   if(bandwidth < 0.01 && bandwidth > -0.01)
   return -1;

   mBandwidth = bandwidth;

   return 0;
}
int
NetworkDevice::getNetworkBandwidth(void)
{
   return mBandwidth;
}

bool
NetworkDevice::resourceIsEnough(ResourceUser *user)
{
	if (resourceCanBeShared(user))
		return true;
    return user->getRequiredBandwidth() <= getFreeBandwidth();
}

bool
NetworkDevice::attachUser(ResourceUser *user)
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
NetworkDevice::detachUser(ResourceUser *user)
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
