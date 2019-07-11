#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_

#ifdef __cplusplus

#include <vector>

#include "CpvrRes.h"

namespace Hippo {

class CpvrRes;

class UserManager {
public:

    UserManager();
    ~UserManager();

    virtual bool attachUser(CpvrRes *user);
    virtual bool detachUser(CpvrRes *user);
    virtual CpvrRes *getUserByTaskID(char *scheduleID);

protected:
    std::vector<CpvrRes*> mCpvrResUserArray;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _USER_MANAGER_H_

