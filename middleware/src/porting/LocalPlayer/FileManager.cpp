#include <stdio.h>
#include <sys/stat.h>
#include "FileManager.h"
#include "Assertions.h"

#define MAX_SCAN_FILE_COUNT 5000

namespace Hippo{

FileManager::FileManager(std::string path)
:m_ScanStatus(Scan_NotStart)
,m_stopScanFlag(0)
,m_scanDir("")
{
   m_rootDir = path;   
   m_fileTree = new DirFileInfo(path);       
}

FileManager::~FileManager()
{
    this->releaseFileTree(m_fileTree, false);
}

int FileManager::stopScan()
{
    if (m_ScanStatus == FileManager::Scan_Running)
        m_stopScanFlag = 1;
    
    while (m_ScanStatus == FileManager::Scan_Running)
        sleep(1);    
    
    m_ScanStatus = FileManager::Scan_stop;
    return 0;
}  
    
FileInfo *FileManager::getFileInfo(std::string path)
{
    LogUserOperDebug("FileManager::getFileInfo path is [%s]\n", path.c_str());
    if (access(path.c_str(), 0)) {
        LogUserOperError("getFileInfo path can not access\n");
        return NULL;    
    }   
      
    int index = 0;
    DirFileInfo *pInfo = m_fileTree;
    FileInfo *tempInfo = NULL;            
    std::vector<std::string> *rootPathList = pathParser(m_rootDir);  
    if (!rootPathList) {
        LogUserOperError("getFileInfo pathParser error 1\n");
        return NULL;    
    }
    std::vector<std::string> *cusPathList = pathParser(path);
    if (!cusPathList) {
        LogUserOperError("getFileInfo pathParser error 2\n");
        delete rootPathList;
        return NULL;    
    }        
    for(unsigned int i = 0; i < rootPathList->size(); i++) {//check user dir is in rootdir
        if (rootPathList->at(i).compare(cusPathList->at(i))) {
            LogUserOperError("user path error\n");
            goto Err;     
        }    
    }
    if (rootPathList->size() == cusPathList->size()) {
         tempInfo = static_cast<FileInfo*>(pInfo); 
         LogUserOperDebug("goto ok\n");  
         goto OK;
    }
    for (unsigned int i = rootPathList->size(); i < cusPathList->size(); i++ ) {
        if (!pInfo) {
            LogUserOperError("pInfo is NULL\n");
            goto Err;    
        }
        tempInfo = pInfo->getItemByName(cusPathList->at(i));
        if (!tempInfo) {
            LogUserOperDebug("getFileInfo this path is not scan\n");
            index = i;
            goto NotFind;    
        }    
        if (tempInfo->getFileType() == FileInfo::Dir_file)
            pInfo = static_cast<DirFileInfo*>(tempInfo);
        else
            pInfo = NULL;
    }
    
OK: 
    if (tempInfo->getFileType() == FileInfo::Dir_file) { //if dir file, check wheather need to scan dir
        m_stopScanFlag = 0;
        scanDir(static_cast<DirFileInfo*>(tempInfo), false ); 
    }
    delete rootPathList;
    delete cusPathList;
    return tempInfo;
     
NotFind: 
    for(unsigned int i = index; i < cusPathList->size(); i++) {
        if (!pInfo) {
            LogUserOperError("222pInfo is NULL\n");
            goto Err;    
        } 
        std::string path = pInfo->getFullPath() + "/" + cusPathList->at(i);
        LogUserOperDebug("path is [%s]\n", path.c_str()); 
             
             
 
        if (i < cusPathList->size() - 1){ 
            tempInfo = new DirFileInfo(path);
        } else {
            struct stat buf;    
            if (stat(path.c_str(), &buf)) {//error happens , try stat64  
                LogUserOperError("stat error\n");
            }       
            if (S_ISDIR(buf.st_mode) )        
                tempInfo = new DirFileInfo(path);
            else
                tempInfo = new RegularFileInfo(path);    
        }
        pInfo->addItem(tempInfo);
        if (tempInfo->getFileType() == FileInfo::Dir_file)
            pInfo = static_cast<DirFileInfo*>(tempInfo);
        else
            pInfo = NULL;       
    }
    if (tempInfo->getFileType() == FileInfo::Dir_file) {//when last file is dir
        m_stopScanFlag = 0;
        scanDir(static_cast<DirFileInfo*>(tempInfo), false );
    }    
    delete rootPathList;
    delete cusPathList;
    return tempInfo;
    
Err:
    delete rootPathList;
    delete cusPathList;        
    return NULL;
}      

FileInfo *FileManager::getFileInfo(int id)
{
    LogUserOperDebug("getFileInfo id [%d]\n", id);
    std::string path = m_fileTree->getPathById(id);
    if (!path.compare("")) {
        LogUserOperError("This id can not find path\n");
        return NULL;    
    }   
    return getFileInfo(path);
}    

int FileManager::remove(std::string path)      
{
     FileInfo *pInfo = getFileInfo(path);
     return releaseFileTree(pInfo, true);
}    

int FileManager::reName(FileInfo* file, std::string newName)
{
    if (!file) {
        LogUserOperError("rename error\n");
        return -1;
    }
    char oldN[1024] = {0};
    snprintf(oldN, 1024, "%s", file->getFullPath().c_str());
    char newN[1024] = {0};
    snprintf(newN, 1024, "%s/%s", file->getFullPath().substr(0, file->getFullPath().rfind("/") ).c_str(), newName.c_str() );
    LogUserOperDebug("old filePath is [%s], newName paths is [%s]\n", oldN, newN);
    if (rename(oldN, newN)) {
        LogUserOperError("rename error,,\n");
        return -1;        
    }
    sync();
    releaseFileTree(file, false); 
    FileInfo* parent = file->getParentDir();
    scanDir((DirFileInfo*)parent, true);
    while (m_ScanStatus == Scan_Running)
        sleep(1);

    return 0;
}

int FileManager::startScan(std::string path)
{
    LogUserOperDebug("startScan\n");
    if (m_ScanStatus == Scan_Running) {
        LogUserOperDebug("scan thread is running\n");
        return 0;    
    }
    m_scanDir = path;
    m_ScanStatus = Scan_Running;
    m_stopScanFlag = 0;
    if (pthread_create(&m_scanThreadID, NULL, runScanDir, (void *)this)) {
        LogUserOperError("pthread create runScandir error\n");
        m_ScanStatus = Scan_NotStart;
        return -1;    
    }    
    pthread_detach(m_scanThreadID);
    return 0;
}

void* FileManager::runScanDir(void *p)
{
    if (!p) {
        LogUserOperError("runScanDir error\n");
        return NULL;    
    }
    FileManager *self = (FileManager*)p;
    self->scanDir(static_cast<DirFileInfo*>(self->getFileInfo(self->m_scanDir)), true);
    self->m_ScanStatus = Scan_Finish;;//scan finish
    return NULL;
}        

int FileManager::scanDir(DirFileInfo *pInfo, bool isRecurse )
{
    if (m_stopScanFlag) {
        LogUserOperDebug("scanDir stop\n");
        return 0;    
    }
    if (!pInfo) {
        LogUserOperError("scanDir pInfo is NULL\n");
        return -1;    
    }             
    if (pInfo->getTotalFileCount() > MAX_SCAN_FILE_COUNT) {
         //LogUserOperError("MAX_SCAN_FILE_COUNT is reached, stop scan\n");
         return 0;
    }
        
    if (pInfo->getScanFlag() == 0) {//this dir not scaned
        DIR * handle = opendir(pInfo->getFullPath().c_str());   
        if (!handle) {
            LogUserOperDebug("opendir error\n");
            m_stopScanFlag = 1;
            return -1;    
        }
        struct dirent *dirContent = NULL;
        while( (dirContent = readdir(handle)) != 0) {
            if (dirContent->d_name[0] == '.')//hide files its name start with '.'
                continue;  
    	     
    	    std::string fullPath = pInfo->getFullPath() + "/" + dirContent->d_name;
    	    //LogUserOperDebug("dir item full path is [%s]\n", fullPath.c_str());
            if (dirContent->d_type == DT_REG) {//regular file
                if (RegularFileInfo::getFileclassification(fullPath) != RegularFileInfo::UnkonwFile) {
                    RegularFileInfo *regFile = new RegularFileInfo(fullPath);
                    pInfo->addItem(regFile);
                }
            }	             
            else if (dirContent->d_type == DT_DIR) { //dir file
                DirFileInfo *DirFile = new DirFileInfo(fullPath);
                pInfo->addItem(DirFile);    
            }
            else { //other file
                //do nothing
            }
        }
        closedir(handle);
        pInfo->setScanFlag();
    }
    if (isRecurse) {
        std::vector<FileInfo *>* itemList = pInfo->getItemList();
        if (itemList) {
            for (unsigned int i = 0; i < itemList->size(); ++i) {
                if (itemList->at(i)->getFileType() == FileInfo::Dir_file) {
                    scanDir(static_cast<DirFileInfo*>(itemList->at(i))); //recurse
                }
            } 
            delete itemList;
        }
    }
    return 0;
} 

int FileManager::traverseFileTree(std::string path, filterFile* filetrFun,  void* pData, std::vector<FileInfo*>* retFileList)
{  
    FileInfo *pInfo = getFileInfo(path);
    if (!pInfo) {
        perror("getFileInfo error\n");
        return -1;    
    }    
    if (filetrFun(pInfo)) {
        if (retFileList)
            retFileList->push_back(pInfo);
    }

    if (pInfo->getFileType() == FileInfo::Dir_file) {
        std::vector<FileInfo *> *subFileList = (static_cast<DirFileInfo*>(pInfo))->getItemList();   
        for(unsigned int i = 0; i < subFileList->size(); i++) 
            traverseFileTree(subFileList->at(i)->getFullPath(), filetrFun, pData, retFileList);   //recursion
    } 
      
    return 0;    
}

int FileManager::releaseFileTree(FileInfo *pNode, bool isRemoveFile) 
{
    if (!pNode) {
        LogUserOperError("releaseFileTree error pNode is NULL\n");
        return -1;    
    } 
    
    DirFileInfo *pParentDir = static_cast<DirFileInfo*>(pNode->getParentDir());
    if (pParentDir) {
        if (!isRemoveFile) {
            pParentDir->resetScanFlag();
        }
        pParentDir->removeItem(pNode);  
    }    

    if (pNode->getFileType() == FileInfo::Regular_file){        
        if (isRemoveFile)
            pNode->removeFile();
        delete pNode;
    }   
    else if (pNode->getFileType() == FileInfo::Dir_file) {
        DirFileInfo *pDirInfo = static_cast<DirFileInfo*>(pNode);
        std::vector<FileInfo *> *fileList = pDirInfo->getItemList();   
        for(unsigned int i = 0; i < fileList->size(); i++) {
            FileManager::releaseFileTree(fileList->at(i));   //recursion
        }
        delete fileList;
        if (isRemoveFile)
            pNode->removeFile();        
        delete pNode;
    } 
    else {
        LogUserOperError("releaseFileTree unknow file type\n");    
    }
    return 0;
}      

/*path shoud like /mnt/usb1/video  */
/*user should release vector after use*/
std::vector<std::string>* FileManager::pathParser(std::string path)
{
    std::vector<std::string> *pList = new std::vector<std::string>;
    unsigned pos_left = 0;
    unsigned pos_right = 0;
    while(1) {
        pos_left = path.find_first_of("/", pos_right);
        if (pos_left == std::string::npos) {
            LogUserOperError("pathParser pos_left is npos\n");
            return NULL;    
        } 
        if (pos_left + 1 >= path.length()) {
            LogUserOperError("pathParser path is not valied\n");
            return NULL;             
        }        
        pos_right = path.find_first_of("/", pos_left + 1);
        if (pos_right == std::string::npos) {//last item
            std::string item = path.substr(pos_left + 1);
            pList->push_back(item);
            break;    
        }          
        if ((pos_right - pos_left - 1) > 0) {     
            std::string item = path.substr(pos_left + 1, pos_right - pos_left - 1);
            pList->push_back(item);
        }
    }    
    return pList;
} 

std::vector<std::string>* FileManager::ParserString(std::string path, std::string separator)
{
    std::vector<std::string> *pList = new std::vector<std::string>;
    if (path.length() == 0) {
        return pList;    
    }
    unsigned pos_left = 0;
    unsigned pos_right = 0;
    int fisstFlag = 0;    
    while(1) {
        pos_left = path.find_first_of(separator, pos_right);
        if (fisstFlag == 0) {
            fisstFlag = 1;
            if (pos_left == std::string::npos) {
                pList->push_back(path);
                break;
            } 
            if (pos_left != 0) {
                std::string item = path.substr(0, pos_left);
                pList->push_back(item);  
                pos_right =  pos_left;
                continue;
            }   
        }
        if (pos_left == std::string::npos) {
            LogUserOperError("ParserString pos_left is npos\n");
            return NULL;    
        } 
        if (pos_left + 1 >= path.length()) {
            break;             
        }        
        pos_right = path.find_first_of(separator, pos_left + 1);
        if (pos_right == std::string::npos) {//last item
            std::string item = path.substr(pos_left + 1);
            pList->push_back(item);
            break;    
        }          
        if ((pos_right - pos_left - 1) > 0) {     
            std::string item = path.substr(pos_left + 1, pos_right - pos_left - 1);
            pList->push_back(item);
        }
    }    
    return pList;
}     

}; //namespace Hippo
