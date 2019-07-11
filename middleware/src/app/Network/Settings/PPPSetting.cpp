#include "PPPSetting.h"
#include <string.h>

PPPSetting::PPPSetting()
    : mUsername("none")
    , mPassword("none")
    , mRetryTimes(3)
{
}

PPPSetting::~PPPSetting()
{

}

PPPSetting&
PPPSetting::operator = (const PPPSetting& rhs)
{
    mUsername = rhs.getUsername();
    mPassword = rhs.getPassword();
    mRetryTimes = rhs.getRetryTimes();
    return *this;
}

void
PPPSetting::setUsername(const char* username)
{
    if (username && strlen(username) > 0)
        mUsername = username;
}

const char*
PPPSetting::getUsername() const
{
    return mUsername.c_str();
}

void
PPPSetting::setPassword(const char* password)
{
    if (password && strlen(password) > 0)
        mPassword = password;
}

const char*
PPPSetting::getPassword() const
{
    return mPassword.c_str();
}

void
PPPSetting::setRetryTimes(int times)
{
    mRetryTimes = times;
}

const int
PPPSetting::getRetryTimes() const
{
    return mRetryTimes;
}
