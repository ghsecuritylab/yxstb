
#include "SettingDigest.h"

#include "Assertions.h"
#include "openssl/sha.h"
#include "ind_mem.h"
#include "mid/mid_mutex.h"
#include "config/pathConfig.h"
#include "NetworkFunctions.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define YX_MAX_DIGEST_COUNT 20
#define YX_DIGEST_PATH CONFIG_FILE_DIR"/.digest"

static int getMacSHA256Value(char * buf, int len)
{
    char macAdd[20] = {0};
    char digestValue[65] = {0};
    SHA256_CTX sha256;
    unsigned char retBuf[32] = {0};
    int i;

    network_tokenmac_get(macAdd, 20, 0);
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, macAdd, strlen(macAdd));
    SHA256_Final(retBuf, &sha256);

    for (i = 0; i < 32; i++)
        sprintf(&digestValue[i * 2], "%02X", retBuf[i]);
    if (len<=64)
        return -1;
    IND_MEMCPY(buf, digestValue, 64);
    return 0;
}

int updateFileDigest( const char * pFilePath)
{
    if (access(pFilePath, (F_OK | R_OK))) {
        LogUserOperError("access[%s]\n", pFilePath);
        return -1;
    }

    char *fileNamePtr =  (char *)strrchr(pFilePath, '/');
    if (!fileNamePtr) {
        LogUserOperError("strrchr[%s]\n", pFilePath);
        return -1;
    }
    fileNamePtr++;//point to the file name

    char macDigest[65] = {0};
    getMacSHA256Value(macDigest, 65);

    int readLen = 0;
    char readBuf[1025] = {0};
    FILE *fp = fopen(pFilePath, "r");
    if (!fp) {
        LogUserOperError("checkFileDigest[%s]\n", pFilePath);
        return -1;
    }
    unsigned char digestValue[65] = {0};
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    while(1) {
        readLen = fread(readBuf, 1, 1024, fp);
        if (readLen != 1024) {
            if (feof(fp)) {//to the end of the file
                SHA256_Update(&sha256, readBuf, readLen);
                fclose(fp);
                break;
            } else {//error happens
                LogUserOperError("checkFileDigest[%s]\n", pFilePath);
                fclose(fp);
                return -1;
            }
        }
        SHA256_Update(&sha256, readBuf, readLen);
        IND_MEMSET(readBuf, 0, sizeof(readBuf));
    }
    SHA256_Update(&sha256, macDigest, 64);
    unsigned char retBuf[32] = {0};
    SHA256_Final(retBuf, &sha256);

    int i;
    for (i = 0; i < 32; i++)
        sprintf((char*)&digestValue[i * 2], "%02X", retBuf[i]);

//    LogUserOperDebug("digestValue[%s]\n", digestValue);

    char *digestString[YX_MAX_DIGEST_COUNT] = {NULL};
    int digestCount = 0;
    int j = 0;
    FILE *fileFp = fopen(YX_DIGEST_PATH, "r");
    if (fileFp) {//file exist and open it successfully
        for(digestCount = 0; digestCount<YX_MAX_DIGEST_COUNT; digestCount++) {
            digestString[digestCount] = (char*)IND_MALLOC(1024);
            IND_MEMSET(digestString[digestCount], 0, 1024);
            if (!fgets(digestString[digestCount], 1024, fileFp))
                break;
        }
        fclose(fileFp);
        if (digestCount == YX_MAX_DIGEST_COUNT) {
            LogUserOperError("YX_MAX_DIGEST_COUNT may not large enough\n");
            digestCount--;
        }

        for(j = 0; j<=digestCount; j++) {
            if (strstr(digestString[j], fileNamePtr)) {
                //LogUserOperDebug("find digestString[%s]\n", digestString[j]);
                break;
            }
        }
        mode_t oldMask = umask(0077);
        fileFp = fopen(YX_DIGEST_PATH, "w");
        umask(oldMask);
        if (!fileFp) {
            LogUserOperError("access[%s]\n", pFilePath);
            for(j = 0; j <=digestCount; j++) {
                if(digestString[j])
                    IND_FREE(digestString[j]);
            }
            return -1;
        }

        if (j <= digestCount) {//find digest recoard
            char * digestPtr = strstr(digestString[j], "::");
            if(!digestPtr) {
                for(j = 0; j <=digestCount; j++) {
                    if(digestString[j])
                        IND_FREE(digestString[j]);
                }
                return -1;
            }
            digestPtr = digestPtr+2;
            IND_MEMCPY(digestPtr, digestValue, 64);
        } else {
            char temp[1024] = {0};
            sprintf(temp, "%s::%s\n", fileNamePtr, digestValue);
            fputs(temp, fileFp);//add recoard
        }

        for(j = 0; j <=digestCount; j++) {
            fputs(digestString[j], fileFp);
            IND_FREE(digestString[j]);
        }
        fflush(fileFp);
        fclose(fileFp);
    }else {
        mode_t oldMask = umask(0077);
        fileFp = fopen(YX_DIGEST_PATH, "w");
        umask(oldMask);
        if (!fileFp) {
            LogUserOperError("access[%s]\n", pFilePath);
            return -1;
        }
        char temp[1024] = {0};
        sprintf(temp, "%s::%s\n", fileNamePtr, digestValue);
        fputs(temp, fileFp);
        fflush(fileFp);
        fclose(fileFp);
    }

    return 0;
}

int checkFileDigest(const char * pFilePath)
{
//  LogUserOperDebug("checkFileDigest[%s]\n", pFilePath);
    if(access(pFilePath, (F_OK | R_OK))) {
        LogUserOperError("access[%s]\n", pFilePath);
        return -1;
    }

    if (access(YX_DIGEST_PATH, (F_OK | R_OK)) ) {
        LogUserOperError("access[%s]\n", YX_DIGEST_PATH);
        return -1;
    }

    char *fileNamePtr =  (char *)strrchr(pFilePath, '/');
    if (!fileNamePtr) {
        LogUserOperError("strrchr[%s]\n", pFilePath);
        return -1;
    }
    fileNamePtr++;//point to the file name

    char *pDigistPtr = NULL;
    FILE *fileFp = fopen(YX_DIGEST_PATH, "r");
    if (!fileFp) {
        LogUserOperError("checkFileDigest[%s]\n", YX_DIGEST_PATH);
        return -1;
    }

    char lineBuf[1024] = {0};
    while(1) {
        if (!fgets(lineBuf, 1024, fileFp) ){//error happens or to the end
            fclose(fileFp);
            LogUserOperError("checkFileDigest[%s]\n", YX_DIGEST_PATH);
            return -1;
        }
        char * tempPtr = strstr(lineBuf, fileNamePtr);
        if (tempPtr) {//find the file name
            fclose(fileFp);
            break;
        }
    }
    pDigistPtr = strstr(lineBuf, "::");
    if (!pDigistPtr) {
        LogUserOperError("pDigistPtr is NULL\n");
        return -1;
    }
    pDigistPtr = pDigistPtr +2;//point to digest string

//    LogUserOperDebug("pDigistPtr[%s]\n", pDigistPtr);

    char macDigest[65] = {0};
    getMacSHA256Value(macDigest, 65);

    int readLen = 0;
    char readBuf[1025] = {0};
    FILE *fp = fopen(pFilePath, "r");
    if (!fp) {
        LogUserOperError("checkFileDigest[%s]\n", pFilePath);
        return -1;
    }
    unsigned char digestValue[65] = {0};
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    while(1) {
        readLen = fread(readBuf, 1, 1024, fp);
        if (readLen != 1024) {
            if (feof(fp)) {//to the end of the file
                SHA256_Update(&sha256, readBuf, readLen);
                fclose(fp);
                break;
            } else {
                LogUserOperError("checkFileDigest[%s]\n", pFilePath);
                fclose(fp);
                return -1;
            }
        }
        SHA256_Update(&sha256, readBuf, readLen);
        IND_MEMSET(readBuf, 0, sizeof(readBuf));
    }
    SHA256_Update(&sha256, macDigest, 64);
    unsigned char retBuf[32] = {0};
    SHA256_Final(retBuf, &sha256);

    int i;
    for (i = 0; i < 32; i++)
        sprintf((char*)&digestValue[i * 2], "%02X", retBuf[i]);

//    LogUserOperDebug("digestValue[%s]\n", digestValue);

    if (memcmp(digestValue, pDigistPtr, 64)) {
        LogUserOperError("someone changed the config file [%s], stb will delete the file\n", fileNamePtr);
        remove(pFilePath);
    }

    return 0;
}