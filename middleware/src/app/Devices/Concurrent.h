#ifndef _Concurrent_H_
#define _Concurrent_H_

#include "Device.h"
#include "Resource.h"

#ifdef __cplusplus

namespace Hippo {

class Concurrent : public Device, public Resource {
public:
    Concurrent(int num);
    ~Concurrent();

    virtual Type type() { return RT_Performance; }
	virtual int getFreeConcurrentNumber();
    virtual int setConcurrentNumber(int);
    virtual int getConcurrentNumber(void);
	
    virtual bool resourceIsEnough(ResourceUser *);
	virtual ResourceUser* resourceCanBeShared(ResourceUser*);
	
    virtual bool attachUser(ResourceUser *user);
    virtual bool detachUser(ResourceUser *user);

protected:
    int mConcurrentNumber;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NetworkDevice_H_
