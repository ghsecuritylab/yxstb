#ifndef _TunerDevice_H_
#define _TunerDevice_H_

#include "Device.h"
#include "Resource.h"

#ifdef __cplusplus

namespace Hippo {

class TunerDevice : public Device, public Resource {
public:
    TunerDevice(int pIndex);
    ~TunerDevice();

    virtual Type type() { return RT_Tuner; }
    virtual int index() { return m_tunerIndex; }

    virtual LockState checkTunerIsLock();

    virtual bool resourceIsEnough(ResourceUser*);
    virtual bool attachUser(ResourceUser*);
    virtual bool detachUser(ResourceUser*);
	virtual bool tunerResourceIsEnough(int, int);
protected:
    int m_tunerIndex;
    int m_lockedProgramNumber;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _TunerDevice_H_
