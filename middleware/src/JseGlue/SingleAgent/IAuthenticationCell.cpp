#include "JseGlueAssertions.h"
#include "IAuthenticationCell.h"
#include "Hippo_Context.h"
#include <Hippo_HString.h>

#include <stdio.h>
#include <string.h>

using Hippo::HippoContext;
using Hippo::HString;

int 
IAuthenticationCell::CTCGetAuthInfo(const char* pEncToken, char* info)
{
    HString token = pEncToken;
    HString out;
    HippoContext::getContextInstance()->AuthenticationCTCGetAuthInfo(token, out);
    strncpy(info, out.c_str(), IJSEBUFFSIZE);
    info[IJSEBUFFSIZE] = 0;

    LogJseGlueDebug("<%s, %s>\n", pEncToken, info);
    return 0;
}

int 
IAuthenticationCell::CTCSetConfig(const char* name, const char* value)
{
    HString fieldName = name;
    HString fieldValue = value;
    HippoContext::getContextInstance()->AuthenticationCTCSetConfig(fieldName, fieldValue);

    LogJseGlueDebug("<%s, %s>\n", name, value);
    return 0;
}

int
IAuthenticationCell::CTCGetConfig(const char* name, char* value)
{
    HString fieldName = name;
    HString fieldValue;
    HippoContext::getContextInstance()->AuthenticationCTCGetConfig(fieldName, fieldValue); 
    strncpy(value, fieldValue.c_str(), IJSEBUFFSIZE);
    value[IJSEBUFFSIZE] = 0;

    LogJseGlueDebug("<%s, %s>\n", name, value);
    return 0;
}

int 
IAuthenticationCell::CTCStartUpdate()
{
    HString fieldName = "CTCStartUpdate";
    HString fieldValue;
    HippoContext::getContextInstance()->ioctlWrite(fieldName, fieldValue);
    return 0;
}

int 
IAuthenticationCell::Login()
{
    HString fieldName = "CTCLogin";
    HString fieldValue;
    HippoContext::getContextInstance()->ioctlWrite(fieldName, fieldValue);
    return 0;
}

int 
IAuthenticationCell::ioctlRead(const char* name, char* value)
{
    HString fieldName = name; 
    HString fieldValue;

    HippoContext::getContextInstance()->ioctlRead(fieldName, fieldValue);
    strncpy(value, fieldValue.c_str(), IJSEBUFFSIZE);
    value[IJSEBUFFSIZE] = 0;

    LogJseGlueDebug("<%s, %s>\n", name, value);
    return 0;
}

int 
IAuthenticationCell::ioctlWrite(const char* name, const char* value)
{
    HString fieldName = name;
    HString fieldValue = value;

    HippoContext::getContextInstance()->ioctlWrite(fieldName, fieldValue);

    LogJseGlueDebug("<%s, %s>\n", name, value);
    return 0;
}

int 
IAuthenticationCell::CUGetAuthInfo(const char* pEncToken, char* info)
{
    HString token = pEncToken;
    HString out;

    HippoContext::getContextInstance()->AuthenticationCUGetAuthInfo(token, out);
    strncpy(info, out.c_str(), IJSEBUFFSIZE);
    out[IJSEBUFFSIZE] = 0;

    LogJseGlueDebug("<%s, %s>\n", pEncToken, info);
    return 0;
}

int 
IAuthenticationCell::CUSetConfig(const char* name, const char* value)
{
    HString fieldName = name;
    HString fieldValue = value;

    HippoContext::getContextInstance()->AuthenticationCUSetConfig(fieldName, fieldValue);

    LogJseGlueDebug("<%s, %s>\n", name, value);
    return 0;
}

int 
IAuthenticationCell::CUGetConfig(const char* name, char* value)
{
    HString fieldName = name;
    HString fieldValue;

    HippoContext::getContextInstance()->AuthenticationCUGetConfig(fieldName, fieldValue);
    strncpy(value, fieldValue.c_str(), IJSEBUFFSIZE);
    value[IJSEBUFFSIZE] = 0;

    LogJseGlueDebug("<%s, %s>\n", name, value);
    return 0;
}

IAuthenticationCell::IAuthenticationCell()
{
}

//export
IAuthentication* createIAuthentication(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);
    return new IAuthenticationCell;
}

