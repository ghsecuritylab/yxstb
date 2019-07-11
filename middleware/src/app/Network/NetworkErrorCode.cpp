#include "NetworkErrorCode.h"

NetworkErrorCode::NetworkErrorCode(int codeid, int showmode, const char* message, int promptid)
    : mCodeId(codeid)
    , mShowMode(showmode)
    , mPromptId(promptid)
    , mMessage("")
{
    if (message)
        mMessage.assign(message);
}

NetworkErrorCode::NetworkErrorCode()
    : mCodeId(0), mShowMode(1)
    , mMessage("")
{
}

NetworkErrorCode::~NetworkErrorCode()
{
    mMessage.clear();
}

void
NetworkErrorCode::setCodeId(int codeid)
{
     mCodeId = codeid;
}

int
NetworkErrorCode::getCodeId() const
{
    return mCodeId;
}

void
NetworkErrorCode::setShowMode(int showmode)
{
    mShowMode = showmode;
}

int
NetworkErrorCode::getShowMode() const
{
    return mShowMode;
}

void
NetworkErrorCode::setMessage(const char* message)
{
    mMessage.assign(message);
}

const char*
NetworkErrorCode::getMessage() const
{
    return mMessage.c_str();
}

void 
NetworkErrorCode::setPromptId(int promptid)
{
    mPromptId = promptid;
}

int 
NetworkErrorCode::getPromptId() const
{
    return mPromptId;
}

bool 
NetworkErrorCode::isAuthFail(int hycode)
{
    return ((hycode & 0x00FF) == to_positive_sign(PPPOE_AUTH_FAILED)) ? true : false;
}
