#include "CpvrAssertions.h"
#include "CpvrAuxTools.h"
#include "CpvrTaskManager.h"
#include "CpvrList.h"
#include "Cpvr.h"
#include "CpvrRes.h"
#include "SystemManager.h"
#include "ProgramChannelC20.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace Hippo {

Cpvr::Cpvr()
{
}

Cpvr::~Cpvr()
{
}

int
Cpvr::recordStart(char *scheduleID, int needReqRes)
{
    int ResReq = 0;
    int diskSpace = 0;
    std::string m_stringID;

    ProgramChannelC20 *program = NULL;
    SystemManager &sysManager = systemManager();

    cpvr_channel_info_t chInfo;
    cpvr_task_info_t taskInfo;

    if (scheduleID && cpvr_list_task_get_by_schedule_id(scheduleID, &taskInfo, NULL) != 0)
        return -1;

    m_stringID += (taskInfo.content_source == IPTV_PVR ? "MULTIPLE" : "DVB");
    m_stringID += atoi(taskInfo.channel_num);

    program = (ProgramChannelC20 *)(sysManager.channelList().getProgramByStringID(m_stringID.c_str()));
    if (program != NULL) {
        CpvrRes *cpvrResource = NULL;

        memset(&chInfo, 0, sizeof(chInfo));
        chInfo.ch_num = atoi(taskInfo.channel_num);
        if (taskInfo.content_source == IPTV_PVR) {
            strcpy(chInfo.ch_url, program->GetChanURL().c_str());
            chInfo.ch_bandwidth = program->GetChanBandwith();
            chInfo.sqacode = program->GetSqaCode();
            chInfo.retcode = program->GetRetCode();
        }
#ifdef INCLUDE_DVBS                             /* template mod. */
        else {
            char buf[2048] = {0};
            std::string json_bgn = "{\"ChannelKey\":";

            snprintf(buf, sizeof(buf), "%s%d,\"ProgramNum\":%d	\
                        ,\"ServiceID\":%d,\"PMTPID\":%d	\
                        ,\"Freq10KHZ\":%d}", json_bgn.c_str(), program->GetChanKey(), program->GetDVB_ProgNum(), \
                        program->GetDVB_ProgNum(), program->GetDVB_PMT_PID(), program->GetDVB_TpFreq()/10);
            strncpy(chInfo.ch_url, buf, sizeof(chInfo.ch_url));

            chInfo.freq = program->GetDVB_TpFreq();
        }
#else
        else {
            printf("DVBS does not defined.\n");
            return -1;
        }
#endif
        chInfo.ch_parental_rating = program->GetChanStatus();

        if (needReqRes) {
            cpvrResource = new CpvrRes();
            if (cpvrResource == NULL)
                return -1;

            cpvrResource->setType(CpvrRes::PVR);

            if (chInfo.ch_bandwidth > 0){
                ResReq |= Resource::RTM_Bandwidth;
                cpvrResource->setRequiredBandwidth(chInfo.ch_bandwidth);
            }
            if (diskSpace > 0) {
                ResReq |= Resource::RTM_Disk;
                cpvrResource->setRequiredDiskSpace(diskSpace);
            }
            if (chInfo.ch_num > 0 && taskInfo.content_source != IPTV_PVR){
                ResReq |= Resource::RTM_Tuner;
                cpvrResource->setRequiredProgNumber(chInfo.ch_num);
            }
            cpvrResource->setRequirement(ResReq);

            cpvrResource->mScheduleID = scheduleID;
            cpvrUserMng.attachUser(cpvrResource);

            if (!resourceManager().requestResource(cpvrResource)) {
                printf("failed to request resource!\n");
                cpvr_task_state_set(scheduleID, IN_PROGRESS_WITH_ERROR_STATE, OVERSTEP_ALLOWABLE_TASK);
                return -1;
            }

        }

        cpvr_task_start(scheduleID, &chInfo);
    }
}

int
Cpvr::recordStop(char *scheduleID)
{
    CpvrRes *cpvrResource = cpvrUserMng.getUserByTaskID(scheduleID);
    if (cpvrResource) {
        cpvr_task_stop(scheduleID, TASK_OK);
    }
}

int
Cpvr::recordClose(char *scheduleID)
{
    CpvrRes *cpvrResource = cpvrUserMng.getUserByTaskID(scheduleID);
    if (cpvrResource) {
	#ifdef RESOURCE_MANAGER
        resourceManager().releaseResource(cpvrResource);
	#endif
        cpvrUserMng.detachUser(cpvrResource);
        delete(cpvrResource);
        cpvrResource = NULL;

        cpvr_task_stop(scheduleID, TASK_OK);
    }
}


int
Cpvr::recordEnd(char *scheduleID)
{
    CpvrRes *cpvrResource = cpvrUserMng.getUserByTaskID(scheduleID);
    if (cpvrResource) {
	#ifdef RESOURCE_MANAGER
        resourceManager().releaseResource(cpvrResource);
	#endif
        cpvrUserMng.detachUser(cpvrResource);
        delete(cpvrResource);
        cpvrResource = NULL;
    }
    cpvr_task_end(scheduleID);
}

int
Cpvr::recordDelete(char *scheduleID)
{
    CpvrRes *cpvrResource = cpvrUserMng.getUserByTaskID(scheduleID);
    if (cpvrResource) {
	#ifdef RESOURCE_MANAGER
        resourceManager().releaseResource(cpvrResource);
	#endif
        cpvrUserMng.detachUser(cpvrResource);
        delete(cpvrResource);
        cpvrResource = NULL;
        cpvr_task_end(scheduleID);
    }
}


int
Cpvr::recordReleaseResource(char *scheduleID)
{
    CpvrRes	*cpvrResource = cpvrUserMng.getUserByTaskID(scheduleID);
    if (cpvrResource) {
	#ifdef RESOURCE_MANAGER
        resourceManager().releaseResource(cpvrResource);
	#endif
        cpvrUserMng.detachUser(cpvrResource);
        delete(cpvrResource);
        cpvrResource = NULL;
    }
}


Message *
Cpvr::timerCreate(int what, int arg, time_t tm)
{
    Message *msg = obtainMessage(what, arg, 0);
    if (msg != NULL) {
        sendMessageDelayed(msg, tm);
    }

    return msg;
}

void
Cpvr::handleMessage(Message *msg)
{
    int i_ret = -1;
    unsigned int ui_now = 0;
    need_handle_task_t first;

    i_ret = need_handle_queue_node_get(0, &first);
    if (i_ret == 0) {
        ui_now = getLocalTime();
        printf("what is %d, schedule id is %s, now is %ld, time point is %ld.\n", first.what, first.schedule_id, ui_now, first.time_point);
        if (ui_now >= first.time_point) {
            switch (first.what) {
            case MSG_TASK_START:
                recordStart(first.schedule_id, 1);
                need_handle_queue_node_remove(first.what, first.schedule_id);
                break;
            case MSG_TASK_START_WITHOUT_REQ_RES:
                recordStart(first.schedule_id, 0);
                need_handle_queue_node_remove(first.what, first.schedule_id);
                break;
            case MSG_TASK_STOP:
                recordStop(first.schedule_id);
                need_handle_queue_node_remove(first.what, first.schedule_id);
                break;
            case MSG_TASK_CLOSE:
                recordClose(first.schedule_id);
                need_handle_queue_node_remove(first.what, first.schedule_id);
                break;
            case MSG_TASK_END:
                recordEnd(first.schedule_id);
                need_handle_queue_node_remove(first.what, first.schedule_id);
                break;
            case MSG_TASK_DELETE:
                break;
            case MSG_TASK_RELEASE_RES:
                recordReleaseResource(first.schedule_id);
                need_handle_queue_node_remove(first.what, first.schedule_id);
                break;
            case MSG_SEND_START_IN_ONE_MINUTE_STATE:
                cpvr_task_start_in_one_minute_state_set(first.schedule_id);
                need_handle_queue_node_remove(first.what, first.schedule_id);
                break;
            case MSG_UNKNOWN:
                need_handle_queue_node_remove(first.what, first.schedule_id);
                break;
            }
        }
    }

    ui_now = getLocalTime();
    i_ret = need_handle_queue_node_get(0, &first);
    if (i_ret < 0 || (i_ret == 0 && first.time_point - ui_now > 1))
        timerCreate(MSG_UNKNOWN, 0, 1000);
    else
        timerCreate(MSG_UNKNOWN, 0, 0);
}

static Cpvr *gCpvr = NULL;
void *cpvr_timer_create(int what, int arg, time_t tm)
{
    if (gCpvr == NULL)
        gCpvr = new Cpvr();

    return gCpvr->timerCreate(what, arg, tm);
}

} // namespace Hippo

