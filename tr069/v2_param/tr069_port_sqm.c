
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

static unsigned int gSQMLisenPort = 37001;
static unsigned int gSQMServerPort = 37000;

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static TimerTask *gTask = NULL;

static void errorCodeSetTimer(int arg)
{
    pthread_mutex_lock(&g_mutex);
    extendConfigWrite("sqm");
    pthread_mutex_unlock(&g_mutex);
}

static int sqmGetValue(char *name, char *str, unsigned int size)
{
    int ret = -1;

    if (strncmp("Device.X_00E0FC.SQMConfiguration.", name, 33))
        return -1;
    name += 33;

    pthread_mutex_lock(&g_mutex);
    if (!strcmp(name, "SQMLisenPort")) {
        snprintf(str, size, "%d", gSQMLisenPort);
    } else if (!strcmp(name, "SQMServerPort")) {
        snprintf(str, size, "%d", gSQMServerPort);
    } else {
        TR069Error("name = Device.X_00E0FC.SQMConfiguration.%s\n", name);
        goto Err;
    }

    ret = 0;
Err:
    pthread_mutex_unlock(&g_mutex);
    return ret;
}

static int sqmSetValue(char *name, char *str, unsigned int x)
{
    int ret, val;

    val = atoi(str);
    if (val < 0 || val > 65535) {
        TR069Error("%s = %s\n", name, str);
        return -1;
    }

    if (strncmp("Device.X_00E0FC.SQMConfiguration.", name, 33))
        return -1;
    tr069_port_setValue(name, str, x);

    name += 33;

    ret = 0;
    pthread_mutex_lock(&g_mutex);
    if (!strcmp(name, "SQMLisenPort")) {
        gSQMLisenPort = val;
    } else if (!strcmp(name, "SQMServerPort")) {
        gSQMServerPort = val;
    } else {
        TR069Error("name = Device.X_00E0FC.SQMConfiguration.%s\n", name);
        ret = -1;
    }

    pthread_mutex_unlock(&g_mutex);

    if (0 == ret)
        timerTaskRegist(gTask, 1, errorCodeSetTimer, 0);

    return ret;
}

void tr069_port_sqmInit(void)
{
    TR069Debug("init\n");
    if (gTask)
        return;

    extendConfigInit( );
    extendConfigInsetObject("sqm");

    extendConfigInsetUnsigned("sqm.SQMLisenPort",           &gSQMLisenPort);
    extendConfigInsetUnsigned("sqm.SQMServerPort",          &gSQMServerPort);

    extendConfigRead("sqm");
    TR069Printf("gSQMLisenPort = %d, gSQMServerPort = %d\n", gSQMLisenPort, gSQMServerPort);

    tr069_api_registFunction("Device.X_00E0FC.SQMConfiguration.SQMLisenPort",   sqmGetValue, sqmSetValue);
    tr069_api_registFunction("Device.X_00E0FC.SQMConfiguration.SQMServerPort",  sqmGetValue, sqmSetValue);

    gTask = timerTaskCreate( );
}
