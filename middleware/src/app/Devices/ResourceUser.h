#ifndef _ResourceUser_H_
#define _ResourceUser_H_

#include "Resource.h"
#include <string>

#ifdef __cplusplus

namespace Hippo {

class ResourceUser {
public:
    ResourceUser();
    ~ResourceUser();

    enum Type {
    	Unknown,
    	SimplePlay,
    	TimeShift,
    	PIP,
    	PVR,
    	Download,
    	Max
    };

	enum OccupyMode {
    	Alone = 1,
    	Share
    };

    virtual Type type() = 0;

    virtual int getRequirement() = 0;

    virtual float getRequiredBandwidth() { return 0.0; }
    virtual float getCurrentBandwidth() { return 0.0; }
    /*
    *  tuner required by program number not frequence
    */
    virtual int getRequiredProgNumber() { return 0; }
    virtual int getSpecialDevice() { return -1; }

    virtual int getRequiredDiskSpace() { return 0; }
    virtual int getUsedDiskSpace() { return 0; }

    virtual float getPerformanceRate() { return 0.0; }

	virtual int getObjectID(std::string& objectID) { return 0; }

    virtual int onAttachToResource(Resource*) { return 0; }
    virtual int onDetachFromResource(Resource*) { return 0; }

    virtual int onAcquireResource() = 0;
    virtual int onLoseResource() = 0;

    virtual int closeForResource() = 0;

    int priority() { return mPriority; }
    int setPriority(int pPriority) { int old = mPriority; mPriority = pPriority; return old; }

    bool canBePreempted() { return mCanBePreempted; }
    void setCanBePreempted(bool can) { mCanBePreempted = can; }

	int occupyMode() { return mOccupyMode; }
    void setOccupyMode(int mode) { mOccupyMode = mode; }

protected:
    int mPriority;
    bool mCanBePreempted;
	int mOccupyMode;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _ResourceUser_H_
