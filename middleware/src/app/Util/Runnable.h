#ifndef _RUNNABLE_H_
#define _RUNNABLE_H_

#include "Object.h"

#ifdef __cplusplus

namespace Hippo {

class Runnable : public Object {
public:
    Runnable() {};
    virtual ~Runnable() {};
    virtual void run() = 0;
};

} /* namespace Hippo */

#endif // __cplusplus

#endif /* _RUNNABLE_H_ */
