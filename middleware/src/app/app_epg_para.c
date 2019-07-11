/*************************************************
  Copyright (C), 1988-2008, yuxing software. Co., Ltd.
Description:   ���ļ���Ҫ�����������ݣ�
1��md5 �㷨
2��ҵ�����������غ���
3��EDS ����
4��EPG ��ҳ����
5������״̬���
 *************************************************/
#include "Assertions.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <openssl/md5.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "app_include.h"

#include "sys_basic_macro.h"
#include "sys_msg.h"
#include "browser_event.h"

#include "config/webpageConfig.h"
#include "app_sys.h"
#include "ind_mem.h"
#include "Business.h"

#include "Tr069.h"

#include "ntp/mid_ntp.h"
#include "mid/mid_mutex.h"

#include "MessageTypes.h"
#include "MessageValueSystem.h"

#include "build_info.h"

#include "NetworkFunctions.h"

#define MIDDLEWARE_VERSION "10"
#define CHANNEL_VERSION "0"

static mid_mutex_t g_app_mutex = NULL;
static char g_app_stb_monitor_tms_url[LARGE_URL_MAX_LEN + 4] = {0};

/*************************************************
Description:��ʼ��Ӧ�ò�
Input:       ��
output:     ��
Return:     ��
 *************************************************/
void app_Init(void)
{
    g_app_mutex = mid_mutex_create();
    if(g_app_mutex == NULL)
        END_OUT("g_app_mutex is null.\n");
        TR069_STATISTIC_START();
End:
    return;
}

void app_stbmonitor_tms_url_set(const char* url)
{
    if(mid_mutex_lock(g_app_mutex))
        LogSafeOperWarn("\n");
    if(url) {
        IND_MEMSET(g_app_stb_monitor_tms_url, 0, LARGE_URL_MAX_LEN + 4);
        if(strlen(url) > LARGE_URL_MAX_LEN) {
            IND_STRNCPY(g_app_stb_monitor_tms_url, url, LARGE_URL_MAX_LEN);
            g_app_stb_monitor_tms_url[LARGE_URL_MAX_LEN] = '\0';
        }
        else
            IND_STRCPY(g_app_stb_monitor_tms_url, url);
        LogSafeOperDebug("url = %s\n\n", url);
    }
    else
        g_app_stb_monitor_tms_url[0] = '\0';
    LogSafeOperDebug("g_app_stb_monitor_tms_url = %s\n\n", g_app_stb_monitor_tms_url);

    int tX = 0, tY = 0, tWidth = 0, tHeight = 0;

    YX_SDK_codec_rect_get(&tX, &tY, &tWidth, &tHeight);
    if(tX != 0 && tY != 0) {
        ;//SysVideoClearFlagSet(1);
    }
    mid_mutex_unlock(g_app_mutex);
    return;
}

/*************************************************
Description:��ȡstb���ߡ�tms�Ĳ���URL
Input:     ��
output:   ָ�����URL��ַ
Return:   0:�ɹ�;-1:ʧ��
 *************************************************/
int app_stbmonitor_tms_url_get(char *buf)
{
    LogSafeOperDebug("g_app_stb_monitor_tms_url ======= %s\n\n", g_app_stb_monitor_tms_url);
    IND_STRCPY(buf, g_app_stb_monitor_tms_url);
    return 0;
}

