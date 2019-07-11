
#include "MonitorConfig.h"

#include "cryptoFunc.h"
#include "charConvert.h"

#include "config/pathConfig.h"
#include "Assertions.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>


static char LOCALKEY[]=  "Hybroad Vision..";

#ifdef __cplusplus
extern "C" {
#endif


static void useDefaultMonitorInfo(char* user, int userlen, char* pswd, int pswdlen)
{
    if (user && userlen > 0)
        snprintf(user, userlen, "huawei");
    if (pswd && pswdlen > 0){
        snprintf(pswd, pswdlen, ".287aW");
	}
}

void getMonitorInfo(char* user, int userlen, char* pswd, int pswdlen)
{
    char* p;
    char* buffer;
    int len, total;
    FILE* fp = fopen(CONFIG_FILE_DIR"/monitorpasswd", "rb");
    if (!fp) {
        goto Err;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer = (char*)malloc(len + 1);
    memset(buffer, 0, len + 1);
    total = 0;
    while (total < len) {
        int bytes_read = fread(buffer + total, 1, len - total, fp);
        if (bytes_read <= 0) {
            fclose(fp);
            free(buffer);
            goto Err;
        }
        total += bytes_read;
    }
    fclose(fp);

    p = strchr(buffer, ':');
    if (!p) {
        free(buffer);
        goto Err;
    }
    *p = '\0';
    p++;
    if (pswd && pswdlen > 0) {
        char * buf = (char*)malloc(strlen(p));
        memset(buf, 0, strlen(p));
        char tmp[512] = {0};
        int ret = hex2Data(p, strlen(p), tmp, sizeof(tmp));
        aesEcbDecrypt(tmp, ret, LOCALKEY, buf, strlen(p));
        snprintf(pswd, pswdlen, "%s", buf);
        free(buf);
    }
    if (user && userlen > 0) {
        snprintf(user, userlen, "%s", buffer);
    }
    free(buffer);
    return;
Err:
    useDefaultMonitorInfo(user, userlen, pswd, pswdlen);
    return;
}

int checkMonitorInfo(const char* user, const char* pswd)
{
    char u[1024], p[1024];
    ASSERT(user != NULL || pswd != NULL);
    if (!user && !pswd)
        return -1;
    getMonitorInfo(u, sizeof(u), p, sizeof(p));
    if (user) {
        if (strcmp(user, u))
            return 0;
    }
    if (pswd) {
        if (strcmp(pswd, p))
            return 0;
    }
    return 1;
}

void saveMonitorInfo(const char* user, const char* pswd)
{
    ASSERT(user != NULL);
    ASSERT(pswd != NULL);
    if (!user || !pswd)
        return;

    int bufferLen = (strlen(pswd) / 16 + 1) * 32 + 4 + strlen(user) + 4;
    char* buffer = (char*)malloc(bufferLen);
    char encrypted[1024] = {0};

    int ret = aesEcbEncrypt((char*)pswd, strlen(pswd), LOCALKEY, encrypted, sizeof(encrypted));
    data2Hex(encrypted, ret, encrypted, sizeof(encrypted));
    snprintf(buffer, bufferLen, "%s:%s", user, encrypted);

    mode_t oldMask = umask(0077);
    FILE* fp = fopen(CONFIG_FILE_DIR"/monitorpasswd", "w+");
    umask(oldMask);
    if (!fp) {
        free(buffer);
        return;
    }
    int len, total;
    total = 0;
    len = strlen(buffer);
    while (total < len) {
        int bytes_written = fwrite(buffer + total, 1, len - total, fp);
        if (bytes_written <= 0) {
            break;
        }
        total += bytes_written;
    }
    fclose(fp);
    free(buffer);
    return;
}

void resetMonitorInfo(void)
{
    saveMonitorInfo("huawei", ".287aW");
}

#ifdef __cplusplus
} // extern "C"
#endif

