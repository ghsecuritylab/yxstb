#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

struct format_info{
    char fmdir[128 + 1];
    char mountedDir[128 + 1];
    int fs_type;
};

#ifdef __cplusplus
extern "C" {
#endif

int getDiskCount();
int refreshDiskInfo();
int getDiskInfo(int pIndex, char *name, int nameLen, long *size, int *position, int *storageFlag);
int getPartitionCount(const char *diskName);
int setPvrTagInfoInDisk(const char *pDisk, const char *pPartition);
int getPvrDisk(char *diskName, int diskNameLen, char *partitionName, int partitionNameLen);
int umountDiskByName(const char * pName);
int formatPartition(const char *dev, const char * partition, int type);
int getPartitionFormatProcess();
int checkPartition(const char *dev, const char * partition);
int getPartitionCheckProcess();
int getPartitionInfo(const char *diskName, int partitionIndex, char *pName, int nameLen, char *pMountPath, int mountPathLen,
                     long *totalSize_M, long *freeSize_M, char *fileSystype, int fsTypeBufLen, int *storateFlag);
#ifdef __cplusplus
}
#endif

#endif

