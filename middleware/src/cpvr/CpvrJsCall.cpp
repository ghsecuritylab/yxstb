#include "CpvrAssertions.h"
#include "CpvrJsCall.h"
#include "CpvrAuxTools.h"
#include "CpvrTaskManager.h"
#include "CpvrList.h"
#include "Cpvr.h"
#include "CpvrDB.h"
#include "Tstv.h"
#include "BrowserBridge/Huawei/BrowserEventQueue.h"

#include "Hippo_api.h"
#include "mid/mid_http.h"
#include "mid/mid_tools.h"
#include "mid_record.h"
#include "ind_pvr.h"
#include "disk_info.h"
#include "app_tool.h"
#include "app_epg_para.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/queue.h>
#include <semaphore.h>
#include <pthread.h>

extern char* global_cookies;
namespace Hippo {

static CPVR_TASK_PRIORITY cpvr_task_priority_str2enum(char *priority)
{
    if (!strncmp(priority, "RECORD_IF_NO_CONFLICTS", 22))
        return RECORD_IF_NO_CONFLICTS;
    else if (!strncmp(priority, "RECORD_WITH_CONFLICT", 20))
        return RECORD_WITH_CONFLICT;
    else if (!strncmp(priority, "RECORD_ANYWAY", 13))
        return RECORD_ANYWAY;
    else
        return RECORD_IF_NO_CONFLICTS;
}

static void sendEventPvrTasklistGetCompletedToEpg()
{
    //add finish refresh pvr list event
    std::string refreshPvrListEvent = "{\"type\":\"EVENT_PVR_TASKLIST_GET_COMPLETED\"}";
    browserEventSend(refreshPvrListEvent.c_str(), NULL);
}

static char s_pvrListVersion[15] = {"\0"};
static int cpvr_list_recv(int result, char* buf, int len, int arg)
{
    int valid = 0;
    char *p = NULL;

    char tag[64 + 4] = {0};
    char line[2048 + 4] = {0};
    char value[2048-64+4] = {0};
    struct pvr_partition pvrPartition;
    cpvr_task_info_t cpvr_info;

    if (buf == NULL || len <= 0)
        return -1;

    LogCpvrDebug("buf = %s, len = %d\n", buf, len);

    if (pvr_partition_info_get(&pvrPartition) != 0) {
        LogCpvrError("no pvr partition\n");
        return -1;
    }

    cpvr_tmp_list_release();
    memset(&cpvr_info, 0, sizeof(cpvr_info));
    while(1) {
        p = file_get_line(buf, len, line);
        if (p == NULL)
            break;
        len -= (p - buf);
        buf = p;

        if (line_get_tag(line, strlen(line), tag, value) < 0) {
            if (!strncmp(tag, "fatherScheduleID", 16))
                cpvr_info.task_type = REC_TYPE_NORMAL;
            continue;
        }

        if (!strncmp(tag, "PVRVer", 6)) {
            strncpy(s_pvrListVersion, value, 14);
            s_pvrListVersion[14] = 0;
            continue;
        }

        /** ä¸‹å‘çš„pvr ä»»åŠ¡åˆ—è¡¨ä¸­ä»¥æ­¤é¡¹å¼€å§‹**/
        if (!strncmp(tag, "ScheduleID", 10) && value) {
            if (valid) {
                cpvr_tmp_list_task_add(&cpvr_info);
                memset(&cpvr_info, 0, sizeof(cpvr_info));
                valid = 0;
            }
            valid = 1;
            strncpy(cpvr_info.schedule_id, value, sizeof(cpvr_info.schedule_id));
            continue;
        }
        if (!strncmp(tag, "ChannelNum", 10) && value) {
            strncpy(cpvr_info.channel_num, value, sizeof(cpvr_info.channel_num));
            continue;
        }
        if (!strncmp(tag, "ChannelName", 11) && value) {
            strncpy(cpvr_info.channel_name, value, sizeof(cpvr_info.channel_name));
            continue;
        }
        if (!strncmp(tag, "ProgramID", 9) && value) {
            strncpy(cpvr_info.prog_id, value, sizeof(cpvr_info.prog_id));
            continue;
        }
        if (!strncmp(tag, "ProgramTitle", 12) && value) {
            strncpy(cpvr_info.prog_title, value, sizeof(cpvr_info.prog_title));
            continue;
        }
        if (!strncmp(tag, "StartTime", 9) && value) {
            cpvr_info.start_time = timeStringToSecNum(value);
            continue;
        }
        if (!strncmp(tag, "EndTime", 7) && value) {
            cpvr_info.end_time = timeStringToSecNum(value);
            continue;
        }
        if (!strncmp(tag, "PackageId", 9) && value) {
            strncpy(cpvr_info.package_id, value, sizeof(cpvr_info.package_id));
            continue;
        }
        if (!strncmp(tag, "PackageName", 11) && value) {
            strncpy(cpvr_info.package_name, value, sizeof(cpvr_info.package_name));
            continue;
        }
        if (!strncmp(tag, "Priority", 8) && value) {
            cpvr_info.priority = cpvr_task_priority_str2enum(value);
            continue;
        }

        if (!strncmp(tag, "contentPath", 11) && value) {
            if (strncmp(value, "DVB", 3) == 0)
                cpvr_info.content_source = DVB_PVR;
            else
                cpvr_info.content_source = IPTV_PVR;
        }

        if (!strncmp(tag, "pltvTag", 7) && value) {
            cpvr_info.pltv_tag = atoi(value);
            continue;
        }
        if (!strncmp(tag, "fatherScheduleID", 16) && value) {
            int i = 0;
            char *ptr = value;
            cpvr_info.task_type = REC_TYPE_SUB;
            if (*ptr == '[') {
                ptr ++;
                while(1) {
                    char *p_end = strchr(ptr, ',');
                    if (p_end) {
                        *p_end = 0;
                        strncpy(cpvr_info.main_task_id[i++], ptr, p_end-ptr);
                        ptr = p_end + 1;
                    } else {
                        p_end = strchr(ptr, ']');
                        if (p_end) {
                            *p_end = 0;
                            strncpy(cpvr_info.main_task_id[i], ptr, p_end-ptr);
                        }
                        break;
                    }
                }
            } else
                strncpy(cpvr_info.main_task_id[0], value, strlen(value));
            continue;
        }
        if (!strncmp(tag, "ProgramCode", 11) && value) {
            strncpy(cpvr_info.prog_id, value, sizeof(cpvr_info.prog_id));
            continue;
        }
    }
    if (valid) {
        cpvr_tmp_list_task_add(&cpvr_info);
    }

    cpvrTaskUpdateFromMEM();
    sendEventPvrTasklistGetCompletedToEpg();
    return 0;
Err:
    return -1;
}

int cpvr_list_refresh()
{
    int len = 0;
    char url[512+4] = {0};

    len = app_construct_url(url, (char*)"EPG/jsp/getchannelpvrlist.jsp");
    if (mid_http_call(url, (mid_http_f)cpvr_list_recv, 0, NULL, 0, global_cookies) != 0) {
        LogCpvrDebug("mid_http_call failed\n");
        return -1;
    }

    return 0;
}

int cpvr_list_hearbeat_check(char *pvr_list_ver)
{
    static char old_ver[128] = {0};

    if (pvr_list_ver == NULL)
        return -1;

    LogCpvrDebug("old version is %s, new version is %s.\n", old_ver, pvr_list_ver);
    if (strncmp(old_ver, pvr_list_ver, strlen(pvr_list_ver)) != 0){
        cpvr_list_refresh();
    } else {
        strcpy(old_ver, pvr_list_ver);
    }
}


}

extern "C" {
void cpvr_module_init()
{
    // tanf£¬´ý´¦Àí
    if (Hippo::cpvr_open() == 0) {
#if defined(HUAWEI_C20)
        TstvJseRegister(IoctlContextType_eHWBaseC20);
#endif
        Hippo::cpvr_timer_create(MSG_UNKNOWN, 0, 1000);
        //Hippo::cpvr_task_resume_all(REC_TYPE_UKNOW);
    }
}

void cpvr_module_enable()
{
	// tanf£¬ ´ý´¦Àí

    if (Hippo::cpvr_open() == 0) {
#if defined(HUAWEI_C20)
        TstvJseRegister(IoctlContextType_eHWBaseC20);
#endif
        Hippo::cpvr_task_resume_all(REC_TYPE_UKNOW);
    }
}

void cpvr_module_disable()
{
	// tanf£¬ ´ý´¦Àí

    Hippo::cpvr_close();
#if defined(HUAWEI_C10)
#elif defined(HUAWEI_C20)
    //TODO
    TstvUnJseRegister(IoctlContextType_eHWBaseC20);
#endif
}

int cpvrListHearbeatCheck(char * pvr_list_ver)
{
    return Hippo::cpvr_list_hearbeat_check(pvr_list_ver);
}

int cpvrListRefresh()
{
    return Hippo::cpvr_list_refresh();
}

int getPvrListVersion(char *value)
{
    strncpy(value, Hippo::s_pvrListVersion, 14);
    value[14] = 0;
    return 0;
}

void cpvrTaskResumeAfterReboot(int flag)
{
    static int isntpSyncOk = 0;
    static int isRefreshChannelListFinish = 0;
    static int isCpvrTaskResume = 1;

    if (1 == flag)
        isntpSyncOk = 1;
    else if (2 == flag)
        isRefreshChannelListFinish = 1;

    if (isCpvrTaskResume && isntpSyncOk && isRefreshChannelListFinish) {
        Hippo::cpvr_task_resume_all(REC_TYPE_UKNOW);
        isCpvrTaskResume = 0;
    }
}

}
