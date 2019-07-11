
#include "JseHWHDDmangment.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#ifdef INCLUDE_cPVR
#include "CpvrJsCall.h"
#endif

#include "disk_js_api.h"
#include "json/json_object.h"
#include "json/json_tokener.h"

#include <stdio.h>
#include <string.h>

extern "C" int yos_systemcall_runSystemCMD(char *buf, int *ret);

static int JseAllDiskNumberInfoRead(const char* param, char* value, int len)
{
    LogJseDebug("JseAllDiskNumberInfoRead \n");
    int diskCount = getDiskCount();
    if (!diskCount) {
        snprintf(value, 1024, "{\"HDD_count\":0}");
        return 0;
    }

    int i = 0;
    json_object *diskArray = json_object_new_array();
    for (i = 0; i < diskCount; i++) {
        char diskName[128] = {0};
        int position = 0;
        char connectPort[32] = {0};
        long diskSize = 0;
        int storateFlag = 0;
        if (getDiskInfo(i, diskName, 128, &diskSize, &position, &storateFlag)) {
            LogJseError("getDiskInfo error\n");
            return 0;
        }
        if (0 == position)
            snprintf(connectPort, 32, "SATA");
        else if (1 == position)
            snprintf(connectPort, 32, "USB1");
        else if (2 == position)
            snprintf(connectPort, 32, "USB2");
        else
            snprintf(connectPort, 32, "USB1");
        int partitionCount = getPartitionCount(diskName);
        char tmpBuf[128] = "";

        json_object *diskInfo = json_object_new_object();
        json_object_object_add(diskInfo, "deviceName", json_object_new_string(diskName));
        json_object_object_add(diskInfo, "partitionCount", json_object_new_int(partitionCount));
        snprintf(tmpBuf, 128, "%ld", diskSize / 1024);
        json_object_object_add(diskInfo, "totalSpace", json_object_new_string(tmpBuf));
        json_object_object_add(diskInfo, "usedForStorage", json_object_new_int(storateFlag));
        if (storateFlag) {
            int tstvSpace = 2000;
            snprintf(tmpBuf, 128, "%d", tstvSpace);
            json_object_object_add(diskInfo, "reserverSpace", json_object_new_string(tmpBuf));
        }
        json_object_object_add(diskInfo, "connectedPort", json_object_new_string(connectPort));
        json_object_array_add(diskArray, diskInfo);
    }
    char *jsonstr_disk_info = (char *)json_object_to_json_string(diskArray);
    snprintf(value, 4096, "{\"HDD_count\":%d,\"devices\":%s}", diskCount, jsonstr_disk_info);
    LogJseDebug("value[%s]\n", value);
    json_object_put(diskArray);
    return 0;
}
static int JseDiskDetailInfoRead(const char* param, char* value, int len)
{
    LogJseDebug("JseDiskDetailInfoRead [%s]\n", param);
    if (!strlen(param)) {
        LogJseError("param is NULL!\n");
        return -1;
    }

    json_object *json_info = json_tokener_parse(param);
    if (!json_info) {
        LogJseError("json_info is NULL!\n");
        return -1;
    }
    char *diskName = (char*)json_object_get_string(json_object_object_get(json_info, "deviceName"));

    LogJseDebug("JseDiskDetailInfoRead [%s]\n", param);
    if (!diskName) {
        LogJseError("json_object_get_string deviceName error\n");
        return -1;
    }

    int partitionCount = getPartitionCount(diskName);
    if (0 == partitionCount) {
        snprintf(value, 4096, "{\"partitionCount\":0,\"partitions\":[]}");
        return 0;
    }
    json_object *diskArray = json_object_new_array();
    int partitionIndex = 0;
    for (partitionIndex = 0; partitionIndex < partitionCount; partitionIndex++) {
        char partitionName[128] = {0};
        char mountPath[256] = {0};
        long totalSize_M = 0;
        long freeSize_M = 0;
        char fileSysType[32] = {0};
        int storateFlag = 0;
        char tmpBuf[128] = "";
        if (getPartitionInfo(diskName, partitionIndex,
                            partitionName, 128,
                            mountPath, 256,
                            &totalSize_M,
                            &freeSize_M,
                            fileSysType, 32,
                            &storateFlag)) {
            LogJseError("getPartitionCount error\n");
            return 0;
        }
        if (!strlen(mountPath))
            continue;
        json_object *diskInfo = json_object_new_object();
        json_object_object_add(diskInfo, "partitionName", json_object_new_string(partitionName));
        json_object_object_add(diskInfo, "mountPath", json_object_new_string(mountPath));
        json_object_object_add(diskInfo, "filesystemType", json_object_new_string(fileSysType));
        snprintf(tmpBuf, 128, "%ld", freeSize_M / 1024);
        json_object_object_add(diskInfo, "freeSpace", json_object_new_string(tmpBuf));
        snprintf(tmpBuf, 128, "%ld", totalSize_M / 1024);
        json_object_object_add(diskInfo, "totalSpace", json_object_new_string(tmpBuf));
        json_object_object_add(diskInfo, "usedForStorage", json_object_new_int(storateFlag));
        json_object_array_add(diskArray, diskInfo);
    }
    char *jsonstr_disk_info = (char *)json_object_to_json_string(diskArray);
    snprintf(value, 4096, "{\"partitionCount\":%d,\"partitions\":%s}", partitionCount, jsonstr_disk_info);
    LogJseDebug("value[%s]\n", value);
    json_object_put(diskArray);
    json_object_put(json_info);
    return 0;
}

static int JsePVRPartitionWrite(const char* param, char* value, int len)
{
    LogJseDebug("JsePVRPartitionWrite [%s]\n", value);
    json_object *json_info = json_tokener_parse(value);
    if (!json_info) {
        LogJseError("json_info is NULL!\n");
        return -1;
    }

    char *diskName = (char*)json_object_get_string(json_object_object_get(json_info, "device"));
    char *partition = (char*)json_object_get_string(json_object_object_get(json_info, "partition"));

    if (diskName && partition) {
#ifdef INCLUDE_cPVR
        cpvr_module_disable();
#endif
        setPvrTagInfoInDisk(diskName, partition);
#ifdef INCLUDE_cPVR
        cpvr_module_enable();
#endif
    } else {
        LogJseError("JsePVRPartitionWrite error\n");
    }
    json_object_put(json_info);
    return 0;
}
static int JseInitializeDiskWrite(const char* param, char* value, int len)
{
    LogJseError("JseInitializeDiskWrite not support\n");
    return 0;
}
static int JseInitializeProgressRead(const char* param, char* value, int len)
{
    LogJseError("JseInitializeProgressRead not support\n");
    return 0;
}

static int JseFormatDiskPartitionWrite(const char* param, char* value, int len)
{
    LogJseDebug("JseFormatDiskPartitionWrite [%s]\n", value);
    json_object *json_info = json_tokener_parse(value);
    if (!json_info) {
        LogJseError("json_info is NULL!\n");
        return -1;
    }
    char *diskName = (char*)json_object_get_string(json_object_object_get(json_info, "deviceName"));
    char *partition = (char*)json_object_get_string(json_object_object_get(json_info, "partition"));
    char *fsType = (char*)json_object_get_string(json_object_object_get(json_info, "FilesystemType"));
    char *mode = (char*)json_object_get_string(json_object_object_get(json_info, "formatModel"));
    int fileSys = 3; //type:   //1 fat 2 xfs 3 ext3

    formatPartition(diskName, partition, fileSys);
    json_object_put(json_info);
    return 0;
}

static int JseFormatProgressRead(const char* param, char* value, int len)
{
    LogJseDebug("JseFormatProgressRead [%s]\n", param);
    json_object *json_info = json_tokener_parse(param);
    if (!json_info) {
        LogJseError("json_info is NULL!\n");
        return -1;
    }

    char *diskName = (char*)json_object_get_string(json_object_object_get(json_info, "device"));
    char *partition = (char*)json_object_get_string(json_object_object_get(json_info, "partition"));

    if (!diskName) {
        LogJseError("JseFormatProgressRead diskName is null\n");
        return -1;
    }
    json_object *diskInfo = json_object_new_object();

    char progress[64] = {0};
    snprintf(progress, 64, "%d", getPartitionFormatProcess());
    json_object_object_add(diskInfo, "formatProgress", json_object_new_string(progress));
    json_object_object_add(diskInfo, "device", json_object_new_string(diskName));

    char *jsonstr_disk_info = (char *)json_object_to_json_string(diskInfo);
    snprintf(value, 1024, "%s", jsonstr_disk_info);
    LogJseDebug("JseFormatProgressRead [%s]\n", value);

    json_object_put(diskInfo);
    json_object_put(json_info);
    return 0;
}

static int JseRestoreFilesystemWrite(const char* param, char* value, int len)
{
    LogJseDebug("JseRestoreFilesystemWrite [%s]\n", value);
    json_object *json_info = json_tokener_parse(value);
    if (!json_info) {
        LogJseError("json_info is NULL!\n");
        return -1;
    }

    char *diskName = (char*)json_object_get_string(json_object_object_get(json_info, "device"));
    char *partition = (char*)json_object_get_string(json_object_object_get(json_info, "partition"));


    checkPartition(diskName, partition);
    json_object_put(json_info);
    return 0;
}
static int JseRestoreProgressRead(const char* param, char* value, int len)
{
    LogJseDebug("JseRestoreProgressRead [%s]\n", param);
    json_object *json_info = json_tokener_parse(param);
    if (!json_info) {
        LogJseError("json_info is NULL!\n");
        return -1;
    }

    char *diskName = (char*)json_object_get_string(json_object_object_get(json_info, "device"));
    if (!diskName) {
        LogJseError("JseRestoreProgressRead diskName is null\n");
        return -1;
    }
    json_object *diskInfo = json_object_new_object();
    char progress[64] = {0};
    snprintf(progress, 64, "%d", getPartitionCheckProcess());
    json_object_object_add(diskInfo, "RestoreProgress", json_object_new_string(progress));
    json_object_object_add(diskInfo, "device", json_object_new_string(diskName));
    char *jsonstr_disk_info = (char *)json_object_to_json_string(diskInfo);
    snprintf(value, 1024, "%s", jsonstr_disk_info);

    json_object_put(diskInfo);
    json_object_put(json_info);
    return 0;
}

static int JseStopRestoreWrite(const char* param, char* value, int len)
{
    LogJseError("JseStopRestoreWrite not support\n");
    return 0;
}

static int JseRemoveDeviceWrite(const char* param, char* value, int len)
{
    LogJseDebug("JseRemoveDeviceWrite [%s]\n", value);
    json_object *json_info = json_tokener_parse(value);
    if (!json_info) {
        LogJseError("json_info is NULL!\n");
        return -1;
    }
    char *diskName = NULL;

    diskName = (char*)json_object_get_string(json_object_object_get(json_info, "device"));
    if (!diskName) {
        LogJseError("JseRemoveDeviceWrite check disk name error\n");
        return 0;
    }
    umountDiskByName(diskName);
    json_object_put(json_info);
    return 0;
}

JseHWHDDmangment::JseHWHDDmangment()
    : JseGroupCall("HDDmangment")
{
    JseCall* call;

    call = new JseFunctionCall("setPVRPartition", 0, JsePVRPartitionWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("initialize", 0, JseInitializeDiskWrite);
    regist(call->name(), call);
    call = new JseFunctionCall("initialize.progress",  JseInitializeProgressRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("partition.format", 0, JseFormatDiskPartitionWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("format.progress", JseFormatProgressRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("filesystemRestore", 0, JseRestoreFilesystemWrite);
    regist(call->name(), call);
    call = new JseFunctionCall("filesystemRestore.progress", JseRestoreProgressRead, 0);
    regist(call->name(), call);
    call = new JseFunctionCall("filesystemRestore.Stop", 0, JseStopRestoreWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("deviceRemove", 0, JseRemoveDeviceWrite);
    regist(call->name(), call);
}

JseHWHDDmangment::~JseHWHDDmangment()
{
}

int
JseHWHDDmangment::call(const char* name, const char* param, char* value, int length, int set)
{
    std::map<std::string, JseCall*>::iterator it = m_callMap.find(name);
    LogJseDebug("JseHWHDDmangment find call (it->second)name :%s\n", (it->second)->name());
    if ( it != m_callMap.end()) {
        return (it->second)->call(name, param, value, length, set);
    } else {
        LogJseDebug("JseHWHDDmangment not find call :%s\n", name);
        return JseGroupCall::call(name, param, value, length, set);
    }
}



/*************************************************
Description: 初始化华为磁盘管理模块配置定义的接口，由JseHWModules.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWHDDmangmentInit()
{
#if defined(hi3560e)
    system("killall -9  usbmounter");
#else
    char cmdStr[] = "killall -9  usbmounter";
    yos_systemcall_runSystemCMD(cmdStr, 0);
#endif
    refreshDiskInfo();

    JseCall* call;

    //C10/C20 regist
    call = new JseFunctionCall("getAllDiskNumberInfo", JseAllDiskNumberInfoRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseFunctionCall("GetDiskInfo", JseDiskDetailInfoRead, 0);
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseHWHDDmangment();
    JseRootRegist(call->name(), call);
    return 0;
}

