
#include "JseHWLocalPlayer.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"
#include "File/JseHWFile.h"

#include "FileManager.h"
#include "FileInfo.h"
#include "DirFileInfo.h"
#include "RegularFileInfo.h"

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

static std::vector<FileInfo*> DirList;
static std::vector<FileInfo*> UnknowList;
static std::vector<FileInfo*> VideoList;
static std::vector<FileInfo*> AudioList;
static std::vector<FileInfo*> ImageList;
static std::vector<FileInfo*> AllList;

extern int currentDirId;
extern FileManager* pFileManager;
extern std::vector<FileInfo*> *pCurrentList;

extern "C" int ReleseDirInfo(const char *pPath);

static int JseSearchByKeyWordRead(const char* param, char* value, int len)
{
    if (!pFileManager) {
        LogJseError("pFileManager is NULL\n");
        return -1;
    }
    if (!pCurrentList) {
        LogJseError("pCurrentList is NULL\n");
        return -1;
    }
    LogJseDebug("JseSearchByKeyWordRead param [%s]\n", param);
    json_object *json_info = json_tokener_parse(param);
    if (!json_info) {
        LogJseError("JseSearchByKeyWordRead json_tokener_parse error\n");
        return -1;
    }
    std::string keyWord = (char*)json_object_get_string(json_object_object_get(json_info, "keyWord"));
    std::string matchRule = (char*)json_object_get_string(json_object_object_get(json_info, "matchRule"));
    std::string filter = (char*)json_object_get_string(json_object_object_get(json_info, "filter"));
    json_object_put(json_info);
    LogJseDebug("keyword[%s], matchRule[%s],filter[%s]\n", keyWord.c_str(), matchRule.c_str(), filter.c_str());

    FileInfo * currentInfo = pFileManager->getFileInfo(currentDirId);
    if (!currentInfo) {
        LogJseError("JseSearchByKeyWordRead currentInfo is null\n");
        return -1;
    } else {
        pFileManager->startScan(currentInfo->getFullPath());
    }

    int filterDir = 0;
    int filterUnknow = 0;
    int filterVideo = 0;
    int filterAudio = 0;
    int filterImage = 0;
    int filterAll = 0;
    std::string sep = ",";
    std::vector<std::string>*filterList = pFileManager->ParserString(filter, sep);
    if (filterList) {
        for (unsigned int i = 0; i < filterList->size(); i++) {
            if (atoi(filterList->at(i).c_str()) == 1) {
                filterVideo = 1;
            }
            else if (atoi(filterList->at(i).c_str()) == 2) {
                filterAudio = 1;
            }
            else if (atoi(filterList->at(i).c_str()) == 3) {
                filterImage = 1;
            }
            else if (atoi(filterList->at(i).c_str()) == 5) {
                filterAll = 1;
            }
            else if (atoi(filterList->at(i).c_str()) == 6) {
                filterDir = 1;
            }
            else if (atoi(filterList->at(i).c_str()) == 7) {
                filterUnknow = 1;
            }
        }
    }
    LogJseDebug("filterVideo[%d],filterAudio[%d],filterImage[%d],filterAll[%d]\n", filterVideo, filterAudio, filterImage, filterAll);
    int count = 1;
    while(1) {
        FileManager::ScanStatus status =  pFileManager->getScanStatus();
        if (status != FileManager::Scan_Running)
            break;
        else
            usleep(100000);
        count++;
        if (count%10 == 0)
            LogJseDebug("scan now , waiting...\n");
        if (count >= 50) {
            LogJseDebug("wait to much time\n");
            //break;
        }
    }
    int rule = 0;
    if (!matchRule.compare("exact"))
        rule = 1;
    else if (!matchRule.compare("acronym"))
        rule = 2;
    else if (!matchRule.compare("acronymMix"))
        rule = 3;

    std::vector<int>* result = pFileManager->getFileInfo(PlayerRootDir)->matchName(keyWord, rule);
    VideoList.clear();
    AudioList.clear();
    ImageList.clear();
    AllList.clear();
    for (unsigned int i = 0; i < result->size(); i++) {
        FileInfo *pInfo = pFileManager->getFileInfo(result->at(i))  ;
        if (pInfo) {//filter out items those not blong to the current dir
            std::vector<std::string> *currentPathList = pFileManager->pathParser(currentInfo->getFullPath());
            std::vector<std::string> *searchedPathList =  pFileManager->pathParser(pInfo->getFullPath());
            if ((!currentPathList) || (!searchedPathList)) {
                LogJseError("JseSearchByKeyWordRead (!currentPathList) || (!searchedPathList)\n");
                return -1;
            }
            int continueFlag = 0;
            if (searchedPathList->size() <= currentPathList->size())
                continueFlag = 1;
            else {
                for (unsigned int index = 0; index < currentPathList->size(); index++) {
                    if (currentPathList->at(index).compare(searchedPathList->at(index))) {
                        continueFlag = 1;
                        break;
                    }
                }
            }
            delete currentPathList;
            delete searchedPathList;
            if (continueFlag)
                continue;
        }

        if (pInfo && (pInfo->getFileType() == FileInfo::Regular_file)) {
            LogJseDebug("fileName is [%s] type is [%d]\n", pInfo->getName().c_str(), pInfo->getFileType());

            RegularFileInfo *pFileInfo = static_cast<RegularFileInfo*>(pInfo);
            if (pFileInfo->getClassification() == RegularFileInfo::VideoFile) {
                VideoList.push_back(pInfo);
                if (filterVideo || filterAll)
                    AllList.push_back(pInfo);
            }
            else if (pFileInfo->getClassification() == RegularFileInfo::AudioFile) {
                AudioList.push_back(pInfo);
                if (filterAudio || filterAll)
                    AllList.push_back(pInfo);
            }
            else if (pFileInfo->getClassification() == RegularFileInfo::ImageFile) {
                ImageList.push_back(pInfo);
                if (filterImage || filterAll)
                    AllList.push_back(pInfo);
            }
            else if (pFileInfo->getClassification() == RegularFileInfo::UnkonwFile) {
                UnknowList.push_back(pInfo);
                if (filterImage || filterAll)
                    AllList.push_back(pInfo);
            }
        }
        else if (pInfo && (pInfo->getFileType() == FileInfo::Dir_file)) {
            DirList.push_back(pInfo);
            if (filterDir || filterAll)
                AllList.push_back(pInfo);
        }
    }
    delete result;

    LogJseDebug("JseSearchByKeyWordRead start build json string\n");
    json_object *filterArray = json_object_new_array();

    if (filterVideo || filterAll) {
        json_object *videoInfo = json_object_new_object();
        json_object_object_add(videoInfo, "mediaType", json_object_new_string("1"));
        json_object_object_add(videoInfo, "count", json_object_new_int(VideoList.size()));
        json_object_array_add(filterArray, videoInfo);
    }

    if (filterAudio || filterAll) {
        json_object *audioInfo = json_object_new_object();
        json_object_object_add(audioInfo, "mediaType", json_object_new_string("2"));
        json_object_object_add(audioInfo, "count", json_object_new_int(AudioList.size()));
        json_object_array_add(filterArray, audioInfo);
    }

    if (filterImage || filterAll) {
        json_object *imageInfo = json_object_new_object();
        json_object_object_add(imageInfo, "mediaType", json_object_new_string("3"));
        json_object_object_add(imageInfo, "count", json_object_new_int(ImageList.size()));
        json_object_array_add(filterArray, imageInfo);
    }

    char *jsonstr_media_info = (char *)json_object_to_json_string(filterArray);

    int mediaCount =  AllList.size();
    snprintf(value, 4096, "{\"count\":%d,\"dirID\":%d,\"classDistribution\":%s}", mediaCount, currentDirId, jsonstr_media_info);
    LogJseDebug("aFiledvalue[%s]\n", value);
    json_object_put(filterArray);
    return 0;
}

static int JseGetSearchListRead(const char* param, char* value, int len)
{
    if (!pFileManager) {
        LogJseError("pFileManager is NULL\n");
        return -1;
    }
    if (!pCurrentList) {
        LogJseError("pCurrentList is NULL\n");
        return -1;
    }
    LogJseDebug("JseGetSearchListRead param [%s]\n", param);
    json_object *json_info = json_tokener_parse(param);
    if (!json_info){
        LogJseError("JseFileGetCount json parse error\n");
        return -1;
    }
    int dirID = atoi((char*)json_object_get_string(json_object_object_get(json_info, "dirID")));
    int tIndex = atoi((char*)json_object_get_string(json_object_object_get(json_info, "index")));
    int tCount = atoi((char*)json_object_get_string(json_object_object_get(json_info, "count")));
    LogJseDebug("JseGetSearchListRead dirId [%d], index[%d], count[%d]\n", dirID, tIndex, tCount);
    json_object_put(json_info);

    json_object *json_file_array_info = json_object_new_array();
    if (!json_file_array_info) {
        LogJseError("JseFileGetList error json_object_new_array\n");
        return -1;
    }

    int fileCount = AllList.size();
    LogJseDebug("filecount [%d]\n", fileCount);
    int tGetFileCount = 0;
    for (int i = tIndex; (i < tCount + tIndex) && (i < fileCount); i++) {
        FileInfo *pInfo = AllList.at(i);
        LogJseDebug("get search list[%s]\n", pInfo->getName().c_str());
        json_object *json_file_info = json_object_new_object();
        json_object_object_add(json_file_info, "name", json_object_new_string(pInfo->getName().c_str()));
        if (pInfo->getFileType() == FileInfo::Dir_file) {
            json_object_object_add(json_file_info, "isdir", json_object_new_string("1"));
            json_object_object_add(json_file_info, "fileCount", json_object_new_int(static_cast<DirFileInfo*>(pInfo)->getItemCount()));
        } else {
            json_object_object_add(json_file_info, "isdir", json_object_new_string("0"));
            json_object_object_add(json_file_info, "smallName", json_object_new_string( static_cast<RegularFileInfo*>(pInfo)->getThumbnailPath().c_str()));
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

static int JseEnableTagWrite(const char* param, char* value, int len)
{
    if (!value) {
        LogJseError("JseEnableTagWrite error\n");
        return -1;
    }
    int flag = atoi(value);
    if (0 == flag) {
        LogJseDebug("disable localplayer release buf\n");
        ReleseDirInfo(PlayerRootDir);
    }
    else if (1 == flag) {
         LogJseDebug("start localplayer\n");
        //now do nothing
    }
    else {
        LogJseError("JseEnableTagWrite error, unknow flag\n");
    }
    return 0;
}

JseHWLocalPlayer::JseHWLocalPlayer()
    : JseGroupCall("localPlayer")
{
    JseCall* call;

    call = new JseFunctionCall("searchByKeyWord", JseSearchByKeyWordRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("getSearchList", JseGetSearchListRead, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("enableTag", 0, JseEnableTagWrite);
    regist(call->name(), call);
}

JseHWLocalPlayer::~JseHWLocalPlayer()
{
}

/*************************************************
Description: 初始化华为播放流控中LocalPlayer模块配置定义的接口，由JseHWPlays.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWLocalPlayerInit()
{
    JseCall* call;

    //C10/C20 regist
    call = new JseHWLocalPlayer();
    JseRootRegist(call->name(), call);

    JseHWFileInit();
    return 0;
}

