
#include "JseHWFile.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "FileManager.h"
#include "FileInfo.h"
#include "DirFileInfo.h"
#include "RegularFileInfo.h"
#include "libzebra.h"

#include "config/pathConfig.h"

#include "json/json_tokener.h"
#include "json/json_object.h"
#include "json/json_public.h"

#include <string>
#include <vector>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PlayerRootDir "/mnt"

using namespace Hippo;

extern int currentDirId;
extern FileManager* pFileManager;
extern std::vector<FileInfo*> *pCurrentList;
extern int sortOrder;

extern "C" {
bool compareName(FileInfo *a, FileInfo *b);
bool compareSize(FileInfo *a, FileInfo *b);
bool compareType(FileInfo *a, FileInfo *b);
bool compareCreateTime(FileInfo *a, FileInfo *b);
int get_jpeg_info(char *path, int *w, int *h);
int get_png_info(char *path, int *w, int *h);
int get_gif_info(char *path, int *w, int *h);
int get_bmp_info(char *path, int *w, int *h);
int ygp_pic_shrink(char *path, char *shrink_path, int *w, int *h, u32_t color, void *param);
}

static int JseFileOpenDirRead(const char *param, char *value, int len )
{
    if (!pFileManager) {
            pFileManager = new FileManager(PlayerRootDir);
            if (!pFileManager) {
                LogJseError("new pFileManager is NULL\n");
                return -1;
            }
    }
    if (!pCurrentList) {
           pCurrentList = new std::vector<FileInfo*>;
            if (!pCurrentList) {
                LogJseError("new pCurrentList is NULL\n");
                return -1;
            }
    }
    LogJseDebug("JseFileOpenDir param [%s]\n", param);
    json_object *json_dir_info = json_tokener_parse(param);
    if (!json_dir_info) {
        LogJseError("JseFileOpenDir, json_tokener_parse error\n");
        return -1;
    }
    std::string path = (char*)json_object_get_string(json_object_object_get(json_dir_info, "path"));
    std::string filterStr = (char*)json_object_get_string(json_object_object_get(json_dir_info, "filter"));
    int tSort = json_object_get_int(json_object_object_get(json_dir_info, "sort"));
    int order = json_object_get_int(json_object_object_get(json_dir_info, "order"));
    json_object_put(json_dir_info);

    unsigned int found = 0;
    int isFilterVideo = 0;
    int isFilterAudio = 0;
    int isFilterImage = 0;
    int isFilterOther = 0;
    int isShowAll = 0;
    while ((found = filterStr.find_first_of("12345",found)) != std::string::npos) {
        int filter = atoi(filterStr.substr(found, 1).c_str());
        if (1 == filter )/*video*/
            isFilterVideo = 1;
        else if ( 2 == filter )/*audio*/
            isFilterAudio = 1;
        else if ( 3 == filter )/*image*/
            isFilterImage = 1;
        else if ( 4 == filter )/*other*/
            isFilterOther = 1;
        else if ( 5 == filter )/*all*/
            isShowAll = 1;

        if ((found = filterStr.find_first_of(",",found)) == std::string::npos)
            break;
    }
    LogJseDebug("sort[%d],isFilterVideo[%d],isFilterAudio[%d],isFilterImage[%d], isFilterGame[%d], isShowAll[%d]",
                 tSort, isFilterVideo, isFilterAudio, isFilterImage, isFilterOther, isShowAll);

    std::string path2 = "";
    if (path.find("file:///") != std::string::npos ) {
        path2 = path.substr(7);
    } else {
        path2 = path;
    }
    FileInfo * pInfo = pFileManager->getFileInfo(path2);
    if (!pInfo) {
        LogJseError("JseFileOpenDir getFileInfo error\n");
        return -1;
    }

    if (pInfo->getFileType() != FileInfo::Dir_file) {
        LogJseError("user input path is not dir file\n");
        return -1;
    }
    DirFileInfo *pDirInfo = static_cast<DirFileInfo*>(pInfo);

    if (pCurrentList)
        delete pCurrentList;
    pCurrentList = pDirInfo->getItemList();
    for (std::vector<FileInfo*>::iterator it = pCurrentList->begin(); it != pCurrentList->end(); ++it) {
        if (isShowAll) {
            break;
        }

        if ((*it)->getFileType() == FileInfo::Regular_file) {
            if (!isFilterVideo && (static_cast<RegularFileInfo*>(*it)->getClassification() == RegularFileInfo::VideoFile)) {
                pCurrentList->erase(it);
                it--;
                continue;
            }
            if (!isFilterAudio && (static_cast<RegularFileInfo*>(*it)->getClassification() == RegularFileInfo::AudioFile)) {
                pCurrentList->erase(it);
                it--;
                continue;
            }
            if (!isFilterImage && (static_cast<RegularFileInfo*>(*it)->getClassification() == RegularFileInfo::ImageFile)) {
                pCurrentList->erase(it);
                it--;
                continue;
            }
            if (!isFilterOther && (static_cast<RegularFileInfo*>(*it)->getClassification() == RegularFileInfo::UnkonwFile)) {
                pCurrentList->erase(it);
                it--;
                continue;
            }
        }
    }

    sortOrder = order;
    if (0 == tSort )/*name*/
        sort(pCurrentList->begin(), pCurrentList->end(), compareName);
    else if (1 == tSort)/*size*/
        sort(pCurrentList->begin(), pCurrentList->end(), compareSize);
    else if (2 == tSort)/*type*/
        sort(pCurrentList->begin(), pCurrentList->end(), compareType);
    else if (3 == tSort)/*time*/
        sort(pCurrentList->begin(), pCurrentList->end(), compareCreateTime);
    else
        sort(pCurrentList->begin(), pCurrentList->end(), compareName);

    for (unsigned int i = 0; i < pCurrentList->size(); i++) {
        LogJseDebug("%s\n", pCurrentList->at(i)->getName().c_str());
    }
    currentDirId = pDirInfo->getFileId();
    snprintf(value, 4096, "%d", pDirInfo->getFileId());
    LogJseDebug("opendir [%s]\n", value);
    return 0;
}

static int JseFileGetCountRead(const char *param, char *value, int len )
{
    if (!pFileManager) {
        LogJseError("pFileManager is NULL\n");
        return -1;
    }
    if (!pCurrentList) {
        LogJseError("pCurrentList is NULL\n");
        return -1;
    }
    LogJseDebug("param [%s]\n", param);
    json_object *json_info = json_tokener_parse(param);
    if (!json_info){
        LogJseError("JseFileGetCount json parse error\n");
        return -1;
    }
    std::string globalId= (char*)json_object_get_string(json_object_object_get(json_info, "dirID"));
    json_object_put(json_info);
    int id = atoi(globalId.c_str());
    LogJseDebug("JseFileGetCount dir id [%d], count [%d]\n", id, pCurrentList->size());

    snprintf(value, 4096, "%d", pCurrentList->size());
    return 0;
}

static int JseFileGetListRead(const char *param, char *value, int len )
{
    if (!pFileManager) {
        LogJseError("pFileManager is NULL\n");
        return -1;
    }
    if (!pCurrentList) {
        LogJseError("pCurrentList is NULL\n");
        return -1;
    }
    LogJseDebug("param [%s]\n", param);
    json_object *json_info = json_tokener_parse(param);
    if (!json_info){
        LogJseError("JseFileGetCount json parse error\n");
        return -1;
    }
    int dirID = atoi((char*)json_object_get_string(json_object_object_get(json_info, "dirID")));
    int tIndex = atoi((char*)json_object_get_string(json_object_object_get(json_info, "index")));
    int tCount = atoi((char*)json_object_get_string(json_object_object_get(json_info, "count")));
    LogJseDebug("dirId [%d], index[%d], count[%d]\n", dirID, tIndex, tCount);
    json_object_put(json_info);

    json_object *json_file_array_info = json_object_new_array();
    if (!json_file_array_info) {
        LogJseError("JseFileGetList error json_object_new_array\n");
        return -1;
    }

    int fileCount = pCurrentList->size();
    int tGetFileCount = 0;
    for (int i = tIndex; (i < tCount + tIndex) && (i < fileCount); i++) {
        FileInfo *pInfo = pCurrentList->at(i);
        json_object *json_file_info = json_object_new_object();
        json_object_object_add(json_file_info, "name", json_object_new_string(pInfo->getName().c_str()));
        if (pInfo->getFileType() == FileInfo::Dir_file) {
            json_object_object_add(json_file_info, "isdir", json_object_new_string("1"));
            json_object_object_add(json_file_info, "fileCount", json_object_new_int(static_cast<DirFileInfo*>(pInfo)->getItemCount()));
        } else {
            json_object_object_add(json_file_info, "isdir", json_object_new_string("0"));
            RegularFileInfo* p = (RegularFileInfo*)pInfo;
            if ((p->getClassification() == RegularFileInfo::VideoFile)
                || (p->getClassification() == RegularFileInfo::AudioFile)) {
                std::string fileFullPath = p->getFullPath();
                unsigned pos = fileFullPath.find_last_of(".");
                std::string picPath = fileFullPath.substr(0, pos) + ".jpeg";
                if (!access(picPath.c_str(), R_OK)) {
                    LogJseDebug("Exist smallName picPath [%s]\n", picPath.c_str());
                    json_object_object_add(json_file_info, "smallName", json_object_new_string( picPath.c_str()));
                }
                else {
                    LogJseDebug("Not exist smallName picPath [%s]\n", picPath.c_str());
                    json_object_object_add(json_file_info, "smallName", json_object_new_string(""));
                }
            }
            else if (p->getClassification() == RegularFileInfo::ImageFile) {
                std::string fileFullPath = p->getFullPath();
                json_object_object_add(json_file_info, "smallName", json_object_new_string( fileFullPath.c_str()));
            }
        }
        std::string url = "file://" + pInfo->getFullPath();
        json_object_object_add(json_file_info, "accessUrl", json_object_new_string(url.c_str()));
        json_object_object_add(json_file_info, "itemID", json_object_new_int(pInfo->getFileId()));
        char sizeBuf[256] = {0};
        snprintf(sizeBuf, 256, "%lld", pInfo->getFileSizeByte());
        json_object_object_add(json_file_info, "size", json_object_new_string(sizeBuf));
        time_t timeSec = pInfo->getFileCreateTime();
        struct tm  * ct = gmtime(&timeSec);
        char ctString[256] = {0};
        snprintf(ctString, 256, "%04d-%02d-%02d %02d:%02d:%02d", (ct->tm_year + 1900), (ct->tm_mon + 1), ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec);
        json_object_object_add(json_file_info, "ctime", json_object_new_string(ctString));
        timeSec = pInfo->getFileModifytime();
        struct tm  * mt = gmtime(&timeSec);
        char mtString[256] = {0};
        snprintf(mtString, 256, "%04d-%02d-%02d %02d:%02d:%02d", (mt->tm_year + 1900), (mt->tm_mon + 1), mt->tm_mday, mt->tm_hour, mt->tm_min, mt->tm_sec);
        json_object_object_add(json_file_info, "mtime", json_object_new_string(mtString));

        if (strlen((char*)json_object_to_json_string(json_file_array_info)) + strlen((char*)json_object_to_json_string(json_file_info)) > 4096) {
            LogJseDebug("waring: json string is more than 4096 byte\n");
            break;
        }
        json_object_array_add(json_file_array_info, json_file_info);
        tGetFileCount += 1;
    }
    char *jsonstr_file_info = (char *)json_object_to_json_string(json_file_array_info);
    snprintf(value, 4096, "{\"count\":%d,\"fileList\":%s}", tGetFileCount, jsonstr_file_info);
    LogJseDebug("value=[%s]\n", value);
    json_object_put(json_file_array_info);
    return 0;
}

static int JseFileDelWrite(const char *param, char *value, int len )
{
    if (!pFileManager) {
        LogJseError("pFileManager is NULL\n");
        return -1;
    }
    if (!pCurrentList) {
        LogJseError("pCurrentList is NULL\n");
        return -1;
    }
    LogJseDebug("param [%s]\n", param);
    json_object *json_info = json_tokener_parse(value);
    if (!json_info) {
        LogJseError("JseFileDel json_tokener_parse error\n");
        return -1;
    }
    std::string itemId = (char*)json_object_get_string(json_object_object_get(json_info, "itemID"));
    json_object_put(json_info);
//    pFileManager->remove(atoi(itemId.c_str()));
    return 0;
}

static int JseRenameFileWrite(const char* param, char* value, int len)
{
    if(!pFileManager) {
        LogJseError("pFileManager is NULL\n");
        return -1;
    }
    if (!pCurrentList) {
        LogJseError("pCurrentList is NULL\n");
        return -1;
    }
    struct json_object *object  = NULL;
    struct json_object* obj = NULL;
    std::string oldName;
    std::string newName;
    int ret = 0;
    LogJseDebug("value = %s\n", value);

    object = json_tokener_parse_string(value);
    if (!object) {
        json_object_delete(object);
        return -1;
    }
    obj = json_object_get_object_bykey(object,"oldName");
    if  (!obj) {
        json_object_delete(object);
        return -1;
    }
    oldName = json_object_get_string(obj);
    obj = json_object_get_object_bykey(object,"newName");
    if (!obj) {
        json_object_delete(object);
        return -1;
    }
    newName = json_object_get_string(obj);

    if (oldName.empty() || newName.empty()) {
        json_object_delete(object);
        return -1;
    }

    ret = rename(oldName.c_str(), newName.c_str());
    json_object_delete(object);

    return ret;
}

JseHWFile::JseHWFile()
    : JseGroupCall("File")
{
    JseCall* call;

    call = new JseFunctionCall("OpenDir", JseFileOpenDirRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("GetCount", JseFileGetCountRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("GetList", JseFileGetListRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("Del", 0, JseFileDelWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("RenameDir", 0, JseRenameFileWrite);
    regist(call->name(), call);
}

JseHWFile::~JseHWFile()
{
}


static int JseMediaInfoGet(const char* param, char* value, int len )
{
    if(!pFileManager) {
        LogJseError("pFileManager is NULL\n");
        return -1;
    }
    if (!pCurrentList) {
        LogJseError("pCurrentList is NULL\n");
        return -1;
    }
    LogJseDebug("JseMediaInfoGet param [%s]\n", param);

    json_object *json_info = json_tokener_parse(param);
    if (!json_info) {
        LogJseError("JseMediaInfoGet json_tokener_parse error\n");
        return -1;
    }
    std::string itemID = (char*)json_object_get_string(json_object_object_get(json_info, "itemID"));
    LogJseDebug("itemID [%s]\n", itemID.c_str());
    json_object_put(json_info);

    FileInfo *pInfo = pFileManager->getFileInfo(atoi(itemID.c_str()));
    if (!pInfo) {
        LogJseError("JseMediaInfoGet pInfo is NULL\n");
        return -1;
    }
    if (pInfo->getFileType() != FileInfo::Regular_file) {
        LogJseError("JseMediaInfoGet not media file\n");
        return -1;
    }
    RegularFileInfo *pFileInfo = static_cast<RegularFileInfo*>(pInfo);

    json_object *json_file_info = json_object_new_object();
    if (!json_file_info) {
        LogJseError("JseMediaInfoGet json_object_new_object \n");
        return -1;
    }
    unsigned pos = pFileInfo->getName().find_last_of(".");
    std::string fileExtension = "";
    if (pos != std::string::npos) {
        fileExtension = pFileInfo->getName().substr(pos + 1);
        json_object_object_add(json_file_info, "fileType", json_object_new_string(fileExtension.c_str()));
    }

    json_object_object_add(json_file_info, "totalTime", json_object_new_int(pFileInfo->getTotalTime()));
    json_object_object_add(json_file_info, "fileName", json_object_new_string(pFileInfo->getName().c_str()));
    char sizeBuf[256] = {0};
    snprintf(sizeBuf, 256, "%lld", pInfo->getFileSizeByte());
    json_object_object_add(json_file_info, "size", json_object_new_string(sizeBuf));
    time_t timeSec = pInfo->getFileCreateTime();
    struct tm  * ct = gmtime(&timeSec);
    char ctString[256] = {0};
    snprintf(ctString, 256, "%04d-%02d-%02d %02d:%02d:%02d", (ct->tm_year + 1900), (ct->tm_mon + 1), ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec);
    json_object_object_add(json_file_info, "ctime", json_object_new_string(ctString));
    timeSec = pInfo->getFileModifytime();
    struct tm  * mt = gmtime(&timeSec);
    char mtString[256] = {0};
    snprintf(mtString, 256, "%04d-%02d-%02d %02d:%02d:%02d", (mt->tm_year + 1900), (mt->tm_mon + 1), mt->tm_mday, mt->tm_hour, mt->tm_min, mt->tm_sec);
    json_object_object_add(json_file_info, "mtime", json_object_new_string(mtString));

    if (pFileInfo->getClassification() == RegularFileInfo::VideoFile) {
        YX_VIDEO_DECODER_STATUS videoStatus;
        memset(&videoStatus, 0, sizeof(YX_VIDEO_DECODER_STATUS));
        if (ymm_decoder_getVideoStatus(&videoStatus) == 0) {
            std::string videoType = "";
            switch (videoStatus.type) {
            case YX_VIDEO_TYPE_MPEG1:
                videoType = "MPEG1";
                break;
            case YX_VIDEO_TYPE_MPEG2:
                videoType = "MPEG2";
                break;
            case YX_VIDEO_TYPE_H264:
                videoType = "H264";
                break;
            case YX_VIDEO_TYPE_H263:
                videoType = "H263";
                break;
            case YX_VIDEO_TYPE_VC1:
                videoType = "VC1";
                break;
            case YX_VIDEO_TYPE_VC1_SM:
                videoType = "VC1_SM";
                break;
            case YX_VIDEO_TYPE_MPEG4PART2:
                videoType = "MPEG4PART2";
                break;
            case YX_VIDEO_TYPE_DIVX311:
                videoType = "DIVX311";
                break;
            case YX_VIDEO_TYPE_AVS:
                videoType = "AVS";
                break;
            case YX_VIDEO_TYPE_REAL8:
                videoType = "REAL8";
                break;
            case YX_VIDEO_TYPE_REAL9:
                videoType = "REAL9";
                break;
            default:
                videoType = "MPG";
                break;
            }

            std::string frameRate = "";
            switch(videoStatus.frame_rate) {
            case YX_VIDEO_FRAME_RATE_23_976:
                frameRate = "23";
                break;
            case YX_VIDEO_FRAME_RATE_24:
                frameRate = "24";
                break;
            case YX_VIDEO_FRAME_RATE_25:
                frameRate = "25";
                break;
            case YX_VIDEO_FRAME_RATE_29_97:
                frameRate = "29";
                break;
            case YX_VIDEO_FRAME_RATE_30:
                frameRate = "30";
                break;
            case YX_VIDEO_FRAME_RATE_50:
                frameRate = "50";
                break;
            case YX_VIDEO_FRAME_RATE_59_94:
                frameRate = "59";
                break;
            case YX_VIDEO_FRAME_RATE_60:
                frameRate = "60";
                break;
            default:
                frameRate = "30";
                break;
            }

            char videoFormat[16] = {0};
            sprintf(videoFormat, "%dx%d", videoStatus.source_width, videoStatus.source_height);

            json_object_object_add(json_file_info, "videoFormat", json_object_new_string(videoType.c_str()));
            json_object_object_add(json_file_info, "frameRate", json_object_new_string(frameRate.c_str()));
            json_object_object_add(json_file_info, "resolution", json_object_new_string(videoFormat));
        } else {
            LogJseError("ymm_decoder_getVideoStatus error\n");
        }
    }
    if ((pFileInfo->getClassification() == RegularFileInfo::VideoFile)
        || (pFileInfo->getClassification() == RegularFileInfo::AudioFile)) {
        YX_AUDIO_DECODER_STATUS audioStatus;
        memset(&audioStatus, 0, sizeof(YX_AUDIO_DECODER_STATUS));
        if (ymm_decoder_getAudioStatus(&audioStatus) == 0) {
            std::string audioType = "";
            switch(audioStatus.type) {
            case YX_AUDIO_TYPE_PCM:
                audioType = "PCM";
                break;
            case YX_AUDIO_TYPE_MPEG:
                audioType = "MPEG";
                break;
            case YX_AUDIO_TYPE_MP3:
                audioType = "MP3";
                break;
            case YX_AUDIO_TYPE_AAC:
                audioType = "AAC";
                break;
            case YX_AUDIO_TYPE_AACPLUS:
                audioType = "AACPLUS";
                break;
            case YX_AUDIO_TYPE_AC3:
                audioType = "AC3";
                break;
            case YX_AUDIO_TYPE_AC3PLUS:
                audioType = "AC3PLUS";
                break;
            case YX_AUDIO_TYPE_DTS:
                audioType = "DTS";
                break;
            case YX_AUDIO_TYPE_LPCM_DVD:
                audioType = "LPCM_DVD";
                break;
            case YX_AUDIO_TYPE_LPCM_HDDVD:
                audioType = "LPCM_HDDVD";
                break;
            case YX_AUDIO_TYPE_LPCM_BLURAY:
                audioType = "LPCM_BLURAY";
                break;
            case YX_AUDIO_TYPE_DTSHD:
                audioType = "DTSHD";
                break;
            case YX_AUDIO_TYPE_WMASTD:
                audioType = "WMASTD";
                break;
            case YX_AUDIO_TYPE_WMAPRO:
                audioType = "WMAPRO";
                break;
            case YX_AUDIO_TYPE_AVS:
                audioType = "AVS";
                break;
            case YX_AUDIO_TYPE_AACPLUS_ADTS:
                audioType = "AACPLUS_ADTS";
                break;
            case YX_AUDIO_TYPE_AAC_LOAS:
                audioType = "AAC_LOAS";
                break;
            case YX_AUDIO_TYPE_WAVPCM:
                audioType = "WAVPCM";
                break;
            case YX_AUDIO_TYPE_DRA:
                audioType = "DRA";
                break;
            case YX_AUDIO_TYPE_AMRNB:
                audioType = "AMRNB";
                break;
            case YX_AUDIO_TYPE_COOK:
                audioType = "COOK";
                break;
            default:
                audioType = "PCM";
                break;
            }
            json_object_object_add(json_file_info, "audioFormat", json_object_new_string(audioType.c_str()));
            json_object_object_add(json_file_info, "sampleRate", json_object_new_int(audioStatus.sample_rate));
        } else {
            LogJseError("ymm_decoder_getAudioStatus error\n");
        }
    }

    if (pFileInfo->getClassification() == RegularFileInfo::ImageFile) {
        LogJseDebug("JseMediaInfoGet RegularFileInfo::ImageFile\n");
        int w = 720;
        int h = 576;
        if ((strcasecmp(fileExtension.c_str(), "jpg") == 0)
            ||(strcasecmp(fileExtension.c_str(), "jpeg") == 0)) {
            get_jpeg_info((char*)pFileInfo->getFullPath().c_str(), &w, &h);
        }
        else if (strcasecmp(fileExtension.c_str(), "png") == 0) {
            get_png_info((char*)pFileInfo->getFullPath().c_str(), &w, &h);
        }
        else if (strcasecmp(fileExtension.c_str(), "gif") == 0) {
            get_gif_info((char*)pFileInfo->getFullPath().c_str(), &w, &h);
        }
        else if (strcasecmp(fileExtension.c_str(), "bmp") == 0) {
            get_bmp_info((char*)pFileInfo->getFullPath().c_str(), &w, &h);
        }
        else {
            LogJseError("unknow image formate\n");
        }
        char picResolution[256] = {0};
        snprintf(picResolution, 256, "%dx%d", w, h);
        json_object_object_add(json_file_info, "resolution", json_object_new_string(picResolution));

        LogJseDebug("cal ygp_pic_shrink to create thumbnail\n");
        w = 100;
        h = 100;
        char tempPath[] = {DEFAULT_TEMP_DATAPATH"/thumbnail"};
        int ret = ygp_pic_shrink((char *)pFileInfo->getFullPath().c_str(), tempPath, &w, &h, 0, 0);
        if (ret) {
            LogJseError("JseMediaInfoGet ygp_pic_shrink error\n");
            json_object_object_add(json_file_info, "imageDamage", json_object_new_int(1));
        }
        else
            json_object_object_add(json_file_info, "imageDamage", json_object_new_int(0));
    }

    int StreamRate = 0;
    if(pFileInfo->getTotalTime()) {
        StreamRate = pInfo->getFileSizeByte() / 1024;
        StreamRate = StreamRate / pFileInfo->getTotalTime();
    }
    json_object_object_add(json_file_info, "rate", json_object_new_int(StreamRate));

    char *jsonstr_file_info = (char *)json_object_to_json_string(json_file_info);
    strcpy(value, jsonstr_file_info);
    LogJseDebug("value=[%s]\n", value);
    json_object_put(json_file_info);
    return 0;
}

JseHWMediaInfo::JseHWMediaInfo()
    : JseGroupCall("MediaInfo")
{
    JseCall* call;

    call = new JseFunctionCall("Get", JseMediaInfoGet, 0);
    regist(call->name(), call);
}

JseHWMediaInfo::~JseHWMediaInfo()
{
}


/*************************************************
Description: 初始化华为播放流控中File模块配置定义的接口，由JseHWPlays.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWFileInit()
{
    JseCall* call;

    //C10/C20 regist
    call = new JseHWFile();
    JseRootRegist(call->name(), call);

    //C10/C20 regist
    call = new JseHWMediaInfo();
    JseRootRegist(call->name(), call);
    return 0;
}

