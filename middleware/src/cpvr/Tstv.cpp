#include "Tstv.h"
#include "CpvrAssertions.h"
#include "CpvrAuxTools.h"
#include "CpvrTaskManager.h"
#include "CpvrList.h"
#include "Cpvr.h"
#include "CpvrRes.h"
#include "SystemManager.h"
#include "ProgramChannelC20.h"
#include "BrowserBridge/Huawei/BrowserEventQueue.h"

#include "AppSetting.h"
#include "Hippo_api.h"
#include "mid_stream.h"
#include "mid_record.h"
#include "mid/mid_time.h"
#include "mid/mid_tools.h"
#include "disk_info.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace Hippo {

static Tstv *gTstv = NULL;

Tstv::Tstv()
{
    mChanKey= -1;
    this->mStartFlag = 0;
    mRecordStatus = RECORD_UNINITIALIZED;
}

Tstv::~Tstv()
{
    mChanKey = -1;
    this->mStartFlag = 0;
    mRecordStatus = RECORD_UNINITIALIZED;
    gTstv = NULL;
}

int
Tstv::startRecord()
{
    int ResReq = 0;
    //int diskSpace = 0;
    std::string m_stringID;
    ProgramChannelC20 *program = NULL;
    SystemManager &sysManager = systemManager();

    cpvr_channel_info_t chInfo;
    cpvr_task_info_t taskInfo;

    CpvrRes *tstvResource = new CpvrRes();
    if (tstvResource == NULL)
        return -1;

    m_stringID += (!mChanDomain.compare("IPTV") ? "MULTIPLE" : "DVB");
    m_stringID += mChanKey;

    program = (ProgramChannelC20 *)(sysManager.channelList().getProgramByStringID(m_stringID.c_str()));
    if (program == NULL) {
        return -1;
    }
    memset(&chInfo, 0, sizeof(chInfo));
    chInfo.ch_num = atoi(taskInfo.channel_num);
    if (!mChanDomain.compare("IPTV")) {
        //strcpy(chInfo.ch_url, program->GetChanURL().c_str());
        chInfo.ch_bandwidth = program->GetChanBandwith();
    } else {
#ifdef INCLUDE_DVBS                             /* template mod. */
        char buf[2048] = {0};
        std::string json_bgn = "{\"ChannelKey\":";

        snprintf(buf, sizeof(buf), "%s%d,\"ProgramNum\":%d	\
                        ,\"ServiceID\":%d,\"PMTPID\":%d	\
                        ,\"Freq10KHZ\":%d}", json_bgn.c_str(), program->GetChanKey(), program->GetDVB_ProgNum(), \
                        program->GetDVB_ProgNum(), program->GetDVB_PMT_PID(), program->GetDVB_TpFreq()/10);

        //strncpy(chInfo.ch_url, json_url.c_str(), sizeof(chInfo.ch_url));
        chInfo.freq = program->GetDVB_TpFreq();
#else
        printf("DVBS does not defined.\n");
        return -1;
#endif
    }
    chInfo.ch_parental_rating = program->GetChanStatus();

    tstvResource->setType(CpvrRes::TimeShift);

    if (chInfo.ch_bandwidth > 0){
        ResReq |= Resource::RTM_Bandwidth;
        tstvResource->setRequiredBandwidth(chInfo.ch_bandwidth);
    }
    //if (diskSpace > 0) {
        //ResReq |= Resource::RTM_Disk;
        //tstvResource->setRequiredDiskSpace(diskSpace);
    //}
    if (chInfo.freq > 0){
        ResReq |= Resource::RTM_Tuner;
        tstvResource->setRequiredFrequency(chInfo.freq);
    }
    tstvResource->setRequirement(ResReq);

    tstvResource->mScheduleID = "tstvScheduleID";
    tstvUserMng.attachUser(tstvResource);
#ifdef RESOURCE_MANAGER
    if (!resourceManager().requestResource(tstvResource)) {
        LogCpvrError("failed to request resource!\n");
        this->buildEvent(RECORD_COLLIDE);
        return -1;
    }
#endif
    /*开始录制，但是否录制成功由record_port.c中的
    录制消息判断，而不是此函数的返回值。*/
    mid_stream_timeshift_open();

    char starttime[15] = "";
    mid_tool_time2string(mid_time(), starttime, 0);
    this->mStartFlag = 1;
    this->mStartTime = starttime;
    return 0;
}

int
Tstv::stopRecord()
{
    /*停止本地时移录制，并自动
    删除本地时移所录制的文件*/
    mid_stream_timeshift_close();
    this->releaseRecordResource();
    this->mStartFlag = 0;
    delete this;
    return 0;
}

int
Tstv::releaseRecordResource()
{
    char taskID[] = "tstvScheduleID";
    CpvrRes *tstvResource = tstvUserMng.getUserByTaskID(taskID);
    if (tstvResource) {
	#ifdef RESOURCE_MANAGER
        resourceManager().releaseResource(tstvResource);
	#endif
        tstvUserMng.detachUser(tstvResource);
        delete(tstvResource);
        tstvResource = NULL;
    }
    return 0;
}

int
Tstv::buildEvent(RECORDSTATUS recordStatus)
{
    std::string json_bgn = "{\"type\":\"EVENT_LOCAL_TIMESHIFT_EVENT\",\"Event_subtype\":\"";
    std::string event_descript[8][2] = {{"LocalTSTV_Start_Success", "0"},
        {"", "7"},
        {"LocalTSTV_Start_Failed_For_No_Disk", "1"},
        {"Recording_Failed_For_Disk_Write_Error", "6"},
        {"Recording_Failed_For_Disk_Detached", "5"},
        {"LocalTSTV_Start_Failed_For_No_Support_Filesystem", "2"},
        {"LocalTSTV_Start_Failed_For_Insufficient_DiskSpace", "3"},
        {"LocalTSTV_Conflitc_With_In_Progress_Scheduled_PVR", "4"}};
    int Event_subtype = recordStatus;
    char buf[2048] = {0};

    LogCpvrDebug("buildEvent recordStatus(%d)\n" , recordStatus);
    this->mRecordStatus = recordStatus;
    this->mStartFlag = (Event_subtype == 0 ? 1 : 0);
    snprintf(buf, sizeof(buf), "%s%s\",\"event_descript\":\"%s\",\"chanKey\":\"%d\",\"chaDomain\":\"%s\"}", \
        json_bgn.c_str(), event_descript[Event_subtype][1].c_str(), event_descript[Event_subtype][0].c_str(), this->mChanKey, this->mChanDomain);
    LogCpvrDebug("json_bgn(%s)\n", buf);
    browserEventSend(buf, NULL);
    return 0;
}

static int tstvSpaceChange()
{
    int MaxChannelBandwidth = 0, MaxDuration = 0;
    int localTimeshiftEnable = 0;
    int localTimeshiftMaxDuration = 0;

    appSettingGetInt("maxChannelBandwidth", &MaxChannelBandwidth, 0);
    appSettingGetInt("localTimeShift_enable", &localTimeshiftEnable, 0);
    appSettingGetInt("localTimeShift_maxDuration", &localTimeshiftMaxDuration, 0);

    if (localTimeshiftEnable) {
        MaxChannelBandwidth = MaxChannelBandwidth * 1024;
        MaxDuration = localTimeshiftMaxDuration * 60;
        mid_record_set_reserve(MaxChannelBandwidth, MaxDuration);
        LogCpvrDebug("tstv space changed[%d bps:%d sec]\n",MaxChannelBandwidth , MaxDuration);
    }
    return 0;
}

static int TstvGetParameter(const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult)
{
    if(NULL == aFieldValue) {
        LogCpvrError("NULL pointer!\n");
        return -1;
    }
    int localTimeshiftEnable = 0;
    int localTimeshiftMaxDuration = 0;
    int localTimeshiftStartmode = 0;
    appSettingGetInt("localTimeShift_enable", &localTimeshiftEnable, 0);
    appSettingGetInt("localTimeShift_maxDuration", &localTimeshiftMaxDuration, 0);
    appSettingGetInt("localTImeShift_startmode", &localTimeshiftStartmode, 0);

    //sprintf(aFieldValue, "{\"isEnable\":\"%d\",\"BufferLength\":\"%d\",\"StartMode\":\"%d\"}",sysLocalTimeShiftEnableStatGet(), sysLocalTimeShiftMaxDurationGet(), sysLocalTimeShiftStartModeGet());
    sprintf(aFieldValue, "{\"isEnable\":\"%d\",\"BufferLength\":\"%d\"}",localTimeshiftEnable, localTimeshiftMaxDuration);
    LogCpvrDebug("aFieldValue:%s\n",aFieldValue);
    return 0;
}

/*
*input: {"isEnable":"1","BufferLength":"7200","StartMode":"1"}
*
*/
static int TstvSetParameter(const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
    struct json_object* object = NULL;
    struct json_object* obj = NULL;
    int isEnable,BufferLength,startMode;

    struct pvr_partition pvrPartition;

    if (pvr_partition_info_get(&pvrPartition) != 0) {
        appSettingSetInt("localTimeShift_enable", 0);
        LogCpvrError("no pvr partition\n");
        return -1;
    }
    LogCpvrDebug("aFieldValue:%s\n",aFieldValue);
    object = json_tokener_parse_string(aFieldValue);
    if(object==NULL) {
        appSettingSetInt("localTimeShift_enable", 0);
        LogCpvrError(" not  json!\n");
        return -1;
    }
    obj = json_object_get_object_bykey(object,"isEnable");
    if(obj!=NULL) {
        isEnable=atoi(json_object_get_string(obj));
        appSettingSetInt("localTimeShift_enable", isEnable);
        LogCpvrDebug("isEnable:%d\n",isEnable);
    } else
        appSettingSetInt("localTimeShift_enable", 0);

    obj = json_object_get_object_bykey(object,"BufferLength");
    if(obj!=NULL) {
        BufferLength=atoi(json_object_get_string(obj));
        appSettingSetInt("localTimeShift_maxDuration", BufferLength);
        tstvSpaceChange();
        LogCpvrDebug("BufferLength:%d\n",BufferLength);
    }

    obj = json_object_get_object_bykey(object,"StartMode");
    if(obj!=NULL) {
        startMode=atoi(json_object_get_string(obj));
        appSettingSetInt("localTImeShift_startmode", startMode);
        LogCpvrDebug("startMode:%d\n", startMode);
    }
    if (object) json_object_delete(object);
    return 0;
Err:
    if (object) json_object_delete(object);
    return -1;
}

static int TstvGetRecordStatus(const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
    int status = RECORD_UNINITIALIZED;
    int channelNum = -1;
    std::string msg = "";
    std::string chanDomain = "";
    std::string tstv_fail_msg[8] = {"Recording_In_Progress",
        "No_Channel_Need_To_Record",
        "Start_Failed_For_No_Disk",
        "Recording_Failed _For_Disk_Write_Error",
        "Recording_Failed _For_Disk_Detached",
        "Start_Failed_For_No_Support_Filesystem",
        "Start_Failed_For_Insufficient_DiskSpace",
        "Start_Failed_For_Conflict"};

    if (gTstv) {
        status=gTstv->mRecordStatus;
        channelNum = gTstv->mChanKey;
        msg = tstv_fail_msg[gTstv->mRecordStatus];
        chanDomain = gTstv->mChanDomain;
    }
    sprintf(aFieldValue,"{\"RecordStatus\":\"%d\",\"StatusDescript\":\"%s\",\"chanKey\":%d,\"chanDomain\":\"%s\"}", status, msg.c_str(), channelNum, chanDomain.c_str());
    LogCpvrDebug("aFieldValue:%s\n",aFieldValue);
}

int TstvSetStartRecord(const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
    struct json_object* object = NULL;
    struct json_object* obj = NULL;
    if (gTstv) {
        gTstv->stopRecord();
        //delete gTstv;
    }
    LogCpvrDebug("aFieldValue=%s\n", aFieldValue);

    int localTimeshiftEnable = 0;
    appSettingGetInt("localTimeShift_enable", &localTimeshiftEnable, 0);
    if(!localTimeshiftEnable) {
        LogCpvrError(" stb not allow start tstv!\n");
        return -1;
    }
    gTstv = new Tstv();
    if (!gTstv) {
        LogCpvrError(" tstv new error!\n");
        return -1;
    }
    object = json_tokener_parse_string(aFieldValue);
    if(object==NULL) {
        LogCpvrError(" not json!\n");
        return -1;
    }
    obj = json_object_get_object_bykey(object,"chanKey");
    if(obj!=NULL) {
        gTstv->mChanKey = atoi(json_object_get_string(obj));
        LogCpvrDebug("chanKey:%d\n", gTstv->mChanKey);
    }

    obj = json_object_get_object_bykey(object,"chanDomain");
    if(obj!=NULL) {
        gTstv->mChanDomain = json_object_get_string(obj);
        LogCpvrDebug("chanDomain:%s\n", gTstv->mChanDomain.c_str());
    } else
        gTstv->mChanDomain = "IPTV";

    if (object) json_object_delete(object);
    return gTstv->startRecord();
}

int TstvSetStopRecord(const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
    struct json_object* object = NULL;
    struct json_object* obj = NULL;
    int chanKey = -1;
    std::string chanDomain;
    LogCpvrDebug("aFieldValue=%s\n", aFieldValue);

    object = json_tokener_parse_string(aFieldValue);
    if(object==NULL) {
        LogCpvrError(" not json!\n");
        return -1;
    }
    obj = json_object_get_object_bykey(object,"chanKey");
    if(obj!=NULL) {
        chanKey = atoi(json_object_get_string(obj));
        LogCpvrDebug("chanKey:%d\n", chanKey);
    }

    obj = json_object_get_object_bykey(object,"chanDomain");
    if(obj!=NULL) {
        chanDomain = json_object_get_string(obj);
        LogCpvrDebug("chanDomain:%s\n", chanDomain.c_str());
    } else
        chanDomain = "IPTV";

    if (object) json_object_delete(object);

    if (gTstv) {
        LogCpvrDebug("tstv [%s:%s]\n", chanDomain.c_str(), gTstv->mChanDomain.c_str());
        if(chanKey == gTstv->mChanKey && !chanDomain.compare(gTstv->mChanDomain.c_str())){
            gTstv->stopRecord();
            LogCpvrDebug("Tstv Stop Record success!\n");
            return 0;
        }
    }
    LogCpvrDebug("Tstv Stop Record fail!\n");
    return -1;
}

static int TstvGetstartTime(const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult)
{
    if (NULL == aFieldValue) {
        LogCpvrError("NULL pointer!\n");
        return -1;
    }
    if (gTstv)
        sprintf(aFieldValue, "%s", gTstv->mStartTime.c_str());
    LogCpvrDebug("aFieldValue:%s\n", aFieldValue);
    return 0;
}

}

extern "C" {

int TstvStartFlagGet()
{
    if (Hippo::gTstv)
        return Hippo::gTstv->mStartFlag;
    return 0;
}

int TstvDealRecordMsg(int msg)
{
    switch(msg) {
    case LOCAL_TS_DISK_FULL:
        if (Hippo::gTstv)
            Hippo::gTstv->buildEvent(Hippo::RECORD_NO_FREE_SPACE);
        break;
    case LOCAL_TS_START:
        if (Hippo::gTstv)
            Hippo::gTstv->buildEvent(Hippo::RECORD_START_SUCCESS);
        break;
    case LOCAL_TS_DATA_DAMAGE :
        if (Hippo::gTstv)
            Hippo::gTstv->buildEvent(Hippo::RECORD_DISK_FIETYPE_ERR);
        break;
    case LOCAL_TS_PVR_CONFLICT:
        if (Hippo::gTstv)
            Hippo::gTstv->buildEvent(Hippo::RECORD_COLLIDE);
        break;
    case LOCAL_TS_DISK_ERROR:
        if (Hippo::gTstv)
            Hippo::gTstv->buildEvent(Hippo::RECORD_NO_DISK);
        break;
    case LOCAL_TS_MSG_ERROR:
        if (Hippo::gTstv)
            Hippo::gTstv->buildEvent(Hippo::RECORD_DISK_WRITE_ERR);
        break;
    case LOCAL_TS_DISK_DETACHED:
        if (Hippo::gTstv)
            Hippo::gTstv->buildEvent(Hippo::RECORD_DISK_REMOVED);
        break;
    default:
        break;
    }
    return 0;
}

int TstvJseRegister(int type)
{
    a_Hippo_API_JseRegister("PVR.LocalTimeshift.Parameter", Hippo::TstvGetParameter, Hippo::TstvSetParameter, (ioctl_context_type_e)type);
    a_Hippo_API_JseRegister("PVR.LocalTimeshift.RecordStatus", Hippo::TstvGetRecordStatus, NULL, (ioctl_context_type_e)type);
    a_Hippo_API_JseRegister("PVR.LocalTimeshift.StartRecord", NULL,Hippo::TstvSetStartRecord, (ioctl_context_type_e)type);
    a_Hippo_API_JseRegister("PVR.LocalTimeshift.StopRecord", NULL, Hippo::TstvSetStopRecord, (ioctl_context_type_e)type);
    a_Hippo_API_JseRegister("PVR.LocalTimeshift.startTime.get", Hippo::TstvGetstartTime, NULL, (ioctl_context_type_e)type);
    return 0;
}

int TstvUnJseRegister(int type)
{
    if (Hippo::gTstv)
        Hippo::gTstv->stopRecord();

    a_Hippo_API_UnJseRegister("PVR.LocalTimeshift.Parameter", (ioctl_context_type_e)type);
    a_Hippo_API_UnJseRegister("PVR.LocalTimeshift.RecordStatus", (ioctl_context_type_e)type);
    a_Hippo_API_UnJseRegister("PVR.LocalTimeshift.StartRecord", (ioctl_context_type_e)type);
    a_Hippo_API_UnJseRegister("PVR.LocalTimeshift.StopRecord", (ioctl_context_type_e)type);
    a_Hippo_API_UnJseRegister("PVR.LocalTimeshift.startTime.get", (ioctl_context_type_e)type);
    return 0;
}

}

