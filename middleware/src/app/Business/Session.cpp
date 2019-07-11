
#include "Session.h"
#include "Assertions.h"

#include "mid_stream.h"
#include "mid_sys.h"
#include "mid/mid_time.h"
#include "sys_basic_macro.h"
#include "app_sys.h"
#include "preconfig.h"

#include <string.h>
#include <stdlib.h>

char Session::s_platformCode[4 + 1] = {0};
int Session::s_platform = DEFAULT_PLATFORM;
int Session::s_userStatus = 10;
char Session::s_userGroupNMB[EPG_SERVICEENTRY_MAX_NUM] = {0};

static Session g_session;

Session::Session()
{
}

Session::~Session()
{
}

void
Session::setUserStatus(int value)
{
    s_userStatus = value;
    return;
}

int
Session::getUserStatus(void)
{
    return s_userStatus;
}

int
Session::setUserGroupNMB(const char *buf)
{
    if(NULL == buf)
        ERR_OUT("The UserGroupNMB is NULL!\n");
    if(strcmp(s_userGroupNMB, buf))
        strcpy(s_userGroupNMB, buf);
    return 0;
Err:
    return -1;
}

char*
Session::getUserGroupNMB()
{
    return s_userGroupNMB;
}

/*EPG页面挑战认证所需cnonce参数随机生成*/
static char cnonce[128] = "\0";
void app_cnonce_create(void)
{
    char serial[33] = {0};
    int date;
    mid_sys_serial(serial);
    srand(mid_clock());
    date = rand();
    snprintf(cnonce, 17, "%04x%s\n", date, serial + 9);
    LogSafeOperDebug("cnonce[%s]\n", cnonce);
}

char*
Session::getCnonce(void)
{
    if(*cnonce == '\0') {
        app_cnonce_create();
    }
    return cnonce;
}

char*
Session::getPlatformCode()
{
    return s_platformCode;
}

int
Session::setPlatformCode(const char *buf)
{
    if(NULL == buf) {
        LogSafeOperError("The PlatformCode is NULL!\n");
        return -1;
    }

    if(strncmp(s_platformCode, buf, 4))
        strncpy(s_platformCode, buf, 4);
    s_platformCode[4] = '\0';
    if(strcmp(s_platformCode , "0200"))
        mid_stream_standard(RTSP_STANDARD_CTC_GUANGDONG);
    return 0;
}

int
Session::getPlatform(void)
{
    LogSafeOperDebug("Platform [%d]\n", s_platform);
    return s_platform;
}

void
Session::setPlatform(int platform_t)
{
    LogSafeOperDebug("Platform [%d]\n", platform_t);

    if((PLATFORM_ZTE != platform_t) && (PLATFORM_HW != platform_t)) {
        return;
    }

    s_platform = platform_t;
    return;
}

char* global_cookies = NULL;
char buf[128] = {0};
int
Session::setCookie(char *value)
{
	if(value[0] != '\0') {
		snprintf(buf, 128, "Cookie: JSESSIONID=%s", value);
		global_cookies = buf;
	}
    return 0;
}


Session &session()
{
    return g_session;
}

extern "C" {

int SessionSetPlatformCode(const char *buf)
{
    session().setPlatformCode(buf);
    return 0;
}

char* SessionGetPlatformCode()
{
    return session().getPlatformCode();
}

int SessionGetPlatform(void)
{
    return session().getPlatform();
}

}//extern "C"

