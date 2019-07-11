
#ifdef INCLUDE_LocalPlayer


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "json/json.h"
#include "json/json_public.h"
#include "Hippo_api.h"
#include "Assertions.h"
#include "config/pathConfig.h"

#include "DirFileInfo.h"
#include "RegularFileInfo.h"
#include "FileManager.h"
#include "libzebra.h"
#include "Assertions.h"

#define PlayerRootDir "/mnt"

extern"C"{

using namespace Hippo;
using namespace std;

extern int get_jpeg_info(char *path, int *w, int *h);
extern int get_png_info(char *path, int *w, int *h);
extern int get_gif_info(char *path, int *w, int *h);
extern int get_bmp_info(char *path, int *w, int *h);
extern int ygp_pic_shrink(char *path, char *shrink_path, int *w, int *h, u32_t color, void *param);
int currentDirId = 0;
std::vector<FileInfo*> *pCurrentList = NULL;
FileManager * pFileManager = NULL;

int ReleseDirInfo(const char *pPath)
{
    if(!pFileManager) {
        LogUserOperError("pFileManager is NULL\n");
        return -1;
    }
    if (!pCurrentList) {
        LogUserOperError("pCurrentList is NULL\n");
        return -1;
    }

    if (!pPath) {
        LogUserOperError("LocalPlayerFileManagerReleseDir path is NULL\n");
        return -1;
    }
    LogUserOperDebug("ReleseDirInfo [%s]\n", pPath);
    std::string path = pPath;
    if (pCurrentList->size() > 0) {
        if (pCurrentList->at(0)->getFullPath().find(path) != std::string::npos) {
            pCurrentList->clear();
        }
    }
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

void LocalPlayerDiskRefresh(int type, const char *diskName, int position, const char *mountPoint)
{
    extern void HDPlayerDiskRefresh(int type, const char *diskName, int position, const char *mountPoint);
    HDPlayerDiskRefresh(type, diskName, position, mountPoint);
    if(!pFileManager) {
        LogUserOperError("pFileManager is NULL\n");
        return;
    }
    if (!pCurrentList) {
        LogUserOperError("pCurrentList is NULL\n");
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


int sortOrder = 0;
bool compareName(FileInfo *a, FileInfo *b)
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
        return item_1->getName() < item_2->getName();
    }
    if ((item_1->getFileType() == FileInfo::Regular_file) && (item_2->getFileType() == FileInfo::Regular_file)) {
        return item_1->getName() < item_2->getName();
    }
    return item_1->getFileType()<item_2->getFileType();
}

bool compareSize(FileInfo *a, FileInfo *b)
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

bool compareType(FileInfo *a, FileInfo *b)
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
    return item_1->getFileType() < item_2->getFileType();
}

bool compareCreateTime(FileInfo *a, FileInfo *b)
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

void a_LocalPlayer_JseMapInit()
{
    extern int  HDplayer_jseAPI_Register(ioctl_context_type_e type);
    HDplayer_jseAPI_Register(IoctlContextType_eHWBase);
}

};//extern"C"

#endif//Localplayer
