#ifndef _ResourceManager_H_
#define _ResourceManager_H_

#include "ResourceUser.h"
#include "json/json_public.h"

#ifdef __cplusplus

#include <vector>
#include <map>

namespace Hippo {

class Resource;
class ResourceUser;

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    bool addNetworkResource(Resource*);
    bool removeNetworkResource(Resource*);

    bool addConcurrentResource(Resource*);
    bool removeConcurrentResource(Resource*);
	int  concurrentResourceCount();
	int idleConcurrentResourceCount();
	bool getConcurrentResourceDetail(json_object *jsonTunerRes);
	
    bool addTunerResource(Resource*);
    bool removeTunerResource(Resource*);
    int tunerResourceCount() { return m_tunerArray.size(); }
	int idleTunerResourceCount();
    bool tunerResourceIsEnough(int chNum, int userType);
    bool addDiskResource(Resource*);
    bool removeDiskResource(Resource*);

    bool requestResource(ResourceUser*);
    bool releaseResource(ResourceUser*);

    bool addUserToWaitingArray(ResourceUser*);

	bool getTunerResourceDetail(json_object *jsonTunerRes);

protected:
    bool resourceIsEnough(ResourceUser*, Resource**);
    bool allocResource(ResourceUser*, Resource**);
    bool freeResource(ResourceUser*);

    int suspendResourceUser(ResourceUser*);

    ResourceUser* getLowestPriorityUser(std::vector<Resource*>& array);

    int getPriorityByType(ResourceUser*);

    std::vector<Resource*> m_networkDeviceArray;
    std::vector<Resource*> m_tunerArray;
    std::vector<Resource*> m_diskArray;
	std::vector<Resource*> m_concurrentArray;
    std::vector<ResourceUser*> m_waitingArray;

public:
    int m_hightPriority;
    int m_priorityStep;
    int m_priorityMap[ResourceUser::Max];
};

ResourceManager &resourceManager();

} // namespace Hippo

#endif // __cplusplus

#endif // _ResourceManager_H_
