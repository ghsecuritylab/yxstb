#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "Assertions.h"
#include "UtilityTools.h"

int removeFile(const char *pFilePath)
{ 
    if (!pFilePath) {
        LogUserOperError("File path not exist\n");
        return -1;
    }
    struct stat buf;
    if (lstat(pFilePath, &buf)) {
        LogUserOperError("Stat file errer\n");
        return -1;
    }
    if ((geteuid() != buf.st_uid)
        && (getegid() != buf.st_gid)
        && (geteuid() != 0)) {
        LogUserOperError("Y have no access to delete the file \n");
        return -1;
    }
    
    int ret = 0;
    if (S_ISLNK(buf.st_mode)) {
        if (unlink(pFilePath)) {
            LogUserOperError("Unlink file error\n");
            ret = -1;        
        }		
    }
    else if (S_ISDIR(buf.st_mode)) {//dir file
        DIR * dir = NULL;       
        dir =opendir(pFilePath);   
        if (!dir) {
            LogUserOperError("Open dir error\n");  
            ret = -1;        
        } else {
            struct dirent * ptr = NULL;
            while((ptr = readdir(dir)) != NULL)
            {
                if (strncmp(ptr->d_name, ".", 1) && strncmp(ptr->d_name, "..", 2)) {
                    char tempPath[512] = {0};
                    sprintf(tempPath, "%s/%s", pFilePath, ptr->d_name);
                    removeFile(tempPath);
                }
            }
            closedir(dir);
            rmdir(pFilePath);	
        }
    } else { //other file
        if (unlink(pFilePath)) {
            LogUserOperError("Unlink file error\n");
            ret = -1;        
        }
    }
    return ret;
}

