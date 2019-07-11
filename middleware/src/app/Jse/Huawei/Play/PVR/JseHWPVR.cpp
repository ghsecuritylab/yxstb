
#include "JseHWPVR.h"
#include "JseHWSchedule.h"
#include "JseHWRecordFiles.h"
//#include "JseHWLocalTimeshift.h"

#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "JseAssertions.h"

#include "CpvrTaskManager.h"
#include "CpvrAuxTools.h"
#include "CpvrJsCall.h"

#include "json_object.h"
#include "json_public.h"
#include "disk_info.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace Hippo;


static int JsePVRAbilityRead(const char* aFieldParam, char* value, int len)
{
    cpvr_ability_t ability;
    std::string queryType = "";
    std::string deviceID = "";
    char param_str[128] = {0};
    struct json_object* object = NULL;

    //if (param == NULL) return -1;
    LogJseDebug("param value is %s\n", aFieldParam);

    object = (struct json_object*)json_tokener_parse_string(aFieldParam);
    if (object) {
        if (param_string_get(object, "queryType", param_str, sizeof(param_str)-1) == -1) {
            queryType = "local";
        } else
            queryType = param_str;

        if (param_string_get(object, "deviceID", param_str, sizeof(param_str)-1) == -1) {
            ;//deviceID = "";
        } else
            deviceID = param_str;
    } else
        queryType = "local";

    if (!queryType.compare("dlna") && deviceID.empty()) {
        ;//get pvr ability by deviceID
    } else if (!queryType.compare("local")) {
        memset(&ability, 0, sizeof(ability));
        cpvr_ability_get(&ability);
        sprintf(value, "{\"AllowPVR\":%d,\"AllowConcurrent\":\"%d\",\"DiskQuta\":\"%d\"}", ability.allow_pvr, ability.allow_concurrent, ability.disk_quta/1024);
    } else {
        LogJseError("get pvr ability error![%s:%s]\n", queryType.c_str(), deviceID.c_str());
        return -1;
    }
    LogJseDebug("return value is %s.\n", value);
    return 0;
}

static int JsePVRAbilityWrite(const char* aFieldParam, char* value, int len)
{

    cpvr_ability_t ability;

    char param_str[128] = {0};

    struct json_object* object = NULL;

    if (value == NULL)
        return -1;

    LogJseDebug("param value is %s\n", value);

    object = (struct json_object*)json_tokener_parse_string(value);
    if (object) {
        struct pvr_partition pvr_partition;

        if (pvr_partition_info_get(&pvr_partition) != 0) {
            json_object_delete(object);
            return -1;
        }

        cpvr_ability_get(&ability);

        if (param_int_get(object, "AllowPVR", &ability.allow_pvr) == -1) {
            ability.allow_pvr = 1;
        }

        if (param_string_get(object, "AllowConcurrent", param_str, sizeof(param_str)-1) == -1) {
            ability.allow_concurrent = 0;
        } else
            ability.allow_concurrent = atoi(param_str);

        if (param_string_get(object, "DiskQuta", param_str, sizeof(param_str)-1) == -1) {
            ability.disk_quta = pvr_partition.partition_free_size;
        } else
            ability.disk_quta = atoi(param_str);

        if (!ability.allow_pvr)
            cpvr_task_stop_all(REC_TYPE_UKNOW);
        cpvr_ability_set(&ability);
        //cpvr_task_resume_all(REC_TYPE_UKNOW);

        json_object_delete(object);
    }

    return 0;
}

static int JsePVRRefreshListWrite(const char* aFieldParam, char* value, int len)
{
    cpvrListRefresh();
    return 0;
}

static int JsePVRVersionRead(const char* aFieldParam, char* value, int len)
{
    getPvrListVersion(value);
    LogUserOperDebug("pvr list version [%s]\n", value);

    return 0;
}




/*************************************************
Description: ��"."��ֵ�����
Polymorphism: ���һЩ������ֶ�
Input: name:  �ӿڵ�����
       buffer: ��һ��"."֮��ĵ�����
       length: buffer����
Return: ��һ��"."֮�������ĸָ��
 *************************************************/
static const char* firstSection(const char* name, char* buffer, int length)
{
    const char* result = 0;

    int i;

    // Schedule�ֶαȽ��ң����ֲ��ע��
    if (strstr(name, "Schedule") || (!strncmp(name, "TaskConflictList.Get", strlen("TaskConflictList.Get")))) {
    	strncpy(buffer, name, length);
    	return result;
    }

    for (i = 0; name[i] != '\0'; i++) {
        if (i == length)
            return result;

        if (name[i] == '.') {
            result = name + (i + 1);
            break;
        }
        else
            buffer[i] = name[i];
    }
    buffer[i] = 0;

    return result;
}

/*************************************************
Description: Jse �ӿ��ڲ����ڵ�����ע��ĺ���,��ΪJseGroupCall����ʱ���øú���
Polymorphism: �ı���firstSection�ķ���
Input: name:�ӿڵ�����
       param: ���ݵĲ���
       value: ����ʱ��buffer
       length:buffer�ĳ���
       set: ��д��־
Return: 0�ɹ�����,��������������
 *************************************************/
int
JseHWPVR::call(const char* name, const char* param, char* value, int length, int set)
{
    LogJseDebug("-name[%s]-param[%s]-value[%s]-length[%d]-\n", name, param, value, length);
    if (!name)
        return -1;

    char section[64];
    const char* nextSection = firstSection(name, section, sizeof(section));
    std::map<std::string, JseCall*>::iterator it = m_callMap.find(section);
    if (it != m_callMap.end())
        return (it->second)->call(nextSection, param, value, length, set);
    else
        return -1;
}

JseHWPVR::JseHWPVR()
    : JseGroupCall("PVR")
{
    JseCall* call;

    //C20 regist
    call = new JseHWRecordFiles();
    regist(call->name(), call);

    //C20 regist
    //call = new JseHWLocalTimeshift();
    //regist(call->name(), call);


}

JseHWPVR::~JseHWPVR()
{
}

/*************************************************
Description: ��ʼ����ΪPVRģ�����ö���Ľӿڣ���JseHWPlays.cpp����.��Ӻ��С�Schedule���ֶ�ע��call�ĸ�д��
Input: ��
Return: ��
 *************************************************/
int JseHWPVRInit()
{
    JseCall* call;
    JseGroupCall* callPVR;

    //C20 regist
    callPVR = new JseHWPVR();
	call = callPVR;
    JseRootRegist(call->name(), call);
    //C20 regist
    JseHWScheduleInit(callPVR);  // ����ΪJseHWPVR��ָ��

    //C20 regist
    call = new JseFunctionCall("PVRAbility", JsePVRAbilityRead, JsePVRAbilityWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("hw_op_refreshchannelpvrlist", 0, JsePVRRefreshListWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("pvrListVersion", JsePVRVersionRead, 0);
    JseRootRegist(call->name(), call);

    return 0;
}


