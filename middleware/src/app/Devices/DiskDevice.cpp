
#include "DiskDevice.h"

#include "ResourceUser.h"


namespace Hippo {

DiskDevice::DiskDevice(int diskSpace)
	: mDiskSpace(diskSpace)
{
}

DiskDevice::~DiskDevice()
{
}

int 
DiskDevice::getFreeDiskSpace()
{
    int used = 0.0;

    std::vector<ResourceUser *>::iterator it;
    for (it = m_resourceUserArray.begin(); it != m_resourceUserArray.end(); ++it) {
        used += (*it)->getRequiredDiskSpace();
    }

    return mDiskSpace - used;
}

bool 
DiskDevice::resourceIsEnough(ResourceUser *user)
{
    return user->getRequiredDiskSpace() <= getFreeDiskSpace();
}

bool 
DiskDevice::attachUser(ResourceUser *user)
{
    return Resource::attachUser(user);
}

bool 
DiskDevice::detachUser(ResourceUser *user)
{
    return Resource::detachUser(user);
}

} // namespace Hippo
