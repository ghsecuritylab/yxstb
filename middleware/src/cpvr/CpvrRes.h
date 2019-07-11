#ifndef	CPVR_RESOURCE_H
#define CPVR_RESOURCE_H

#include "ResourceManager.h"
#include "ResourceUser.h"
#include <Hippo_HString.h>

#ifdef __cplusplus

#include <string>

namespace Hippo {

class CpvrRes : public ResourceUser {
public:

    CpvrRes();
    ~CpvrRes();

    std::string mScheduleID;
    ResourceUser::Type type() { return mResourceUserType; }
    void setType(ResourceUser::Type T) { mResourceUserType = T; }

    int getRequirement() { return mResourceRequirement; }
    void setRequirement(int ResReq) { mResourceRequirement = ResReq; }

    float getRequiredBandwidth() { return mRequiredBandwidth; }
    void setRequiredBandwidth(float Bandwidth) { mRequiredBandwidth = Bandwidth; }

    int getRequiredFrequency() { return mRequiredFrequency; }
    void setRequiredFrequency(int Freq) { mRequiredFrequency = Freq; }

	int getRequiredProgNumber() { return mRequiredProgNumber; }
    void setRequiredProgNumber(int ProgNumber) { mRequiredProgNumber = ProgNumber; }

    int getRequiredDiskSpace() { return mRequiredDiskSpace; }
    void setRequiredDiskSpace(int DiskSpace) { mRequiredDiskSpace = DiskSpace; }

    int onAcquireResource();
    int onLoseResource();
    int closeForResource();

protected:
    ResourceUser::Type mResourceUserType;
    int mResourceRequirement;
    float mRequiredBandwidth;
    int mRequiredFrequency;
    int mRequiredDiskSpace;
	int mRequiredProgNumber;
};

} // namespace Hippo

#endif // __cplusplus

#endif

