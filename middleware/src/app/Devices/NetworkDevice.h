#ifndef _NetworkDevice_H_
#define _NetworkDevice_H_

#include "Device.h"
#include "Resource.h"

#ifdef __cplusplus

namespace Hippo {

class NetworkDevice : public Device, public Resource {
public:
    NetworkDevice(float bandwidth);
    ~NetworkDevice();

    virtual Type type() { return RT_Bandwidth; }

    virtual float getFreeBandwidth();

    virtual int setNetworkBandwidth(float bandwidth);
	
    virtual int getNetworkBandwidth(void);
	
    virtual bool resourceIsEnough(ResourceUser *);
	virtual ResourceUser* resourceCanBeShared(ResourceUser*);
    virtual bool attachUser(ResourceUser *user);
    virtual bool detachUser(ResourceUser *user);

protected:
    float mBandwidth;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NetworkDevice_H_
