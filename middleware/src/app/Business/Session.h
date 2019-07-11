#ifndef Session_h
#define Session_h

#include "sys_basic_macro.h"

#ifdef __cplusplus

class Session {
public:
    Session();
    ~Session();

    int getUserStatus(void);
    void setUserStatus(int);
	char* getUserGroupNMB();
    int setUserGroupNMB(const char *);

	char* getCnonce(void);

    char* getPlatformCode();
    int setPlatformCode(const char *);
    int getPlatform(void);
    void setPlatform(int);

    int setCookie(char *);

private:
    static int s_userStatus;
    static char s_userGroupNMB[EPG_SERVICEENTRY_MAX_NUM];
    static char s_platformCode[4 + 1];
    static int s_platform;
};

Session &session();

#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif

char* SessionGetPlatformCode();
int SessionSetPlatformCode(const char *);
int SessionGetPlatform(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // Session_h
