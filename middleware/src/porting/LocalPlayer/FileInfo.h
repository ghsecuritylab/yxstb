#ifndef FileInfo_H
#define FileInfo_H

#include <stdio.h>
#include <unistd.h>
#include <string>
#include <map>
#include <algorithm>
#include <cctype>
#include <functional>
#include <vector>

namespace Hippo{

class FileInfo{
public:
    enum FileType{
        Dir_file,
        Regular_file,
        Other_file,
    };
    FileInfo(std::string path, FileType type);
    ~FileInfo();
    
    std::string getName(){ return m_name;}
    std::string getFullPath(){ return m_fullPath;}
    FileType getFileType(){ return m_type;}
    int getFileId(){return m_id;}
    long long getFileSizeByte(){return m_sizeByte;}
    int getFileCreateTime(){ return m_createTime;}
    int getFileModifytime(){ return m_modifyTime;}

    std::string getPathById(int id);
    int setParentDir( FileInfo *);
    FileInfo* getParentDir();
    virtual int removeFile(){return 0;}
    std::vector<int> * matchName(std::string key, int rule);
    int getTotalFileCount(){ return s_fileIdMap.size();}
        
protected:
    std::string m_name;
    int m_id;
    std::string m_fullPath;
    FileType m_type;
    long long m_sizeByte;
    unsigned int m_createTime;
    unsigned int m_modifyTime;
    static int allocateFileId();
    static std::map<int, std::string> s_fileIdMap;
    static std::map<int, std::string> s_IdNameMap;
    static std::map<int, std::string> s_acronymMap;
    FileInfo *m_parentDir;   
}; //class FileInfo

}; //namespace Hippo

#endif //FileInfo_H
