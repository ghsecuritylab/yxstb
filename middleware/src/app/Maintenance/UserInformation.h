
#ifndef ANDROID
#ifndef UserInfomation_h
#define UserInfomation_h

#define DEFAULT_USER "huawei"
#define DEFAULT_PASSWD ".287aW"

#ifdef __cplusplus
extern "C" {
#endif

// User Account
int  checkUserAccountInfo(const char* user, const char* pswd);
void getUserAccountInfo(char* user, int userlen, char* pswd, int pswdlen);
void resetUserAccountInfo(void);
void saveUserAccountInfo(const char* user, const char* pswd);

#ifdef __cplusplus
} // extern "C"
#endif 

#endif // UserInfomation_h

#endif // #ifndef ANDROID

