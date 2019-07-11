
#include "ResourceManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Resource.h"
#include "ResourceUser.h"

#include "json/json_public.h"

#include <sstream>

namespace Hippo {

ResourceManager::ResourceManager()
{
    m_hightPriority = 100;
    m_priorityStep =  10;
    m_priorityMap[ResourceUser::PVR] = m_hightPriority - m_priorityStep;
    m_priorityMap[ResourceUser::SimplePlay] = m_hightPriority - 2*m_priorityStep;
    m_priorityMap[ResourceUser::PIP] = m_hightPriority - 3*m_priorityStep;
    m_priorityMap[ResourceUser::Download] = m_hightPriority - 4*m_priorityStep;
}

ResourceManager::~ResourceManager()
{
}

bool
ResourceManager::addNetworkResource(Resource* resource)
{
    int i;

    if (resource == NULL)
        return false;

    for (i = 0; i < m_networkDeviceArray.size(); i++) {
        if (m_networkDeviceArray[i] == resource)
            return true;
    }
    m_networkDeviceArray.push_back(resource);
    return true;
}

bool
ResourceManager::removeNetworkResource(Resource* resource)
{
    std::vector<Resource *>::iterator it;

    for (it = m_networkDeviceArray.begin(); it != m_networkDeviceArray.end(); ++it) {
        if (resource == *it) {
            m_networkDeviceArray.erase(it);
            return true;
        }
    }
    return false;
}

bool
ResourceManager::addConcurrentResource(Resource* resource)
{
    int i;

    if (resource == NULL)
        return false;

    for (i = 0; i < m_concurrentArray.size(); i++) {
        if (m_concurrentArray[i] == resource)
            return true;
    }
    m_concurrentArray.push_back(resource);
    return true;
}

bool
ResourceManager::removeConcurrentResource(Resource* resource)
{
    std::vector<Resource *>::iterator it;

    for (it = m_concurrentArray.begin(); it != m_concurrentArray.end(); ++it) {
        if (resource == *it) {
            m_concurrentArray.erase(it);
            return true;
        }
    }
    return false;
}

int
ResourceManager::concurrentResourceCount()
{
	int count = 0;

	std::vector<Resource *>::iterator it;

    for (it = m_concurrentArray.begin(); it != m_concurrentArray.end(); ++it) {
        count += (*it)->getConcurrentNumber();
    }

    return count;
}

int
ResourceManager::idleConcurrentResourceCount()
{
	int count = 0;

	std::vector<Resource *>::iterator it;

    for (it = m_concurrentArray.begin(); it != m_concurrentArray.end(); ++it) {
        count += (*it)->getFreeConcurrentNumber();
    }

    return count;
}


bool
ResourceManager::getConcurrentResourceDetail(json_object *jsonConcurrentRes)
{
	int i = 0;
	int hasIdle = 0;
	int userTotal = 0;

	json_object	*jsonArray = json_object_new_array();

	if (jsonConcurrentRes == NULL || jsonArray == NULL)
		return false;

	json_object_add_object(jsonConcurrentRes, "totalNumber", json_object_new_int(concurrentResourceCount()));

	if (idleConcurrentResourceCount() != 0)
		hasIdle = 1;
	json_object_add_object(jsonConcurrentRes, "isFree", json_object_new_int(hasIdle));

	for (i = 0; i < m_concurrentArray.size(); i++) {
		int j = 0;
		json_object *jsonOneUser = NULL;

        int count = m_concurrentArray[i]->userCount();
		if (count == 0)
			continue;

		userTotal += count;
		for (j = 0; j < count; j++) {
			ResourceUser* user = NULL;

			jsonOneUser = json_object_create_object();
			user = m_concurrentArray[i]->getUserByIndex(j);
			if (jsonOneUser && user) {
				std::string str;

				switch(user->type()) {
				case ResourceUser::SimplePlay :
					str = "Playing";
					break;
				case ResourceUser::Download :
					str = "Download";
					break;
				case ResourceUser::PVR :
					str = "PVR";
					break;
				default :
					break;
				}
				json_object_add_object(jsonOneUser, "appType", json_object_new_string(str.c_str()));

				user->getObjectID(str);
				json_object_add_object(jsonOneUser, "objectID", json_object_new_string(str.c_str()));

				json_object_array_add(jsonArray, jsonOneUser);
			}

		}

    }

	json_object_add_object(jsonConcurrentRes, "countOfApps", json_object_new_int(userTotal));
	json_object_add_object(jsonConcurrentRes, "apps", jsonArray);

	return true;
}


bool
ResourceManager::addTunerResource(Resource* resource)
{
    int i;

    if (resource == NULL)
        return false;

    for (i = 0; i < m_tunerArray.size(); i++) {
        if (m_tunerArray[i] == resource)
            return true;
    }
    m_tunerArray.push_back(resource);
    return true;
}

bool
ResourceManager::removeTunerResource(Resource* resource)
{
    std::vector<Resource *>::iterator it;

    for (it = m_tunerArray.begin(); it != m_tunerArray.end(); ++it) {
        if (resource == *it) {
            m_tunerArray.erase(it);
            return true;
        }
    }
    return false;
}

bool
ResourceManager::tunerResourceIsEnough(int chNum, int userType)
{
	int i = 0;

	for (i = 0; i < m_tunerArray.size(); i++) {
        if (m_tunerArray[i]->tunerResourceIsEnough(chNum, userType)) {
            return true;
        }
    }

	return false;
}

int
ResourceManager::idleTunerResourceCount()
{
	int i = 0;
	int count = 0;

	for (i = 0; i < m_tunerArray.size(); i++) {
		if (!m_tunerArray[i]->checkTunerIsLock()) {
			count ++;
		}
	}

	return count;
}

bool
ResourceManager::getTunerResourceDetail(json_object *jsonTunerRes)
{
	int i = 0;
	int hasIdle = 0;
	int userTotal = 0;

	json_object	*jsonArray = json_object_new_array();

	if (jsonTunerRes == NULL || jsonArray == NULL)
		return false;

	json_object_add_object(jsonTunerRes, "totalNumber", json_object_new_int(tunerResourceCount()));

	if (idleTunerResourceCount() != 0)
		hasIdle = 1;
	json_object_add_object(jsonTunerRes, "isFree", json_object_new_int(hasIdle));

	for (i = 0; i < m_tunerArray.size(); i++) {
		int j = 0;
		json_object *jsonOneUser = NULL;

        int count = m_tunerArray[i]->userCount();
		if (count == 0)
			continue;

		userTotal += count;
		for (j = 0; j < count; j++) {
			ResourceUser* user = NULL;

			jsonOneUser = json_object_create_object();
			user = m_tunerArray[i]->getUserByIndex(j);
			if (jsonOneUser && user) {
				std::string str;

				switch(user->type()) {
				case ResourceUser::SimplePlay :
					str = "Live";
					break;
				case ResourceUser::PIP :
					str = "PIP";
					break;
				case ResourceUser::PVR :
					str = "PVR";
					break;
				default :
					break;
				}
				json_object_add_object(jsonOneUser, "appType", json_object_new_string(str.c_str()));

				user->getObjectID(str);
				json_object_add_object(jsonOneUser, "objectID", json_object_new_string(str.c_str()));

				json_object_array_add(jsonArray, jsonOneUser);
			}

		}

    }

	json_object_add_object(jsonTunerRes, "countOfApps", json_object_new_int(userTotal));
	json_object_add_object(jsonTunerRes, "apps", jsonArray);

	return true;
}


bool
ResourceManager::addDiskResource(Resource* resource)
{
    int i;

    if (resource == NULL)
        return false;

    for (i = 0; i < m_diskArray.size(); i++) {
        if (m_diskArray[i] == resource)
            return true;
    }
    m_diskArray.push_back(resource);
    return true;
}

bool
ResourceManager::removeDiskResource(Resource* resource)
{
    std::vector<Resource*>::iterator it;

    for (it = m_diskArray.begin(); it != m_diskArray.end(); ++it) {
        if (resource == *it) {
            m_diskArray.erase(it);
            return true;
        }
    }
    return false;
}

bool
ResourceManager::requestResource(ResourceUser* user)
{
    if (user == NULL)
        return false;

Retry:
    Resource* standbyResources[Resource::RT_Max] = {0};
    if (resourceIsEnough(user, standbyResources)) {
        //...
        return allocResource(user, standbyResources);
    }

    /* Suspend someone that it's priority large than other conflictive user. */
    ResourceUser* toBeSuspended = NULL;
    int wanted = user->getRequirement();

    if (wanted & Resource::RTM_Bandwidth) {
        ResourceUser* lowest = getLowestPriorityUser(m_networkDeviceArray);
        if (lowest == NULL)
        	;
        else if (toBeSuspended == NULL)
            toBeSuspended = lowest;
        else if (getPriorityByType(toBeSuspended) < getPriorityByType(lowest))
            toBeSuspended = lowest;
    }

    if (wanted & Resource::RTM_Tuner) {
        ResourceUser* lowest = getLowestPriorityUser(m_tunerArray);
        if (lowest == NULL)
        	;
        else if (toBeSuspended == NULL)
            toBeSuspended = lowest;
        else if (getPriorityByType(toBeSuspended) < getPriorityByType(lowest))
            toBeSuspended = lowest;
    }

    if (wanted & Resource::RTM_Disk) {
        ResourceUser* lowest = getLowestPriorityUser(m_diskArray);
        if (lowest == NULL)
        	;
        else if (toBeSuspended == NULL)
            toBeSuspended = lowest;
        else if (getPriorityByType(toBeSuspended) < getPriorityByType(lowest))
            toBeSuspended = lowest;
    }

    if (toBeSuspended) {
        suspendResourceUser(toBeSuspended);
        /* Try again. */
        goto Retry;
    }
    else
        return false;
}

bool
ResourceManager::releaseResource(ResourceUser* user)
{
    if (user != NULL) {
        freeResource(user);

        if (m_waitingArray.size()) {
            std::vector<ResourceUser*>::iterator it;

            for (it = m_waitingArray.begin(); it != m_waitingArray.end(); ++it) {
                if (user == *it) {
                    m_waitingArray.erase(it);
                    break;
                }
            }
        }
    }

Retry:
    if (m_waitingArray.size()) {
        std::vector<ResourceUser*>::iterator it;

        for (it = m_waitingArray.begin(); it != m_waitingArray.end(); ++it) {
            ResourceUser* tmp = *it;
            Resource* standbyResources[Resource::RT_Max] = {0};
            if (resourceIsEnough(tmp, standbyResources)) {
                m_waitingArray.erase(it);

                allocResource(tmp, standbyResources);
                tmp->onAcquireResource();

                /* Try again. */
                goto Retry;
            }
        }
    }

    return true;
}

bool
ResourceManager::addUserToWaitingArray(ResourceUser* user)
{
    //if (user->canBePreempted()) {
        std::vector<ResourceUser*>::iterator it;
        for (it = m_waitingArray.begin(); it != m_waitingArray.end(); ++it) {
            if (m_priorityMap[user->type()] <= m_priorityMap[(*it)->type()]) {
                m_waitingArray.insert(it, 1, user);
            }
        }
        return true;
    //}
    //return false;
}

bool
ResourceManager::resourceIsEnough(ResourceUser* user, Resource** standbyResources)
{
    bool enough = false;
    int wanted = user->getRequirement();
    int i;

    if (wanted & Resource::RTM_Bandwidth) {
        enough = false;
        for (i = 0; i < m_networkDeviceArray.size(); i++) {
            if (m_networkDeviceArray[i]->resourceIsEnough(user)) {
                standbyResources[Resource::RT_Bandwidth] = m_networkDeviceArray[i];
                enough = true;
            }
        }
        if (!enough)
            return false;
    }

    if (wanted & Resource::RTM_Tuner) {
        enough = false;
        for (i = 0; i < m_tunerArray.size(); i++) {
            if (m_tunerArray[i]->resourceIsEnough(user)) {
                standbyResources[Resource::RT_Tuner] = m_tunerArray[i];
                enough = true;
            }
        }
        if (!enough)
            return false;
    }

    if (wanted & Resource::RTM_Disk) {
        enough = false;
        for (i = 0; i < m_diskArray.size(); i++) {
            if (m_diskArray[i]->resourceIsEnough(user)) {
                standbyResources[Resource::RT_Disk] = m_diskArray[i];
                enough = true;
            }
        }
        if (!enough)
            return false;
    }

    if (wanted & Resource::RTM_Performance) {
		enough = false;
        for (i = 0; i < m_concurrentArray.size(); i++) {
            if (m_concurrentArray[i]->resourceIsEnough(user)) {
                standbyResources[Resource::RT_Performance] = m_concurrentArray[i];
                enough = true;
            }
        }
        if (!enough)
            return false;
    }

    return enough;
}

bool
ResourceManager::allocResource(ResourceUser* user, Resource** standbyResources)
{
    int wanted = user->getRequirement();

    user->setPriority(getPriorityByType(user));

    if (wanted & Resource::RTM_Bandwidth) {
        standbyResources[Resource::RT_Bandwidth]->attachUser(user);
    }

    if (wanted & Resource::RTM_Tuner) {
        standbyResources[Resource::RT_Tuner]->attachUser(user);
    }

    if (wanted & Resource::RTM_Disk) {
        standbyResources[Resource::RT_Disk]->attachUser(user);
    }

    if (wanted & Resource::RTM_Performance) {
        standbyResources[Resource::RT_Performance]->attachUser(user);
    }

    return true;
}

bool
ResourceManager::freeResource(ResourceUser* user)
{
    int wanted = user->getRequirement();
    int i;

    if (wanted & Resource::RTM_Bandwidth) {
        for (i = 0; i < m_networkDeviceArray.size(); i++) { //no useful
            m_networkDeviceArray[i]->detachUser(user);
            break;
        }
    }

    if (wanted & Resource::RTM_Tuner) {
        for (i = 0; i < m_tunerArray.size(); i++) { //no useful
            m_tunerArray[i]->detachUser(user);
            break;
        }
    }

    if (wanted & Resource::RTM_Disk) {
        for (i = 0; i < m_diskArray.size(); i++) { //no useful
            m_diskArray[i]->detachUser(user);
            break;
        }
    }

    if (wanted & Resource::RTM_Performance) {
        for (i = 0; i < m_concurrentArray.size(); i++) {
            m_concurrentArray[i]->detachUser(user);
            break;
        }
    }

    return true;
}

int
ResourceManager::suspendResourceUser(ResourceUser* user)
{
    freeResource(user);

    if (user->canBePreempted()) {
        std::vector<ResourceUser*>::iterator it;
        for (it = m_waitingArray.begin(); it != m_waitingArray.end(); ++it) {
            if (m_priorityMap[user->type()] <= m_priorityMap[(*it)->type()]) {
                m_waitingArray.insert(it, 1, user);
            }
        }

        user->onLoseResource();
    }
    else {
        user->closeForResource();
    }
    return 0;
}

ResourceUser*
ResourceManager::getLowestPriorityUser(std::vector<Resource*>& array)
{
    ResourceUser* user = NULL;
    int i;

    for (i = 0; i < array.size(); i++) {
        ResourceUser* tmp = array[i]->getLowestPriorityUser();
        if (tmp != NULL) {
            if (user == NULL)
                user = tmp;
            else if (getPriorityByType(user) > getPriorityByType(tmp))
                user = tmp;
        }
    }
    return user;
}

int
ResourceManager::getPriorityByType(ResourceUser* user)
{
    return m_priorityMap[user->type()];
}

static ResourceManager gResourceManager;

ResourceManager &resourceManager()
{
    return gResourceManager;
}

} // namespace Hippo

