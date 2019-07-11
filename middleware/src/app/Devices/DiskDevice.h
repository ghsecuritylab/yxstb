#ifndef _DiskDevice_H_
#define _DiskDevice_H_

#include "Device.h"
#include "Resource.h"

#ifdef __cplusplus

namespace Hippo {

class DiskDevice : public Device, public Resource {
public:
    DiskDevice(int diskSpace);
    ~DiskDevice();

    virtual Type type() { return RT_Disk; }

    virtual int getFreeDiskSpace();

    virtual bool resourceIsEnough(ResourceUser *);

    virtual bool attachUser(ResourceUser *user);
    virtual bool detachUser(ResourceUser *user);

protected:
    int mDiskSpace;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _DiskDevice_H_
