
#include "tr069_port_errorcode.h"

#include "tmerTask.h"
#include "extendConfig.h"

#include "tr069_api.h"
#include "tr069_port.h"
#include "tr069_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static int gErrorCodeSwitch = 0;
static int gErrorCodeInterval = 300;

#define ERRORCODE_ARRAY_SIZE    64

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct tagErrorCode ErrorCode;
struct tagErrorCode {
    ErrorCode *next;
    int instance;
    unsigned int ErrorCodeTime;
    int ErrorCodeValue;
};

static ErrorCode *gPost = NULL;
static ErrorCode **gPushArray = NULL;
static int gPushIndex = 0;
static TimerTask *gTask = NULL;

static int gEventID = 0;

static void errorCodeTime2str(char *str, int size, time_t t)
{
    struct tm tm;

    memset(&tm, 0, sizeof(tm));

    localtime_r(&t, &tm);
    snprintf(str, size, "%04d-%02d-%02dT%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

static void errorCodeTimer(int arg)
{
    int i;
    char buf[40];
    ErrorCode *ec, *push;

    pthread_mutex_lock(&g_mutex);

    push = NULL;
    for (i = gPushIndex - 1; i >= 0; i--) {
        ec = gPushArray[i];
        if (!ec)
            break;
        gPushArray[i] = NULL;
        ec->next = push;
        push = ec;
    }
    for (i = ERRORCODE_ARRAY_SIZE - 1; i >= gPushIndex; i--) {
        ec = gPushArray[i];
        if (!ec)
            break;
        gPushArray[i] = NULL;
        ec->next = push;
        push = ec;
    }
    TR069Debug("gPost = %p, gPush = %p\n", gPost, push);

    if (gPost) {
        sprintf(buf, "Device.X_00E0FC.ErrorCode.");
        tr069_api_setValue("Event.DeleteObject", buf, 0);
        while (gPost) {
            ec = gPost;
            gPost = gPost->next;
            free(ec);
        }
    }

    if (push) {
        ec = push;
        while (ec) {
            ec->instance = tr069_api_setValue("Event.AddObject", "Device.X_00E0FC.ErrorCode.", 0);
            TR069Debug("instance = %d, errorcode = %d\n", ec->instance, ec->ErrorCodeValue);
            if (ec->instance > 0) {
                sprintf(buf, "Device.X_00E0FC.ErrorCode.%d.", ec->instance);
                tr069_api_setValue("Event.Parameter", buf, gEventID);
            }
            ec = ec->next;
        }
        tr069_api_setValue("Event.Post", "", gEventID);
    }
    gPost = push;
    pthread_mutex_unlock(&g_mutex);
}

static void errorCodeTaskStop(void)
{
    TR069Debug("stop\n");

    timerTaskUnRegist(gTask, errorCodeTimer, 0);
}

static void errorCodeTaskStart(void)
{
    TR069Debug("switch = %d, interval = %d\n", gErrorCodeSwitch, gErrorCodeInterval);

    if(gErrorCodeSwitch && gErrorCodeInterval > 0)
        timerTaskRegist(gTask, gErrorCodeInterval, errorCodeTimer, 0);
}

static int errorCodeGetValue(char *name, char *str, unsigned int size)
{
    int ret = -1;

    if (strncmp("Device.X_00E0FC.", name, 16))
        return -1;
    name += 16;

    pthread_mutex_lock(&g_mutex);
    if (!strcmp(name, "ErrorCodeSwitch")) {
        snprintf(str, size, "%d", gErrorCodeSwitch);
    } else if (!strcmp(name, "ErrorCodeInterval")) {
        snprintf(str, size, "%d", gErrorCodeInterval);
    } else if (!strncmp(name, "ErrorCode.", 10)) {
        int instance;
        char *p;
        ErrorCode *ec;

        if (!gPost)
            goto Err;

        name += 10;
        p = strchr(name, '.');
        if (!p)
            goto Err;
        instance = atoi(name);

        ec = gPost;
        while (ec) {
            if (instance == ec->instance)
                break;
            ec = ec->next;
        }
        if (!ec)
            goto Err;

        name = p + 1;
        if (!strcmp(name, "ErrorCodeTime")) {
            errorCodeTime2str(str, size, ec->ErrorCodeTime);
        } else if (!strcmp(name, "ErrorCodeValue")) {
            snprintf(str, size, "%d", ec->ErrorCodeValue);
        } else {
            TR069Error("Device.X_00E0FC.ErrorCode.%d.%s\n", instance, name);
        }
    }

    ret = 0;
Err:
    pthread_mutex_unlock(&g_mutex);
    return ret;
}

static void errorCodeSetTimer(int arg)
{
    pthread_mutex_lock(&g_mutex);
    extendConfigWrite("errorcode");
    pthread_mutex_unlock(&g_mutex);
}

static int errorCodeSetValue(char *name, char *str, unsigned int x)
{
    int ret, val;

    if (strncmp("Device.X_00E0FC.", name, 16))
        return -1;
    name += 16;

    ret = 0;
    val = atoi(str);

    pthread_mutex_lock(&g_mutex);
    if (!strcmp(name, "ErrorCodeSwitch")) {
        if(0 != val && 1 != val)
            goto Err;
        if(val != gErrorCodeSwitch) {
            gErrorCodeSwitch = val;
            if(gErrorCodeSwitch)
                errorCodeTaskStart( );
            else
                errorCodeTaskStop( );
        }
    } else if (!strcmp(name, "ErrorCodeInterval")) {
        if(val != gErrorCodeInterval) {
            gErrorCodeInterval = val;
            errorCodeTaskStart( );
        }
    } else {
        ret = -1;
    }

Err:
    pthread_mutex_unlock(&g_mutex);

    if (0 == ret)
        timerTaskRegist(gTask, 1, errorCodeSetTimer, 0);

    return ret;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
static int errorCodePost(char *arg0, char *arg1, unsigned int errorcode)
{
    int ret;
    ErrorCode *ec;

    TR069Debug("errorcode = %d\n", errorcode);
    if(!errorcode)
        return -1;

    ret = -1;
    pthread_mutex_lock(&g_mutex);

    ec = gPushArray[gPushIndex];
    if (!ec) {
        ec = (ErrorCode*)calloc(sizeof(ErrorCode), 1);
        if (!ec) {
            TR069Error("calloc errorcode = %d\n", errorcode);
            goto Err;
        }
        gPushArray[gPushIndex] = ec;
    }
    gPushIndex++;
    if (gPushIndex >= ERRORCODE_ARRAY_SIZE)
        gPushIndex = 0;

    ec->ErrorCodeTime = time(NULL);
    ec->ErrorCodeValue = errorcode;

    ret = 0;
Err:
    pthread_mutex_unlock(&g_mutex);
    return ret;
}

void tr069_port_errorCodeInit(void)
{
    TR069Debug("init\n");
    if (gTask)
        return;

    extendConfigInit( );
    extendConfigInsetObject("errorcode");

    extendConfigInsetInt("errorcode.ErrorCodeSwitch",               &gErrorCodeSwitch);
    extendConfigInsetInt("errorcode.ErrorCodeInterval",          &gErrorCodeInterval);

    extendConfigRead("errorcode");
    TR069Printf("ErrorCodeSwitch = %d, ErrorCodeInterval = %d\n", gErrorCodeSwitch, gErrorCodeInterval);

    tr069_api_registFunction("Device.X_00E0FC.ErrorCodeSwitch",     errorCodeGetValue, errorCodeSetValue);
    tr069_api_registFunction("Device.X_00E0FC.ErrorCodeInterval",   errorCodeGetValue, errorCodeSetValue);
    tr069_api_registFunction("Device.X_00E0FC.ErrorCode.",          errorCodeGetValue, errorCodeSetValue);

    tr069_api_registOperation("ErrorCode", errorCodePost);

    gEventID = tr069_api_setValue("Event.Regist", "X CTC ErrorCode", 0);

    gTask = timerTaskCreate( );

    gPushArray = (ErrorCode**)calloc(sizeof(ErrorCode*) * ERRORCODE_ARRAY_SIZE, 1);
}
