#include "UdiskUpgrade.h"
#include "UpgradeManager.h"
#include "FileStreamDataSource.h"
#include "UpgradeCommon.h"
#include "UpgradeSource.h"
#include "UpgradeUdiskSource.h"
#include "UpgradeAssertions.h"
#include "UpgradeData.h"
#include "stbinfo/stbinfo.h"

#include "mid/mid_tools.h"
#include "libzebra.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

extern "C"  void mid_sys_stbtype_get(char *stbType);

namespace Hippo {

static FileStreamDataSource g_UdiskDataSource;
static UpgradeSource* g_UdiskUpgradeSource = NULL;

static int UdisUpgradeDetect(const char* upgradePath)
{
    DIR* upgradeDir = NULL;
    struct dirent* ptr = NULL;
    FILE *fp = NULL;
    struct stat lastver;
    struct stat comver;
    char filePath[256] = {0};
    char tempPath[256] = {0};
    char stbType[32] = {0};
    char configInfo[1024] = {0};
    char versionPath[256] = {0};
    char fileName[64] ={0};
    char* p = NULL;
    int ret = 0;

    snprintf(stbType, sizeof(stbType), "%s", StbInfo::STB::Model());
    UpgradeLogDebug("app_upgrade stbType:%s\n",stbType);
    memset(&lastver,0,sizeof(struct stat));
    memset(&comver,0,sizeof(struct stat));
    UpgradeLogDebug("upgrade :%s\n",upgradePath);
    upgradeDir = opendir(upgradePath);
    if(NULL == upgradeDir)
        return 1;

    while((ptr = readdir(upgradeDir))!=NULL){
        UpgradeLogDebug("ptr->d_name:%s\n",ptr->d_name);
        p = strstr(ptr->d_name,stbType);
        if(p){
            memset(tempPath,0,sizeof(tempPath));
            sprintf(tempPath,"%s/%s", upgradePath, ptr->d_name);
            UpgradeLogDebug("tempPath:%s, Line:%d\n", tempPath, __LINE__);
            if (lstat(tempPath,&comver))
                UpgradeLogDebug("lstat err\n");
            if(comver.st_mtime >= lastver.st_mtime){
                memset(filePath,0,sizeof(filePath));
                strcpy(filePath,tempPath);
                lastver.st_mtime = comver.st_mtime ;
            }
        }
    }
    closedir(upgradeDir);

    if(0 == strcmp(filePath,"")) {
        UpgradeLogDebug("error: filepath is NULL!\n");
        return 1;
    }

    UpgradeLogDebug("filepath is %s\n",filePath);
    upgradeDir = opendir(filePath);

    if(NULL == upgradeDir)
        return 1;

    strncpy(versionPath, filePath, 256);
    closedir(upgradeDir);

    strcat(filePath, "/config.ini");
    if( access( filePath, R_OK ) < 0 ){
        UpgradeLogDebug("ERROR,There is no this upgrade file is %s\n",filePath);
        return 1;
    }

    fp = fopen(filePath,"r+");
    if(!fp)
        return 1;
    ret = fread(configInfo, 1, 1024, fp);

    if(ret <= 0){
        fclose(fp);
        UpgradeLogDebug("filePath:%s,read failed\n",filePath);
        return 1;
    }
    fclose(fp);
    UpgradeLogDebug("configInfo = %s\n", configInfo);

    if (GetUpgradeFileUrl(configInfo, NULL, 0, versionPath, true, g_UdiskUpgradeSource) < 0)
        return 1;

    return 0;
}

int UdiskUpgradeStart(const char* upgradePath)
{
    if (!g_UdiskUpgradeSource)
        g_UdiskUpgradeSource = new UpgradeUdiskSource();

    int ret = UdisUpgradeDetect(upgradePath);

    UpgradeLogDebug("ret = %d\n", ret);

    if (ret == 0) {
        g_UdiskUpgradeSource->m_dataSource = &g_UdiskDataSource;
        g_UdiskDataSource.setBuffer(g_UdiskUpgradeSource->getRingBuffer());

        bool result = upgradeManager()->startMonitorORUdiskUpgrade(g_UdiskUpgradeSource);
        if (result)
            return 0;

        return -1;
    }

    return ret;
}

}

