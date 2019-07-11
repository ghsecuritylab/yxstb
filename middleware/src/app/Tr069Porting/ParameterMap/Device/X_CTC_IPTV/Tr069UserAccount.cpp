#include "Tr069UserAccount.h"

#include "Tr069FunctionCall.h"
#include "MessageTypes.h"
#include "config.h"
#include "NativeHandler.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef TR069_ZERO_CONFIG

static char g_ZeroConfigSerialNum[32] = {0};
static int g_ZeroConfigEnable = -1;

static int getAppTr069PortUserAccountEnable(char* str, unsigned int val)
{
	snprintf(str, val, "%d", g_ZeroConfigEnable);

	return 0;
}

static int setAppTr069PortUserAccountEnable(char* str, unsigned int val)
{
    char p[2] = {0};
    p[0] = *str;
    g_ZeroConfigEnable = atoi(p);
    if(g_ZeroConfigEnable) {
        sendMessageToNativeHandler(MessageType_Tr069, ITMS_OPEN_SERIAL_NUM_BOX, 0, 0);
    }
    return 0;
}

static int getAppTr069PortZeroConfigSerialNum(char* str, unsigned int val)
{
    strncpy(str, g_ZeroConfigSerialNum, val);

    return 0;
}

static int setAppTr069PortZeroConfigSerialNum(char* str, unsigned int val)
{
    strncpy(g_ZeroConfigSerialNum, str, val);

    return 0;
}
#endif//TR069_ZERO_CONFIG

Tr069UserAccount::Tr069UserAccount()
	: Tr069GroupCall("UserAccount")
{
#ifdef TR069_ZERO_CONFIG

    Tr069Call* UserAccountEnable = new Tr069FunctionCall("Enable", getAppTr069PortUserAccountEnable, setAppTr069PortUserAccountEnable);
    Tr069Call* AccountNumber     = new Tr069FunctionCall("AccountNumber", getAppTr069PortZeroConfigSerialNum, setAppTr069PortZeroConfigSerialNum);


    regist(UserAccountEnable->name(), UserAccountEnable);
    regist(AccountNumber->name(), AccountNumber);


#endif//TR069_ZERO_CONFIG
}

Tr069UserAccount::~Tr069UserAccount()
{
}
