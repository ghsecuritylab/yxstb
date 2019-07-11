
#ifdef INCLUDE_LocalPlayer

#include "AudioFileParserFactory.h"
#include "RegularFileInfo.h"
#include "DirFileInfo.h"
#include "FileManager.h"
#include "Assertions.h"
#include "CodeTransform.h"
#include "json/json.h"
#include "json/json_public.h"
#include "Hippo_api.h"
#include "libzebra.h"
#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>

#define JSE_DATABUF_LEN 4096

extern "C" {
extern int getPartitionCount(const char *diskName);
extern int getPartitionInfo(const char *diskName,
                                int partitionIndex,
                                char *pName, int nameLen,
                                char *pMountPath, int mountPathLen,
                                long *totalSize_M,
                                long *freeSize_M,
                                char *fileSystype, int fsTypeBufLen,
                                int *storateFlag);
extern int get_jpeg_info(char *path, int *w, int *h);
extern int get_png_info(char *path, int *w, int *h);
extern int get_gif_info(char *path, int *w, int *h);
extern int get_bmp_info(char *path, int *w, int *h);


using namespace Hippo;

#define PlayerRootDir "/mnt"
static FileManager* pFileManager = NULL;
static std::string currentDiskName = "sda";
static int stopFlag = 0;
static void* startDiskScan(void* p)
{
    if(!pFileManager) {
        pFileManager = new FileManager(PlayerRootDir);
        if (!pFileManager) {
            LogUserOperError("new pFileManager is NULL\n");
            return NULL;
        }
    }  
    char* diskName = (char*)p;
    if (FileManager::Scan_Running == pFileManager->getScanStatus()) {
        LogUserOperDebug("diskScan thread is running\n");
        return NULL;    
    }      
    int partitionCount = getPartitionCount(diskName);
    if (partitionCount <= 0) {
       LogUserOperError("startDiskScan error, partitionCount <= 0\n");    
       return NULL; 
    }
    for (int i = 0; i < partitionCount; i++) {
        char nameBuf[256] = {0};
        char mountPathBuf[256] = {0};
        long size = 0;
        long freeSize = 0;
        char fileSysType[256] = {0};
        int storateFlag = 0;
        if (getPartitionInfo(diskName, i, nameBuf, 256, mountPathBuf, 256, &size, &freeSize, fileSysType, 256, &storateFlag)) {
            LogUserOperError("getPartitionInfo error\n");
            return NULL;
        }
        pFileManager->startScan(mountPathBuf);
//        FileManager::ScanStatus status = FileManager::Scan_stop;
        while(1) { //wait scan finish
            if (stopFlag) { //user stop
                LogUserOperDebug("user stop\n");
                pFileManager->stopScan();
                goto stopTag;
            }

            if (pFileManager->getScanStatus() != FileManager::Scan_Running)
                break;
            else 
                usleep(100000);
        }
    }   
stopTag:         
    
    return NULL;
}

static int stopDiskScan()
{
    if(!pFileManager) {
        LogUserOperError("stopDiskScan error\n");
        return -1;    
    }    
    if (pFileManager->getScanStatus() == FileManager::Scan_Running) 
        stopFlag = 1;
        
    return 0;
}

static int JseStartDiskScan(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    if(!pFileManager) {
        pFileManager = new FileManager(PlayerRootDir);
        if (!pFileManager) {
            LogUserOperError("new pFileManager is NULL\n");
            return -1;
        }
    }  
    if ( FileManager::Scan_Running == pFileManager->getScanStatus()) {
        LogUserOperDebug("diskScan thread is running\n");
        return 0;    
    } 
    currentDiskName = aFieldValue;
    LogUserOperDebug("JseStartDiskScan aFieldValue [%s]\n", aFieldValue);
    if (getPartitionCount(aFieldValue) <= 0) {
        LogUserOperError("JseStartDiskScan error, partitionCount <= 0\n");
        return -1;    
    }
    pthread_t threadID;
    if (pthread_create(&threadID, NULL, startDiskScan, (void*)aFieldValue)) {
        LogUserOperError("JseStartDiskScan pthread_create error\n");
    }
    pthread_detach(threadID);
    return 0;
}

static int JseStopDiskScan(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    stopDiskScan();
    return 0;    
}

static int videoCount = 0;
static int audioCount = 0;
static int imageCount = 0;
static bool countMediaFile(FileInfo* info)
{
    if (!info) {
        LogUserOperError("countMediaFile error\n");
        return false;    
    }        
    LogUserOperDebug("countMediaFile [%s]\n", info->getName().c_str());
    if (((FileInfo*)info)->getFileType() == FileInfo::Regular_file) {
        if (((RegularFileInfo*)info)->getClassification() == RegularFileInfo::VideoFile)
            videoCount++;
        else if (((RegularFileInfo*)info)->getClassification() == RegularFileInfo::AudioFile)   
            audioCount++;
        else if (((RegularFileInfo*)info)->getClassification() == RegularFileInfo::ImageFile)     
            imageCount++; 
    } 
    return false;
}

static int JseGetDiskScanRet(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
	if (!pFileManager) {
        LogUserOperError("JseGetDiskScanRet error, pFileManager is NULL\n");
        return -1; 		
	}
    if (pFileManager->getScanStatus() == FileManager::Scan_stop) {
        snprintf(aFieldValue, JSE_DATABUF_LEN, "{\"Status\":\"Stop\"}");
        return 0;
    }
    if (pFileManager->getScanStatus() == FileManager::Scan_Running) {
        snprintf(aFieldValue, JSE_DATABUF_LEN, "{\"Status\":\"Run\"}");
        return 0;    
    }
    if (pFileManager->getScanStatus() != FileManager::Scan_Finish) {
        snprintf(aFieldValue, JSE_DATABUF_LEN, "{\"Status\":\"Stop\"}");
        return 0;
    }

    videoCount = 0;
    audioCount = 0;
    imageCount = 0;    
    int partitionCount = getPartitionCount(currentDiskName.c_str());
    if (partitionCount <= 0) {
       LogUserOperError("startDiskScan error, partitionCount <= 0\n");    
       return -1; 
    }
    for (int i = 0; i < partitionCount; i++) {
        char nameBuf[256] = {0};
        char mountPathBuf[256] = {0};
        long size = 0;
        long freeSize = 0;
        char fileSysType[256] = {0};
        int storateFlag = 0;
        if (getPartitionInfo(currentDiskName.c_str(), i, nameBuf, 256, mountPathBuf, 256, &size, &freeSize, fileSysType, 256, &storateFlag)) {
            LogUserOperError("getPartitionInfo error\n");
            return -1;
        }
        LogUserOperDebug("mount path is [%s]\n", mountPathBuf);
        pFileManager->traverseFileTree(mountPathBuf, countMediaFile, NULL, NULL);
    }
    snprintf(aFieldValue, JSE_DATABUF_LEN, "{\"Status\":\"Finish\", \"VideoCount\":\"%d\", \"AudioCount\":\"%d\", \"ImageCount\":\"%d\"}", videoCount, audioCount, imageCount); 
    LogUserOperDebug("JseGetDiskScanRet[%s]\n", aFieldValue);
    return 0;
}

static std::string browserType = "Directory";
static int JseSetBrowserType(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    LogUserOperDebug("JseSetBrowserType [%s]\n", aFieldValue);
    browserType = aFieldValue;
    return 0;
}

static std::string IncludeVideo = "NO";
static std::string IncludeAudio = "NO";
static std::string IncludeImage = "NO";
static std::string IncludeDirectory = "NO";
static std::string IncludeOtherFile = "NO";
static std::string IncludeSubDir = "NO";
static int JseSetMediaFilter(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    LogUserOperDebug("JseSetMediaFilter [%s]\n", aFieldValue);
    json_object *json_info = json_tokener_parse(aFieldValue);
    if (!json_info) {
        LogUserOperError("JseSetMediaFilter json_tokener_parse error\n");
        return -1;    
    }
    IncludeVideo = (char*)json_object_get_string(json_object_object_get(json_info, "IncludeVideo"));        
    IncludeAudio = (char*)json_object_get_string(json_object_object_get(json_info, "IncludeAudio"));  
    IncludeImage = (char*)json_object_get_string(json_object_object_get(json_info, "IncludeImage"));  
    IncludeDirectory = (char*)json_object_get_string(json_object_object_get(json_info, "IncludeDirectory"));                          
    IncludeOtherFile = (char*)json_object_get_string(json_object_object_get(json_info, "IncludeOtherFile"));         
    json_object_put(json_info);
    LogUserOperDebug("JseSetMediaFilter exit\n");
    return 0;
}

static int sortOrder = 0;
static bool compareName(FileInfo *a, FileInfo *b)
{
    FileInfo *item_1 = NULL;
    FileInfo *item_2 = NULL;
    if (!sortOrder) {
        item_1 = a;
        item_2 = b;
    } else {
        item_1 = b;
        item_2 = a;
    }
    if(!item_1 || !item_2) {
        LogUserOperError("compareName error\n");
        return false;
    }
    if ((item_1->getFileType() == FileInfo::Dir_file) && (item_2->getFileType() == FileInfo::Dir_file)) {
        return item_1->getName() < item_2->getName();
    }
    if ((item_1->getFileType() == FileInfo::Regular_file) && (item_2->getFileType() == FileInfo::Regular_file)) {
        return item_1->getName() < item_2->getName();
    }
    return item_1->getFileType() > item_2->getFileType();
}

static bool compareSize(FileInfo *a, FileInfo *b)
{
    FileInfo *item_1 = NULL;
    FileInfo *item_2 = NULL;
    if (sortOrder) {
        item_1 = a;
        item_2 = b;
    } else {
        item_1 = b;
        item_2 = a;
    }
    if(!item_1 || !item_2) {
        LogUserOperError("compareName error\n");
        return false;
    }
    if ((item_1->getFileType() == FileInfo::Dir_file) && (item_2->getFileType() == FileInfo::Dir_file)) {
        return item_1->getFileSizeByte() < item_2->getFileSizeByte();
    }
    if ((item_1->getFileType() == FileInfo::Regular_file) && (item_2->getFileType() == FileInfo::Regular_file)) {
        return item_1->getFileSizeByte() < item_2->getFileSizeByte();
    }
    return item_1->getFileType() > item_2->getFileType();
}

static bool compareType(FileInfo *a, FileInfo *b)
{
    FileInfo *item_1 = NULL;
    FileInfo *item_2 = NULL;
    if (sortOrder) {
        item_1 = a;
        item_2 = b;
    } else {
        item_1 = b;
        item_2 = a;
    }
    if(!item_1 || !item_2) {
        LogUserOperError("compareType error\n");
        return false;
    }
    if ((item_1->getFileType() == FileInfo::Dir_file) && (item_2->getFileType() == FileInfo::Dir_file)) {
        return item_1->getName() < item_2->getName();
    }
    if ((item_1->getFileType() == FileInfo::Regular_file) && (item_2->getFileType() == FileInfo::Regular_file)) {
        RegularFileInfo *pInfo_1 = static_cast<RegularFileInfo *>(item_1);
        RegularFileInfo *pInfo_2 = static_cast<RegularFileInfo *>(item_2);
        if (pInfo_1->getClassification() == pInfo_2->getClassification()) {
            std::string suffix_1;
            std::string suffix_2;
            std::string name_1;
            std::string name_2;
            std::string::size_type s1, s2;
            s1 = pInfo_1->getName().find_last_of('.');
            s2 = pInfo_2->getName().find_last_of('.');
            name_1 = pInfo_1->getName().substr(0, s1);
            name_2 = pInfo_2->getName().substr(0, s2);
            suffix_1 = pInfo_1->getName().substr(s1 + 1, pInfo_1->getName().length() - s1);
            suffix_2 = pInfo_2->getName().substr(s2 + 1, pInfo_2->getName().length() - s2);
            if (suffix_1 == suffix_2)
                return name_1 < name_2;
            else
                return suffix_1 < suffix_2;
        } else {
            return pInfo_1->getClassification() < pInfo_2->getClassification();
        }
    }
    return item_1->getFileType() > item_2->getFileType();
}

static bool compareCreateTime(FileInfo *a, FileInfo *b)
{
    FileInfo *item_1 = NULL;
    FileInfo *item_2 = NULL;
    if (sortOrder) {
        item_1 = a;
        item_2 = b;
    } else {
        item_1 = b;
        item_2 = a;
    }
    if(!item_1 || !item_2) {
        LogUserOperError("compareCreateTime error\n");
        return false;
    }
    if ((item_1->getFileType() == FileInfo::Dir_file) && (item_2->getFileType() == FileInfo::Dir_file)) {
        return item_1->getFileCreateTime() < item_2->getFileCreateTime();
    }
    if ((item_1->getFileType() == FileInfo::Regular_file) && (item_2->getFileType() == FileInfo::Regular_file)) {
        return item_1->getFileCreateTime() < item_2->getFileCreateTime();
    }
    return item_1->getFileType() > item_2->getFileType();
}

static bool compareArtist(FileInfo *a, FileInfo *b)
{
    FileInfo *item_1 = NULL;
    FileInfo *item_2 = NULL;
    if (sortOrder) {
        item_1 = a;
        item_2 = b;
    } else {
        item_1 = b;
        item_2 = a;
    }
    
    if(!item_1 || !item_2) {
        LogUserOperError("compareArtist error\n");
        return true;
    }
    
    if ((item_1->getFileType() == FileInfo::Regular_file) && (item_2->getFileType() == FileInfo::Regular_file)) {   
        RegularFileInfo *pInfo_1 = static_cast<RegularFileInfo *>(item_1);
        RegularFileInfo *pInfo_2 = static_cast<RegularFileInfo *>(item_2);         
        if (pInfo_1->getArtist().compare(pInfo_2->getArtist().c_str()) >= 0) 
            return false;
    }
    return true;
}

static bool compareAlbum(FileInfo *a, FileInfo *b)
{
    FileInfo *item_1 = NULL;
    FileInfo *item_2 = NULL;
    if (sortOrder) {
        item_1 = a;
        item_2 = b;
    } else {
        item_1 = b;
        item_2 = a;
    }
    if(!item_1 || !item_2) {
        LogUserOperError("compareAlbum error\n");
        return true;
    }
    
    if ((item_1->getFileType() == FileInfo::Regular_file) && (item_2->getFileType() == FileInfo::Regular_file)) {  
        RegularFileInfo *pInfo_1 = static_cast<RegularFileInfo *>(item_1);
        RegularFileInfo *pInfo_2 = static_cast<RegularFileInfo *>(item_2);        
        if (pInfo_1->getAlbum().compare(pInfo_2->getAlbum().c_str()) >= 0) 
                return false;
    }
    return true;
}

static bool filterFile(FileInfo* info)
{
    if (!info) {
        LogUserOperError("countMediaFile error\n");
        return false;    
    }     
    
    if (!browserType.compare("Directory")) { //only filter dir
        if (info->getFileType() == FileInfo::Dir_file) {
 //         LogUserOperDebug("dir name is [%s]\n", info->getName().c_str());
            DirFileInfo* pDirInfo = (DirFileInfo*)info;
            std::vector<FileInfo *>* pItemList = pDirInfo->getItemList();
            for (int i = 0; i < pDirInfo->getItemCount(); i++) {
                if (pItemList->at(i)->getFileType() == FileInfo::Regular_file) {
                    if (((RegularFileInfo*)(pItemList->at(i)))->getClassification() == RegularFileInfo::VideoFile) {
                        if (!IncludeVideo.compare("YES"))
                            return true;
                    }
                    if (((RegularFileInfo*)(pItemList->at(i)))->getClassification() == RegularFileInfo::AudioFile)  {
                        if (!IncludeAudio.compare("YES"))
                            return true;
                    } 
                    if (((RegularFileInfo*)(pItemList->at(i)))->getClassification() == RegularFileInfo::ImageFile) {
                        if (!IncludeImage.compare("YES"))
                            return true;
                    }    
                    if (((RegularFileInfo*)(pItemList->at(i)))->getClassification() == RegularFileInfo::UnkonwFile) {
                        if (!IncludeOtherFile.compare("YES"))
                            return true;
                    }                                 
                }   
            } 
            delete pItemList; 
        }
    } 
    else { //only filter media file
        if (info->getFileType() == FileInfo::Regular_file) {
            if (((RegularFileInfo*)info)->getClassification() == RegularFileInfo::VideoFile) {
                if (!IncludeVideo.compare("YES"))
                    return true;
            }
            if (((RegularFileInfo*)info)->getClassification() == RegularFileInfo::AudioFile)  {
                if (!IncludeAudio.compare("YES"))
                    return true;
            } 
            if (((RegularFileInfo*)info)->getClassification() == RegularFileInfo::ImageFile) {
                if (!IncludeImage.compare("YES"))
                    return true;
            }    
            if (((RegularFileInfo*)info)->getClassification() == RegularFileInfo::UnkonwFile) {
                if (!IncludeOtherFile.compare("YES"))
                    return true;
            }         
        }         
    }
    return false;    
}

static std::vector<FileInfo*>* pCurrentList = new std::vector<FileInfo*>;
static int JseGetFileCount(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    if (!pFileManager) {
        LogUserOperError("JseGetFileCount error\n");
        return -1;    
    }
    LogUserOperDebug("JseGetFileCount [%s]\n", aFieldParam);
    json_object *json_info = json_tokener_parse(aFieldParam);  
    if (!json_info) {
        LogUserOperError("JseGetFileCount json_tokener_parse error\n");
        return -1;    
    }      
    std::string Path = (char*)json_object_get_string(json_object_object_get(json_info, "Path"));        
    IncludeSubDir = (char*)json_object_get_string(json_object_object_get(json_info, "IncludeSubDir"));  
    std::string SortType = (char*)json_object_get_string(json_object_object_get(json_info, "SortType"));   
    json_object_put(json_info);
        
    pCurrentList->clear();
    
    if (!IncludeSubDir.compare("YES")) { //traverse file tree
        pFileManager->traverseFileTree(Path, filterFile, NULL, pCurrentList);
    } 
    else { //get current dir info
        FileInfo* info = pFileManager->getFileInfo(Path);
        if (!info) {
            LogUserOperError("JseGetFileCount error, info is null\n");
            return -1;    
        }
        if (info->getFileType() == FileInfo::Dir_file) {
            DirFileInfo* pDirInfo = (DirFileInfo*)info;
            std::vector<FileInfo *>* pItemList = pDirInfo->getItemList();
            for (unsigned int i = 0; i < pItemList->size(); i++) {
                if (pItemList->at(i)->getFileType() == FileInfo::Regular_file) {
                    if (((RegularFileInfo*)(pItemList->at(i)))->getClassification() == RegularFileInfo::VideoFile) {
                        if (!IncludeVideo.compare("YES")) {
                            pCurrentList->push_back(pItemList->at(i));
                            continue;
                        }
                    }
                    if (((RegularFileInfo*)(pItemList->at(i)))->getClassification() == RegularFileInfo::AudioFile)  {
                        if (!IncludeAudio.compare("YES")) {
                            pCurrentList->push_back(pItemList->at(i));
                            continue;
                        }
                    } 
                    if (((RegularFileInfo*)(pItemList->at(i)))->getClassification() == RegularFileInfo::ImageFile) {
                        if (!IncludeImage.compare("YES")) {
                            pCurrentList->push_back(pItemList->at(i));
                            continue;
                        }
                    }    
                    if (((RegularFileInfo*)(pItemList->at(i)))->getClassification() == RegularFileInfo::UnkonwFile) {
                        if (!IncludeOtherFile.compare("YES")) {
                            pCurrentList->push_back(pItemList->at(i));
                            continue;
                        }
                    }                                 
                }
                else if  (pItemList->at(i)->getFileType() == FileInfo::Dir_file) {
                    if (!IncludeDirectory.compare("YES")) {
                        pCurrentList->push_back(pItemList->at(i));
                        continue;
                    }
                } 
            } 
            delete pItemList;
        }        
    }
    //sort file list
    if (!SortType.compare("ByName"))
        sort(pCurrentList->begin(), pCurrentList->end(), compareName);
    else if (!SortType.compare("ByCreatTime"))
        sort(pCurrentList->begin(), pCurrentList->end(), compareCreateTime);
    else if (!SortType.compare("BySize"))
        sort(pCurrentList->begin(), pCurrentList->end(), compareSize);
    else if (!SortType.compare("ByMediaType"))
        sort(pCurrentList->begin(), pCurrentList->end(), compareType);
       
    snprintf(aFieldValue, JSE_DATABUF_LEN, "%d", pCurrentList->size());
    LogUserOperDebug("JseGetFileCount[%s]\n", aFieldValue);
    return 0;     
}

static bool parserMediaFile(FileInfo* info)
{
    if (!info) {
        LogUserOperError("parserMediaFile error\n");
        return false;    
    }        
    LogUserOperDebug("parserMediaFile [%s]\n", info->getName().c_str());
    if (info->getFileType() == FileInfo::Regular_file) {
        RegularFileInfo* pRegInfo = static_cast<RegularFileInfo*>(info);
        if (pRegInfo->getClassification() == RegularFileInfo::AudioFile) {
            AudioFileParserFactory factory;
            factory.setPath(info->getFullPath().c_str());
            if (((RegularFileInfo*)info)->getArtist().compare("unknow") && ((RegularFileInfo*)info)->getAlbum().compare("unknow"))
                return true;
            AudioFileParser *pParser = factory.getParser();
            if (pParser) {
                ((RegularFileInfo*)info)->setArtist(pParser->getArtist());
                ((RegularFileInfo*)info)->setAlbum(pParser->getAlbum());
                ((RegularFileInfo*)info)->setYear(pParser->getYear());
                 ((RegularFileInfo*)info)->setComments(pParser->getComments());
                delete pParser;    
            }
             return true; 
        }
    }

    return false;
}

static std::vector<FileInfo*>* pFilterFileList = new std::vector<FileInfo*>;
static int JseFilterFile(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    int itemCount = 0;
    json_object *fileArray = json_object_new_array();
    LogUserOperDebug("JseFilterFile[%s]\n", aFieldParam);
    json_object *json_info = json_tokener_parse(aFieldParam); 
    if (!json_info) {
    	LogUserOperError("JseFilterFile json_tokener_parse error\n");
    	return -1;
    }
    std::string path = json_object_get_string(json_object_object_get(json_info, "Path"));
    std::string matchType = json_object_get_string(json_object_object_get(json_info, "MatchType"));
    std::string keyWorld = json_object_get_string(json_object_object_get(json_info, "KeyWorld"));
    json_object_put(json_info);
    
    FileInfo* preInfo = NULL;
    pFilterFileList->clear();
    pFileManager->traverseFileTree(path.c_str(), parserMediaFile, NULL, pFilterFileList);
    if (!matchType.compare("AllArtist")) {
          //sort(pFilterFileList->begin(), pFilterFileList->end(), compareArtist);
          FileInfo* tempInfo = 0;
          for (unsigned i = 0; i < pFilterFileList->size() - 1; i++) {
             for (unsigned j = i + 1; j < pFilterFileList->size(); j++) {
                if (compareArtist(pFilterFileList->at(i), pFilterFileList->at(j))) {
                        tempInfo = pFilterFileList->at(j);
                        pFilterFileList->at(j) = pFilterFileList->at(i);
                        pFilterFileList->at(i) =  tempInfo;
                }
             }
          }
        std::map<std::string, int> itemMap;
        int count = 0;
        for (unsigned i = 0; i < pFilterFileList->size(); i++) {
            if (!preInfo) {
                preInfo = pFilterFileList->at(i);
                count = 1;
                continue;
            }
            if (!((RegularFileInfo*)preInfo)->getArtist().compare(((RegularFileInfo*)(pFilterFileList->at(i)))->getArtist())) {
                count++;    
            }
            else {
                itemMap.insert(std::pair<std::string, int>(((RegularFileInfo*)preInfo)->getArtist(), count));    
                preInfo =  pFilterFileList->at(i);
                count = 1;
            }
    	}
        if (preInfo)        
            itemMap.insert(std::pair<std::string, int>(((RegularFileInfo*)preInfo)->getArtist(), count));	
    	    
        std::map<std::string,int>::iterator it;
        for(it = itemMap.begin(); it != itemMap.end(); ++it) {
            json_object *mediaInfo = json_object_new_object();
            json_object_object_add(mediaInfo, "Artist", json_object_new_string(it->first.c_str()));
            json_object_object_add(mediaInfo, "ItemCount", json_object_new_int(it->second));   
            json_object_array_add(fileArray, mediaInfo);  
        } 
        itemCount = itemMap.size();       
	}
	else if (!matchType.compare("AllAlbumList")) {
            //sort(pFilterFileList->begin(), pFilterFileList->end(), compareAlbum); 
          FileInfo* tempInfo = 0;
          for (unsigned i = 0; i < pFilterFileList->size() - 1; i++) {
             for (unsigned j = i + 1; j < pFilterFileList->size(); j++) {
                if (compareAlbum(pFilterFileList->at(i), pFilterFileList->at(j))) {
                        tempInfo = pFilterFileList->at(j);
                        pFilterFileList->at(j) = pFilterFileList->at(i);
                        pFilterFileList->at(i) =  tempInfo;
                }
             }
          }            
            std::map<std::string, int> itemMap;
            int count = 0;
            for (unsigned i = 0; i < pFilterFileList->size(); i++) {
                if (!preInfo) {
                    preInfo = pFilterFileList->at(i);
                    count++;
                    continue;
                }
                
                if (!((RegularFileInfo*)preInfo)->getAlbum().compare(((RegularFileInfo*)(pFilterFileList->at(i)))->getAlbum())) {
                    count++;    
                }
                else {
                    itemMap.insert(std::pair<std::string, int>(((RegularFileInfo*)preInfo)->getAlbum(), count));    
                    preInfo =  pFilterFileList->at(i);
                    count = 1;
                }
            }
            if (preInfo)
                itemMap.insert(std::pair<std::string, int>(((RegularFileInfo*)preInfo)->getArtist(), count));    	
                
        std::map<std::string,int>::iterator it;
        for(it = itemMap.begin(); it != itemMap.end(); ++it) {
            json_object *mediaInfo = json_object_new_object();
            json_object_object_add(mediaInfo, "Album", json_object_new_string(it->first.c_str()));
            json_object_object_add(mediaInfo, "ItemCount", json_object_new_int(it->second));   
            json_object_array_add(fileArray, mediaInfo);  
        }    
        itemCount = itemMap.size();	        
	}
	else if (!matchType.compare("OneArtist")) {
        	for (unsigned i = 0; i < pFilterFileList->size(); i++) {
                if (!keyWorld.compare(((RegularFileInfo*)(pFilterFileList->at(i)))->getArtist())) {
                    json_object *mediaInfo = json_object_new_object();
                    json_object_object_add(mediaInfo, "FileType", json_object_new_string("MediaFile"));
                    json_object_object_add(mediaInfo, "Type", json_object_new_string("Audio"));
                    json_object_object_add(mediaInfo, "FileId", json_object_new_int(pFilterFileList->at(i)->getFileId()));   
                    json_object_array_add(fileArray, mediaInfo);   
                    itemCount++;                  
                }
        	}	        
	}
	else if (!matchType.compare("OneAlbum")) {
   	    for (unsigned i = 0; i < pFilterFileList->size(); i++) {
            if (!keyWorld.compare(((RegularFileInfo*)(pFilterFileList->at(i)))->getAlbum())) {
                json_object *mediaInfo = json_object_new_object();
                json_object_object_add(mediaInfo, "FileType", json_object_new_string("MediaFile"));
                json_object_object_add(mediaInfo, "Type", json_object_new_string("Audio"));
                json_object_object_add(mediaInfo, "FileId", json_object_new_int(pFilterFileList->at(i)->getFileId()));   
                json_object_array_add(fileArray, mediaInfo);   
                itemCount++;                  
            }
        }	        
	}
	    
    char *jsonstr_file_info = (char *)json_object_to_json_string(fileArray);
    snprintf(aFieldValue, JSE_DATABUF_LEN, "{\"Count\":\"%d\", \"FileList\":%s}", itemCount, jsonstr_file_info);
    json_object_put(fileArray); 
	
    return 0;
}

static int JseGetFileList(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    LogUserOperDebug("JseGetFileList [%s]\n", aFieldParam);
    json_object *json_info = json_tokener_parse(aFieldParam);  
    if (!json_info) {
        LogUserOperError("JseGetFileList json_tokener_parse error\n");
        return -1;    
    }       
    int startIndex = atoi(json_object_get_string(json_object_object_get(json_info, "StartIndex")));        
    unsigned int count = atoi(json_object_get_string(json_object_object_get(json_info, "Count")));  
    json_object_put(json_info);     
    
    json_object *fileArray = json_object_new_array();   
    unsigned int i = 0;
    for(i = startIndex; (i < pCurrentList->size()) && (i <  (count + startIndex)); i++) {
        json_object *mediaInfo = json_object_new_object();
        if (pCurrentList->at(i)->getFileType() == FileInfo::Dir_file)
            json_object_object_add(mediaInfo, "FileType", json_object_new_string("Directory"));
        else {
            json_object_object_add(mediaInfo, "FileType", json_object_new_string("MediaFile"));
            if (((RegularFileInfo*)pCurrentList->at(i))->getClassification() == RegularFileInfo::AudioFile)
                json_object_object_add(mediaInfo, "Type", json_object_new_string("Audio"));
            else if (((RegularFileInfo*)pCurrentList->at(i))->getClassification() == RegularFileInfo::VideoFile)
                json_object_object_add(mediaInfo, "Type", json_object_new_string("Video"));
            else if (((RegularFileInfo*)pCurrentList->at(i))->getClassification() == RegularFileInfo::ImageFile)
                json_object_object_add(mediaInfo, "Type", json_object_new_string("Image"));
            else
                LogUserOperError("unkone media file type\n");
        }
        json_object_object_add(mediaInfo, "FileId", json_object_new_int(pCurrentList->at(i)->getFileId()));   
        json_object_array_add(fileArray, mediaInfo);  
    }
    char *jsonstr_file_info = (char *)json_object_to_json_string(fileArray);
    snprintf(aFieldValue, JSE_DATABUF_LEN, "{\"Count\":\"%d\", \"FileList\":%s}", (i - startIndex), jsonstr_file_info);
    json_object_put(fileArray);
    LogUserOperDebug("JseGetFileList[%s]\n", aFieldValue);
    return 0;
}


static std::string searchKeyWord = "";
static std::string searchMatchRule = "";    
static bool searchFile(FileInfo* info)
{
    if (info->getFileType() == FileInfo::Dir_file) 
        return false;
    
    if (info->getName().find(searchKeyWord) != std::string::npos) 
        return true;    
    else {
        char outBuf[1024] = {0};
        int outLen = 1024;
        getAcronymFromUtf8((unsigned char*)(info->getName().c_str()), info->getName().length(), outBuf, &outLen);    
        std::string tempName =  outBuf;  
        unsigned pos = tempName.find("|");
        std::string acronymName = tempName.substr(0, pos);
        std::string key_lower = searchKeyWord;
        std::transform( key_lower.begin(), key_lower.end(), key_lower.begin(), std::ptr_fun <int, int> ( std::tolower ) );   
        if (acronymName.find(key_lower) != std::string::npos) {
            return true;
        }         
    }   
    return false;    
}

static std::vector<FileInfo*>* pSearchList = new std::vector<FileInfo*>;
static int JseSearchFile(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    if (!pFileManager) {
        LogUserOperError("JseSearchFile error\n");
        return -1;    
    }
    LogUserOperDebug("JseSearchFile [%s]\n", aFieldParam);
    json_object *json_info = json_tokener_parse(aFieldParam); 
    if (!json_info) {
        LogUserOperError("JseSearchFile json_tokener_parse error\n");
        return -1;    
    }   
    searchKeyWord = (char*)json_object_get_string(json_object_object_get(json_info, "keyWord"));        
    searchMatchRule = (char*)json_object_get_string(json_object_object_get(json_info, "matchRule"));  
    std::string SortType = (char*)json_object_get_string(json_object_object_get(json_info, "SortType")); 
    std::string path = (char*)json_object_get_string(json_object_object_get(json_info, "path")); 
    json_object_put(json_info);   
    
    pSearchList->clear();
    
    pFileManager->traverseFileTree(path.c_str(), searchFile, NULL, pSearchList); 
    //sort file list
    #if 0
    if (!SortType.compare("ByName"))
        sort(pSearchList->begin(), pSearchList->end(), compareName);
    else if (!SortType.compare("ByCreatTime"))
        sort(pSearchList->begin(), pSearchList->end(), compareCreateTime);
    else if (!SortType.compare("BySize"))
        sort(pSearchList->begin(), pSearchList->end(), compareSize);
    else if (!SortType.compare("ByMediaType"))
        sort(pSearchList->begin(), pSearchList->end(), compareType);    
    #endif   
    int dirCount = 0;
    int vCount = 0;
    int aCount = 0;
    int iCount = 0;
    for (unsigned int i = 0; i < pSearchList->size(); i++) {
        if (pSearchList->at(i)->getFileType() == FileInfo::Dir_file) 
            dirCount++;
        else if (pSearchList->at(i)->getFileType() == FileInfo::Regular_file) {
            if (((RegularFileInfo*)pSearchList->at(i))->getClassification() == RegularFileInfo::VideoFile) 
                vCount++;
            else if  (((RegularFileInfo*)pSearchList->at(i))->getClassification() == RegularFileInfo::AudioFile) 
                aCount++;
            else if (((RegularFileInfo*)pSearchList->at(i))->getClassification() == RegularFileInfo::ImageFile) 
                iCount++   ;    
        }
    }
    snprintf(aFieldValue, JSE_DATABUF_LEN, "{\"count\":\"%d\", \"Video\":\"%d\", \"Audio\":\"%d\", \"Image\":\"%d\", \"Directory\":\"%d\"}", 
                    pSearchList->size(), vCount, aCount, iCount, dirCount);
    LogUserOperDebug("JseSearchFile [%s]\n", aFieldValue);                    
    return 0;                    
}

static int JseGetSearchedFileList(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    json_object *json_info = json_tokener_parse(aFieldParam);    
    if (!json_info) {
        LogUserOperError("JseGetSearchedFileList json_tokener_parse error\n");
        return -1;    
    }     
    int startIndex = atoi(json_object_get_string(json_object_object_get(json_info, "StartIndex")));        
    unsigned int count = atoi(json_object_get_string(json_object_object_get(json_info, "Count")));  
    json_object_put(json_info);     
    
    json_object *fileArray = json_object_new_array();   
    unsigned int i = 0;
    for(i = startIndex; (i < pSearchList->size()) && (i < (count + startIndex)); i++) {
        json_object *mediaInfo = json_object_new_object();
        json_object_object_add(mediaInfo, "FileType", json_object_new_string("MediaFile"));
        if (((RegularFileInfo*)pSearchList->at(i))->getClassification() == RegularFileInfo::AudioFile)
            json_object_object_add(mediaInfo, "Type", json_object_new_string("Audio"));
        else if (((RegularFileInfo*)pSearchList->at(i))->getClassification() == RegularFileInfo::VideoFile)
            json_object_object_add(mediaInfo, "Type", json_object_new_string("Video"));
        else if (((RegularFileInfo*)pSearchList->at(i))->getClassification() == RegularFileInfo::ImageFile)
            json_object_object_add(mediaInfo, "Type", json_object_new_string("Image"));
        else
            LogUserOperError("unkone media file type\n");        
        
        json_object_object_add(mediaInfo, "FileId", json_object_new_int(pSearchList->at(i)->getFileId()));   
        json_object_array_add(fileArray, mediaInfo);  
    }
    char *jsonstr_file_info = (char *)json_object_to_json_string(fileArray);
    snprintf(aFieldValue, JSE_DATABUF_LEN, "{\"Count\":\"%d\", \"FileList\":%s}", (i - startIndex), jsonstr_file_info);
    json_object_put(fileArray);
    LogUserOperDebug("JseGetSearchedFileList [%s]\n", aFieldValue);
    return 0;    
}

static int JseDeleteFile(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    LogUserOperDebug("JseDeleteFile [%s]\n", aFieldValue);
    if (!pFileManager) {
        LogUserOperError("JseDeleteFile error\n");
        return -1;    
    }    
    json_object *json_info = json_tokener_parse(aFieldValue);    
    if (!json_info) {
        LogUserOperError("JseGetSearchedFileList json_tokener_parse error\n");
        return -1;    
    } 
    int fileID = atoi(json_object_get_string(json_object_object_get(json_info, "FileId")));
    json_object_put(json_info);
    FileInfo * pInfo = pFileManager->getFileInfo(fileID);   
    if (!pInfo) {
        LogUserOperError("JseDeleteFile error, pInfo is NULL\n");
        return -1;  
    }
    pFileManager->remove(pInfo->getFullPath());
    return 0;    
}

static int JseRenameFile(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    LogUserOperDebug("JseRenameFile [%s]\n", aFieldValue);
    if (!pFileManager) {
        LogUserOperError("JseDeleteFile error\n");
        return -1;    
    }    
    json_object *json_info = json_tokener_parse(aFieldValue);    
    if (!json_info) {
        LogUserOperError("JseGetSearchedFileList json_tokener_parse error\n");
        return -1;    
    } 
    int fileID = atoi(json_object_get_string(json_object_object_get(json_info, "FileId")));
    std::string newName = json_object_get_string(json_object_object_get(json_info, "NewName"));
    json_object_put(json_info);
    FileInfo * pInfo = pFileManager->getFileInfo(fileID);   
    if (!pInfo) {
        LogUserOperError("JseDeleteFile error, pInfo is NULL\n");
        return -1;  
    }
    pFileManager->reName(pInfo,  newName);
    
    return 0;    
}

static int JseGetMediaInfo(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    if (!pFileManager) {
        LogUserOperError("JseGetMediaInfo error\n");
        return -1;    
    }    
    LogUserOperDebug("JseGetMediaInfo [%s]\n", aFieldParam);
    json_object *json_info = json_tokener_parse(aFieldParam); 
    if (!json_info) {
        LogUserOperError("JseGetMediaInfo json_tokener_parse error\n");
        return -1;    
    }       
    int fileID = atoi(json_object_get_string(json_object_object_get(json_info, "FileId")));        
    json_object_put(json_info);    
    
    FileInfo* pInfo = pFileManager->getFileInfo(fileID);
    if (!pInfo) {
        LogUserOperError("JseGetMediaInfo error\n");
        return -1;    
    }
    json_object *json_file_info = json_object_new_object();
    json_object_object_add(json_file_info, "Name", json_object_new_string(pInfo->getName().c_str()));
    json_object_object_add(json_file_info, "Path", json_object_new_string(pInfo->getFullPath().c_str()));
    char sizeBuf[128] = {0};
    snprintf(sizeBuf, 128, "%lld", pInfo->getFileSizeByte());
    json_object_object_add(json_file_info, "Size", json_object_new_string(sizeBuf));
    time_t timeSec = pInfo->getFileCreateTime();
    struct tm  * ct = gmtime(&timeSec);
    char ctString[256] = {0};
    snprintf(ctString, 256, "%04d-%02d-%02d %02d:%02d:%02d", (ct->tm_year + 1900), (ct->tm_mon + 1), ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec);
    json_object_object_add(json_file_info, "CreateTime", json_object_new_string(ctString));
    if (pInfo->getFileType() == FileInfo::Dir_file) { // count items in subdir, according filters
        int subMediaFileCount = 0;
        int subDirCount = 0;
        DirFileInfo* pDirInfo = (DirFileInfo*)pInfo;
        std::vector<FileInfo *>* pItemList = pDirInfo->getItemList();
        for (int i = 0; i < pDirInfo->getItemCount(); i++) {
            if (pItemList->at(i)->getFileType() == FileInfo::Regular_file) {
                if (((RegularFileInfo*)(pItemList->at(i)))->getClassification() == RegularFileInfo::VideoFile) {
                    if (!IncludeVideo.compare("YES"))
                        subMediaFileCount++;
                }
                if (((RegularFileInfo*)(pItemList->at(i)))->getClassification() == RegularFileInfo::AudioFile)  {
                    if (!IncludeAudio.compare("YES"))
                        subMediaFileCount++;
                } 
                if (((RegularFileInfo*)(pItemList->at(i)))->getClassification() == RegularFileInfo::ImageFile) {
                    if (!IncludeImage.compare("YES"))
                        subMediaFileCount++;
                }    
                if (((RegularFileInfo*)(pItemList->at(i)))->getClassification() == RegularFileInfo::UnkonwFile) {
                    if (!IncludeOtherFile.compare("YES"))
                        subMediaFileCount++;
                }                                 
            }   
            else if (pItemList->at(i)->getFileType() == FileInfo::Dir_file) {
                subDirCount++;
            }
        } 
        delete pItemList;         
        json_object_object_add(json_file_info, "SubMediaFileCount", json_object_new_int(subMediaFileCount)); 
        json_object_object_add(json_file_info, "SubDirCount", json_object_new_int(subDirCount)); 
    }
    else {
        RegularFileInfo* p = (RegularFileInfo*)pInfo;
        if (p->getClassification() == RegularFileInfo::VideoFile) {
            std::string fileFullPath = p->getFullPath();
            unsigned pos = fileFullPath.find_last_of(".");
            std::string picPath = fileFullPath.substr(0, pos) + ".jpeg";
            if (!access(picPath.c_str(), R_OK)) {
                LogUserOperDebug("Exist smallName picPath [%s]\n", picPath.c_str());
                json_object_object_add(json_file_info, "posterImage", json_object_new_string( picPath.c_str()));
            }
            else {
                LogUserOperDebug("Not exist smallName picPath [%s]\n", picPath.c_str());
                json_object_object_add(json_file_info, "posterImage", json_object_new_string(""));
            }
            json_object_object_add(json_file_info, "Type", json_object_new_string("Video"));
        }
        else if (p->getClassification() == RegularFileInfo::AudioFile) {
            json_object_object_add(json_file_info, "Type", json_object_new_string("Audio"));
            if (!p->isInfoSync()) {//parser the file
                YX_LOCALPLAYFILE_INFO mediaInfo;
                memset(&mediaInfo, 0, sizeof(YX_LOCALPLAYFILE_INFO));
                if (!ymm_stream_getFileInfoFromHiPlayer((char*)p->getFullPath().c_str(), &mediaInfo)) 
                    p->setFileTotalTime((int)(mediaInfo.FileTotalTime/1000));
                else {
                    LogUserOperError("ymm_stream_getFileInfoFromHiPlayer error\n");
                    p->setFileTotalTime(230);
                }
                
                AudioFileParserFactory factory;
                factory.setPath(p->getFullPath().c_str());
                AudioFileParser *pParser = factory.getParser();
                if (pParser) {
                    p->setArtist(pParser->getArtist());
                    p->setAlbum(pParser->getAlbum());
                    p->setYear(pParser->getYear());
                    p->setComments(pParser->getComments());
                    delete pParser;    
                }
                p->setInfoSync(true);                
            }
            char totalTimeStr[64] = {0};
            snprintf(totalTimeStr, sizeof(totalTimeStr), "%d", p->getFileTotalTime());
            LogUserOperDebug("TotalTime [%s]\n", totalTimeStr);
            json_object_object_add(json_file_info, "TotalTime", json_object_new_string(totalTimeStr));
            json_object_object_add(json_file_info, "Artist", json_object_new_string(((RegularFileInfo*)pInfo)->getArtist().c_str())); 
            json_object_object_add(json_file_info, "Album", json_object_new_string(((RegularFileInfo*)pInfo)->getAlbum().c_str()));
            json_object_object_add(json_file_info, "Year", json_object_new_string(((RegularFileInfo*)pInfo)->getYear().c_str())); 
            json_object_object_add(json_file_info, "Comments", json_object_new_string(((RegularFileInfo*)pInfo)->getComments().c_str()));
            json_object_object_add(json_file_info, "Publisher", json_object_new_string(((RegularFileInfo*)pInfo)->getComments().c_str()));//use comments as publisher
        }
        else if (p->getClassification() == RegularFileInfo::ImageFile) {
            unsigned pos = p->getName().find_last_of(".");
            std::string fileExtension = "";
            if (pos != std::string::npos) 
                fileExtension = p->getName().substr(pos + 1);
            int w = 720;
            int h = 576;
            if ((!strcasecmp(fileExtension.c_str(), "jpg") )||(!strcasecmp(fileExtension.c_str(), "jpeg"))) 
                get_jpeg_info((char*)p->getFullPath().c_str(), &w, &h);
            else if (!strcasecmp(fileExtension.c_str(), "png")) 
                get_png_info((char*)p->getFullPath().c_str(), &w, &h);
            else if (!strcasecmp(fileExtension.c_str(), "gif")) 
                get_gif_info((char*)p->getFullPath().c_str(), &w, &h);
            else if (!strcasecmp(fileExtension.c_str(), "bmp")) 
                get_bmp_info((char*)p->getFullPath().c_str(), &w, &h);
            else
                LogUserOperError("image type error\n");
            char picResolution[256] = {0};
            snprintf(picResolution, 256, "%dx%d", w, h);
            json_object_object_add(json_file_info, "resolution", json_object_new_string(picResolution));    
#if 1
            if (pInfo->getFileSizeByte() > 20*1024*1024) { 
                LogUserOperError("This image is too large, STB can not support it\n");
                json_object_object_add(json_file_info, "imageDamage", json_object_new_int(1));
            }
            else {
                LogUserOperDebug("cal ygp_pic_shrink to create thumbnail\n");
                std::string tempPath = p->getFullPath().substr(0, p->getFullPath().rfind("/")) + "/." + p->getName() + ".bmp";
                 if (!access(tempPath.c_str(), R_OK)) {
                    LogUserOperDebug("small pic exist\n");
                    json_object_object_add(json_file_info, "smallImgPath", json_object_new_string(tempPath.c_str()));     
                    json_object_object_add(json_file_info, "imageDamage", json_object_new_int(0)); 
                } 
                else {                    
                    int sw = 200;
                    int sh = 200;
                    if (h && w) {
                        if (w > h) {
                            sw = 200;
                            sh = (int)(200*(float)h/w);    
                        }
                        else {
                            sh = 200;
                            sw = (int)(200*(float)w/h);
                        }
                    }
                     
                    int ret = ygp_pic_shrink((char*)p->getFullPath().c_str(), (char*)tempPath.c_str(), &sw, &sh, 0, 0);   
                    if (ret) {
                        LogUserOperError("JseMediaInfoGet ygp_pic_shrink error\n");    
                        json_object_object_add(json_file_info, "imageDamage", json_object_new_int(1));
                    } 
                    else {
                        json_object_object_add(json_file_info, "smallImgPath", json_object_new_string(tempPath.c_str()));  
                        json_object_object_add(json_file_info, "imageDamage", json_object_new_int(0));  
                    }
                }
            }
#endif            
            json_object_object_add(json_file_info, "Type", json_object_new_string("Image"));
        }        
    }
    
    char *jsonstr_file_info = (char *)json_object_to_json_string(json_file_info); 
    snprintf(aFieldValue, JSE_DATABUF_LEN, "%s", jsonstr_file_info);
    json_object_put(json_file_info);
    LogUserOperDebug("JseGetMediaInfo [%s]\n", aFieldValue);
    return 0;
}

static std::vector<std::string>* lyricsList = new std::vector<std::string>;
static int JseGetLyricsInfo(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    if (!pFileManager) {
        LogUserOperError("JseGetLyricsInfo error\n");
        return -1;    
    }    
    LogUserOperDebug("JseGetLyricsInfo [%s]\n", aFieldParam);
    json_object *json_info = json_tokener_parse(aFieldParam); 
    if (!json_info) {
        LogUserOperError("JseGetMediaInfo json_tokener_parse error\n");
        return -1;    
    }       
    int fileID = atoi(json_object_get_string(json_object_object_get(json_info, "FileId")));        
    json_object_put(json_info);      
    FileInfo* pInfo = pFileManager->getFileInfo(fileID);
    if (!pInfo) {
        LogUserOperError("JseGetMediaInfo error\n");
        return -1;    
    }    
    std::string lyricsPath = pInfo->getFullPath().substr(0, pInfo->getFullPath().rfind(".")) + ".lrc";
    LogUserOperDebug("lyrics path is [%s]\n", lyricsPath.c_str());
    if (!access(lyricsPath.c_str(), R_OK)) {
        LogUserOperDebug("have lyrics\n");
         lyricsList->clear();
         FILE* fp = fopen(lyricsPath.c_str(), "r");
         if (!fp) {
            LogUserOperError("fopen file error\n");
            snprintf(aFieldValue, JSE_DATABUF_LEN, "{\"HaveLyrics\":\"No\", \"TotalLines\":\"0\"}");
            return -1;
         }
         char stringBuf[1024] = {0};
         while(fgets(stringBuf, 1024, fp)) {
            lyricsList->push_back(stringBuf);
            memset(stringBuf, 0, sizeof(stringBuf));
         }
         fclose(fp);
         snprintf(aFieldValue, JSE_DATABUF_LEN, "{\"HaveLyrics\":\"Yes\", \"TotalLines\":\"%d\"}", lyricsList->size());
         return 0;        
    }
    
    snprintf(aFieldValue, JSE_DATABUF_LEN, "{\"HaveLyrics\":\"No\", \"TotalLines\":\"0\"}");
    return 0;
}

static int JseGetLyricsByLine(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    LogUserOperDebug("JseGetLyricsByLine [%s]\n", aFieldParam);
    unsigned int index = atoi(aFieldParam);
    if (index < lyricsList->size()) 
        snprintf(aFieldValue, JSE_DATABUF_LEN, "%s", lyricsList->at(index).c_str());
    else 
        LogUserOperError("index is out of range\n");
    
    return 0;
}

static int ReleseDirInfo(const char *pPath)
{
    if(!pFileManager) {
        LogUserOperError("pFileManager is NULL\n");
        return -1;
    }

    if (!pPath) {
        LogUserOperError("LocalPlayerFileManagerReleseDir path is NULL\n");
        return -1;
    }
    LogUserOperDebug("ReleseDirInfo [%s]\n", pPath);
    std::string path = pPath;

    std::vector<std::string>* rootNameList = pFileManager->pathParser(PlayerRootDir);
    if ( !rootNameList || (rootNameList->size() == 0)) {
        LogUserOperError("ReleseDirInfo parser rootpath error\n");
        return -1;
    }

    std::vector<std::string>* nameList = pFileManager->pathParser(path);
    if ( nameList && (nameList->size() > 1)) {
        if (nameList->at(0).compare(rootNameList->at(0))) {
            LogUserOperError("path is error 000\n");
            goto Err;
        }
        pFileManager->releaseFileTree(pFileManager->getFileInfo(path), false);

        delete rootNameList;
        delete nameList;
        return 0;
    }

Err:
    delete rootNameList;
    delete nameList;
    LogUserOperError("path is error\n");
    return -1;
}

void HDPlayerDiskRefresh(int type, const char *diskName, int position, const char *mountPoint)
{
    if(!pFileManager) {
        LogUserOperError("pFileManager is NULL\n");
        return;
    }

    if (!mountPoint) {
        LogUserOperError("mountPoint is NULL\n");
        return ;
    }
    LogUserOperDebug("umount [%s]\n", mountPoint);
    std::string mPoint = mountPoint;
    std::string sep = "|";
    std::vector<std::string> *nameList = pFileManager->ParserString(mPoint, sep);
    for(unsigned int i = 0; i < nameList->size(); i++) {
         ReleseDirInfo(nameList->at(i).c_str());
    }
    delete nameList;
}


static int m_w = 320;
static int m_h = 240;

void HDPlayerPictureScaleGet(int* w,  int* h)
{
    *w = m_w;
    *h = m_h;	
}

static int JseSetPictureScale(const char *aFieldName, const char *aFieldParam, char *aFieldValue, int aResult)
{
    std::string param = aFieldValue;
    std::size_t pos;

    pos = param.find("*");
    	
    m_w = atoi(param.substr(0, pos).c_str());
    m_h = atoi(param.substr(pos + 1, param.length()).c_str());
	return 0;
}


int  HDplayer_jseAPI_Register(ioctl_context_type_e type)
{
    a_Hippo_API_JseRegister( "HDplayer.StartScan",          NULL,               JseStartDiskScan,          type );
    a_Hippo_API_JseRegister( "HDplayer.StopScan",           NULL,               JseStopDiskScan,           type );
    a_Hippo_API_JseRegister( "HDplayer.GetScanStatus",      JseGetDiskScanRet,  NULL,                      type );
    a_Hippo_API_JseRegister( "HDplayer.SetFileBrowserType", NULL,               JseSetBrowserType,         type );
    a_Hippo_API_JseRegister( "HDplayer.SetMediaFilter",     NULL,               JseSetMediaFilter,         type );
    a_Hippo_API_JseRegister( "HDplayer.GetFileCount",       JseGetFileCount,    NULL,                      type );
    a_Hippo_API_JseRegister( "HDplayer.GetFileList",        JseGetFileList,     NULL,                      type );
    a_Hippo_API_JseRegister( "HDplayer.SearchFile",         JseSearchFile,      NULL,                      type );
    a_Hippo_API_JseRegister( "HDplayer.GetSearchedFileList",JseGetSearchedFileList,               NULL,    type );
    a_Hippo_API_JseRegister( "HDplayer.DeleteFile",         NULL,               JseDeleteFile,             type );
    a_Hippo_API_JseRegister( "HDplayer.RenameFile",         NULL,               JseRenameFile,             type );
    a_Hippo_API_JseRegister( "HDplayer.GetMediaInfo",       JseGetMediaInfo,    NULL,                      type );
    a_Hippo_API_JseRegister( "HDplayer.FilterFile",         JseFilterFile,    NULL,                      type );
    a_Hippo_API_JseRegister( "HDplayer.GetLyricsInfo",         JseGetLyricsInfo,    NULL,                      type );
    a_Hippo_API_JseRegister( "HDplayer.GetLyricsByLine",         JseGetLyricsByLine,    NULL,                      type );
    a_Hippo_API_JseRegister( "HDplayer.SetPictureScale",         NULL,    JseSetPictureScale,                      type );

    
    return 0;
}

}; //extern "C"

#endif
