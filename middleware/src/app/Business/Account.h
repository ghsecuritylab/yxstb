#ifndef Account_h
#define Account_h

#include "sys_basic_macro.h"

#ifdef __cplusplus

class Account {
public:
    Account();
    ~Account();

	int setEncryptionType(const char *buf);
	char* getEncryptionType();
    int setShareKey(const char *buf);
    char* getShareKey(void);


private:
	static char s_encryptionType[4 + 1];
    static char s_shareKey[AREAID_LEN];

};

Account &account();
#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif

int AccountSetShareKey(const char *buf);
char* AccountGetShareKey(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // Account_h
