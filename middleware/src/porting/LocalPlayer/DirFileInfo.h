#ifndef DirFileInfo_H
#define DirFileInfo_H

#include <unistd.h>
#include <map>
#include <vector>
#include <string>
#include "FileInfo.h"

namespace Hippo{

class DirFileInfo : public FileInfo{
public:
    DirFileInfo(std::string path);
    ~DirFileInfo();
    
    int addItem(FileInfo *);
    int removeItem(FileInfo *);
    int getItemCount();
    FileInfo *getItemByName(std::string);
    std::vector<FileInfo *>* getItemList();
    virtual int removeFile();
    int getScanFlag(){return scanFlag;}
    int setScanFlag(){scanFlag = 1; return 0;}
    int resetScanFlag(){scanFlag = 0; return 0;}
private:
    std::map<std::string, FileInfo *> m_itemMap;   
    int scanFlag;
};//class DirFileInfo

}; //namespace Hippo

#endif //DirFileInfo_H
