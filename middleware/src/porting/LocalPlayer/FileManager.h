#ifndef FileManager_H
#define FileManager_H

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <vector>
#include <dirent.h>
#include "FileInfo.h"
#include "DirFileInfo.h"
#include "RegularFileInfo.h"

namespace Hippo{

class FileManager{    
public:
    enum ScanStatus{
        Scan_NotStart,
        Scan_Running,
        Scan_Finish,  
        Scan_stop,
    };
    FileManager(std::string path);
    ~FileManager();
    
    int startScan(std::string);
    int stopScan();  
    ScanStatus getScanStatus(){return m_ScanStatus; }   
    
    typedef bool filterFile(FileInfo*);
    int traverseFileTree(std::string path, filterFile* filetrFun, void* pData, std::vector<FileInfo*>* retFileList);
        
    FileInfo *getFileInfo(std::string path);    
    FileInfo *getFileInfo(int id);    
    int remove(std::string path);   
    int reName(FileInfo* file, std::string newName);
    int releaseFileTree(FileInfo *pNode, bool isRemoveFile = false);
    std::vector<std::string>* ParserString(std::string path, std::string separator);
    std::vector<std::string>* pathParser(std::string);
private:
    ScanStatus m_ScanStatus;
    int m_stopScanFlag;
    std::string m_rootDir;
    std::string m_scanDir;
    DirFileInfo *m_fileTree;
        
    pthread_t m_scanThreadID;
    static void * runScanDir( void *);
    int scanDir(DirFileInfo *pInfo, bool isRecurse = true ); 
};//class FileManager
    
};//namespace Hippo

#endif//FileManager_H
