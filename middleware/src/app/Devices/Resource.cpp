
#include "Resource.h"

#include "ResourceUser.h"


namespace Hippo {

bool
Resource::attachUser(ResourceUser* user)
{
    std::vector<ResourceUser*>::iterator it;

    for (it = m_resourceUserArray.begin(); it != m_resourceUserArray.end(); ++it) {
        if (user->priority() > (*it)->priority()) {
            m_resourceUserArray.insert(it, 1, user);
            return true;
        }
    }

    m_resourceUserArray.push_back(user);
    user->onAttachToResource(this);
    return true;
}

bool
Resource::detachUser(ResourceUser* user)
{
    std::vector<ResourceUser*>::iterator it;

    for (it = m_resourceUserArray.begin(); it != m_resourceUserArray.end(); ++it) {
        if (user == *it) {
            user->onDetachFromResource(this);
            m_resourceUserArray.erase(it);
            break;
        }
    }

    return true;
}

bool
Resource::checkUser(int userType)
{
    std::vector<ResourceUser*>::iterator it;

    for (it = m_resourceUserArray.begin(); it != m_resourceUserArray.end(); ++it) {
		
        if (userType == (*it)->type()) {
            return true;
        }
    }

    return false;
}


ResourceUser* 
Resource::getLowestPriorityUser()
{
    if (m_resourceUserArray.size() == 0)
        return NULL;
    return m_resourceUserArray.back();
}

ResourceUser* 
Resource::getHighestPriorityUser()
{
    if (m_resourceUserArray.size() == 0)
        return NULL;
    return m_resourceUserArray.front();
}

ResourceUser* 
Resource::getUserByIndex(int idx)
{	
	if (m_resourceUserArray.size() == 0 
		|| idx < 0
		|| idx >= m_resourceUserArray.size())
        return NULL;
	else
		return m_resourceUserArray[idx];
}


} // namespace Hippo
