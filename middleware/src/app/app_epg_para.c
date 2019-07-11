/*************************************************
  Copyright (C), 1988-2008, yuxing software. Co., Ltd.
Description:   本文件主要包含以下内容：
1：md5 算法
2：业务入口设置相关函数
3：EDS 设置
4：EPG 首页设置
5：连接状态检测
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
Description:初始化应用层
Input:       无
output:     无
Return:     无
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
Description:获取stb工具、tms的播放URL
Input:     无
output:   指向结束URL地址
Return:   0:成功;-1:失败
 *************************************************/
int app_stbmonitor_tms_url_get(char *buf)
{
    LogSafeOperDebug("g_app_stb_monitor_tms_url ======= %s\n\n", g_app_stb_monitor_tms_url);
    IND_STRCPY(buf, g_app_stb_monitor_tms_url);
    return 0;
}

