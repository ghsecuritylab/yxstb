
#include "TunerDevice.h"

#include "ResourceUser.h"


namespace Hippo {

TunerDevice::TunerDevice(int pIndex)
	: m_tunerIndex(pIndex)
	, m_lockedProgramNumber(-1)
{
}

TunerDevice::~TunerDevice()
{
}

Resource::LockState 
TunerDevice::checkTunerIsLock()
{
    LockState CurrentState = RLS_Free;

    if (m_lockedProgramNumber != -1)
        CurrentState = RLS_Locked;

    return CurrentState;
}

bool
TunerDevice::resourceIsEnough(ResourceUser* user)
{
    bool enough = true;

    if ((user->getSpecialDevice() != -1) && (user->getSpecialDevice() != m_tunerIndex))
        return false;

    if (user->getRequiredProgNumber() == m_lockedProgramNumber || m_lockedProgramNumber == -1)
        return enough;

    return false;
}

bool
TunerDevice::tunerResourceIsEnough(int RrogNumber, int userType)
{
    bool enough = true;

	if ((userType == ResourceUser::SimplePlay || userType == ResourceUser::PIP)
		&& Resource::checkUser(userType))
		return true;
	
    if (RrogNumber == m_lockedProgramNumber || m_lockedProgramNumber == -1)
        return enough;

    return false;
}


bool
TunerDevice::attachUser(ResourceUser* user)
{
    m_lockedProgramNumber = user->getRequiredProgNumber();
    return Resource::attachUser(user);
}

bool
TunerDevice::detachUser(ResourceUser* user)
{
    m_lockedProgramNumber = -1;
    return Resource::detachUser(user);
}

} // namespace Hippo
