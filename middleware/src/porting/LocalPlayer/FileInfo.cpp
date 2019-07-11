
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#include "FileInfo.h"
#include "CodeTransform.h"
#include "Assertions.h"

namespace Hippo{

std::map<int, std::string> FileInfo::s_fileIdMap;
std::map<int, std::string> FileInfo::s_IdNameMap;
std::map<int, std::string> FileInfo::s_acronymMap;
    
FileInfo::FileInfo(std::string path, FileType type)
:m_parentDir(NULL)
{
    if (access(path.c_str(), 0)) {
        LogUserOperError("FileInfo access name error\n");
        return;    
    }    
    m_fullPath = path;
    unsigned int pos = m_fullPath.find_last_of("/");
    if (pos == std::string::npos) {
        LogUserOperError("#############Error path is error\n");
        return;
    }
    if (pos + 1 >= m_fullPath.length()) {
        LogUserOperError("path format is error\n");
        return;    
    }
    m_name = m_fullPath.substr(pos + 1);
    m_id = allocateFileId();
    m_type = type;    
    
    s_fileIdMap.insert(std::pair<int, std::string>(m_id, m_fullPath));
    s_IdNameMap.insert(std::pair<int, std::string>(m_id, m_name));  
    char acronym[1024] = {0};
    int outLen = 1024;
    getAcronymFromUtf8((unsigned char*)m_name.c_str(), m_name.length(), acronym, &outLen); 
    std::string strAcronym = acronym;
    s_acronymMap.insert(std::pair<int, std::string>(m_id, strAcronym));
        
    struct stat buf;    
    if (stat(m_fullPath.c_str(), &buf)) {//error happens , try stat64       
        int fd = open(m_fullPath.c_str(), O_RDONLY);
        if (fd == -1) {
            LogUserOperError("open file error\n");
            return;
        }
        struct stat64 pStat;
        if (fstat64(fd, &pStat)) {
            LogUserOperError("fstat64 file error\n");
            close(fd);
            return;                
        }        
		this->m_sizeByte = pStat.st_size;
		this->m_createTime = pStat.st_ctime;
		this->m_modifyTime = pStat.st_mtime;	
		close(fd);           
    } else {
		this->m_sizeByte = buf.st_size;
		this->m_createTime = buf.st_ctime;
		this->m_modifyTime = buf.st_mtime;        
    }
}    

FileInfo::~FileInfo()
{
    std::map<int,std::string>::iterator it = s_fileIdMap.find(m_id);
    if (it != s_fileIdMap.end()) {
        s_fileIdMap.erase(it);
    } 
    it = s_IdNameMap.find(m_id);
    if (it != s_fileIdMap.end()) {
        s_IdNameMap.erase(it);
    }     
    it = s_acronymMap.find(m_id);
    if (it != s_acronymMap.end()) {
        s_acronymMap.erase(it);
    }           
}  

int FileInfo::allocateFileId()
{
    static int id = 1;
    id++;
    if (id >= 0x1fffffff) {
        LogUserOperDebug("file is is more than 0x1fffffff\n");
    }
    return id;  
}

std::string FileInfo::getPathById(int id)
{
    std::map<int,std::string>::iterator it = s_fileIdMap.find(id);
    if (it == s_fileIdMap.end()) {
       LogUserOperError("getPathById error , invalid id\n");
       return ""; 
    }
    LogUserOperDebug("get id [%d], path[%s]\n", id, it->second.c_str());
    return it->second;
} 

int FileInfo::setParentDir( FileInfo *pInfo)   
{
    if (!pInfo) {
        LogUserOperError("setParentDir error pInfo is NULL\n");
        return -1;    
    }
    m_parentDir = pInfo;
    return 0;
}    

FileInfo* FileInfo::getParentDir()
{
    return m_parentDir;    
}

std::vector<int> * FileInfo::matchName(std::string key, int rule)
{
    std::vector<int> * result = new std::vector<int>;
    if (rule == 0) {//fuzzy match
        std::string key_lower = key;
        std::transform( key_lower.begin(), key_lower.end(), key_lower.begin(), std::ptr_fun <int, int> ( std::tolower ) );
        for (std::map<int, std::string>::iterator it = s_IdNameMap.begin(); it != s_IdNameMap.end(); ++it) {
            std::string name_lower = it->second;
            std::transform( name_lower.begin(), name_lower.end(), name_lower.begin(), std::ptr_fun <int, int> ( std::tolower ) );            
            if (name_lower.find(key_lower) != std::string::npos) {
                result->push_back(it->first);
            }            
        }  
    } 
    else if (rule == 1) {//exact match
        for (std::map<int, std::string>::iterator it = s_IdNameMap.begin(); it != s_IdNameMap.end(); ++it) {
            if (it->second.compare(key) == 0) {
                result->push_back(it->first);    
            } 
        }  
    } 
    else if (rule == 2) { //acronym match only consider Hanzi
        std::string key_lower = key;
        for (std::string::iterator it = key_lower.begin(); it != key_lower.end(); ++it) {
            if (!isalpha(*it)) {
                key_lower.erase(it);
                --it;   
            }    
        }    
        std::transform( key_lower.begin(), key_lower.end(), key_lower.begin(), std::ptr_fun <int, int> ( std::tolower ) );   
        for (std::map<int, std::string>::iterator it = s_acronymMap.begin(); it != s_acronymMap.end(); ++it) {               
            if (it->second.find(key_lower) != std::string::npos) {
                result->push_back(it->first);
            }
        }       
    }
    else if (rule == 3) { //acronym match consider Hanzi, character, Number
        std::string key_upper = key;
        for (std::string::iterator it = key_upper.begin(); it != key_upper.end(); ++it) {
            if (!isalnum(*it)) {
                key_upper.erase(it);
                --it;   
            }    
        }            
        std::transform( key_upper.begin(), key_upper.end(), key_upper.begin(), std::ptr_fun <int, int> ( std::toupper ) );        
        for(std::map<int, std::string>::iterator it = s_acronymMap.begin(); it != s_acronymMap.end(); ++it) {
            if (it->second.find(key_upper) != std::string::npos) {
                result->push_back(it->first);
            }
        }             
    }
    return result; 
}

}; //namespace Hippo