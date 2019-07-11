
#include "Account.h"
#include "sys_basic_macro.h"

#include "Assertions.h"

#include <string.h>

char Account::s_encryptionType[4 + 1] = {0};
char Account::s_shareKey[AREAID_LEN] = {0};

static Account g_account;

Account::Account()
{
}

Account::~Account()
{
}

int
Account::setEncryptionType(const char *buf)
{
    if(NULL == buf) {
        LogSafeOperError("The EncryptionType is NULL!\n");
        return -1;
    }

    if(strncmp(s_encryptionType, buf, 4))
        strncpy(s_encryptionType, buf, 4);
    s_encryptionType[4] = '\0';

    return 0;
}

char*
Account::getEncryptionType()
{
    return s_encryptionType;
}

int
Account::setShareKey(const char *buf)
{
    if(NULL == buf)
        ERR_OUT("The ShareKey is NULL!\n");
    if(strcmp(s_shareKey, buf))
        strcpy(s_shareKey, buf);
    return 0;
Err:
    return -1;
}

char*
Account::getShareKey(void)
{
    return s_shareKey;
}

Account &account()
{
    return g_account;
}

int AccountSetShareKey(const char *buf)
{
    return account().setShareKey(buf);
}

char* AccountGetShareKey(void)
{
    return account().getShareKey();
}

