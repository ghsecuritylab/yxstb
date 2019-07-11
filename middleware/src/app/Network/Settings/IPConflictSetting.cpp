#include "IPConflictSetting.h"

IPConflictSetting::IPConflictSetting()
    : mReplyTime(10)
    , mConflictTime(1)
    , mUnconflictTime(60)
{
}

IPConflictSetting::~IPConflictSetting()
{
}

IPConflictSetting&
IPConflictSetting::operator = (const IPConflictSetting& rhs)
{
    mReplyTime = rhs.getReplyTime();
    mConflictTime = rhs.getConflictTime();
    mUnconflictTime = rhs.getUnconflictTime();
    return *this;
}

void
IPConflictSetting::setReplyTime(int replyTime)
{
   mReplyTime = replyTime;
}

const int
IPConflictSetting::getReplyTime() const
{
    return mReplyTime;
}

void
IPConflictSetting::setConflictTime(int conflictTime)
{
   mConflictTime = conflictTime;
}

const int
IPConflictSetting::getConflictTime() const
{
    return mConflictTime;
}

void
IPConflictSetting::setUnconflictTime(int unconflictTime)
{
   mUnconflictTime = unconflictTime;
}

const int
IPConflictSetting::getUnconflictTime() const
{
    return mUnconflictTime;
}
