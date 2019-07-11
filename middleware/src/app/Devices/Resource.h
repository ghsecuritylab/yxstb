#ifndef _Resource_H_
#define _Resource_H_

#ifdef __cplusplus

#include <vector>

namespace Hippo {

class ResourceUser;

class Resource {
public:
    enum Type {
    	RT_Bandwidth = 0,
    	RT_Tuner,
    	RT_Disk,
    	RT_Performance,
    	RT_Max
    };
    enum TypeMask {
    	RTM_Bandwidth   = 0x01 << RT_Bandwidth,
    	RTM_Tuner       = 0x01 << RT_Tuner,
    	RTM_Disk        = 0x01 << RT_Disk,
    	RTM_Performance = 0x01 << RT_Performance
    };

    virtual Type type() = 0;
    virtual int index() { return 0; }

    virtual float getFreeBandwidth() { return 0; }
    virtual int getFreeDiskSpace() { return 0; }

	virtual int getFreeConcurrentNumber() { return 0; }
	virtual int getConcurrentNumber() { return 0; }
	
    enum LockState {
    	RLS_Free,
    	RLS_Locked,
    	RLS_Unusable
    };

    virtual LockState checkTunerIsLock() { return RLS_Unusable; }

    virtual bool resourceIsEnough(ResourceUser*) { return false; }
	virtual ResourceUser* resourceCanBeShared(ResourceUser*) { return NULL; }
	virtual bool tunerResourceIsEnough(int, int) { return false; }
    virtual bool attachUser(ResourceUser* user);
    virtual bool detachUser(ResourceUser* user);
	virtual bool checkUser(int userType);
	
	int userCount() { return m_resourceUserArray.size(); }
	ResourceUser* getUserByIndex(int idx);
    ResourceUser* getLowestPriorityUser();
    ResourceUser* getHighestPriorityUser();

protected:
    std::vector<ResourceUser*> m_resourceUserArray;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _Resource_H_
