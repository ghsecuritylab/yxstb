#include "CpvrAssertions.h"
#include "CpvrRes.h"
#include "UserManager.h"

#include <string.h>

namespace Hippo {

UserManager::UserManager()
{
}

UserManager::~UserManager()
{
}

bool
UserManager::attachUser(CpvrRes *user)
{
    int i;

    if (user == NULL)
        return false;

    LogCpvrDebug("attach user, schedule id is %s\n", user->mScheduleID.c_str());
    for (i = 0; i < mCpvrResUserArray.size(); i++) {
        if (mCpvrResUserArray[i] == user)
            return true;
    }
    mCpvrResUserArray.push_back(user);
    return true;
}

bool
UserManager::detachUser(CpvrRes *user)
{
    std::vector<CpvrRes *>::iterator it;

    LogCpvrDebug("detach user, schedule id is %s\n", user->mScheduleID.c_str());
    for (it = mCpvrResUserArray.begin(); it != mCpvrResUserArray.end(); ++it) {
        if (user == *it) {
            mCpvrResUserArray.erase(it);
            break;
        }
    }

    return true;
}

CpvrRes *
UserManager::getUserByTaskID(char *scheduleID)
{
    std::vector<CpvrRes *>::iterator it;

    for (it = mCpvrResUserArray.begin(); it != mCpvrResUserArray.end(); ++it) {
        if (strncmp(scheduleID, (*it)->mScheduleID.c_str(), strlen(scheduleID)) == 0) {
            return *it;
        }
    }

    return NULL;
}

} // namespace Hippo

