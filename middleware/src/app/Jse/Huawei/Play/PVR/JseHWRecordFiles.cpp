
#include "JseHWRecordFiles.h"

#include "JseFunctionCall.h"

#include "JseAssertions.h"

#include "CpvrTaskManager.h"
#include "CpvrAuxTools.h"
#include "CpvrList.h"

#include "json_object.h"
#include "json_public.h"
#include "disk_info.h"
#include "mid_record.h"
#include "mid/mid_tools.h"
#include "pvr/int_pvr.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace Hippo;



// 此函数只被JsePVRFilesListRead调用
static int cpvr_json_build_one_file(char* aFieldValue, cpvr_task_info_t* p_info)
{
    struct PVRInfo f_info;

    int index = 0, type = 0;
    char tmp_buf[512] = {0};

    sprintf(tmp_buf, "%d", p_info->file_id);
    json_add_str(aFieldValue, "filename", tmp_buf);
    json_add_str(aFieldValue, "filepath", "file:///");
    json_add_str(aFieldValue, "prog_title", p_info->prog_title);

    mid_tool_time2str(p_info->real_start_time, tmp_buf, 0);
    json_add_str(aFieldValue, "date", tmp_buf);

    sprintf(tmp_buf, "%d", ind_pvr_get_time(p_info->file_id));
    json_add_str(aFieldValue, "duration", tmp_buf);

    ind_pvr_get_info(p_info->file_id, &f_info);
    sprintf(tmp_buf, "%d", f_info.byte_length);
    json_add_str(aFieldValue, "FileSize", tmp_buf);

    json_add_str(aFieldValue, "SourceTask", p_info->schedule_id);

    json_add_str(aFieldValue, "parentalRating", p_info->parental_rating);

    sprintf(tmp_buf, "%d", p_info->bandwidth);
    json_add_str(aFieldValue, "ChanBandwith", tmp_buf);

    backspace_comma(aFieldValue);
}


static int JsePVRFilesCountRead(const char* aFieldParam, char* value, int len)
{
	printf("_____tanf test:JsePVRFilesCountRead1,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    int i_cnt = 0;

    i_cnt = cpvr_list_get_by_file_exist(0, 0, NULL);
    sprintf(value, "{\"filesCount\":%d}", i_cnt);
    LogJseDebug("return value is %s\n", value);
    printf("_____tanf test:JsePVRFilesCountRead2,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    return 0;
}

static int JsePVRFilesListRead(const char* aFieldParam, char* value, int len)
{
	printf("_____tanf test:JsePVRFilesListRead1,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    int i_cnt = 0, i_pos = 0, i_count = 0;

    struct json_object* object = NULL;
    struct _copied_hdr_t hdr = TAILQ_HEAD_INITIALIZER(hdr);

    if (aFieldParam == NULL) return -1;
    LogJseDebug("param value is %s\n", aFieldParam);

    object = (struct json_object*)json_tokener_parse_string(aFieldParam);
    if (object) {
        char param_str[256] = {0};

        if(param_string_get(object, "position", param_str, sizeof(param_str)-1) == 0)
            i_pos = atoi(param_str);

        if(param_string_get(object, "count", param_str, sizeof(param_str)-1) == 0)
            i_count = atoi(param_str);

        json_object_delete(object);
    }

    i_cnt = cpvr_list_get_by_file_exist(i_pos, i_count, &hdr);
    sprintf(value, "{\"fileCount\":%d", i_cnt);
    if (i_cnt > 0) {
        _copied_node_t* p_node = NULL;

        strcat(value, ",\"Files\":[");
        TAILQ_FOREACH(p_node, &hdr, list)
        {
            strcat(value, "{");
            cpvr_json_build_one_file(value, &p_node->info);
            strcat(value, "},");
        }

        if(value[strlen(value) - 1] == ',')
            value[strlen(value) - 1] = 0;

        strcat(value, "]");

        cpvr_copied_list_release(&hdr);
    }
    strcat(value, "}");

    LogJseDebug("return value is %s\n", value);
    printf("_____tanf test:JsePVRFilesListRead2,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    return 0;
}

static int JsePVRFilesDeleteWrite(const char* aFieldParam, char* value, int len)
{
	printf("_____tanf test:JsePVRFilesDeleteWrite,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    int i_cnt = 0;

    struct json_object* object = NULL;

    cpvr_task_info_t task_info;
    cpvr_task_live_t task_live;
    cpvr_task_filter_param_t param;

    if (value == NULL) return -1;
    LogJseDebug("param value is %s\n", value);

    object = (struct json_object*)json_tokener_parse_string(value);
    if (object) {
        char param_str[256] = {0};

        if (param_string_get(object, "filename", param_str, sizeof(param_str)-1) == 0) {
            param.file_id = atoi(param_str);
        } else if (param_string_get(object, "schedule_id", param.schedule_id, sizeof(param.schedule_id)-1) != 0) {
            json_object_delete(object);
            return -1;
        }

        json_object_delete(object);
    }

    i_cnt = cpvr_list_task_get(&param, &task_info, &task_live);
    if (i_cnt > 0) {
        cpvr_task_delete(&task_info, NULL, 1);
    }

    return 0;
}

static int JsePVRFilesDeleteAllWrite(const char* aFieldParam, char* value, int len)
{
	printf("_____tanf test:JsePVRFilesDeleteAllWrite,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    pthread_t pid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    pthread_create(&pid, &attr, cpvr_task_delete_all_files, NULL);

    return 0;
}

static int JsePVRFilesDeleteProRead(const char* aFieldParam, char* value, int len)
{
	printf("_____tanf test:JsePVRFilesDeleteProRead1,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    sprintf(value, "{\"Delete_Progress\":\"%d%\"}", cpvr_task_delete_all_files_progress_get());
    LogJseDebug("return value is %s\n", value);
    printf("_____tanf test:JsePVRFilesDeleteProRead2,aFieldParam=[%s],value=[%s],len=[%d]\n",aFieldParam,value,len);
    return 0;
}




/*************************************************
Description: Jse 接口内部用于调用已注册的函数,当为JseGroupCall对象时调用该函数
Polymorphism: PVR.RecordFiles.xx字段在RecordFiles后改写call，直接在map查找剩下的所有字段。
Input: name:接口调用名
       param: 传递的参数
       value: 返回时的buffer
       length:buffer的长度
       set: 读写标志
Return: 0成功调用,其它非正常调用
 *************************************************/
int
JseHWRecordFiles::call(const char* name, const char* param, char* value, int length, int set)
{
    LogJseDebug("-name[%s]-param[%s]-value[%s]-length[%d]-\n", name, param, value, length);
    if (!name)
        return -1;

    char section[64];
    // const char* nextSection = 0;
    std::map<std::string, JseCall*>::iterator it = m_callMap.find(name);
    if (it != m_callMap.end())
        return (it->second)->call(0, param, value, length, set);
    else
        return -1;
}

JseHWRecordFiles::JseHWRecordFiles()
    : JseGroupCall("RecordFiles")
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("Count", JsePVRFilesCountRead, 0);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("Get", JsePVRFilesListRead, 0);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("Delete", 0, JsePVRFilesDeleteWrite);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("Delete.ByTask", 0, JsePVRFilesDeleteWrite);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("DeleteAll", 0, JsePVRFilesDeleteAllWrite);
    regist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("DeleteAll.progress", JsePVRFilesDeleteProRead, 0);
    regist(call->name(), call);


}

JseHWRecordFiles::~JseHWRecordFiles()
{
}

