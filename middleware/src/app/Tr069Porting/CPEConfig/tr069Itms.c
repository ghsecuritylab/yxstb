#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SysSetting.h"
#include "Session.h"
#include "app_sys.h"
#include "app_epg_para.h"

#include "ind_mem.h"
#include "ind_string.h"
#include "config.h"

#include "MessageTypes.h"
#include "NativeHandler.h"
#include "Assertions.h"
#include "TR069Assertions.h"

#include "tr069_api.h"
//#include "tr069Itms.h"

static int g_itms_status = 0;

int itms_start(void)
{
	int tr069Enable = 0;
    sysSettingGetInt("tr069_enable", &tr069Enable, 0);
	if(PLATFORM_ZTE == SessionGetPlatform())
        tr069Enable = 1;

    if(tr069Enable == 0) {
        LogTr069Error("[DBG]not enable\n");
        return -1;
    }

    if(g_itms_status == 1) {
        LogTr069Error("[DBG]already start\n");
        return -1;
    }

    g_itms_status = 1;
#ifndef INCLUDE_HMWMGMT
    tr069_api_setValue("Config.STUNUsername", NULL, 1);
#endif
    LogTr069Debug("[itms_start]\n");
    sendMessageToNativeHandler(MessageType_Tr069, TR069_MSG_START, 0, 0);
    return 0;
}

int itms_stop(void)
{
    int tr069Enable = 0;
    sysSettingGetInt("tr069_enable", &tr069Enable, 0);
    if(tr069Enable) {
        LogTr069Error("[DBG]not enable\n");
        return -1;
    }

    g_itms_status = 0;

    LogTr069Debug("[itms_stop]\n");
    sendMessageToNativeHandler(MessageType_Tr069, TR069_MSG_STOP, 0, 0);
    return 0;
}

int itms_isstarted(void)
{
    return g_itms_status;
}

