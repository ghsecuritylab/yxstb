#ifndef ANDROID

#include "UserInformation.h"

#include "tools.h"
#include "Assertions.h"
//#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
//#include <crypt.h>;

#define TRUE_USER 1
#define FALSE_USER 0

#ifdef __cplusplus
extern "C" {
#endif

void saveUserAccountInfo(const char* user, const char* pswd)
{
    ASSERT(user != NULL);
    ASSERT(pswd != NULL);

    if (!user || !pswd)
        return;

    char buf[1024 * 5] = {0};
    char lineBuf[1024] = {0};
    int n = 0;
    int firstLine = 0;
    char* result = crypt(pswd, "$1$$");

    n = snprintf(buf, 1024, "%s:%s:0:0:%s:/root:/bin/sh\n", user, result, user);

    FILE* fp;
    if ((fp = fopen("/etc/passwd", "r")) == NULL) {
        LogSafeOperError("fopen (/etc/passwd) failed: %s\n", strerror(errno));
        return;
    }

    while (fgets(lineBuf, 1024, fp) != NULL) {
        if (firstLine == 1) {
            if (n + strlen(lineBuf) < 1024 * 5)
                n += snprintf(buf + n, 1024, "%s", lineBuf);
        }
        firstLine = 1;
    }
    fclose(fp);
    fp = NULL;

    if ((fp = fopen("/etc/passwd", "w")) == NULL) {
        LogSafeOperError("fopen (/etc/passwd) failed: %s\n", strerror(errno));
        return;
    }
    fwrite(buf, 1, strlen(buf), fp);
    fclose(fp);
}

void resetUserAccountInfo(void)
{
    saveUserAccountInfo(DEFAULT_USER, DEFAULT_PASSWD);
}

// tanf todo:qu diao
static int checkDefaultUserAccountInfo(const char* user, const char* pswd)
{
    ASSERT(user != NULL || pswd != NULL);

    if (user == NULL && pswd == NULL)
        return FALSE_USER;
    if (user != NULL)
        if (strcmp(user, DEFAULT_USER) != 0)
            return FALSE_USER;
    if (pswd != NULL)
        if (strcmp(pswd, DEFAULT_PASSWD) != 0)
            return FALSE_USER;


    return TRUE_USER;
}

int checkUserAccountInfo(const char* user, const char* pswd)
{

    ASSERT(user != NULL || pswd != NULL);

    if (user == NULL && pswd == NULL)
        return FALSE_USER;

    if (!IsFileExists("/etc/passwd")) {
        resetUserAccountInfo();
        return checkDefaultUserAccountInfo(user, pswd);
    }

    char buf[1025] = {'\0'};
    FILE* fp;
    int ret;

    if ((fp = fopen("/etc/passwd", "rb")) == NULL) {
        LogSafeOperError("fopen (/etc/passwd) failed: %s\n", strerror(errno));
        return FALSE_USER;
    }
    ret = fread(buf, 1, 1024, fp);
    buf[ret] = '\0';
    fclose(fp);

    char* p = strchr(buf, ':');
    if (!p) {
        resetUserAccountInfo();
        return checkDefaultUserAccountInfo(user, pswd);
    }

    *p = '\0';
    p ++;

    char* p1 = strchr(p, ':');
    if (p1 == NULL)
        p1 = p + strlen(p);
    *p1 = '\0';

    if (user) {
        if (strcmp(user, buf) != 0)
            return FALSE_USER;
    }
    if (pswd) {
        const char* salt = "$1$$";
        char* result = crypt(pswd, salt);
        if (strcmp(result, p) != 0)
            return FALSE_USER;
    }
    return TRUE_USER;
}

// you can't get login password by this function.
void getUserAccountInfo(char* user, int userlen, char* pswd, int pswdlen)
{
    ASSERT(user != NULL || pswd != NULL);

    if (!user && !pswd)
        return;
    if (!IsFileExists("/etc/passwd")) {
        resetUserAccountInfo();
        if (user)
            snprintf(user, userlen, DEFAULT_USER);
        if (pswd)
            snprintf(pswd, pswdlen, DEFAULT_PASSWD);
        return;
    }

    char buf[1025] = {'\0'};
    FILE* fp;
    int ret;
    if ((fp = fopen("/etc/passwd", "rb")) == NULL) {
        LogSafeOperError("fopen (/etc/passwd) failed: %s", strerror(errno));
        if (user)
            snprintf(user, userlen, DEFAULT_USER);
        if (pswd)
            snprintf(pswd, pswdlen, DEFAULT_PASSWD);
        return;
    }
    ret = fread(buf, 1, 1024, fp);
    buf[ret] = '\0';
    fclose(fp);

    char* p  = strchr(buf, ':');
    if (!p) {
        resetUserAccountInfo();
        if (user)
            snprintf(user, userlen, DEFAULT_USER);
        if (pswd)
            snprintf(pswd, pswdlen, DEFAULT_PASSWD);
        return;
    }

    *p = '\0';
    p++;
    char* p1 = strchr(p, ':');
    if (!p1)
        p1 = p + strlen(p);
    *p1 = '\0';
    if (user)
        snprintf(user, userlen, "%s", buf);
    if (pswd)
        strcpy(pswd, "");
}

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // #ifndef ANDROID

