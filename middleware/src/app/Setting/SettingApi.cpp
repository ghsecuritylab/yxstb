#include "SettingApi.h"

#include "Assertions.h"
#include "ind_mem.h"
#include "ind_cfg.h"
#include "sys_basic_macro.h"
#include "mid/mid_mutex.h"
#include "config/pathConfig.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>

#define CONFIG_LEN (32 * 1024)

int settingConfigRead(CfgTree_t tree, char *rootname)
{
    int len = 0;
    FILE *fp = NULL;
    std::string filename("");

    char *buf = (char *)IND_MALLOC(CONFIG_LEN);
    if(buf == NULL) {
        LogSafeOperError("malloc :: memory");
        return -1;
    }

    len = strlen(rootname);
    if(len > 16) {
        LogSafeOperError("%s too large\n", rootname);
        return -1;
    }
    filename +=  CONFIG_FILE_DIR"/yx_config_";
    filename += rootname;
    filename += ".ini";
    if (access(filename.c_str(), (F_OK | R_OK))) {
        LogSafeOperError("[%s] not exist or can not read\n", filename.c_str());
        return -1;
    }

    fp = fopen(filename.c_str(), "rb");
    if(NULL == fp) {
        LogSafeOperError("Can not open file [%s]\n", filename.c_str());
        return -1;
    }

    len = fread(buf, 1, CONFIG_LEN, fp);
    fclose(fp);
    if(len <= 0 || len >= CONFIG_LEN) {
        LogSafeOperError("len = %d, CONFIG_LEN = %d\n", len, CONFIG_LEN);
        return -1;
    }
    buf[len] = 0;
    ind_cfg_input(tree, rootname, buf, len);
    IND_FREE(buf);
    return 0;
}

int settingConfigWrite(CfgTree_t tree, char *rootname)
{
    int len = 0;
    FILE *fp = NULL;
    std::string filename("");
    static mid_mutex_t mutex;

    char *buf = (char *)IND_MALLOC(CONFIG_LEN);
    if(buf == NULL) {
        LogSafeOperError("malloc :: memory");
        return -1;
    }

    len = strlen(rootname);
    if(len > 16) {
        LogSafeOperError("%s too large\n", rootname);
        return -1;
    }

    if (!mutex)
        mutex = mid_mutex_create();

    if(mid_mutex_lock(mutex))
        LogSafeOperWarn("\n");

    filename +=  CONFIG_FILE_DIR"/yx_config_";
    filename += rootname;
    filename += ".ini";
    len = ind_cfg_output(tree, rootname, buf, CONFIG_LEN);
    if(len <= 0) {
        LogSafeOperError("tree_cfg_input = %d\n", len);
        mid_mutex_unlock(mutex);
        return -1;
    }

    mode_t oldMask = umask(0077);
    fp = fopen(filename.c_str(), "wb");
    umask(oldMask);
    if(NULL == fp) {
        LogSafeOperError("fopen = %s\n", filename.c_str());
        mid_mutex_unlock(mutex);
        return -1;
    }

    fwrite(buf, 1, len, fp);
    fclose(fp);
    sync();

    IND_FREE(buf);
    mid_mutex_unlock(mutex);
    return 0;
}
