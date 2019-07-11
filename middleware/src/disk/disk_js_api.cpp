
#include "DiskManager.h"
#ifdef INCLUDE_cPVR
#include "CpvrJsCall.h"
#endif

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "Assertions.h"
#include "Hippo_api.h"
#include "json/json.h"
#include "json/json_object.h"
#include "json/json_public.h"


using namespace Hippo;

extern "C" {

extern int yos_systemcall_runSystemCMD(char *buf, int *ret);

static DiskManager * g_DiskManager = 0;
static pthread_mutex_t mymutex=PTHREAD_MUTEX_INITIALIZER;

static int diskMangerInit()
{
    pthread_mutex_lock(&mymutex);
    LogUserOperDebug("diskMangerInit\n");
   if (!g_DiskManager) {
        g_DiskManager = new  DiskManager();
    }
    pthread_mutex_unlock(&mymutex);
    return 0;
}

int getDiskCount()
{
    diskMangerInit();
    pthread_mutex_lock(&mymutex);
    LogUserOperDebug("getDiskCount\n");
    if(!g_DiskManager) {
        LogUserOperError("error g_DiskManager is null \n");
        pthread_mutex_unlock(&mymutex);
        return -1;
    }
    int ret=  g_DiskManager->getDiskCount();
    pthread_mutex_unlock(&mymutex);
    return ret;
}

int refreshDiskInfo()
{
    diskMangerInit();
    pthread_mutex_lock(&mymutex);
    LogUserOperDebug("refreshDiskInfo\n");
    if(!g_DiskManager) {
        LogUserOperError("error g_DiskManager is null \n");
        pthread_mutex_unlock(&mymutex);
        return -1;
    }
    int ret =  g_DiskManager->refresh();
    pthread_mutex_unlock(&mymutex);
    return ret;
}

int getDiskInfo(int pIndex, char *name, int nameLen, long *size, int *position, int *storageFlag)
{
    if ((!position) || (!size) || (!storageFlag)) {
        LogUserOperError("position or size error\n");
        return -1;
    }
    pthread_mutex_lock(&mymutex);
    LogUserOperDebug("getDiskInfo\n");
    if(!g_DiskManager) {
        LogUserOperError("error g_DiskManager is null \n");
        pthread_mutex_unlock(&mymutex);
        return -1;
    }
    if ((pIndex < 0) || (pIndex >= g_DiskManager->getDiskCount())) {
        LogUserOperError("getDiskInfo index error\n");
        pthread_mutex_unlock(&mymutex);
        return -1;
    }
    if (name) {
        snprintf(name, nameLen, "%s", g_DiskManager->getDiskDevice(pIndex)->getName().c_str());
        *position = g_DiskManager->getDiskDevice(pIndex)->getPosition();
        *size = g_DiskManager->getDiskDevice(pIndex)->getSize();
        *storageFlag = 0;//first set 0
        if (g_DiskManager->getPvrTag(g_DiskManager->getDiskDevice(pIndex)->getName(), "null") == 1) {
              *storageFlag = 1;
        }
        pthread_mutex_unlock(&mymutex);
        return 0;
    }
    pthread_mutex_unlock(&mymutex);
    return -1;
}

int getPartitionCount(const char *diskName)
{
    pthread_mutex_lock(&mymutex);
    LogUserOperDebug("getPartitionCount\n");
    if(!g_DiskManager) {
        LogUserOperError("error g_DiskManager is null \n");
        pthread_mutex_unlock(&mymutex);
        return -1;
    }
    if (diskName) {
        std::string name = diskName;
        DiskDev *diskDev = g_DiskManager->getDiskDevice(name);
        if (diskDev) {
            int ret = diskDev->getPartitionCount();
            pthread_mutex_unlock(&mymutex);
            return ret;
        }
        else
            LogUserOperError("getPartitionCount diskDev is NULL\n");
    }
    LogUserOperError("getPartitionCount diskName is null \n");
    pthread_mutex_unlock(&mymutex);
    return 0;

}

int getPartitionInfo(const char *diskName,
                                int partitionIndex,
                                char *pName, int nameLen,
                                char *pMountPath, int mountPathLen,
                                long *totalSize_M,
                                long *freeSize_M,
                                char *fileSystype, int fsTypeBufLen,
                                int *storateFlag)
{
   if ((!pName) || (!totalSize_M) || (!freeSize_M) || (!fileSystype) || (!storateFlag) || (!pMountPath)) {
        LogUserOperError("parameter error\n");
        return -1;
    }
   pthread_mutex_lock(&mymutex);
    LogUserOperDebug("getPartitionInfo\n");
    if(!g_DiskManager) {
        LogUserOperError("error g_DiskManager is null \n");
        pthread_mutex_unlock(&mymutex);
        return -1;
    }
    std::string name =  diskName;
    DiskDev* pDiskDev = g_DiskManager->getDiskDevice(name);
    if (!pDiskDev) {
        LogUserOperError("error pDiskDev is null \n");
        pthread_mutex_unlock(&mymutex);
        return -1;
    }
    DiskPartition *partition = pDiskDev->getPartition(partitionIndex);
    if (partition) {
        snprintf(pName, nameLen, "%s", partition->getName().c_str());
        snprintf(pMountPath, mountPathLen, "%s", partition->getMountPoint().c_str());
        *totalSize_M = (long)(partition->getTotalSize()/1024);
        *freeSize_M = (long)(partition->getFreeSize()/1024);
        long fsType = partition->getFsType();
        *storateFlag = 0;
        if(g_DiskManager->getPvrTag(name, partition->getName()) == 1) {
           *storateFlag = 1;
        }
        LogUserOperDebug("filesystem type [%ld]\n", fsType);
        if(fsType == 0xEF53 )
            snprintf(fileSystype, fsTypeBufLen, "EXT3");
        else if(fsType ==  0xEF52)
            snprintf(fileSystype, fsTypeBufLen, "EXT2");
        else if(fsType == 0x4d44)
            snprintf(fileSystype, fsTypeBufLen, "FAT32");
        else if(fsType == 0x4d43)
            snprintf(fileSystype, fsTypeBufLen, "FAT");
        else if(fsType == 0x5346544e)
            snprintf(fileSystype, fsTypeBufLen, "NTFS");
        else if(fsType == 0x58465342)
            snprintf(fileSystype, fsTypeBufLen, "XFS");
        else if(fsType == 0x4244)
            snprintf(fileSystype, fsTypeBufLen, "HFS");
        else if(fsType == 0xF995E849)
            snprintf(fileSystype, fsTypeBufLen, "HFS+");
        else
            snprintf(fileSystype, fsTypeBufLen, "UNKOWN");
        LogUserOperDebug("get disk [%s] partition [%d], name [%s], pMountPath[%s] Tsize[%d], Fsize[%d], fs[%s],storFlag[%d]\n",
                    diskName, partitionIndex, pName, pMountPath, *totalSize_M, *freeSize_M, fileSystype, *storateFlag);
        pthread_mutex_unlock(&mymutex);
        return 0;
    }
    LogUserOperError("getPartition error\n");
    pthread_mutex_unlock(&mymutex);
    return -1;
}

int setPvrTagInfoInDisk(const char *pDisk, const char *pPartition)
{
    if ((!pDisk) || (!pPartition)) {
        LogUserOperError("setPvrTagInfoInDisk error\n");
        return -1;
    }
    std::string disk = pDisk;
    std::string partition = pPartition;
    pthread_mutex_lock(&mymutex);
    LogUserOperDebug("setPvrTagInfoInDisk\n");
    if(!g_DiskManager) {
        LogUserOperError("error g_DiskManager is null \n");
        pthread_mutex_unlock(&mymutex);
        return -1;
    }
    int ret = g_DiskManager->setPvrTag(disk, partition);
    pthread_mutex_unlock(&mymutex);
    return ret;
}

int getPvrDisk(char *diskName, int diskNameLen, char *partitionName, int partitionNameLen)
{
   if ((!diskName) || (!partitionName)) {
        LogUserOperError("getPvrDisk error\n");
        return -1;
    }
   pthread_mutex_lock(&mymutex);
   LogUserOperDebug("getPvrDisk\n");
    if(!g_DiskManager) {
        LogUserOperError("error g_DiskManager is null \n");
        pthread_mutex_unlock(&mymutex);
        return -1;
    }
    if (g_DiskManager->getPvrDiskFlag()) {
        snprintf(diskName, diskNameLen, "%s", g_DiskManager->getPvrDiskName().c_str());
        snprintf(partitionName, partitionNameLen, "%s", g_DiskManager->getPvrPartitionName().c_str());
        pthread_mutex_unlock(&mymutex);
        return 0;
    }
    pthread_mutex_unlock(&mymutex);
    return -1;
}

int umountDiskByName(const char * pName)
{
    if(!g_DiskManager) {
        LogUserOperError("error g_DiskManager is null \n");
        return -1;
    }
    if (!pName) {
        LogUserOperError("umountDiskByName pName is null\n");
        return -1;
    }
    std::string name = pName;
    g_DiskManager->umountDisk(name);
    return 0;
}

int formatPartition(const char *dev, const char * partition, int type)
{
    if(!g_DiskManager) {
        LogUserOperError("error g_DiskManager is null \n");
        return -1;
    }
    if (!dev ) {
        LogUserOperError("formatDiskPartition dev is null\n");
        return -1;
    }
    //type:   //1 fat 2 xfs 3 ext3
    std::string disk = dev;
    if (partition) {
        std::string partit = partition;
        g_DiskManager->formatPartition( disk, partit, type);
    } else {//merge partition
        g_DiskManager->formatPartition( disk, "null", type);
    }

    return 0;
}

int getPartitionFormatProcess()
{
    if(!g_DiskManager) {
        LogUserOperError("error g_DiskManager is null \n");
        return -1;
    }
    return g_DiskManager->getFormatProgress();
}

int checkPartition(const char *dev, const char * partition)
{
    if(!g_DiskManager) {
        LogUserOperError("error g_DiskManager is null \n");
        return -1;
    }
    if (!dev ) {
        LogUserOperError("checkPartition pName is null\n");
        return -1;
    }
    LogUserOperDebug("check partition#########\n");
    std::string disk = dev;
    if (partition) {
        std::string parti = partition;
        g_DiskManager->checkPartition(disk, parti);
    } else {//merge partition
        g_DiskManager->checkPartition(disk, "null");
    }
    return 0;
}

int getPartitionCheckProcess()
{
    if(!g_DiskManager) {
        LogUserOperError("error g_DiskManager is null \n");
        return -1;
    }
    return g_DiskManager->getPartitionCheckProgress();
}

};//extern "C"

