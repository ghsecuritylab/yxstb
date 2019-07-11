
#include "DirFileInfo.h"
#include "Assertions.h"

namespace Hippo{

DirFileInfo::DirFileInfo(std::string path)
:FileInfo(path, Dir_file)
,scanFlag(0)    
{
            
}

DirFileInfo::~DirFileInfo()
{
    
} 

int DirFileInfo::addItem(FileInfo *item)
{
    if (!item) {
        LogUserOperError("addItem error item is NULL\n");
        return -1;    
    } 
    item->setParentDir(this);       
    m_itemMap.insert(std::pair<std::string, FileInfo*>(item->getName(), item));
    return 0;
}   

int DirFileInfo::removeItem(FileInfo *pInfo)
{
    if (!pInfo) {
        LogUserOperError("removeItem error pInfo is NULL\n");
        return -1;    
    }        
    std::map<std::string, FileInfo *>::iterator it = m_itemMap.find(pInfo->getName());
    if (it == m_itemMap.end()) {
        LogUserOperError("removeItem error not find it\n");
        return -1;
    }        
    m_itemMap.erase(it);
    return 0;
}

int DirFileInfo::getItemCount()
{
    return m_itemMap.size();
}

FileInfo *DirFileInfo::getItemByName(std::string name)
{
    std::map<std::string, FileInfo*>::iterator it =  m_itemMap.find(name);
    if (it == m_itemMap.end()) {
        LogUserOperError("getItemByName error not find it\n");
        return NULL;
    }
    return it->second;
}

std::vector<FileInfo *>* DirFileInfo::getItemList()
{
    std::vector<FileInfo *>  *fileList = new std::vector<FileInfo *>;
    for (std::map<std::string, FileInfo*>::iterator it = m_itemMap.begin(); it != m_itemMap.end(); ++it) {
        fileList->push_back(it->second);
    }   
    return fileList;
}    

int DirFileInfo::removeFile()
{
    LogUserOperDebug("remove dir [%s]\n", m_fullPath.c_str());
    rmdir(m_fullPath.c_str());
    return 0;
}

}; //namespace Hippo